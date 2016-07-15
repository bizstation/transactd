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
#include <global/replication/haCommand.h>
#include <locale.h>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <bzs/rtl/stl_uty.h>
#include <bzs/db/protocol/tdap/btrDate.h>

#ifdef __BCPLUSPLUS__
#define BZS_LINK_BOOST_PROGRAM_OPTIONS
#include <bzs/env/boost_bcb_link.h>
#endif

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace boost::program_options;
using namespace std;

static const char* cmds = "switchover,failover,demote_to_slave,"
                            "set_failover_enable,set_server_role,health_check";
#define CMD_SWITCHOVER            0
#define CMD_FAILOVER              1
#define CMD_DEMOTE_TO_SLAVE       2
#define CMD_SET_FAILOVER_ENABLE   3
#define CMD_SET_SERVER_ROLE       4
#define CMD_HEALTH_CHECK          5


_tstring str_conv(std::string& v)
{
#ifdef _UNICODE
    wchar_t wbuf[1024];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, v.c_str(), -1, wbuf, 1024);
    return wbuf;
#else
    return v;
#endif //_UNICODE
}

class haMgrNotify : public haNotify
{
    _tstring m_host; 
public:
    virtual ~haMgrNotify() {};
    void onUpdateStaus(int status, const _TCHAR* msg)
    {
        const _TCHAR* p=_T("");
        switch(status)
        {
        case HA_NF_ROLE_SLAVE: p = _T(": set role=SLAVE ");break;
        case HA_NF_CANNELNAME: p = _T(": channel name=");break;
        case HA_SLAVE_STOP_ALL: p = _T(": stop slave all ");break;
        case HA_CHANGE_MASTER: p = _T(": change master to new master, pos=");break;
        case HA_SWITCH_MASTER: p = _T(": change master to new master, pos=");break;
        case HA_SLAVE_START: p = _T(": start slave ");break;
        case HA_NF_WAIT_TRX_START: p = _T(": waiting for trx...");break;
        case HA_NF_WAIT_TRX_COMP: p = _T(": wait is completed, pos=");break;
        case HA_NF_SLAVE_LIST: p = _T("SLAVE_LIST=");break;
        case HA_NF_PROMOTE_MASTER: p = _T(": promote to master ");break;
        case HA_NF_PROMOTE_CHANNEL: p = _T(": channel name=");break;
        case HA_NF_ROLE_MASTER: p = _T(": set role=MASTER ");break;
        case HA_NF_WAIT_POS_START: p = _T(": waiting for until...");break;
        case HA_NF_WAIT_POS_COMP: p = _T(": wait is completed, pos=");break;
        case HA_SLAVE_STOP: p = _T(": stop slave ");break;
        case HA_SET_READ_ONLY: p = _T(": set READ_ONLY=1");break;
        case HA_NF_DELAY: p = _T(": SQL thread delay=");break;
        case HA_NF_MSG_OK:  
            tcout << "  " << m_host << _T(": ") << msg << _T(" OK!") << endl; 
            return;
        case HA_NF_MSG_NG:  
            tcout << "  " << m_host << _T(": ") << msg << _T(" NG!") << endl; 
            return;
        }
        tcout << "  " << m_host << p << msg << endl;
    }
    void setHostName(const _TCHAR* host)
    {
        m_host = host;
    }
};

int checkParam(failOverParam& pm, int cmd, int v)
{
    if (cmd != CMD_FAILOVER)
    {
        if (pm.master.host == _T(""))
        {
            cout << "--cur_master is requered" << endl;
            return 1;
        }
    }else
    {
        if (pm.slaves == _T(""))
        {
            cout << "--slaves is requered" << endl;
            return 1;
        }
    }
        
    if (cmd == CMD_SWITCHOVER || cmd == CMD_DEMOTE_TO_SLAVE)
    {
        if (pm.newMaster.host == _T(""))
        {
            cout << "--new_master is requered" << endl;
            return 1;
        }
        if (pm.newMaster.repUser == "")
        {
            cout << "--repl_user is requered" << endl;
            return 1;
        }
        if (pm.newMaster.repPasswd == "")
        {
            cout << "--repl_passwd is requered" << endl;
            return 1;
        }
    }
    return 0;
}

int getCommandLineOption(int argc, _TCHAR* argv[], failOverParam& pm, int& cmd, int& v)
{
    std::string c, host, newMaster, user, pwd, slaves; 
    std::string port = "3306"; 
    bool readonly = false;
    bool disable_demote = false;
    options_description opt("command line option");
    opt.add_options()("command,c", value<std::string>(&c),
                      "command [switchover | failover | demote_to_slave | set_failover_enable | set_server_role | health_check]")
        ("cur_master,o", value<std::string>(&host), "current master host name")
        ("new_master,n", value<std::string>(&newMaster), "new master host name")
        ("channel,C", value<std::string>(&pm.newMaster.channel), "new master channel name")
        ("repl_port,P", value<std::string>(&pm.newMaster.repPort), "new master port")
        ("repl_user,r", value<std::string>(&pm.newMaster.repUser), "new master repl user ")
        ("repl_passwd,d", value<std::string>(&pm.newMaster.repPasswd), "new master repl password ")
        ("repl_option,O", value<std::string>(&pm.newMaster.repOption),  "option params for change master(ex: MASTER_CONNECT_RETRY=30)")
        ("slaves,s", value<std::string>(&slaves), "slave list for failover")
        ("portmap,a", value<std::string>(&pm.portMap), "port map ex:3307:8611")
        ("value,v", value<int>(&v), "value (For set_failover_enable or set_server_role)")
        ("username,u", value<std::string>(&user), "transactd username")
        ("password,p", value<std::string>(&pwd),  "transactd password")
        ("readonly,R", value<bool>(&readonly),  "0 | 1: When it is 1, the READONLY variable will be set ON to slaves and OFF to a master")
        ("disable_demote,D", value<bool>(&disable_demote),  "0 | 1: disable old master demote");
    variables_map values;
    store(parse_command_line(argc, argv, opt), values);
    notify(values);
    if (!values.count("command"))
    {
        cout << opt << endl;
        return 1;
    }
    vector<string> cmdList;
    split(cmdList, cmds, ",");
    vector<string>::iterator it = find(cmdList.begin(), cmdList.end(), c);
    if (it== cmdList.end())
    {
        cout << opt << endl;
        return 1;
    }
    cmd = (int)(it - cmdList.begin());
    if ((cmd == CMD_SET_SERVER_ROLE || cmd == CMD_SET_FAILOVER_ENABLE) &&
            !values.count("value"))
    {
        cout << opt << endl;
        return 1;
    }
    pm.newMaster.host = str_conv(newMaster);
    pm.master.host = str_conv(host);
    pm.master.user = str_conv(user);
    pm.master.passwd = str_conv(pwd);
    pm.slaves = str_conv(slaves);
    if (readonly) pm.option |= OPT_READONLY_CONTROL;
    if (disable_demote) pm.option |= OPT_DISABLE_OLD_TO_SLAVE;
    return checkParam(pm, cmd, v);
}

void printDateTime()
{
    btrDateTime bt;
    char tmp[256];
    bt.time.i = getNowTime();
    bt.date.i = getNowDate();
    btrstoa(bt, tmp, true);
    cout << tmp;
}

void printMessage(const char* msg, const char* msg2="")
{
    printDateTime();
    cout << " " << msg << msg2 << endl;
}

#ifdef _UNICODE
void printMessage(const wchar_t* msg, const wchar_t* msg2=_T(""))
{
    printDateTime();
    tcout << " " << msg << msg2 << endl;
}
#endif

/* Command line paramater 
command line option:
  -c [ --command ]         command [switchover | failover | demote_to_slave | set_failover_enable | set_server_role | health_check]
  -o [ --cur_master ]      current master host name
  -n [ --new_master ]      new master host name
  -C [ --channel ]         new master channel name
  -P [ --repl_port ]       new master port
  -r [ --repl_user ]       new master repl user
  -d [ --repl_passwd ]     new master repl password
  -O [ --repl_option ]     option params for change master(ex:MASTER_CONNECT_RETRY=30)
  -s [ --slaves ]          slave list for failover
  -a [ --portmap ]         port map ex:3307:8611
  -v [ --value ]           value (For set_failover_enable or set_server_role)
  -u [ --username ]        transactd username
  -p [ --password ]        transactd password
  -R [ --readonly ]        0 | 1: When it is 1, the READONLY variable will be set ON to slaves and OFF to a master
  -D [ --disable_demote ]  0 | 1: disable old master demote

example:
haMgr64 -c switchover -o localhost -n localhost:8611 -P 3307 -r replication_user -d xxxx -u root -p xxxx 

haMgr64 -c failover -s localhost:8611,localhost:8612 -a 8610:3306,8611:3307,8612:3308
 -u root -p

haMgr64 -c demote_to_slave -o localhost -n localhost:8611 -P 3307 -r replication_user -d xxxx
 -d abcd -u root -p 

haMgr64 -c set_failover_enable -v 1 -o localhost -u root -p

haMgr64 -c set_server_role -o localhost -v 1 -u root -p

haMgr64 -c health_check -o localhost -s localhost:8611,localhost:8612 -u root -p

*/

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _WIN32
    // set locale to current user locale.
    std::locale::global(std::locale(""));
#endif
    int ret = 1;
    try
    {
        failOverParam pm;
        int cmd = 0;
        int v = 0;
        ret = getCommandLineOption(argc, argv, pm, cmd, v);
        if (ret == 0)
        {
            haMgrNotify nf;
            switch (cmd)
            {
            case CMD_SWITCHOVER: 
                pm.option |= OPT_SO_AUTO_SLVAE_LIST;
                printMessage("Starting switch over...");
                switchOrver(pm, &nf); 
                break;
            case CMD_FAILOVER:
                printMessage("Starting fail over...");
                failOrver(pm, &nf); 
                break;
            case CMD_DEMOTE_TO_SLAVE:  
                printMessage("Starting demote to slave...");
                demoteToSlave(pm, &nf); 
                break;
            case CMD_SET_FAILOVER_ENABLE: 
                pm.option |= OPT_SO_AUTO_SLVAE_LIST;
                setEnableFailOver(pm, v == 1); 
                break;
            case CMD_SET_SERVER_ROLE:  
                setServerRole(pm, v); 
                break;
            case CMD_HEALTH_CHECK:
                printMessage("Starting health check...");
                int errors = healthCheck(pm, &nf);
                char tmp[256];
                if (errors)
                    sprintf_s(tmp, 256, "%d errors detected.", errors);
                else
                    sprintf_s(tmp, 256, "No errors detected.");
                cout << endl << "  *** " << tmp << endl << endl;
                ret = errors;
                break;
            }
            printMessage("Done!");
        }
    }
    catch (bzs::rtl::exception& e)
    {
        printMessage(_T("Error! "), getMsg(e)->c_str());
        ret = 1;
    }
    catch (std::exception& e)
    {
        printMessage("Error! ", e.what());
        ret = 1;
    }
    return ret;
}
//---------------------------------------------------------------------------
