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
#include <my_config.h>
#include "database.h"
#include "IReadRecords.h"
#include <boost/bind.hpp>
#include "percentageKey.h"
#include "mydebuglog.h"
#include "mysqlThd.h"
#include "mysqlProtocol.h"
#include "bookmark.h"
#include <bzs/rtl/stl_uty.h>
#include <boost/shared_array.hpp>
#include <bzs/db/protocol/tdap/mysql/databaseSchema.h>


extern unsigned int g_timestamp_always;

/* implemnts in mysqlProtocol.cpp */
extern short getBinlogPos(THD* thd, binlogPos* pos, THD* tmpThd, bzs::db::IblobBuffer* bb);


namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

using namespace std;

#define KEYLEN_ALLCOPY 0


#if (MODE_READ_ONLY != TD_OPEN_READONLY)
#error "MODE_READ_ONLY != TD_OPEN_READONLY"
#endif

#if (MODE_EXCLUSIVE != TD_OPEN_EXCLUSIVE)
#error "MODE_EXCLUSIVE != TD_OPEN_EXCLUSIVE"
#endif

#if (MODE_READ_EXCLUSIVE != TD_OPEN_READONLY_EXCLUSIVE)
#error "MODE_READ_EXCLUSIVE != TD_OPEN_READONLY_EXCLUSIVE"
#endif


unsigned int hash(const char* s, size_t len)
{
    unsigned int h = 0;
    for (size_t i = 0; i < len; i++)
        h = h * 137 + *(s + i);
    return h % 1987;
}

tableCacheCounter::tableCacheCounter()
{
}

int tableCacheCounter::getHash(const std::string& dbname,
                               const std::string& tbname)
{
    char tmp[256];
    sprintf_s(tmp, 256, "%s%s", dbname.c_str(), tbname.c_str());
    return hash(tmp, strlen(tmp));
}

size_t tableCacheCounter::getCounterIndex(const std::string& dbname,
                                          const std::string& tbname)
{
    int h = getHash(dbname, tbname);
    vector<int>::iterator pos = find(m_tables.begin(), m_tables.end(), h);
    if (pos == m_tables.end())
    {
        m_tables.push_back(h);
        m_counts.push_back(0);
        return m_counts.size() - 1;
    }
    return pos - m_tables.begin();
}

void tableCacheCounter::addref(const std::string& dbname,
                               const std::string& tbname)
{

    size_t pos = getCounterIndex(dbname, tbname);
    ++m_counts[pos];
}

int tableCacheCounter::count(const std::string& dbname,
                             const std::string& tbname)
{
    boost::mutex::scoped_lock lck(m_mutex);
    size_t pos = getCounterIndex(dbname, tbname);
    return m_counts[pos];
}

void tableCacheCounter::release(const std::string& dbname,
                                const std::string& tbname)
{
    size_t pos = getCounterIndex(dbname, tbname);
    --m_counts[pos];
}

bool lockTable(THD* thd, TABLE* tb)
{
    bool append = (thd->lock != 0);
#ifdef MARIADB_10_1
    thd->variables.option_bits |= OPTION_TABLE_LOCK;
#endif
    MYSQL_LOCK* lock = mysql_lock_tables(thd, &tb, 1, 0);
    if (!append)
        thd->lock = lock;
    else if (lock)
    {
        MYSQL_LOCK* lockMrg = mysql_lock_merge(thd->lock, lock);
        if (lockMrg)
            thd->lock = lockMrg;
    }
    DEBUG_WRITELOG_SP1("LOCK TABLE table =%s\n", tb->s->table_name.str);

    return (lock != NULL);
}

/** The present lock type is returned.
 *  The true lock type of innodb is controlled by m_thd->lex->sql_command.
 */
thr_lock_type locktype(bool trn, enum_sql_command cmd)
{
    if (trn)
        return TL_WRITE;
    thr_lock_type lock_type = TL_READ;
    switch (cmd)
    {
    case SQLCOM_INSERT:
    case SQLCOM_DELETE:
    case SQLCOM_UPDATE:
    case SQLCOM_DROP_TABLE:
    case SQLCOM_CREATE_TABLE:
    case SQLCOM_CREATE_INDEX:
        lock_type = TL_WRITE;
    default:
        break;
    }
    return lock_type;
}

bool unlockTables(bool releaseStatementLock, THD* thd, bool rollback, database::tableList* tables)
{
    if (thd->lock)
    {
        bool ret;
        if (rollback)
            ret = trans_rollback_stmt(thd);
        else
            ret = trans_commit_stmt(thd);
        if (releaseStatementLock)
            thd->mdl_context.release_statement_locks();
        if (tables)
        {
            for (size_t i=0;i<tables->size();++i)
                mysql_lock_remove(thd, thd->lock, (*tables)[i]->internalTable());
        }else
        {
            mysql_unlock_tables(thd, thd->lock);
            thd->lock = 0;
        }
        return !ret;
    }
    return false;
}

#ifdef _MSC_VER
#pragma warning(disable : 4355)
#endif

tableCacheCounter database::tableRef;

database::database(const char* name, short cid)
    : m_dbname(name), m_thd(createThdForThread()),m_inAutoTransaction(NULL), 
       m_privilege(0xFFFF), m_inTransaction(0), m_inSnapshot(0), m_stat(0),
       m_usingExclusive(false), m_trnType(0), m_cid(cid), m_inprocessSnapshot(false)
{
    cp_security_ctx(m_thd)->skip_grants();
    
    //backup current sctx
    m_backup_sctx = cp_security_ctx(m_thd);
    setDbName(m_thd, m_dbname.c_str());
}

#ifdef _MSC_VER
#pragma warning(default : 4355)
#endif

database::~database()
{
    use();
    unUseTables(true/*rollback*/);
    closeForReopen();
    {
        boost::mutex::scoped_lock lck(tableRef.mutex());
        for (size_t i = 0 ; i < m_tables.size(); ++i)
        {
            if (m_tables[i])
                database::tableRef.release(name(), m_tables[i]->m_name);
        }
        m_tables.clear(); // It clears ahead of the destructor of m_trn.
    }
    deleteThdForThread(m_thd);
}

unsigned char* database::getUserSha1Passwd(const char* host, const char* user,
                                            unsigned char* buf)
{
    table* tb2 = NULL;
    table* tb = NULL;
    unsigned char* retPtr = NULL;
    std::string dbname = m_dbname;
    m_dbname = "mysql";
    try
    {
        use();
        tb2 = openTable("user", TD_OPEN_READONLY, "", "");
        if (tb2)
        {
	        tb = useTable(tb2->id(), SQLCOM_SELECT, NULL);
	        if (tb)
	        {
	            tb->setKeyNum((char)0);
	            std::vector<std::string> keyValues;
	            keyValues.push_back(host);
	            keyValues.push_back(user);
	            tb->setKeyValues(keyValues, -1, NULL);
	            tb->seekKey(HA_READ_KEY_EXACT, tb->keymap());
	            if (tb->stat() == 0)
	            {
	                int size;
	                const char* p =  tb->valStr(MYSQL_USER_FIELD_PASSWORD, size);
	                if (strlen(p))
	                {
	                    get_salt_from_password(buf, p);
	                    retPtr = buf;
	                }
	            }
	            tb->unUse();
	        }
            closeTable(tb2);
        }
        m_dbname = dbname;
        return retPtr;
    }
    catch (...)
    {
    }
    if (tb)
        tb->unUse();
    if (tb2)
        closeTable(tb2);
    m_dbname = dbname;
    return retPtr;
}

int database::findSecurityCtxs(const std::string& dbname)
{
    for (size_t i=0;i < m_securityCtxs.size(); ++i)
        if (m_securityCtxs[i].db == dbname) return (int)i;
    return -1;
}

void database::addDbName(const std::string& dbname)
{
    m_securityCtxs.push_back(sec_db(dbname));
    sec_db* secx = &m_securityCtxs[m_securityCtxs.size() -1];
    
    // Change security_ctx
    secx->security_ctx.restore_security_context(m_thd, &secx->security_ctx);

    //Get Grant
	if (m_backup_sctx->cp_master_accsess() == (ulong)~NO_ACCESS)
		cp_security_ctx(m_thd)->skip_grants();
	else
	{
		const char* user = m_backup_sctx->cp_priv_user();
		const char* host = m_backup_sctx->cp_priv_host();
		bool ret = ::setGrant(m_thd, host ? host : "", user ? user : "", dbname.c_str());
		if (ret)
			check_access(m_thd, SELECT_ACL, dbname.c_str(), &secx->privilege, NULL, false, true);
		else
			secx->privilege = 0;
	}
	//Restore current ctx
    restoreSctx();
}

// true ok false fail
bool database::setGrant(const char* host, const char* user, const char* dbname)
{
	bool ret = ::setGrant(m_thd, host, user, dbname);
    if (ret)
    {
        check_access(m_thd, SELECT_ACL, dbname, &m_privilege, NULL, false, true);
    }
    return ret;
}

void database::restoreSctx()
{
    m_backup_sctx->restore_security_context(m_thd, m_backup_sctx);
}

void database::changeSctx(int index)
{
    m_backup_sctx->restore_security_context(m_thd, &m_securityCtxs[index].security_ctx);
}

// for mysql database only
short database::aclReload()
{
    if (name() != "mysql")
        return STATUS_ACCESS_DENIED;
    if(!(m_privilege & GRANT_ACL))
        return STATUS_ACCESS_DENIED;
    short ret = STATUS_SUCCESS;

    THD* thdCur = _current_thd();
    try
    {
        boost::shared_ptr<THD> thd(createThdForThread(), deleteThdForThread);
        attachThd(thd.get());
        acl_reload(thd.get());
    }
    catch (...)
    {
        ret = 1;
    }
    attachThd(thdCur);
    return ret;
}

void database::use() const
{
    attachThd(m_thd);
    m_thd->clear_error();
}

void database::prebuildIsoratinMode()
{
    if (m_thd->lock == 0)
    {
        cp_thd_set_read_only(m_thd, m_inSnapshot != 0);
        if (m_inTransaction)
            m_thd->tx_isolation = m_iso;
        else if(m_inSnapshot)
        {
            if (m_iso)
                m_thd->tx_isolation = m_iso;
            else
                m_thd->tx_isolation = ISO_REPEATABLE_READ;
        }
        else
            m_thd->tx_isolation = (enum_tx_isolation)m_thd->variables.tx_isolation;
    }
    if (m_inTransaction || (m_inSnapshot && m_iso))
        m_thd->in_lock_tables = 1;// WITH LOCK
}

