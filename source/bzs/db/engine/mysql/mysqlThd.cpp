/* =================================================================
 Copyright (C) 2012-2016 BizStation Corp All rights reserved.

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
#include "mysqlThd.h"
#include "mysqlInternal.h"
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>

unsigned int g_openDatabases = 0;

extern unsigned int g_lock_wait_timeout;
extern char* g_transaction_isolation;
static int transaction_isolation_cache = -1;

#if((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION))
boost::mutex g_db_count_mutex;
LEX_CSTRING NULL_CSTR = { NULL, 0 };
#endif


#ifdef USETLS
tls_key g_tlsiID;
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

#if !(__clang__ && __APPLE__)
#define noexcept
#endif

void* operator new(size_t t)
{
    return td_malloc(t, MY_WME);
}

void operator delete(void* p) noexcept
{
    td_free(p);
}

void* operator new [](size_t t)
{
    return td_malloc(t, MY_WME);
}

void
operator delete[](void* p) noexcept
{
    td_free(p);
}

void initThread(THD* thd)
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

void waitForServerStart()
{
    safe_mysql_mutex_lock lck(&LOCK_server_started);
    while (!mysqld_server_started)
        mysql_cond_wait(&COND_server_started, &LOCK_server_started);
}

int getTransactdIsolation()
{
    if (transaction_isolation_cache != -1) 
        return transaction_isolation_cache;
    else if (strcmp(g_transaction_isolation, "READ-COMMITTED") == 0)
        return transaction_isolation_cache = ISO_READ_COMMITTED;
    else if (strcmp(g_transaction_isolation, "REPEATABLE-READ") == 0)
        return transaction_isolation_cache = ISO_REPEATABLE_READ;
    else if (strcmp(g_transaction_isolation, "SERIALIZABLE") == 0)
        return transaction_isolation_cache = ISO_SERIALIZABLE;
    return transaction_isolation_cache = ISO_READ_UNCOMMITTED;
}

unsigned int getTransactdLockWaitTimeout()
{
    return g_lock_wait_timeout;
}

THD* buildTHD()
{
    if (!mysqld_server_started)
        waitForServerStart();

    THD* thd = new THD();
    
    cp_set_new_thread_id(thd);
    
    thd->thread_stack = reinterpret_cast<char*>(&thd);
    thd->store_globals();
	thd->system_thread = NON_SYSTEM_THREAD;
#ifndef MYSQL_578_LATER
	assert(thd->mysys_var);
	cp_set_mysys_var(thd->mysys_var);
    const NET v = { 0 };
    thd->net = v;
#endif
    thd->variables.option_bits |= OPTION_BIN_LOG;
    thd->variables.tx_isolation = (ulong)getTransactdIsolation();

    thd->clear_error();
    char tmp[256];
    sprintf_s(tmp, 256, "set innodb_lock_wait_timeout=%d;",
              g_lock_wait_timeout);
	cp_query_command(thd, tmp);

    if (thd->variables.sql_log_bin)
        thd->set_current_stmt_binlog_format_row();
    return thd;
}

THD* attachThd(THD* thd)
{
	THD* curThd = cp_thread_get_THR_THD();

	if (!thd)
		cp_thread_set_THR_THD(thd);
    else
    {
        initThread(thd);
#ifndef MYSQL_578_LATER
        assert(thd->mysys_var);
#endif
        if (curThd != thd)
        {
            thd->store_globals();
#ifndef MYSQL_578_LATER
			cp_set_mysys_var(thd->mysys_var);
#endif
        }
    }
    return curThd;
}

THD* createThdForThread()
{
    initThread(NULL);
    THD* thd = buildTHD();

    attachThd(thd);
    return thd;
}

void deleteThdForThread(THD* thd)
{
    try
    {
        cp_restore_globals(thd);
	    cp_thd_release_resources(thd);
	    cp_dec_dbcount(thd);
        releaseTHD(thd);
    }
    catch(...) {};
}
