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
#include "haCommand.h"
#include <vector>
#include <algorithm>
#pragma package(smart_init)

using namespace std;
using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;

#define SLAVES_SIZE 4096

void split(vector<_tstring>& ss, const _TCHAR* s)
{
    const _TCHAR* p = s;
    const _TCHAR* pp = _tcschr(p, _T(','));
    while (pp)
    {
        ss.push_back(_tstring(p, pp));
        p = pp + 1;
        pp = _tcschr(p, _T(','));
    }
    size_t len = _tcslen(p);
    if (len)
        ss.push_back(_tstring(p, p + len));
}

#ifdef _UNICODE
inline void notify(haNotify* nf, int status, const wchar_t* msg)
{
    if (nf) nf->onUpdateStaus(status, msg);
}
inline void notify(haNotify* nf, int status, const char* msg)
{
    if (nf)
    {
        wchar_t wbuf[1024];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, msg, -1, wbuf, 1024);
        nf->onUpdateStaus(status, wbuf);
    }
}
#else
inline void notify(haNotify* nf, int status, const char* msg)
{
    if (nf) nf->onUpdateStaus(status, msg);
}
#endif
inline void notify(haNotify* nf, int status, binlogPos& bpos)
{
    if (nf)
    {
        char buf[2048];
        sprintf_s(buf, 1024, "%s:%llu %s",bpos.filename, bpos.pos, bpos.gtid);
        notify(nf, status, buf);
    }
}

inline void nfSetHostName(haNotify* nf, const _TCHAR* host)
{
    if (nf) nf->setHostName(host);
}

database_ptr createDb(const _TCHAR* host, const _TCHAR* user, const _TCHAR* passwd, bool newConnection)
{
    connectParams cp(_T("tdap"), host, _T("mysql"),
                    NULL, user, passwd);
    database_ptr db = createDatabaseObject();
    if (db->connect(cp.uri(), newConnection))
        db->open(cp.uri(), cp.type(), cp.mode());
    return db;
}

connMgr_ptr getMasterMgr(database_ptr db, const node& nd)
{
    connMgr_ptr mgr = createConnMgr(db.get());
    connectParams cp(_T("tdap"), nd.host.c_str(),
     _T(""), NULL, nd.user.c_str(), nd.passwd.c_str());
    mgr->connect(cp.uri());
    validateStatus(mgr, _T("getMasterMgr"));
    return  mgr;
}

_TCHAR* getHost(_TCHAR* hostStatus)
{
    _TCHAR* p = _tcschr(hostStatus, _T('\t'));
    if (p) *p = 0x00;
    return hostStatus;
}

_TCHAR* slaveList(connMgr_ptr& mgr, _TCHAR* slaves)
{ // auto listup slave hosts from masetr

    const connMgr::records& recs = mgr->slaveHosts();
    validateStatus(mgr, _T("slaveHost"));
    _TCHAR tmp[MAX_PATH];
    if (recs.size())
         _tcscpy_s(slaves, SLAVES_SIZE, getHost((_TCHAR*)recs[0].value(tmp, MAX_PATH)));
    for (size_t i = 1; i < recs.size(); ++i)
    {
        _tcscat_s(slaves, SLAVES_SIZE, _T(","));
        _tcscat_s(slaves, SLAVES_SIZE, getHost((_TCHAR*)recs[i].value(tmp, MAX_PATH)));
    }
    return slaves;
}

int readHaVar(connMgr_ptr& mgr)
{
    const connMgr::records& recs = mgr->statusvars();
    int ha = HA_ROLE_SLAVE;
    if (recs.size() > TD_SVAR_HA)
        ha = (int)recs[TD_SVAR_HA].longValue;
    return ha;
}

bool isEnableFailOver(int ha)
{
    return (ha & HA_ENABLE_FAILOVER) == HA_ENABLE_FAILOVER;
}

bool isRoleMaster(int ha)
{
    return (ha & HA_ROLE_MASTER) == HA_ROLE_MASTER;
}

bool isRoleMaster(connMgr_ptr& mgr)
{
    return isRoleMaster(readHaVar(mgr));
}

