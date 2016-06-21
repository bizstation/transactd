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
    if (!pp)
        pp = p + _tcslen(p);
    while (*p && pp)
    {
        ss.push_back(_tstring(p, pp));
        p = pp + 1;
        if (*p)
        {
            pp = _tcschr(p, _T(','));
            if (!pp)
                ss.push_back(_tstring(p, p + _tcslen(p)));
        }
    }
}

database_ptr createDb(const _TCHAR* host, const _TCHAR* user, const _TCHAR* passwd, bool newConnection)
{
    connectParams cp(_T("tdap"), host, _T("mysql"),
                    NULL, user, passwd);
    database_ptr db = createDatabaseObject();
    connectOpen(db, cp, newConnection);
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

_TCHAR* slaveList(const node& nd, _TCHAR* slaves)
{ // auto listup slave hosts from masetr
    database_ptr db = createDatabaseObject();
    connMgr_ptr mgr = getMasterMgr(db, nd);

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
    mgr.reset();
    return slaves;
}

void makeSlaveList(const failOverParam& pm)
{
    _TCHAR slaves[SLAVES_SIZE];
    slaveList(pm.master, slaves);
    pm.slaves = slaves;
}

void getBinlogPos(connMgr_ptr mgr, binlogPos& bpos)
{
    const connMgr::records& rs = mgr->sqlvars();
    validateStatus(mgr, _T("sqlvars"));
    if (rs.size() > TD_SQL_VER_BINLOG_GTID)
    {
        strcpy_s(bpos.filename, BINLOGNAME_SIZE, rs[TD_SQL_VER_BINLOG_FILE].name);
        bpos.pos = (unsigned int)rs[TD_SQL_VER_BINLOG_POS].longValue;
        bpos.setGtid(rs[TD_SQL_VER_BINLOG_GTID].value_ptr());
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
    connMgr::records recs = mgr->channels();
    for (size_t i = 0;i < recs.size(); ++i)
    {
        const connMgr::records& rs = mgr->slaveStatus(recs[i].name);
        if (rs.size() > SLAVE_STATUS_MASTER_HOST)
        {
            _TCHAR buf[MAX_PATH];
            if (_tcscmp(rs[SLAVE_STATUS_MASTER_HOST].value(buf, MAX_PATH), oldMaster) == 0)
                return recs[i].name;
        }
    }
    return "";
}

void changeMaster(const node& nd, const masterNode& master, database_ptr db,
        connMgr_ptr mgr,  binlogPos* bpos)
{
    replicationParam pm;
    pm.master = master;
    pm.type = isMariadb(db) ? REP_TYPE_GTID_MA : REP_TYPE_GTID_MY;
    pm.master.channel = getChannlName(mgr, nd.host.c_str());
    replSlave rpl(db, pm, mgr.get());
    rpl.stop(all);
    if (bpos)
        rpl.changeMaster(bpos);
    else
        rpl.switchMaster(replSlave::slave_pos);
    rpl.start();
}

class safeHaSalves
{
    vector<connMgr_ptr> m_mgrs;
    vector<_tstring>& m_slaves;
    vector<database_ptr> m_dbs;
    const node& m_nd;
    bool m_locked;
    bool m_isSwitchOver;
    bool m_isDisableOldMasterToSalve;
    bool m_oldMasterRoleChanged;
    bool m_newMasterRoleChanged;

    connMgr_ptr createMgr(database_ptr db, const _TCHAR* host)
    {
        connectParams cp(_T("tdap"), host, _T(""), NULL,
                                 m_nd.user.c_str(), m_nd.passwd.c_str());
        
        connMgr_ptr mgr = createConnMgr(db.get());
        mgr->connect(cp.uri());
        validateStatus(mgr, _T("connMgr connect"));
        return mgr;
    }

    void addMgr(const _TCHAR* host)
    {
        database_ptr db = createDatabaseObject();
        m_dbs.push_back(db);
        m_mgrs.push_back(createMgr(db, host));
    }

    void createMgrList()
    {
        for (size_t i= 0; i< m_slaves.size(); ++i)
             addMgr(m_slaves[i].c_str());
    }

    void addDbs(vector<database_ptr>& dbs, const _TCHAR* host, bool newConnection)
    {
        dbs.push_back(createDb(host, m_nd.user.c_str(), m_nd.passwd.c_str(), newConnection));
    }

    void createDbList(vector<database_ptr>& dbs)
    {
        for (size_t i= 0; i< m_slaves.size(); ++i)
            addDbs(dbs, m_slaves[i].c_str(), false);
    }

    void doChangeMaster(const masterNode& master, database_ptr db,
        connMgr_ptr mgr,  binlogPos* bpos)
    {
        ::changeMaster(m_nd, master, db, mgr, bpos);
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
        pm.master.channel = getChannlName(mgr, m_nd.host.c_str());
        const_cast<masterNode&>(master).channel = pm.master.channel;
        replSlave rpl(db, pm, mgr.get());

        rpl.stop(one);
        setMasterRoleStatus(mgr);
        if (bpos)
        {   // Sync SQLthread binlog pos to old master
            rpl.startUntil(*bpos);
            rpl.waitForSlaveSync(*bpos, 2, NULL);
            rpl.stop(one);
        }
        rpl.resetAll(); // reset only this channel
        if (readOnlyControl)
        {
            db->execSql("set global read_only = OFF");
            validateStatus(db, _T("read_only"));
        }
    }

public:

    safeHaSalves(vector<_tstring>& slaves, const failOverParam& pm) :
                m_slaves(slaves), m_nd(pm.master), m_locked(false),
                m_isSwitchOver(pm.isSwitchOver()),
                m_isDisableOldMasterToSalve(pm.isDisableOldMasterToSalve()),
                m_oldMasterRoleChanged(false),
                m_newMasterRoleChanged(false)
    {
        createMgrList();
    }

    void lock()
    {
        m_locked = true;
        for (size_t i = 0; i < m_mgrs.size(); ++i)
        {
            m_mgrs[i]->haLock();
            validateStatus(m_mgrs[i], _T("haLock"));
        }
    }

    int selectPromoteHost()
    {
        string fname;
        unsigned int pos = 0;
        int index = -1;
        for (int i = 0; i < (int)m_mgrs.size(); ++i)
        {
            const connMgr::records& rs = m_mgrs[i]->slaveStatus();
            validateStatus(m_mgrs[i], _T("slaveStatus"));
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
        return index;
    }

    void changeMaster(const masterNode& master, size_t newMasterIndex, binlogPos* bpos,
                        bool readOnlyControl )
    {
        vector<database_ptr> dbs;
        createDbList(dbs);

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
            if (i != newMasterIndex)
                doChangeMaster(master, dbs[i], m_mgrs[i], NULL);
        }
    }

    void setEnableFailOver(bool v)
    {
        for (size_t i = 0; i < m_mgrs.size(); ++i)
        {
            m_mgrs[i]->setEnableFailover(v);
            validateStatus(m_mgrs[i], _T("setEnableFailOver"));
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
                connMgr_ptr mgr = createMgr(db, m_nd.host.c_str());
                mgr->setRole(HA_ROLE_MASTER);
                mgr.reset();
            }
        }
        catch(...){}
    }

    ~safeHaSalves()
    {
        recoverMasterRole();
        if (m_locked)
        {
            for (int i = (int)m_mgrs.size() - 1; i >= 0 ; --i)
                m_mgrs[i]->haUnlock();
        }
        m_mgrs.clear();
    }
};

class masterControl
{
    database_ptr m_db;
    connMgr_ptr m_mgr;
    const node& m_nd;
    bool m_locked;
    bool m_toReadOnly;

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
    masterControl(const node& nd, bool toReadOnly) : 
        m_nd(nd), m_locked(false), m_toReadOnly(toReadOnly)
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
        connMgr::records recs;
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
        
        if (m_toReadOnly)
        {
            database_ptr db = createDb(m_nd.host.c_str(), m_nd.user.c_str(), m_nd.passwd.c_str(), true);
            //db->execSql("flush tables with read lock");
            //validateStatus(db, _T("flush tables"));
            db->execSql("set global read_only = ON");
            validateStatus(db, _T("read_only"));
        }
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

void doSwitchOrver(const failOverParam& pm)
{
    overrideCompatibleMode cpblMode(database::CMP_MODE_MYSQL_NULL);
    size_t newMasterIndex;
    vector<_tstring> slaves;
    split(slaves, pm.slaves.c_str());
    if (pm.isSwitchOver())
    {
        vector<_tstring>::iterator it = find(slaves.begin(), slaves.end(), pm.newMaster.host);
        newMasterIndex = it - slaves.begin();
    }
    binlogPos* bpos_ptr = NULL;
    binlogPos bpos;
    bool readOnlyControl = (pm.option & OPT_READONLY_CONTROL) != 0;

    safeHaSalves slvs(slaves, pm);
    slvs.lock();
    if (pm.isSwitchOver())
    {
        masterControl oldMaster(pm.master, readOnlyControl);
        // block new connection to old master
        if (oldMaster.setRole(HA_ROLE_SLAVE))
            slvs.setOldMasterRoleChanged();
        // wait for current transaction
        oldMaster.waitForFinishTrx(bpos);
        bpos_ptr = &bpos;
        // release master trx lock
    }else if (pm.isFailOver())
    {
        newMasterIndex = slvs.selectPromoteHost();
        if (newMasterIndex >= slaves.size())
            THROW_BZS_ERROR_WITH_MSG(_T("Invalid new Master."));
        failOverParam& mp = const_cast<failOverParam&>(pm);
        mp.newMaster.host = slaves[newMasterIndex];
        mp.newMaster.repPort = getNewMasterPort(toUtf8(pm.newMaster.host), pm.portMap);
    }
    slvs.changeMaster(pm.newMaster, newMasterIndex, bpos_ptr, readOnlyControl);
}

void failOrver(const failOverParam& pm)
{
    pm.option |= OPT_FO_FAILOVER;
    pm.option &= ~OPT_SO_AUTO_SLVAE_LIST;
    doSwitchOrver(pm);
}

void switchOrver(const failOverParam& pm)
{
    if (pm.option & OPT_SO_AUTO_SLVAE_LIST)
        makeSlaveList(pm);
    pm.option &= ~OPT_FO_FAILOVER;
    doSwitchOrver(pm);
}

void masterToSlave(const failOverParam& pm)
{
    overrideCompatibleMode cpblMode(database::CMP_MODE_MYSQL_NULL);
    database_ptr dbt = createDb(pm.master.host.c_str(), pm.master.user.c_str(),
            pm.master.passwd.c_str(), false);
    database_ptr db = createDatabaseObject();
    connMgr_ptr mgr = getMasterMgr(db, pm.master);
    mgr->setRole(HA_ROLE_SLAVE);
    validateStatus(mgr, _T("setRoleMaster"));
    changeMaster(pm.master, pm.newMaster, dbt, mgr, NULL);
    mgr.reset();
}

void setEnableFailOver(const failOverParam& pm, bool v)
{
    if (pm.option & OPT_SO_AUTO_SLVAE_LIST)
        makeSlaveList(pm);
    vector<_tstring> slaves;
    split(slaves, pm.slaves.c_str());
    pm.option &= ~OPT_FO_FAILOVER;
    safeHaSalves slvs(slaves, pm);
    slvs.setEnableFailOver(v);
}

void setServerRole(const failOverParam& pm, int v)
{
    masterControl srv(pm.master, false);
    srv.setRole(v);
}


