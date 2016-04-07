#ifndef BZS_DB_ENGINE_MYSQL_MYSQLPROTOCOL_H
#define BZS_DB_ENGINE_MYSQL_MYSQLPROTOCOL_H
/* =================================================================
 Copyright (C) 2016 BizStation Corp All rights reserved.

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
#include "mysqlInternal.h"
#include <bzs/env/compiler.h>
#include <bzs/db/transactd/connectionRecord.h>


#if (MYSQL_VERSION_ID > 100000)
#  define USE_BINLOG_GTID     1  // like 0-1-50
#elif (!defined(_WIN32) || MYSQL_VERSION_ID > 50700 || MYSQL_VERSION_ID < 50600) // Linux or MySQL 5.5 5.7
#  define USE_BINLOG_VAR      1  
#else // MySQL 5.6  on windows On windows MySQL 5.6 can not access mysql_bin_log variable
#  define NOTUSE_BINLOG_VAR   1  
#endif


// REPL_POS_TYPE
#define BINLOGNAME_SIZE                 119
#define GTID_SIZE                       64

#define REPL_POSTYPE_MARIA_GTID         1  // see tdapapi.h
#define REPL_POSTYPE_POS                2  // see tdapapi.h

#define OPEN_TABLE_TIMEOUT_SEC 2

pragma_pack1
struct binlogPos
{
    my_off_t pos;
    char type;
    char filename[BINLOGNAME_SIZE];
    char gtid[GTID_SIZE];
};
pragma_pop

class safe_commit_lock
{
    THD* m_thd;
    MDL_ticket* m_commits_lock;
public:
    safe_commit_lock(THD* thd);
    bool lock();
    ~safe_commit_lock();
};

short getBinlogPos(THD* thd, binlogPos* pos, THD* tmpThd);
int getSlaveStatus(THD* thd, bzs::db::transactd::connection::records& recs);
int execSql(THD* thd, const char* sql);
void readDbList(THD* thd, bzs::db::transactd::connection::records& recs);
bool setGrant(THD* thd, const char* host, const char* user,  const char* db);
bool copyGrant(THD* thd, THD* thdSrc, const char* db);
void setDbName(THD* thd, const char* name);

#endif // BZS_DB_ENGINE_MYSQL_MYSQLPROTOCOL_H
