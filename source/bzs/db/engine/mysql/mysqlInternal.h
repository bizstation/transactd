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

#ifdef NOMINMAX
#undef NOMINMAX
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4800)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#pragma warning(disable : 4805)
#endif

#include <my_config.h>
#include <mysql/plugin.h>
#include <mysql_version.h>
#include <sql/sql_const.h>
#include "sql/mysqld.h"

#if ((MYSQL_VERSION_ID >= 50600) && !defined(MARIADB_BASE_VERSION))
#include "sql/global_threads.h"
#endif

#include "sql/sql_plugin.h"
#include "sql/sql_cache.h"
#include "sql/structs.h"
#include "sql/sql_priv.h"
#include "sql/sql_class.h"
#include "sql/unireg.h"
#include "sql/lock.h"
#include "sql/key.h"
#include "my_global.h"
#include "sql/transaction.h"
#include "sql/sql_base.h"
#include "sql/sql_parse.h"
#include "sql/sql_table.h"
#include "sql/sql_db.h"
#include "sql_acl.h"
#include "mysqld_error.h"

#if ((MYSQL_VERSION_ID > 50700) && !defined(MARIADB_BASE_VERSION))
#include "sql/log.h"
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

#if (MYSQL_VERSION_ID < 50600) // MySQL and MariaDB both
#define user_defined_key_parts key_parts
#define MDL_SHARED_UPGRADABLE MDL_SHARED_WRITE
#define cp_get_sql_error() stmt_da->sql_errno()
#define cp_isOk() stmt_da->is_ok()
#define cp_set_overwrite_status(A) stmt_da->can_overwrite_status = A
#elif((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION))
#define cp_get_sql_error() get_stmt_da()->mysql_errno()
#define query_cache_invalidate3(A, B, C) query_cache.invalidate(A, B, C)
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)
#else
#define cp_get_sql_error() get_stmt_da()->sql_errno()
#define cp_isOk() get_stmt_da()->is_ok()
#define cp_set_overwrite_status(A) get_stmt_da()->set_overwrite_status(A)

#endif

#if (MYSQL_VERSION_NUM < 50600) // MySQL Only
#define ha_index_next index_next
#define ha_index_prev index_prev
#define ha_index_first index_first
#define ha_index_last index_last
#define ha_index_next_same index_next_same
#define ha_rnd_next rnd_next
#define ha_rnd_pos rnd_pos
#endif

#if ((MYSQL_VERSION_NUM < 50600) || defined(MARIADB_BASE_VERSION))

inline void cp_add_global_thread(THD* thd)
{
    ;
}

inline void cp_remove_global_thread(THD* thd)
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

inline void cp_thd_set_read_only(THD* thd)
{
    ;
}

inline bool cp_thd_get_read_only(THD* thd)
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

inline void cp_add_global_thread(THD* thd)
{
    add_global_thread(thd);
}

inline void cp_remove_global_thread(THD* thd)
{
    remove_global_thread(thd);
}

inline void cp_thd_release_resources(THD* thd)
{
    thd->release_resources();
}

inline void cp_set_mysys_var(st_my_thread_var* var)
{
    set_mysys_var(var);
}

inline void cp_restore_globals(THD* thd)
{
    thd->restore_globals();
}

inline void cp_thd_set_read_only(THD* thd)
{
    thd->tx_read_only = (thd->variables.tx_read_only != 0);
}

inline bool cp_thd_get_read_only(THD* thd)
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

/* memory management */
#if ((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION))
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
#else
#define td_malloc(A, B) my_malloc(A, B)
#define td_realloc(A, B, C) my_realloc(A, B, C)
#define td_strdup(A, B) my_strdup(A, B)
#define td_free(A) my_free(A)

inline void releaseTHD(THD* thd)
{
    delete thd;
}
#endif

#endif // BZS_DB_ENGINE_MYSQL_MYSQLINTERNAL_H
