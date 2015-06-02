#ifndef BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
#define BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
/* =================================================================
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
 ================================================================= */

#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifndef MYSQL_DYNAMIC_PLUGIN
#define MYSQL_DYNAMIC_PLUGIN
#endif

#define MYSQL_SERVER 1

#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif

#define HAVE_PSI_INTERFACE // 50700
#define NO_USE_MALLOC_SERVICE_INTERFACE

#ifdef ERROR
#undef ERROR
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4800)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#pragma warning(disable : 4805)
#pragma warning(disable : 4005)
#define NOMINMAX 
#endif
#include <my_config.h>
#include <mysql_version.h>
#if defined(MARIADB_BASE_VERSION) && (MYSQL_VERSION_ID > 100000)
#define MARIADDB_10_0 MYSQL_VERSION_ID
#endif

#include <sql/sql_const.h>
#include "my_global.h"
#include <math.h>
#if ((MYSQL_VERSION_ID > 50700) && !defined(MARIADB_BASE_VERSION))
// Not use malloc service
#define MYSQL_SERVICE_MYSQL_ALLOC_INCLUDED
typedef unsigned int PSI_memory_key;
typedef int myf_t;
extern "C" {
	extern void * my_malloc(PSI_memory_key key, size_t size, myf_t flags);
	extern void * my_realloc(PSI_memory_key key, void *ptr, size_t size, myf_t flags);
	extern void my_free(void *ptr);
	extern void * my_memdup(PSI_memory_key key, const void *from, size_t length, myf_t flags);
	extern char * my_strdup(PSI_memory_key key, const char *from, myf_t flags);
	extern char * my_strndup(PSI_memory_key key, const char *from, size_t length, myf_t flags);
}
#include "sql/log.h"
#endif //For MYSQL 5.7

#include "sql/sql_class.h"
#include <mysql/plugin.h>
#include "sql/mysqld.h"

#if ((MYSQL_VERSION_ID >= 50600) && (MYSQL_VERSION_ID < 50700) && !defined(MARIADB_BASE_VERSION))
#include "sql/global_threads.h"
#endif

#include "sql/sql_plugin.h"
#include "sql/sql_cache.h"
#if (MYSQL_VERSION_ID < 50700)
#include "sql/structs.h"
#include "sql/sql_priv.h"
#endif
#include "sql/unireg.h"
#include "sql/lock.h"
#include "sql/key.h"
#include "sql/transaction.h"
#include "sql/sql_base.h"
#include "sql/sql_parse.h"
#include "sql/sql_table.h"
#include "sql/sql_db.h"
#include "sql_acl.h"
#include "sql/sql_show.h"
#include "mysqld_error.h"
#include <password.h>

/* mysql.user password field index */
#ifndef MYSQL_USER_FIELD_PASSWORD
#define MYSQL_USER_FIELD_PASSWORD 2
#endif

#undef test
#undef sleep
#ifdef ERROR
#define ERROR 0
#endif

#ifdef _MSC_VER
#pragma warning(default : 4996)
#pragma warning(default : 4267)
#pragma warning(default : 4800)
#pragma warning(default : 4805)
#pragma warning(default : 4005)
#endif

#undef min
#undef max
#define THD_KILLED_STATE THD::killed_state
#define THD_NOT_KILLED THD::NOT_KILLED

#if defined(MARIADB_BASE_VERSION)
#define MARIADB_VERSION_NUM MYSQL_VERSION_ID
#define MYSQL_VERSION_NUM 1000000
#else
#define MARIADB_VERSION_NUM 1000000
#define MYSQL_VERSION_NUM MYSQL_VERSION_ID
#endif

#if defined(MARIADB_BASE_VERSION)
#define MDL_SHARED_UPGRADABLE MDL_SHARED_WRITE
#undef THD_KILLED_STATE
#define THD_KILLED_STATE killed_state
#undef THD_NOT_KILLED
#define THD_NOT_KILLED NOT_KILLED
#endif

#if (MYSQL_VERSION_ID < 50600) // MySQL and MariaDB both  5.5
#define user_defined_key_parts key_parts
#define MDL_SHARED_UPGRADABLE MDL_SHARED_WRITE
#define cp_get_sql_error() stmt_da->sql_errno()
#define cp_isOk() stmt_da->is_ok()
#define cp_set_overwrite_status(A) stmt_da->can_overwrite_status = A
#elif((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION)) // MySQL 5.7
#define cp_get_sql_error() get_stmt_da()->mysql_errno()
#define query_cache_invalidate3(A, B, C) query_cache.invalidate(A, B, C)
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)
#else                                                               // MySQL 5.6 Mariadb 10.0
#define cp_get_sql_error() get_stmt_da()->sql_errno()
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)

#endif