void makeSlaveList(const failOverParam& pm, bool* roleMaster=NULL)
{
    database_ptr db = createDatabaseObject();
    connMgr_ptr mgr = getMasterMgr(db, pm.master);

    _TCHAR slaves[SLAVES_SIZE] = {0};
    slaveList(mgr, slaves);
    pm.slaves = slaves;
    if (roleMaster)
        *roleMaster = isRoleMaster(mgr);
    mgr.reset();
}

void getBinlogPos(connMgr_ptr mgr, binlogPos& bpos)
{
    const connMgr::records& rs = mgr->extendedvars();
    validateStatus(mgr, _T("extendedvars"));
    if (rs.size() > TD_EXTENDED_VAR_BINLOG_GTID)
    {
        strcpy_s(bpos.filename, BINLOGNAME_SIZE, rs[TD_EXTENDED_VAR_BINLOG_FILE].name);
        bpos.pos = (unsigned int)rs[TD_EXTENDED_VAR_BINLOG_POS].longValue;
        bpos.setGtid(rs[TD_EXTENDED_VAR_BINLOG_GTID].value_ptr());
    }
}

bool isMariadb(database_ptr db)
{
    btrVersions vers;
    db->getBtrVersion(&vers);
    validateStatus(db, _T("isMariadb"));
    btrVersion ver = vers.versions[VER_IDX_DB_SERVER];
    return ver.isMariaDB();
}

string getChannlName(connMgr_ptr mgr, const _TCHAR* oldMaster)
{
    connRecords_ptr recs_p  = createConnRecords(mgr->channels());
    connMgr::records& recs = *recs_p.get();

    for (size_t i = 0;i < recs.size(); ++i)
    {
        const connMgr::records& rs = mgr->slaveStatus(recs[i].name);
        if (rs.size() > SLAVE_STATUS_MASTER_HOST)
        {
            _TCHAR buf[MAX_PATH];
            if (_tcsicmp(rs[SLAVE_STATUS_MASTER_HOST].value(buf, MAX_PATH), oldMaster) == 0)
                return recs[i].name;
        }
    }
    return "";
}

void changeMaster(const node& nd, const masterNode& master, database_ptr db,
        connMgr_ptr mgr,  binlogPos* bpos, haNotify* nf)
{
    _TCHAR host[MAX_PATH];
    _TCHAR port[20];
    endPoint(db->uri(), host, MAX_PATH, port, 20);
    if (port[0])
    {
        _tcscat_s(host, MAX_PATH -_tcslen(host), _T(":"));
        _tcscat_s(host, MAX_PATH -_tcslen(host), port);
    }

    replicationParam pm;
    pm.master = master;
    pm.type = isMariadb(db) ? REP_TYPE_GTID_MA : REP_TYPE_GTID_MY;
    pm.master.channel = getChannlName(mgr, nd.host.c_str());

    nfSetHostName(nf, host);
    notify(nf, HA_NF_CANNELNAME, pm.master.channel.c_str());
    replSlave rpl(db, pm, mgr.get());
    rpl.stop(all);
    notify(nf, HA_SLAVE_STOP_ALL, _T(""));
    if (bpos)
    {
        rpl.changeMaster(bpos);
        notify(nf, HA_CHANGE_MASTER, bpos->gtid);
    }
    else
    {
        rpl.switchMaster(replSlave::slave_pos);
        notify(nf, HA_SWITCH_MASTER, "slave_pos");
    }
    rpl.start();
    notify(nf, HA_SLAVE_START, _T(""));
}

connMgr_ptr createMgr(database_ptr db, const _TCHAR* host, const node& nd, bool throwError)
{
    connectParams cp(_T("tdap"), host, _T(""), NULL,
                                nd.user.c_str(), nd.passwd.c_str());
        
    connMgr_ptr mgr = createConnMgr(db.get());
    if (!mgr->connect(cp.uri()))
    {
        if (throwError)
            validateStatus(mgr, _T("connMgr connect"));
    }
    return mgr;
}

class safeHaSalves
{
    vector<connMgr_ptr> m_mgrs;
    vector<_tstring>& m_slaves;
    vector<database_ptr> m_dbs;
    const string& m_channel;
    const node& m_nd;
    haNotify* m_nf;
    size_t m_lockSize;
    bool m_isSwitchOver;
    bool m_isDisableOldMasterToSalve;
    bool m_oldMasterRoleChanged;
    bool m_newMasterRoleChanged;

