/* =================================================================
 Copyright (C) 2015 BizStation Corp All rights reserved.

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
#include "testbase.h"
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>
#include <bzs/db/protocol/tdap/client/hostNameResolver.h>
#include <limits.h>
#include <stdlib.h>
#include <global/replication/haCommand.h>

#define DBNAMEV3 _T("test_v3")

char host1[128] = {"localhost"};
char host2[128] = {"localhost:8611"};
char host3[128] = {"localhost:8612"};
char g_repUser[128] = "replication_user";
char g_repPasswd[128] = "abcd";

char slaveHostsWithPort[512];
static const char* portMap = "8610:3306,8611:3307,8612:3308";

void init_commandLine(char* argv)
{
    if (strstr(argv, "--host1=") == argv)
        strcpy_s(host1, 128, (argv + 8)); 
    if (strstr(argv, "--host2=") == argv)
        strcpy_s(host2, 128, (argv + 8)); 
    if (strstr(argv, "--host3=") == argv)
        strcpy_s(host3, 128, (argv + 8)); 
    if (strstr(argv, "--repl_user=") == argv)
        strcpy_s(g_repUser, 128, (argv + 12)); 
    if (strstr(argv, "--repl_passwd=") == argv)
        strcpy_s(g_repPasswd, 128, (argv + 14)); 

}

_tstring str_conv(const char* v)
{
#ifdef _UNICODE
    wchar_t wbuf[1024];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, v, -1, wbuf, 1024);
    return wbuf;
#else
    return v;
#endif //_UNICODE
}

const char* getHostList()
{
    sprintf_s(slaveHostsWithPort, 512, "%s,%s,%s", host1, host2, host3);
    return slaveHostsWithPort;
}

const char* g_port[3] = {"3306", "3307", "3308"};
const char* getReplPort(const _tstring& host)
{
    _TCHAR tmp[128];
    _tcscpy_s(tmp ,128, host.c_str());
    int n = 0;
    const _TCHAR* p = _tcsstr(tmp, _T(":"));
    if (p)
        n = _ttol(p+1) - 8610;
    return g_port[n];    
}

void start_Resolver()
{
    const char* slaves = "slave1,slave2";
    const char* slaveHostsWithPort = getHostList();
    addPortMap(3307, 8611);
    addPortMap(3308, 8612);
    string u = toUtf8(g_userName);
    string p = toUtf8(g_password);
    int ret = startResolver("master", slaves, slaveHostsWithPort, 1, u.c_str(), p.c_str());
    BOOST_CHECK_MESSAGE(ret == 0, "startResolver ret = " << ret);
}

void testSetServerRole()
{
    failOverParam pm;
    pm.master.host = str_conv(host1);
    pm.master.user = g_userName;
    pm.master.passwd = g_password;
    try
    {
        setServerRole(pm, HA_ROLE_MASTER);     
        BOOST_CHECK(true);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK_MESSAGE(false, "setServerRole Error");
        _tprintf(getMsg(e)->c_str());
    }
}

void testNameResover()
{
    start_Resolver();
    BOOST_CHECK_MESSAGE(std::string(slaveHost()) == host2, slaveHost());
    BOOST_CHECK_MESSAGE(std::string(masterHost()) == host1, masterHost());

    database_ptr db = createDatabaseObject();
    bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == true);
    table* tb = db->openTable(_T("fieldtest"));
    BOOST_CHECK(db->stat() == 0);
    tb->seekFirst();
    tb->setFV(_T("int_1_byte"), 1);
    tb->update();
    BOOST_CHECK(tb->stat() == 0);
    db->close();
    ret = db->open(makeUri(PROTOCOL, _T("slave1"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == true);
    db->close();

    clearResolver();
    BOOST_CHECK(std::string(slaveHost()) == "");
    BOOST_CHECK(std::string(masterHost()) == "");
    stopResolver();
    ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == false);
}

void makeSOParam(failOverParam& pm, const _tstring& master, const _tstring& newMaster)
{
    pm.master.host = master;
    pm.master.user = g_userName;
    pm.master.passwd = g_password;
    pm.newMaster.host = newMaster;
    pm.newMaster.repPort = getReplPort(newMaster);
    pm.newMaster.repUser = g_repUser;
    pm.newMaster.repPasswd = g_repPasswd;
}

void testSwitchTo(const _tstring& master, const _tstring& newMaster)
{ 
    try
    {
        stopResolver();
        failOverParam pm;
        pm.option |= OPT_SO_AUTO_SLVAE_LIST;
        makeSOParam(pm,master, newMaster);
        switchOrver(pm);
        BOOST_CHECK(true);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(getMsg(e)->c_str());
    }
}

void testNameResover8611()
{
    start_Resolver();
    bool ret = std::string(slaveHost()) == host2 || std::string(slaveHost()) == host3;
    BOOST_CHECK_MESSAGE(ret = true, slaveHost());
    BOOST_CHECK_MESSAGE(std::string(masterHost()) == host2, masterHost());
    database_ptr db = createDatabaseObject();
    ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == true);
    db->close();
    ret = db->open(makeUri(PROTOCOL, _T("slave1"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == true);
    db->close();

    clearResolver();
    BOOST_CHECK(std::string(slaveHost()) == "");
    BOOST_CHECK(std::string(masterHost()) == "");
    stopResolver();
    ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
    BOOST_CHECK(ret == false);
}

void testSwitchLiveSeek()
{
    database_ptr db = createDatabaseObject();
    try
    {
        start_Resolver();
        bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
        BOOST_CHECK(db->stat() == 0);
        if (ret)
        {
            table* tb = db->openTable(_T("fieldtest"));
            BOOST_CHECK(db->stat() == 0);
            table* tb2 = db->openTable(_T("timetest"));
            BOOST_CHECK(db->stat() == 0);
            if (tb && tb2)
            {
                tb->seekFirst();
                tb2->seekFirst();
                failOverParam pm;
                pm.option |= OPT_SO_AUTO_SLVAE_LIST;
                makeSOParam(pm, str_conv(host2), str_conv(host3));
                switchOrver(pm);
                BOOST_CHECK(true);

                tb->seekNext();
                BOOST_CHECK(tb->stat() == 0);
                BOOST_CHECK(tb->getFVint(_T("id")) == 2);
                tb2->seekNext();
                BOOST_CHECK(tb2->stat() == 0);
                BOOST_CHECK(tb2->getFVint(_T("id")) == 2);
                tb->release();
                tb2->release();
            }
        }
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(getMsg(e)->c_str());
    }
}

void testSwitchLiveOpen()
{
    try
    {
        database_ptr db = createDatabaseObject();
        start_Resolver();
        failOverParam pm;
        pm.option |= OPT_SO_AUTO_SLVAE_LIST;
        makeSOParam(pm, str_conv(host3), str_conv(host1));
        switchOrver(pm);
        bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
        BOOST_CHECK(db->stat() == 0);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(getMsg(e)->c_str());
    }
}

void testEnableFailOver(bool v)
{
    failOverParam pm;
    pm.master.host = str_conv(host1);
    pm.master.user = g_userName;
    pm.master.passwd = g_password;
    pm.option |= OPT_SO_AUTO_SLVAE_LIST;
    try
    {
        setEnableFailOver(pm, v);        
        BOOST_CHECK(true);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK_MESSAGE(false, "setEnableFailOver Error");
        _tprintf(getMsg(e)->c_str());
    }
}

void testFailOverBlock()
{
    database_ptr db = createDatabaseObject();
    start_Resolver();
    failOverParam pm;
    pm.master.user = g_userName;
    pm.master.passwd = g_password;
    char tmp[512];
    sprintf_s(tmp, 512, "%s,%s", host2, host3);
    pm.slaves = str_conv(tmp);
    pm.portMap = portMap;
    try
    {
        failOrver(pm);
        bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
        BOOST_CHECK_MESSAGE(false, "failorver not block");
    }
    catch (bzs::rtl::exception& /*e*/)
    {
        BOOST_CHECK(true);
        //_tprintf(getMsg(e)->c_str());
    }
}

