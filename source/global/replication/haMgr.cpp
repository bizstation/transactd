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

#ifdef __BCPLUSPLUS__
#define BZS_LINK_BOOST_PROGRAM_OPTIONS
#include <bzs/env/boost_bcb_link.h>
#endif

using namespace bzs::db::protocol::tdap::client;
using namespace boost::program_options;
using namespace std;

static const char* cmds = "switchover,failover,master_to_slave,"
                            "set_failover_enable,set_server_role";
#define CMD_SWITCHOVER            0
#define CMD_FAILOVER              1
#define CMD_MASTER_TO_SLAVE       2
#define CMD_SET_FAILOVER_ENABLE   3
#define CMD_SET_SERVER_ROLE       4


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

int checkParam(failOverParam& pm, int cmd, int v)
{
    if (cmd != CMD_FAILOVER)
    {
        if (pm.master.host == _T(""))
        {
            std::cout << "--cur_master is requered" << std::endl;
            return 1;
        }
    }else
    {
        if (pm.slaves == _T(""))
        {
            std::cout << "--slaves is requered" << std::endl;
            return 1;
        }
    }
        
    if (cmd == CMD_SWITCHOVER || cmd == CMD_MASTER_TO_SLAVE)
    {
        if (pm.newMaster.host == _T(""))
        {
            std::cout << "--new_master is requered" << std::endl;
            return 1;
        }
        if (pm.newMaster.repPort == "")
        {
            std::cout << "--repl_port is requered" << std::endl;
            return 1;
        }
        if (pm.newMaster.repUser == "")
        {
            std::cout << "--repl_user is requered" << std::endl;
            return 1;
        }
    }
    return 0;
}

int getCommandLineOption(int argc, _TCHAR* argv[], failOverParam& pm, int& cmd, int& v)
{
    std::string c, host, newMaster, port, user, pwd, slaves;    
    options_description opt("command line option");
    opt.add_options()("command,c", value<std::string>(&c),
                      "command [switchover | failover | master_to_slave | set_failover_enable | set_server_role]")
        ("cur_master,m", value<std::string>(&host), "current master host name")
        ("new_master,n", value<std::string>(&newMaster), "new master host name")
        ("repl_port,t", value<std::string>(&pm.newMaster.repPort), "new master port")
        ("repl_user,r", value<std::string>(&pm.newMaster.repUser), "new master repl user ")
        ("repl_passwd,d", value<std::string>(&pm.newMaster.repPasswd), "new master repl password ")
        ("slaves,s", value<std::string>(&slaves), "slave list for failover")
        ("portmap,a", value<std::string>(&pm.portMap), "port map ex:3307:8611")
        ("value,v", value<int>(&v), "value (For set_failover_enable or set_server_role)")
        ("username,u", value<std::string>(&user), "transactd username")
        ("password,p", value<std::string>(&pwd),  "transactd password");
    variables_map values;
    store(parse_command_line(argc, argv, opt), values);
    notify(values);
    if (!values.count("command"))
    {
        std::cout << opt << std::endl;
        return 1;
    }
    vector<string> cmdList;
    split(cmdList, cmds, ",");
    vector<string>::iterator it = find(cmdList.begin(), cmdList.end(), c);
    if (it== cmdList.end())
    {
        std::cout << opt << std::endl;
        return 1;
    }
    cmd = (int)(it - cmdList.begin());
    if ((cmd == CMD_SET_SERVER_ROLE || cmd == CMD_SET_FAILOVER_ENABLE) &&
            !values.count("value"))
    {
        std::cout << opt << std::endl;
        return 1;
    }
    pm.newMaster.host = str_conv(newMaster);
    pm.master.host = str_conv(host);
    pm.master.user = str_conv(user);
    pm.master.passwd = str_conv(pwd);
    pm.slaves = str_conv(slaves);
    return checkParam(pm, cmd, v);
}

/* Command line paramater example

ha -c switchover -m localhost -n localhost:8611 -t 3307 -r replication_user
 -d abcd -u root -p 

ha -c failover -s localhost:8611,localhost:8612 -a 8610:3306,8611:3307,8612:3308
 -u root -p

ha -c master_to_slave -m localhost -n localhost:8611 -t 3307 -r replication_user
 -d abcd -u root -p 

ha -c set_failover_enable -v 1 -m localhost -u root -p

ha -c set_server_role -m localhost -v 1 -u root -p
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
            switch (cmd)
            {
            case CMD_SWITCHOVER: 
                pm.option |= OPT_SO_AUTO_SLVAE_LIST;
                switchOrver(pm); 
                break;
            case CMD_FAILOVER:
                failOrver(pm); 
                break;
            case CMD_MASTER_TO_SLAVE:  
                masterToSlave(pm); 
                break;
            case CMD_SET_FAILOVER_ENABLE: 
                pm.option |= OPT_SO_AUTO_SLVAE_LIST;
                setEnableFailOver(pm, v == 1); 
                break;
            case CMD_SET_SERVER_ROLE:  
                setServerRole(pm, v); 
                break;
            }
        }
    }
    catch (bzs::rtl::exception& e)
    {
        _ftprintf(stderr, _T("Error ! %s\n"), getMsg(e)->c_str());
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return ret;
}
//---------------------------------------------------------------------------