    void addMgr(const _TCHAR* host)
    {
        database_ptr db = createDatabaseObject();
        m_dbs.push_back(db);
        m_mgrs.push_back(createMgr(db, host, m_nd, m_isSwitchOver));
    }

    void createMgrList()
    {
        for (size_t i= 0; i< m_slaves.size(); ++i)
             addMgr(m_slaves[i].c_str());
    }

    void addDbs(vector<database_ptr>& dbs, const _TCHAR* host, bool newConnection)
    {
        database_ptr db = createDb(host, m_nd.user.c_str(), m_nd.passwd.c_str(), newConnection);
        if (m_isSwitchOver)
            validateStatus(db, _T("database connect open"));
        dbs.push_back(db);
    }

    void createDbList(vector<database_ptr>& dbs)
    {
        for (size_t i= 0; i< m_slaves.size(); ++i)
            addDbs(dbs, m_slaves[i].c_str(), false);
    }

    void doChangeMaster(const masterNode& master, database_ptr db,
        connMgr_ptr mgr,  binlogPos* bpos)
    {
        ::changeMaster(m_nd, master, db, mgr, bpos, m_nf);
    }

    void setMasterRoleStatus(connMgr_ptr mgr)
    {
        mgr->setRole(HA_ROLE_MASTER);
        validateStatus(mgr, _T("setRole"));
        m_newMasterRoleChanged = true;
        if (!m_isSwitchOver || m_isDisableOldMasterToSalve)
            m_oldMasterRoleChanged = true;
    }

    void promoteMaster(const masterNode& master, database_ptr db, connMgr_ptr mgr,
                             binlogPos* bpos, bool readOnlyControl)
    {
        replicationParam pm;
        pm.master = master;
        pm.type = isMariadb(db) ? REP_TYPE_GTID_MA : REP_TYPE_GTID_MY;
        if (m_isSwitchOver)
        {
            pm.master.channel = getChannlName(mgr, m_nd.host.c_str());
            const_cast<masterNode&>(master).channel = pm.master.channel;
        }
        else
            pm.master.channel = m_channel;

        nfSetHostName(m_nf, master.host.c_str());
        notify(m_nf, HA_NF_PROMOTE_MASTER, _T(""));
        notify(m_nf, HA_NF_PROMOTE_CHANNEL, m_channel.c_str());

        replSlave rpl(db, pm, mgr.get());

        rpl.stop(one);
        setMasterRoleStatus(mgr);
        notify(m_nf, HA_NF_ROLE_MASTER, _T(""));
        if (bpos)
        {   // Sync SQLthread binlog pos to old master
            rpl.startUntil(*bpos);
            notify(m_nf, HA_NF_WAIT_POS_START, _T(""));
            rpl.waitForSlaveSync(*bpos, 2, NULL);
            notify(m_nf, HA_NF_WAIT_POS_COMP, *bpos);
            
            rpl.stop(one);
            notify(m_nf, HA_SLAVE_STOP, _T(""));
        }
        rpl.resetAll(); // reset only this channel
        if (readOnlyControl)
        {
            db->execSql("set global read_only=OFF");
            validateStatus(db, _T("read_only off"));
            notify(m_nf, HA_SET_READ_ONLY, _T(""));
        }
    }

public:

    safeHaSalves(vector<_tstring>& slaves, const failOverParam& pm, haNotify* nf) :
                m_slaves(slaves), m_channel(pm.newMaster.channel),
                m_nd(pm.master), m_nf(nf), m_lockSize(0),
                m_isSwitchOver(pm.isSwitchOver()),
                m_isDisableOldMasterToSalve(pm.isDisableOldMasterToSalve()),
                m_oldMasterRoleChanged(false),
                m_newMasterRoleChanged(false)
    {
        createMgrList();
    }

    void lock()
    {
        m_lockSize = m_mgrs.size();
        for (size_t i = 0; i < m_lockSize; ++i)
        {
            if (m_mgrs[i]->isOpen())
            {
                m_mgrs[i]->haLock();
                validateStatus(m_mgrs[i], _T("haLock"));
            }
        }
    }

