#ifndef BZS_NETSVC_CLINET_TCPCLIENT_H
#define BZS_NETSVC_CLINET_TCPCLIENT_H
/* =================================================================
 Copyright (C) 2012 2013 BizStation Corp All rights reserved.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 02111-1307, USA.
 ================================================================= */

#pragma warning(disable : 4005) //BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT redefine bug 
#include <bzs/netsvc/client/iconnection.h>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#pragma warning(default : 4005)

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#if (BOOST_VERSION > 104900)
#include <boost/asio/deadline_timer.hpp>
#else
#if (!defined(__BCPLUSPLUS__) || defined(__clang__) || (__BCPLUSPLUS__ > 0x0651))
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#else
#define NO_CONNECT_TIMER
#endif
#endif


#include <stdio.h>
#include <vector>
#ifdef LINUX
#include <pthread.h> 
#include <signal.h>
#endif


using namespace boost;
using namespace boost::system;

#define READBUF_SIZE 66000
#define WRITEBUF_SIZE 66000

#define PORTNUMBUF_SIZE 10
#define CLIENT_ERROR_CANT_CREATEPIPE    3106
#define CLIENT_ERROR_SHAREMEM_DENIED    3104
#define CLIENT_ERROR_CONNECTION_FAILURE 3106
#define ERROR_TD_NET_TOO_BIGDATA        3802
#define MAX_DATA_SIZE 10485760 // 10MB

#ifdef _WIN32
extern bool win_thread_pool_shutdown;
#endif

namespace bzs
{
namespace netsvc
{
namespace client
{

#ifndef _WIN32

class signalmask
{
    sigset_t m_signmask, m_sigomask;
public:
    signalmask()
    {
        sigfillset(&m_signmask);
        sigdelset(&m_signmask, SIGQUIT | SIGKILL | SIGHUP | SIGINT |SIGTERM); 
        pthread_sigmask(SIG_SETMASK, &m_signmask , &m_sigomask);
    }

    ~signalmask()
    {
        pthread_sigmask(SIG_SETMASK, &m_sigomask, NULL); 
    }
};
#endif

class exception : public std::exception
{
    int m_error;
    std::string m_msg;

public:
    exception(int errorCode, const char* const& message)
        : std::exception(), m_error(errorCode), m_msg(message)
    {
    }

    ~exception() throw() {}

    const char* what() const throw() { return m_msg.c_str(); }

    int error() { return m_error; }
};

typedef bool (*handshake)(connection* c, void* data);
class connections
{
    std::vector<connection*> m_conns;
    std::string m_pipeName;
    boost::asio::io_service m_ios;
    boost::asio::ip::tcp::resolver m_resolver;
    mutex m_mutex;
    boost::system::error_code m_e;
    static bool m_usePipedLocal;

    connection* getConnection(asio::ip::tcp::endpoint& ep);
    asio::ip::tcp::endpoint endpoint(const std::string& host,
                                     const char* port,
                                     boost::system::error_code& ec);
    bool isUseNamedPipe(asio::ip::tcp::endpoint& ep);
#ifdef USE_PIPE_CLIENT
    connection* getConnectionPipe();
#endif
    inline connection* doConnect(connection* c);
    inline connection* createConnection(asio::ip::tcp::endpoint& ep, bool namedPipe);
    inline bool doHandShake(connection* c, handshake f, void* data);
public:
    connections(const char* pipeName);
    ~connections();
    connection* connect(const std::string& host, const char* port, handshake f,
                        void* data, bool newConnection = false);
    bool reconnect(connection* c, const std::string& host, const char* port,
                        handshake f, void* data);

    connection* getConnection(const std::string& host, const char* port);
    bool disconnect(connection* c);
    int connectionCount();
    const boost::system::error_code& connectError() const { return m_e; };
    static char m_port[PORTNUMBUF_SIZE];
    static int connectTimeout;
    static int netTimeout;
};

/** Implementation of Part of the connection interface
 */
#define VER_ST_SIZE 12
class connectionBase : public connection
{
protected:
    friend class connections;

