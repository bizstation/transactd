/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

namespace bzs
{
using namespace netsvc::server;
namespace db
{
using namespace protocol::tdap::mysql;
using namespace engine::mysql;

namespace transactd
{

connManager::~connManager()
{
}

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

const database* connManager::getDatabase(const module* mod, int dbid) const
{

    igetDatabases* dbm =
        dynamic_cast<igetDatabases*>(mod->m_commandExecuter.get());
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

void connManager::getDatabaseList(const module* mod) const
{
    igetDatabases* dbm =
        dynamic_cast<igetDatabases*>(mod->m_commandExecuter.get());

    const databases& dbs = dbm->dbs();
    for (size_t j = 0; j < dbs.size(); j++)
    {
        if (dbs[j])
        {
            const database* db = dbs[j].get(); 
            m_records.push_back(connection::record());
            connection::record& rec = m_records[m_records.size() - 1];
            rec.conId = (unsigned __int64)mod;
            rec.cid = db->clientID();
            rec.dbid = (unsigned short)j;
            rec.status = 0;
            rec.inTransaction = db->inTransaction();
            rec.inSnapshot = db->inSnapshot();
            if (rec.inTransaction)
            {
                if (db->transactionIsolation() == ISO_REPEATABLE_READ)
                    rec.trnType = MULTILOCK_GAP;
                else if (db->transactionIsolation() == ISO_READ_COMMITTED)
                {
                    if (db->transactionType() == TRN_RECORD_LOCK_SINGLE)
                        rec.trnType = SINGLELOCK_NOGAP;
                    else
                        rec.trnType = MULTILOCK_NOGAP;
                }
            }
            if (rec.inSnapshot)
            {
                if (db->transactionIsolation() == 0)
                    rec.trnType = CONSISTENT_READ;
                else if (db->transactionIsolation() == ISO_REPEATABLE_READ)
                    rec.trnType = MULTILOCK_GAP_SHARE;
                else if (db->transactionIsolation() == ISO_READ_COMMITTED)
                    rec.trnType = MULTILOCK_NOGAP_SHARE;
            }
            strncpy_s(rec.name, 64, dbs[j]->name().c_str(), 64);
        }
    }
}

const connManager::records& connManager::getRecords(unsigned __int64 conid,
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
                getDatabaseList(mod);
            else
            {
                const database* db = getDatabase(mod, dbid);
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
                            rec.conId = (unsigned __int64)mod;
                            rec.cid = db->clientID();
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

void connManager::doDisconnect(unsigned __int64 conid)
{
    module* mod = const_cast<module*>(getMod(conid));
    if (mod)
    {
        igetDatabases* dbm =
            dynamic_cast<igetDatabases*>(mod->m_commandExecuter.get());
        const databases& dbs = dbm->dbs();
        if (dbs.size())
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
    for (size_t i = 0; i < modules.size(); i++)
    {
        const module* mod = dynamic_cast<module*>(modules[i]);
        if (mod && ((unsigned __int64)mod != m_me))
            doDisconnect((unsigned __int64)mod);
    }
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
