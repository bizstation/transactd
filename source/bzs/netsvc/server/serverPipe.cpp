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

#include "serverPipe.h"
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/asio/write.hpp> 
#include <boost/thread/xtime.hpp>
#include <algorithm>
#include <boost/enable_shared_from_this.hpp>
#include "IAppModule.h"
#include <bzs/rtl/exception.h>



using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;



char* getWindowsErrMsg( DWORD ErrorCode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPSTR) &lpMsgBuf,	0, NULL);
	return (char*)lpMsgBuf;
}	

#define PIPE_EOF_ERROR_CODE boost::system::windows_error::broken_pipe
#define BUFSIZE 0


namespace bzs 
{
namespace netsvc 
{
namespace server 
{
namespace pipe
{

void getSecurityAttribute(SECURITY_ATTRIBUTES& sa, SECURITY_DESCRIPTOR& sd)
{
	
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd,TRUE, NULL,FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
}

acceptor::acceptor():m_fd(0),m_cancel(false)
{

}

void acceptor::open(const std::string& pipeName)
{
	m_pipeName = pipeName;	
}

void acceptor::cancel()
{
	m_cancel = true;	
}

void acceptor::accept(platform_stream& pipe)
{
	m_fd=NULL;
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	getSecurityAttribute(sa, sd);
	
	char pipeName[100];
	sprintf_s(pipeName, 100, "\\\\.\\pipe\\%s", m_pipeName.c_str());
	m_fd = CreateNamedPipe( pipeName, // pipe name
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			,PIPE_TYPE_BYTE |PIPE_READMODE_BYTE |PIPE_WAIT
			,PIPE_UNLIMITED_INSTANCES                   // max. instances
			,BUFSIZE									// output buffer size
			,BUFSIZE                                    // input buffer size 
			,0											// client time-out
			,&sa);                                     // default security attribute
	if (m_fd == INVALID_HANDLE_VALUE)
		THROW_BZS_ERROR_WITH_MSG(getWindowsErrMsg(GetLastError()));
	OVERLAPPED overlapped =	{0};
	overlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);
	BOOL ret = ConnectNamedPipe(m_fd, &overlapped);
	
	//Connection may be completed by timing at this time.
	if ((ret==FALSE) && (GetLastError() == ERROR_PIPE_CONNECTED))
	{  
		CloseHandle(overlapped.hEvent);
		pipe.assign(m_fd);
		return;
	}
	if ((ret != FALSE) || (GetLastError() != ERROR_IO_PENDING))
	{
		CloseHandle(overlapped.hEvent);
		CloseHandle(m_fd);
		THROW_BZS_ERROR_WITH_MSG("ConnectNamedPipe error");
		return ;
	}
	
	//A shutdown is supervised periodically.
	DWORD waitRes;
	while (1)
	{
		if (m_cancel)
		{
			CloseHandle(overlapped.hEvent);
			CloseHandle(m_fd);
			break;	
		}
		waitRes = WaitForSingleObject(overlapped.hEvent, 5);
		if (waitRes == WAIT_OBJECT_0)
		{
			CloseHandle(overlapped.hEvent);
			pipe.assign(m_fd);
			break;
		}else if (waitRes == WAIT_TIMEOUT)
			;
		else
		{
			CloseHandle(overlapped.hEvent);
			CloseHandle(m_fd);
			THROW_BZS_ERROR_WITH_MSG("WaitForSingleObject error");
		}
	}
}

unsigned int g_connections=0;
unsigned int g_waitThread=0;


// ---------------------------------------------------------------------------
//		connection
// ---------------------------------------------------------------------------



class IExitCheckHandler
{
public:
	virtual ~IExitCheckHandler(){};
	virtual bool isExit()=0;
};


class exitCheckHnadler : public IExitCheckHandler
{
	HANDLE m_thread;
	bool m_cancel;
	IAppModule* m_module;
public:

	exitCheckHnadler(DWORD tid):m_cancel(false),m_module(NULL)
	{
		m_thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION /*THREAD_QUERY_INFORMATION*/, FALSE, tid);
		/*if (!m_thread)
		{
			char buf[1024];
			wsprintf(buf, "OpenThread error :id = %d %s", tid, getWindowsErrMsg(GetLastError()));
			THROW_BZS_ERROR_WITH_MSG(buf);
		}*/
	}