    asio::io_service* m_ios;
    asio::ip::tcp::endpoint m_ep;
    idirectReadHandler* m_reader;
    size_t m_readLen;
    int m_refCount;
    int m_charsetServer;
    char m_vers[VER_ST_SIZE];
    bool m_connected;
    bool m_isHandShakable;
    boost::system::error_code m_e;

    void addref() { ++m_refCount; }

    void release() { --m_refCount; }

    int refCount() const { return m_refCount; }

    bool isConnected() const { return m_connected; };

    const asio::ip::tcp::endpoint& endpoint() const { return m_ep; }

    int charsetServer() const { return m_charsetServer; };

    void setCharsetServer(int v) { m_charsetServer = v; }
  
public:
    connectionBase(asio::ip::tcp::endpoint& ep)
        : m_ios(new asio::io_service),
          m_ep(ep), m_reader(NULL), m_refCount(0),
          m_charsetServer(-1), m_connected(false), m_isHandShakable(true)
    {
        memset(m_vers, 0, VER_ST_SIZE);
    }
    virtual ~connectionBase()
    {
        #ifdef _WIN32
        if (!win_thread_pool_shutdown)
            delete m_ios;
        #else
        delete m_ios;
        #endif
    }
    void setDirectReadHandler(idirectReadHandler* p){ m_reader = p; }
    bool isHandShakable() const {return m_isHandShakable;};
    const boost::system::error_code& error() const { return m_e; };

    void* versions() {return (void*)m_vers;};
};

#ifdef __APPLE__
template <class T>
class asio_tcp_io
{
    T& m_socket;
public:
    asio_tcp_io(T& socket):m_socket(socket){}

    size_t readAll(char* buf, size_t size, system::error_code& e)
    {
        return asio::read(m_socket, asio::buffer(buf, size),
                            asio::transfer_all(), e);
    }

    size_t readSome(char* buf, size_t size, size_t minimum,
                            boost::system::error_code& e)
    {
        return asio::read(m_socket, asio::buffer(buf, size),
                            asio::transfer_at_least(minimum), e);
    }

    void write(const char* buf, size_t size, int /*flag*/, boost::system::error_code& e)
    {
        asio::write(m_socket, asio::buffer(buf, size),
                            asio::transfer_all(), e);
    }

    template <typename MutableBufferSequence>
    void writeMultibuffer(const MutableBufferSequence& buffer,
                                                boost::system::error_code& e)
    {
        asio::write(m_socket, buffer, asio::transfer_all(), e);
    }

    void on_connected() {}

};
#else // __APPLE__  (not __APPLE__)

#if (defined(_WIN32))
#define MSG_WAITALL 0x8
#define MSG_EOR 0
#endif

#ifndef MSG_MORE
#define MSG_MORE 0
#endif

#ifdef LINUX
#define SOCKET_ERROR -1
#endif

#if (BOOST_VERSION > 104900)
#define SYSTEM_CATEGORY system_category()
#else
#define SYSTEM_CATEGORY system_category
#endif

inline int getErrorCode()
{
#ifndef _WIN32
    return errno;
#else
    return WSAGetLastError();
#endif
}

template <class T>
class native_tcp_io
{
    T& m_socket;

public:
    native_tcp_io(T& socket):m_socket(socket){}

#ifdef _WIN32
    size_t readAll2(char* buf, size_t size, system::error_code& e)
    {
        int n = 0;
        do
        {
            int nn = recv(m_socket.native(), buf + n, (int)size - n, 0);
            if (n == SOCKET_ERROR)
            {
                e = error_code(getErrorCode(), SYSTEM_CATEGORY);
                break;
            }
            n += nn;
        }while (n != size);
        return (size_t)n;
    }
#endif

    size_t readAll(char* buf, size_t size, system::error_code& e)
    {
        errno = 0;
        int n = recv(m_socket.native(), buf, (int)size, MSG_WAITALL);
        if (n == SOCKET_ERROR)
        {
#ifdef _WIN32
            if (WSAEOPNOTSUPP == getErrorCode())
                return readAll2(buf, size, e);
            else
#endif
                e = error_code(getErrorCode(), SYSTEM_CATEGORY);
        }
        return (size_t)n;
    }

