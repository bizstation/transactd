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
#include "serverCpt.h"
#include "IAppModule.h"
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
#include <bzs/rtl/exception.h>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;



namespace bzs
{
namespace netsvc
{
namespace server
{
namespace cpt //connection per thread
{

unsigned int g_connections = 0;
unsigned int g_waitThread = 0;
// ---------------------------------------------------------------------------
//		connection
// ---------------------------------------------------------------------------
#define READBUF_SIZE 66000
#define WRITEBUF_SIZE 66000

class connection  : public iconnection, private boost::noncopyable 			
{
	mutable io_service m_ios;

	static mutex m_mutex;
	tcp::socket m_socket;
	std::vector<char> m_buffer;
	std::vector<char> m_result;
	buffers m_optionalBuffes;
	shared_ptr<IAppModule> m_module;
	bool m_segmentWrite;
	
	size_t m_readLen;
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
				if (m_optionalBuffes.size())
					m_optionalBuffes.clear();
				if (m_module->execute(vecBuffer(m_result), size, &m_optionalBuffes) == EXECUTE_RESULT_QUIT)
					return ;
				else
				{
					m_readLen = 0;
					DEBUG_PROFILE_START(1)
					{
						if (m_optionalBuffes.size())
						{
							m_optionalBuffes.insert(m_optionalBuffes.begin(), buffer(&m_result[0], size));
							async_write(m_socket, m_optionalBuffes, boost::asio::transfer_all(), boost::bind(&connection::handle_write, this,
								boost::asio::placeholders::error));
						}else
						{
							async_write(m_socket, buffer(&m_result[0], size), boost::asio::transfer_all(), boost::bind(&connection::handle_write, this,
								boost::asio::placeholders::error));
						
						}
					}
					m_module->cleanup();
					return;
				}
			}
			