void database::prebuildExclusieLockMode(table* tb)
{
    m_thd->variables.option_bits |= OPTION_TABLE_LOCK;
    m_thd->lex->sql_command = SQLCOM_LOCK_TABLES;
    if (tb->mode() == MODE_EXCLUSIVE)
        tb->m_table->reginfo.lock_type = TL_WRITE;
    else
        tb->m_table->reginfo.lock_type = TL_READ_NO_INSERT;
    m_thd->in_lock_tables = 1;
}

void database::prebuildLocktype(table* tb, enum_sql_command& cmd, rowLockMode* lck) 
{
    bool trn = ((m_inTransaction > 0) && !tb->isReadOnly());
    if (m_inSnapshot) 
        cmd = SQLCOM_SELECT;
    thr_lock_type lock_type = TL_READ; 

    // ExclusveMode and Snapshot can not specify lock type.
    // Auto transaction can.
    if (lck && lck->lock)
    {   
        if (m_inTransaction)
            lock_type = (lck->read) ? TL_READ : TL_WRITE;
        else if (!tb->isExclusveMode() && noUserTransaction()) 
        {
            if (lck->read)
                THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_LOCKTYPE, "Invalid lock type.");
                
            assert(cmd == SQLCOM_SELECT);
            lock_type = TL_WRITE;
            cmd = SQLCOM_UPDATE;
        }
    }
    else
        lock_type = locktype(trn, cmd); 
    m_thd->in_lock_tables = (lock_type >= TL_WRITE);
    m_thd->lex->sql_command = cmd;

    if ((lock_type >= TL_WRITE) &&
             (tb->isReadOnly() || cp_thd_get_global_read_only(m_thd)))
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ACCESS_DENIED, "Access denined.");

    if ((lock_type >= TL_WRITE) &&
        (m_thd->variables.sql_log_bin))
        m_thd->set_current_stmt_binlog_format_row();
 
    tb->m_table->reginfo.lock_type = lock_type;
}

void database::changeIntentionLock(table* tb, thr_lock_type lock_type)
{  
    if (lock_type != tb->m_table->reginfo.lock_type)
    {
        tb->m_table->reginfo.lock_type = lock_type;
        m_thd->variables.option_bits |= OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN;
        tb->m_table->file->ha_external_lock(m_thd, F_UNLCK);
        if (lock_type == TL_READ)
            tb->m_table->file->init_table_handle_for_HANDLER();//prebuilt->select_lock_type = LOCK_NONE;
        m_thd->in_lock_tables = 1;
        tb->m_table->file->ha_external_lock(m_thd, 
                (lock_type == TL_WRITE) ? F_WRLCK : F_RDLCK);
        
        if  (m_iso == ISO_READ_COMMITTED)
        {
            if (lock_type == TL_READ)
            {
                THR_LOCK_DATA* to[2] = {NULL};
                tb->m_table->file->store_lock(m_thd, to, lock_type);
            }
        }
        // For call build_tmplate()
        tb->m_table->file->ha_index_or_rnd_end();
        if (tb->keyNum() >= 0)
            tb->m_table->file->ha_index_init(tb->keyNum(), true);
        else
            tb->m_table->file->ha_rnd_init(true);
            
        m_thd->variables.option_bits &= ~(OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN);
        tb->m_validCursor = false;
        m_thd->in_lock_tables = 0;
    }
}

/*
How to set the lock value to InnoDB prebuilt->select_lock_type variable.

-- First call
LOCK_NONE : file->init_table_handle_for_HANDLER();
LOCK_S    : m_table->reginfo.lock_type = TL_READ and 
            thd->in_lock_tables = 1;
LOCK_X    : m_table->reginfo.lock_type = TL_WRITE and
            thd->in_lock_tables = 1;
-- Chage type
LOCK_X -> LOCK_S
          : thd->variables.option_bits |= OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN;
            file->ha_external_lock(m_thd, F_UNLCK);
            file->init_table_handle_for_HANDLER();
            file->ha_external_lock(m_thd, F_WRLCK);
            thd->variables.option_bits &= ~(OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN);

LOCK_S -> LOCK_X
          : thd->variables.option_bits |= OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN;
            file->ha_external_lock(m_thd, F_UNLCK);
            file->ha_external_lock(m_thd, F_RDLCK);
            thd->variables.option_bits &= ~(OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN);
*/
table* database::useTable(int index, enum_sql_command cmd, rowLockMode* lck)
{
    if (index >= (int)m_tables.size())
        THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid table id.");
                                     
    table* tb = m_tables[index].get();
    if (tb == NULL)
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_FILE_NOT_OPENED,
                                     "Invalid table id.");
    if (tb->m_blobBuffer && tb->blobFields() && !m_inprocessSnapshot)
        tb->m_blobBuffer->clear();

    // Change to shared lock is user tranasction only.
    if (lck && lck->read && lck->lock && !m_inTransaction)
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_LOCKTYPE,
                                         "Invalid lock type."); 
    
    // in-transaction or in-snapshort or exclusive or inAutoTransaction(lock delay)
    //  is opened
    if (tb->islocked())
    {
        if (tb->m_table == NULL)
            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_FILE_NOT_OPENED,
                                         "Invalid table id.");
        if (m_inTransaction && (m_iso >= ISO_READ_COMMITTED))
            changeIntentionLock(tb, (lck && lck->lock && lck->read) ? TL_READ : TL_WRITE);

        if (tb->isExclusveMode())
        {
            prebuildLocktype(tb, cmd, NULL);
            tb->startStmt();
        }
        return tb;
    }

    tb->checkACL(cmd);
    //checkACL(cmd);
    prebuildLocktype(tb, cmd, lck);
    prebuildIsoratinMode();

    if (tb->isExclusveMode())
        prebuildExclusieLockMode(tb);
    
    if (!lockTable(m_thd, tb->m_table))
    {
        m_thd->in_lock_tables = 0;
        m_thd->variables.option_bits &= ~OPTION_TABLE_LOCK;
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_CANNOT_LOCK_TABLE,
                                         "lockTable error.");
    }
    if (tb->isExclusveMode())
        m_thd->variables.option_bits &= ~OPTION_TABLE_LOCK;
    
    tb->initForHANDLER();
    tb->setLocked(true);
    m_thd->in_lock_tables = 0;
    return tb;
}

size_t database::getNomalOpenTables(tableList& tables)
{
    for (int i = (int)m_tables.size() - 1; i >= 0; i--)
    {
        boost::shared_ptr<table>& tb = m_tables[i];
        if (tb && tb->islocked() && tb->isNomalMode())
            tables.push_back(tb);
    }
    return tables.size();
}

void database::unUseTable(table* tb)
{
    if (tb->islocked() && (m_inTransaction + m_inSnapshot == 0))
    { // Only this table is lock release.
        bool needUnlock =
            (locktype(false, m_thd->lex->sql_command) == TL_WRITE);
        bool rollback = (!tb->isChanged() && needUnlock);
        bool ret = true;
        if(m_usingExclusive)
        {
            tableList tables;
            if (getNomalOpenTables(tables))
                ret = unlockTables(needUnlock, m_thd, rollback, &tables);
        }
        else
            ret = unlockTables(needUnlock, m_thd, rollback, NULL);

        if (ret)
        {
            tb->resetTransctionInfo(m_thd);
            //unlock whole tebles, 
            //  but a table can not know unlocked if a table in-autoTransaction
            if (m_inAutoTransaction)
            {
                if (tb != m_inAutoTransaction)
                    m_inAutoTransaction->resetTransctionInfo(m_thd);
                m_inAutoTransaction = NULL;
            }
            DEBUG_WRITELOG_SP1("UNLOCK TABLE table =%s\n",
                               tb->m_table->s->table_name.str);
        }
        else
        {
            DEBUG_WRITELOG_SP1("UNLOCK TABLE ERROR table =%s\n",
                               tb->m_table->s->table_name.str);
            if (m_thd->is_error())
            {

                if (ER_LOCK_WAIT_TIMEOUT == m_thd->cp_get_sql_error())
                    m_stat = STATUS_CANNOT_LOCK_TABLE;
                else
                    m_stat = m_thd->cp_get_sql_error();
                THROW_BZS_ERROR_WITH_CODEMSG(m_stat,
                                             "Transaction commit error.");
            }
        }
    }
}

void database::unUseTables(bool rollback)
{
    // All the table lock release
    bool needUnlock =
        (locktype((m_inTransaction > 0), m_thd->lex->sql_command) == TL_WRITE);
    m_inTransaction = 0;
    m_inSnapshot = 0;
    m_inAutoTransaction = NULL;
       
    bool ret = true;
    if(m_usingExclusive)
    {
        tableList tables;
        if (getNomalOpenTables(tables))
            ret = unlockTables(needUnlock, m_thd, rollback, &tables);
    }else
        ret = unlockTables(needUnlock, m_thd, rollback, NULL);

    for (int i = 0; i < (int)m_tables.size(); i++)
    {
        if (m_tables[i])
            m_tables[i]->resetTransctionInfo(m_thd);
    }

    if (!ret)
    {
        if (m_thd->is_error())
        {
            DEBUG_WRITELOG("UNLOCK TABLES ERROR \n");
            if (ER_LOCK_WAIT_TIMEOUT == m_thd->cp_get_sql_error())
                m_stat = STATUS_CANNOT_LOCK_TABLE;
            else
                m_stat = m_thd->cp_get_sql_error();
            THROW_BZS_ERROR_WITH_CODEMSG(m_stat, "Transaction commit error.");
        }
    }else
    {
        ;
        DEBUG_WRITELOG("UNLOCK TABLES \n")
    }
}

bool database::beginTrn(short type, enum_tx_isolation iso)
{
    if (m_inSnapshot)
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ALREADY_INSNAPSHOT, "Snapshot is already beginning.");        
    
    bool ret = false;
    m_stat = STATUS_SUCCESS;
    if (m_inTransaction == 0)
    {
        m_trnType = type;
        // if ISO_REPEATABLE_READ ,change to force ISO_SERIALIZABLE 
        if (iso == ISO_REPEATABLE_READ)
            m_iso = ISO_SERIALIZABLE; 
        else
            m_iso = iso;
        if (m_inAutoTransaction)
            m_inAutoTransaction->unUse();
        ret = true;
    }
    ++m_inTransaction;
    if (m_thd->variables.sql_log_bin && (m_inTransaction==1))
    {
        for (int i = 0; i < (int)m_tables.size(); ++i)
        {
            if (m_tables[i])
                useTable(i, SQLCOM_SELECT, NULL);
        }
    }    
    return ret;
}

