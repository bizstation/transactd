#ifndef BZS_NETSVC_SERVER_SERVERCPT_H
#define BZS_NETSVC_SERVER_SERVERCPT_H
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
#include "iserver.h"
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

namespace boost
{
class condition_variable;
}
namespace bzs
{
namespace netsvc
{
namespace server
{

class IAppModuleBuilder;
extern boost::condition_variable condition;

namespace cpt
{

extern unsigned int g_connections;
extern unsigned int g_waitThread;

class connection;
class listener;

/** connection per thread server
 */
class server : public iserver, private boost::noncopyable
{
    std::vector<boost::shared_ptr<listener> > m_listeners;
    boost::asio::io_service m_ios;
    boost::asio::deadline_timer m_timer;
    const std::size_t m_maxConnections;
    bool m_stopped;
    void startTimer();
    void startAsyncAccept();
    void doStop();
    void doClose();
    void run();
    void onCheckInternlShutdown(const boost::system::error_code& e);

public:
    server(const size_t max_connections, const char* hostCheckName);
    void addApplication(boost::shared_ptr<IAppModuleBuilder>,
                        const std::string& address, const std::string& port);
    ~server();
    void start();
    void stop();
    void registerErrorHandler(inotifyHandler* eh) { erh = eh; };
    boost::asio::io_service& ios() { return m_ios; }
    const std::size_t maxConnections() const { return m_maxConnections; };
    bool checkConnections();
    static inotifyHandler* erh;
};

} // namespace cpt
} // namespace sever
} // namespace netsvc
} // namespace bzs

#endif // BZS_NETSVC_SERVER_SERVERCPT_H
