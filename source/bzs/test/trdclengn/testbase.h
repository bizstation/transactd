#ifndef BZS_TEST_TRDCLENGN_TESTBASE_H
#define BZS_TEST_TRDCLENGN_TESTBASE_H
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
//#include <boost/test/test_tools.hpp>

#if defined(__BCPLUSPLUS__)
#pragma warn -8012
#pragma warn -8022
#endif
#include <boost/test/included/unit_test.hpp>
#ifndef BOOST_TEST_MESSAGE
#define BOOST_TEST_MESSAGE BOOST_MESSAGE
#endif
#if defined(__BCPLUSPLUS__)
#pragma warn .8012
#pragma warn .8022
#endif

#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/client/stringConverter.h>
#include <stdio.h>
#include <bzs/db/protocol/tdap/client/filter.h>
#include <bzs/example/queryData.h>
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <bzs/db/protocol/tdap/myDateTime.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace std;

#define TDAP
#ifdef TDAP
#define PROTOCOL _T("tdap")
#else
#define PROTOCOL _T("btrv")
#endif
static _TCHAR HOSTNAME[MAX_PATH] = { _T("127.0.0.1") };
#define DBNAME _T("test")
#define BDFNAME _T("test")
#define ISOLATION_READ_COMMITTED

static _TCHAR g_uri[MAX_PATH];
static _TCHAR g_userName[MYSQL_USERNAME_MAX + 1]={0x00};
static _TCHAR g_password[MAX_PATH]={0x00};

const _TCHAR* makeUri(const _TCHAR* protocol, const _TCHAR* host,
                      const _TCHAR* dbname, const _TCHAR* dbfile=_T(""))
{
    connectParams cp(protocol, host, dbname, dbfile, g_userName, g_password);
    _tcscpy_s(g_uri, 260, cp.uri());
    return g_uri;
}

static bool use_nullfield = true;
static bool use_mysqlNullMode = true;

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]);

void init_commandLine(char* argv);


boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (strstr(argv[i], "--host=") == argv[i])
        {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP,
                                (CP_ACP == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                                argv[i] + 7, -1, HOSTNAME, MAX_PATH);
#else
            strcpy_s(HOSTNAME, MAX_PATH, argv[i] + 7);
#endif
        }
        if (strstr(argv[i], "--user=") == argv[i])
        {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP,
                                (CP_ACP == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                                argv[i] + 7, -1, g_userName, MYSQL_USERNAME_MAX+1);
#else
            strcpy_s(g_userName, MYSQL_USERNAME_MAX+1, argv[i] + 7);
#endif        
        }
        
        if (strstr(argv[i], "--pwd=") == argv[i])
        {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP,
                                (CP_ACP == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                                argv[i] + 6, -1, g_password, MAX_PATH);
#else
            strcpy_s(g_password, MAX_PATH, argv[i] + 6);
#endif        
        }
        if (strstr(argv[i], "--nullfield=") == argv[i])
            use_nullfield = atol(argv[i] + 12) != 0;  
        if (strstr(argv[i], "--mysqlnull=") == argv[i])
            use_mysqlNullMode = atol(argv[i] + 12) != 0;

        init_commandLine(argv[i]);

    }
    printf("Transactd test ... \nMay look like progress is stopped, \n"
            "but it is such as record lock test, please wait.\n");
    if (!use_mysqlNullMode)
        database::setCompatibleMode(database::CMP_MODE_OLD_NULL);
    return 0;
}

#ifdef _WIN32
    #ifdef _UNICODE
        static const uchar_td g_td_charsetIndex = CHARSET_UTF8;
    #else
        static const uchar_td g_td_charsetIndex = CHARSET_CP932;
    #endif
#else
    static const uchar_td g_td_charsetIndex = CHARSET_UTF8;
#endif

#endif // BZS_TEST_TRDCLENGN_TESTBASE_H