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
#include "serverTpool.h"
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/asio/write.hpp> 
#include <boost/thread/xtime.hpp>
#include <algorithm>
#include <boost/enable_shared_from_this.hpp>

#include <bzs/rtl/debuglog.h>
#include <bzs/env/crosscompile.h>
#include "IAppModule.h"

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;



#define READBUF_SIZE 66000
#define WRITEBUF_SIZE 66000

namespace bzs 
{
namespace netsvc 
{
namespace server 
{
namespace tpool
{

unsigned int g_connections = 0;

// ---------------------------------------------------------------------------
//		connection
// ---------------------------------------------------------------------------

/** Asynchronous thread pool server
 *  In transaction then use one thread. that like connection per thread.
 * 
 */
class connection : public boost::enable_shared_from_this<connection>,
		public iconnection, private boost::noncopyable
{
	static mutex m_mutex;
	io_service& m_ios;
	boost::asio::ip::tcp::socket m_socket;
	shared_ptr<IAppModule> m_module;
	vector_buffer m_buffer;
	vector_buffer m_result;
	buffers m_optionalBuffes;
		
	size_t m_readLen;
	char* m_statck;

	bool syncReadWrite()
	{
		bool complete = false;
		boost::system::error_code e;
		m_readLen = 0;
		while (1)
		{
			m_readLen += boost::asio::read(m_socket, boost::asio::buffer(&m_buffer[m_readLen], m_buffer.size()-m_readLen)
				,boost::asio::transfer_at_least(4), e);
			if (e) return false;
			
			size_t n = m_module->onRead((const char*)&m_buffer[0], m_readLen, complete);
			if (!complete)
			{
				if (n > m_buffer.size()) m_buffer.resize(n);
				m_readLen += boost::asio::read(m_socket, boost::asio::buffer(&m_buffer[m_readLen], n-m_readLen)
					, boost::asio::transfer_all(), e);
				if (e) return false;
				n = m_module->onRead((const char*)&m_buffer[0], m_readLen, complete);
			}
			if (complete)
			{
				m_readLen = 0;
				size_t size=0;
				m_optionalBuffes.clear();
				vecBuffer vbuf(m_result);
				bzs::netsvc::server::IResultBuffer& buf = vbuf;
				int ret = m_module->execute(buf, size, &m_optionalBuffes);
				
				if (ret == EXECUTE_RESULT_QUIT)
					return false;
				m_optionalBuffes.insert(m_optionalBuffes.begin(), buffer(&m_result[0], size));
				if (ret == EXECUTE_RESULT_FORCSE_ASYNC)
					return true;
				else
				{ 
					m_readLen = 0;
					DEBUG_PROFILE_START(1)
					
					boost::asio::write(m_socket, m_optionalBuffes, boost::asio::transfer_all(), e);
					if (e) return false;
				}
			}
		}
	}
	
	void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred)
	{
		DEBUG_PROFILE_START(m_readLen==0)
		if (!e)
		{
			bool complete = false;
			
			if (bytes_transferred==0)
				return ;
			m_readLen += bytes_transferred;
			size_t n = m_module->onRead(&m_buffer[0], m_readLen, complete);
			if (complete)
			{
				DEBUG_PROFILE_END(1, "handle_read")
				size_t size=0;
				m_optionalBuffes.clear();
				vecBuffer vbuf(m_result);
				bzs::netsvc::server::IResultBuffer& buf = vbuf;
				int ret = m_module->execute(buf, size, &m_optionalBuffes);
				
				if (ret == EXECUTE_RESULT_QUIT)
					return ;
				
				m_optionalBuffes.insert(m_optionalBuffes.begin(), buffer(&m_result[0], size));
				if (ret == EXECUTE_RESULT_FORCSE_SYNC)
				{
					boost::system::error_code ec;
					boost::asio::write(m_socket, m_optionalBuffes, boost::asio::transfer_all(), ec);
					if (ec) return ;
					syncReadWrite();
				}
				DEBUG_PROFILE_START(1)
				async_write(m_socket, m_optionalBuffes, boost::bind(&connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
					
				return;
			}
			
			if (n > m_buffer.size())
				m_buffer.resize(n);
			m_socket.async_read_some(buffer(&m_buffer[m_readLen], m_buffer.size()-m_readLen),
					boost::bind(&connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_write(const boost::system::error_code& e)
	{
		if (!e)
		{
			DEBUG_PROFILE_END(1, "write")
			if (m_module->isShutDown())
				return;
			start();
		}
	}

	
public:
	explicit connection(io_service& ios):m_socket(ios),m_ios(ios)
	{
		m_buffer.resize(READBUF_SIZE);
		m_result.resize(WRITEBUF_SIZE);

		mutex::scoped_lock lck(m_mutex);
		connections.push_back(this);
		++g_connections;

		
	}
	
	~connection()
	{
		if (connections.size())
		{
			mutex::scoped_lock lck(m_mutex);
			std::vector<connection*>::iterator it = find(connections.begin(), connections.end(), this);
			if (it != connections.end())
			{
				connections.erase( it);
				--g_connections;
			}
		}
	}
	
	boost::asio::ip::tcp::socket& socket(){return m_socket;}
	
	void asyncWrite(const char* p, unsigned int size)
	{
		boost::asio::write(m_socket, buffer(p, size),  boost::asio::transfer_all());
	}

	void sendConnectAccept()
	{
		size_t n = m_module->onAccept(&m_result[0], WRITEBUF_SIZE);
		if (n)
			boost::asio::write(m_socket, buffer(&m_result[0], n), boost::asio::transfer_all());
	}
	
	void start()
	{
		m_readLen = 0;
		m_socket.async_read_some(buffer(&m_buffer[0], m_buffer.size()),
			bind(&connection::handle_read, shared_from_this(), placeholders::error,
			placeholders::bytes_transferred));
	}
	
	void setModule(boost::shared_ptr<IAppModule> p)
	{
		m_module = p;
	}
	
	void close()
	{
		m_ios.post(boost::bind(&connection::doClose, this));
	}
	
	void doClose()
	{
		boost::system::error_code ec;
		m_socket.close(ec);
	}
	
	static std::vector<connection* > connections;
	
	static void stop()
	{
		mutex::scoped_lock lck(m_mutex);
		boost::system::error_code  ec;
		for (size_t i=0;i<connections.size();i++)
			connections[i]->socket().close(ec);
	}

};

std::vector<connection* > connection::connections;
mutex connection::m_mutex;

// ---------------------------------------------------------------------------
//		listener
// ---------------------------------------------------------------------------

class listener
{
	boost::shared_ptr<IAppModuleBuilder> m_app;	
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::shared_ptr<connection> m_newConnection;
	server* m_srv;
public:
	listener(server* srv, 
			shared_ptr<IAppModuleBuilder> app, const std::string& address, const std::string& port)
			:m_srv(srv), m_app(app),m_acceptor(srv->ios()),m_newConnection(new connection(srv->ios()))
	{
		tcp::resolver resolver(srv->ios());
		tcp::resolver::query query(address, port, resolver_query_base::numeric_service);
		tcp::endpoint endpoint = *resolver.resolve(query);

		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(tcp::acceptor::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen();
		m_acceptor.async_accept(m_newConnection->socket()
			, bind(&listener::handle_accept, this, placeholders::error));
	}
	
	void handle_accept(const boost::system::error_code& e)
	{
		if (!e)
		{
			const boost::asio::ip::tcp::no_delay nodelay(true);
			m_newConnection->socket().set_option(nodelay);
			boost::system::error_code ec;
			tcp::endpoint endpoint = m_newConnection->socket().remote_endpoint(ec);
			boost::shared_ptr<IAppModule> mod(m_app->createSessionModule(endpoint,m_newConnection.get(), SERVER_TYPE_TPOOL));
			m_newConnection->setModule(mod);
			if (mod->checkHost(m_srv->hostCheckName()))
			{
				m_newConnection->sendConnectAccept();
				m_newConnection->start();
			}
			m_newConnection.reset(new connection(m_srv->ios()));
			m_acceptor.async_accept(m_newConnection->socket(), boost::bind(&listener::handle_accept, this,
				boost::asio::placeholders::error));
		}
	}
	
	void cancel()
	{
		boost::system::error_code  ec;
		m_acceptor.cancel(ec);
	}
};


// ---------------------------------------------------------------------------
//		server
// ---------------------------------------------------------------------------

static boost::shared_ptr<boost::thread> serverThread;
unsigned int server::m_threadPoolSize;
inotifyHandler* server::erh=NULL;
server::server(unsigned int thread_pool_size, const char* hostCheckName)
	:m_timer(m_ios),m_hostCheckName(hostCheckName),m_stopping(0)
{
	m_threadPoolSize = thread_pool_size;
	
}

server::~server()
{
	m_listeners.clear();
}

void server::addApplication(boost::shared_ptr<IAppModuleBuilder> app, const std::string& address
		, const std::string& port)
{
	m_listeners.push_back(boost::shared_ptr<listener>(new listener(this, app, address, port)));
}

void server::startTimer()
{
	m_timer.expires_from_now(posix_time::seconds(10));
	m_timer.async_wait(bind(&server::onCheckInternlShutdown, this, placeholders::error));

}

/* check shutdown flags
 * call start timer agin if it is not shutdown.
 */
void server::onCheckInternlShutdown(const boost::system::error_code& e)
{
	bool shutdown = false;
	{
		mutex::scoped_lock lck(modulesMutex);
		for (size_t i=0;i<modules.size();i++)
		{
			IAppModule* mod = modules[i];
			if (mod->isShutDown())
			{
				shutdown = true;
				break;
			}
		}	
	}
	if (shutdown)
		stop();
	else
		startTimer();
}

void start1(io_service* p)
{
	try
	{
		p->run();
	}
	catch(...){};
}

/** crete server thread and return immediately. 
 */
void server::start()
{
	serverThread.reset(new boost::thread(bind(&server::run, this)));	
}

void server::run()
{
	if (erh)
			erh->printInfo("Start Tpool server.");
	startTimer();
	std::vector<boost::shared_ptr<boost:: thread> >threads;
	thread::attributes attr;
	attr.set_stack_size( 125 *  1024);
	for (std::size_t i = 0; i < m_threadPoolSize; ++i)
	{
		thread *t = new thread(attr, bind(&start1, &m_ios));
		shared_ptr<thread> thread(t);
		threads.push_back(thread);
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	
}

void server::stop()
{
	if (m_stopping) return;
	m_stopping = true;
	if (erh)
		erh->printInfo("Stopping Tpool server ...");
	for (size_t i = 0;i < m_listeners.size();i++)
		m_listeners[i]->cancel();
	connection::stop(); // Connection is not closed if io_service is stopped.
	m_ios.stop();
	serverThread->join();
	m_ios.reset();

}

}//namespace tpool
}//namespace sever
}//namespace netsvc
}//namespace bzs
