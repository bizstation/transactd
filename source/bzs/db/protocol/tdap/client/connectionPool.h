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

short __STDCALL dllUnloadCallbackFunc();

template <class Database_Ptr>
class connectionPool
{
	std::vector<Database_Ptr> m_dbs;
	mutable boost::mutex m_mutex;
	mutable boost::mutex m_mutex2;
	mutable boost::condition m_busy;
	int m_maxConnections;
	DLLUNLOADCALLBACK_PTR m_regitfunc;
	friend short __STDCALL dllUnloadCallbackFunc();

	Database_Ptr addOne(const connectParams& param)
	{
		Database_Ptr db;
		db = createDatabaseForConnectionPool(db);
		connectOpen(db, param, true/* new connection*/);
		m_dbs.push_back(db);
		return m_dbs[m_dbs.size()-1];
	}
	

public:
	connectionPool(int maxConnections=0):m_maxConnections(maxConnections)
	{
#ifdef WIN32
		m_regitfunc = nsdatabase::getDllUnloadCallbackFunc();
		if (m_regitfunc)
			m_regitfunc(dllUnloadCallbackFunc);
#endif
	};
	
	~connectionPool()
	{
		if (m_regitfunc)
			m_regitfunc(NULL);
	}

	/** Delivery database instance
		If a connect error is occured then bzs::rtl::exception exception is thrown.
	*/
	Database_Ptr get(const connectParams* param=NULL)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		assert((param && m_maxConnections) || m_dbs.size());

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
		m_maxConnections = (int)size;
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

	// max 5second
	bool reset(int waitSec=5)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		bool flag;
		for (int j=0;j<waitSec*100;j++)
		{
			flag = false;
			for (size_t i = 0;i<m_dbs.size();i++)
				if (m_dbs[i].use_count() != 1)  flag = true;
			if (!flag)
				break;
			Sleep(100 * MCRTOMM);
		}
		m_dbs.clear();
		return flag;
	}

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
		#error "Please define the USE_DBM_CONNECTION_POOL when you need a connection pool";
	#endif
#endif

extern stdCconnectionPool cpool;
void releaseConnection(stdDbmCconnectionPool* pool);




}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_CONNECTIONPOOL_H