    size_t readSome(char* buf, size_t size, size_t minimum,
                                                boost::system::error_code& e)
    {
        errno = 0;
        int n = recv(m_socket.native(), buf, (int)size, 0);
        if (n == SOCKET_ERROR)
            e = error_code(getErrorCode(), SYSTEM_CATEGORY);
        return (size_t)n;
    }

    void write(const char* buf, size_t size, int flag, boost::system::error_code& e)
    {
        errno = 0;
        int n = send(m_socket.native(), buf, (int)size, flag);
        if (n == SOCKET_ERROR)
            e = error_code(getErrorCode(), SYSTEM_CATEGORY);
    }

    template <typename MutableBufferSequence>
    void writeMultibuffer(const MutableBufferSequence& buffer, boost::system::error_code& e)
    {
        buffers::const_iterator it = buffer.begin();
        buffers::const_iterator ite = buffer.end();
        while (it != ite)
        {
            std::size_t s = asio::buffer_size(*it);
            const char* p = asio::buffer_cast<const char*>(*it);
            write(p, s, (it == (ite - 1)) ? MSG_EOR : MSG_MORE, e);
            if (e) break;
            ++it;
        }
    }

    void on_connected()
    {
    #ifndef _WIN32
        int val = 0;
        ioctl(m_socket.native(), FIONBIO, &val);
    #else
        u_long val = 0;
        ioctlsocket(m_socket.native(), FIONBIO, &val);
    #endif
    }
};
#endif // __APPLE__

template <class T> class connectionImple : public connectionBase
{
protected:
    T m_socket;
    buffers m_optionalBuffes;

    bool checkError(const system::error_code& e)
    {
        if (e)
            return false;
        return true;
    }

    void cleanup()
    {
        if (m_connected)
        {
            try
            {
                m_ios->stop();
                m_socket.close();
                m_ios->reset();
            }
            catch (...)
            {
            }
            m_connected = false;
        }
    }

    ~connectionImple()
    {
        cleanup();
    }

    char* asyncWriteRead(unsigned int writeSize)
    {
#ifndef _WIN32
        signalmask smask;
#endif
        write(writeSize);
        if (m_e) return NULL;
        char* p = read();
        return p;
    }

    buffers* optionalBuffers() { return &m_optionalBuffes; }

    void reconnect(asio::ip::tcp::endpoint& ep)
    {
        cleanup();
        m_ios->reset();
        m_ep = ep;
        connect();
    }

public:
    connectionImple(asio::ip::tcp::endpoint& ep)
        : connectionBase(ep)/*, m_datalen(0)*/, m_socket(*m_ios)
    {
    }
};

#define _size_holder asio::placeholders::bytes_transferred
#define _error_holder asio::placeholders::error

/** Implementation of The TCP connection.
 */
#ifndef NO_CONNECT_TIMER
#define USE_CONNECT_TIMER
#endif

template <class T>
class tcpConnection : public connectionImple<asio::ip::tcp::socket>
{
    T s_io;
#ifdef USE_CONNECT_TIMER
    asio::deadline_timer m_timer;
#endif
    std::vector<char> m_readbuf;
    std::vector<char> m_sendbuf;

    void resizeReadBuffer(unsigned int n)
    {
        if (n > MAX_DATA_SIZE)
            throw exception(ERROR_TD_NET_TOO_BIGDATA, "read");
        if (n > m_readbuf.size())
            m_readbuf.resize(n);
    }

    char* sendBuffer(size_t size)
    {
        if (size > m_sendbuf.size())
            m_sendbuf.resize(size);
        return &m_sendbuf[0];
    }

    unsigned int sendBufferSize() { return (unsigned int)m_sendbuf.size(); };

    void setReadBufferSizeIf(size_t size)
    {
        if (m_readbuf.size() < size)
            m_readbuf.resize(size);
    }

    bool queryFunction(unsigned int v)
    {
        if (v == CONNECTION_FUNCTION_DIRECT_READ)
            return true;
        return false;
    }

