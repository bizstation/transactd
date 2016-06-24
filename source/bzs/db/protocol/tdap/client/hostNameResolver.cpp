/* =================================================================
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
 ================================================================= */
#include "hostNameResolver.h"
#include <boost/thread.hpp>
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>
#include <bzs/env/tstring.h>
#include <vector>
#include <string>
#include <algorithm>
#include "nsDatabase.h"
#include "database.h"
#include "connMgr.h"
#ifdef WIN32
#include <Shlobj.h>
#include <direct.h>
#endif

using namespace std;
boost::mutex g_nr_mutex;
static string masterRoleName;
static vector<string> slaveRoleNames;
static vector<string> slaveHosts;

static string cache_master;
static string cache_slave;
static string user;
static string passwd;
const char* __STDCALL hostNameResolver(const char* vhost, const char* port, char* buf, unsigned int& opt);

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

struct portMap
{
    short myPort;
    short tdPort;
    portMap(short m, short td) : myPort(m), tdPort(td){}
};

static database* g_db=NULL;
static connMgr* g_mgr = NULL;
static connMgr::records g_recs;
static vector<portMap> g_portMap;
static int g_slaveIndex = 0;
static short g_stat = 0;
static short g_slaveNum = 0;
static bool  g_failover = false;
static bool  g_failoverError = false;
void split(vector<string>& ss, const char* s)
{
    const char* p = s;
    const char* pp = strchr(p, ',');
    if (!pp)
        pp = p + strlen(p);
    while (*p && pp)
    {
        ss.push_back(string(p, pp));
        p = pp + 1;
        if (*p)
        {
            pp = strchr(p, ',');
            if (!pp) 
                ss.push_back(string(p, p + strlen(p)));
        }
    }
}

void disconnect()
{
    if (g_mgr)
        g_mgr->release();
    if (g_db) 
        database::destroy(g_db);
    g_mgr = NULL;
    g_db = NULL;
}

bool connect(const string& host)
{
    char buf[1024];
    const char* up="@";
    const char* pp="?pwd=";
    if (user == "") { up = ""; pp = "";}

    sprintf_s(buf, 1024, "tdap://%s%s%s/%s%s", user.c_str(), up,
            host.c_str(), pp, passwd.c_str());
#ifdef _UNICODE
    _TCHAR bufw[1024];
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, bufw, 1024);
    const _TCHAR* uri = bufw;                
#else
    const _TCHAR* uri = buf;
#endif
    if (g_mgr->connect(uri))
        return true;
    return false;
}

bool checkSlaveStatus(const char* chnnel)
{
    g_recs = g_mgr->slaveStatus(chnnel);
    if (g_recs.size() > SLAVE_STATUS_SLAVE_SQL_RUNNING)
        return (strcmp(g_recs[SLAVE_STATUS_SLAVE_SQL_RUNNING].name, "Yes") == 0) &&
                (strcmp(g_recs[SLAVE_STATUS_SLAVE_IO_RUNNING].name, "Yes") == 0);
    return false;
}

bool readMaster(const string& host)
{
    if (connect(host))
    {
        g_recs = g_mgr->channels(true /*withLock*/);
        if (g_recs.size())
        {
            char channle[MAX_PATH];
            g_recs[0].value(channle, MAX_PATH);
            g_recs = g_mgr->statusvars();
            int ha = HA_ROLE_SLAVE;
            if (g_recs.size() > TD_SVAR_HA)
                ha = (int)g_recs[TD_SVAR_HA].longValue;
            if ((ha & (HA_ROLE_MASTER | HA_ROLE_NONE)) == HA_ROLE_SLAVE)
            {
                if (checkSlaveStatus(channle))
                    return true;
            }
        }else
        {
            g_recs = g_mgr->statusvars();
            int ha = HA_ROLE_MASTER;
            if (g_recs.size() > TD_SVAR_HA)
                ha = (int)g_recs[TD_SVAR_HA].longValue;
            if ((ha & HA_ROLE_MASTER) == HA_ROLE_MASTER)
                cache_master = host;
        }
    }
    if (g_mgr)
    {
        g_mgr->release();
        g_db->close();
        g_mgr = connMgr::create(g_db);
    }
    return false;
}

