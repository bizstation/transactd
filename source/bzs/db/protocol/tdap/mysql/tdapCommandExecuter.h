#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H
/*=================================================================
   Copyright (C) 2012-2016 BizStation Corp All rights reserved.

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

#include <bzs/db/engine/mysql/dbManager.h>
#include <bzs/db/protocol/ICommandExecuter.h>
#include "request.h"
#include <bzs/env/crosscompile.h>


#define GRANT_APPLY_NONE 0
#define GRANT_APPLY_ALL  1

#define AUTH_TYPE_NONE_STR  "none"
#define AUTH_TYPE_MYSQL_STR  "mysql_native"

#define AUTH_TYPE_NONE  0
#define AUTH_TYPE_MYSQL 1

extern char* g_auth_type;
extern int g_tableNmaeLower;

namespace bzs
{
namespace db
{
class blobBuffer;
namespace engine{namespace mysql{class database;}}
namespace protocol
{
namespace tdap
{
namespace mysql
{

class ReadRecordsHandler;
struct extRequest;
struct extRequestSeeks;

#define FOR_SQL true

std::string getDatabaseName(const request& req, bool forSql = false);
std::string getTableName(const request& req, bool forSql = false);
int errorCode(int ha_error);
bool isMetaDb(const request& req);

class dbExecuter : public engine::mysql::dbManager
{
    ReadRecordsHandler* m_readHandler;
    blobBuffer* m_blobBuffer;
    unsigned char m_scramble[MYSQL_SCRAMBLE_LENGTH+1];
    void releaseDatabase(request& req, int op);
    std::string makeSQLcreateTable(const request& req);
    bool connect(request& req);
    inline void doCreateTable(request& req);
    inline bool doOpenTable(request& req, char* buf, bool reconnect=false);
    inline void doSeekKey(request& req, int op, engine::mysql::rowLockMode* lock);
    inline void doMoveFirst(request& req, engine::mysql::rowLockMode* lock);
    inline void doMoveKey(request& req, int op, engine::mysql::rowLockMode* lock);
    inline int doReadMultiWithSeek(request& req, int op,
                                   netsvc::server::netWriter* nw);
    inline int doReadMulti(request& req, int op, netsvc::server::netWriter* nw);
    inline void doStepRead(request& req, int op, engine::mysql::rowLockMode* lock);

    inline void doInsert(request& req);
    inline void doUpdate(request& req);
    inline void doUpdateKey(request& req);
    inline void doDelete(request& req);
    inline void doDeleteKey(request& req);
    inline void doInsertBulk(request& req);
    inline void doGetSchema(request& req, netsvc::server::netWriter* nw);
    inline void doStat(request& req);
    inline short seekEach(extRequestSeeks* ereq, bool noBookMark);
    inline short seekBookmarkEach(extRequestSeeks* ereq, bool noBookmark);
    inline bool doAuthentication(request& req, engine::mysql::database* db);
    bool getDatabaseWithAuth(request& req, engine::mysql::database** db, bool connect=false);
public:
    dbExecuter(netsvc::server::IAppModule* mod);
    ~dbExecuter();
    int commandExec(request& req, netsvc::server::netWriter* nw);
    size_t getAcceptMessage(char* message, size_t size);
    inline int errorCode(int ha_error) { return tdap::mysql::errorCode(ha_error);}
    inline short_td errorCodeSht(int ha_error) { return (short_td)tdap::mysql::errorCode(ha_error);}
    netsvc::server::IAppModule* mod() { return m_mod; };
    friend class commandExecuter;
};

/** Command dispatcher for connectionManager
 */
class connMgrExecuter
{
    friend class safeLockReadChannels;
    request& m_req;
    __int64 m_modHandle;
    blobBuffer* m_blobBuffer;
    int definedDatabases(char* buf, size_t& size);
    int schemaTables(char* buf, size_t& size);
    int definedTables(char* buf, size_t& size);
    int definedViews(char* buf, size_t& size);
    int slaveStatus(netsvc::server::netWriter* nw);
    int channels(char* buf, size_t& size);
    int slaveHosts(netsvc::server::netWriter* nw);
    int systemVariables(char* buf, size_t& size);
    int statusVariables(char* buf, size_t& size);
    int extendedVariables(netsvc::server::netWriter* nw);
    int read(char* buf, size_t& size);
    int disconnectOne(char* buf, size_t& size);
    int disconnectAll(char* buf, size_t& size);
    void execHaCommand();
public:
    connMgrExecuter(request& req, unsigned __int64 parent, blobBuffer* bb);
    int commandExec(netsvc::server::netWriter* nw);
};

class commandExecuter : public ICommandExecuter,
                        public engine::mysql::igetDatabases
{
    mutable request m_req;
    boost::shared_ptr<dbExecuter> m_dbExec;

    int readStatistics(char* buf, size_t& size);
    int cmdStatistics(char* buf, size_t& size);

public:
    commandExecuter(netsvc::server::IAppModule* mod);
    ~commandExecuter();
    size_t perseRequestEnd(const char* p, size_t size, bool& comp) const;

    size_t getAcceptMessage(char* message, size_t size)
    {
        return m_dbExec->getAcceptMessage(message, size);
    }

    bool parse(const char* p, size_t size);

    int execute(netsvc::server::netWriter* nw);

    bool isShutDown() { return m_dbExec->isShutDown(); }

    void cleanup(){}

    const engine::mysql::databases& dbs() const { return m_dbExec->dbs(); }

    boost::mutex& mutex() { return m_dbExec->mutex(); }
};

} // namespace mysql
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H
