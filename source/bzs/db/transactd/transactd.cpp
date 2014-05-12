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
#include <bzs/netsvc/server/serverCpt.h>
#include <bzs/netsvc/server/serverTpool.h>
#include <bzs/rtl/exception.h>
#include <bzs/rtl/debuglog.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma warning(disable:4800) 
#include <bzs/db/engine/mysql/mysqlInternal.h>
#pragma warning(default:4800) 

#include <bzs/db/engine/mysql/mysqlThd.h>
#include "appBuilderImple.h"

/** tcp server type
 */
#define TCP_CPT_SERVER    1
#define TCP_TPOOL_SERVER  2

using namespace bzs::netsvc::server;

/** system valiables
 */
static char* g_listenAddress=NULL;
static char* g_listenPort = NULL;
static char* g_hostCheckUserName = NULL;

static unsigned int g_tcpServerType = TCP_CPT_SERVER;

static unsigned int g_maxTcpConnections = 200;
static unsigned int g_pool_threads = 20;
unsigned int g_pipeCommSharememSize = 3145728;
int g_tableNmaeLower = 1; //defined in btrvProtocol.h 
int g_useBtrvVariableTable = 1;
unsigned int g_lock_wait_timeout = 1;
char* g_transaction_isolation = NULL;

/** tcp server 
 */
boost::shared_ptr<iserver> srv;

/** pipe server
 */
#ifdef WIN32
	#define PIPE_SERVER
	#include <bzs/netsvc/server/serverpipe.h>
	static unsigned int g_maxPipeConnections = 50;
	static int g_usePipedLocal = 1;
	#define PIPE_NAME "Transactd" //this name is fixed. 
	boost::shared_ptr<iserver> srv_p;
#endif

#ifdef USE_HANDLERSOCKET
	static char* g_hs_listenPort = NULL;
	static int g_use_hs = 0;
#endif

class serveErrorHandler : public inotifyHandler
{
public:

	void printError(const char* msg)
	{
		sql_print_error("%s%s","Transactd: ", msg);
	}

	void printInfo(const char* msg)
	{
		sql_print_information("%s%s","Transactd: ", msg);
	}
};

serveErrorHandler srvErh;

/** entry point for plugin
 */
static int transactd_plugin_init(void *p)
{
	try
	{
		if (g_tcpServerType == TCP_CPT_SERVER)
		{
			boost::shared_ptr<IAppModuleBuilder> app(new transctionalIF(PROTOCOL_TYPE_BTRV|PROTOCOL_TYPE_ASYNCWRITE));
			cpt::server* p = new cpt::server(g_maxTcpConnections, g_hostCheckUserName);
			p->addApplication(app, g_listenAddress, g_listenPort);
			#ifdef USE_HANDLERSOCKET
			if (g_use_hs)
			{
				boost::shared_ptr<IAppModuleBuilder> app_hs(new transctionalIF(PROTOCOL_TYPE_HS));
				p->addApplication(app_hs, g_listenAddress, g_hs_listenPort);
			}
			#endif
			srv.reset(p);
			
		}
		else if (g_tcpServerType == TCP_TPOOL_SERVER)
		{
			boost::shared_ptr<IAppModuleBuilder> app(new transctionalIF(PROTOCOL_TYPE_BTRV|PROTOCOL_TYPE_ASYNCWRITE));
			tpool::server* p = new tpool::server(g_pool_threads, g_hostCheckUserName);
			p->addApplication(app, g_listenAddress, g_listenPort);
			#ifdef USE_HANDLERSOCKET
			if (g_use_hs)
			{
				boost::shared_ptr<IAppModuleBuilder> app_hs(new transctionalIF(PROTOCOL_TYPE_HS|PROTOCOL_TYPE_MEMBUFFER));
				p->addApplication(app_hs, g_listenAddress, g_hs_listenPort);
			}
			#endif
			srv.reset(p);
		}
		if (srv)
		{
			srv->registerErrorHandler(&srvErh);
			srv->start();
		}
	#ifdef PIPE_SERVER
		if (g_usePipedLocal)
		{
			boost::shared_ptr<IAppModuleBuilder> app(new transctionalIF(PROTOCOL_TYPE_BTRV|PROTOCOL_TYPE_MEMBUFFER));
			srv_p.reset(new pipe::server(app, PIPE_NAME , g_maxPipeConnections, g_pipeCommSharememSize, g_hostCheckUserName));
			srv_p->registerErrorHandler(&srvErh);
			srv_p->start();
		}
	#endif
		DEBUG_PROFILE_INIT()
		return 0;
	}
	catch(boost::system::system_error &e)
	{
		char buf[1024];
		sprintf_s(buf, 1024, "Transactd error at startup. :%s",  e.what());
		sql_print_error("%s", buf);
		return 1;
	}
	catch(bzs::rtl::exception &e)
	{
		if(const std::string *msg = getMsg(e))
		{
			std::string s = "Transactd error at startup. :" + *msg;
			sql_print_error("%s", s.c_str());
		}else
			sql_print_error("%s", boost::diagnostic_information(e).c_str());
		return 1;	
	}
	catch(...)
	{
		sql_print_error("%s", "Transactd unknown error at startup.");
	}
	return 1;
}

