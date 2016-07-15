/* =================================================================
 Copyright (C) 2012 2013 2016 BizStation Corp All rights reserved.

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

#include "dbManager.h"
#include "errorMessage.h"
#include <bzs/netsvc/server/IAppModule.h> //for result value macro.
#include <bzs/rtl/exception.h>
#include <time.h>
#include "mysqlThd.h"
#include <bzs/db/engine/mysql/mysqlProtocol.h>

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

#define STATUS_ALREADY_INSNAPSHOT       204
#define STATUS_ALREADY_INTRANSACTION    205
#define STATUS_ALREADY_INEXCLUSIVE      206

class smartDbsReopen
{
    THD* m_thd;
    std::vector<boost::shared_ptr<database> >& m_dbs;
public:
    static std::string removeName;

    smartDbsReopen(THD* thd, std::vector<boost::shared_ptr<database> >& dbs) : m_thd(thd), m_dbs(dbs)
    {
        for (size_t i = 0; i < m_dbs.size(); i++)
        {
            if (m_dbs[i] && m_dbs[i]->thd() == m_thd)
            {
                if (m_dbs[i]->inSnapshot())
                    THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ALREADY_INSNAPSHOT, "Already in snapshot.");
                else if (m_dbs[i]->inTransaction())
                    THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ALREADY_INTRANSACTION, "Already in transaction.");
                else if(m_dbs[i]->usingExclusveMode())
                {
                    int code = STATUS_ALREADY_INEXCLUSIVE;
                    std::string s = "Already in exclusive mode when DDL command execute.";
                    printWarningMessage(&code, &s);
                }
                m_dbs[i]->use();
                m_dbs[i]->unUseTables(false);
                m_dbs[i]->closeForReopen();
            }
        }
        //attachThd(m_thd);
    }

    ~smartDbsReopen()
    {
        for (size_t i = 0; i < m_dbs.size(); i++)
        {
            if (m_dbs[i] && m_dbs[i]->thd() == m_thd)
            {
                if (removeName != m_dbs[i]->name())
                {
                    m_dbs[i]->use();
                    m_dbs[i]->reopen();
                }
            }
        }
        attachThd(m_thd);
    }
};

std::string smartDbsReopen::removeName = "";


dbManager::dbManager(netsvc::server::IAppModule* mod) : m_autoHandle(0), m_thd(NULL),
       m_mod(mod), m_authChecked(false)
{
}

dbManager::~dbManager()
{
    if (m_thd)
        deleteThdForThread(m_thd);
}

THD* dbManager::getThd()
{
    if (!m_thd)
    {
        m_thd = createThdForThread();
        
    }
    else
       attachThd(m_thd);
    return m_thd;
}

bool dbManager::isShutDown() const
{
    boost::mutex::scoped_lock lck(m_mutex);
#if defined(MARIADB_BASE_VERSION)
    killed_state st = NOT_KILLED;
#else
    THD::killed_state st = THD::NOT_KILLED;
#endif
    for (size_t i = 0; i < m_dbs.size(); i++)
        if ((m_dbs[i] != NULL) && (m_dbs[i]->thd()->killed != st))
            return true;
    return false;
}

void dbManager::checkNewHandle(int newHandle) const
{
    for (size_t i = 0; i < m_handles.size(); i++)
        if (m_handles[i].id == newHandle)
            THROW_BZS_ERROR_WITH_CODEMSG(1, "Allready exits handle.");
}

// Lock for isSutdown(), called by another thread
void dbManager::releaseDatabase(short cid)
{
    boost::mutex::scoped_lock lck(m_mutex);
    int index = -1;
    for (size_t i = 0; i < m_dbs.size(); i++)
    {
        if ((m_dbs[i] != NULL) && (cid == m_dbs[i]->clientID()))
        {
            index = (int)i;
            break;
        }
    }
    if (index == -1)
        return;
    // close tables release thd
    m_dbs[index].reset();

    // erase handles
    for (int i = (int)m_handles.size() - 1; i >= 0; i--)
        if (m_handles[i].db == index)
            m_handles.erase(m_handles.begin() + i);
}

int dbManager::trxProcessing()
{
    int transactins = 0;
    for (size_t i = 0; i < m_dbs.size(); i++)
    {
        if (m_dbs[i]->inTransaction() || m_dbs[i]->inSnapshot())
            ++transactins;
    }
    return transactins;
}

database* dbManager::useDataBase(int id) const
{
    if (id >= (int)m_dbs.size())
        THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid database id.");
    if (m_dbs[id] == NULL)
        THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid database id.");
    m_dbs[id]->use();
    return m_dbs[id].get();
}

database* dbManager::createDatabase(const char* dbname, short cid) const
{
    database* db = new database(dbname, cid);
    return db;
}

handle* dbManager::getHandle(int handle) const
{
    for (size_t i = 0; i < m_handles.size(); i++)
    {
        if (m_handles[i].id == handle)
            return &m_handles[i];
    }
    char tmp[256];
    sprintf(tmp, "Invalid handle. handle = %d", handle);
    THROW_BZS_ERROR_WITH_CODEMSG(1, tmp);
}

int dbManager::getDatabaseID(short cid) const
{
    for (size_t i = 0; i < m_dbs.size(); i++)
    {
        if (m_dbs[i] != NULL && (m_dbs[i]->clientID() == cid))
            return (int)i;
    }
    return -1;
}

database* dbManager::getDatabaseCid(short cid) const
{
    int id = getDatabaseID(cid);
    if (id == -1)
    {
        char tmp[256];
        sprintf(tmp, "Can not get database object. cid = %d", cid);
        THROW_BZS_ERROR_WITH_CODEMSG(1, tmp);
    }
    return useDataBase(id);
}

database* dbManager::getDatabase(const char* dbname, short cid, bool& created) const
{
    created = false;
    int id = getDatabaseID(cid);
    if (id == -1)
    {
        boost::shared_ptr<database> db(createDatabase(dbname, cid));
        if (db == NULL)
            THROW_BZS_ERROR_WITH_CODEMSG(1, "Can not create database object.");
        id = (int)m_dbs.size();
        boost::mutex::scoped_lock lck(m_mutex);
        m_dbs.push_back(db);
        created = true;
    }
    return useDataBase(id);
}

table* dbManager::getTable(int hdl, enum_sql_command cmd, engine::mysql::rowLockMode* lck) const
{
    handle* h = getHandle(hdl);
    if ((h->db < (int)m_dbs.size()))
        return useDataBase(h->db)->useTable(h->tb, cmd, lck);

    char tmp[256];
    sprintf(tmp, "Invalid handle. handle = %d db = %d tb = %d", hdl, h->db, h->tb);
    THROW_BZS_ERROR_WITH_CODEMSG(1, tmp);
}

int dbManager::addHandle(int dbid, int tableid, int assignid)
{
    ++m_autoHandle;
    if (assignid == -1)
        assignid = m_autoHandle;
    m_handles.push_back(handle(assignid, (short)dbid, (short)tableid));
    return assignid;
}

int dbManager::ddl_execSql(database* db, const std::string& sql_stmt)
{
    THD* thd = getThd();
    int result = 0;
    if (db)
    {
        copyGrant(thd, db->thd(), db->name().c_str());
        setDbName(thd, db->name().c_str());
    }
    else
    {
        if (m_mod->isSkipGrants())
            cp_security_ctx(thd)->skip_grants();
        else
            setGrant(thd, m_mod->host(), m_mod->user(), "");
        setDbName(thd, "");
    }
    result = errorCode(execSql(thd, sql_stmt.c_str()));
    if (thd->mdl_context.has_locks())
        close_thread_tables(thd);
    thd->mdl_context.release_transactional_locks();
    if (db)
        db->use();
    return result;
}

int dbManager::ddl_createDataBase(/*THD* thd,*/ const std::string& dbname)
{
    std::string cmd = "create database `" + dbname + "`";
    return ddl_execSql(NULL, cmd);
}