    int selectPromoteHost()
    {
        string fname;
        unsigned int pos = 0;
        int index = -1;
        for (int i = 0; i < (int)m_mgrs.size(); ++i)
        {
            if (m_mgrs[i]->isOpen())
            {
                const connMgr::records& rss = m_mgrs[i]->statusvars();
                if (rss.size() > TD_SVAR_HA) 
                {
                    int ha = (int)rss[TD_SVAR_HA].longValue;
                    if ((ha & HA_ENABLE_FAILOVER) != HA_ENABLE_FAILOVER)
                        return -2;
                }    
                const connMgr::records& rs = m_mgrs[i]->slaveStatus(m_channel.c_str());
                if (m_mgrs[i]->stat() == 0)
                {
                    string tmp = rs[SLAVE_STATUS_MASTER_LOG_FILE].name;
                    unsigned int tmpPos = (unsigned int)rs[SLAVE_STATUS_EXEC_MASTER_LOG_POS].longValue;
                    if (tmp > fname)
                    {
                        fname = tmp;
                        pos = tmpPos;
                        index = i;
                    }else if(tmp == fname)
                    {
                        if (tmpPos > pos)
                        {
                            pos = tmpPos;
                            index = i;
                        }
                    }
                }
            }
        }
        return index;
    }

    void changeMaster(const masterNode& master, size_t newMasterIndex, binlogPos* bpos,
                        bool readOnlyControl )
    {
        vector<database_ptr> dbs;
        createDbList(dbs);
        if (dbs.size() == 0) return;
        // First promote new master
        promoteMaster(master, dbs[newMasterIndex],
                                    m_mgrs[newMasterIndex], bpos, readOnlyControl);
        if (m_isSwitchOver && !m_isDisableOldMasterToSalve)
        {
            // The old master change to slave.
            addMgr(m_nd.host.c_str()); //Append old master.
            vector<connMgr_ptr>::iterator itmgr = m_mgrs.end() -1;
            database_ptr db = createDb(m_nd.host.c_str(), m_nd.user.c_str(), m_nd.passwd.c_str(), true);
            doChangeMaster(master, db, *itmgr, bpos);
        }

        //Slaves change master.
        for (size_t i = 0; i < dbs.size(); ++i)
        {
            if (i != newMasterIndex && dbs[i]->isOpened())
                doChangeMaster(master, dbs[i], m_mgrs[i], NULL);
        }
    }

    void setEnableFailOver(bool v)
    {
        for (size_t i = 0; i < m_mgrs.size(); ++i)
        {
            if (m_mgrs[i]->isOpen())
            {
                m_mgrs[i]->setEnableFailover(v);
                validateStatus(m_mgrs[i], _T("setEnableFailOver"));
            }
        }
    }

    connMgr_ptr mgr(size_t index) { return m_mgrs[index];}

    void setOldMasterRoleChanged() {m_oldMasterRoleChanged = true;}

    void recoverMasterRole()
    {
        try
        {
            if (m_newMasterRoleChanged != m_oldMasterRoleChanged)
            {
                database_ptr db = createDatabaseObject();
                connMgr_ptr mgr = createMgr(db, m_nd.host.c_str(), m_nd, m_isSwitchOver);
                mgr->setRole(HA_ROLE_MASTER);
                mgr.reset();
            }
        }
        catch(...){}
    }

    void unlock()
    {
        if (m_lockSize)
        {
            for (int i = (int)m_lockSize - 1; i >= 0 ; --i)
            {
                if (m_mgrs[i]->isOpen())
                    m_mgrs[i]->haUnlock();
            }
            m_lockSize = 0;
        }
    }

    ~safeHaSalves()
    {
        unlock();
        recoverMasterRole();
        m_mgrs.clear();
    }
};

class masterControl
{
    database_ptr m_db;
    connMgr_ptr m_mgr;
    const node& m_nd;
    bool m_locked;

    void trxLock()
    {
        m_mgr->setTrxBlock(true);
        validateStatus(m_mgr, _T("Master trx lock"));
        m_locked = true;
    }

