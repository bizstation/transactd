/*=================================================================
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
=================================================================*/
#include "connectionPool.h"

#ifdef __BCPLUSPLUS__
#ifdef _WIN64
#define BZS_LINK_BOOST_SYSTEM
#define BZS_LINK_BOOST_THREAD
#define BZS_LINK_BOOST_CHRONO
namespace boost
{
void tss_cleanup_implemented()
{
}
}
#else
#define BZS_LINK_BOOST_THREAD
namespace boost
{
extern "C" void tss_cleanup_implemented()
{
}
}
#endif
#include <bzs/env/boost_bcb_link.h>
#endif

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

#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
struct busyWaitArguments
{
    mutable boost::condition* m_busy;
    mutable boost::mutex* m_mutex2;
};

void scopedLock(boost::mutex::scoped_lock* lck)
{
    lck->lock();
}

void busyWait(busyWaitArguments* args)
{
    boost::mutex::scoped_lock lck(*(args->m_mutex2));
    args->m_busy->wait(lck);
}
#endif // TRANSACTD_RB_CALL_WITHOUT_GVL

short __STDCALL dllUnloadCallbackFunc()
{
    cpool.reset(0);
    cpool.m_regitfunc = NULL;
    return 0;
}

void releaseConnection(stdDbmCconnectionPool* pool)
{
    pool->releaseOne();
}

template <class Database_Ptr>
connectionPool<Database_Ptr>::connectionPool(int maxConnections)
    : m_maxConnections(maxConnections)
{
#ifdef USE_DLLUNLOAD_CALLBACK
    m_regitfunc = nsdatabase::getDllUnloadCallbackFunc();
    if (m_regitfunc)
        m_regitfunc(dllUnloadCallbackFunc);
#else
    m_regitfunc = NULL;
#endif
}

template <class Database_Ptr> connectionPool<Database_Ptr>::~connectionPool()
{
    if (m_regitfunc)
        m_regitfunc(NULL);
}

template <class Database_Ptr>
Database_Ptr connectionPool<Database_Ptr>::addOne(const connectParams& param)
{
    Database_Ptr db;
    db = createDatabaseForConnectionPool(db);
    connectOpen(db, param, true /* new connection*/);
    m_dbs.push_back(db);
    return m_dbs[m_dbs.size() - 1];
}

/** Delivery database instance
        If a connect error is occured then bzs::rtl::exception exception is
   thrown.
*/
template <class Database_Ptr>
Database_Ptr connectionPool<Database_Ptr>::get(const connectParams* param)
{
    boost::mutex::scoped_lock lck(m_mutex, boost::defer_lock);
#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
    TRANSACTD_RB_CALL_WITHOUT_GVL(scopedLock, lck);
#else
    lck.lock();
#endif
    assert((param && m_maxConnections) || m_dbs.size());

    while (1)
    {
        for (size_t i = 0; i < m_dbs.size(); i++)
        {
            if (m_dbs[i].use_count() == 1)
            {
                if (param)
                {
                    Database_Ptr db = m_dbs[i];
                    if (isSameUri(param, db))
                        return db;
                }
                else
                    return m_dbs[i];
            }
        }
        // create a new database object if there is no free one.
        if (param && (m_maxConnections > (int)m_dbs.size()))
            return addOne(*param);
// Wait until releaseOne() called
#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
        busyWaitArguments bwArgs;
        bwArgs.m_busy = &m_busy;
        bwArgs.m_mutex2 = &m_mutex2;
        TRANSACTD_RB_CALL_WITHOUT_GVL(busyWait, bwArgs);
#else
        boost::mutex::scoped_lock lck(m_mutex2);
        m_busy.wait(lck);
#endif
    }
}

/** Create database and login the server with each connection.
        If a connect error is occured then bzs::rtl::exception exception is
   thrown.
*/
template <class Database_Ptr>
void connectionPool<Database_Ptr>::reserve(size_t size,
                                           const connectParams& param)
{
    boost::mutex::scoped_lock lck(m_mutex, boost::defer_lock);
#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
    TRANSACTD_RB_CALL_WITHOUT_GVL(scopedLock, lck);
#else
    lck.lock();
#endif
    m_maxConnections = (int)size;
    for (size_t i = 0; i < size; ++i)
        addOne(param);
}

/** Set max connections.*/
template <class Database_Ptr>
void connectionPool<Database_Ptr>::setMaxConnections(int n)
{
    boost::mutex::scoped_lock lck(m_mutex, boost::defer_lock);
#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
    TRANSACTD_RB_CALL_WITHOUT_GVL(scopedLock, lck);
#else
    lck.lock();
#endif
    m_maxConnections = n;
}

/** Return max connections. */
template <class Database_Ptr>
int connectionPool<Database_Ptr>::maxConnections() const
{
    return m_maxConnections;
}

template <class Database_Ptr> void connectionPool<Database_Ptr>::releaseOne()
{
    m_busy.notify_one();
}

// max 5second
template <class Database_Ptr>
bool connectionPool<Database_Ptr>::reset(int waitSec)
{
    boost::mutex::scoped_lock lck(m_mutex, boost::defer_lock);
#ifdef TRANSACTD_RB_CALL_WITHOUT_GVL
    TRANSACTD_RB_CALL_WITHOUT_GVL(scopedLock, lck);
#else
    lck.lock();
#endif
    bool flag = true;
    for (int j = 0; j < waitSec * 100; j++)
    {
        flag = false;
        for (size_t i = 0; i < m_dbs.size(); i++)
            if (m_dbs[i].use_count() != 1)
                flag = true;
        if (!flag)
            break;
        Sleep(100 * MCRTOMM);
    }
    m_dbs.clear();
    return flag;
}

#ifdef USE_DBM_CONNECTION_POOL
template class connectionPool<dbmanager_ptr>;
#else
template class connectionPool<database_ptr>;
#endif

stdCconnectionPool cpool;

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
