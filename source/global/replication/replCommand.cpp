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
#pragma hdrstop
#include "replCommand.h"
#pragma package(smart_init)
using namespace std;
using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;

#define ER_BAD_SLAVE                    26200
#define WARN_NO_MASTER_INF              26617
#define ER_UNTIL_REQUIRES_USING_GTID    26949
#define ER_SLAVE_CHANNEL_DOES_NOT_EXIST 28074


connectParams node::cp(int opt)
{
    bool nodb = (opt & CP_NO_DB) != 0;
    bool noschem = nodb || ((opt & CP_NO_SCHEMA) != 0);
    connectParams cp1(_T("tdap"),
             host.c_str(),
             nodb ? _T("") : db.c_str(),
             noschem ? _T("") : schema.c_str(),
             user.c_str(), passwd.c_str());
    return cp1;
}

void node::setOption(int op, bool enable)
{
    if (enable)
        options |= op;
    else
        options &= ~op;
}

//------------------------------------------------------------------------------
struct replImpl
{
    replicationParam& pm;

    replImpl(replicationParam& p)  : pm(p){}

    virtual bool ignoreError(short stat) { return false;}

    virtual void setSlaveSkipError(database_ptr db, const char* /*gtid*/)
    {
        execSql(db, "set global sql_slave_skip_counter=1");
    }

    virtual void setGtidSlavePos(database_ptr db, const binlogPos& bpos,
            connMgr* mgr){}

    virtual string stopSlaveSql() {return "stop slave";}

    virtual string stopAllSlaveSql() {return "stop slave";}

    virtual string startSlaveSql() {return "start slave";}

    virtual string startAllSlaveSql() {return "start slave";}

    virtual string resetSlaveSql() {return "reset slave";}

    virtual string resetSlaveAllSql() {return "reset slave all";}

    virtual string resetMasterSql() {return "reset master";}

    virtual string changeMasterSql(const binlogPos& bpos, connMgr* /*mgr*/)
    {
        char tmp[128];
        _i64toa_s(bpos.pos, tmp, 128, 10);
        return changeMasterBaseSql()
                        + ", master_log_file='" + string(bpos.filename)
                        + "', master_log_pos=" + tmp;
    }

    virtual string switchMasterSql(replSlave::autoPosType)
    {
        THROW_BZS_ERROR_WITH_MSG(_T("No support switchMaster at this replication type."));
    }

    virtual string startSlaveUntilSql(const binlogPos& bpos)
    {
        char buf[128];
        _i64toa_s(bpos.pos, buf, 128, 10);
        return "start slave until master_log_file='"
                            + string(bpos.filename) + "', master_log_pos="
                            + string(buf);
    }
protected:
    std::vector<string> getGtids(char* str)
    {
        std::vector<string> gtids;
        char* en = str + strlen(str);;
        char* st = str;
        while (st < en)
        {
            char* p = strchr(st, ',');
            if (p) *p = 0x00;
            gtids.push_back(st);
            st =  p ? p + 1 : en;
            if (*st == '\n') ++st;
        }
        return gtids;
    }

    string changeMasterBaseSql()
    {
        masterNode& mn = pm.master;
        string host = toUtf8(mn.host);
        // remove port number
        size_t pos = host.find(":");
        if (pos != std::string::npos)
            host.erase(host.begin() + pos, host.end());
        string s = "change master to master_host='"  + host + "'";
        if (mn.repPort != "")
            s += ", master_port=" + mn.repPort;
        if (mn.repUser != "")
        {
            s += ", master_user='" + mn.repUser + "'";
            s += ", master_password='" + mn.repPasswd + "'";
        }
        if (mn.repOption != "")
            s += "," + mn.repOption;
        return s;
   }
};

struct replImplMaria : public replImpl
{
    replImplMaria(database_ptr db, replicationParam& p)  : replImpl(p)
    {
        if (pm.master.channel != "")
        {
            char tmp[256];
            sprintf_s(tmp, 256, "set session default_master_connection='%s'",
                            pm.master.channel.c_str());
            execSql(db, tmp);
        }
    }

    bool ignoreError(short stat) { return stat == WARN_NO_MASTER_INF;}

    string stopAllSlaveSql() {return "stop all slaves";}

    string startAllSlaveSql() {return "start all slaves";}

