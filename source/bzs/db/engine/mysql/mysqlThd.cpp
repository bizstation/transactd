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
#include "mysqlThd.h"
#include "mysqlInternal.h"
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>

unsigned int g_openDatabases = 0;

extern unsigned int g_lock_wait_timeout;
extern char* g_transaction_isolation;

#ifdef USETLS
tls_key g_tlsiID ;
#else
__THREAD char* __THREAD_BCB t_stack = NULL;
#endif


inline char* getStackaddr()
{
#ifdef USETLS
	return (char*)tls_getspecific(g_tlsiID);
#else
	return t_stack;
#endif
}

inline void setStackaddr(char* v)
{
#ifdef USETLS
	tls_setspecific(g_tlsiID, v);
#else
	t_stack = v;
#endif
}


void* operator new(size_t t)
{   
	return td_malloc(t, MY_WME);   
}   

void operator delete(void* p)
{   
	td_free(p);   
}   
  
void* operator new[](size_t t)
{   
	return td_malloc(t, MY_WME);   
}   

void operator delete[](void* p)
{   
	td_free(p);   
} 

void initThread(THD* thd )
{
	if (getStackaddr() == NULL)
	{
		my_thread_init();
		int v;
		setStackaddr(reinterpret_cast<char*>(&v));
		
	}
	if (thd)
		thd->thread_stack = getStackaddr();
}

void endThread()
{
	setStackaddr(0);
}

THD* buildTHD()
{
	THD* thd = new THD();
	mysql_mutex_lock(&LOCK_thread_count);
	thd->variables.pseudo_thread_id = thread_id++;
	++g_openDatabases;
	cp_add_global_thread(thd);
	mysql_mutex_unlock(&LOCK_thread_count);
	thd->thread_id = thd->variables.pseudo_thread_id;
	thd->thread_stack = reinterpret_cast<char*>(&thd);
	thd->store_globals();
	assert(thd->mysys_var);
	set_mysys_var(thd->mysys_var);
	thd->system_thread = NON_SYSTEM_THREAD;
	const NET v = {0};
	thd->net = v;
	
	thd->variables.option_bits |= OPTION_BIN_LOG;
	if (strcmp(g_transaction_isolation, "READ-COMMITTED")==0)
		thd->variables.tx_isolation = ISO_READ_COMMITTED;
	else if (strcmp(g_transaction_isolation, "REPEATABLE-READ")==0)
		thd->variables.tx_isolation = ISO_REPEATABLE_READ;
	else if (strcmp(g_transaction_isolation, "READ-UNCOMMITTED")==0)
		thd->variables.tx_isolation = ISO_READ_UNCOMMITTED;
	else if (strcmp(g_transaction_isolation, "SERIALIZABLE")==0)
		thd->variables.tx_isolation = ISO_SERIALIZABLE;

	thd->clear_error();
	char tmp[256];
	sprintf_s(tmp, 256,"set innodb_lock_wait_timeout=%d;", g_lock_wait_timeout);
	dispatch_command(COM_QUERY, thd, tmp, (uint)strlen(tmp));

	td_free(thd->db);
	thd->db = td_strdup("bizstation", MYF(0));
	if(thd->variables.sql_log_bin)
		thd->set_current_stmt_binlog_format_row();
	return thd;
}

THD* attachThd(THD* thd) 
{
	THD* curThd = my_pthread_getspecific(THD*, THR_THD);
	

	if (!thd)
		my_pthread_setspecific_ptr(THR_THD, thd);
	else
	{
		initThread(thd);
		assert(thd->mysys_var);
		if (curThd != thd)
		{
			thd->store_globals();
			set_mysys_var(thd->mysys_var);
		}
	}
	return curThd;
}

THD* createThdForThread()
{
	initThread(NULL);
	THD* thd =  buildTHD();
	
	attachThd(thd);
	return thd;
}

void deleteThdForThread(THD* thd)
{
	cp_restore_globals(thd);
	mysql_mutex_lock(&LOCK_thread_count);
	--g_openDatabases;
	
	if (thd->mdl_context.has_locks())
		thd->mdl_context.set_transaction_duration_for_all_locks();

	cp_thd_release_resources(thd);
	cp_remove_global_thread(thd);
	mysql_mutex_unlock(&LOCK_thread_count);
	releaseTHD(thd);
	
}