    void cleanup()
    {
#ifdef USE_CONNECT_TIMER
        m_timer.cancel();
#endif
        connectionImple<asio::ip::tcp::socket>::cleanup();
    }
#ifdef USE_CONNECT_TIMER
    void setTimer(int time)
    {
#ifdef _WIN32
        time /= 1000;
#endif
        m_timer.cancel();
        m_timer.expires_from_now(boost::posix_time::seconds(time));
        m_timer.async_wait(boost::bind(&tcpConnection::on_timer, this, _1));
    }
#endif

    void on_connect(const boost::system::error_code& e)
    {
#ifdef USE_CONNECT_TIMER
        m_timer.cancel();
#endif
        m_e = e;
        if (!checkError(e))
            return ;
        s_io.on_connected(); 
        m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
        m_connected = true;
        setTimeouts(connections::netTimeout);
    }

    void on_timer(const boost::system::error_code& e) 
    {
        if (e)
        {
            m_e = asio::error::timed_out;
            return;
        }
        #pragma warning(disable : 4996)
        try{ m_socket.cancel(); } catch (...) {}
        #pragma warning(default : 4996)
    }

    void setTimeouts(int time)
    {
        struct timeval timeout;
        timeout.tv_usec = 0;
        timeout.tv_sec = time;
        int ret = setsockopt(m_socket.native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,
                   sizeof(timeout));
        ret = setsockopt(m_socket.native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout,
                   sizeof(timeout));
    }
#ifdef USE_CONNECT_TIMER
    void connect()
    {
        setTimer(connections::connectTimeout);
        m_socket.async_connect(m_ep,
            bind(&tcpConnection::on_connect, this, _error_holder));
        m_ios->run();
        m_ios->reset();
    }
#else
    void connect()
    {
        setTimeouts(connections::connectTimeout);
        m_socket.connect(m_ep);
        m_e = error_code(getErrorCode(), SYSTEM_CATEGORY);
        on_connect(m_e);
        m_ios->reset();
    }
#endif

    unsigned int directRead(void* buf, unsigned int size)
    {
        m_e.clear();
        unsigned int len = (unsigned int)s_io.readAll((char*)buf, size, m_e);
        return len;
    }

    void* directReadRemain(unsigned int size)
    {
        if (size > m_readbuf.size())
            m_readbuf.resize(size);
        m_e.clear();
        m_readLen += s_io.readAll(&m_readbuf[0], size, m_e);
        return &m_readbuf[0];
    }

    char* read()
    {
        m_readLen = 0;
        if (!m_connected)
        {
            m_e = asio::error::not_connected;
            return &m_readbuf[0];
        }
        unsigned int n;
        m_e.clear();
        if (m_reader)
        {
            m_readLen = s_io.readAll((char*)&n, 4, m_e);
            m_readLen += m_reader->onRead(n - 4, this);
        }else
        {
            m_readLen = s_io.readSome(&m_readbuf[0], m_readbuf.size(), 4, m_e);
            n = *((unsigned int*)(&m_readbuf[0]));
        }
        if (!m_e)
        {

            if (n > m_readLen)
            {
                resizeReadBuffer(n);
                m_readLen += s_io.readAll(&m_readbuf[m_readLen], n - m_readLen, m_e);
            }
        }
        return &m_readbuf[0];
    }

    void write(unsigned int writeSize)
    {
        if (!m_connected)
        {
            m_e = asio::error::not_connected;
            return;
        }
        m_e.clear();
        if (m_optionalBuffes.size())
        {
            m_optionalBuffes.insert(m_optionalBuffes.begin(),
                                    asio::buffer(sendBuffer(0), writeSize));
            s_io.writeMultibuffer(m_optionalBuffes, m_e);
            m_optionalBuffes.clear();
        }
        else
            s_io.write(sendBuffer(0), writeSize, 0/*MSG_EOR*/, m_e);
    }
public:
    tcpConnection(asio::ip::tcp::endpoint& ep)
        : connectionImple<asio::ip::tcp::socket>(ep),s_io(m_socket)
#ifdef USE_CONNECT_TIMER
        , m_timer(*m_ios)
#endif
    {
        m_readbuf.resize(READBUF_SIZE);
        m_sendbuf.resize(WRITEBUF_SIZE);
    }
};
#ifdef __APPLE__
    typedef tcpConnection<asio_tcp_io<asio::ip::tcp::socket> > asio_tcpConnection;
#else
    typedef tcpConnection<native_tcp_io<asio::ip::tcp::socket> > native_tcpConnection;
#endif

#ifdef USE_PIPE_CLIENT

/** Implementation of The Named pipe connection.
 */
class pipeConnection : public connectionImple<platform_stream>
{
    const std::string& m_pipeName;

