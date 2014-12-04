#ifndef BZS_NETSVC_CLINET_ICONNECTION_H
#define BZS_NETSVC_CLINET_ICONNECTION_H
/* =================================================================
 Copyright (C) 2014 BizStation Corp All rights reserved.

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
#ifdef __BCPLUSPLUS__
#pragma warn -8012
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <vector>

#ifdef __BCPLUSPLUS__
#pragma warn +8012
#endif

#ifdef _WIN32
#define USE_PIPE_CLIENT
#endif

#ifdef USE_PIPE_CLIENT
#include <boost/asio/windows/stream_handle.hpp>
using boost::asio::windows::stream_handle;
typedef stream_handle platform_stream;
typedef HANDLE platform_descriptor;
#define PIPE_EOF_ERROR_CODE boost::system::windows_error::broken_pipe
typedef DWORD thread_id;
#define threadid GetCurrentThreadId
#else // NOT USE_PIPE_CLIENT
typedef pthread_t thread_id;
#define threadid pthread_self
#endif // NOT USE_PIPE_CLIENT

namespace bzs
{
namespace netsvc
{
namespace client
{

typedef std::vector<boost::asio::const_buffer> buffers;

class connection;

class idirectReadHandler
{

public:
    /* Call by connection at first 4byte readed. 
       size : The rest of the data size.  = TotalSize - 4  
    */
    virtual unsigned int onRead(unsigned int size, connection* c) = 0;
};

/** The connection interface
 *
 */
class connection
{
public:
    virtual ~connection(){};
    virtual void connect() = 0;
    virtual void addref() = 0;
    virtual void release() = 0;
    virtual int refCount() const = 0;
    virtual bool isConnected() const = 0;
    virtual const boost::asio::ip::tcp::endpoint& endpoint() const = 0;
    virtual thread_id tid() const = 0;
    virtual char* sendBuffer(size_t size) = 0;
    virtual unsigned int sendBufferSize() = 0;
    virtual buffers* optionalBuffers() = 0;
    virtual char* asyncWriteRead(unsigned int writeSize) = 0;
    //virtual unsigned int datalen() const = 0; // additinal info at segment read
    //virtual unsigned short rows() const = 0; // additinal info at segment read
    virtual void setReadBufferSizeIf(size_t size) = 0;
    virtual void setDirectReadHandler(idirectReadHandler* p) = 0;
    virtual unsigned int directRead(void* buf, unsigned int size) = 0;
    virtual void* directReadRemain(unsigned int size) = 0;
    virtual bool queryFunction(unsigned int v) = 0;
};

#define CONNECTION_FUNCTION_DIRECT_READ 1


} // namespace client
} // namespace netsvc
} // namespace bzs

#endif // BZS_NETSVC_CLINET_ICONNECTION_H