bool database::commitTrn()
{
    m_stat = STATUS_SUCCESS;
    if (m_inTransaction > 0)
    {
        --m_inTransaction;
        if (m_inTransaction == 0)
            unUseTables(false);
    }
    return (m_inTransaction == 0);
}

bool database::abortTrn()
{
    m_stat = STATUS_SUCCESS;
    if (m_inTransaction > 0)
    {
        --m_inTransaction;
        if (m_inTransaction == 0)
            unUseTables(true);
    }
    return (m_inTransaction == 0);
}

void database::useAllTables()
{
    for (int i = 0; i < (int)m_tables.size(); ++i)
    {
        if (m_tables[i])
            useTable(i, SQLCOM_SELECT, NULL);
    }
}

bool database::beginSnapshot(enum_tx_isolation iso, binlogPos* bpos, THD* tmpThd, IblobBuffer* bb)
{
    if (m_inTransaction)
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ALREADY_INTRANSACTION, "Transaction is already beginning.");        
    
    bool ret = false;
    m_stat = STATUS_SUCCESS;
    if (m_inSnapshot == 0)
    {
        m_iso = iso;
        if (m_inAutoTransaction)
            m_inAutoTransaction->unUse();
        ret = true;
    }
    ++m_inSnapshot;
    if (m_inSnapshot==1)
    {
        if (bpos)
        {
            safe_commit_lock commit(m_thd);
            if (!commit.lock())
            {
                m_stat = STATUS_LOCK_ERROR;
                return false;
            }
            bb->clear();
            
            m_stat = getBinlogPos(m_thd, bpos, tmpThd, bb);
            
            if (m_stat) return false;
            m_inprocessSnapshot = true;
            useAllTables(); // execute scope in safe_commit_lock 
            m_inprocessSnapshot = false;
        }else
            useAllTables();
    }  
    return ret;
}

bool database::endSnapshot()
{
    m_stat = STATUS_SUCCESS;
    if (m_inSnapshot > 0)
    {
        --m_inSnapshot;
        if (m_inSnapshot == 0)
            unUseTables(false);
    }
    return (m_inSnapshot == 0);
}

/*bool database::existsTable(const char* tablename)
{
    char path[FN_REFLEN + 1];
    enum legacy_db_type not_used;
    build_table_filename(path, sizeof(path) - 1, name().c_str(), tablename, "", 0);
    frm_type_enum ftype = dd_frm_type(thd(), (char*)path, &not_used);
    return FRMTYPE_TABLE == ftype;
}*/

/** Metadata lock, a table name is case-sensitive
 *  However, in actual opening, it is not distinguished at Windows.
 */
TABLE* database::doOpenTable(const std::string& name, short mode,
                             const char* ownerName, string& dbname)
{
    m_thd->variables.lock_wait_timeout = OPEN_TABLE_TIMEOUT_SEC;
    TABLE_LIST tables;
    tables.init_one_table(dbname.c_str(), dbname.size(), name.c_str(),
		            name.size(), name.c_str(), TL_READ);
    long want_access = SELECT_ACL;//IS_MODE_READONLY(mode) ? SELECT_ACL : SELECT_ACL|INSERT_ACL|UPDATE_ACL|DELETE_ACL; 
    if(!(m_privilege & want_access) &&
        (check_grant(m_thd, want_access, &tables, FALSE, 1, true)))
    {
        m_stat = STATUS_ACCESS_DENIED;
        return NULL;
    }
	cp_set_mdl_request_types(tables, mode);

    Open_table_context ot_act(m_thd, OPEN_TABLE_FLAG_TYPE);
    m_thd->cp_set_overwrite_status(true);
	if (cp_open_table(m_thd, &tables, &ot_act))
    {
        m_stat = STATUS_TABLE_NOTOPEN;
        if (ER_LOCK_WAIT_TIMEOUT == m_thd->cp_get_sql_error())
            m_stat = STATUS_CANNOT_LOCK_TABLE;
        else
        {
            m_stat = m_thd->cp_get_sql_error();
            cp_open_error_release(m_thd, tables);
        }
        return NULL;
    }
	cp_set_transaction_duration_for_all_locks(m_thd);
    
    // Check owner name
    if (ownerName && ownerName[0])
    {
        const char* p = tables.table->s->comment.str;
        if ((p[0] == '%') && (p[1] == '@') && (p[2] == '%'))
        {
            int readNoNeed = p[3] - '0';
            if ((mode == TD_OPEN_READONLY) && readNoNeed)
                ;
            else if (strcmp(p + 4, ownerName))
            {
                m_stat = STATUS_INVALID_OWNERNAME;
                return NULL;
            }
        }
    }

    tables.table->use_all_columns();
    tables.table->open_by_handler = 1;
#if (defined(DEBUG) && defined(WIN32))
    char buf[10];
    sprintf(buf, "%d", tables.table->field[1]->key_length());
    OutputDebugString(buf);
#endif
    m_stat = STATUS_SUCCESS;
    return tables.table;
}

table* database::openTable(const std::string& name, short mode,
                           const char* ownerName, std::string dbname)
{
    if (m_thd->variables.sql_log_bin && m_inTransaction)
    {
        m_stat = STATUS_ALREADY_INTRANSACTION;
        return NULL;
    }
    bool mysql_null = IS_MODE_MYSQL_NULL(mode);
    if (mysql_null)
        mode -= TD_OPEN_MASK_MYSQL_NULL;

    int index = -1;   
    if (dbname == "")
    {
        dbname = m_dbname;
        restoreSctx();
    }
    else if (dbname == m_dbname)
        restoreSctx();
    else
    {
        index =  findSecurityCtxs(dbname);
        if (index == -1)
            addDbName(dbname);
        index =  (int)m_securityCtxs.size() - 1;
        assert(index >= 0);
        changeSctx(index);
    }

    TABLE* t = doOpenTable(name, mode, ownerName, dbname);
    if (t)
    {
        boost::shared_ptr<table> tb(
            new table(t, *this, name, mode, (int)m_tables.size(), mysql_null));
        {
            boost::mutex::scoped_lock lck(tableRef.mutex());
            m_tables.push_back(tb);
            tableRef.addref(dbname, name); // addef first then table open.
        }
        tb->m_dbname = dbname;
        tb->m_privilege = (index == -1) ? this->m_privilege : m_securityCtxs[index].privilege;
        m_stat = STATUS_SUCCESS;
        if (tb->isExclusveMode())
            ++m_usingExclusive;
        
        if (index != -1) restoreSctx();
        return tb.get();
    }
    if (index != -1) restoreSctx();
    return NULL;
}

void database::closeTable(const std::string& name, bool drop)
{
    for (int i = (int)m_tables.size() - 1; i >= 0; i--)
    {
        if (m_tables[i] && (m_tables[i]->m_name == name))
        {
            closeTable(m_tables[i].get());
            return;
        }
    }
}

int tableUseCount(const std::vector<boost::shared_ptr<table> >& tables,
                  const char* name)
{
    int ret = 0;
    for (int i = (int)tables.size() - 1; i >= 0; i--)
    {
        if (tables[i] && (tables[i]->name() == name))
            ++ret;
    }
    return ret;
}

void database::closeTable(table* tb)
{
    if (m_inSnapshot)
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_ALREADY_INSNAPSHOT, "Snapshot is already beginning."); 
    for (int i = (int)m_tables.size() - 1; i >= 0; i--)
    {
        if (m_tables[i] && (m_tables[i].get() == tb))
        {
            if (tb->isExclusveMode())
            {
                --m_usingExclusive;
                // Chenge to normal, in order to be listed on the normal list.
                struct ChangeMode
                {
                    table* m_tb;
                    short m_mode;
                    ChangeMode(table* tb):m_tb(tb)
                    {
                        m_mode = tb->m_mode;
                        m_tb->m_mode = 0;
                    }
                    ~ChangeMode(){m_tb->m_mode = m_mode;}
                }modeChanger(tb);

                m_tables[i]->unUse();
            }else
                m_tables[i]->unUse();
            
            m_tables[i]->m_table->file->ha_index_or_rnd_end();

            TABLE** tbl;
            for (tbl= &m_thd->open_tables; *tbl; tbl= &((*tbl)->next))
                if(*tbl == m_tables[i]->m_table)
                    break;
            if (*tbl)
                close_thread_table(m_thd, tbl);
            releaseTable(i);
            DEBUG_WRITELOG_SP1("CLOSE TABLE table id=%d \n", i);
        }
    }
}

void database::releaseTable(size_t index)
{
    boost::mutex::scoped_lock lck(database::tableRef.mutex());
    database::tableRef.release(name(), m_tables[index]->m_name);
    m_tables[index].reset();
}

void database::closeForReopen()
{
    // A transaction is committed compulsorily.
    for (size_t i = 0; i < m_tables.size(); i++)
    {
        if (m_tables[i] && (m_tables[i]->m_table != NULL))
            m_tables[i]->resetInternalTable(NULL);
    }
    trans_commit_stmt(m_thd);
    if (m_thd->mdl_context.has_locks())
        close_thread_tables(m_thd);
    m_thd->mdl_context.release_transactional_locks();
    m_usingExclusive = 0;
}

void database::reopen(bool forceReadonly)
{
    for (size_t i = 0; i < m_tables.size(); i++)
    {
        if (m_tables[i] && (m_tables[i]->m_table == NULL))
        {
            if (forceReadonly)
                m_tables[i]->m_mode = TD_OPEN_READONLY;
            TABLE* table = doOpenTable(m_tables[i]->m_name.c_str(),
                                        m_tables[i]->m_mode, NULL, 
                                        m_tables[i]->m_dbname);
            if (table)
                m_tables[i]->resetInternalTable(table);
            else
                releaseTable(i);
        }
    }
}

bool database::existsDatabase()
{
    return !check_db_dir_existence(m_dbname.c_str());
}


class autoincSetup
{
    TABLE* m_table;

public:
    autoincSetup(TABLE* table) : m_table(table)
    {
        m_table->next_number_field = m_table->found_next_number_field;
    }

    ~autoincSetup() { m_table->next_number_field = 0; }
};

/** Number of NIS fields
 */
unsigned short nisFieldNum(TABLE* tb)
{
    if (tb->s->null_fields)
    {
        int offset = 1;

        for (int i = tb->s->fields - offset; i >= 0; --i)
            if (isNisField(tb->s->field[i]->field_name) == false)
                return (unsigned short)(tb->s->fields - i - offset);
        return tb->s->fields;
    }
    return 0;
}

