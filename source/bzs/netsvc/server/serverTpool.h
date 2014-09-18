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
#ifndef BZS_NETSVC_SERVER_TPOOLSERVER_H
#define BZS_NETSVC_SERVER_TPOOLSERVER_H

#include "iserver.h"
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>



#define SERVER_SYNC_THREAD_PER_IOS

namespace boost{class condition_variable;}
namespace bzs 
{
namespace netsvc 
{
namespace server 
{
class IAppModuleBuilder;


namespace tpool
{
extern unsigned int g_connections;


class connection;
class listener;
class server: public iserver, private boost::noncopyable
{
	std::vector<boost::shared_ptr<listener> > m_listeners;
	boost::asio::io_service m_ios;
	boost::asio::deadline_timer m_timer;
	const char* m_hostCheckName;
	bool m_stopping;
	void startTimer();
	void doClose();
	void onCheckInternlShutdown(const boost::system::error_code& e);
	void run();
public:
	explicit server(unsigned int thread_pool_size, const char* hostCheckName);
	~server();
	void start();
	void stop();
	static unsigned int m_threadPoolSize;
	void registerErrorHandler(inotifyHandler* eh){erh = eh;}; 
	void addApplication(boost::shared_ptr<IAppModuleBuilder> app
				, const std::string& address, const std::string& port);
	boost::asio::io_service& ios(){return m_ios;}
	const char* hostCheckName(){return m_hostCheckName;};
	static inotifyHandler* erh;
	
};

}//namesapce tpool
}//namespace sever
}//namespace netsvc
}//namespace bzs

#endif //BZS_NETSVC_SERVER_TPOOLSERVER_H