	~exitCheckHnadler()
	{
		if (m_thread)
			CloseHandle(m_thread);
	}

	void setModule(IAppModule* p)
	{
		m_module = p;
	}

	bool isExit()
	{
		if (m_cancel)return true;
		DWORD ExitCode;
		if (m_thread && GetExitCodeThread(m_thread, &ExitCode))
			if (STILL_ACTIVE!=ExitCode) return true;
		if (m_module && m_module->isShutDown())
			return true;
		return false;
	}

	void setCancel(bool value){m_cancel=value;};
	bool cancel(){return m_cancel;}
};

class winEventComm
{
	HANDLE m_recv;
	HANDLE m_send;
	bool m_sent;
	bool m_cancel;
public:

	winEventComm(const char* rcvName, const char* sndName):m_cancel(false)
	{
		SECURITY_DESCRIPTOR sd;
		SECURITY_ATTRIBUTES sa;
		getSecurityAttribute(sa, sd);
		m_recv = CreateEvent(&sa, FALSE, FALSE, rcvName);
		if (!m_recv)
			THROW_BZS_ERROR_WITH_MSG(getWindowsErrMsg(GetLastError()));
		m_send = CreateEvent(&sa, FALSE, FALSE, sndName);
		if (!m_send)
			THROW_BZS_ERROR_WITH_MSG(getWindowsErrMsg(GetLastError()));
	}

	~winEventComm()
	{
		if (!m_sent)SetEvent(m_send);
		if (m_recv)CloseHandle(m_recv);
		if (m_send)CloseHandle(m_send);
	}

	bool recv(int checkTimeSpan, IExitCheckHandler* handler)
	{
		DWORD wait;
		do{
			if (m_cancel || (handler && (handler->isExit())))
				return 0;
		}while(WAIT_OBJECT_0 != (wait = WaitForSingleObject(m_recv, checkTimeSpan))); 
		m_sent = false;
		return true;
	}

	bool send()
	{
		if (SetEvent(m_send)==0)
			return false;

		m_sent = true;
		return true;
	}

	void chancel(){m_cancel = true;}
};

class sharedMem
{
	HANDLE m_mapFile;
	
	char* m_readPtr;
	char* m_writePtr;
	DWORD m_size;
public:

	sharedMem(const char* name, unsigned int memsize)
	{
		SECURITY_DESCRIPTOR sd;
		SECURITY_ATTRIBUTES sa;
		getSecurityAttribute(sa, sd);
		
		SYSTEM_INFO SystemInfo;
		GetSystemInfo(&SystemInfo);
		m_size = memsize/SystemInfo.dwAllocationGranularity + 1;
		m_size =  m_size*SystemInfo.dwAllocationGranularity;


		m_mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, m_size*2, name);
		if (m_mapFile == NULL)
			THROW_BZS_ERROR_WITH_MSG("CreateFileMapping error");

		m_readPtr = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_size);
		if (m_readPtr == NULL)
			THROW_BZS_ERROR_WITH_MSG("MapViewOfFile R error");
		m_writePtr = (char*)MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, m_size, m_size);
		if (m_writePtr == NULL)
			THROW_BZS_ERROR_WITH_MSG("MapViewOfFile W error");
	}

	~sharedMem()
	{
		if (m_mapFile)
		{
			UnmapViewOfFile(m_readPtr) ;
			UnmapViewOfFile(m_writePtr) ;
			CloseHandle(m_mapFile);
		}
	}
	size_t size() const {return (size_t)m_size;}

	char* readBuffer()
	{
		return m_readPtr;
	}

	char* writeBuffer()
	{
		return m_writePtr;
	}
};

class sharedMemBuffer : public IResultBuffer
{
	sharedMem& m_buf;
public:
	sharedMemBuffer(sharedMem& v):m_buf(v){}
	void resize(size_t v){}	
	size_t  size()const{return m_buf.size();}
	char* ptr(){return m_buf.writeBuffer();}
};

class connection  : public iconnection, private noncopyable 			
{
	mutable io_service m_ios;
	static mutex m_mutex;
	platform_stream m_socket;
	
	shared_ptr<winEventComm> m_comm;
	shared_ptr<exitCheckHnadler> m_exitHandler;
	shared_ptr<sharedMem> m_sharedMem;
	shared_ptr<IAppModule> m_module;
	size_t m_readLen;
	