    void trxUnlock()
    {
        if (m_locked)
            m_mgr->setTrxBlock(false);
        m_locked = false;
    }

public:
    masterControl(const node& nd) : m_nd(nd), m_locked(false) 
    {
        m_db = createDatabaseObject();
        m_mgr = getMasterMgr(m_db, nd);
    }

    ~masterControl()
    {
        trxUnlock();
        m_mgr.reset();
    }

    void waitForFinishTrx(binlogPos& bpos)
    {
        trxLock();
        bool cancel = false;
        int trx_count;
        size_t use_db_count;
        //connMgr::records recs;
        connRecords_ptr recs_p  = createConnRecords();
        connMgr::records& recs = *recs_p.get();
        do
        {
            trx_count = 0;
            use_db_count = 0;
            recs = m_mgr->connections();
            for (size_t i = 0; i < recs.size(); ++i)
            {
                Sleep(1);
                int trx_per_connection = 0;
                size_t use_db_per_connection = 0;
                const connMgr::records& rs = m_mgr->inUseDatabases(recs[i].conId);
                for (size_t j = 0 ;j < rs.size(); ++j)
                {
                    trx_per_connection += rs[j].inTransaction;
                    trx_count += trx_per_connection;
                }
                use_db_count += rs.size();
                use_db_per_connection = rs.size();
                // Killing connection that using database and no transactions.
                if (trx_per_connection == 0 && use_db_per_connection)
                    m_mgr->postDisconnectOne(recs[i].conId);
            }
        }while ((trx_count != 0 || use_db_count != 0) && !cancel);
        
        getBinlogPos(m_mgr, bpos);

        trxUnlock();
    }

    bool setRole(int v)
    {
        m_mgr->setRole(v);
        validateStatus(m_mgr, _T("setRole"));
        return true;
    }
};

class masterReadOnly
{
    database_ptr m_db;
public:
    masterReadOnly(const node& nd, bool toReadOnly)
    {
        if (toReadOnly)
        {
            m_db = createDb(nd.host.c_str(), nd.user.c_str(), nd.passwd.c_str(), true);
            //db->execSql("flush tables with read lock");
            //validateStatus(db, _T("flush tables"));
            m_db->execSql("set global read_only=ON");
            validateStatus(m_db, _T("read_only ON"));
        }
    }
    void check()
    {
        m_db.reset();
    }
    ~masterReadOnly()
    {
        if (m_db)
            m_db->execSql("set global read_only=OFF");
    }
};

string getNewMasterPort(const string& host, const string& map)
{
    char tmp[MAX_PATH];
    char* p;
    strcpy_s(tmp, sizeof(tmp), host.c_str());
    p = strchr(tmp, ':');
    if (p)
    {
        ++p;
        strcat_s(p, sizeof(tmp) - (p - tmp), ":");
        size_t pos = map.find(p);
        if (pos != string::npos)
        {
            int n = atol(map.c_str() + pos + strlen(p));
            _ltoa_s(n, tmp, sizeof(tmp), 10);
            return tmp;
        }
    }
    return "3306";
}

