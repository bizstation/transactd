#ifndef BZS_NETSVC_IAPPMODULE_H
#define BZS_NETSVC_IAPPMODULE_H
/*=================================================================
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
=================================================================*/

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

/** Application framework interface
 *	IModuleBuilder: module builder.
 *	IAppModule :The module created for every session.
 *              This receives received data and returns a result.
 *  It will be done, if these interfaces are implement and a server's
 *constructor is passed.
 */

namespace bzs
{
namespace netsvc
{
namespace server
{

class iconnection;

#define EXECUTE_RESULT_SUCCESS 0
#define EXECUTE_RESULT_QUIT 1
#define EXECUTE_RESULT_FORCSE_ASYNC 2
#define EXECUTE_RESULT_FORCSE_SYNC 3
#define EXECUTE_RESULT_ACCESS_DNIED 4
typedef std::vector<boost::asio::const_buffer> buffers;
typedef std::vector<char> vector_buffer;

class IResultBuffer
{
public:
    virtual void resize(size_t v) = 0;
    virtual size_t size() const = 0;
    virtual char* ptr() = 0;
};

class vecBuffer : public IResultBuffer
{
    vector_buffer& m_vecbuf;

public:
    vecBuffer(vector_buffer& v) : m_vecbuf(v) {}
    void resize(size_t v) { m_vecbuf.resize(v); }
    size_t size() const { return m_vecbuf.size(); }
    virtual char* ptr() { return &m_vecbuf[0]; }
};

class IAppModule
{
public:
    virtual ~IAppModule(){};
    // virtual int execute(char* result, size_t& size, buffers* optionalData) =
    // 0;
    virtual int execute(IResultBuffer& result, size_t& size,
                        buffers* optionalData) = 0;
    virtual size_t onRead(const char* data, size_t size, bool& complete) = 0;
    virtual size_t onAccept(char* message, size_t bufsize) = 0;
    virtual void reset() = 0;
    virtual bool isShutDown() = 0;
    virtual bool checkHost(const char* hostCheckname, /*out*/char* hostName, int size) = 0;
    virtual void cleanup() = 0;
    virtual const char* user() const = 0;
    virtual const char* host() const = 0;
    virtual void setUser(const char* v) = 0;
    virtual void setHost(const char* v) = 0;
    virtual void skipGrants(bool v) = 0;
    virtual bool isSkipGrants() const = 0;
};

// Defines at Implementing of IAppModule , two variables below.
extern boost::mutex modulesMutex;
extern std::vector<IAppModule*> modules;

#define SERVER_TYPE_TPOOL 1
#define SERVER_TYPE_CPT 0

/**
 * used by worker thread
 * use with dynamic_cast for your application
 */
class IAppModuleBuilder
{
public:
    virtual ~IAppModuleBuilder(){};
    virtual IAppModule*
    createSessionModule(const boost::asio::ip::tcp::endpoint& endpoint,
                        iconnection* connection, bool tpool) = 0;
};

/* Currently, this writer used by resultWriter and request::serializeForExt only
*/
class netWriter
{
protected:
    IResultBuffer* resultBuffer;

    buffers* m_optionalData;
    char* m_ptr;
    char* m_curPtr;
    unsigned int m_clientBuffferSize;

public:
    enum eWriteMode
    {
        copyOnly,
        curSeekOnly,
        netwrite,
        writeEnd
    };
    netWriter() : datalen(0) {}
    virtual ~netWriter(){};
    inline void setClientBuffferSize(unsigned int v)
    {
        m_clientBuffferSize = v;
        if (bufferSize() < v + 100)
            resize(v + 100);
    }

    inline void resize(size_t v)
    {
        resultBuffer->resize(v);
        m_ptr = resultBuffer->ptr();
    };
    inline size_t bufferSize() const { return resultBuffer->size(); };
    inline buffers* optionalData() { return m_optionalData; };
    inline char* ptr() const { return m_ptr; }
    inline char* curPtr() const { return m_curPtr; }
    virtual unsigned int resultLen() const = 0;

    /*  At non recordset operations, the datalen variable is a holder of total
       send data size .
            At recordset operations, the datalen variable is specify current
       send data size - RETBUF_EXT_RESERVE_SIZE.
    */
    size_t datalen;

    /* Used by recordsetHndler only */
    virtual size_t bufferSpace() const
    {
        return resultBuffer->size() - (curPtr() - m_ptr);
    }

    virtual void reset(IResultBuffer* retBuf, buffers* optData)
    {
        resultBuffer = retBuf;
        m_optionalData = optData;
        datalen = 0;
        m_ptr = resultBuffer->ptr();
    }
    virtual void beginExt(bool includeBlob) = 0;
    virtual bool asyncWrite(const char* p, unsigned int n,
                            eWriteMode mode = copyOnly) = 0;
    virtual bool write(const char* p, size_t n, eWriteMode mode = copyOnly) = 0;
    virtual void incremetRows() = 0;
    virtual unsigned short getParamMask(bool includeBlob) = 0;
    virtual void writeHeadar(unsigned short paramMask, short result) = 0;
    virtual unsigned int allreadySent() const = 0;
};

} // namespace server
} // namespace netsvc
} // namespace bzs
#endif // BZS_NETSVC_IAPPMODULE_H
