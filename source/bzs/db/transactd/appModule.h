#ifndef BZS_DB_TRANSACTD_APPMODULE_H
#define BZS_DB_TRANSACTD_APPMODULE_H
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
#include <my_config.h>
#include <bzs/netsvc/server/IAppModule.h>
#include <bzs/db/protocol/ICommandExecuter.h>

class THD;

namespace bzs
{
namespace db
{
namespace transactd
{

/** Feature
 *  -Client authentication.
 *  -Check tcp recieve of end of data.
 *  -Tcp connection disconnect function.
 *  -Execute client command.
 *  -Anser of shutdown.
 */
class module : public netsvc::server::IAppModule, private boost::noncopyable
{
    friend class connManager;
    mutable boost::mutex m_mutex;
    boost::shared_ptr<protocol::ICommandExecuter> m_commandExecuter;
    const boost::asio::ip::tcp::endpoint m_endpoint;
    bzs::netsvc::server::iconnection* m_connection;
    const char* m_readBuf;
    size_t m_readSize;
    netsvc::server::netWriter* m_nw;
    bool m_useThreadPool;
    bool perseLineEnd(const char* p, size_t size) const;
    size_t onRead(const char* data, size_t size, bool& complete);
    size_t onAccept(char* message, size_t bufsize);
public:
    module(const boost::asio::ip::tcp::endpoint& endpoint,
           bzs::netsvc::server::iconnection* connection, bool tpool, int type);
    ~module();
    void reset();
    int execute(netsvc::server::IResultBuffer& result, size_t& size,
                netsvc::server::buffers* optionalData);
    void cleanup() { m_commandExecuter->cleanup(); };
    bool isShutDown() { return m_commandExecuter->isShutDown(); }
    bool checkHost(const char* hostCheckname, /*out*/char* hostName, int size);
    void disconnect();
    boost::mutex& mutex() const { return m_mutex; };
};

} // namespace transactd
} // namespace db
} // namespace bzs

#endif // BZS_DB_TRANSACTD_APPMODULE_H
