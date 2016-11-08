#ifndef BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
#define BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
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
#ifdef __GNUC__
#pragma interface // implementation mysqlProtocol.cpp
#endif
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
#pragma warning(disable : 4101)
#define NOMINMAX 
#endif
#include <my_config.h>
#include <mysql_version.h>

#if defined(MARIADB_BASE_VERSION)
#  if (MYSQL_VERSION_ID > 100107) 
#    define MARIADB_10_1 MYSQL_VERSION_ID
#  elif (MYSQL_VERSION_ID > 100000)
#    define MARIADB_10_0 MYSQL_VERSION_ID
#  else      
#    define MARIADB_5_5 MYSQL_VERSION_ID
#  endif
#else
#  if (MYSQL_VERSION_ID > 50700) 
#     define MYSQL_5_7 MYSQL_VERSION_ID
#  elif (MYSQL_VERSION_ID > 50600)
#     define MYSQL_5_6 MYSQL_VERSION_ID
#  else
#    define MYSQL_5_5 MYSQL_VERSION_ID
#  endif
#endif

#include <sql/sql_const.h>
#include "my_global.h"
#include <math.h>
#if defined(MYSQL_5_7)
#ifndef HAVE_REPLICATION
#define HAVE_REPLICATION
#endif

// Not use malloc service
#define MYSQL_SERVICE_MYSQL_ALLOC_INCLUDED
typedef unsigned int PSI_memory_key;
typedef int myf_t;
extern "C" {
	extern void * my_malloc(PSI_memory_key key, size_t size, myf_t flags);
	extern void * my_realloc(PSI_memory_key key, void *ptr, size_t size, myf_t flags);
	extern void my_free(void *ptr);
	extern void my_claim(void *ptr);
	extern void * my_memdup(PSI_memory_key key, const void *from, size_t length, myf_t flags);
	extern char * my_strdup(PSI_memory_key key, const char *from, myf_t flags);
	extern char * my_strndup(PSI_memory_key key, const char *from, size_t length, myf_t flags);
}
#include "sql/log.h"
#endif //MYSQL_5_7

#include "sql/sql_class.h"
#include <mysql/plugin.h>
#include "sql/mysqld.h"

#if defined(MYSQL_5_6)
#include "sql/global_threads.h"
#endif //MYSQL_5_6

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
#include "sql/handler.h"
#include "sql/sql_db.h"
#include "sql_acl.h"
#include "sql/sql_show.h"
#include "mysqld_error.h"
#include <password.h>

/* mysql.user password field index */
#ifndef MYSQL_USER_FIELD_PASSWORD
#  if defined(MYSQL_5_7)
#     define MYSQL_USER_FIELD_PASSWORD 40
#  else
#     define MYSQL_USER_FIELD_PASSWORD 2
#  endif
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
#pragma warning(default : 4101)
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
#define cp_reset_diagnostics_area() stmt_da->reset_diagnostics_area()
#define cp_master_accsess() master_access
#define cp_priv_host() priv_host
#define cp_priv_user() priv_user
#elif (defined(MYSQL_5_7))
#define cp_get_sql_error() get_stmt_da()->mysql_errno()
#define query_cache_invalidate3(A, B, C) query_cache.invalidate(A, B, C)
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)
#define cp_reset_diagnostics_area() get_stmt_da()->reset_diagnostics_area()
#define cp_master_accsess() master_access()
#define cp_priv_host() host().str
#define cp_priv_user() user().str
#else // MySQL 5.6 Mariadb 10.0 10.1                                                              
#define cp_get_sql_error() get_stmt_da()->sql_errno()
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)
#define cp_reset_diagnostics_area() get_stmt_da()->reset_diagnostics_area()
#define cp_master_accsess() master_access
#define cp_priv_host() priv_host
#define cp_priv_user() priv_user

#endif

#if defined(MYSQL_5_5) 
#define ha_index_next index_next
#define ha_index_prev index_prev
#define ha_index_first index_first
#define ha_index_last index_last
#define ha_index_next_same index_next_same
#define ha_rnd_next rnd_next
#define ha_rnd_pos rnd_pos
#   if (MYSQL_VERSION_NUM >= 50544)
#       define FINDFILE_6PRAMS
#   endif
#endif // MySQL 5.5 Only