int dbManager::ddl_dropDataBase(/*THD* thd,*/ const std::string& dbname,
                                const std::string& dbSqlname, short cid)
{
    std::string cmd = "drop database `" + dbSqlname + "`";
    int ret = ddl_execSql(NULL, cmd);
    return ret;
}

int dbManager::ddl_createTable(database* db, const char* cmd)
{
    smartDbsReopen reopen(db->thd(), m_dbs);
    return ddl_execSql(db, cmd);
}

int dbManager::ddl_dropTable(database* db, const std::string& tbname,
                             const std::string& dbSqlname,
                             const std::string& tbSqlname)
{
    smartDbsReopen reopen(db->thd(), m_dbs);
    std::string cmd = "drop table `" + dbSqlname + "`.`" + tbSqlname + "`";
    return ddl_execSql(db, cmd);
}

int dbManager::ddl_addIndex(database* db, const std::string& tbname,
                        const std::string& cmd)
{
    std::string c = "ALTER TABLE `" + db->name() + "`." + cmd;
    smartDbsReopen reopen(db->thd(), m_dbs);
    return ddl_execSql(db, c);
}

/** Key name of multi byte charctord is not supported. Use only ascii.
 */
int dbManager::ddl_dropIndex(database* db, const std::string& tbname, const char* keyname)
{
    std::string s = "drop index `" + std::string(keyname) + "` on `" + db->name() +
                    "`.`" + tbname + "`";
    smartDbsReopen reopen(db->thd(), m_dbs);
    return ddl_execSql(db, s.c_str());
}

int dbManager::ddl_renameTable(database* db, const std::string& oldName,
                               const std::string& dbSqlName,
                               const std::string& oldSqlName,
                               const std::string& newSqlName)
{
    std::string cmd = "rename table `" + dbSqlName + "`.`" + oldSqlName +
                      "` to `" + dbSqlName + "`.`" + newSqlName + "`";
    smartDbsReopen reopen(db->thd(), m_dbs);
    return ddl_execSql(db, cmd);
}

int dbManager::ddl_replaceTable(database* db, const std::string& name1,
                                const std::string& name2,
                                const std::string& dbSqlName,
                                const std::string& nameSql1,
                                const std::string& nameSql2)
{ // rename name1 to name2.
    char nameSql3[255];

    time_t timer_ = time(NULL);
    struct tm* t = localtime(&timer_);
    sprintf(nameSql3, "%s_trsctd_%4d%02d%02d_%02d%02d%02d", nameSql1.c_str(),
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
            t->tm_sec);


    std::string cmd = "rename table `" + dbSqlName + "`.`" + nameSql2 +
                      "` to `" + dbSqlName + "`.`" + nameSql3 + "`,`" +
                      dbSqlName + "`.`" + nameSql1 + "` to `" + dbSqlName +
                      "`.`" + nameSql2 + "`";
    smartDbsReopen reopen(db->thd(), m_dbs);
    int ret = ddl_execSql(db, cmd);
    if (ret == 0)
    {
        std::string cmd = "drop table `" + dbSqlName + "`.`" + nameSql3 + "`";
        return ddl_execSql(db, cmd);
    }
    return ret;
}

int dbManager::ddl_tableComment(database* db, const std::string& tbname,
                         const char* comment)
{
    std::string s = "alter table `" + db->name() + "`.`" + tbname +
                    "` comment \"" + comment + "\"";
    smartDbsReopen reopen(db->thd(), m_dbs);
    return ddl_execSql(db, s.c_str());
}

void dbManager::clenupNoException()
{
    try
    {
        if (m_tb)
            m_tb->unUse();
    }
    catch (...)
    {
    }
}

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs
