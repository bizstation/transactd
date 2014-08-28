#pragma once
#ifndef	BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H
#define	BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H

/* =================================================================
 Copyright (C) 20014 BizStation Corp All rights reserved.

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

public:
	inline pooledDbManager():m_inUse(false){};
	
	inline pooledDbManager(const connectParams* param):m_inUse(false)
	{
		use(param);
	}

	inline ~pooledDbManager()
	{
		if (m_inUse)
			unUse();
	}

	inline void use(const connectParams* param=NULL)
	{
		m_db = cpool.get(param);
		m_inUse = true;
	}

	inline void unUse()
	{
		m_db.reset();
		releaseConnection(&cpool);
		m_inUse = false;
	}

	inline void reset(int v) {cpool.reset(v);}

	inline table_ptr table(const _TCHAR* name){return m_db->table(name);}

	inline database* db()const {return m_db->db();}

	inline const _TCHAR* uri() const{return m_db->uri();}

	inline char_td mode() const{return m_db->mode();}

	inline bool isOpened() const{return m_db->isOpened();}

	inline void setOption(__int64 v){m_db->setOption(v);};

	inline __int64 option(){return m_db->option();};

	inline void beginTrn(short bias){m_db->beginTrn(bias);};

	inline void endTrn(){m_db->endTrn();}

	inline void abortTrn(){m_db->abortTrn();}

	inline int enableTrn(){return m_db->enableTrn();}

	inline void beginSnapshot(){m_db->beginSnapshot();}

	inline void endSnapshot(){m_db->endSnapshot();}

	inline short_td stat() const {return m_db->stat();}

	inline uchar_td* clientID() const{return m_db->clientID();}

	inline static void setMaxConnections(int maxWorkerNum){cpool.setMaxConnections(maxWorkerNum);};
	inline static int  maxConnections(){return cpool.maxConnections();};
	inline static void reserve(size_t size, const connectParams& param){cpool.reserve(size, param);}
	//inline static bool reset(int waitSec=5){return cpool.reset(waitSec);}
};


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif	//BZS_DB_PROTOCOL_TDAP_CLIENT_POOLEDDATABASEMANAGER_H

