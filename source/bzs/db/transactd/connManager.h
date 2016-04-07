#ifndef BZS_DB_TRANSACTD_CONNMANAGER_H
#define BZS_DB_TRANSACTD_CONNMANAGER_H
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
#include "connectionRecord.h"
#include <bzs/netsvc/server/IAppModule.h>

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{
class database;
class igetDatabases;
}
}
namespace transactd
{

class module;

class connManager
{
public:
    

private:
    mutable connection::records m_records;
    unsigned __int64 m_me;
    mutable short m_stat;
    void getConnectionList() const;
    void getDatabaseList(bzs::db::engine::mysql::igetDatabases* dbm, 
                            const module* mod) const;
    const bzs::db::engine::mysql::database* getDatabase(
                bzs::db::engine::mysql::igetDatabases* dbm, int dbid) const;
    void doDisconnect(unsigned __int64 conid);
    void doDisconnectAll();
    const connection::records& getTableList(const char* dbname, int type) const;
    bool checkGlobalACL(THD* thd, ulong wantAccess) const;
public:
    connManager(unsigned __int64 me) : m_me(me), m_stat(0){};
    virtual ~connManager();
    const connection::records& systemVariables() const;
    const connection::records& getRecords(unsigned __int64 conid, int dbid) const;
    const connection::records& definedDatabases() const;
    const connection::records& schemaTables(const char* dbname) const;
    const connection::records& definedTables(const char* dbname, int type) const;
    const connection::records& readSlaveStatus() const;
    void disconnect(unsigned __int64 conid);
    void disconnectAll();
    short stat() const {return m_stat;}
};

} // namespace transactd
} // namespace db
} // namespace bzs

#endif // BZS_DB_TRANSACTD_CONNMANAGER_H