void doSwitchOrver(const failOverParam& pm, haNotify* nf)
{
    vector<_tstring> slaves;
    slaves.reserve(10);
    overrideCompatibleMode cpblMode(database::CMP_MODE_MYSQL_NULL);
    size_t newMasterIndex;
    if (pm.slaves == _T(""))
        THROW_BZS_ERROR_WITH_MSG(_T("No slave host(s)."));
    split(slaves, pm.slaves.c_str());
    if (pm.isSwitchOver())
    {
        vector<_tstring>::iterator it = find(slaves.begin(), slaves.end(), pm.newMaster.host);
        newMasterIndex = it - slaves.begin();
    }
    binlogPos* bpos_ptr = NULL;
    binlogPos bpos;
    bool readOnlyControl = (pm.option & OPT_READONLY_CONTROL) != 0;

    nfSetHostName(nf, _T(""));
    notify(nf, HA_NF_SLAVE_LIST, pm.slaves.c_str());
    safeHaSalves slvs(slaves, pm, nf);
    slvs.lock();
    if (pm.isSwitchOver())
    {
        masterReadOnly read_only(pm.master, readOnlyControl);
        { 
            masterControl oldMaster(pm.master);
            // block new connection to old master
            if (oldMaster.setRole(HA_ROLE_SLAVE))
            {
                slvs.setOldMasterRoleChanged();
                nfSetHostName(nf, pm.master.host.c_str());
                notify(nf, HA_NF_ROLE_SLAVE, _T(""));
            }
            // wait for current transaction
            notify(nf, HA_NF_WAIT_TRX_START, _T(""));
            oldMaster.waitForFinishTrx(bpos);
            notify(nf, HA_NF_WAIT_TRX_COMP, bpos);
            bpos_ptr = &bpos;
            // release master trx lock
        }
        read_only.check();
    }else if (pm.isFailOver())
    {
        int index =  slvs.selectPromoteHost();
        if (index < 0 || index >= (int)slaves.size())
        {
            if (index == -2)
                THROW_BZS_ERROR_WITH_MSG(_T("Failover is disabled."));
            else
                THROW_BZS_ERROR_WITH_MSG(_T("Invalid new Master."));
        }
        newMasterIndex = (size_t)index;
        failOverParam& mp = const_cast<failOverParam&>(pm);
        mp.newMaster.host = slaves[newMasterIndex];
        mp.newMaster.repPort = getNewMasterPort(toUtf8(pm.newMaster.host), pm.portMap);
    }
    slvs.changeMaster(pm.newMaster, newMasterIndex, bpos_ptr, readOnlyControl);
}

void failOrver(const failOverParam& pm, haNotify* nf)
{
    pm.option |= OPT_FO_FAILOVER;
    pm.option &= ~OPT_SO_AUTO_SLVAE_LIST;
    doSwitchOrver(pm, nf);
}

void switchOrver(const failOverParam& pm, haNotify* nf)
{
    if (pm.option & OPT_SO_AUTO_SLVAE_LIST)
        makeSlaveList(pm);
    pm.option &= ~OPT_FO_FAILOVER;
    doSwitchOrver(pm, nf);
}

void demoteToSlave(const failOverParam& pm, haNotify* nf)
{
    overrideCompatibleMode cpblMode(database::CMP_MODE_MYSQL_NULL);
    database_ptr dbt = createDb(pm.master.host.c_str(), pm.master.user.c_str(),
            pm.master.passwd.c_str(), false);
    nfSetHostName(nf, pm.master.host.c_str());
    database_ptr db = createDatabaseObject();
    connMgr_ptr mgr = getMasterMgr(db, pm.master);

    mgr->setRole(HA_ROLE_SLAVE);
    validateStatus(mgr, _T("setRoleMaster"));
    notify(nf, HA_NF_ROLE_SLAVE, _T(""));
    changeMaster(pm.master, pm.newMaster, dbt, mgr, NULL, nf);
    mgr.reset();
}

void setEnableFailOver(const failOverParam& pm, bool v)
{
    if (pm.option & OPT_SO_AUTO_SLVAE_LIST)
        makeSlaveList(pm);
    vector<_tstring> slaves;
    split(slaves, pm.slaves.c_str());
    pm.option &= ~OPT_FO_FAILOVER;
    safeHaSalves slvs(slaves, pm, NULL);
    slvs.setEnableFailOver(v);
}

void setServerRole(const failOverParam& pm, int v)
{
    masterControl srv(pm.master);
    srv.setRole(v);
}

const _TCHAR* roleName(bool isMaster)
{
    return isMaster ? _T("Role = Master") : _T("Role = Slave");    
}

void notifyBrank(haNotify* nf)
{
    nfSetHostName(nf, _T(""));
    notify(nf, HA_NF_BLANK, _T(""));
}

bool lockTest(connMgr_ptr mgr, haNotify* nf)
{
    bool lock = mgr->haLock();
    if (lock)
        mgr->haUnlock();
    notify(nf, lock ? HA_NF_MSG_OK : HA_NF_MSG_NG, "HA lock");
    return lock;
}