bool table::noKeybufResult = true;

table::table(TABLE* myTable, database& db, const std::string& name, short mode,
             int id, bool mysqlnull)
    : m_table(myTable), m_name(name), m_mode(mode), m_id(id), m_db(db),
      m_keybuf(new unsigned char[MAX_KEYLEN]),
      m_stat(0),
      m_keyconv(m_table->key_info, m_table->s->keys), m_blobBuffer(NULL), 
      m_readCount(0), m_updCount(0), m_delCount(0), m_insCount(0), m_privilege(0xFFFF),
      m_keyNum(-1), m_nonNcc(false), m_validCursor(true), m_cursor(false), 
      m_locked(false), m_changed(false), m_nounlock(false), m_bulkInserting(false),
      m_delayAutoCommit(false),m_forceConsistentRead(false), m_mysqlNull(mysqlnull),
      m_timestampAlways(g_timestamp_always != 0)
{

    m_table->read_set = &m_table->s->all_set;

    m_recordFormatType = RF_VALIABLE_LEN;
#ifdef USE_BTRV_VARIABLE_LEN
    m_lastVarLenBytes = 0;
#endif
    // Is the Nis field included or not?
    m_nisNullFields = nisFieldNum(m_table);

    if (m_table->s->varchar_fields + m_table->s->blob_fields == 0)
        m_recordFormatType = RF_FIXED_LEN;
#ifdef USE_BTRV_VARIABLE_LEN
    else if (m_table->s->varchar_fields == 1)
    {
        Field** fd = m_table->field + lastVarFieldNum();
        if (isVarType((*fd)->type()) && ((*fd)->part_of_key.is_clear_all()) &&
            ((*fd)->key_start.is_clear_all()) &&
            (((*fd)->charset()) == &my_charset_bin))
        {
            m_recordFormatType = RF_FIXED_PLUS_VALIABLE_LEN;

            /* The number of bytes of the length area of the last VAR field */
            m_lastVarLenBytes = lastVarFiled()->field_length < 256 ? 1 : 2;
        }
    }
#endif
    if (m_nisNullFields)
        m_recordFormatType |= RF_INCLUDE_NIS;

    // Cache null fields ,timestamp fields and  no null mode null fields lists
    int nullbits = 0;
    for (int i = 0; i < (int)m_table->s->fields; ++i)
    {
        Field* fd = m_table->field[i];
        if (fd->null_bit)
        {
            if (m_mysqlNull)
                ++nullbits;
            else if (fd->part_of_key.is_clear_all())
                m_noNullModeNullFieldList.push_back(fd);
        }

        if (fd->unireg_check == Field::TIMESTAMP_UN_FIELD ||
                fd->unireg_check == Field::TIMESTAMP_DNUN_FIELD ||
                fd->unireg_check == Field::TIMESTAMP_DN_FIELD)
            m_timeStampFields.push_back(fd);
    }
    m_nullBytesCl = (nullbits + 7) / 8; 

#ifdef USE_BTRV_VARIABLE_LEN
    m_recordLenCl = (uint)(m_table->s->reclength - m_table->s->null_bytes -
                           m_nisNullFields - m_lastVarLenBytes);
#else
    m_recordLenCl =
        (uint)(m_table->s->reclength - m_table->s->null_bytes - m_nisNullFields);
#endif
    
#if (MYSQL_VERSION_ID < 50600)
    m_table->timestamp_field_type = TIMESTAMP_NO_AUTO_SET;
#endif

}

table::~table()
{
    resetInternalTable(NULL);
    for (size_t i = 0; i < preparedStatements.size(); ++i)
        preparedStatements[i]->release();
}

void table::resetTransctionInfo(THD* thd)
{
    if (m_table)
    {
        if (m_changed)
            query_cache_invalidate3(thd, m_table, 1);
        m_changed = false;
        m_table->next_number_field = 0;
        m_table->file->next_insert_id = 0;
    }
    if (isNomalMode())
        m_locked = false;
    m_validCursor = false;
    m_nounlock = false;
}

void table::resetInternalTable(TABLE* table)
{
    if (table == NULL)
    {
        if (m_table)
            m_table->file->ha_index_or_rnd_end();
        m_table = NULL;
    }
    else
    {
        m_table = table;
        m_table->read_set = &m_table->s->all_set;
        m_locked = false;
        m_changed = false;
        m_validCursor = false;
        m_nounlock = false;
        m_keyconv.setKey(m_table->key_info);
    }
}

void table::checkACL(enum_sql_command cmd)
{
    switch(cmd)
    {
    case SQLCOM_UPDATE: 
        if (!(m_privilege & UPDATE_ACL)) THROW_BZS_ERROR_WITH_CODE(STATUS_ACCESS_DENIED); 
        break;             
    case SQLCOM_INSERT: 
        if (!(m_privilege & INSERT_ACL)) THROW_BZS_ERROR_WITH_CODE(STATUS_ACCESS_DENIED); 
        break;             
    case SQLCOM_DELETE: 
        if (!(m_privilege & DELETE_ACL)) THROW_BZS_ERROR_WITH_CODE(STATUS_ACCESS_DENIED); 
        break;
    default:
        break;
    }
}

bool table::setNonKey(bool scan)
{
    if (m_keyNum != -2)
    {
        m_table->file->ha_index_or_rnd_end();
        int ret = m_table->file->ha_rnd_init(scan);
        if (ret)
            THROW_BZS_ERROR_WITH_CODEMSG(ERROR_INDEX_RND_INIT,
                                         "setNonKey rnd_init error.");
        m_keyNum = -2;
    }
    return true;
}

bool table::setKeyNum(char num, bool sorted)
{
    if (num < 0)
    {
        m_stat = STATUS_INVALID_KEYNUM;
        return false;
    }
    if ((m_keyNum != num) ||
        ((m_keyNum >= 0) && (m_table->file->inited == handler::NONE)))
    {
        m_table->file->ha_index_or_rnd_end();
        if (!keynumCheck(num))
        {
            m_stat = STATUS_INVALID_KEYNUM;
            return false;
        }
        m_keyNum = num;
        m_table->file->ha_index_init(m_keyNum, sorted);
    }
    return true;
}

void table::fillNull(uchar* ptr, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (ptr[i] == 0)
        {
            memset(ptr + i, 0, size - i);
            break;
        }
    }
}

void table::setKeyValues(const uchar* ptr, int size)
{
    KEY& key = m_table->key_info[(int)m_keyNum];
    memcpy(&m_keybuf[0], ptr, std::min(MAX_KEYLEN, size));
    int pos = 0;
    for (int j = 0; j < (int)key.user_defined_key_parts; j++)
    {
        KEY_PART_INFO& seg = key.key_part[j];
        if (seg.field->type() == MYSQL_TYPE_STRING)
            fillNull(&m_keybuf[pos], seg.field->pack_length());
        pos += seg.field->pack_length();
    }
}

/**
 If null able field segment that need add null indicator byte befor segment.
 Then key buffer length equal key length + null able segments.

 The purpose of null key is non create index.
 At read operation set not null(zero) to null indicator.

 Size bytes of var and  blob field is 2 byte fixed.
 All most of key_part.length(segment lenght) is equal field.pack_length.
 But blob and prefix index is not equal pack_length.

 Client needs to make the right image except for null byte.

 @return -1: whole segment copied. 
          n: number of segments copied. 
 */
short table::setKeyValuesPacked(const uchar* ptr, int size)
{
    KEY& key = m_table->key_info[(int)m_keyNum];
    int to = 0;
    const uchar* from = ptr;
    int ret = -1;
    for (int j = 0; j < (int)key.user_defined_key_parts; j++)
    {
        KEY_PART_INFO& seg = key.key_part[j];
        if (seg.null_bit && isNisField(seg.field->field_name))
        {
            m_keybuf[to++] = 0x00; // set not null to nis field.
            m_keybuf[to++] = 0x00; // set value to nis field. 
            //continue next segment
        }
        else
        {
            if (seg.null_bit)
            {
                m_keybuf[to++] = *from;
                ++from;
            }
            unsigned short copylen = seg.length; // length = store_len - varlen
            unsigned short copyspace = copylen;
            if (seg.key_part_flag & HA_BLOB_PART ||
                seg.key_part_flag & HA_VAR_LENGTH_PART)
            {
                copylen += 2; // varlen= allways 2byte
                copyspace = copylen;
                unsigned short len = *((unsigned short*)from) + 2;
                copylen = std::min<unsigned short>(copylen, len);
                if (copylen != copyspace)
                    memset(&m_keybuf[to], 0, copyspace);
            }

            if ((from + copylen) - ptr > size)
            {
                if (ret == -1)
                    ret = j;
                // key data size is too short as whole key_parts length
                memset(&m_keybuf[to], 0, copyspace);
            }
            else
            {
                memcpy(&m_keybuf[to], from, copylen);
                from += copyspace;
            }
            to += copyspace;

            if (to >= MAX_KEYLEN)
                THROW_BZS_ERROR_WITH_CODEMSG(STATUS_KEYBUFFERTOOSMALL, "");
        }
    }
    return ret;
}

uint table::keyPackCopy(uchar* ptr)
{
    // if nokey and getbookmark operation then keynum = -2
    if (m_keyNum < 0)
        return 0;

    KEY& key = m_table->key_info[(int)m_keyNum];
    if ((key.flags & HA_NULL_PART_KEY) || (key.flags & HA_VAR_LENGTH_KEY))
    {
        int from = 0;
        uchar* to = ptr;
        for (int j = 0; j < (int)key.user_defined_key_parts; j++)
        {
            KEY_PART_INFO& seg = key.key_part[j];
            if (seg.null_bit)
                from++;
            if (seg.null_bit && isNisField(seg.field->field_name))
                from++;
            else
            {
                int copylen = seg.length;
                if (seg.key_part_flag & HA_BLOB_PART ||
                    seg.key_part_flag & HA_VAR_LENGTH_PART)
                    copylen += 2;
                memcpy(to, &m_keybuf[from], copylen);
                to += copylen;
                from += copylen;
            }
        }
        return (uint)(to - ptr);
    }
    memcpy(ptr, keybuf(), keylen());
    return keylen();
}

void* table::record() const
{
    Field* fd = m_table->field[0];
    return fd->ptr;
}

/** if offset and lastVarLenBytes() is non zero that is
  @m_recordFormatType=RF_FIXED_PLUS_VALIABLE_LEN.
    ptr is excluding null flag sgement.
 */