void shutdownSrv(boost::shared_ptr<iserver>& srv)
{
	try
	{
		if (srv)
		{
			srv->stop();
			srv.reset();
		}
	}
	catch(boost::system::system_error &e)
	{
		sql_print_error("Transactd %s", e.what());	
	}
	catch(bzs::rtl::exception &e)
	{
		if(const std::string *msg = getMsg(e))
		{
			std::string s = "Transactd. :" + *msg;
			sql_print_error("%s", s.c_str());
		}else
			sql_print_error("%s", boost::diagnostic_information(e).c_str());
	}
	catch(...)
	{
		sql_print_error("%s", "Transactd Unknown");
	}
}

/** called from mysql at shutdown plugin.
 */
static int transactd_plugin_deinit(void *p)
{
	#ifdef PIPE_SERVER
	shutdownSrv(srv_p);
	#endif
	shutdownSrv(srv);
	return 0;	
}

static struct st_mysql_daemon transactd_info =
{ 
	MYSQL_DAEMON_INTERFACE_VERSION 
};

/** system valiable define
 */
static MYSQL_SYSVAR_STR(address, g_listenAddress, PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC, "0::0|0.0.0.0", NULL, NULL, "0.0.0.0");
static MYSQL_SYSVAR_STR(port, g_listenPort, PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC, NULL, NULL, NULL, "8610");
static MYSQL_SYSVAR_STR(hostcheck_username, g_hostCheckUserName, PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC, NULL, NULL, NULL, "root");
static MYSQL_SYSVAR_UINT(max_tcp_connections, g_maxTcpConnections, PLUGIN_VAR_READONLY, "1..3000", 0, 0, 200, 1,3000, 0);
static MYSQL_SYSVAR_INT(table_name_lowercase, g_tableNmaeLower, PLUGIN_VAR_READONLY,"", 0, 0, 1, 0,1, 0);
static MYSQL_SYSVAR_INT(use_btrv_variable_table, g_useBtrvVariableTable, PLUGIN_VAR_READONLY,"", 0, 0, 1, 0,1, 0);
static MYSQL_SYSVAR_UINT(pool_threads, g_pool_threads, PLUGIN_VAR_READONLY,"1..1000", 0, 0, 20, 1,1000, 0);
static MYSQL_SYSVAR_UINT(tcp_server_type, g_tcpServerType, PLUGIN_VAR_READONLY,"1..2", 0, 0, TCP_CPT_SERVER, 1,2, 0);
static MYSQL_SYSVAR_UINT(lock_wait_timeout, g_lock_wait_timeout, PLUGIN_VAR_READONLY,"1..3600", 0, 0, 1, 1,3600, 0);
static MYSQL_SYSVAR_STR(transaction_isolation, g_transaction_isolation, PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC, NULL, NULL, NULL, "READ-COMMITTED");