    string changeMasterSql(const binlogPos& bpos, connMgr* mgr)
    {
        if (pm.type == REP_TYPE_REGACY)
            return replImpl::changeMasterSql(bpos, mgr);
        return changeMasterBaseSql() + ", master_use_gtid=slave_pos";
    }

    string switchMasterSql(replSlave::autoPosType v)
    {
        if (v == replSlave::slave_pos)
            return changeMasterBaseSql() + ", master_use_gtid=slave_pos";
        else
            return changeMasterBaseSql() + ", master_use_gtid=current_pos";
    }

    void setGtidSlavePos(database_ptr db, const binlogPos& bpos, connMgr* mgr)
    {
        if (pm.type == REP_TYPE_REGACY) return;
        string gtid = getAllGtids(bpos, mgr);
        string s = "set global gtid_slave_pos=\"" + gtid + "\"";
        execSql(db, s.c_str());
    }
private:
    inline uint_td domainid(const char* gtid) {return strtoul(gtid, NULL, 10);}

    string getAllGtids(const binlogPos& bpos, connMgr* mgr)
    {
        string gtid = bpos.gtid;
        uint_td domain = domainid(gtid.c_str());
        //const connMgr::records recs = mgr->channels();
        connRecords_ptr recs_p  = createConnRecords(mgr->channels());
        const connMgr::records& recs = *recs_p.get();


        validateStatus(mgr, _T("channels"));
        if (recs.size())
        {
            const connMgr::records& recs2 = mgr->slaveStatus(recs[0].name);
            validateStatus(mgr, _T("Gtids slaveStatus"));
            if (recs2.size() > SLAVE_STATUS_MA_GTID_IO_POS)
            {
                char tmp[256];
                recs2[SLAVE_STATUS_MA_GTID_IO_POS].value(tmp, 256);
                std::vector<string> gtids = getGtids(tmp);

                for (size_t i=0; i < gtids.size(); ++i)
                {
                    if (domain != domainid(gtids[i].c_str()))
                        gtid += string(",") + gtids[i];
                }
            }
        }
        return gtid;
    }
};

//------------------------------------------------------------------------------
struct replImplMy : public replImpl
{
    replImplMy(replicationParam& p)  : replImpl(p){}

    void setSlaveSkipError(database_ptr db, const char* gtid)
    {
        char tmp[256] = "set gtid_next = \"";
        strcat_s(tmp, 256, gtid);
        strcat_s(tmp, 256, "\"");
        execSql(db, tmp);
        execSql(db, "start transaction");
        execSql(db, "commit");
        execSql(db, "set gtid_next = automatic");
    }

    string changeMasterSql(const binlogPos& bpos, connMgr* mgr)
    {
        if (pm.type == REP_TYPE_REGACY)
            return replImpl::changeMasterSql(bpos, mgr);
        return changeMasterBaseSql() + ", master_auto_position=1";
    }

    string switchMasterSql(replSlave::autoPosType /*v*/)
    {
        return changeMasterBaseSql() + ", master_auto_position=1";
    }

    void setGtidSlavePos(database_ptr db, const binlogPos& bpos, connMgr* mgr)
    {
        if (pm.type == REP_TYPE_REGACY || !pm.slave.option(OPT_RESET_MASTER))
            return;
        string s = "set global gtid_purged=\"" + gtidset(bpos, mgr) + "\"";
        execSql(db, "reset master");
        execSql(db, s.c_str());
    }
private:
    virtual string gtidset(const binlogPos& bpos, connMgr* /*mgr*/)
    {
        return bpos.gtid;
    }
};

//------------------------------------------------------------------------------
struct replImplMy57 : public replImplMy
{
    replImplMy57(replicationParam& p)  : replImplMy(p){}

    bool ignoreError(short stat)
    {
        return (stat == ER_SLAVE_CHANNEL_DOES_NOT_EXIST);
    }

    string stopSlaveSql() {return "stop slave" + channelSql();}

    string startSlaveSql() {return "start slave" + channelSql();}

    string resetSlaveSql() {return "reset slave" + channelSql();}

    string resetSlaveAllSql() {return "reset slave all" + channelSql();}

    string startSlaveUntilSql(const binlogPos& bpos)
    {
        return replImpl::startSlaveUntilSql(bpos) + channelSql();
    }

    string changeMasterSql(const binlogPos& bpos, connMgr* mgr)
    {
        return replImplMy::changeMasterSql(bpos, mgr) + channelSql();
    }

