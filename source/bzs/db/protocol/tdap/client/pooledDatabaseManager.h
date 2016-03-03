#pragma once
#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H

/* =================================================================
 Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include "trdboostapi.h"
#include "connectionPool.h"
namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{
/** @cond INTERNAL */
class xaTransaction
{
    std::vector<dbmanager_ptr> m_dbs;

public:
    void add(dbmanager_ptr& db) { m_dbs.push_back(db); }

    void unUse()
    {
        for (int i = (int)m_dbs.size() - 1; i >= 0; --i)
        {
            int ref = m_dbs[i].use_count();
            m_dbs.erase(m_dbs.begin() + i);
            if (ref == 2)
                releaseConnection(&cpool);// Notify release
        }
    }

    void beginTrn(short bias)
    {
        for (size_t i = 0; i < m_dbs.size(); ++i)
        {
            m_dbs[i]->beginTrn(bias);
            if (m_dbs[i]->stat())
            {
                abortTrn();
                nstable::throwError(m_dbs[i]->uri(), m_dbs[i]->stat());
            }
        }
    }

    void endTrn()
    {
        for (size_t i = 0; i < m_dbs.size(); ++i)
            m_dbs[i]->endTrn();
    }

    void abortTrn()
    {
        for (size_t i = 0; i < m_dbs.size(); ++i)
            m_dbs[i]->abortTrn();
    }
};
/** @endcond */

/*
--------------------------------------
pooledDbManager::setMaxConnections(n);

pooledDbManager db;
db.use(param)
activeTable a(db, "user");
....
db.unUse();
--------------------------------------
Thread safe
Method : non thread safe.
Object : thread safe.

*/
class pooledDbManager : public idatabaseManager
{
    dbmanager_ptr m_db;
    bool m_inUse;
    xaTransaction m_xa;
    bool m_use_xa;

public:
    inline pooledDbManager() : m_inUse(false),m_use_xa(false){};

    inline pooledDbManager(const connectParams* param) : m_inUse(false)
    {
        use(param);
    }

    inline ~pooledDbManager()
    {
        if (m_inUse)
            unUse();
    }

    inline bool isUseXa() const {return m_use_xa;}

    inline void setUseXa(bool v) {m_use_xa = v;}

    inline void use(const connectParams* param = NULL)
    {
        m_db = cpool.get(param);
        m_inUse = true;
        m_xa.add(m_db);
    }

    inline void unUse()
    {
        m_db.reset();
        m_xa.unUse();
        m_inUse = false;
    }

    inline void reset(int v) { cpool.reset(v); }

    inline table_ptr table(const _TCHAR* name) { return m_db->table(name); }

    inline database* db() const { return m_db->db(); }

    inline const _TCHAR* uri() const { return m_db->uri(); }

    inline char_td mode() const { return m_db->mode(); }

    inline bool isOpened() const { return m_db->isOpened(); }

    inline void setOption(__int64 v) { m_db->setOption(v); }

    inline __int64 option() { return m_db->option(); }

    inline void beginTrn(short bias) 
    { 
        (m_use_xa == true) ? m_xa.beginTrn(bias) : m_db->beginTrn(bias); 
    }

    inline void endTrn() 
    { 
        (m_use_xa == true) ? m_xa.endTrn() : m_db->endTrn(); 
    }

    inline void abortTrn() 
    { 
        (m_use_xa == true) ? m_xa.abortTrn() : m_db->abortTrn(); 
    }

    inline int enableTrn() { return m_db->enableTrn(); }

    inline void beginSnapshot(short bias = CONSISTENT_READ, binlogPos* bpos = NULL)
    {
        m_db->beginSnapshot(bias, bpos);
    }

    inline void endSnapshot() { m_db->endSnapshot(); }

    inline short_td stat() const { return m_db->stat(); }

    inline uchar_td* clientID() const { return m_db->clientID(); }

    inline static void setMaxConnections(int maxWorkerNum)
    {
        cpool.setMaxConnections(maxWorkerNum);
    }

    inline static int maxConnections() { return cpool.maxConnections(); }
    
    inline static void reserve(size_t size, const connectParams& param)
    {
        cpool.reserve(size, param);
    }
    
    inline int usingCount() const { return cpool.usingCount(); }

};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H