			if (n > m_buffer.size())
				m_buffer.resize(n);
			async_read(m_socket, buffer(&m_buffer[m_readLen], n - m_readLen)
				,boost::asio::transfer_all()
				,boost::bind(&connection::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
	}
	
	void handle_write(const boost::system::error_code& e)
	{
		if (!e)
		{
			if (m_segmentWrite == false)
			{
				DEBUG_PROFILE_END(1, "write")
				async_read(m_socket, buffer(&m_buffer[0], 4)
					,boost::asio::transfer_all()
					,boost::bind(&connection::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
		}
				
	}
public:
	
	connection(): m_socket(m_ios)
		,m_readLen(0),m_segmentWrite(false)
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

	void asyncWrite(const char* p, size_t size)
	{
		boost::asio::write(m_socket, buffer(p, size),  boost::asio::transfer_all());
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
	
	io_service& ios()const{return m_ios;};
	
	tcp::socket& socket(){return m_socket;}
	
	void setModule(boost::shared_ptr<IAppModule> p)
	{
		m_module = p;
	}
	
	void sendConnectAccept()
	{
		m_ios.reset();
		m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
		m_socket.set_option(boost::asio::socket_base::receive_buffer_size(512*1024));
		m_socket.set_option(boost::asio::socket_base::send_buffer_size (1024*1024*10));

		size_t n = m_module->onAccept(&m_result[0], WRITEBUF_SIZE);
		if (n)
			boost::asio::write(m_socket, buffer(&m_result[0], n), boost::asio::transfer_all());
	}
	
	void start()
	{
		m_socket.async_read_some(buffer(&m_buffer[0], m_buffer.size()),
					boost::bind(&connection::handle_read, this,
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		m_ios.run();
	}
	
	static std::vector<connection* > connections;
	
	static void stop()
	{
		mutex::scoped_lock lck(m_mutex);
		try
		{
			for (size_t i=0;i<connections.size();i++)
				connections[i]->ios().stop();
		}
		catch(boost::system::system_error &){};
	}
	
};
std::vector<connection* > connection::connections;
mutex connection::m_mutex;

// ---------------------------------------------------------------------------
//		worker
// ---------------------------------------------------------------------------

class worker : private boost::noncopyable 
{

	static boost::mutex m_mutex;	
	static std::vector< shared_ptr<boost::thread> > m_threads;
	static std::vector<worker*> m_workers; 
	static worker* findWaitThread()
	{
		mutex::scoped_lock lck(m_mutex);
		for (size_t i=0;i<m_workers.size();i++)
			if (m_workers[i]->m_connection==NULL)
				return m_workers[i];
		return NULL;
	}
	
	boost::shared_ptr<connection>  m_connection;
	
	bool resume(){return shutdown || m_connection;}
	
	~worker()
	{
		mutex::scoped_lock lck(m_mutex);
		m_workers.erase( find(m_workers.begin(), m_workers.end(), this));
	}
	
public:
	static bool shutdown;
	static const char* hostCheckName;
	static boost::condition_variable condition;
	
	static void registThread(shared_ptr<boost::thread> t)
	{
		mutex::scoped_lock lck(m_mutex);
		m_threads.push_back(t);
	}

	static void join()
	{
		for (size_t i=0;i<m_threads.size();i++)
			m_threads[i]->join();
		//delete thread
		m_threads.erase(m_threads.begin(), m_threads.end());
	}

	static worker* get(const IAppModuleBuilder* app)
	{
		worker* p = findWaitThread();
		if (p==NULL)
		{
			boost::thread::attributes attr;
			attr.set_stack_size( 125 * 1024); 
			p = new worker();
			shared_ptr<boost::thread> t(new boost::thread(attr, bind(&worker::run, p, app)));
			registThread(t);
		}
		return p;
	}

	worker()
	{
		mutex::scoped_lock lck(m_mutex);
		m_workers.push_back(this);
	}

	void setConnection(boost::shared_ptr<connection> conn)
	{
		m_connection = conn;
	}

	void run(const IAppModuleBuilder*  app)
	{
		try
		{
			while (!shutdown)
			{
				if (m_connection)
				{
					boost::system::error_code ec;
					tcp::endpoint endpoint = m_connection->socket().remote_endpoint(ec);
					boost::shared_ptr<IAppModule> mod(((IAppModuleBuilder*)app)->createSessionModule(endpoint, m_connection.get(), SERVER_TYPE_CPT));
					m_connection->setModule(mod);
					if (mod->checkHost(hostCheckName))
					{
						m_connection->sendConnectAccept();
						m_connection->start(); //It does not return, unless a connection is close. 
					}
					m_connection.reset();
				}
				//TODO A used thread -- it is all held. 
				//The number of maintenance is decided and it is made not to hold any more.
				mutex::scoped_lock lck(m_mutex); 
				++g_waitThread;
				condition.wait(lck, bind(&worker::resume, this));
				--g_waitThread;
			}
		}
		catch(bzs::rtl::exception &e)
		{
			if (server::erh)
			{
				if(const std::string *msg = getMsg(e))
				{
					std::string s = "Cpt server " + *msg;
					server::erh->printError(s.c_str());
				}else
					server::erh->printError(boost::diagnostic_information(e).c_str());
			}
		}
		catch(...)
		{
			if (server::erh)
				server::erh->printError("Cpt server Unknown");
		}
		//An end of this thread will delete self.
		delete this;
	}

};

bool worker::shutdown = false;
const char* worker::hostCheckName;
boost::condition_variable worker::condition;
boost::mutex worker::m_mutex;	
std::vector< boost::shared_ptr<boost::thread> > worker::m_threads;
std::vector<worker*>  worker::m_workers;

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
			:m_srv(srv), m_app(app),m_acceptor(srv->ios())
	{

		m_newConnection.reset(new connection());
		tcp::resolver resolver(m_newConnection->ios());
		tcp::resolver::query query(address, port, resolver_query_base::numeric_service);
		tcp::endpoint endpoint = *resolver.resolve(query);
	
		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(tcp::acceptor::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen((int)m_srv->maxConnections());
	}
	
	void startAsyncAccept()
	{
		if (m_srv->checkConnections())
			m_acceptor.async_accept(m_newConnection->socket(), boost::bind(&listener::onAccept, this, placeholders::error));
	}

	void onAccept(const boost::system::error_code& e)
	{
		//connection is passed to a thread and it resumes.
		if (!e)
		{
			worker* w = worker::get(m_app.get());
			w->setConnection(m_newConnection);
			worker::condition.notify_all();
			m_newConnection.reset(new connection());
			startAsyncAccept();
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
inotifyHandler* server::erh=NULL;

/** server
 *	If it starts, a server will create the exclusive thread for accpter 
 *	and will go into an infinite loop. 
 */
server::server(std::size_t max_connections, const char* hostCheckName)
		: m_maxConnections(max_connections)	,m_timer(m_ios),m_stopped(false)
{
	worker::hostCheckName = hostCheckName;
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

/** Start the server
 */
void server::start()
{
	shared_ptr<boost::thread> t(new boost::thread(bind(&server::run, this)));
	worker::registThread(t);
}

bool server::checkConnections()
{
	while (connection::connections.size() > m_maxConnections)
	{
		Sleep(100*MCRTOMM);
		if (m_stopped)
			return false;
	}
	return true;
}

void server::startAsyncAccept()
{
	for (size_t i = 0;i < m_listeners.size();i++)
		m_listeners[i]->startAsyncAccept();
}

/** Strat the time.
 *  If the timer times out then call shoutdown ceck function
 */
void server::startTimer()
{
	m_timer.expires_from_now(posix_time::seconds(10));
	m_timer.async_wait(bind(&server::onCheckInternlShutdown, this, placeholders::error));

}

void server::run()
{
	if (erh)
		erh->printInfo("Start Cpt server.");
	
	startAsyncAccept();
	startTimer();
	m_ios.run();
}

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
		doStop();
	else
		startTimer();
}

void server::doStop()
{
	if (!m_stopped)
	{
		m_stopped = true;
		if (erh)
			erh->printInfo("Stopping Cpt server ...");

		for (size_t i = 0;i < m_listeners.size();i++)
			m_listeners[i]->cancel();
		worker::shutdown = true;
		connection::stop();
		m_ios.stop();
		worker::condition.notify_all();
	}
}

void server::stop()
{
	doStop();
	worker::join();

}


}//namespace cpt
}//namespace server
}//namesapce netsvc
}//namespace bzs