    string switchMasterSql(replSlave::autoPosType v)
    {
        return replImplMy::switchMasterSql(v) + channelSql();
    }

private:
    string channelSql() {return " for channel '" + pm.master.channel + "'";}

    string uuid(string& gtid)
    {
        size_t pos = gtid.find(':');
        if (pos != string::npos)
            return gtid.substr(0, pos);
        assert(0);
        return "";
    }

    string gtidset(const binlogPos& bpos, connMgr* mgr)
    {  // all channels
        char tmp[4096];
        string gtid = bpos.gtid;
        strcpy_s(tmp, 4096, gtid.c_str());
        std::vector<string> gds = getGtids(tmp);
        std::vector<string> uuids;
        for (size_t i=0;i<gds.size(); ++i)
            uuids.push_back(uuid(gds[i]));

        //const connMgr::records recs = mgr->channels();
        connRecords_ptr recs_p  = createConnRecords(mgr->channels());
        const connMgr::records& recs = *recs_p.get();


        validateStatus(mgr, _T("channels"));
        for (size_t i = 0; i < recs.size(); ++i)
        {
            if (pm.master.channel != recs[i].name)
            {
                const connMgr::records& recs2 = mgr->slaveStatus(recs[i].name);
                validateStatus(mgr, _T("Gtids slaveStatus"));
                if (recs2.size() > SLAVE_STATUS_EXECUTED_GTID_SET)
                {
                    recs2[SLAVE_STATUS_EXECUTED_GTID_SET].value(tmp, 4096);
                    std::vector<string> gtids = getGtids(tmp);

                    for (size_t i=0; i < gtids.size(); ++i)
                    {
                        string uid = uuid(gtids[i]);
                        if (find(uuids.begin(), uuids.end(), uid) == uuids.end())
                            gtid += string(",") + gtids[i];
                    }
                    return gtid;
                }
            }
        }
        return gtid;
    }
};

//------------------------------------------------------------------------------
//    support functions
//------------------------------------------------------------------------------
bool isMySqlGtidMode(connMgr* mgr)
{
     const connMgr::records& rec = mgr->extendedvars();
     return  (rec.size() && (rec[TD_EXTENDED_VAR_MYSQL_GTID_MODE].longValue != 0));
}

void notyfy(replicationNotify* nf, int v)
{
    if (nf) nf->onUpdateStaus(v);
}

int resolv(replicationNotify* nf, int type, const connMgr::records& recs, int defValue)
{
    if (nf)
        defValue = nf->onResolvError(type, recs);
    return defValue;
}

__int64 getSlaveIOPos(const connMgr::records& recs)
{
    if (recs.size() > SLAVE_STATUS_READ_MASTER_LOG_POS)
        return recs[SLAVE_STATUS_READ_MASTER_LOG_POS].longValue;
    return 0;
}

std::string getSkipGtid(const connMgr::records& recs)
{
    string s;
    if (recs.size() <= SLAVE_STATUS_EXECUTED_GTID_SET)
        return s;
    const char* uuid = recs[SLAVE_STATUS_MASTER_UUID].name;
    char buf[1024];
    char* p = (char*)recs[SLAVE_STATUS_EXECUTED_GTID_SET].name;
    if (recs[SLAVE_STATUS_EXECUTED_GTID_SET].type == 2)
        p =  (char*)recs[SLAVE_STATUS_EXECUTED_GTID_SET].longValue;

    p = strstr(p, uuid);
    if (!p)
    {   // the first transaction error
        s = uuid;
        return s + ":1";
    }
    strcpy_s(buf, MAX_PATH, p);
    p = buf;
    char* pp = strchr(p, ',');
    if (pp) *pp = 0x00;
    pp = strrchr(p, ':');
    if (pp)
    {
        *pp = 0x00;
        s = p;
        s += ":";
        p = pp+1;
        pp = strchr(p, '-');
        if (pp) p = pp +1;
        unsigned __int64 v = _atoi64(p);
        ++v;
        _i64toa_s(v, buf, MAX_PATH, 10);
        s += buf;
    }
    return s;
}

bool isGtidAutoPos(const connMgr::records& recs)
{
    return recs[SLAVE_STATUS_AUTO_POSITION].longValue != 0;
}

