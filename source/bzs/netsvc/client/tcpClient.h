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

#include <bzs/netsvc/client/iconnection.h>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/mutex.hpp>
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
#define CLIENT_ERROR_CANT_CREATEPIPE 3106
#define CLIENT_ERROR_SHAREMEM_DENIED 3104
#define CLIENT_ERROR_CONNECTION_FAILURE 3106
#define MAX_DATA_SIZE 10485760 // 10MB

namespace bzs
{
namespace netsvc
{
namespace client
{

typedef bool (*handshake)(connection* c, void* data);
class connections
{
    std::vector<connection*> m_conns;
    boost::asio::io_service m_ios;
    mutex m_mutex;
    std::string m_pipeName;
    static bool m_usePipedLocal;

    connection* getConnection(asio::ip::tcp::endpoint& ep);
    asio::ip::tcp::endpoint endpoint(const std::string& host,
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
    connection* connect(const std::string& host, handshake f,
                        void* data, bool newConnection = false);
    connection* getConnection(const std::string& host);
    bool disconnect(connection* c);
    int connectionCount();
    static char port[PORTNUMBUF_SIZE];
    static short timeout;
};

/** Implementation of Part of the connection interface
 */
class connectionBase : public connection
{
protected:
    friend class connections;

    asio::io_service m_ios;
    asio::ip::tcp::endpoint m_ep;
    std::vector<char> m_readbuf;
    std::vector<char> m_sendbuf;
    idirectReadHandler* m_reader;
    size_t m_readLen;
    int m_refCount;
    int m_charsetServer;
    bool m_connected;
    bool m_isHandShakable;

    void addref() { ++m_refCount; }

    void release() { --m_refCount; }

    int refCount() const { return m_refCount; }

    bool isConnected() const { return m_connected; };

    const asio::ip::tcp::endpoint& endpoint() const { return m_ep; }

    int charsetServer() const { return m_charsetServer; };

    void setCharsetServer(int v) { m_charsetServer = v; }

public:
    connectionBase(asio::ip::tcp::endpoint& ep)
        : m_ep(ep), m_reader(NULL), m_refCount(0),
          m_charsetServer(-1), m_connected(false), m_isHandShakable(true)
    {
    }

    void setDirectReadHandler(idirectReadHandler* p){ m_reader = p; }

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