    char* m_readbuf_p;
    char* m_writebuf_p;
    unsigned int m_sendBufferSize;

    HANDLE m_recvEvent;
    HANDLE m_sendEvent;
    HANDLE m_mapFile;

    unsigned int directRead(void* buf, unsigned int size)
    {
        assert(0);
        return 0;
    }
    
    void* directReadRemain(unsigned int size)
    {
        assert(0);
        return NULL;
    }

    bool queryFunction(unsigned int v)
    {
        if (v == CONNECTION_FUNCTION_DIRECT_READ)
            return false;
        return false;
    }

    char* GetErrorMessage(DWORD ErrorCode)
    {
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
            ErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR)&lpMsgBuf, 0, NULL);
        return (char*)lpMsgBuf;
    }

    char* getUniqName(const char* name, char* buf)
    {
        char* p = buf;
        DWORD processId = GetCurrentProcessId();
		unsigned __int64 clientid = (unsigned __int64) this;
        sprintf_s(p, 120, "%s_%u_%llu", name, processId, clientid);
        return p;
    }

    void createKernelObjects(unsigned int shareMemSize)
    {
        char tmp[50];
        char buf[120];
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        int size = shareMemSize / SystemInfo.dwAllocationGranularity + 1;
        m_sendBufferSize = size * SystemInfo.dwAllocationGranularity;

        sprintf_s(tmp, 50, "Global\\%s", m_pipeName.c_str());
        m_mapFile =
            CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                              m_sendBufferSize * 2, getUniqName(tmp, buf));
        if (m_mapFile == NULL)
        {
            m_e = boost::system::error_code(CLIENT_ERROR_SHAREMEM_DENIED, get_system_category());
            return;
        }

        m_writebuf_p = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0,
                                            0, m_sendBufferSize);
        if (m_writebuf_p == NULL)
        {
            m_e = boost::system::error_code(CLIENT_ERROR_SHAREMEM_DENIED, get_system_category());
            return;
        }

