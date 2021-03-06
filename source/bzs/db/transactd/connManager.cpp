/*=================================================================
   Copyright (C) 2013-2016 BizStation Corp All rights reserved.

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

#include "appModule.h"
#include <bzs/db/protocol/tdap/mysql/tdapCommandExecuter.h>
#include "connManager.h"
#include <bzs/db/engine/mysql/mysqlThd.h>
#include <bzs/db/engine/mysql/mysqlProtocol.h>
#include <bzs/db/protocol/tdap/mysql/databaseSchema.h>
#include <bzs/db/engine/mysql/errorMessage.h>


/* implemnts in transactd.cpp */
extern const char* get_trd_sys_var(int index);

/* implemnts in transactd.cpp */
extern const unsigned int* get_trd_status_var(int index);

/* implemnts in mysqlProtocol.cpp */
extern int getSlaveStatus(THD* thd, const char* channel, 
            bzs::db::transactd::connection::records& recs, bzs::db::IblobBuffer* bb);
extern int getSlaveHosts(THD* thd, bzs::db::transactd::connection::records& recs,
            bzs::db::IblobBuffer* bb);
extern int getChannels(THD* thd, bzs::db::transactd::connection::records& recs);

extern short getBinlogPos(THD* thd, binlogPos* bpos, THD* tmpThd, bzs::db::IblobBuffer* bb);