#if (MYSQL_VERSION_NUM < 50600) // MySQL 5.5 Only
#define ha_index_next index_next
#define ha_index_prev index_prev
#define ha_index_first index_first
#define ha_index_last index_last
#define ha_index_next_same index_next_same
#define ha_rnd_next rnd_next
#define ha_rnd_pos rnd_pos
#endif

#if ((MYSQL_VERSION_NUM < 50600) || defined(MARIADB_BASE_VERSION))

inline void add_global_thread(THD* thd)
{
    ;
}

inline void remove_global_thread(THD* thd)
{
    ;
}

inline void cp_thd_release_resources(THD* thd)
{
    ;
}

inline void cp_restore_globals(THD* thd)
{
    my_pthread_setspecific_ptr(THR_THD, 0);
    my_pthread_setspecific_ptr(THR_MALLOC, 0);
}

inline void cp_thd_set_read_only(THD* thd, bool v)
{
    ;
}

inline bool cp_thd_get_global_read_only(THD* thd)
{
    return false;
}

inline bool cp_open_table(THD* thd, TABLE_LIST* tables,
                          Open_table_context* ot_act)
{
    return open_table(thd, tables, thd->mem_root, ot_act);
}
#define set_mysys_var(A)
#else


inline void cp_thd_release_resources(THD* thd)
{
    thd->release_resources();
}

inline void cp_set_mysys_var(st_my_thread_var* var)
{
#if(MYSQL_VERSION_NUM < 50700)
    set_mysys_var(var);
#else
	set_mysys_thread_var(var);
#endif
}

inline void cp_restore_globals(THD* thd)
{
    thd->restore_globals();
}

inline void cp_thd_set_read_only(THD* thd, bool v)
{
    thd->tx_read_only = (v || thd->variables.tx_read_only);
}

inline bool cp_thd_get_global_read_only(THD* thd)
{
    return (thd->variables.tx_read_only != 0);
}

inline bool cp_open_table(THD* thd, TABLE_LIST* tables,
                          Open_table_context* ot_act)
{
    return open_table(thd, tables, ot_act);
}
#endif

#if (MYSQL_VERSION_NUM < 50611)
#define ha_index_read_map index_read_map
#endif


extern unsigned int g_openDatabases;
#if((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION))
#define OPEN_TABLE_FLAG_TYPE 0

#define td_malloc(A, B) my_malloc(PSI_NOT_INSTRUMENTED, A, B)
#define td_realloc(A, B, C) my_realloc(PSI_NOT_INSTRUMENTED, A, B, C)
#define td_strdup(A, B) my_strdup(PSI_NOT_INSTRUMENTED, A, B)
#define td_free(A) my_free(A)

/* On Windows,
"operator delete()" function is implemented in mysqld.
But "operator new" operation implement in transactd.dll.
Therefore, memory managers differ. */
#ifdef _WIN32

inline void releaseTHD(THD* thd)
{
	thd->~THD();
	operator delete((void*)thd);
}
#else

inline void releaseTHD(THD* thd)
{
	delete thd;
}
#endif

#include <boost/thread/mutex.hpp>
extern boost::mutex g_db_count_mutex;
extern void global_thd_manager_add_thd(THD *thd);
extern void global_thd_manager_remove_thd(THD *thd);

inline void  cp_set_new_thread_id(THD* thd) 
{
    thd->set_new_thread_id();
	global_thd_manager_add_thd(thd);
	boost::mutex::scoped_lock lck(g_db_count_mutex);
    ++g_openDatabases;
}

inline void cp_dec_dbcount(THD* thd)
{
	{
		boost::mutex::scoped_lock lck(g_db_count_mutex);
		--g_openDatabases;
	}
	global_thd_manager_remove_thd(thd);
}

inline Security_context* cp_security_ctx(THD* thd)
{
	return thd->security_context();
}

inline int cp_record_count(handler* file, ha_rows* rows)
{
	return file->ha_records(rows);
}

#define cp_strdup(A, B) my_strdup(PSI_INSTRUMENT_ME, (A), (B))
#define cp_set_mysys_var(A) set_mysys_thread_var(A)
inline void cp_set_db(THD* thd, char* p)
{
	thd->set_db(to_lex_cstring(p));
}


inline THD* cp_thread_get_THR_THD()
{
	return my_thread_get_THR_THD();
	return NULL;
}

inline int cp_thread_set_THR_THD(THD* thd)
{
	return my_thread_set_THR_THD(thd);
	return 0;
}

inline void cp_set_transaction_duration_for_all_locks(THD* /*thd*/)
{
	;
}

