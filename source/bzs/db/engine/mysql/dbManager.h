#ifndef BZS_DB_ENGINE_MYSQL_DBMANAGER_H
#define BZS_DB_ENGINE_MYSQL_DBMANAGER_H
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
#include <my_config.h>
#include <boost/thread/mutex.hpp>
#include <bzs/db/engine/mysql/database.h>

/* dbManager original error code */
#define DBM_ERROR_TABLE_USED HA_ERR_LAST + 1

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

struct handle
{
    handle(int i, short d, short t) : id(i), db(d), tb(t), cid(0){};

    int id;
    short db;
    short tb;
    short cid;
};


class dbManager
{
    // Lock for isSutdown(), called by another thread
    mutable boost::mutex m_mutex;

    int m_autoHandle;

protected:
    mutable databases m_dbs;

    mutable std::vector<handle> m_handles;

    table* m_tb;
    std::string m_user;
    std::string m_host;
    bool m_authChecked;

    database* createDatabase(const char* dbname, short cid) const;
    void releaseDatabase(short cid);
    handle* getHandle(int handle) const;
    database* getDatabase(const char* dbname, short cid, bool& created) const;
    database* getDatabaseCid(short cid) const;
    int getDatabaseID(short cid) const;
    table* getTable(int handle, enum_sql_command cmd = SQLCOM_SELECT, engine::mysql::rowLockMode* lck=NULL) const;
    void checkNewHandle(int newHandle) const;
    int addHandle(int dbid, int tableid, int assignid = -1);
    database* useDataBase(int id) const;
    int closeCacheTable(database* db, const std::string& tbname);
    int ddl_execSql(THD* thd, const std::string& sql_stmt);
    int ddl_createDataBase(THD* thd, const std::string& dbname);
    int ddl_dropDataBase(THD* thd, const std::string& dbname,
                         const std::string& dbSqlname);
    int ddl_useDataBase(THD* thd, const std::string& dbSqlname);
    int ddl_dropTable(database* db, const std::string& tbname,
                      const std::string& sqldbname,
                      const std::string& sqltbname);
    int ddl_addIndex(database* db, const std::string& tbname,
                        const std::string& cmd);
    int ddl_renameTable(database* db, const std::string& oldName,
                        const std::string& dbSqlName,
                        const std::string& oldSqlName,
                        const std::string& newSqlName);
    int ddl_replaceTable(database* db, const std::string& name1,
                         const std::string& name2, const std::string& dbSqlName,
                         const std::string& nameSql1,
                         const std::string& nameSql2);
    std::string makeSQLChangeTableComment(const std::string& dbSqlName,
                                          const std::string& tableSqlName,
                                          const char* comment);
    std::string makeSQLDropIndex(const std::string& dbSqlName,
                                 const std::string& tbSqlName,
                                 const char* name);
    void clenupNoException();
    virtual int errorCode(int ha_error) = 0;

public:
    dbManager();
    virtual ~dbManager();
    bool isShutDown() const;

    const databases& dbs() const { return m_dbs; }

    boost::mutex& mutex() { return m_mutex; }
};

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_DBMANAGER_H
