
#ifndef GLOBAL_REPLICATION_REPLCOMMANDH
#define GLOBAL_REPLICATION_REPLCOMMANDH
/*=================================================================
   Copyright (C) 2016 BizStation Corp All rights reserved.

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
#include <bzs/db/protocol/tdap/client/fields.h>
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>

namespace bz = bzs::db::protocol::tdap;
namespace bzc = bzs::db::protocol::tdap::client;

// Copy mode
#define CP_NO_SCHEMA              1
#define CP_NO_DB                  2

// Node options
#define OPT_UNTIL_WAIT            1
#define OPT_CREATE_BY_SCHEM_IF    2
#define OPT_DISABLE_LOGBIN        4
#define OPT_RESET_MASTER          8


// Replication mode
#define REP_TYPE_REGACY           0
#define REP_TYPE_GTID_MA          1
#define REP_TYPE_GTID_MY          2


// Replication error
#define ERROR_IO_THREAD               1
#define ERROR_SQL_THREAD              2
#define ERROR_LOG_BROKN               3
#define ERROR_FAILED                  4

// Replication error resolve results
#define RESOLV_RESULT_CANCEL          2
#define RESOLV_RESULT_YES             6
#define RESOLV_RESULT_NO              7

// Replication execute status
#define REP_NF_SLAVE_INITED           -1
#define REP_NF_SLAVE_STOPPED          0
#define REP_NF_SLAVE_SNAPSHOT_STARTED 1
#define REP_NF_SLAVE_UNTIL_STARTED    2
#define REP_NF_SLAVE_WAIT_SUCCESS     3
#define REP_NF_SLAVE_COPYED           4
#define REP_NF_SLAVE_MASTER_CHANGED   5
#define REP_NF_SLAVE_STARTED          6
#define REP_NF_SLAVE_CHECKED          7
#define REP_NF_SLAVE_DIFF_END         8
#define REP_NF_WAIT                   100


// Replication params
struct node
{
private:
    int options;
public:
    std::_tstring host;
    std::_tstring port;
    std::_tstring db;
    std::_tstring schema;
    std::_tstring user;
    std::_tstring passwd;

    node() : options(OPT_UNTIL_WAIT) {};
    bzc::connectParams cp(int opt=0);
    bool option(int op) const { return (options & op) != 0 ;}
    void setOption(int op, bool enable=true);
};

struct masterNode : public node
{
    masterNode() : node(){}
    std::string repPort;
    std::string repUser;
    std::string repPasswd;
    std::string repOption;
    std::string channel;
};


struct replicationParam
{
    masterNode master;
    node slave;
    std::vector<std::_tstring> skipTables;
    int type;
    bool needSlaveWait() { return slave.option(OPT_UNTIL_WAIT);}
    void setSlaveWait(bool v){ slave.setOption(OPT_UNTIL_WAIT, v);}
    bool isCreateBySchemIf() { return slave.option(OPT_CREATE_BY_SCHEM_IF);}
    void setCreateBySchemIf(bool v){ slave.setOption(OPT_CREATE_BY_SCHEM_IF, v);}
    bool isDisableLogbin() { return slave.option(OPT_DISABLE_LOGBIN);}
    void setDisableLogbin(bool v){ slave.setOption(OPT_DISABLE_LOGBIN, v);}
    replicationParam() : type(REP_TYPE_REGACY) {}
};

class replicationNotify
{
public:
    virtual ~replicationNotify() {};
    virtual void onUpdateStaus(int status) = 0;
    virtual int onResolvError(int type, const bzc::connMgr::records& recs) = 0;
};


// replication manegemnet operations
class replSlave
{
    struct replImpl* m_impl;
    bzc::database_ptr m_db;
    bzc::connMgr* m_mgr;
    bool m_mysqlGtid;
    void validateError(const _TCHAR* msg);
    bool isSlaveSync(bzc::binlogPos& bpos, replicationNotify* nf);
    bool resolvBroknError(const bzc::connMgr::records& recs, replicationNotify* nf);
    bool resolvIOError(const bzc::connMgr::records& recs, replicationNotify* nf);
    bool resolvSqlError(const bzc::connMgr::records& recs, bzc::binlogPos& bpos, replicationNotify* nf);

public:
    enum autoPosType{slave_pos, current_pos};

    replSlave(bzc::database_ptr db, replicationParam& pm, bzc::connMgr* mgr);
    ~replSlave();
    void setSkipError(const char* gtid);
    void stop(bool all=true);
    void start(bool all=true);
    void startNoThrow();
    void reset();
    void resetAll();
    bool startUntil(bzc::binlogPos& bpos);
    void waitForSlaveSync(bzc::binlogPos& bpos, int waitTime, replicationNotify* nf);
    void changeMaster(const bzc::binlogPos* bpos);
    void switchMaster(autoPosType v);
    void skipLogEvent(const char* gtid);
    void stopAndReset();
    void resetMaster();
    bool isMysqlGtidMode() const {return m_mysqlGtid;}
    const replicationParam& params() const;
};

std::string toUtf8(const std::_tstring& s);
bool isGtidAutoPos(const bzc::connMgr::records& recs);
bool isPosBrokn(const bzc::connMgr::records& recs);
std::string getSkipGtid(const bzc::connMgr::records& recs);
bool isSlaveSqlRunning(const bzc::connMgr::records& recs);
bool isSlaveIoRunning(const bzc::connMgr::records& recs);
void notyfy(replicationNotify* nf, int v);
int resolv(replicationNotify* nf, int type, const bzc::connMgr::records& recs, int defValue);

static const bool all=true;
static const bool one=false;


#endif // GLOBAL_REPLICATION_REPLCOMMANDH