#ifdef PIPE_SERVER
static MYSQL_SYSVAR_UINT(pipe_comm_sharemem_size, g_pipeCommSharememSize, PLUGIN_VAR_READONLY,"66000..52428800", 0, 0, 3145728, 66000,52428800, 0);
static MYSQL_SYSVAR_UINT(max_pipe_connections, g_maxPipeConnections, PLUGIN_VAR_READONLY, "1..100", 0, 0, 50, 1,100, 0);
static MYSQL_SYSVAR_INT(use_piped_local, g_usePipedLocal, PLUGIN_VAR_READONLY, "", 0, 0, 1, 0, 1,0);
#endif

#ifdef USE_HANDLERSOCKET
static MYSQL_SYSVAR_STR(hs_port, g_hs_listenPort, PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC, NULL, NULL, NULL, "9999");
static MYSQL_SYSVAR_INT(use_handlersocket, g_use_hs, PLUGIN_VAR_READONLY,"", 0, 0, 0, 0,1, 0);
#endif

/** system valiables struct.
 */
static struct st_mysql_sys_var *g_systemVariables[] =
{
	MYSQL_SYSVAR(address),
	MYSQL_SYSVAR(port),
	MYSQL_SYSVAR(max_tcp_connections),
	MYSQL_SYSVAR(hostcheck_username),
	MYSQL_SYSVAR(table_name_lowercase),
	MYSQL_SYSVAR(use_btrv_variable_table),
	MYSQL_SYSVAR(pool_threads),
	MYSQL_SYSVAR(tcp_server_type),
	MYSQL_SYSVAR(lock_wait_timeout),
	MYSQL_SYSVAR(transaction_isolation),
#ifdef PIPE_SERVER
	MYSQL_SYSVAR(pipe_comm_sharemem_size),
	MYSQL_SYSVAR(max_pipe_connections),
	MYSQL_SYSVAR(use_piped_local), 
#endif
#ifdef USE_HANDLERSOCKET
	MYSQL_SYSVAR(use_handlersocket), 
	MYSQL_SYSVAR(hs_port),
#endif
	0
};

/** show valiables struct.
 */
static st_mysql_show_var g_showVariables[] =
{
	{"trnsctd_tcp_connections", (char*)&cpt::g_connections, SHOW_INT},
	{"trnsctd_tcp_wait_threads", (char*)&cpt::g_waitThread, SHOW_INT},
	{"trnsctd_tpool_connections", (char*)&tpool::g_connections, SHOW_INT},
	{"trnsctd_tpool_threads", (char*)&tpool::server::m_threadPoolSize, SHOW_INT},
#ifdef PIPE_SERVER
	{"trnsctd_pipe_connections", (char*)&pipe::g_connections, SHOW_INT},
	{"trnsctd_pipe_wait_threads", (char*)&pipe::g_waitThread, SHOW_INT},
#endif
	{"trnsctd_cur_open_databases", (char*)&g_openDatabases, SHOW_INT},
	{0, 0, (enum_mysql_show_type)0}
};

/** plugin struct.
 */
mysql_declare_plugin(transactd)
{
	MYSQL_DAEMON_PLUGIN,
	&transactd_info,
	"transactd",//this is plugin name for mysql.
	"Hisashi Yaguchi at BizStation Corp",
	"Transactional access demon for mysql",
	PLUGIN_LICENSE_GPL,
	transactd_plugin_init,
	transactd_plugin_deinit,
	0x0100,
	g_showVariables,
	g_systemVariables,
	NULL
}

mysql_declare_plugin_end;

#ifdef USETLS
extern tls_key g_tlsiID;

#ifdef __APPLE__ 

void __attribute__ ((constructor)) onLoadLibrary(void);
void __attribute__ ((destructor)) onUnloadLibrary(void);

void onLoadLibrary(void)
{
     pthread_key_create(&g_tlsiID, NULL);
}

void onUnloadLibrary(void)
{
     pthread_key_delete(g_tlsiID);  
}

#else //WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{ 
    switch (fdwReason) 
    {
    case DLL_PROCESS_ATTACH:
		if ((g_tlsiID = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		break;
    case DLL_PROCESS_DETACH:
        TlsFree(g_tlsiID);
        break;
    default:
        break;  
    }  
    return TRUE; 
}

#endif //__APPLE__

#endif //USETLS