bool selectSlave()
{
    // First try, specified slave server
    g_slaveIndex = g_slaveNum;
    if (g_slaveNum < (short)slaveHosts.size() && readMaster(slaveHosts[g_slaveNum]))
        return true;
    // After, each slave of slave list
    for (size_t i=0;i < slaveHosts.size(); ++i)
    {
        if ((short)i != g_slaveNum)
        {
            g_slaveIndex = (int)i;
            if (readMaster(slaveHosts[i]))
                return true;
        }
    }
    g_mgr->release();
    g_mgr = NULL;
    return false;
}

char* getHostName(char* str)
{
    char* p = strchr(str, '\t');
    if (p)
        *p = 0x00;
    return str;
}

int setSlaveHosts()
{
    if (g_slaveIndex < (short)slaveHosts.size())
    {
        cache_slave =  slaveHosts[g_slaveIndex];
        return 0;
    }
    return RV_SLAVE_HOSTS_NOT_FOUND;
}

void addPort(short port)
{
    for (size_t i=0;i<g_portMap.size();++i)
    {
        if (g_portMap[i].myPort == port)
        {
            char tmp[24];
            _ltoa_s(g_portMap[i].tdPort, tmp, 24, 10);
            cache_master += ":" + string(tmp);
            return;
        }
    }
}

void setMasterHost()
{
    char tmp[MAX_PATH];
    cache_master = g_recs[SLAVE_STATUS_MASTER_HOST].value(tmp, MAX_PATH);
    g_recs[SLAVE_STATUS_MASTER_PORT].value(tmp, MAX_PATH);
    addPort((short)atol(tmp));
}

const char* logPath(char* buf)
{
#ifdef _WIN32
    SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
    strcat_s(buf, MAX_PATH, PSEPARATOR_A "BizStation");
    _mkdir(buf);
    strcat_s(buf, MAX_PATH, PSEPARATOR_A "Transactd");
    _mkdir(buf);
#else
    strcpy_s(buf, MAX_PATH, "/var/log");
#endif
    strcat_s(buf, MAX_PATH, PSEPARATOR_A "trnsctcl_error.log");
    return buf;
}

int callFailover()
{
    char* fmt = "haMgr%d -c failover -s %s -u %s -p %s %s >> %s";
    char tmp[4096];
    int cpu = sizeof(char*) == 8 ? 64 :32;
    string slaves;
    for (size_t i = 0; i < slaveHosts.size(); ++i)
        slaves += slaveHosts[i] + ",";
    if (slaves.size())
        slaves.erase(slaves.end() - 1);

    string portmap;
    for (size_t i = 0; i < g_portMap.size(); ++i)
    {
        if (i == 0) portmap = "-a ";
        sprintf_s(tmp, 4096, "%d:%d,", g_portMap[i].tdPort, g_portMap[i].myPort);
        portmap += tmp;
    }
    if (portmap.size())
        portmap.erase(portmap.end() - 1);

    char buf[MAX_PATH];
    logPath(buf);

    sprintf_s(tmp, 4096, fmt, cpu, slaves.c_str(), user.c_str(), passwd.c_str(),
            portmap.size() ? portmap.c_str() : "", buf);
    return system(tmp);
}

/* Read from the slave server's master */
int setHosts()
{
    g_db = database::create();
    g_mgr = connMgr::create(g_db);
    cache_master = "-";
    cache_slave = "-";
    int ret = RV_INVALID_SLAVES;
    if (selectSlave())
    {
        setMasterHost();
        disconnect();
        ret = setSlaveHosts();
    }else if (cache_master != "-")
        ret = RV_SLAVE_HOSTS_NOT_FOUND;
    disconnect();
    if (g_failover && !g_failoverError)
    {
        g_failover = false;
        bool fa = cache_master == "-";
        if(!fa)
        {
            g_db = database::create();
            g_mgr = connMgr::create(g_db);
            bool ret = connect(cache_master);
            short stat =  g_mgr->stat();
            disconnect();
            fa = (ret == false &&
                stat >= ERROR_TD_HOSTNAME_NOT_FOUND &&
                stat < ERROR_TD_NET_REMOTE_DISCONNECT);

        }
        if (fa)
        {
            g_failoverError = callFailover() != 0;
            if (!g_failoverError)
                return setHosts();
        }
    }
    return ret;
}

const char* masterHostInternal()
{
    if (cache_master == "")
        setHosts();
    return cache_master.c_str();
}

const char* slaveHostInternal()
{
    if (cache_slave == "")
        setHosts();
    return cache_slave.c_str();
}

const char* masterHost()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    return cache_master.c_str();
}

const char* slaveHost()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    return cache_slave.c_str();
}

/* During the startResolver,resolver dose not work. */
int startResolver(const char* master, const char* slaves, 
    const char* slvHosts, short slaveNum,const char* userName, 
    const char* password)
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    nsdatabase::registerHostNameResolver(NULL);
    masterRoleName = master;
    split(slaveRoleNames, slaves);
    split(slaveHosts, slvHosts);
    g_slaveNum = slaveNum;
    user = userName;
    passwd = password;
    int ret = setHosts();
    if (ret <= RV_SLAVE_HOSTS_NOT_FOUND)
    {
        if (!nsdatabase::registerHostNameResolver(hostNameResolver))
            ret = RV_REGISTER_FUNC_ERROR;
    }
    return ret;
}

void stopResolver()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    nsdatabase::registerHostNameResolver(NULL);
}

void updateRsolver()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    if (g_failoverError) return;
    nsdatabase::registerHostNameResolver(NULL);
    g_failover = true;
    boost::thread* t = new boost::thread(setHosts);
    t->join();
    delete t;
    g_failover = false;
    nsdatabase::registerHostNameResolver(hostNameResolver);
}

void clearResolver()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    cache_master = "";
    cache_slave = "";
    g_failoverError = false;
}

void addPortMap(short mysqlPort, short transactdPort)
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    g_portMap.push_back(portMap(mysqlPort, transactdPort));
}
void clearPortMap()
{
    boost::mutex::scoped_lock lck(g_nr_mutex);
    g_portMap.clear();
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

using namespace bzs::db::protocol::tdap::client;

const char* __STDCALL hostNameResolver(const char* vhost, const char* port, char* buf, unsigned int& opt)
{
    if (opt & HST_OPTION_CLEAR_CACHE)
        updateRsolver();
    // Resolve a host name from a role name.
    boost::mutex::scoped_lock lck(g_nr_mutex);

    if (masterRoleName == vhost)
    {
        const char* p = masterHostInternal();
        if (*p && (*p != '-'))
        {
            strcpy_s(buf, MAX_PATH, p);
            opt |= HST_OPTION_ROLE_MASTER | CL_OPTION_CHECK_ROLE;
            opt &= ~HST_OPTION_ROLE_SLAVE;
            return buf;
        }
    }
    else
    {
        vector<string>::iterator it = 
                find(slaveRoleNames.begin(), slaveRoleNames.end(), vhost);
        if (it != slaveRoleNames.end())
        {
            const char* p = slaveHostInternal();
            if (*p && (*p != '-'))
            {
                strcpy_s(buf, MAX_PATH, p);
                opt |= HST_OPTION_ROLE_SLAVE | CL_OPTION_CHECK_ROLE;
                opt &= ~HST_OPTION_ROLE_MASTER;
                return buf;
            }
        }
    }
    strcpy_s(buf, MAX_PATH, vhost);
    if (port[0])
    {
        strcat_s(buf, MAX_PATH, ":");
        strcat_s(buf, MAX_PATH, port);
    }
    return buf;
}