        m_readbuf_p = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0,
                                           m_sendBufferSize, m_sendBufferSize);
        if (m_readbuf_p == NULL)
        {
            m_e = boost::system::error_code(CLIENT_ERROR_SHAREMEM_DENIED, get_system_category());
            return;
        }

        sprintf_s(tmp, 50, "Global\\%sToClnt", m_pipeName.c_str());
        m_recvEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, getUniqName(tmp, buf));
        if (m_recvEvent == NULL)
        {
            m_e = boost::system::error_code(CLIENT_ERROR_SHAREMEM_DENIED, get_system_category());
            return;
        }

        sprintf_s(tmp, 50, "Global\\%sToSrv", m_pipeName.c_str());
        m_sendEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, getUniqName(tmp, buf));
        if (m_sendEvent == NULL)
            m_e = boost::system::error_code(CLIENT_ERROR_SHAREMEM_DENIED, get_system_category());
    }

    void write(unsigned int writeSize)
    {
        //m_datalen = 0;
        //m_rows = 0;
        m_e.clear();
        BOOL ret = SetEvent(m_sendEvent);
        if (ret == FALSE)
            m_e = asio::error::connection_aborted;
    }

    char* read()
    {
        int t = 0;
        m_e.clear();
        while (WAIT_TIMEOUT == WaitForSingleObject(m_recvEvent, 1000))
        {
            t += 1000;
            DWORD n = 0;
            BOOL ret = GetNamedPipeHandleState(m_socket.native(), NULL, &n,
                                           NULL, NULL, NULL, 0);
            if(ret == FALSE || n < 2)
                m_e = boost::system::error_code(CLIENT_ERROR_CONNECTION_FAILURE, get_system_category());
            else if (connections::netTimeout == t)
                m_e = asio::error::timed_out;
            if (m_e) break;
        }
        return m_readbuf_p;
    }

    ~pipeConnection()
    {
       cleanup();
    }

    void cleanup()
    {
        if (m_connected)
        {
            if (m_writebuf_p)
                memset(m_writebuf_p, 0, sizeof(unsigned int));
            DWORD n = 0;
            BOOL ret = GetNamedPipeHandleState(m_socket.native(), NULL, &n,
                                            NULL, NULL, NULL, 0);
            if(m_sendEvent && ret && n > 1)
            {
                SetEvent(m_sendEvent);
                while (WAIT_TIMEOUT ==  
                    WaitForSingleObject(m_recvEvent, 1000))
                    ;
            }
        }

        if (m_recvEvent)
            CloseHandle(m_recvEvent);
        if (m_sendEvent)
            CloseHandle(m_sendEvent);
        if (m_writebuf_p)
            UnmapViewOfFile(m_writebuf_p);
        if (m_readbuf_p)
            UnmapViewOfFile(m_readbuf_p);
        if (m_mapFile)
            CloseHandle(m_mapFile);
        m_sendEvent = NULL;
        m_recvEvent = NULL;
        m_writebuf_p = NULL;
        m_readbuf_p = NULL;
        m_mapFile = NULL;
        connectionImple<platform_stream>::cleanup();
    }

    void connect()
    {
        platform_descriptor fd;
#ifdef WIN32
        m_e.clear();
        char pipeName[100];
        if (m_ep.port() != 8610)
            sprintf_s(pipeName, 100, "\\\\.\\pipe\\%s%u", m_pipeName.c_str(), m_ep.port());
        else
            sprintf_s(pipeName, 100, "\\\\.\\pipe\\%s", m_pipeName.c_str());
        int i = 1000;
        while (--i)
        {
            fd = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
            if (fd != INVALID_HANDLE_VALUE)
                break;
            if (ERROR_PIPE_BUSY != GetLastError())
                break;
            Sleep(1);
        }
        if (fd == INVALID_HANDLE_VALUE)
        {
            m_e = boost::system::error_code(CLIENT_ERROR_CANT_CREATEPIPE, get_system_category());
            return;
        }
#endif // NOT WIN32

        m_socket.assign(fd);
        
        // send processId and clientid;
        DWORD processId = GetCurrentProcessId();
        int size = 16;
        char buf[256];
        char* p = buf;
        memcpy(p, &size, sizeof(int));
        memcpy(p + 4, &processId, sizeof(DWORD));
        __int64 clientid = (__int64) this;
        memcpy(p + 8, &clientid, sizeof(__int64));
        try
        {
            boost::asio::write(m_socket, boost::asio::buffer(p, size));
            boost::asio::read(m_socket, boost::asio::buffer(p, 7));
        }
        catch (boost::system::system_error& e)
        {
            m_e = e.code();
        }
        if (!m_e)
        {
            unsigned int* shareMemSize = (unsigned int*)(p+3);
            m_isHandShakable = (p[0] == 0x00);
            m_connected = true;
            createKernelObjects(*shareMemSize);
        }
    }

    char* sendBuffer(size_t size) { return m_writebuf_p; }

    unsigned int sendBufferSize() { return m_sendBufferSize; }

    buffers* optionalBuffers() { return NULL; } // not support

    void setReadBufferSizeIf(size_t size) {} // not support

public:
    pipeConnection(asio::ip::tcp::endpoint& ep, const std::string& pipeName)
        : connectionImple<platform_stream>(ep), m_pipeName(pipeName),
          m_mapFile(NULL), m_recvEvent(NULL), m_sendEvent(NULL),
          m_writebuf_p(NULL), m_readbuf_p(NULL), m_sendBufferSize(0)

    {
    }
};
#endif // NOT WIN32

} // namespace client
} // namespace netsvc
} // namespace bzs

#endif // BZS_NETSVC_CLINET_TCPCLIENT_H