	void run()
	{
		bool sentResult = true;
		while (sentResult)
		{
			if (m_comm->recv(3000, m_exitHandler.get())==false)
				return; 
			bool complete = false;
			m_readLen = *((unsigned int*)m_sharedMem->readBuffer());
			if (m_readLen==0)
				return; 
			m_module->onRead(m_sharedMem->readBuffer(), m_readLen, complete);
			if (complete)
			{
				size_t size=0;
				if (m_module->execute(sharedMemBuffer(*m_sharedMem), size, NULL) == EXECUTE_RESULT_QUIT)
					return ;
				else
					m_readLen = 0;

				sentResult = m_comm->send();
				m_module->cleanup();
			}
		}
	}
	
	char* getUniqName(DWORD id,__int64 clientid, const char* name, char* buf, int size)
	{
		sprintf_s(buf, size, "%s_%lu_%Lu", name, id, clientid);
		return buf;
	}
	
public:
	static std::string m_pipeName;
	static unsigned int m_shareMemSize;
	connection(): m_socket(m_ios),m_readLen(0)
	{
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
		m_exitHandler.reset();
	}
	
	void close()
	{
		m_comm->chancel();
	}

	void asyncWrite(const char* p, size_t size)
	{
		
	}

	io_service& ios()const{return m_ios;};
	
	platform_stream& socket(){return m_socket;}
	
	void setModule(shared_ptr<IAppModule> p)
	{
		m_module = p;
		if (m_exitHandler)
			m_exitHandler->setModule(p.get());
	}
	
	void start()
	{
		m_ios.reset();
		
		char buf[128];
		char buf2[50];
		char tmp[50];
		char tmp2[50];
		system::error_code e;
		std::size_t len = asio::read(m_socket, buffer(buf, 16), e);
		if (len!=16)
			THROW_BZS_ERROR_WITH_MSG("readThredID error");
		
		DWORD clinetThreadID= *((DWORD*)(buf+4));
		__int64 clientid = *((__int64*)(buf+8));
		sprintf_s(tmp, 50, "Global\\%s", m_pipeName.c_str());
		m_sharedMem.reset(new sharedMem(getUniqName(clinetThreadID, clientid, tmp , buf, 128), m_shareMemSize));
		sprintf_s(tmp, 50, "Global\\%sToSrv", m_pipeName.c_str());
		sprintf_s(tmp2, 50, "Global\\%sToClnt", m_pipeName.c_str());
		m_comm.reset(new winEventComm(getUniqName(clinetThreadID, clientid, tmp, buf, 128)
									, getUniqName(clinetThreadID, clientid, tmp2, buf2, 50)));
		m_exitHandler.reset(new exitCheckHnadler(clinetThreadID));
		if (m_module)
			m_exitHandler->setModule(m_module.get());
		m_module->onAccept(tmp, 50);

		strcpy(tmp, "OK");
		memcpy(tmp+3, &m_shareMemSize, 4);
		asio::write(m_socket, buffer(tmp, 7), e);
		run();
		
	}
	
	void cancel()
	{
		if (m_exitHandler)
			m_exitHandler->setCancel(true);
		ios().stop();
		socket().cancel();
	}
	
	static std::vector<connection* > connections;
	
	static void stop()
	{
		mutex::scoped_lock lck(m_mutex);
		try
		{
			for (size_t i=0;i<connections.size();i++)
				connections[i]->cancel();
		}
		catch(system::system_error &){};
	}
	
};

std::vector<connection* > connection::connections;
mutex connection::m_mutex;
std::string connection::m_pipeName;
unsigned int connection::m_shareMemSize;

// ---------------------------------------------------------------------------
//		worker
// ---------------------------------------------------------------------------

class worker : private noncopyable 
{

	static mutex m_mutex;	
	static std::vector< shared_ptr<thread> > m_threads;
	static std::vector<worker*> m_workers; //used by Muliti thread.
	static worker* worker::findWaitThread()
	{
		mutex::scoped_lock lck(m_mutex);
		for (size_t i=0;i<m_workers.size();i++)
			if (m_workers[i]->m_connection==NULL)
				return m_workers[i];
		return NULL;
	}
	
	shared_ptr<connection>  m_connection;
	
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
	static void worker::registThread(shared_ptr<thread> t)
	{
		mutex::scoped_lock lck(m_mutex);
		m_threads.push_back(t);
	}