inline void cp_set_mdl_request_types(TABLE_LIST& tables, short mode)
{
	if (mode == -2 /* TD_OPEN_READONLY */)
		tables.mdl_request.set_type(MDL_SHARED_READ);
	else if (mode == -4 /* TD_OPEN_EXCLUSIVE */)
		tables.mdl_request.set_type(MDL_SHARED_NO_READ_WRITE);
	else if (mode == -6 /* TD_OPEN_READONLY_EXCLUSIVE */)
		tables.mdl_request.set_type(MDL_SHARED_READ_ONLY);
	else
		tables.mdl_request.set_type(MDL_SHARED_WRITE);
	
	tables.mdl_request.duration = MDL_TRANSACTION;
}

inline void cp_open_error_release(THD* /*thd*/, TABLE_LIST& /*tables*/)
{
    ;
}

#else //Not MySQL 5.7
#define OPEN_TABLE_FLAG_TYPE MYSQL_OPEN_GET_NEW_TABLE

#define td_malloc(A, B) my_malloc(A, B)
#define td_realloc(A, B, C) my_realloc(A, B, C)
#define td_strdup(A, B) my_strdup(A, B)
#define td_free(A) my_free(A)

inline void releaseTHD(THD* thd)
{
	delete thd;
}
inline  void cp_set_new_thread_id(THD* thd)
{
    mysql_mutex_lock(&LOCK_thread_count);
    ++g_openDatabases;
    thd->variables.pseudo_thread_id = thread_id++;
	add_global_thread(thd);
    mysql_mutex_unlock(&LOCK_thread_count);
    thd->thread_id = thd->variables.pseudo_thread_id;
}
inline void cp_dec_dbcount(THD* thd)
{
    mysql_mutex_lock(&LOCK_thread_count);
    --g_openDatabases;
    mysql_mutex_unlock(&LOCK_thread_count);
	remove_global_thread(thd);
}
inline Security_context* cp_security_ctx(THD* thd)
{
	return thd->security_ctx;
}

inline int cp_record_count(handler* file, ha_rows* rows)
{
	*rows = file->records();
	return 0;
}
#define cp_strdup(A, B) my_strdup((A), (B))
#define cp_set_mysys_var(A) set_mysys_var(A)

inline void cp_set_db(THD* thd, char* p)
{
	td_free(thd->db);
	thd->db = p;
}

inline THD* cp_thread_get_THR_THD()
{
	return (THD*)my_pthread_getspecific(THD*, THR_THD);
}

inline int cp_thread_set_THR_THD(THD* thd)
{
	my_pthread_setspecific_ptr(THR_THD, thd);
	return 0;
}
/*
inline void cp_set_transaction_duration_for_all_locks(THD* thd)
{
	thd->mdl_context.set_transaction_duration_for_all_locks();
}

inline void cp_set_mdl_request_types(TABLE_LIST& tables, short mode)
{
	tables.mdl_request.set_type(MDL_SHARED_READ);
    tables.mdl_request.duration = MDL_EXPLICIT;
}

inline void cp_open_error_release(THD* thd, TABLE_LIST& tables)
{
    thd->mdl_context.release_lock(tables.mdl_request.ticket);
}
*/

inline void cp_set_transaction_duration_for_all_locks(THD* thd)
{
}

inline void cp_set_mdl_request_types(TABLE_LIST& tables, short mode)
{
	if (mode == -2 /* TD_OPEN_READONLY */)
		tables.mdl_request.set_type(MDL_SHARED_READ);
	else if (mode == -4 /* TD_OPEN_EXCLUSIVE */)
		tables.mdl_request.set_type(MDL_SHARED_NO_READ_WRITE);
	else if (mode == -6 /* TD_OPEN_READONLY_EXCLUSIVE */)
		tables.mdl_request.set_type(MDL_SHARED_READ);
	else
		tables.mdl_request.set_type(MDL_SHARED_WRITE);
	
	tables.mdl_request.duration = MDL_TRANSACTION;
}

inline void cp_open_error_release(THD* thd, TABLE_LIST& tables)
{
   
}


#endif

/* find_files is static function in maridb. 
   make_db_list function is not static, but it is not list in sql_show.h.  
*/

#ifdef MARIADDB_10_0
    typedef Dynamic_array<LEX_STRING*> SQL_Strings;
    typedef struct st_lookup_field_values
    {
        LEX_STRING db_value, table_value;
        bool wild_db_value, wild_table_value;
    } LOOKUP_FIELD_VALUES;

    extern int make_db_list(THD *thd, Dynamic_array<LEX_STRING*> *files,
                 LOOKUP_FIELD_VALUES *lookup_field_vals);
    inline int db_list(THD *thd, SQL_Strings *files)
    {
        LOOKUP_FIELD_VALUES lv;
        memset(&lv, 0 ,sizeof(LOOKUP_FIELD_VALUES));
        return make_db_list(thd, files, &lv);
    }
#else
    typedef List<LEX_STRING> SQL_Strings;
    inline int db_list(THD *thd, SQL_Strings *files)
    {
        return find_files(thd, files, NullS,  mysql_data_home, "", true);
    }
#endif

#endif // BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