__int64 getSlaveExecPos(const connMgr::records& recs)
{
    if (recs.size() > SLAVE_STATUS_EXEC_MASTER_LOG_POS)
        return recs[SLAVE_STATUS_EXEC_MASTER_LOG_POS].longValue;
    return 0;
}

bool isPosBrokn(const connMgr::records& recs)
{
    __int64 iop = getSlaveIOPos(recs);
    __int64 exp = getSlaveExecPos(recs);
    return exp > iop;
}

bool isSamePosAsSlaveExecPos(const connMgr::records& recs, binlogPos& bpos)
{
    return (strcmp(recs[SLAVE_STATUS_MASTER_LOG_FILE].name, bpos.filename)) == 0 &&
                (recs[SLAVE_STATUS_EXEC_MASTER_LOG_POS].longValue == bpos.pos);
}

bool isSlaveSqlRunning(const connMgr::records& recs)
{
    return strcmp(recs[SLAVE_STATUS_SLAVE_SQL_RUNNING].name, "Yes") == 0;
}

bool isSlaveIoRunning(const connMgr::records& recs)
{
    return strcmp(recs[SLAVE_STATUS_SLAVE_IO_RUNNING].name, "Yes") == 0;
}

int getSlaveIoErrno(const connMgr::records& recs)
{
    if (recs.size() > SLAVE_STATUS_LAST_IO_ERRNO)
        return (int)recs[SLAVE_STATUS_LAST_IO_ERRNO].longValue;
    return 0;
}

bool replSlave::resolvSqlError(const connMgr::records& recs,
        binlogPos& bpos, replicationNotify* nf)
{
    int ret = resolv(nf, ERROR_SQL_THREAD, recs, RESOLV_RESULT_CANCEL);
    if (RESOLV_RESULT_CANCEL == ret)
        THROW_BZS_ERROR_WITH_MSG(
            _T("SQL thread has error(s).\nPlease retry after remove error(s)."));
    else if (RESOLV_RESULT_YES == ret)
    {
        skipLogEvent(getSkipGtid(recs).c_str());
        startUntil(bpos);
        return false;
    }else
        stopAndReset();
    return true;
}

bool replSlave::resolvIOError(const connMgr::records& recs, replicationNotify* nf)
{
    int ret = resolv(nf, ERROR_IO_THREAD, recs, RESOLV_RESULT_CANCEL);
    if(RESOLV_RESULT_YES != ret)
        THROW_BZS_ERROR_WITH_MSG(_T("IO thread has error(s).\nPlease retry after remove error(s)."));
    stopAndReset();

    return true;
}

bool replSlave::resolvBroknError(const connMgr::records& recs, replicationNotify* nf)
{
    int ret = resolv(nf, ERROR_LOG_BROKN, recs, RESOLV_RESULT_CANCEL);
    if(RESOLV_RESULT_YES !=ret)
        THROW_BZS_ERROR_WITH_MSG(_T("Log position error.\nRebuild of replication is required."));
    stopAndReset();
    return true;
}

string toUtf8(const _tstring& s)
{
  #ifdef _UNICODE
  char tmp[MAX_PATH]={0};
  WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, tmp, MAX_PATH, NULL, NULL);
  return tmp;
  #else
  return s;
  #endif
}

//------------------------------------------------------------------------------
//    class replSlave
//------------------------------------------------------------------------------
replSlave::replSlave(database_ptr db, replicationParam& pm, connMgr* mgr)
    : m_db(db), m_mgr(mgr), m_mysqlGtid(false)
{

    if (pm.type == REP_TYPE_GTID_MA)
        m_impl = new replImplMaria(db, pm); // mariadb gtid
    else
    {
        btrVersions vers;
        db->getBtrVersion(&vers);
        validateStatus(db, _T("get slave version"));
        btrVersion ver = vers.versions[VER_IDX_DB_SERVER];
        if (ver.majorVersion == 5 && ver.minorVersion < 6)
            m_impl = new replImpl(pm);     // mysql  maridb 5.5
        else if (ver.isMariaDB())
            m_impl = new replImplMaria(db, pm);// maridb 10.0 10.1
        else
        {
            m_mysqlGtid = isMySqlGtidMode(mgr);
            if (!m_mysqlGtid)
                m_impl = new replImpl(pm);       // 5.6 5.7 regacy
            else if (ver.minorVersion > 6)
                m_impl = new replImplMy57(pm);// 5.7 gtid
            else
                m_impl = new replImplMy(pm);  // 5.6 gtid
        }
    }
}

