#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
/*=================================================================
   Copyright (C) 2013-2016 BizStation Corp All rights reserved.

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
#include <boost/shared_ptr.hpp>
#include <vector>
#pragma warning(disable : 4251)

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
class connMgr;
class stringBuffer;
typedef boost::shared_ptr<connMgr> connMgr_ptr;

class DLLLIB connRecords
{
    typedef bzs::db::transactd::connection::record record;
	friend class connMgr;
	std::vector<record> m_records;
	boost::shared_ptr<stringBuffer> m_buf;
	inline void resize(size_t size) { m_records.resize(size); }
	inline void erase(size_t index) { m_records.erase(m_records.begin() + index); }

public:
    connRecords();
    connRecords(const connRecords& r);
    connRecords& operator=(const connRecords& r);
	const record& operator[] (size_t index) const;
	record& operator[] (size_t index);
	size_t size() const;
	void clear();
    void release();
    static connRecords* create();
    static connRecords* create(const connRecords& r);
};

typedef boost::shared_ptr<connRecords> connRecords_ptr;

inline void releaseConnRecords(connRecords* p)
{
    if (p) p->release();
}

inline connRecords_ptr createConnRecords(const connRecords& r)
{
    return connRecords_ptr(connRecords::create(r), releaseConnRecords);
}

inline connRecords_ptr createConnRecords()
{
    return connRecords_ptr(connRecords::create(), releaseConnRecords);
}


class DLLLIB connMgr : private nstable  // no copyable
{

public:
    typedef bzs::db::transactd::connection::record record;
	typedef connRecords records;
private:
	connMgr::records m_records;
    __int64 m_params[2];
    database* m_db;
    std::_tstring m_uri;
    btrVersion m_pluginVer;
    btrVersion m_serverVer;
    void allocBuffer();
    void writeRecordData(){};
    void onReadAfter(){};
    const records& getRecords(bool isInUseTable = false);
    void convertFromOldFormat(bool isInUseTable);
    ~connMgr();
    explicit connMgr(const connMgr& r);  //no copyable
    connMgr& operator=(const connMgr& r); //no copyable
    const connMgr::records& doDefinedTables(const _TCHAR* dbname, int type);
    void setBlobFieldPointer(const bzs::db::blobHeader* bd);
    explicit connMgr(database* db);
    const records& blobOperation(int op);
public:

    bool connect(const _TCHAR* uri);
    void disconnect();
    const records& databases();
    const records& tables(const _TCHAR* dbname);
    const records& views(const _TCHAR* dbname);
    const records& schemaTables(const _TCHAR* dbname);
    const records& slaveStatus(const char* channel=0);
#ifdef _UNICODE
    const records& slaveStatus(const wchar_t* channel);
#endif
    const records& channels(bool withLock = false);
    const records& slaveHosts();
    const records& sysvars();
    const records& extendedvars();
    const records& statusvars();
    const records& connections();
    const records& inUseDatabases(__int64 connid);
    const records& inUseTables(__int64 connid, int dbid);
    void postDisconnectOne(__int64 connid);
    void postDisconnectAll();
    bool haLock();
    void haUnlock();
    bool setRole(int v);
    bool setTrxBlock(bool v);
    bool setEnableFailover(bool v);
    database* db() const;
    const _TCHAR* slaveStatusName(uint_td id) const;
    using nstable::stat;
    using nstable::tdapErr;
    using nstable::release;
    using nstable::isOpen;
    static void removeSystemDb(records& recs);
    static const _TCHAR* sysvarName(uint_td id);
    static const _TCHAR* statusvarName(uint_td id);
    static const _TCHAR* extendedVarName(uint_td id);
    static connMgr* create(database* db);
};

/**
   Releaser for boost shared_ptr.
   ex : boost::shared_ptr<connMgr> mgr(connMgr::create(), releaseConnMgr);
*/
inline void releaseConnMgr(connMgr* p)
{
    if (p) p->release();
}

inline connMgr_ptr createConnMgr(database* db)
{
    return connMgr_ptr(connMgr::create(db), releaseConnMgr);
}


} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#pragma warning(default : 4251)
#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_CTDCONNMGR_H
