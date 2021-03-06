#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
/*=================================================================
   Copyright (C) 2014 2016 BizStation Corp All rights reserved.

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
#include "trdboostapi.h"
#include <vector>

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

/* Single database inplemantation idatabaseManager
*/

class databaseManager : public idatabaseManager, private boost::noncopyable
{
    database_ptr m_dbPtr;
    database* m_db;
    std::vector<table_ptr> m_tables;
    int findTable(const _TCHAR* name)
    {
        for (int i = 0; i < (int)m_tables.size(); ++i)
            if (_tcscmp(m_tables[i]->tableDef()->tableName(), name) == 0)
                return i;
        return -1;
    }

    void connect(const connectParams& param, bool newConnection = false)
    {
        if (!newConnection && m_db && m_db->isOpened())
            return;
        connectOpen(m_db, param, newConnection);
    }

public:
    databaseManager()
    {
        m_dbPtr = createDatabaseObject();
        m_db = m_dbPtr.get();
    }

    databaseManager(database_ptr db) : m_dbPtr(db), m_db(db.get()){};

    databaseManager(database* db) : m_db(db){};

    void reset(int) { m_db->close(); }

    table_ptr table(const _TCHAR* name)
    {
        int index = findTable(name);
        if (index != -1)
            return m_tables[index];
        table_ptr t = openTable(m_db, name, TD_OPEN_NORMAL);
        if (t)
            m_tables.push_back(t);
        return t;
    }

    inline database* db() const { return m_db; }

    inline void use(const connectParams* param = NULL)
    {
        if (param)
            connect(*param, false);
    }

    inline void unUse(){};

    inline void setOption(__int64){};

    inline __int64 option() { return 0; };

    inline void beginTrn(short bias) { m_db->beginTrn(bias); };

    inline void endTrn() { m_db->endTrn(); }

    inline void abortTrn() { m_db->abortTrn(); }

    inline int enableTrn() { return m_db->enableTrn(); }

    inline void beginSnapshot(short bias = CONSISTENT_READ, binlogPos* bpos=NULL)
    {
        m_db->beginSnapshot(bias, bpos);
    }

    inline void endSnapshot() { m_db->endSnapshot(); }

    inline const _TCHAR* uri() const { return m_db->uri(); }

    inline char_td mode() const { return m_db->mode(); }

    inline bool isOpened() const { return m_db->isOpened(); }

    inline short_td stat() const { return m_db->stat(); }

    inline uchar_td* clientID() const { return m_db->clientID(); }
};

/** @cond INTERNAL */
/* multi databases and a single thread inplemantation idatabaseManager
*/
inline void releaseDatabaseDummy(database* p)
{
}
inline void releaseDbManagerDummy(idatabaseManager* p)
{
}
/** @endcond */

/** Single thread distribution database manager */
class disbDbManager : public idatabaseManager, private boost::noncopyable
{
    std::vector<database_ptr> m_dbs;
    database* m_db;

    std::vector<table_ptr> m_tables;
    int findTable(const _TCHAR* name)
    {
        for (int i = 0; i < (int)m_tables.size(); ++i)
            if (_tcscmp(m_tables[i]->tableDef()->tableName(), name) == 0)
                return i;
        return -1;
    }

    int findDbIndex(const connectParams* param) const
    {
        for (int i = 0; i < (int)m_dbs.size(); ++i)
            if (isSameUri(param, m_dbs[i]))
                return i;
        return -1;
    }

public:
    disbDbManager()
    {
        database_ptr p(createDatabaseObject());
        addDb(p);
    }

    disbDbManager(database* db)
    {
        //No delete , because managing life cycle is db owner.
        database_ptr d(db, releaseDatabaseDummy);
        addDb(d);
    }

    disbDbManager(database_ptr& db) { addDb(db); }

    void reset(int)
    {
        m_tables.clear();
        m_dbs.clear();
        database_ptr p(createDatabaseObject());
        addDb(p);
    }

    // change currnt
    database_ptr& addDb(database_ptr& db)
    {
        m_dbs.push_back(db);
        m_db = db.get();
        return db;
    }

    table_ptr table(const _TCHAR* name)
    {
        int index = findTable(name);
        if (index != -1)
            return m_tables[index];
        table_ptr t = openTable(m_db, name, TD_OPEN_NORMAL);
        if (t)
            m_tables.push_back(t);
        return t;
    }

    database* db() const { return m_db; }

    inline void use(const connectParams* param = NULL)
    {
        if (param)
        {
            int index = findDbIndex(param);
            if (index == -1)
            {
                database_ptr p = createDatabaseObject();
                addDb(p);
                m_db = p.get();
                connectOpen(m_db, *param, false);
            }
            else
                m_db = m_dbs[index].get();
        }
        else
            m_db = m_dbs[0].get();
    }

    inline void unUse(){};

    inline const _TCHAR* uri() const { return m_db->uri(); }

    inline char_td mode() const { return m_db->mode(); }

    inline bool isOpened() const { return m_db->isOpened(); }

    inline void setOption(__int64){};

    inline __int64 option() { return 0; };

    inline void beginTrn(short bias) 
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

    inline void endTrn() 
    { 
        for (size_t i = 0; i < m_dbs.size(); ++i)
            m_dbs[i]->endTrn();
    }

    inline void abortTrn() 
    { 
        for (size_t i = 0; i < m_dbs.size(); ++i)
            m_dbs[i]->abortTrn();
    }

    inline int enableTrn() { return m_db->enableTrn(); }

    inline void beginSnapshot(short bias = CONSISTENT_READ, binlogPos* bpos=NULL)
    {
        m_db->beginSnapshot(bias, bpos);
    }

    inline void endSnapshot() { m_db->endSnapshot(); }

    inline short_td stat() const { return m_db->stat(); }

    inline uchar_td* clientID() const { return m_db->clientID(); }
};
/** @cond INTERNAL */

template <>
inline dbmanager_ptr createDatabaseForConnectionPool(dbmanager_ptr& c)
{
#ifdef SWIGRUBY
    dbmanager_ptr p(new databaseManager(), releaseDbManagerDummy);
#else
    dbmanager_ptr p(new databaseManager());
#endif
    return p;
}

template <>
inline void connectOpen(dbmanager_ptr db, const connectParams& connPrams,
                        bool newConnection)
{
    connectOpen(db->db(), connPrams, newConnection);
}

/** @endcond */

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