replSlave::~replSlave()
{
    delete m_impl;
}

void replSlave::validateError(const _TCHAR* msg)
{
    if (m_db->stat() == 0 || m_impl->ignoreError(m_db->stat()))
        return;
    validateStatus(m_db, msg);
}

void replSlave::setSkipError(const char* gtid)
{
    m_impl->setSlaveSkipError(m_db, gtid);
}

void replSlave::stop(bool all)
{
    string s;
    if (all)
        s = m_impl->stopAllSlaveSql();
    else
        s = m_impl->stopSlaveSql();
    m_db->execSql(s.c_str());
    validateError(_T("stop slave"));
}

void replSlave::start(bool all)
{
    if (all)
        execSql(m_db, m_impl->startAllSlaveSql().c_str());
    else
        execSql(m_db, m_impl->startSlaveSql().c_str());
}

void replSlave::startNoThrow()
{
    m_db->execSql(m_impl->startSlaveSql().c_str());
}

void replSlave::reset()
{
    string s = m_impl->resetSlaveSql();
    m_db->execSql(s.c_str());
    validateError(_T("reset slave"));
}

void replSlave::resetAll()
{
    string s = m_impl->resetSlaveAllSql();
    m_db->execSql(s.c_str());
    validateError(_T("reset slave all"));
}

bool replSlave::startUntil(binlogPos& bpos)
{
    m_db->execSql(m_impl->startSlaveUntilSql(bpos).c_str());
    if ((m_db->stat() == ER_BAD_SLAVE) ||
        (m_impl->pm.type == REP_TYPE_GTID_MA &&
            m_db->stat() == ER_UNTIL_REQUIRES_USING_GTID))
        return false;
    if (m_db->stat())
        nstable::throwError(_T("start slave until"), m_db->stat());
    return true;
}

void replSlave::changeMaster(const binlogPos* bpos)
{
    m_impl->setGtidSlavePos(m_db, *bpos, m_mgr);
    execSql(m_db, m_impl->changeMasterSql(*bpos, m_mgr).c_str());
}

void replSlave::switchMaster(autoPosType v)
{
    execSql(m_db, m_impl->switchMasterSql(v).c_str());
}

void replSlave::resetMaster()
{
    execSql(m_db, "reset master");
}

void replSlave::skipLogEvent(const char* gtid)
{
    stop(one);
    setSkipError(gtid);
}

void replSlave::stopAndReset()
{
    stop(one);
    reset();
}

const replicationParam& replSlave::params() const
{
    return m_impl->pm;
}

bool replSlave::isSlaveSync(binlogPos& bpos, replicationNotify* nf)
{
    while (1)
    {
        const connMgr::records& recs = m_mgr->slaveStatus(m_impl->pm.master.channel.c_str());
        validateStatus(m_mgr, _T("slave status"));
        /* In the case of first-time replication, size is zero. */
        if (recs.size() == 0)
             return true;
        bool isIgnoreBrokn = isMysqlGtidMode() && isGtidAutoPos(recs);
        if (!isIgnoreBrokn && isPosBrokn(recs))
        {
            if (resolvBroknError(recs, nf))
                return true;
        }
        if (isSamePosAsSlaveExecPos(recs, bpos))
            return true;

        if ((isSlaveIoRunning(recs) == false) && getSlaveIoErrno(recs))
        {
             if (resolvIOError(recs, nf))
                return true;
        }
        else if (isSlaveSqlRunning(recs) == false)
        {
            if (resolvSqlError(recs, bpos, nf))
                return true;
        }
        else
            break;
        Sleep(10);
    }
    return false;
}

void replSlave::waitForSlaveSync(binlogPos& bpos, int waitTime, replicationNotify* nf)
{
    Sleep(100);
    bool slaveSync = false;
    int n = waitTime * 100;
    for (int nn = 0;nn < 10; ++nn)
    {
        if (isSlaveSync(bpos, nf))
        {
            slaveSync = true;
            break;
        }
        for (int i= 0;i < n; ++i)
        {
            notyfy(nf, REP_NF_WAIT);
            Sleep(10);
        }
    }
    if (!slaveSync)
        THROW_BZS_ERROR_WITH_MSG(
                    _T("The slave SQL thread could not be executed")
                    _T("until the target of the log position in time"));
}