void table::setRecord(void* ptr, unsigned short size, int offset)
{
    m_cursor = false;
    unsigned char* p;
    /*if (m_mysqlNull && m_table->s->null_fields)
    {
        p = m_table->record[0];
        size = std::min(size, (unsigned short)recordLen());
    }
    else*/
    {
        p = (unsigned char*)record();
        size = std::min(size, (unsigned short)recordLenCl());
    }
#ifdef USE_BTRV_VARIABLE_LEN
    if (offset + size <=
        (unsigned short)m_table->s->reclength + lastVarLenBytes())
    {
#else
    if (offset + size <= (unsigned short)m_table->s->reclength)
    {

#endif

        if (size > 0)
            memcpy(p + offset, ptr, size);
    }
    else
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_DATASIZE, "");
}

inline bool isNullValue(Field* fd)
{
    if (isVarType(fd->type()))
    {
        int len = *((unsigned char*)fd->ptr);
        if (fd->field_length > 255)
            len = *((unsigned short*)fd->ptr);

        return (len == 0);
    }
    else if (isBlobType(fd->type()))
        return (0 == blob_len(fd));
    else
    {
        unsigned int k = 0;
        for (k = 0; k < fd->key_length(); k++)
            if (fd->ptr[k])
                break;

        if (k == fd->key_length())
            return true;
    }
    return false;
}

inline bool isNullNis(KEY& key, bool all)
{
    for (int j = 1; j < (int)key.user_defined_key_parts; j++)
    {
        Field* fd = key.key_part[j].field;
        if (key.key_part[j].null_bit)
        {
            bool v = isNullValue(fd);
            v ? fd->set_null() : fd->set_notnull();
            if (all && !v)
                return false;
            else if (!all && v)
                return true;
        }
    }
    if (all)
        return true;
    return false;
}

void table::setBlobFieldPointer(const bzs::db::blobHeader* hd)
{

    if (hd)
    {
        assert(hd->curRow < hd->rows);
        const blobField* f = hd->nextField;
        for (int i = 0; i < hd->fieldCount; i++)
        {
            Field* fd = m_table->field[f->fieldNum];
            int sizeByte = blob_var_bytes(fd);
            memcpy(fd->ptr, &f->size, sizeByte);
            const char* data = f->data();
            memcpy(fd->ptr + sizeByte, &data, sizeof(char*));
            f = f->next();
        }
        ++hd->curRow;
        hd->nextField = (blobField*)f;
    }
}

/** A packed data set to the record buffer.
 */
void table::setRecordFromPacked(const uchar* packedPtr, uint size,
                                const bzs::db::blobHeader* hd)
{
    const uchar* p = packedPtr;
    bool nullable = (m_mysqlNull && m_table->s->null_fields);
#ifdef USE_BTRV_VARIABLE_LEN
    if (recordFormatType() & RF_FIXED_PLUS_VALIABLE_LEN)
    {
        int varlenbyte = lastVarLenBytes();
        int varlenStartPos = lastVarFieldPos();
        setRecord((void*)p, varlenStartPos);
        int len = std::min((int)(size - varlenStartPos),
                           (int)recordLenCl() - varlenStartPos);
        if (len > 0)
        {
            setRecord(&len, varlenbyte, varlenStartPos);
            // if (len > 0)
            setRecord((void*)(p + varlenStartPos), len,
                      varlenStartPos + varlenbyte);
        }
        else if (len == 0)
            ;
        else
            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_BUFFERTOOSMALL,
                                         "setRecordFromPacked");
    }
    else if ((recordFormatType() & RF_VALIABLE_LEN) ||
             (recordFormatType() & RF_INCLUDE_NIS))
    {
#else
    if (nullable || (recordFormatType() & RF_VALIABLE_LEN) ||
        (recordFormatType() & RF_INCLUDE_NIS))
    {
#endif
        //copy null bits
        
        
        // It copies for every field.
        // But if null then not recived field image
        unsigned char* null_ptr = (unsigned char*)p;
        unsigned char null_bit = 1; 
        p += m_nullBytesCl;
        for (uint i = 0; i < m_table->s->fields; ++i)
        {
            Field* fd = m_table->field[i];
            bool isNull = false;
            if (fd->null_bit && nullable)
            {
                isNull = (*null_ptr & null_bit) != 0;
                if (isNull)
                    fd->set_null();
                else
                    fd->set_notnull();
                if (null_bit == (uchar)128)
                {
                    ++null_ptr;
                    null_bit = 1;
                }else
                    null_bit = null_bit << 1;
            }

            if (isNisField(fd->field_name))
                fd->ptr[0] = 0x00; // The Nis field is not sent from a client.
            else
            {
                if (!isNull)
                {
                    int len = fd->pack_length();
                    if (isVarType(fd->type()))
                    {
                        len = var_total_len(p, var_bytes(fd));
                        if (len > (int)fd->pack_length())
                            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_BUFFERTOOSMALL,
                                                            "setRecordFromPacked");
                    }
                    else if (size < (uint)len)
                        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_BUFFERTOOSMALL,
                                                        "setRecordFromPacked");
                    memcpy(fd->ptr, p, len);
                    p += len;
                    size -= len;
                }else
                    memset(fd->ptr, 0, fd->pack_length());
            }
        }
        if (m_table->s->blob_fields)
            setBlobFieldPointer(hd);
    }
    else
        setRecord((void*)p, size);
}

/** A record image is packed, and is copied to the specified buffer, and length
 *is returned.
 * -maxsize can be specified when recordFormatType() is RF_VALIABLE_LEN.
 * -Length is not inspected if maxsize is zero.
 * -When a buffer is too short, zero is returned to a result.
 */
