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
#ifndef BZS_NETSVC_SERVER_SERVERPIPE_H
#define BZS_NETSVC_SERVER_SERVERPIPE_H

#include "iserver.h"
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <exception>


typedef HANDLE platform_descriptor;
typedef boost::asio::windows::stream_handle platform_stream;


namespace boost{class condition_variable;}

namespace bzs 
{
namespace netsvc 
{
namespace server 
{
class IAppModuleBuilder;
extern ::boost::condition_variable condition;

namespace pipe
{
extern unsigned int g_connections;
extern unsigned int g_waitThread;


class acceptor : private boost::noncopyable
{
	platform_descriptor m_fd;
	std::string m_pipeName;
	bool m_cancel;
	
public:
	acceptor();
	void open(const std::string& pipeName);
	void cancel();
	void accept(platform_stream& pipe);
	std::string& name(){return m_pipeName;};
};

class connection;

class server :public iserver, private boost::noncopyable
{
	boost::shared_ptr<IAppModuleBuilder> m_app;	
	boost::asio::io_service m_ios;
	acceptor m_acceptor;
	boost::shared_ptr<connection> m_newConnection;
	const std::size_t m_maxConnections;

	bool m_stopped;
	void doStop();
	void doClose();
	void run();
	void onAccept(const boost::system::error_code& e);
public:
	server( boost::shared_ptr<IAppModuleBuilder>,  const std::string& name
			, std::size_t max_connections, unsigned int shareMemSize, const char* hostCheckName);
	void start();
	void stop();
	void registerErrorHandler(inotifyHandler* eh){erh = eh;}; 
	static inotifyHandler* erh;
 
};

}//namespace pipe
}//namespace server
}//namespace netsvc
}//namespace bzs
#endif //BZS_NETSVC_SERVER_SERVERPIPE_H

