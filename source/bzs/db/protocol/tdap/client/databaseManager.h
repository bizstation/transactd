#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
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
/*
class databaseManager : public idatabaseManager, private boost::noncopyable
{
	database_ptr m_dbPtr;
	database* m_db;
	std::vector<table_ptr> m_tables;
	int findTable(const _TCHAR* name)
	{
		for (int i=0;i<(int)m_tables.size();++i)
			if (_tcscmp(m_tables[i]->tableDef()->tableName(), name)==0)
				return i;
		return -1;
	}
public:
	databaseManager()
	{
		m_dbPtr = createDatadaseObject();
		m_db = m_dbPtr.get();
	}

	databaseManager(database_ptr db):m_dbPtr(db),m_db(db.get()){};

	databaseManager(database* db):m_db(db){};

	int connect(const connectParams& param, bool newConnection=false)
	{
		if (!newConnection && m_db && m_db->isOpened()) return 0;
		connectOpen(m_db, param, newConnection);
		return 0;
	}

	table_ptr table(const _TCHAR* name)
	{
		int index =  findTable(name);
		if (index !=-1)
			return  m_tables[index];
		table_ptr t = openTable(m_db, name);
		if (t)
			m_tables.push_back(t);
		return t;
	}

	database* db() const
	{
		return m_db;
	}

	int findDbIndex(const connectParams& param)const
	{
		return 0;
	}
	void setCurDb(int v){};

	inline void setOption(__int64 ){};
	inline __int64 option(){return 0;};
	inline void beginTrn(short bias){m_db->beginTrn(bias);};
	inline void endTrn(){m_db->endTrn();}
	inline void abortTrn(){m_db->abortTrn();}
	inline int enableTrn(){return m_db->enableTrn();}
	inline void beginSnapshot(){m_db->beginSnapshot();}
	inline void endSnapshot(){m_db->endSnapshot();}
	inline short_td stat() const {return m_db->stat();}
	inline uchar_td* clientID() const{return m_db->clientID();}
};
*/

/* multi database inplemantation idatabaseManager
*/
inline void releaseDatabaseDummy(database* p){}

class databaseManager : public idatabaseManager, private boost::noncopyable
{
	std::vector<database_ptr> m_dbs;
	database* m_db;

	std::vector<table_ptr> m_tables;
	int findTable(const _TCHAR* name)
	{
		for (int i=0;i<(int)m_tables.size();++i)
			if (_tcscmp(m_tables[i]->tableDef()->tableName(), name)==0)
				return i;
		return -1;
	}

public:
	databaseManager()
	{
		database_ptr p( createDatadaseObject());
		addDb(p);
	}

	databaseManager(database* db)
	{
		database_ptr d(db, releaseDatabaseDummy);
		addDb(d);
	}

	databaseManager(database_ptr& db)
	{
		addDb(db);
	}

	//change currnt
	database_ptr& addDb(database_ptr& db)
	{
		m_dbs.push_back(db);
		m_db = db.get();
		return db;
	}

	//change currnt
	int connect(const connectParams& param, bool newConnection=false)
	{
		m_db = NULL;
		int n = findDbIndex(param);
		if (n != -1)
			m_db = m_dbs[n].get();

		if ((m_db==NULL) && !m_dbs[0]->isOpened())
		{
			m_db = m_dbs[0].get();
			n = 0;
			newConnection = false;
		}
		if (newConnection || m_db==NULL)
		{
			database_ptr p = createDatadaseObject();
			addDb(p);
			m_db = p.get();
			n = m_dbs.size()-1;
		}
		if (m_db->isOpened()) return n;
		connectOpen(m_db, param, newConnection);
		return n;
	}


	table_ptr table(const _TCHAR* name)
	{
		int index =  findTable(name);
		if (index !=-1)
			return  m_tables[index];
		table_ptr t = openTable(m_db, name);
		if (t)
			m_tables.push_back(t);
		return t;
	}


	database* db() const
	{
		return m_db;
	}

	int findDbIndex(const connectParams& param)const
	{
		for (int i=0;i<(int)m_dbs.size();++i)
		if (m_dbs[i] && m_dbs[i]->isOpened()
					 && (_tcscmp(m_dbs[i]->uri(), param.uri())==0))
			return i;

		return -1;
	}

	void setCurDb(int v)
	{
		assert((int)m_dbs.size() > v);
		m_db = m_dbs[v].get();
	}


	inline void setOption(__int64 ){};
	inline __int64 option(){return 0;};
	inline void beginTrn(short bias){m_db->beginTrn(bias);};
	inline void endTrn(){m_db->endTrn();}
	inline void abortTrn(){m_db->abortTrn();}
	inline int enableTrn(){return m_db->enableTrn();}
	inline void beginSnapshot(){m_db->beginSnapshot();}
	inline void endSnapshot(){m_db->endSnapshot();}
	inline short_td stat() const {return m_db->stat();}
	inline uchar_td* clientID() const{return m_db->clientID();}
};
/** @cond INTERNAL */

template<> inline dbmanager_ptr createDatabaseForConnectionPool(dbmanager_ptr& c)
{
	dbmanager_ptr p(new databaseManager());
	return p;
}


template<> inline void connectOpen(dbmanager_ptr db, const connectParams& connPrams, bool newConnection)
{
	db->connect(connPrams, newConnection);
}

/** @endcond */


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASEMANAGER_H