void testFailOver8611()
{
    database_ptr db = createDatabaseObject();
    start_Resolver();
    failOverParam pm;
    pm.master.user = g_userName;
    pm.master.passwd = g_password;
    char tmp[512];
    sprintf_s(tmp, 512, "%s,%s", host2, host3);
    pm.slaves = str_conv(tmp);
    pm.portMap = portMap;
    try
    {
        failOrver(pm);
        bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
        BOOST_CHECK(db->stat() == 0);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK_MESSAGE(false, "fail orver");
        _tprintf(getMsg(e)->c_str());
    }
}

void testMasterToSlave()
{
    database_ptr db = createDatabaseObject();
    start_Resolver();
    failOverParam pm;
    makeSOParam(pm, str_conv(host1),  str_conv(host2));
    try
    {
        demoteToSlave(pm);
        bool ret = db->open(makeUri(PROTOCOL, _T("master"), DBNAMEV3, BDFNAME));
        BOOST_CHECK(db->stat() == 0);
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(getMsg(e)->c_str());
    }

}

BOOST_AUTO_TEST_SUITE(ha_default)
BOOST_AUTO_TEST_CASE(NameResover)
{
    testSetServerRole();
    testNameResover();
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(SwitchOver)
BOOST_AUTO_TEST_CASE(Switch8611)
{
    testSwitchTo(str_conv(host1), str_conv(host2));
}

BOOST_AUTO_TEST_CASE(NameResover8611)
{
    testNameResover8611();
}

BOOST_AUTO_TEST_CASE(Switch8612)
{
    testSwitchLiveSeek();
}
BOOST_AUTO_TEST_CASE(Switch8610)
{
    testSwitchLiveOpen();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(FailOver)
BOOST_AUTO_TEST_CASE(FailOver8611)
{
    testEnableFailOver(false);
    testFailOverBlock();
    testEnableFailOver(true);
    testFailOver8611();
    testMasterToSlave();
    testSwitchTo(str_conv(host2), str_conv(host1));
}
BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------