    bool isHandShakable() const {return m_isHandShakable;};
};

/** Implementation of the connection template
 */
template <class T> class connectionImple : public connectionBase
{
#ifdef LINUX
    sigset_t m_signmask, m_sigomask;
#endif
protected:
    //unsigned int m_datalen;
    //unsigned short m_rows;
    T m_socket;
    buffers m_optionalBuffes;

    //unsigned int datalen() const { return m_datalen; }

    //unsigned short rows() const { return m_rows; }

    // server send any segment of lower than 0xFFFF data by asyncWrite
    // last 4byte is 0xFFFFFFFF, that is specify end of data
    /*void segmentRead()
    {
        bool end = false;
        unsigned short n;
        while (!end)
        {
            boost::asio::read(m_socket, boost::asio::buffer(&n, 2),
                              boost::asio::transfer_all());

            if (m_readLen + n > m_readbuf.size())
                m_readbuf.resize(m_readLen + n);

            m_readLen += boost::asio::read(
                m_socket, boost::asio::buffer(&m_readbuf[m_readLen], n),
                boost::asio::transfer_all());

            end = (*((unsigned int*)(&m_readbuf[m_readLen - 4])) == 0xFFFFFFFF);
        }
        m_readLen -= 4;
        // additinal data length info
        boost::asio::read(m_socket, boost::asio::buffer(&m_datalen, 4),
                          boost::asio::transfer_all());
        boost::asio::read(m_socket, boost::asio::buffer(&m_rows, 2),
                          boost::asio::transfer_all());
    }*/
    bool queryFunction(unsigned int v)
    {
        if (v == CONNECTION_FUNCTION_DIRECT_READ)
            return true;
        return false;
    }

    unsigned int directRead(void* buf, unsigned int size)
    {
        return (unsigned int)boost::asio::read(m_socket, 
                                                boost::asio::buffer(buf, size),
                                                boost::asio::transfer_all());
    }
    
    void* directReadRemain(unsigned int size)
    {
        if (size > m_readbuf.size())
            m_readbuf.resize(size);
        
        m_readLen += boost::asio::read(
            m_socket,
            boost::asio::buffer(&m_readbuf[0], size),
            boost::asio::transfer_all());
        return &m_readbuf[0];
    }

    template <typename CompletionCondition, typename MutableBufferSequence>
    size_t doRead(const MutableBufferSequence& buf, CompletionCondition cnd)
    {
        boost::system::error_code e;
#ifdef LINUX
        pthread_sigmask(SIG_SETMASK, &m_signmask , &m_sigomask); 
#endif
        size_t n = boost::asio::read(m_socket, buf, cnd, e);
#ifdef LINUX
        pthread_sigmask(SIG_SETMASK, &m_sigomask, NULL); 
#endif
        if (e) throw e;
        return n;    
    }

    char* read()
    {
        if (!m_connected)
            throw system_error(asio::error::not_connected);
        m_readLen = 0;
        //m_datalen = 0;
        //m_rows = 0;
        unsigned int n;

        //m_readLen += boost::asio::read(m_socket, boost::asio::buffer(&n, 4),
        //                               boost::asio::transfer_all());
        /*if (n == 0xFFFFFFFF)
        {
            segmentRead();
            m_readLen += boost::asio::read(m_socket, boost::asio::buffer(&n, 4),
                                           boost::asio::transfer_all());
        }*/
        if (m_reader)
        {
            m_readLen = doRead(boost::asio::buffer(&n, 4), boost::asio::transfer_all());
            
            m_readLen += m_reader->onRead(n - 4, this);
        }else
        {
            m_readLen = doRead(boost::asio::buffer(&m_readbuf[0],m_readbuf.size()),
                    boost::asio::transfer_at_least(4));
            n = *((unsigned int*)(&m_readbuf[0]));
        }
        if ((n > m_readLen) && (n < MAX_DATA_SIZE))
        {
            if (n > m_readbuf.size())
                m_readbuf.resize(n);
            m_readLen += doRead(boost::asio::buffer(&m_readbuf[m_readLen], n - m_readLen),
                    boost::asio::transfer_all());
        }
        return &m_readbuf[0];
    }

    void write(unsigned int writeSize)
    {
        if (!m_connected)
            throw system_error(asio::error::not_connected);
        if (m_optionalBuffes.size())
        {
            m_optionalBuffes.insert(m_optionalBuffes.begin(),
                                    asio::buffer(sendBuffer(0), writeSize));
            boost::asio::write(m_socket, m_optionalBuffes,
                               boost::asio::transfer_all());
            m_optionalBuffes.clear();
        }
        else
            boost::asio::write(m_socket, asio::buffer(sendBuffer(0), writeSize),
                               boost::asio::transfer_all());
    }

public:
    connectionImple(asio::ip::tcp::endpoint& ep)
        : connectionBase(ep)/*, m_datalen(0)*/, m_socket(m_ios)
    {
#ifdef LINUX
        sigfillset(&m_signmask);
#endif
    }

    ~connectionImple()
    {
        try
        {
            m_ios.stop();
            m_socket.close();
        }
        catch (...)
        {
        }
    }

    char* asyncWriteRead(unsigned int writeSize)
    {
        write(writeSize);
        return read();
    }

    buffers* optionalBuffers() { return &m_optionalBuffes; }
};


/** Implementation of The TCP connection.
 */
class tcpConnection : public connectionImple<asio::ip::tcp::socket>
{

public:
    tcpConnection(asio::ip::tcp::endpoint& ep)
        : connectionImple<asio::ip::tcp::socket>(ep)
    {
        m_readbuf.resize(READBUF_SIZE);
        m_sendbuf.resize(WRITEBUF_SIZE);
    }

    void setupTimeouts()
    {
#if defined _WIN32
        int32_t timeout = connections::timeout;
        setsockopt(m_socket.native(), SOL_SOCKET, SO_RCVTIMEO,
                   (const char*)&timeout, sizeof(timeout));
        setsockopt(m_socket.native(), SOL_SOCKET, SO_SNDTIMEO,
                   (const char*)&timeout, sizeof(timeout));
#else
        struct timeval timeout;
        timeout.tv_usec = 0;
        timeout.tv_sec = connections::timeout;
        setsockopt(m_socket.native(), SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout));
        setsockopt(m_socket.native(), SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout));
#endif
    }

    void connect()
    {
        setupTimeouts();
        m_socket.connect(m_ep);
        m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
        m_connected = true;
    }
};

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

    void throwException(const char* msg, int errorCode)
    {
        char buf[4096];
        char user[128];
        char* p = GetErrorMessage(GetLastError());
        DWORD size = 128;
        GetUserName(user, &size);
        sprintf_s(buf, 4096, "User:%s %s %d %s", user, msg, GetLastError(), p);
        LocalFree(p);
        throw exception(errorCode, buf);
    }

    char* getUniqName(const char* name)
    {
        connectionBase::m_readbuf.resize(120);
        char* p = &connectionBase::m_readbuf[0];
        DWORD processId = GetCurrentProcessId();
        __int64 clientid = (__int64) this;
        sprintf_s(p, 120, "%s_%u_%Lu", name, processId, clientid);
        return p;
    }