uint table::recordPackCopy(char* buf, uint maxsize)
{
    char* p = buf;
    bool nullable = (m_mysqlNull && m_table->s->null_fields);
#ifdef USE_BTRV_VARIABLE_LEN
    if (recordFormatType() & RF_FIXED_PLUS_VALIABLE_LEN)
    {
        uint varLenBytes = lastVarLenBytes();
        uint varLenBytesPos = lastVarFieldPos();
        const uchar* data = (const uchar*)record();
        int len = recordLenCl();
        if (varLenBytes)
        {
            memcpy(p, data, varLenBytesPos);
            p += varLenBytesPos;
            data += varLenBytesPos;
            if (varLenBytes == 1)
                len = *data;
            else
                len = *((const unsigned short*)data);
            // In the variable length of tdap type, it returns except for
            // varsize.
            data += varLenBytes;
        }
        memcpy(p, data, len);
        p += len;
    }
    else if ((recordFormatType() & RF_VALIABLE_LEN) ||
             (recordFormatType() & RF_INCLUDE_NIS))
    {
#else
    if (nullable || (recordFormatType() & RF_VALIABLE_LEN) ||
        (recordFormatType() & RF_INCLUDE_NIS))
    {
#endif
        int blobs = 0;
        p = buf + m_nullBytesCl;
        unsigned char* null_ptr = (unsigned char*)buf;//m_table->record[0];
        *null_ptr = 0x00;
        unsigned char null_bit = 1; 
        for (uint i = 0; i < m_table->s->fields; i++)
        {
            Field* fd = m_table->field[i];
            bool isNull = false;
            if (fd->null_bit && nullable)
            {
                //copy null bit
                isNull = fd->is_null();
                if (isNull)
                {
                    if (null_bit == 1)
                        *null_ptr = 0x00;
                    *null_ptr |= null_bit;
                }
                if (null_bit == (uchar)128)
                {
                    ++null_ptr;
                    null_bit = 1;
                }else
                    null_bit = null_bit << 1;
            }
            if (isNisField(fd->field_name))
                ;
            // The Nis field is not sent to a client.
            else if (!isNull)
            {
                uint len = fd->pack_length();
                if (isVarType(fd->type()))
                    len = var_total_len(fd);
                if (maxsize && ((p - buf + len) > maxsize))
                    return 0;
                memcpy(p, fd->ptr, len);
                p += len;
            }
            if (isBlobType(fd->type()))
            {
                ++blobs;
                addBlobBuffer(fd->field_index);
            }
        }
        setBlobFieldCount(blobs);
    }
    else
    {
        const uchar* data = (const uchar*)record();
        int len = recordLenCl();
        memcpy(p, data, len);
        p += len;
    }
    return (uint)(p - buf);
}

ushort table::fieldPackCopy(unsigned char* nullPtr, int& nullbit, unsigned char* dest, short fieldNum)
{
    Field* fd = m_table->field[fieldNum];
    uint len = fd->pack_length();
    if (isVarType(fd->type()))
        len = var_total_len(fd);
#ifdef USE_BTRV_VARIABLE_LEN
    if (lastVarFieldNum() == fieldNum)
    {
        len -= lastVarLenBytes();
        memcpy(dest, fd->ptr + lastVarLenBytes(), len);
    }
    else
#endif
    {
        memcpy(dest, fd->ptr, len);
        if (m_mysqlNull && nullPtr)
        {
            if (fd->is_null())
            {
                nullPtr += (nullbit / 8);
                (*nullPtr) |= (unsigned char)(1L << (nullbit % 8));
            }
			if (fd->null_bit)
				++nullbit;
        }
    }
    return (ushort)len;
}

inline void table::tryConsistentRead(bool noConsistent)
{
    /* Don't read old version that next operation is write, or inTransaqction. */ 
    bool const_read = m_forceConsistentRead || 
                        (
                            m_db.noUserTransaction() && 
                            (m_table->reginfo.lock_type < TL_WRITE) && 
                            !noConsistent
                        ); 
    m_table->file->try_semi_consistent_read(const_read);
}

inline void table::unlockRow(bool noConsistent)
{
    if ((m_forceConsistentRead || m_db.canUnlockRow()) && m_validCursor && !m_nounlock)
        m_table->file->unlock_row();
    tryConsistentRead(noConsistent);
    m_nounlock = false;
}

/* read by key
 * A key field value is set in advance
 */
void table::seekKey(enum ha_rkey_function find_flag, key_part_map keyMap)
{
    m_nonNcc = false;
    if (keynumCheck(m_keyNum))
    {
        unlockRow(m_delayAutoCommit);
        m_stat = m_table->file->ha_index_read_map(
            m_table->record[0], &m_keybuf[0], keyMap /* keymap() */, find_flag);
        setCursorStaus();
        if (m_stat == 0)
        {
            if (find_flag != HA_READ_KEY_EXACT)
                key_copy(&m_keybuf[0], m_table->record[0],
                         &m_table->key_info[(int)m_keyNum], KEYLEN_ALLCOPY);
        }
    }
    else
        m_stat = STATUS_INVALID_KEYNUM;
    if ((m_stat == HA_ERR_KEY_NOT_FOUND) && (find_flag != HA_READ_KEY_EXACT))
        m_stat = HA_ERR_END_OF_FILE;
}

void table::moveKey(boost::function<int()> func)
{
    m_nonNcc = false;
    if (keynumCheck(m_keyNum))
    {
        unlockRow(m_delayAutoCommit);

        m_stat = func();
        setCursorStaus();
        if (m_stat == 0)
            key_copy(&m_keybuf[0], m_table->record[0],
                     &m_table->key_info[(int)m_keyNum], KEYLEN_ALLCOPY);
    }
    else
        m_stat = STATUS_INVALID_KEYNUM;
    if (m_stat == HA_ERR_KEY_NOT_FOUND)
        m_stat = HA_ERR_END_OF_FILE;
}

void table::getNextSame(key_part_map keyMap)
{
    m_nonNcc = false;
    if (keynumCheck(m_keyNum))
    {
        unlockRow(false /*lock*/);
        m_stat = m_table->file->ha_index_next_same(
            m_table->record[0], &m_keybuf[0], keyMap /* keymap() */);
        setCursorStaus();
        if (m_stat == 0)
        {
            key_copy(&m_keybuf[0], m_table->record[0],
                     &m_table->key_info[(int)m_keyNum], KEYLEN_ALLCOPY);
        }
    }
    else
        m_stat = STATUS_INVALID_KEYNUM;
}

void table::getLast()
{
    moveKey(boost::bind(&handler::ha_index_last, m_table->file,
                        m_table->record[0]));
}

void table::getFirst()
{
    moveKey(boost::bind(&handler::ha_index_first, m_table->file,
                        m_table->record[0]));

#if (defined(DEBUG) && defined(WIN32))

    char buf[1024];
    sprintf(buf, "name=%s reclength=%d rec_buff_length=%d\r\n", m_name.c_str(),
            m_table->s->reclength, m_table->s->rec_buff_length);
    OutputDebugString(buf);
    for (uint i = 0; i < m_table->s->fields; i++)
    {
        Field* fd = m_table->s->field[i];

        sprintf(buf, "\tpack_length=%d field_length=%d key_length=%d\r\n",
                fd->pack_length(), fd->field_length, fd->key_length());
        OutputDebugString(buf);
    }
#endif
}

void table::getNext()
{
    if (!m_cursor)
    {
        m_stat = HA_ERR_NO_ACTIVE_RECORD;
        return;
    }
    if (m_nonNcc)
    { // It moves to the position after updating.
        // Since it may be lost, ref is compared and the next is decided.
        movePos(position(true), m_keyNum, true);
        if (m_stat)
            return;
    }
    moveKey(boost::bind(&handler::ha_index_next, m_table->file,
                        m_table->record[0]));
}

void table::getPrev()
{
    if (!m_cursor)
    {
        m_stat = HA_ERR_NO_ACTIVE_RECORD;
        return;
    }
    if (m_nonNcc)
    {
        movePos(position(true), m_keyNum, true);
        if (m_stat)
            return;
    }
    moveKey(boost::bind(&handler::ha_index_prev, m_table->file,
                        m_table->record[0]));
}

bool table::keyCheckForPercent()
{
    if (m_keyNum == -1)
        m_keyNum = m_table->s->primary_key;
    // The value of the beginning of a key
    KEY& key = m_table->key_info[(int)m_keyNum];
    if (key.key_length > 128)
        return false;
    return true;
}

void table::preBuildPercent(uchar* first, uchar* last)
{
    KEY& key = m_table->key_info[(int)m_keyNum];
    getFirst();
    if (m_stat == 0)
    {
        key_copy(first, m_table->record[0], &key, KEYLEN_ALLCOPY);
        getLast();
        if (m_stat == 0)
        {
            key_copy(last, m_table->record[0], &key, KEYLEN_ALLCOPY);
            memset(m_keybuf.get(), 0, MAX_KEYLEN);
        }
        else
            THROW_BZS_ERROR_WITH_CODEMSG(m_stat, "connot seek last position.");
    }
    else
        THROW_BZS_ERROR_WITH_CODEMSG(m_stat, "connot seek first position.");
}

void table::getByPercentage(unsigned short per)
{
    m_nonNcc = false;
    if (!keyCheckForPercent())
    {
        m_stat = STATUS_INVALID_KEYNUM;
        return;
    }

    /* Use constistant read  force */
    smartForceConsistantRead SFCR(this);

    if (per > 9800)
    {
        getLast();
        return;
    }
    else if (per < 200)
    {
        getFirst();
        return;
    }

    uchar keybufFirst[MAX_KEYLEN] = { 0x00 };
    uchar keybufLast[MAX_KEYLEN] = { 0x00 };
    preBuildPercent(keybufFirst, keybufLast);
    if (m_stat == 0)
    {
        // 5% of position is obtained.
        uchar* st = keybufFirst;
        uchar* en = keybufLast;
        uchar* cu = (uchar*)keybuf();
        KEY& key = m_table->key_info[(int)m_keyNum];
        uint keylen = key.key_length + 10;
        boost::shared_array<uchar> stbuf(new uchar[keylen]);
        boost::shared_array<uchar> lsbuf(new uchar[keylen]);
        boost::shared_array<uchar> stbufResult(new uchar[keylen]);
        boost::shared_array<uchar> lsbufResult(new uchar[keylen]);
        percentageKey pk(key, st, en, cu);
        bool forwoard = true;
        int ov = 0;
        int sameCount = 0;
        while (1)
        {
            pk.reset(st, en, cu);
            int ret = pk.setKeyValueByPer(5000, forwoard);
            if (ret == KEY_ALL_SGMENTS_SAME)
                break; // THROW_BZS_ERROR_WITH_CODEMSG(1, "first and last record
            // are same key value.");
            else if (ret == KEY_NEED_SGMENT_COPY)
                pk.copyFirstDeferentSegment();
            int v = percentage(keybufFirst, keybufLast, cu) - 1000;
            (ov == v) ? ++sameCount : sameCount = 0;
            if (sameCount > 100)
                break;
            ov = v;
            if (v < -250)
            {
                memcpy(stbuf.get(), cu, keylen);
                st = stbuf.get();
                forwoard = true;
            }
            else if (v > 250)
            {
                memcpy(lsbuf.get(), cu, keylen);
                en = lsbuf.get();
                forwoard = false;
            }
            else
                break;
        }
        memcpy(stbufResult.get(), cu, keylen);
        if (per > 1000)
        {
            // 95% of position is obtained.
            forwoard = true;
            memcpy(cu, keybufLast, keylen);
            memcpy(lsbuf.get(), keybufLast, keylen);
            en = lsbuf.get();
            while (1)
            {
                pk.reset(st, en, cu);
                int ret = pk.setKeyValueByPer(5000, forwoard);
                if (ret == KEY_ALL_SGMENTS_SAME)
                    break;
                /// THROW_BZS_ERROR_WITH_CODEMSG(1, "connot seek percant
                /// position.");
                else if (ret == KEY_NEED_SGMENT_COPY)
                    pk.copyFirstDeferentSegment();
                int v = percentage(keybufFirst, keybufLast, cu) - 9000;
                (ov == v) ? ++sameCount : sameCount = 0;
                if (sameCount > 100)
                    break;
                ov = v;
                if (v < -250)
                {
                    memcpy(stbuf.get(), cu, keylen);
                    st = stbuf.get();
                    forwoard = true;
                }
                else if (v > 250)
                {
                    memcpy(lsbuf.get(), cu, keylen);
                    en = lsbuf.get();
                    forwoard = false;
                }
                else
                    break;
            }
            memcpy(lsbufResult.get(), cu, keylen);
            pk.reset(stbufResult.get(), lsbufResult.get(), cu);

            pk.setKeyValueByPer(per, true);
        }
        seekKey(HA_READ_KEY_OR_NEXT, keymap());
    }
}

int table::percentage(uchar* first, uchar* last, uchar* cur)
{
    KEY& key = m_table->key_info[(int)m_keyNum];
    // 1 cur to last
    key_range minkey;
    minkey.key = cur;
    minkey.length = key.key_length;
    minkey.keypart_map = keymap();
    minkey.flag = HA_READ_KEY_EXACT;
    key_range maxkey = minkey;
    maxkey.key = last;
    ha_rows rows1 = m_table->file->records_in_range(m_keyNum, &minkey, &maxkey);

    // 2 first to last
    maxkey.key = first;
    ha_rows rows2 = m_table->file->records_in_range(m_keyNum, &maxkey, &minkey);

    // 3 record count
    ha_rows total = recordCount(true);

    // result
    int v1 = 10000 - (int)(rows1 * (double)10000 / total);

    int v2 = (int)(rows2 * (double)10000 / total);
    if (abs(5000 - v1) > abs(5000 - v2))
        return v1;
    return v2;
}

void table::calcPercentage()
{
    if (!keyCheckForPercent())
    {
        m_stat = STATUS_INVALID_KEYNUM;
        return;
    }
    m_percentResult = 0;
    // The present key value is copied.
    uchar keybufCur[MAX_KEYLEN] = { 0x00 };
    key_copy(keybufCur, m_table->record[0], &m_table->key_info[(int)m_keyNum],
             KEYLEN_ALLCOPY);

    uchar keybufFirst[MAX_KEYLEN] = { 0x00 };
    uchar keybufLast[MAX_KEYLEN] = { 0x00 };
    preBuildPercent(keybufFirst, keybufLast);

    if (m_stat == 0)
    { // restore current
        setKeyValues(keybufCur, 128);
        seekKey(HA_READ_KEY_EXACT, keymap());
        if (m_stat == 0)
            m_percentResult = percentage(keybufFirst, keybufLast, keybufCur);
    }
}

void table::stepFirst()
{
    if (m_table->s->primary_key < m_table->s->keys)
    {
        setKeyNum(m_table->s->primary_key);
        getFirst();
    }
    else
    {
        m_keyNum = -1;
        if (setNonKey(true))
        {
            unlockRow(m_delayAutoCommit);
            m_stat = m_table->file->ha_rnd_next(m_table->record[0]);
            setCursorStaus();
        }
        else
            m_stat = STATUS_INVALID_KEYNUM;
    }
}

void table::stepLast()
{
    if (m_table->s->primary_key < m_table->s->keys)
    {
        setKeyNum(m_table->s->primary_key);
        getLast();
    }
    else
        m_stat = STATUS_NOSUPPORT_OP;
}

void table::stepNext()
{
    m_nonNcc = false;
    if (m_table->s->primary_key < m_table->s->keys)
    {
        setKeyNum(m_table->s->primary_key);
        getNext();
    }
    else
    {
        if (m_keyNum != -2)
        {
            if (setNonKey(false))
            {
                unlockRow(false/*lock*/);
                m_stat = m_table->file->ha_rnd_pos(m_table->record[0],
                                                   (uchar*)position(true));
                if (m_stat != 0)
                    return;
            }
        }
        unlockRow(m_delayAutoCommit);
        m_stat = m_table->file->ha_rnd_next(m_table->record[0]);
        setCursorStaus();
    }
}

void table::stepPrev()
{
    m_nonNcc = false;
    if (m_table->s->primary_key < m_table->s->keys)
    {
        setKeyNum(m_table->s->primary_key);
        getPrev();
    }
    else
        m_stat = STATUS_NOSUPPORT_OP;
}

void table::readRecords(IReadRecordsHandler* hdr, bool includeCurrent, int type,
                        bool noBookmark)
{

    if ((m_table->file->inited == handler::NONE) || !m_cursor)
    {
        m_stat = STATUS_NO_CURRENT;
        return;
    }
    m_nonNcc = false;
    int reject = (hdr->rejectCount() == 0) ? 4096 : hdr->rejectCount();
    if (reject == 0xFFFF)
        reject = -1;
    int rows = hdr->maxRows();

    // dummy bookmark , use if bobbokmark=true
    unsigned int tmp = 0;
    const uchar* bm = (unsigned char*)&tmp;

    // Is a current position read or not?
    bool read = !includeCurrent;
    bool forword = 
        (type == READ_RECORD_GETNEXT) || (type == READ_RECORD_STEPNEXT);
    while ((reject != 0) && (rows != 0))
    {
        if (read)
        {
            unlockRow(false);

            if (type == READ_RECORD_GETNEXT)
                m_stat = m_table->file->ha_index_next(m_table->record[0]);
            else if (type == READ_RECORD_GETPREV)
                m_stat = m_table->file->ha_index_prev(m_table->record[0]);
            else if (type == READ_RECORD_STEPNEXT)
                m_stat = m_table->file->ha_rnd_next(m_table->record[0]);
            else if (type == READ_RECORD_STEPPREV)
            {
                m_stat = STATUS_NOSUPPORT_OP;
                return;
            }
            setCursorStaus();
        }
        else
            read = true;

        if (m_stat)
            break;
        int ret = hdr->match(forword);
        if (ret == REC_MACTH)
        {

            if (!noBookmark)
                bm = position();
            m_stat = hdr->write(bm, posPtrLen());
            if (m_stat)
                break;
            --rows;
        }else
        {
            unlock();
            if (ret == REC_NOMACTH_NOMORE)
            {
                m_stat = STATUS_REACHED_FILTER_COND;
                return;
            }
            else
                --reject;
        }
    }
    if (reject == 0)
        m_stat = STATUS_LIMMIT_OF_REJECT;
}

/* seek to pos by ref
 *  need before set keybuf and keynum
 */
// private
void table::seekPos(const uchar* rawPos)
{
    seekKey(HA_READ_KEY_OR_NEXT, keymap());
    if (m_keyNum == (int)m_table->s->primary_key)
        return;
    int cmp;
    while ((m_stat == 0) &&
           ((cmp = m_table->file->cmp_ref(position(true), rawPos)) != 0))
    {
        unlockRow(m_delayAutoCommit);
        m_table->file->ha_index_next_same(m_table->record[0], &m_keybuf[0],
                                          keymap());
    }
}

void table::movePos(const uchar* pos, char keyNum, bool sureRawValue)
{
    const uchar* rawPos = pos;
    if (!sureRawValue && (m_table->file->ref_length > REF_SIZE_MAX))
        rawPos = bms()->getRefByBm(*(unsigned int*)pos);

    setNonKey();
    unlockRow(m_delayAutoCommit);
    m_stat = m_table->file->ha_rnd_pos(m_table->record[0], (uchar*)rawPos);
    setCursorStaus();
    if ((keyNum == -1) || (keyNum == -64) || (keyNum == -2))
        return;
    if (m_stat == 0)
    {
        if (m_keyNum != keyNum)
        { // need key change
            key_copy(&m_keybuf[0], m_table->record[0],
                     &m_table->key_info[(int)keyNum], KEYLEN_ALLCOPY);
            // It seek(s) until ref becomes the same, since it is a duplication
            // key.
            setKeyNum(keyNum);
            seekPos(rawPos);
        }
    }
}

bookmarks* table::bms()
{
    if (m_bms == NULL)
        m_bms.reset(new bookmarks(m_table->file->ref_length));
    return m_bms.get();
}

/** get current record bookmark (position)
 *
 */
const uchar* table::position(bool raw)
{
    m_table->file->position(m_table->record[0]);
    // handler is set uniqu number to ref.
    if (!raw && (m_table->file->ref_length > REF_SIZE_MAX))
        return bms()->getBookmarkPtr(m_table->file->ref);
    return m_table->file->ref;
}

uint table::posPtrLen() const
{
    if (m_table->file->ref_length > REF_SIZE_MAX)
        return sizeof(unsigned int);
    return m_table->file->ref_length;
}

ha_rows table::recordCount(bool estimate)
{
	ha_rows rows = 0;
	m_stat = 0;
	if ((m_table->file->ha_table_flags() &
		(HA_HAS_RECORDS | HA_STATS_RECORDS_IS_EXACT)) != 0)
	{
		m_stat = cp_record_count(m_table->file, &rows);
		return rows;
	}
    if (estimate)
    {   /* Since the answer of innodb is random, 1 returns also 0.
         Since it is important, in the case of 1
         , whether there is nothing or it is scan and investigate.
         info() is update statistics variables */ 
        m_table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
		m_stat = cp_record_count(m_table->file, &rows);
		if (m_stat || rows > 1)
            return rows;
    }
	
    uint n = 0;
    fieldBitmap fb(m_table);
    char keynum = m_keyNum;
    int inited = m_table->file->inited;
    m_table->file->ha_index_or_rnd_end();
    fb.setKeyRead(true);
    if (setKeyNum((char)0, false /* sorted */))
    {
        m_table->file->try_semi_consistent_read(true);
        m_stat = m_table->file->ha_index_first(m_table->record[0]);
        while (m_stat == 0)
        {
            m_table->file->unlock_row();
            ++n;
            ++m_readCount;
            m_stat = m_table->file->ha_index_next(m_table->record[0]);
        }
        fb.setKeyRead(false);

        // restore index init
        if ((inited == (int)handler::INDEX) && (m_keyNum != keynum))
            setKeyNum(keynum);
        else if (inited == (int)handler::RND)
            setNonKey(true /* scan */);
    }
    else
    {
        setNonKey(true /* scan */);
        m_table->file->try_semi_consistent_read(true);
        m_stat = m_table->file->ha_rnd_next(m_table->record[0]);
        while (m_stat == 0)
        {
            m_table->file->unlock_row();
            ++n;
            ++m_readCount;
            m_stat = m_table->file->ha_rnd_next(m_table->record[0]);
        }
    }
    return n;
}

void table::clearBuffer()
{
    empty_record(m_table);
}

bool table::isNisKey(char num) const
{
    if ((num >= 0) && (num < (short)m_table->s->keys))
    {
        Field* fd = m_table->key_info[(int)num].key_part[0].field;
        if (fd->null_bit)
            return true;
    }
    return false;
}

bool setNullIf(KEY& key, bool mysqlNull)
{
    bool nullkey = false;
    Field* fd = key.key_part[0].field; // Nis is only in a head segment.
    if (isNisField(fd->field_name))
    {
        if (isNullNis(key, fd->field_name[3] == 'a'))
        {
            fd->set_null();
            nullkey = true;
        }
        else
            fd->set_notnull();
    }
    else if (!mysqlNull)
    {
        for (int j = 0; j < (int)key.user_defined_key_parts; j++)
        {
            fd = key.key_part[j].field;
            if (key.key_part[j].null_bit && fd->ptr)
            {
                if (isNullValue(fd))
                {
                    fd->set_null();
                    nullkey = true;
                }
                else
                    fd->set_notnull();
            }
        }
    }
    return nullkey;
}

/** Sets null value for all null able key fields
 *  @return null sets field count
 */
int table::setKeyNullFlags()
{
    int setCount = 0;
    if (m_table->s->null_fields)
    {
        for (int i = 0; i < (int)m_table->s->keys; i++)
        {
            KEY& key = m_table->key_info[i];
            if (key.flags & HA_NULL_PART_KEY)
            {
                bool nullKey = setNullIf(key, m_mysqlNull);
                if (nullKey && (i == m_keyNum))
                    ++setCount;
            }
        }
    }
    return setCount;
}

/** Sets null value for all null able fields
  This method effect only !mysqlNull
 */
void table::setFieldNullFlags()
{
    std::vector<Field*>::iterator it = m_noNullModeNullFieldList.begin();
    while (it != m_noNullModeNullFieldList.end())
    {
        if (isNullValue(*it))
            (*it)->set_null();
        else
            (*it)->set_notnull();
        ++it;
    }
}

void table::setTimeStamp(bool insert)
{
    if (m_timeStampFields.size())
        m_db.setCurTime();
    std::vector<Field*>::iterator it = m_timeStampFields.begin();
    while (it != m_timeStampFields.end())
    {
        if (m_timestampAlways)
        {
            if ((insert && cp_has_insert_default_function((*it))) ||
                (!insert && cp_has_update_default_function((*it))))
                (*it)->store(0.0f);
        }
        if (!(*it)->is_null() && ((*it)->val_real() == 0))
        {
            if (insert)
                cp_evaluate_insert_default_function((*it));
            else
                cp_evaluate_update_default_function((*it));
        }
        ++it;
    }  
}

__int64 table::insert(bool ncc)
{
    if ((m_mode == TD_OPEN_READONLY) || (m_table->reginfo.lock_type < TL_WRITE))
    {
        m_stat = STATUS_INVALID_LOCKTYPE;
        return 0;
    }
    __int64 autoincValue = 0;

    {
        autoincSetup setup(m_table);
        setKeyNullFlags();
        if (!m_mysqlNull) setFieldNullFlags();
		setTimeStamp(true /* insert */);
        cp_setup_rpl_bitmap(m_table);
        m_stat = m_table->file->ha_write_row(m_table->record[0]);
        autoincValue = m_table->file->insert_id_for_cur_row;

        if (m_stat == 0 && m_table->file->insert_id_for_cur_row)
        {
            if (!m_bulkInserting)
                m_table->file->ha_release_auto_increment();
        }
    }

    if (m_stat == 0)
    {
        ++m_insCount;
        // innodb default is ncc=-1.
        m_nonNcc = !ncc;
        if (!ncc && !m_bulkInserting)
            key_copy(&m_keybuf[0], m_table->record[0],
                     &m_table->key_info[(int)m_keyNum], KEYLEN_ALLCOPY);
        
        /* Do not change to m_changed = false */
        m_changed = true;
    }
    m_nounlock = setCursorStaus(true);
    return autoincValue;
}

void table::beginUpdate(char keyNum)
{
    m_stat = 0;
    beginDel();
    if (m_stat == 0)
    {
        if (keyNum >= 0)
           setKeyNum(keyNum);
        store_record(m_table, record[1]);
    }
}
void table::beginDel()
{
    if ((m_mode == TD_OPEN_READONLY) || (m_table->reginfo.lock_type < TL_WRITE))
    {
        m_stat = STATUS_INVALID_LOCKTYPE;
        return;
    }

    if (m_db.m_inAutoTransaction == this)
    {
        // Confirmed that hold lock(X) by innodb_lock_monitor
        indexInit();
    }

    if (m_cursor)
    {
        m_stat = 0;
        /* The current position is established in advance.
         If not in transaction then m_validCursor=false */
        if (m_validCursor == false)
        {
            store_record(m_table, record[1]);
            if (m_keyNum >= 0)
            {
                // seek until ref is same.
                uchar rawPos[128];
                memcpy(rawPos, position(true), m_table->file->ref_length);
                seekPos(rawPos);
            }
            else
                movePos(position(true), -1, true);

            // Has blob fields then ignore conflicts.
            if ((m_stat == 0) && (m_table->s->blob_fields == 0) &&
                cmp_record(m_table, record[1]))
                m_stat = STATUS_CHANGE_CONFLICT;

            setCursorStaus();
            if (m_stat)
                return;
        }
    }
    else
        m_stat = STATUS_NO_CURRENT;
}

/** Update current record.
 *  It is indispensable that there is a current record in advance
 *  and beginUpdate() is called.
 */
void table::update(bool ncc)
{
    if (m_stat == 0)
    {
        int nullFieldsOfCurrentKey = setKeyNullFlags();
        if (!m_mysqlNull) setFieldNullFlags();
		setTimeStamp(false /* update */);
        cp_setup_rpl_bitmap(m_table);

        m_stat = m_table->file->ha_update_row(m_table->record[1],
                                              m_table->record[0]);
        if (m_stat == 0 || (m_stat == HA_ERR_RECORD_IS_THE_SAME))
        {
            m_stat = 0;
            m_nonNcc = false;
            if (!ncc) // innodb default is ncc=-1.
            {
                // Only when the present key value is changed
                if (m_keyNum >= 0)
                {
                    const KEY& key = m_table->key_info[(int)m_keyNum];
                    key_copy(&m_keybuf[0], m_table->record[0], (KEY*)&key,
                             KEYLEN_ALLCOPY);

                    // Since the NULL key was set, a current position is
                    // lost.
                    if (nullFieldsOfCurrentKey == 0)
                        m_nonNcc = true;
                }
            }
            /* Do not change to m_changed = false */
            if (m_stat == 0)
            {
                ++m_updCount;
                m_changed = true;
            }
            m_nounlock = setCursorStaus(true);
        }
    }
}

/** del current record.
 *  It is indispensable that there is a current record in advance
 *  and beginDel() is called.
 */
void table::del()
{
    if (m_stat == 0)
    {
        m_stat = m_table->file->ha_delete_row(m_table->record[0]);
        /* Do not change to m_changed = false */
        if (m_stat == 0)
        {
            ++m_delCount;
            m_changed = true;
        }
    }

    //No cursor changed
    m_nounlock = (m_stat == 0);
  
}

#ifdef USE_BTRV_VARIABLE_LEN

/** The offset position of the last VAR field data
 */
unsigned short table::lastVarFieldPos() const
{
    if (m_table->s->varchar_fields &&
        (m_recordFormatType & RF_FIXED_PLUS_VALIABLE_LEN))
        return (unsigned short)(lastVarFiled()->ptr -
                                m_table->s->field[0]->ptr);
    return 0;
}

#endif

/** The data length of the field
 */
unsigned short table::fieldDataLen(int fieldNum) const
{
    Field* fd = m_table->field[fieldNum];
    if (isVarType(fd->type()))
        return var_strlen(fd);
    return fd->pack_length();
}

const char* table::keyName(char keyNum)
{
    if ((keyNum >= 0) && (keyNum < (short)m_table->s->keys))
    {
        KEY& key = m_table->key_info[(int)keyNum];
        return key.name;
    }
    return "";
}

int table::keynumByName(const char* name) const
{
    if (*name == 0x00)
        name = "PRIMARY";
    for (int i = 0; i < (int)m_table->s->keys; i++)
    {
        if (strcmp(m_table->key_info[i].name, name) == 0)
            return i;
    }
    THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_KEYNAME, name);
}