namespace bzs
{
using namespace netsvc::server;
namespace db
{
using namespace protocol::tdap::mysql;
using namespace engine::mysql;

namespace transactd
{

connManager::~connManager(){}

const module* getMod(unsigned __int64 conid)
{
    for (size_t i = 0; i < modules.size(); i++)
    {
        const module* mod = dynamic_cast<module*>(modules[i]);
        if ((unsigned __int64)mod == conid)
            return mod;
    }
    return NULL;
}

const database* connManager::getDatabase(igetDatabases* dbm, int dbid) const
{
    const databases& dbs = dbm->dbs();
    if (dbid < (int)dbs.size())
        return dbs[dbid].get();
    return NULL;
}

void connManager::getConnectionList() const
{
    for (size_t i = 0; i < modules.size(); i++)
    {
        const module* mod = dynamic_cast<module*>(modules[i]);
        if (mod && ((unsigned __int64)mod != m_me))
        {

            m_records.push_back(connection::record());
            connection::record& rec = m_records[m_records.size() - 1];
            rec.conId = (unsigned __int64)mod;
            boost::asio::ip::address adr = mod->m_endpoint.address();
            strncpy_s(rec.name, 64, adr.to_string().c_str(), 64);
        }
    }
}

void connManager::getDatabaseList(igetDatabases* dbm, const module* mod) const
{
    const databases& dbs = dbm->dbs();
    for (size_t j = 0; j < dbs.size(); j++)
    {
        if (dbs[j])
        {
            const database* db = dbs[j].get(); 
            m_records.push_back(connection::record());
            connection::record& rec = m_records[m_records.size() - 1];
            rec.conId = (unsigned __int64)mod;
            rec.id = db->clientID();
            rec.db = (unsigned short)j;
            rec.status = 0;
            rec.inTransaction = db->inTransaction();
            rec.inSnapshot = db->inSnapshot();
            if (rec.inTransaction)
            {
                if (db->transactionIsolation() >= ISO_REPEATABLE_READ)
                    rec.type = MULTILOCK_GAP;
                else if (db->transactionIsolation() == ISO_READ_COMMITTED)
                {
                    if (db->transactionType() == TRN_RECORD_LOCK_SINGLE)
                        rec.type = SINGLELOCK_NOGAP;
                    else
                        rec.type = MULTILOCK_NOGAP;
                }
            }
            if (rec.inSnapshot)
            {
                if (db->transactionIsolation() == 0)
                    rec.type = CONSISTENT_READ;
                else if (db->transactionIsolation() >= ISO_REPEATABLE_READ)
                    rec.type = MULTILOCK_GAP_SHARE;
                else if (db->transactionIsolation() == ISO_READ_COMMITTED)
                    rec.type = MULTILOCK_NOGAP_SHARE;
            }
            strncpy_s(rec.name, 64, dbs[j]->name().c_str(), 64);
        }
    }
}

const connection::records& connManager::getRecords(unsigned __int64 conid,
                                                    int dbid) const
{
    //Lock module count
    boost::mutex::scoped_lock lck(modulesMutex);

    if (conid == 0)
        getConnectionList();
    else
    {
        const module* mod = getMod(conid);
        if (mod)
        {
            igetDatabases* dbm =
                dynamic_cast<igetDatabases*>(mod->m_commandExecuter.get());

            //Lock database add remove
            boost::mutex::scoped_lock lck(dbm->mutex());
 
            if (dbid < 0)
                getDatabaseList(dbm, mod);
            else
            {
                const database* db = getDatabase(dbm, dbid);
                if (db)
                {
                    const std::vector<boost::shared_ptr<table> >& tables =
                        db->tables();

                    //Lock table add release in the db
                    boost::mutex::scoped_lock lckt(database::tableRef.mutex());
                    for (size_t k = 0; k < tables.size(); k++)
                    {
                        const table* tb = tables[k].get();
                        if (tb)
                        {
                            m_records.push_back(connection::record());
                            connection::record& rec =
                                m_records[m_records.size() - 1];
                            rec.id = tb->id();
                            rec.readCount = tb->readCount();
                            rec.updCount = tb->updCount();
                            rec.delCount = tb->delCount();
                            rec.insCount = tb->insCount();
                            rec.status = 0;
                            rec.openNormal = (tb->mode() == TD_OPEN_NORMAL);
                            rec.openReadOnly = (tb->mode() == TD_OPEN_READONLY);
                            rec.openEx = (tb->mode() == TD_OPEN_EXCLUSIVE);
                            rec.openReadOnlyEx = (tb->mode() == TD_OPEN_READONLY_EXCLUSIVE);
                            strncpy_s(rec.name, 64, tb->name().c_str(), 64);
                        }
                    }
                }
            }
        }
    }
    return m_records;
}

const connection::records& connManager::systemVariables() const
{
    m_records.clear();
    for (int i = 0 ; i < TD_VAR_SIZE; ++i)
    {
        const char* p =  ::get_trd_sys_var(i);
        if (p)
        {
            m_records.push_back(connection::record());
            connection::record& rec = m_records[m_records.size() - 1];
            rec.id = i;
            rec.type = 1;
            switch(i)
            {
            case TD_VER_DB:
            case TD_VAR_LISTENADDRESS:
            case TD_VAR_LISTENPORT:
            case TD_VAR_HOSTCHECKNAME:
            case TD_VAR_ISOLATION:
            case TD_VAR_AUTHTYPE:
            case TD_VAR_HSLISTENPORT:
                strncpy(rec.name, p , 65);
                break;
            case TD_VER_SERVER:
                sprintf_s(rec.name, 65, "%d.%d.%d", TRANSACTD_VER_MAJOR, TRANSACTD_VER_MINOR, TRANSACTD_VER_RELEASE);
                break;
            default:
                _ltoa_s(*((unsigned int*)p), rec.name, 65, 10);
                break;
            }
        }
    }
    return m_records;
}

const connection::records& connManager::extendedVariables(blobBuffer* bb) const
{
    m_records.clear();
    boost::shared_ptr<THD> thd(createThdForThread(), deleteThdForThread);
    if (thd == NULL)
        m_stat = 20001;
    else if (!checkGlobalACL(thd.get(), SUPER_ACL | REPL_CLIENT_ACL))
        m_stat = STATUS_ACCESS_DENIED;
    if (m_stat) return m_records;
    binlogPos bpos;
    short ret = getBinlogPos(thd.get(), &bpos, thd.get(), bb);
    if (ret) return m_records;
    for (int i = 0; i < TD_EXTENDED_VAR_SIZE; ++i)
    {
        m_records.push_back(connection::record());
        connection::record& rec = m_records[m_records.size() - 1];
        rec.id = i;
        rec.type = 1;
        switch (i)
        {
        case TD_EXTENDED_VAR_MYSQL_GTID_MODE:
        {
            rec.type = 0;
#if (defined(MYSQL_5_7))
            rec.longValue = (long long)get_gtid_mode(GTID_MODE_LOCK_NONE);
#else
            rec.longValue = 0;
#endif
            break;
        }
        case TD_EXTENDED_VAR_BINLOG_FILE:
            strncpy(rec.name, bpos.filename, CON_REC_VALUE_SIZE);
            break;
        case TD_EXTENDED_VAR_BINLOG_POS:
            rec.longValue = bpos.pos; 
            rec.type = 0; 
            break;
        case TD_EXTENDED_VAR_BINLOG_GTID:
            if (bpos.type == REPL_POSTYPE_GTID)
                rec.type = 2; 
            else
                strncpy(rec.name, bpos.gtid, CON_REC_VALUE_SIZE);
            break;
        }
    }
    return m_records;
}

const connection::records& connManager::statusVariables() const
{
    m_records.clear();
    for (int i = 0 ; i < TD_SVAR_SIZE; ++i)
    {
        const unsigned int* p =  ::get_trd_status_var(i);
        m_records.push_back(connection::record());
        if (p)
        {
            connection::record& rec = m_records[m_records.size() - 1];
            rec.type = 0;
            rec.id = i;
            rec.longValue = *p; 
        }
    }
    return m_records;
}

const connection::records& connManager::definedDatabases() const
{
    m_records.clear();
    boost::shared_ptr<THD> thd(createThdForThread(), deleteThdForThread);
    if (thd != NULL)
    {
        cp_security_ctx(thd.get())->skip_grants();
        readDbList(thd.get(), m_records);
    }
    return m_records;
}

const connection::records& connManager::getTableList(const char* dbname, int type) const
{
    m_records.clear();
    if (dbname && dbname[0])
    {
        boost::shared_ptr<database> db(new database(dbname, 1));
        if (db)
        {
            std::vector<std::string> tablelist;
            schemaBuilder::listTable(db.get(), tablelist, type);
            for (int i = 0; i < (int)tablelist.size(); ++i)
            {
                m_records.push_back(connection::record());
                connection::record& rec = m_records[m_records.size() - 1];
                strncpy(rec.name, tablelist[i].c_str(), 64);
                rec.name[64] = 0x00;
                rec.type = 1;
            }
        }
    }
    return m_records;
}

const connection::records& connManager::definedTables(const char* dbname, int type) const
{
    return getTableList(dbname, type);
}

const connection::records& connManager::schemaTables(const char* dbname) const
{
    return getTableList(dbname, TABLE_TYPE_TD_SCHEMA);
}

bool connManager::checkGlobalACL(THD* thd, ulong wantAccess) const
{
    const module* mod = reinterpret_cast<const module*>(m_me);
    if (mod->isSkipGrants())
        cp_security_ctx(thd)->skip_grants();
    else
        ::setGrant(thd, mod->host(), mod->user(),  NULL);
    bool ret = (cp_masterAccess(thd) & wantAccess) != 0;
    if (!ret)
        m_stat = STATUS_ACCESS_DENIED;
    return ret;
}

const connection::records& connManager::readByProtocol(int type, blobBuffer* bb, 
                                const char* channel) const
{
    m_records.clear();
    boost::shared_ptr<THD> thd(createThdForThread(), deleteThdForThread);
    if (thd == NULL)
        m_stat = 20001;
    else if (!checkGlobalACL(thd.get(), SUPER_ACL | REPL_CLIENT_ACL))
        m_stat = STATUS_ACCESS_DENIED; 
    else
    {
        int ret;
        if (type == TD_STSTCS_SLAVE_STATUS)
            ret = getSlaveStatus(thd.get(), channel, m_records, bb);
        else
            ret = getSlaveHosts(thd.get(), m_records, bb);
        m_stat = errorCode(ret);
    }
    return m_records;
}

const connection::records& connManager::readSlaveStatus(const char* channel, blobBuffer* bb) const
{
    return readByProtocol(TD_STSTCS_SLAVE_STATUS, bb, channel);
}

const connection::records& connManager::slaveHosts(blobBuffer* bb) const
{
    return readByProtocol(TD_STSTCS_SLAVE_HOSTS, bb);
}

const connection::records& connManager::channels() const
{
    m_records.clear();
    boost::shared_ptr<THD> thd(createThdForThread(), deleteThdForThread);
    if (thd == NULL)
        m_stat = 20001;
    else if (!checkGlobalACL(thd.get(), REPL_CLIENT_ACL))
        m_stat = STATUS_ACCESS_DENIED; 
    else
        m_stat = errorCode(getChannels(thd.get(), m_records));
    return m_records;
}

void connManager::doDisconnect(unsigned __int64 conid)
{
    module* mod = const_cast<module*>(getMod(conid));
    if (mod)
    {
        igetDatabases* dbm =
            dynamic_cast<igetDatabases*>(mod->m_commandExecuter.get());
        const databases& dbs = dbm->dbs();
        for (size_t i = 0; i < dbs.size(); ++i)
        {
            if (dbs[i])
            {
                if (!dbs[i]->checkAcl(SHUTDOWN_ACL))
                    m_stat = STATUS_ACCESS_DENIED;
                break;
            }
        }
        if (m_stat == 0)
            mod->disconnect();
    }
}

void connManager::disconnect(unsigned __int64 conid)
{
    boost::try_mutex::scoped_lock m(modulesMutex, boost::try_to_lock_t());

    if (m.owns_lock())
        doDisconnect(conid);
    else
    {
        for (int i = 0; i < 20; i++)
        {
            if (m.try_lock())
                return doDisconnect(conid);
            Sleep(100);
        }
    }
}

void connManager::doDisconnectAll()
{
    short stat = 0;
    for (size_t i = 0; i < modules.size(); i++)
    {
        const module* mod = dynamic_cast<module*>(modules[i]);
        if (mod && ((unsigned __int64)mod != m_me))
            doDisconnect((unsigned __int64)mod);
        if (stat == 0) stat = m_stat;
    }
    m_stat = stat;
}

void connManager::disconnectAll()
{
    boost::try_mutex::scoped_lock m(modulesMutex, boost::try_to_lock_t());

    if (m.owns_lock())
        doDisconnectAll();
    else
    {
        for (int i = 0; i < 20; i++)
        {
            if (m.try_lock())
                return doDisconnectAll();
            ;
            Sleep(100);
        }
    }
}

} // namespace transactd
} // namespace db
} // namespace bzs