int healthCheck(const failOverParam& pm, haNotify* nf)
{
    int ret = 0;
    overrideCompatibleMode cpblMode(database::CMP_MODE_MYSQL_NULL);
    bool roleMaster;

    vector<_tstring> slaves_param;
    split(slaves_param, pm.slaves.c_str());

    makeSlaveList(pm, &roleMaster);
    vector<_tstring> slaves;
    split(slaves, pm.slaves.c_str());
    nfSetHostName(nf, _T(""));
    if (slaves_param.size())
    {
        vector<_tstring> slaves_tmp = slaves;
        if (slaves_param.size() != slaves_tmp.size())
        {
            _TCHAR tmp[256];
            _stprintf_s(tmp, 256 ,_T("slaves count(%u) not equal real slave count(%u)"),
                    (unsigned int)slaves_param.size(), (unsigned int)slaves_tmp.size());
            notify(nf, HA_NF_BLANK, tmp);
            ++ret;
        }
        std::sort(slaves_param.begin(), slaves_param.end());
        std::sort(slaves_tmp.begin(), slaves_tmp.end());
        size_t n = min(slaves_param.size(), slaves_tmp.size());
        for (size_t i = 0; i < n; ++i)
        {
            if (slaves_param[i] != slaves_tmp[i])
            {
                _TCHAR tmp[1024];
                _stprintf_s(tmp, 1024 ,_T("Unmatch host name beetween the command line and slaveHosts of master.")
                    _T("\n   command line=%s : master detect=%s"), 
                        slaves_param[i].c_str(), slaves_tmp[i].c_str());
                notify(nf, HA_NF_BLANK, tmp);
                ++ret;
            }
        }
    }

    notify(nf, HA_NF_SLAVE_LIST, pm.slaves.c_str());
    nfSetHostName(nf, pm.master.host.c_str());
    notify(nf, roleMaster ? HA_NF_MSG_OK : HA_NF_MSG_NG, roleName(roleMaster));
    if (!roleMaster) ++ret;

    {   //master lock test
        database_ptr db = createDatabaseObject();
        connMgr_ptr mgr = createMgr(db, pm.master.host.c_str(), pm.master, false);
        if (!lockTest(mgr, nf)) ++ret;
    }
        
    for (size_t i=0;i < slaves.size(); ++i)
    {
        database_ptr db = createDatabaseObject();
        connMgr_ptr mgr = createMgr(db, slaves[i].c_str(), pm.master, false);
        notifyBrank(nf);
        nfSetHostName(nf, slaves[i].c_str());
        int ha = readHaVar(mgr);
        roleMaster = isRoleMaster(ha);
        notify(nf, roleMaster ? HA_NF_MSG_NG : HA_NF_MSG_OK, roleName(roleMaster));
        if (roleMaster) ++ret;

        bool fo = isEnableFailOver(ha);
        notify(nf, fo ? HA_NF_MSG_OK : HA_NF_MSG_NG, fo ? "Failover is enabled" : "Failover is disabled");
        if (!fo) ++ret;
        
        if (!lockTest(mgr, nf)) ++ret;

        string channel = getChannlName(mgr, pm.master.host.c_str());
        notify(nf, HA_NF_CANNELNAME, channel.c_str());
        const connMgr::records& recs = mgr->slaveStatus(channel.c_str());
        if (recs.size() <= SLAVE_STATUS_EXEC_MASTER_LOG_POS)
        {
            notify(nf, HA_NF_MSG_NG, _T("Can not read slave status."));
            ++ret;
        }
        else
        {
            bool sql_run = _stricmp(recs[SLAVE_STATUS_SLAVE_SQL_RUNNING].name, "Yes") == 0;
            bool io_run = _stricmp(recs[SLAVE_STATUS_SLAVE_IO_RUNNING].name, "Yes") == 0;
            notify(nf, sql_run ? HA_NF_MSG_OK : HA_NF_MSG_NG, _T("SQL thread running"));
            notify(nf, sql_run ? HA_NF_MSG_OK : HA_NF_MSG_NG, _T("IO thread running"));
            if (!sql_run) ++ret;
            if (!io_run) ++ret;
            __int64 read_pos = recs[SLAVE_STATUS_READ_MASTER_LOG_POS].longValue;
            __int64 write_pos = recs[SLAVE_STATUS_EXEC_MASTER_LOG_POS].longValue;
            _TCHAR tmp[50];
            _i64tot_s(read_pos - write_pos, tmp, 50, 10);
            notify(nf, HA_NF_DELAY, tmp);
        }
        mgr.reset();
    }
    return ret;
}