void table::startBulkInsert(ha_rows rows)
{
    if ((m_mode == TD_OPEN_READONLY) || (m_table->reginfo.lock_type < TL_WRITE))
    {
        m_stat = STATUS_INVALID_LOCKTYPE;
        return ;
    }
    m_table->file->ha_start_bulk_insert(rows);
    m_bulkInserting = true;
}

void table::endBulkInsert()
{
    if (m_bulkInserting)
    {
		if (m_keyNum >= 0 && m_keyNum < (char)m_table->s->keys)
			key_copy(&m_keybuf[0], m_table->record[0], &m_table->key_info[(int)m_keyNum],
					KEYLEN_ALLCOPY);
        m_bulkInserting = false;
        m_table->file->ha_release_auto_increment();
        m_table->file->ha_end_bulk_insert();
    }
}

const char* table::valStr(int fieldNum, int& size)
{
    assert((fieldNum >= 0) && (fieldNum < (int)m_table->s->fields));

    Field* fd = m_table->field[fieldNum];
    size = -1;
    if (fd->is_null())
        return "";
    else
        fd->val_str(&m_str, &m_str);
    size = (int)m_str.length();
    return m_str.c_ptr();
}

inline void setSegmentValue(const KEY_PART_INFO& segment, const std::string& v)
{
    segment.field->set_notnull();
    if (!((v.size() == 1) && (v[0] == 0x00)))
        segment.field->store(v.c_str(), (uint)v.size(), &my_charset_bin);
    else
        segment.field->set_null();
}

