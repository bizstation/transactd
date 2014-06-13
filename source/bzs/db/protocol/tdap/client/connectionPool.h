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
#include <bzs/db/protocol/tdap/client/databaseManager.h>
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

	If maxConnections = 0 then not create databases automaticaly at no free a database.
	This case need call reserve() function at start up the process.\n

	Otherwise, create new database automaticaly by get() function with connectParams.
	This case need call setMaxConnections() function at start up the process.

*/
template <class Database_Ptr>
class connectionPool
{
	std::vector<Database_Ptr> m_dbs;
	mutable boost::mutex m_mutex;
	mutable boost::mutex m_mutex2;
	mutable boost::condition m_busy;
	int m_maxConnections;

	Database_Ptr addOne(const connectParams& param)
	{
		Database_Ptr db;
		db = createDatabaseForConnectionPool(db);
		connectOpen(db, param, true/* new connection*/);
		m_dbs.push_back(db);
		return m_dbs[m_dbs.size()-1];
	}


public:
	connectionPool(int maxConnections=0):m_maxConnections(maxConnections){};

	/** Delivery database instance
		If a connect error is occured then bzs::rtl::exception exception is thrown.
	*/
	Database_Ptr get(const connectParams* param=NULL)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		while (1)
		{
			for (size_t i = 0;i<m_dbs.size();i++)
			{
				if (m_dbs[i].use_count() == 1)
				{
					if (param)
					{
						Database_Ptr db = m_dbs[i];
						if (isSameUri(param , db))
							return db;
					}else
						return m_dbs[i];
				}
			}

			//No a free database.
			// create new a database.
			if (param && (m_maxConnections > (int)m_dbs.size()))
				return addOne(*param);
			//Wait until releaseOne() called
			boost::mutex::scoped_lock lck(m_mutex2);
			m_busy.wait(lck);

		}
	}

	/** Create database and login the server with each connection.
		If a connect error is occured then bzs::rtl::exception exception is thrown.
	*/
	void reserve(size_t size, const connectParams& param)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		m_maxConnections = size;
		for (size_t i =0;i<size;++i)
			addOne(param);
	}

	/** Set max connections.*/
	void setMaxConnections(int n)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		m_maxConnections = n;
	}

	/** Return max connections. */
	int maxConnections() const {return m_maxConnections;}

	void releaseOne()
	{
		m_busy.notify_one();
	}

};

typedef connectionPool<database_ptr> stdDbCconnectionPool;
typedef connectionPool<dbmanager_ptr> stdDbmCconnectionPool;

/** stdDbmCconnectionPool is default for connetion pool.
	pooling database and reuse tables.
*/
typedef stdDbmCconnectionPool stdCconnectionPool;
#define POLL_MAXCONNECTIONS 100

extern stdCconnectionPool cpool;

void releaseConnection(stdDbmCconnectionPool* pool);


/** Release is indispensable at the end of database operation.
	This macro set is automatically released using shared_ptr and a variable scope.


	// For C++ applications
	begin_use_pool_database()
	dbmanager_ptr db = get_pool_database()
	...
	... some operations ...
	...
	end_use_pool_database()

*/

#define begin_use_pool_database() \
	boost::shared_ptr<stdCconnectionPool> cpool_ptr(&cpool, releaseConnection); \
{

#define get_pool_database(param) cpool_ptr->get(param);


#define end_use_pool_database() }



}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_CONNECTIONPOOL_H
