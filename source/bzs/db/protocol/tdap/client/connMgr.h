#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
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
#include "nsTable.h"
#include <bzs/db/transactd/connectionRecord.h>
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

class database;
#pragma warning(disable:4251)

class DLLLIB connMgr : private nstable
{
public:
	typedef  bzs::db::transactd::connection::record  record;
	typedef  std::vector<record> records;
protected:


private:
	std::vector<record> m_records;
	__int64 m_params[2];
	database* m_db;
	std::_tstring m_uri;
	void allocBuffer();
	void writeRecordData(){};
	void onReadAfter(){};

	 ~connMgr();
public:
	explicit connMgr(database* db);

	void connect(const _TCHAR* uri);
	void disconnect();
	const records& connections();
	const records& databases(__int64 connid);
	const records& tables(__int64 connid, int dbid);
	void disconnectOne(__int64 connid);
	void disconnectAll();
	short_td stat();

	database* db()const;
	using nstable::tdapErr;
};

#pragma warning(default:4251)


}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