    void createKernelObjects(unsigned int shareMemSize)
    {
        char tmp[50];
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        int size = shareMemSize / SystemInfo.dwAllocationGranularity + 1;
        m_sendBufferSize = size * SystemInfo.dwAllocationGranularity;

        sprintf_s(tmp, 50, "Global\\%s", m_pipeName.c_str());
        m_mapFile =
            CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                              m_sendBufferSize * 2, getUniqName(tmp));
        if (m_mapFile == NULL)
            throwException("CreateFileMapping", CLIENT_ERROR_SHAREMEM_DENIED);

        m_writebuf_p = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0,
                                            0, m_sendBufferSize);
        if (m_writebuf_p == NULL)
            throwException("MapViewOfFile R", CLIENT_ERROR_SHAREMEM_DENIED);

        m_readbuf_p = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0,
                                           m_sendBufferSize, m_sendBufferSize);
        if (m_readbuf_p == NULL)
            throwException("MapViewOfFile W", CLIENT_ERROR_SHAREMEM_DENIED);

        sprintf_s(tmp, 50, "Global\\%sToClnt", m_pipeName.c_str());
        m_recvEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, getUniqName(tmp));
        if (m_recvEvent == NULL)
            throwException("OpenEvent Client", CLIENT_ERROR_SHAREMEM_DENIED);

        sprintf_s(tmp, 50, "Global\\%sToSrv", m_pipeName.c_str());
        m_sendEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, getUniqName(tmp));
        if (m_sendEvent == NULL)
            throwException("OpenEvent Server", CLIENT_ERROR_SHAREMEM_DENIED);
    }

    void write(unsigned int writeSize)
    {
        //m_datalen = 0;
        //m_rows = 0;
        BOOL ret = SetEvent(m_sendEvent);
        if (ret == FALSE)
            throwException("SetEvent", CLIENT_ERROR_CONNECTION_FAILURE);        
    }

    char* read()
    {
        while (WAIT_TIMEOUT == WaitForSingleObject(m_recvEvent, connections::timeout))
        {
            DWORD n = 0;
            BOOL ret = GetNamedPipeHandleState(m_socket.native(), NULL, &n,
                                           NULL, NULL, NULL, 0);
            if(ret == FALSE || n == 0)
                throwException("PipeConnection", CLIENT_ERROR_CONNECTION_FAILURE);
        }
        return m_readbuf_p;
    }

public:
    pipeConnection(asio::ip::tcp::endpoint& ep, const std::string& pipeName)
        : connectionImple<platform_stream>(ep), m_pipeName(pipeName),
          m_mapFile(NULL), m_recvEvent(NULL), m_sendEvent(NULL),
          m_writebuf_p(NULL), m_readbuf_p(NULL), m_sendBufferSize(0)

    {
    }

    ~pipeConnection()
    {
        if (m_connected)
		{
			if (m_writebuf_p)
				memset(m_writebuf_p, 0, sizeof(unsigned int));
			DWORD n = 0;
			BOOL ret = GetNamedPipeHandleState(m_socket.native(), NULL, &n,
                                            NULL, NULL, NULL, 0);
            if(m_sendEvent && ret && n)
            {
				SetEvent(m_sendEvent);
                //Wait for server side close connection
                while (WAIT_TIMEOUT == 
                    WaitForSingleObject(m_recvEvent, connections::timeout))
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
    }

    void connect()
    {
        platform_descriptor fd;
#ifdef WIN32
        char pipeName[100];
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
            throwException("CreateFile", CLIENT_ERROR_CANT_CREATEPIPE);
#endif // NOT WIN32
        m_socket.assign(fd);
        m_connected = true;

        // send processId and clientid;
        DWORD processId = GetCurrentProcessId();
        int size = 16;
        connectionBase::m_readbuf.resize(256);
        char* p = &connectionBase::m_readbuf[0];
        memcpy(p, &size, sizeof(int));
        memcpy(p + 4, &processId, sizeof(DWORD));
        __int64 clientid = (__int64) this;
        memcpy(p + 8, &clientid, sizeof(__int64));
        boost::asio::write(m_socket, boost::asio::buffer(p, size));

        boost::asio::read(m_socket, boost::asio::buffer(p, 7));
        unsigned int* shareMemSize = (unsigned int*)(p+3);
        m_isHandShakable = (p[0] == 0x00);
        createKernelObjects(*shareMemSize);
    }

    char* sendBuffer(size_t size) { return m_writebuf_p; }

    unsigned int sendBufferSize() { return m_sendBufferSize; }

    buffers* optionalBuffers() { return NULL; } // not support

    void setReadBufferSizeIf(size_t size) {} // not support
};
#endif // NOT WIN32

} // namespace client
} // namespace netsvc
} // namespace bzs

#endif // BZS_NETSVC_CLINET_TCPCLIENT_H