void table::setKeyValues(const std::vector<std::string>& values, int keypart,
                         const std::string* inValue)
{
    KEY& key = m_table->key_info[(int)m_keyNum];
    for (int i = 0; i < (int)key.user_defined_key_parts; i++)
        if (i < (int)values.size())
            setSegmentValue(key.key_part[i], values[i]);
    if (keypart != -1)
        setSegmentValue(key.key_part[keypart], *inValue);
    key_copy(&m_keybuf[0], m_table->record[0], &key, KEYLEN_ALLCOPY);
}

unsigned int table::writeDefaultImage(unsigned char* p, size_t size)
{
    unsigned short cllen = (unsigned short)recordLenCl();
    unsigned short len = cllen + m_nullBytesCl;
    int offset = m_table->s->null_bytes;
    memcpy(p, &len, sizeof(unsigned short));
    p += sizeof(ushort_td);
    if (size - 2 >= len)
        memcpy(p + m_nullBytesCl, m_table->s->default_values + offset, cllen);
    
    unsigned char* src = m_table->s->default_values;
    for (int i = 0 ; i < m_nullBytesCl; ++i)
    {
        *p = *src;
        if (!(m_table->s->db_create_options & HA_OPTION_PACK_RECORD))
        {
            *p = *p >> 1;
            *p |= (*(src + 1) & 1) ? 0xf0 : 0;
        }
        ++p; ++src;
    }
    return len + sizeof(unsigned short);
}

unsigned int table::writeSchemaImage(unsigned char* p, size_t size)
{
    protocol::tdap::tabledef* td = schemaBuilder().getTabledef(this, 0, m_mysqlNull, p + 2, size - 2);
    unsigned short len = 0;
    if (td)
        len = td->varSize + 4;
    memcpy(p, &len, sizeof(unsigned short));
    return len + sizeof(unsigned short);
}

void table::getCreateSql(String* s)
{
    TABLE_LIST tables;
    tables.init_one_table(m_dbname.c_str(), m_dbname.size(), m_name.c_str(),
		            m_name.size(), m_name.c_str(), TL_READ);
    tables.table = m_table;
    cp_store_create_info(m_db.thd(), &tables, s, NULL, (int)FALSE);
}

#ifdef USE_HANDLERSOCKET

int table::fieldIndexByName(const char* name) const
{
    int index = 0;
    for (Field** fd = m_table->field; *fd; ++fd)
    {
        if (strcmp(name, (*fd)->field_name) == 0)
            return index;
        ++index;
    }
    THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_FIELDNAME, name);
}

void table::setUseFieldList(const std::string& csv)
{
    std::vector<std::string> values;
    split(values, csv, ",");
    for (int i = 0; i < (int)values.size(); i++)
        addUseField(fieldIndexByName(values[i].c_str()));
}

void table::setValue(int index, const std::string& v, int type)
{
    Field* field = *(m_table->field + index);
    if ((v.size() == 1) && (v[0] == 0x00))
        field->set_null();
    else
        field->set_notnull();
    if ((type == UPDATE_INC) || (type == UPDATE_DEC))
    {
        __int64 old = field->val_int();
        __int64 intv = _atoi64(v.c_str());
        if (type == UPDATE_INC)
            field->store(old + intv, false);
        else
            field->store(old - intv, false);
    }
    else
        field->store(v.c_str(), (uint)v.size(), &my_charset_bin);
}

void table::setUseValues(const std::vector<std::string>& values, int type)
{
    for (int i = 0; i < (int)values.size(); i++)
    {
        if ((int)useFields().size() > i)
            setValue(m_useFields[i], values[i], type);
    }
}

void table::checkFiledIndex(int index)
{
    assert(index >= 0);
}


#endif // USE_HANDLERSOCKET

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs
