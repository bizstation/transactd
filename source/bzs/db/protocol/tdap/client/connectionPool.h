#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_CONNECTIONPOOL_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_CONNECTIONPOOL_H
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
#if HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION
#include <build/swig/ruby/threadBlockRegionWrapper.h>
#endif

#include <bzs/db/protocol/tdap/client/databaseManager.h>
#define BOOST_THREAD_USE_LIB
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
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

/** connection pool class
        Hold database or databaseManager instance and delivery.

        If maxConnections = 0 then not create databases automaticaly at no free
   a database.
        This case need call reserve() function at start up the process.\n

        Otherwise, create new database automaticaly by get() function with
   connectParams.
        This case need call setMaxConnections() function at start up the
   process.
*/

template <class Database_Ptr> class connectionPool
{
    std::vector<Database_Ptr> m_dbs;
    mutable boost::mutex m_mutex;
    mutable boost::mutex m_mutex2;
    mutable boost::condition m_busy;
    int m_maxConnections;
    Database_Ptr addOne(const connectParams& param);
#if (__BCPLUSPLUS__)
public:
#endif
    DLLUNLOADCALLBACK_PTR m_regitfunc;
    friend short __STDCALL dllUnloadCallbackFunc();

public:
    connectionPool(int maxConnections = 0);
    ~connectionPool();

    Database_Ptr get(const connectParams* param = NULL);
    void reserve(size_t size, const connectParams& param);
    void setMaxConnections(int n);
    int maxConnections() const;
    void releaseOne();
    bool reset(int waitSec = 5);
    int usingCount() const;
};

typedef connectionPool<database_ptr> stdDbCconnectionPool;
typedef connectionPool<dbmanager_ptr> stdDbmCconnectionPool;

/** stdDbmCconnectionPool is default for connetion pool.
        pooling database and reuse tables.
*/
#ifdef USE_DBM_CONNECTION_POOL
typedef stdDbmCconnectionPool stdCconnectionPool;
#else
#ifdef USE_DB_CONNECTION_POOL
typedef stdDbCconnectionPool stdCconnectionPool;
#else
#error                                                                         \
    "Please define the USE_DBM_CONNECTION_POOL when you need a connection pool";
#endif
#endif

short __STDCALL dllUnloadCallbackFunc();
void releaseConnection(stdDbmCconnectionPool* pool);

extern stdCconnectionPool cpool;

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_CONNECTIONPOOL_H