	static void worker::join()
	{
		for (size_t i=0;i<m_threads.size();i++)
			m_threads[i]->join();
		m_threads.erase(m_threads.begin(), m_threads.end());
	}
	
	/**	In search of an empty worker thread, 
	 *	if there is nothing, it will create, perform and return.
	 */
	static worker* worker::get(const IAppModuleBuilder* app)
	{
		
		worker* p = findWaitThread();
		if (p==NULL)
		{
			thread::attributes attr;
			attr.set_stack_size( 125 *  1024); 
			p = new worker();
			shared_ptr<thread> t(new thread(attr, bind(&worker::run, p, app)));
			registThread(t);
		}
		return p;
	}
	
	/**
	 *	Call from accepter thread.
	 */
	worker()
	{
		mutex::scoped_lock lck(m_mutex);
		m_workers.push_back(this);
	}

	void worker::setConnection(shared_ptr<connection> conn)
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
					system::error_code ec;
					tcp::endpoint endpoint;
					endpoint.address(address(address_v4::from_string("127.0.0.1")));
					shared_ptr<IAppModule> mod(((IAppModuleBuilder*)app)->createSessionModule(endpoint, m_connection.get(), SERVER_TYPE_CPT));
					m_connection->setModule(mod);
					if (mod->checkHost(hostCheckName))
						m_connection->start(); //It does not return, unless a connection is close. 
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
					std::string s = "Pipe server " + *msg;
					server::erh->printError(s.c_str());
				}else
					server::erh->printError(boost::diagnostic_information(e).c_str());
			}
		}

		catch(...)
		{
			if (server::erh)
				server::erh->printError("pipe server Unknown");
		}
		//An end of this thread will delete self.
		delete this;
	}

};
bool worker::shutdown = false;
const char* worker::hostCheckName;
condition_variable worker::condition;
mutex worker::m_mutex;	
std::vector< shared_ptr<thread> > worker::m_threads;
std::vector<worker*>  worker::m_workers;


// ---------------------------------------------------------------------------
//		server
// ---------------------------------------------------------------------------
inotifyHandler* server::erh=NULL;

/** server
 *	If it starts, a server will create the exclusive thread for accpter 
 *	and will go into an infinite loop. 
 */
server::server(shared_ptr<IAppModuleBuilder> app, const std::string& name 
				, std::size_t max_connections, unsigned int shareMemSize,const char* hostCheckName)
		: m_app(app), m_maxConnections(max_connections),m_stopped(false)
		
{
	worker::hostCheckName = hostCheckName;
	m_newConnection.reset(new connection());
	connection::m_pipeName = name;
	connection::m_shareMemSize = shareMemSize;
	m_acceptor.open(name);
}

/** Start the server
 */
void server::start()
{
	shared_ptr<thread> t(new thread(bind(&server::run, this)));
	worker::registThread(t);
	
}

void server::run()
{
	try
	{
		if (erh)
			erh->printInfo("Start Pipe server.");
		while (1)
		{
			if (m_stopped) return;
			while (connection::connections.size()>m_maxConnections)
			{
				Sleep(100);
				if (m_stopped)
					return;
			}
			//Time to await slight time and no pipe be exists. 
			//A client is trying connection several times.
			m_acceptor.accept(m_newConnection->socket());
			if (m_stopped) return;
			system::error_code e;
			onAccept(e);
		}
	}
	catch(bzs::rtl::exception &e)
	{
		if (erh)
		{
			if(const std::string *msg = getMsg(e))
			{
				std::string s = "Pipe server accept " + *msg;
				erh->printError(s.c_str());
			}else
				erh->printError(boost::diagnostic_information(e).c_str());
		}
		stop();
	}
}

void server::onAccept(const system::error_code& e)
{
	//connection is passed to a thread and it resumes.
	if (!e)
	{
		worker* w = worker::get(m_app.get());
		w->setConnection(m_newConnection);
		worker::condition.notify_all();
		m_newConnection.reset(new connection());
	}
}

void server::doStop()
{
	if (!m_stopped)
	{
		m_stopped = true;
		if (erh)
			erh->printInfo("Stopping Pipe server ...");
		m_acceptor.cancel();
		worker::shutdown = true;
		connection::stop();
		worker::condition.notify_all();
	}
}

void server::stop()
{
	doStop();
	worker::join();
}

}//namespace pipe
}//namespace server
}//namespace netsvc
}//namespace bzs