#if ((MYSQL_VERSION_NUM > 50600) && (MYSQL_VERSION_NUM < 1000000)) // MySQL 5.6.25 or later
#   if (MYSQL_VERSION_NUM >= 50625)
#       define FINDFILE_6PRAMS
#		if (MYSQL_VERSION_NUM >= 50708)
#			define MYSQL_578_LATER
#		endif
#   endif
#endif

#if ((MYSQL_VERSION_NUM < 50600) || defined(MARIADB_BASE_VERSION)) //MySQL 5.5 Mariadb 5.5 10.0 10.1

inline void add_global_thread(THD* thd) {}

inline void remove_global_thread(THD* thd) {}

inline void cp_thd_release_resources(THD* thd) {}

inline void cp_restore_globals(THD* thd)
{
    my_pthread_setspecific_ptr(THR_THD, 0);
    my_pthread_setspecific_ptr(THR_MALLOC, 0);
}
#if (MYSQL_VERSION_ID < 50600)
inline void cp_thd_set_read_only(THD* thd, bool v) {}

inline bool cp_thd_get_global_read_only(THD* thd) { return false; }
#else
inline void cp_thd_set_read_only(THD* thd, bool v)
{
    thd->tx_read_only = (v || thd->variables.tx_read_only);
}

inline bool cp_thd_get_global_read_only(THD* thd)
{
    return (thd->variables.tx_read_only != 0);
}
#endif
inline bool cp_open_table(THD* thd, TABLE_LIST* tables,
                          Open_table_context* ot_act)
{
#if defined(MARIADB_10_1)
    return open_table(thd, tables, ot_act);
#else
    return open_table(thd, tables, thd->mem_root, ot_act);
#endif
}
#define set_mysys_var(A)

inline bool cp_has_insert_default_function(Field* fd) 
{
    return fd->unireg_check == Field::TIMESTAMP_DN_FIELD ||
        fd->unireg_check == Field::TIMESTAMP_DNUN_FIELD;
}

inline bool cp_has_update_default_function(Field* fd)
{
    return fd->unireg_check == Field::TIMESTAMP_UN_FIELD ||
        fd->unireg_check == Field::TIMESTAMP_DNUN_FIELD;
}

inline void cp_evaluate_insert_default_function(Field* fd)
{
#if (MYSQL_VERSION_ID > 50600)
	Field* ft = fd;
#else
	Field_timestamp* ft = (Field_timestamp*)(fd);
#endif
	if (ft)
		ft->set_time();
}

inline void cp_evaluate_update_default_function(Field* fd)
{
#if (MYSQL_VERSION_ID > 50600)
	Field* ft = fd;
#else
	Field_timestamp* ft = (Field_timestamp*)(fd);
#endif
	if (ft)
		ft->set_time();
}

inline unsigned char* cp_null_ptr(Field* fd, unsigned char* /*record*/)
{
    return (unsigned char*)fd->null_ptr;   
}

#else // MySQL 5.6. 5.7

inline void cp_thd_release_resources(THD* thd)
{
    thd->release_resources();
}

#if(MYSQL_VERSION_NUM < 50700)
inline void cp_set_mysys_var(struct st_my_thread_var* var)
{
    set_mysys_var(var);
}
#endif

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

inline bool cp_has_insert_default_function(Field* fd) 
{
    return fd->has_insert_default_function();
}

inline bool cp_has_update_default_function(Field* fd)
{
    return fd->has_update_default_function();

}

inline void cp_evaluate_insert_default_function(Field* fd)
{
    fd->evaluate_insert_default_function();
}

inline void cp_evaluate_update_default_function(Field* fd)
{
    fd->evaluate_update_default_function();
}

inline unsigned char* cp_null_ptr(Field* fd, unsigned char* record)
{
    return fd->null_offset() + record;
}

#endif // MySQL 5.6. 5.7

#if (MYSQL_VERSION_NUM < 50611)
#define ha_index_read_map index_read_map
#endif

extern unsigned int g_openDatabases;

#if (defined(MYSQL_5_7))

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

inline void releaseTHD(THD* thd) { delete thd; }

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

inline ulong cp_masterAccess(THD* thd)
{
	return thd->security_context()->master_access();
}

inline int cp_record_count(handler* file, ha_rows* rows)
{
	return file->ha_records(rows);
}

#define cp_strdup(A, B) my_strdup(PSI_INSTRUMENT_ME, (A), (B))
#define cp_set_mysys_var(A) set_mysys_thread_var(A)
inline void cp_set_db(THD* thd, const char* p)
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

inline void cp_set_transaction_duration_for_all_locks(THD* /*thd*/) {}

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

inline void cp_open_error_release(THD* /*thd*/, TABLE_LIST& /*tables*/){}

inline bool cp_query_command(THD* thd, char* str)
{
	COM_DATA com_data;
	com_data.com_query.query = (char*)str;
	com_data.com_query.length = (uint)strlen(str);
	return dispatch_command(thd, &com_data, COM_QUERY);
}

inline void cp_lex_clear(THD* thd)
{
    thd->lex->reset();
}

inline TABLE_SHARE* cp_get_cached_table_share(THD* thd, const char *db, const char *name)
{
	return get_cached_table_share(thd, db, name);
}

inline int cp_store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
	HA_CREATE_INFO *create_info_arg, int with_db_name)
{
	return store_create_info(thd, table_list, packet, create_info_arg, with_db_name!=0);
}

#define cp_get_executed_gtids get_executed_gtids
extern MYSQL_PLUGIN_IMPORT my_bool opt_show_slave_auth_info;

#define STATUS_VAR_SCOPE ,SHOW_SCOPE_GLOBAL
#define DSMRR_SIZE  sizeof(DsMrr_impl)

#else //Not MySQL 5.7

#define OPEN_TABLE_FLAG_TYPE MYSQL_OPEN_GET_NEW_TABLE
#define td_malloc(A, B) my_malloc(A, B)
#define td_realloc(A, B, C) my_realloc(A, B, C)
#define td_strdup(A, B) my_strdup(A, B)
#define td_free(A) my_free(A)

inline void releaseTHD(THD* thd) { delete thd; } 

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

inline ulong cp_masterAccess(THD* thd)
{
	return thd->security_ctx->master_access;
}

inline int cp_record_count(handler* file, ha_rows* rows)
{
	*rows = file->records();
	return 0;
}

#define cp_strdup(A, B) my_strdup((A), (B))
#define cp_set_mysys_var(A) set_mysys_var(A)

inline void cp_set_db(THD* thd, const char* p)
{
	thd->set_db(p, strlen(p));
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

inline void cp_set_transaction_duration_for_all_locks(THD* thd) {}

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

inline void cp_open_error_release(THD* thd, TABLE_LIST& tables) {}

inline bool cp_query_command(THD* thd, char* str)
{
	return dispatch_command(COM_QUERY, thd, str, (uint)strlen(str));
}

inline void cp_lex_clear(THD* thd)
{
    thd->lex->many_values.empty();
}

#if defined(MARIADB_10_1) ||  defined(MARIADB_10_0)
#define NO_LOCK_OPEN
inline TABLE_SHARE* cp_get_cached_table_share(THD* thd, const char *db, const char *name)
{
	return tdc_acquire_share(thd, db, name, GTS_VIEW |GTS_TABLE );
}
inline void cp_tdc_release_share(TABLE_SHARE* s)
{
    tdc_release_share(s);
}

    #if defined(MARIADB_10_1)
    inline int cp_store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
                          void *create_info_arg, int with_db_name) 
    {
        return show_create_table(thd, table_list,  packet, 
                (Table_specification_st*)create_info_arg, (enum_with_db_name) with_db_name);
    } 
    #elif (MARIADB_10_0 >= 100013)
    inline int cp_store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
                          void *create_info_arg, int with_db_name) 
    {
        return show_create_table(thd, table_list,  packet, 
                (HA_CREATE_INFO*)create_info_arg, (enum_with_db_name) with_db_name);
    }
    #else //Mariadb 10.0.9 - .12
    inline int cp_store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
                          HA_CREATE_INFO* create_info_arg, int with_db_name)
    {
        return store_create_info(thd, table_list, packet, create_info_arg, with_db_name!=0, FALSE);
    }
    
    #endif
#else // Not MARIADB_10_1 || MARIADB_10_0


inline TABLE_SHARE* cp_get_cached_table_share(THD* /*thd*/, const char *db, const char *name)
{
	return get_cached_table_share(db, name);
}
inline int cp_store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
                          HA_CREATE_INFO *create_info_arg, int with_db_name)
{
    return store_create_info(thd, table_list, packet, create_info_arg, with_db_name!=0);
}

#define cp_get_executed_gtids get_logged_gtids

#endif // Not MARIADB_10_1 || MARIADB_10_0

#define STATUS_VAR_SCOPE 
#define DSMRR_SIZE  0
#endif // Not MySQL 5.7


#if (MYSQL_VERSION_ID < 50600) || defined(MARIADB_10_0) 
    #if defined(MARIADB_10_0)
    inline bool cp_tdc_open_view(THD *thd, TABLE_LIST *table_list, const char *alias,
                        const char *cache_key, uint cache_key_length,  uint flags)
    {
        return tdc_open_view(thd, table_list, alias, cache_key , cache_key_length, thd->mem_root, flags);
    }
    #else
    inline bool cp_tdc_open_view(THD *thd, TABLE_LIST *table_list, const char *alias,
                        const char *cache_key, uint cache_key_length,  uint flags)
    {
        return tdc_open_view(thd, table_list, alias, (char *)cache_key , cache_key_length, thd->mem_root, flags);
    }
    #endif
#else
#define cp_tdc_open_view tdc_open_view
#endif

#if (MYSQL_VERSION_ID < 50600)
inline uint cp_get_table_def_key(THD *thd, TABLE_LIST* tables, const char** key)
{
    return create_table_def_key(thd, (char *)*key, tables, 0);
}
#else
inline uint cp_get_table_def_key(THD *thd, TABLE_LIST* tables, const char** key)
{
	return (uint)get_table_def_key(tables, key);
}
#endif
/* find_files is static function in maridb. 
   make_db_list function is not static, but it is not list in sql_show.h.  
*/

#if (defined(MARIADB_10_0) || defined(MARIADB_10_1))
    typedef Dynamic_array<LEX_STRING*> SQL_Strings;
#if (!defined(MARIADB_10_1))
    typedef struct st_lookup_field_values
    {
        LEX_STRING db_value, table_value;
        bool wild_db_value, wild_table_value;
    } LOOKUP_FIELD_VALUES;
#endif
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
#ifdef FINDFILE_6PRAMS
        MEM_ROOT tmp_mem_root;
#		ifdef MYSQL_578_LATER
			init_sql_alloc(key_memory_get_all_tables, &tmp_mem_root, TABLE_ALLOC_BLOCK_SIZE, 0);
#		else
			init_sql_alloc(&tmp_mem_root, TABLE_ALLOC_BLOCK_SIZE, 0);
#		endif
        return find_files(thd, files, NullS,  mysql_data_home, "", true, &tmp_mem_root);
#else
        return find_files(thd, files, NullS,  mysql_data_home, "", true);
#endif        
    }
#endif


#if (defined(MARIADB_10_1) && MARIADB_10_1 > 100108)
inline void cp_setup_rpl_bitmap(TABLE* table)
{
    bitmap_set_all(table->write_set);
    table->rpl_write_set = table->write_set;
}
#else

inline void cp_setup_rpl_bitmap(TABLE* table){};

#endif


class safe_mysql_mutex_lock
{
    mysql_mutex_t *m_lock;
public:
    safe_mysql_mutex_lock(mysql_mutex_t *lock): m_lock(lock)
    {
        if (m_lock)
            mysql_mutex_lock(m_lock);
    }
    ~safe_mysql_mutex_lock()
    {
        if (m_lock)
            mysql_mutex_unlock(m_lock);
    }
};


struct innodb_prebuit
{
	ulong		magic_n;
	void*	    table;
	void*	    index;
	void*		trx;
    //unsigned int dummy; // Bit flags
    
	unsigned	sql_stat_start:1;
#ifndef MYSQL_5_7 
	unsigned	mysql_has_locked:1;
#endif
	unsigned	clust_index_was_generated:1;
	unsigned	index_usable:1;
	unsigned	read_just_key:1;
	unsigned	used_in_HANDLER:1;
	unsigned	template_type:2;
    
    void* dummy2; //mysql_row_templ_t* mysql_template
    void* dummy3; //mem_heap_t*	heap;
    void* dummy4; //ins_node_t*	ins_node;	
    void* dummy5; //byte* ins_upd_rec_buff	
    void* dummy6; //const byte*	default_rec;	
    unsigned long hint_need_to_fetch_extra_cols;
};

/* Values for hint_need_to_fetch_extra_cols */
#define ROW_RETRIEVE_DEFAULT    	0
#define ROW_RETRIEVE_PRIMARY_KEY	1
#define ROW_RETRIEVE_ALL_COLS		2

#define ROW_PREBUILT_ALLOCATED	78540783

#endif // BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
