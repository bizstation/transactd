/* =================================================================
 Copyright (C) 2013 BizStation Corp All rights reserved.

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
//#define BOOST_TEST_MODULE

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
#include <bzs/db/protocol/tdap/client/pooledDatabaseManager.h>
#include <boost/thread.hpp>


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
// #define ISOLATION_REPEATABLE_READ
#define ISOLATION_READ_COMMITTED

static _TCHAR g_uri[MAX_PATH];
static _TCHAR g_userName[MYSQL_USERNAME_MAX + 1]={0x00};
static _TCHAR g_password[MAX_PATH]={0x00};

static const short fdi_id = 0;
static const short fdi_name = 1;

static bool use_nullfield = false;
static bool use_mysqlNullMode = false;


boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]);

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
    }
    printf("Transactd test ... \nMay look like progress is stopped, \n"
            "but it is such as record lock test, please wait.\n");
    if (!use_mysqlNullMode)
        database::setCompatibleMode(database::CMP_MODE_OLD_NULL);
    return 0;
}



const _TCHAR* makeUri(const _TCHAR* protocol, const _TCHAR* host,
                      const _TCHAR* dbname, const _TCHAR* dbfile=_T(""))
{
    connectParams cp(protocol, host, dbname, dbfile, g_userName, g_password);
    _tcscpy_s(g_uri, 260, cp.uri());
    return g_uri;
}

class fixture
{
    mutable database* m_db;

public:
    fixture() : m_db(NULL)
    {

        nsdatabase::setCheckTablePtr(true);
        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
    }

    ~fixture()
    {
        if (m_db)
        {
            // Test for SWIG interface
            m_db->release();
            // Test for c++
            // database::destroy(m_db);
        }
    }

    ::database* db() const { return m_db; }
};

#ifdef _WIN32
class fixtureKanji
{
    mutable database* m_db;

public:
    fixtureKanji() : m_db(NULL)
    {
        nsdatabase::setExecCodePage(932);
        nsdatabase::setCheckTablePtr(true);
        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
    }

    ~fixtureKanji()
    {
        if (m_db)
            m_db->release();
    }
    ::database* db() const { return m_db; }
};
#else
typedef fixture fixtureKanji;
#endif




class fixtureQuery
{
    database_ptr m_db;

public:
    fixtureQuery()
    {
        m_db = createDatabaseObject();
        if (!m_db)
            printf("Error database::create()\n");
        connectParams param(PROTOCOL, HOSTNAME, _T("querytest"),
                            _T("test"), g_userName, g_password);
        param.setMode(TD_OPEN_NORMAL);

        prebuiltData(m_db, param);
    }

    ~fixtureQuery() {}

    database* db() const { return m_db.get(); }
};

table* openTable(database* db, short dbmode = TD_OPEN_NORMAL,
                 short tbmode = TD_OPEN_NORMAL)
{

    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,
             dbmode);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    table* tb = db->openTable(_T("user"), tbmode);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    return tb;
}

void testDropDatabase(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "drop stat = " << db->stat());
}

void testClone(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);

    database* db2 = db->clone();
    BOOST_CHECK_MESSAGE(db2 != NULL, "createNewDataBase stat = " << db->stat());
    db2->close();
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,
              TD_OPEN_NORMAL);

    BOOST_CHECK_MESSAGE(db2->stat() == 0, "db2 close stat = " << db2->stat());
    db->close();
    BOOST_CHECK_MESSAGE(db->stat() == 0, "db close stat = " << db->stat());
    table* tb = db2->openTable(_T("user"), TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "openTable" << db2->stat());
    if (db2)
        db2->release();
    bool ret = nsdatabase::testTablePtr(tb);
    BOOST_CHECK_MESSAGE(ret == true, "testTablePtr");
    tb->release();

    ret = nsdatabase::testTablePtr(tb);
    BOOST_CHECK_MESSAGE(ret == false, "testTablePtr");
}


#define NAMEFIELD_TYPE ft_myvarbinary 
void testCreateNewDataBase(database* db)
{

    db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        testDropDatabase(db);
        db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    }
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createNewDataBase stat = " << db->stat());
    // create table
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createNewDataBase 1 stat = " << db->stat());
    _TCHAR buf[255];
    db->readDatabaseDirectory(buf, 255);
    BOOST_CHECK_MESSAGE(_tstring(buf) != _tstring(_T("")), "readDatabaseDirectory" << buf);

    dbdef* def = db->dbDef();
    if (def)
    {
        /*  user table */
        tabledef td;
        td.setTableName(_T("user"));
        td.setFileName(_T("user.dat"));
        td.id = 1;

#ifdef _WIN32
        td.charsetIndex = CHARSET_CP932;
#else
        td.charsetIndex = CHARSET_UTF8;
#endif

        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "insertTable stat = " << def->stat());

        fielddef* fd = def->insertField(1, 0);
        fd->setName(_T("id"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 1 stat = " << def->stat());

        fd = def->insertField(1, 1);
        fd->setName(_T("name"));
        fd->len = (ushort_td)33;

        //test padChar only string or wstring
        fd->type = ft_string;
        fd->setPadCharSettings(true, false);
        BOOST_CHECK(fd->isUsePadChar() ==  true);
        BOOST_CHECK(fd->isTrimPadChar() == false);
        fd->setPadCharSettings(false, true);
        BOOST_CHECK(fd->isUsePadChar() ==  false);
        BOOST_CHECK(fd->isTrimPadChar() == true);

        //fd->type = ft_zstring;
        fd->type = NAMEFIELD_TYPE;
        fd->setNullable(use_nullfield);
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 2 stat = " << def->stat());

        fd = def->insertField(1, 2);
        fd->setName(_T("select"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        fd->setNullable(use_nullfield);
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 2 stat = " << def->stat());

        fd = def->insertField(1, 3);
        fd->setName(_T("in"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        fd->setNullable(use_nullfield);
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 2 stat = " << def->stat());

        keydef* kd = def->insertKey(1, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 3 stat = " << def->stat());

        /*  group table */
        memset(&td, 0, sizeof(tabledef));
        td.setTableName(_T("group"));
        td.setFileName(_T("group"));
        td.id = 2;
        td.primaryKeyNum = -1;
        td.parentKeyNum = -1;
        td.replicaKeyNum = -1;
        td.pageSize = 2048;
        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "insertTable stat = " << def->stat());

        fd = def->insertField(2, 0);
        fd->setName(_T("id"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(2);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 1 stat = " << def->stat());

        fd = def->insertField(2, 1);
        fd->setName(_T("name"));
        fd->type = ft_zstring;
        fd->len = (ushort_td)33;
        def->updateTableDef(2);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 2 stat = " << def->stat());

        kd = def->insertKey(2, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        def->updateTableDef(2);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 3 stat = " << def->stat());
    }
}

void testVersion(database* db)
{
    db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "Version connect stat = " << db->stat());
    if (0 == db->stat())
    {
        btrVersions vv;
        db->getBtrVersion(&vv);
        BOOST_CHECK_MESSAGE(0 == db->stat(), "Version");
        if (_tcscmp(PROTOCOL, _T("tdap")) == 0)
        {
            BOOST_CHECK_MESSAGE(
                atoi(CPP_INTERFACE_VER_MAJOR) == vv.versions[0].majorVersion,
                "clent_Major = " << vv.versions[0].majorVersion);
            BOOST_CHECK_MESSAGE(
                atoi(CPP_INTERFACE_VER_MINOR) == vv.versions[0].minorVersion,
                "clent_Miner = " << vv.versions[0].minorVersion);
            BOOST_CHECK_MESSAGE((int)'N' == (int)vv.versions[0].type,
                                "clent_Type = " << vv.versions[0].type);

            BOOST_CHECK_MESSAGE(
                ((5 == vv.versions[1].majorVersion) ||
                 (10 == vv.versions[1].majorVersion)),
                "mysql_server_Major = " << vv.versions[1].majorVersion);
            BOOST_CHECK_MESSAGE(
                ((5 <= vv.versions[1].minorVersion) ||
                 (1 >= vv.versions[1].minorVersion)),
                "mysql_server_Miner = " << vv.versions[1].minorVersion);
            BOOST_CHECK_MESSAGE((int)'M' == (int)vv.versions[1].type,
                                "mysql_server_Type = " << vv.versions[1].type);

            BOOST_CHECK_MESSAGE(
                TRANSACTD_VER_MAJOR == vv.versions[2].majorVersion,
                "server_Major = " << vv.versions[2].majorVersion);
            BOOST_CHECK_MESSAGE(
                TRANSACTD_VER_MINOR == vv.versions[2].minorVersion,
                "server_Miner = " << vv.versions[2].minorVersion);
            BOOST_CHECK_MESSAGE((int)'T' == (int)vv.versions[2].type,
                                "server_Type = " << vv.versions[2].type);
        }
    }
}

void testInsert(database* db)
{
    table* tb = openTable(db);

    if (tb->recordCount() == 0)
    {
        tb->clearBuffer();
        tb->setFV((short)0, _T("1"));
        tb->setFV((short)1, _T("kosaka"));
        tb->insert();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert");
    }
    //test invalid keyNum
    tb->clearBuffer();
    tb->setFV((short)0, _T("2"));
    tb->setKeyNum(10);
    tb->insert();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum insert");
    tb->setKeyNum(0);

    db->beginTrn();
    int n = 1;
    tb->seekLast();
    if (tb->stat() == 0)
        n = tb->getFVint(fdi_id) + 1;
    tb->beginBulkInsert(BULKBUFSIZE);
    for (int i = n; i < 20002 + n - 1; i++)
    {
        tb->clearBuffer();
        tb->setFV((short)0, i);
        tb->setFV((short)1, i);
        tb->insert();
    }
    tb->commitBulkInsert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "Insert2");
    db->endTrn();

    uint_td v = tb->recordCount(false);
    BOOST_CHECK_MESSAGE(20002 == v,
                        "RecordCount count = " << v);

    tb->release();
}

void findNextLoop(table* tb, int start, int end)
{
    while (start < end)
    {
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "find stat = " << tb->stat());
        if (tb->stat()) break;
            
        BOOST_CHECK_MESSAGE(start == tb->getFVint(fdi_id), "find value " << start << " bad = " << tb->getFVint(fdi_id));
        tb->findNext(true); 
        ++start;
    }
    BOOST_CHECK_MESSAGE(0 != tb->stat(), "findNext end stat = " << tb->stat());
}

void testFind(database* db)
{

    table* tb = openTable(db);

    //test invalid keyNum
    tb->clearBuffer();
    tb->setKeyNum(10);
    tb->setFilter(_T("id >= 10 and id < 20000"), 1, 0);
    int v = 10;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum find");

   
    tb->setKeyNum(0);
    tb->clearBuffer();
    tb->setFilter(_T("id >= 10 and id < 20000"), 1, 0);
    v = 10;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    findNextLoop(tb, v, 20000);
    
    // backforword
    tb->clearBuffer();
    v = 19999;
    tb->setFV((short)0, v);
    tb->find(table::findBackForword);
    int i = v;
    while (i >= 10)
    {
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "find stat = " << tb->stat());
        if (tb->stat()) break;
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "find value " << i);
        tb->findPrev(true); // 11 ～ 19
        --i;
    }

    tb->clearBuffer();
    v = 20000;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    BOOST_CHECK_MESSAGE(STATUS_EOF == tb->stat(), "find stat");
    tb->release();
}

void testFindNext(database* db)
{
    table* tb = openTable(db);
    tb->setKeyNum(0);
    tb->clearBuffer();
    tb->setFilter(_T("id >= 10 and id < 20000"), 1, 0);
    int v = 10;
    tb->setFV((short)0, v);
    tb->seekGreater(true);
    BOOST_CHECK_MESSAGE(v == tb->getFVint(fdi_id), "findNext GetGreater");
    for (int i = v + 1; i < 20000; i++)
    {
        tb->findNext(true); // 11 ～ 19
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "findNext stat()" << tb->stat());
        if (tb->stat()) break;
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "findNext value");
    }
    tb->release();
}

void testFindIn(database* db)
{

    table* tb = openTable(db);

    //test invalid keyNum
    tb->clearBuffer();
    queryBase q;
    q.addSeekKeyValue(_T("10"), true);
    tb->setQuery(&q);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "find in stat = " << tb->stat());
    tb->setKeyNum(10);
    tb->find();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum seekKeyValue");


    tb->setKeyNum(0);
    tb->clearBuffer();
    q.reset();
    q.addSeekKeyValue(_T("10"), true);
    q.addSeekKeyValue(_T("300000"));
    q.addSeekKeyValue(_T("50"));
    q.addSeekKeyValue(_T("-1"));
    q.addSeekKeyValue(_T("80"));
    q.addSeekKeyValue(_T("5000"));

    tb->setQuery(&q);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "find in stat = " << tb->stat());
    tb->find();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "find in stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(tb->getFVint(fdi_id) == 10, "find in 10");
    tb->findNext();
    BOOST_CHECK_MESSAGE(tb->stat() == 4, "find in 300000 stat =" << tb->stat());

    _TCHAR msg[1024];
    tb->keyValueDescription(msg, 1024);
    int comp = _tcscmp(_T("table:user\nstat:4\nid = 300000\n"), msg);
    BOOST_CHECK_MESSAGE(comp == 0, "find in keyValueDescription");

    tb->findNext();
    BOOST_CHECK_MESSAGE(tb->getFVint(fdi_id) == 50, "find in 50");
    tb->findNext();
    BOOST_CHECK_MESSAGE(tb->stat() == 4, "find in -1");

    tb->keyValueDescription(msg, 1024);
    comp = _tcscmp(_T("table:user\nstat:4\nid = -1\n"), msg);
    BOOST_CHECK_MESSAGE(comp == 0, "find in keyValueDescription");

    tb->findNext();
    BOOST_CHECK_MESSAGE(tb->getFVint(fdi_id) == 80, "find in 80");
    tb->findNext();
    BOOST_CHECK_MESSAGE(tb->getFVint(fdi_id) == 5000, "find in 5000");
    tb->findNext();
    BOOST_CHECK_MESSAGE(STATUS_EOF == tb->stat(), "find in more");

    // Many params
    _TCHAR buf[20];
    for (int j = 1; j <= 10000; ++j)
    {
        _ltot_s(j, buf, 20, 10);
        q.addSeekKeyValue(buf, (j == 1)/* reset */);
    }
    tb->setQuery(&q);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "find in stat = " << tb->stat());
    int i = 0;
    tb->find();
    
    while (0 == tb->stat())
    {
        ++i;
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "findNext in value " << i << " = " << tb->getFVint(fdi_id));
        if (i==9999)
            i = 9999;
        tb->findNext(true);
    }
    BOOST_CHECK_MESSAGE(i == 10000, "findNext in count 10000 !=  " << i);
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "find in end stat = " << tb->stat());

    // LogicalCountLimit
    q.addField(_T("id"));
    tb->setQuery(&q);

    tb->find();
    i = 0;
    while (0 == tb->stat())
    {

        BOOST_CHECK_MESSAGE(++i == tb->getFVint(fdi_id), "findNext in value");
        tb->findNext(true);
    }
    BOOST_CHECK_MESSAGE(i == 10000, "findNext in count");
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "find in end stat = " << tb->stat());

    tb->release();
}

void testPrepare(database* db)
{
    table* tb = openTable(db);
    queryBase q;
    q.queryString(_T("id >= ? and id < ?"));
    q.reject(1).limit(0);
    pq_handle stmt = tb->prepare(&q);
    const _TCHAR* vs[2];
    int nn = makeSupplyValues(vs, 2, _T("10"), _T("20000"));
    
    //Test too short supply values
    bool ret = supplyValues(stmt, vs, nn -1); //stmt->supplyValues(vs, nn); Bad
    BOOST_CHECK_MESSAGE(ret == false, "supplyValues short");
    
    //Test supply values
    ret = supplyValues(stmt, vs, nn);
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues true");
    
    int v = 10;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    findNextLoop(tb, v, 20000);
    
    supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("100"), _T("10000"))); //stmt->supplyValues(vs, nn); Bad
    tb->setPrepare(stmt);
    v = 100;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    findNextLoop(tb, v, 10000);
   
    const _TCHAR* values[11];
    int n = makeSupplyValues(values, 11, _T("abc"), _T("efg"), _T("efg")
                                        , _T("abc"), _T("efg"), _T("efg")
                                        , _T("abc"), _T("efg"), _T("efg")
                                        , _T("abc"), _T("efg"));

    BOOST_CHECK_MESSAGE(n == 11, "makeSupplyValues");

    tb->release();
}

void testPrepareServer(database* db)
{
    table* tb = openTable(db);
    queryBase q;

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->clearBuffer();
    q.queryString(_T("id >= ? and id < ?")).reject(0xFFFF).limit(0);
    pq_handle stmt = tb->prepare(&q, true);
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum prepare");

    tb->setKeyNum(0);
    stmt = tb->prepare(&q, true);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "prepare stat");
    if (tb->stat()) return ;
        
    const _TCHAR* vs[2];
   
    bool ret = supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("10"), _T("200"))); //stmt->supplyValues(vs, nn); Bad
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues true");

    tb->setPrepare(stmt);
    BOOST_CHECK_MESSAGE(tb->stat() == 0, "setQuery stmt");

    int v = 10;
    tb->setFV((short)0, v);

    // Test Bad direction
    tb->find(table::findBackForword); 
    BOOST_CHECK_MESSAGE(1 == tb->stat(), "find direction not equal prepare");
    
    tb->find(table::findForword);
    findNextLoop(tb, v, 200);

    ret = supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("100"), _T("3000"))); 
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues true");

    tb->setPrepare(stmt);
    BOOST_CHECK_MESSAGE(tb->stat() == 0, "setQuery stmt");

    v = 100;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    findNextLoop(tb, v, 3000);
    
    // No seek with
    ret = supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("50"), _T("100"))); 
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues ");
    tb->setPrepare(stmt);
    v = 50;
    tb->setFV((short)0, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seek");
    tb->findNext(false);
    findNextLoop(tb, v, 100);

    // RecordCount
    ret = supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("50"), _T("100"))); 
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues ");
    tb->setPrepare(stmt);
    uint_td num = tb->recordCount(false);
    BOOST_CHECK_MESSAGE(num == 50, "recordCount ");

    // Multi prepare statement
    q.reset();
    q.queryString(_T("id < ?"));
    q.reject(0xFFFF).limit(0);
    pq_handle stmt2 = tb->prepare(&q, true);
    ret = supplyValues(stmt2, vs, makeSupplyValues(vs, 1, _T("50"))); 
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues ");
    tb->setPrepare(stmt2);
    v = 1;
    tb->setFV((short)0, v);
    tb->find(); 
    findNextLoop(tb, v, 50);
    
    ret = supplyValues(stmt, vs, makeSupplyValues(vs, 2, _T("100"), _T("3000"))); 
    BOOST_CHECK_MESSAGE(ret == true, "supplyValues ");
    tb->setPrepare(stmt);
    v = 100;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    findNextLoop(tb, v, 3000);

    tb->release();
}

void testGetPercentage(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();
    int vv = 10001;
    tb->setFV((short)0, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual");
    percentage_td per = tb->getPercentage();

    BOOST_CHECK_MESSAGE(true == (1200 > abs(5000 - per)), "GetPercentage");
    tb->release();
}

void testMovePercentage(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->seekByPercentage(5000); // 50%
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum prepare");

    tb->setKeyNum(0);
    tb->seekByPercentage(5000); // 50%
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "MovePercentage");
    // If mainus is less than 500 then ok.
    BOOST_CHECK_MESSAGE(true == (3000 > abs(10001 - tb->getFVint(fdi_id))),
                        "MovePercentage 1");
    tb->release();
}

void testGetEqual(database* db)
{
    table* tb = openTable(db);

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->clearBuffer();
    tb->setFV((short)0, 10);
    tb->seek();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum seek");

    db->beginSnapshot();
    tb->setKeyNum(0);
    for (int i = 2; i < 20002; i++)
    {
        tb->clearBuffer();
        tb->setFV((short)0, i);

        tb->seek();
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "GetEqual");
    }
    db->endSnapshot();
    tb->release();
}

void testGetNext(database* db)
{
    table* tb = openTable(db);
    for (int j = 0; j < 1; j++)
    {
        db->beginSnapshot();
        int vv = 2;
        tb->clearBuffer();
        tb->setFV(fdi_id, vv);
        tb->seek();
        BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetNext");

        for (int i = 3; i < 20002; i++)
        {
            tb->seekNext();
            BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "GetNext id: " << i << " bad = " << tb->getFVint(fdi_id));
            if (i != tb->getFVint(fdi_id))
                    break;
        }
        db->endSnapshot();
    }
    tb->release();
}

void testGetPrevious(database* db)
{
    table* tb = openTable(db);

    // in-snapshot
    db->beginSnapshot();
    int vv = 20001;
    tb->clearBuffer();
    tb->setFV((short)0, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetPrevious");
    for (int i = 20000; i > 1; i--)
    {
        tb->seekPrev();
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "GetPrevious id: " << i << " bad = " << tb->getFVint(fdi_id));
        if (i != tb->getFVint(fdi_id))
            break;

    }
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(_tstring(_T("kosaka")) == _tstring(tb->getFVstr(1)),
                        "GetPrevious kosaka");
    db->endSnapshot();

    //without snapshot
    vv = 20001;
    tb->clearBuffer();
    tb->setFV((short)0, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetPrevious");
    for (int i = 20000; i > 1; i--)
    {
        tb->seekPrev();
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "GetPrevious id: " << i << " bad = " << tb->getFVint(fdi_id));
        if (i != tb->getFVint(fdi_id))
            break;
    }
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(_tstring(_T("kosaka")) == _tstring(tb->getFVstr(1)),
                        "GetPrevious kosaka");

    tb->release();
}

void testGetGreater(database* db)
{
    table* tb = openTable(db);
    int vv = 15000;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seekGreater(true);
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetGreater true");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(vv + 1 == tb->getFVint(fdi_id), "GetGreater GetNext");

    vv = 14000;
    tb->clearBuffer();
    tb->setFV((short)0, vv);
    tb->seekGreater(false);
    BOOST_CHECK_MESSAGE(vv + 1 == tb->getFVint(fdi_id), "GetGreater false");
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetGreater GetPrevious");
    tb->release();
}

void testGetLessThan(database* db)
{
    table* tb = openTable(db);
    int vv = 15000;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seekLessThan(true);
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetLessThan true");

    tb->seekNext();
    BOOST_CHECK_MESSAGE(vv + 1 == tb->getFVint(fdi_id), "GetLessThan GetNext");

    vv = 14000;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seekLessThan(false);
    BOOST_CHECK_MESSAGE(vv - 1 == tb->getFVint(fdi_id), "GetLessThan false");
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(vv - 2 == tb->getFVint(fdi_id),
                        "GetLessThan GetPrevious");
    tb->release();
}

void testGetFirst(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum seekFirst");

    tb->setKeyNum(0);
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(_tstring(_T("kosaka")) == _tstring(tb->getFVstr(1)),
                        "GetFirst");
    tb->release();
}

void testGetLast(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->seekLast();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), "Invalid keynum seekFirst");
 
    tb->setKeyNum(0);
    tb->seekLast();
    BOOST_CHECK_MESSAGE(20002 == tb->getFVint(fdi_id), "GetLast");
    tb->release();
}

void testMovePosition(database* db)
{
    table* tb = openTable(db);
    int vv = 15000;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seekLessThan(true);
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetLessThan ");

    bookmark_td pos = tb->bookmark();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetBookMark");

    vv = 14000;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seekLessThan(false);
    BOOST_CHECK_MESSAGE(vv - 1 == tb->getFVint(fdi_id), "GetLessThan false");
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(vv - 2 == tb->getFVint(fdi_id),
                        "GetLessThan GetPrevious");

    tb->seekByBookmark(pos);
    BOOST_CHECK_MESSAGE(15000 == tb->getFVint(fdi_id), "MovePosition");

    //test invalid keyNum
    tb->setKeyNum(10);
    tb->seekByBookmark(pos);
    BOOST_CHECK_MESSAGE(STATUS_INVALID_KEYNUM == tb->stat(), 
            "Invalid keynum seekByBookmark stat = " << tb->stat());
 
    tb->release();
}

void testUpdate(database* db)
{
    table* tb = openTable(db);

    db->beginTrn();
    // test of ncc
    int v = 5;
    tb->clearBuffer();
    tb->setFV(fdi_id, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual U");
    v = 30000;
    tb->setFV(fdi_id, v);
    tb->update(table::changeCurrentNcc); // 5 ->  30000 cur 5
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "UpDate00");
    tb->seekNext(); // next 5
    BOOST_CHECK_MESSAGE(6 == tb->getFVint(fdi_id), "UpDate0");
    v = 19999;
    tb->setFV(fdi_id, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(v == tb->getFVint(fdi_id), "UpDate1");
    v = 5;
    tb->setFV(fdi_id, v);
    tb->update(table::changeCurrentCc); // 19999 ->  5 cur 5
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "UpDate11");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "UpDate21");
    BOOST_CHECK_MESSAGE(6 == tb->getFVint(fdi_id), "UpDate2");
    v = 19999;
    tb->setFV(fdi_id, v);
    tb->update(table::changeCurrentCc); // 6 ->  19999 cur 19999
    tb->seekPrev(); // prev 19999
    BOOST_CHECK_MESSAGE(v - 1 == tb->getFVint(fdi_id), "UpDate3");
    v = 10;
    tb->clearBuffer();
    tb->setFV(fdi_id, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual U10");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(11 == tb->getFVint(fdi_id), "GetEqual Next");
    for (int i = 10; i < 19999; i++)
    {
        tb->clearBuffer();
        tb->setFV(fdi_id, i);

        tb->seek();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual U");
        int v = i + 1;
        tb->setFV((short)1, v);
        tb->update();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "UpDate");
    }
    db->endTrn();

    // check update in key
    v = 8;
    tb->setFV(fdi_id, v);
    tb->setFV(fdi_name, _T("ABC"));
    tb->update(table::changeInKey);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update changeInKey");

    tb->clearBuffer();
    tb->setFV(fdi_id, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(_tcscmp(_T("ABC"), tb->getFVstr(fdi_name)) == 0,
                        "update changeInKey2");
    tb->release();
}

void testSnapshot(database* db)
{
    table* tb = openTable(db);
    table* tbg = db->openTable(_T("group"), TD_OPEN_NORMAL);
    database* db2 = database::create();
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    table* tb2 = openTable(db2);
    table* tbg2 = db2->openTable(_T("group"), TD_OPEN_NORMAL);

    /*  No locking repeatable read                        */
    /* -------------------------------------------------- */
    db->beginSnapshot(); // CONSISTENT_READ is default
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginSnapShot");
    db->beginTrn();
    BOOST_CHECK_MESSAGE(STATUS_ALREADY_INSNAPSHOT == db->stat(), "Invalid beginSnapshot");


    tb->setKeyNum(0);
    tb->seekFirst(); 
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst");
    _tstring firstValue = tb->getFVstr(fdi_name);
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekNext");
    BOOST_CHECK_MESSAGE(2 == tb->getFVint(fdi_id), "seekNext");
    tbg->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_EOF == tbg->stat(), "seekFirst tbg");
    BOOST_CHECK_MESSAGE(0 == tbg->recordCount(false), "seekFirst tbg");

    // Change data by another connection. change 2 tables. 
    tb2->setKeyNum(0);
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->setFV(fdi_name, tb2->getFVint(fdi_name) + 1);
    tb2->update(); //Change success
    BOOST_CHECK_MESSAGE(0 == tb2->stat(),
                        "tb2->update stat = " << tb2->stat());
    tbg2->setFV(fdi_id, 1);
    tbg2->setFV(fdi_name, _T("ABC"));
    tbg2->insert();
    BOOST_CHECK_MESSAGE(0 == tbg2->stat(), "tbg2->insert");

    // in-snapshot repeatable read check same value
    tb->seekFirst();
    _tstring secondValue = tb->getFVstr(fdi_name);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "secondValue");
    BOOST_CHECK_MESSAGE(secondValue == firstValue, "repeatableRead");

    tbg->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_EOF == tbg->stat(), "seekFirst tbg");
    BOOST_CHECK_MESSAGE(0 == tbg->recordCount(false), "seekFirst tbg");

    // test update in snapshot
    tb->update();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_LOCKTYPE == tb->stat(), "snapshot update stat = " << tb->stat());

    // test insert in snapshot
    tb->setFV(fdi_id, 0);
    tb->insert();
    BOOST_CHECK_MESSAGE(STATUS_INVALID_LOCKTYPE == tb->stat(), "snapshot insert stat = " << tb->stat());

    //test phantom read
    tb2->setFV(fdi_id, 29999);
    tb2->insert();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "phantom insert");
    tb->setFV(fdi_id, 29999);
    tb->seek();
    BOOST_CHECK_MESSAGE(STATUS_NOT_FOUND_TI == tb->stat(), "phantom read");

    // clean up
    tb2->setFV(fdi_id, 29999);
    tb2->seek();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "seek stat = " << tb2->stat());
    tb2->del();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "del stat = " << tb2->stat());

    db->endSnapshot();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "endSnapShot");
    
    // After snapshot, db can read new versions.
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    BOOST_CHECK_MESSAGE(1 == tb->getFVint(fdi_name), "read new value = 1");
    tbg->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tbg->stat(), "seekFirst tbg");
    BOOST_CHECK_MESSAGE(1 == tbg->recordCount(false), "seekFirst tbg");


    //test gap lock
    db->beginSnapshot(MULTILOCK_GAP_SHARE);
    tb->seekLast();  // id = 30000
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekLast");
    tb->seekPrev();  // id = 20002
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekPrev");
    tb->seekPrev();  // id = 20001
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekPrev");

    tb2->setFV(fdi_id, 29999);
    tb2->insert();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), 
                        "GAP insert stat = " << tb2->stat());

    db->endSnapshot();

    //test gap lock
    db->beginSnapshot(MULTILOCK_NOGAP_SHARE);
    tb->seekLast();  // id = 30000
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekLast");
    tb->seekPrev();  // id = 20002
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekPrev");
    tb->seekPrev();  // id = 20001
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekPrev");

    tb2->setFV(fdi_id, 20002);
    tb2->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "GAP insert");

    
    tb2->seekLast(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "GAP insert");


    db->endSnapshot();

    tbg->release();
    tbg2->release();
    tb->release();
    tb2->release();

    database::destroy(db2);
}

void testConflict(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb2 = openTable(db2);

    tb->setKeyNum(0);
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    /* --------------------------------------------------
     Change Index field
     -------------------------------------------------- */
    // Change data by another connection
    tb2->setKeyNum(0);
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->setFV(fdi_id, tb2->getFVint(fdi_id) - 10);
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update(");
    /* -------------------------------------------------- */
    // Change same record data by original connection
    tb->setFV(fdi_id, tb->getFVint(fdi_id) - 8);
    tb->update();
    BOOST_CHECK_MESSAGE(STATUS_CHANGE_CONFLICT == tb->stat(), "tb->update(");
    /* -------------------------------------------------- */

    /* -------------------------------------------------- */
    /* Change Non index field                             */  
    /* -------------------------------------------------- */
    // Change data by another connection
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->setFV(fdi_name, tb2->getFVint(fdi_name) - 10);
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update(");
    /* -------------------------------------------------- */
    // Change same record data by original connection
    tb->setFV(fdi_name, tb->getFVint(fdi_name) - 8);
    tb->update();
    BOOST_CHECK_MESSAGE(STATUS_CHANGE_CONFLICT == tb->stat(), "tb->update(");
    /* -------------------------------------------------- */
    tb->release();
    tb2->release();
    database::destroy(db2);
}

/* isoration Level ISO_REPEATABLE_READ */
void testTransactionLockRepeatable(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb2 = openTable(db2);

    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");
    // Test Invalid operation
    db->beginSnapshot();
    BOOST_CHECK_MESSAGE(STATUS_ALREADY_INTRANSACTION == db->stat(), "Invalid beginSnapshot");

    /* -------------------------------------------------*/
    /* Test Read with lock                              */
    /* -------------------------------------------------*/
    // lock(X) the first record
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    // Add lock(X) the second record
    tb->seekNext();

    // No transaction user can read allways. Use consistent_read 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");

    tb2->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    
    // The second transaction user can not lock same record.
    db2->beginTrn();
    tb2->setKeyNum(0);

    // Try lock(X)
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");
    db2->endTrn();
    db->endTrn();

    /* -------------------------------------------------*/
    /* Test single record lock and Transaction lock                             */
    /* -------------------------------------------------*/
    // lock(X) non-transaction
    tb2->seekFirst(ROW_LOCK_X);

    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");

    // Try lock(X)
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst");

    // Remove lock(X)
    tb2->seekFirst();

    // Retry lock(X)
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    tb->setFV(fdi_name, _T("ABC"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

    // No transaction user read can read allways. Use consistent_read 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    BOOST_CHECK_MESSAGE(_tstring(_T("ABC")) != _tstring(tb2->getFVstr(fdi_name)), "consistent_read");

    /* -------------------------------------------------*/
    /* Test Transaction lock and Transaction lock       */
    /* -------------------------------------------------*/
    db2->beginTrn();
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "beginTrn");

    // try lock(X) 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");

    // Try unlock updated record. Con not unlock updated record.
    tb->unlock();

    // try lock(X) 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");

    db2->endTrn();
    db->endTrn();

    /* -------------------------------------------------*/
    /* Test phantom read                                */
    /* -------------------------------------------------*/
    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");
    
    // read last row
    tb->seekLast();  //lock(X) last id = 30000
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLast");
    tb->seekPrev(); //Add lock(X)
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");
    int last2 = tb->getFVint(fdi_id);
    
    // insert test row
    tb2->setFV(fdi_id, 29999);
    tb2->insert();   //Can not insert by gap lock
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->insert");

    tb->seekLast();  
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLast");
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");
    BOOST_CHECK_MESSAGE(last2 == tb->getFVint(fdi_id), "phantom read");
    db->endTrn();
    
    /* -------------------------------------------------*/
    /* Test use shared lock option                      */
    /* -------------------------------------------------*/

    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn1");
    
    db2->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "beginTrn2");

    tb->seekLast(ROW_LOCK_S);  
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLast");
    tb2->seekLast(ROW_LOCK_S);  
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekLast");

    tb->seekPrev();//Lock(X)  
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");

    tb2->seekPrev(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekPrev");
 
    tb->seekPrev(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");
    int id = tb->getFVint(fdi_id);

    tb2->setFV(fdi_id, id);
    tb2->seek(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seek");

    db2->endTrn();
    db->endTrn();

    /* -------------------------------------------------*/
    /* Test Abort                                       */
    /* -------------------------------------------------*/
    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");

    // lock(X)
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->setFV(fdi_name, _T("EFG"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

    // move from first record.
    tb->seekNext();
    db->abortTrn();

    tb2->setKeyNum(0);
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(_tcscmp(tb2->getFVstr(fdi_name), _T("ABC")) == 0,
                        "tb->seekFirst");


    /* -------------------------------------------------*/
    /* Test Query and locks Multi record lock           */
    /* -------------------------------------------------*/
    db->beginTrn(MULTILOCK_REPEATABLE_READ);
    
    // Test find records are lock.
    query q;
    q.where(_T("id"), _T("<="), 15).and_(_T("id"), _T("<>"), 13)
        .reject(0xFFFF);
    tb->setQuery(&q);
    tb->setFV(fdi_id, 12);
    tb->find();
    while (tb->stat() == 0)
        tb->findNext();
    BOOST_CHECK_MESSAGE(15 == tb->getFVint(fdi_id), "find last id");
    
    //all records locked
    for (int i = 12 ; i <= 16; ++i)
    {
        tb2->setFV(fdi_id, i);
        tb2->seek(ROW_LOCK_X);
        BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seek");
    }
    db->endTrn();


    tb->release();
    tb2->release();
    database::destroy(db2);
}


void testBug_015(database* db)
{
    table* tb = openTable(db);
    db->beginTrn(SINGLELOCK_NOGAP);
    tb->seekFirst(); // lock(X)
    tb->unlock();
    tb->seekNext();
    /* Here! InnoDB issues an error message, please check the MySQL error log. 
       [InnoDB: Error: unlock row could not find a 3 mode lock on the record]   
    */
    db->endTrn();
    tb->release();
}
/* READ_COMMITTED support select lock type */
void testIssue_016(database* db)
{
    table* tb = openTable(db);
    db->beginTrn(MULTILOCK_NOGAP);
    tb->seekFirst(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst S");
    tb->seekNext(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekNext S");
    tb->seekNext(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekNext X");
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");
    db->endTrn();

    db->beginTrn(MULTILOCK_GAP);
    tb->seekFirst(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst S");
    tb->seekNext(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekNext S");
    tb->seekNext(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekNext X");
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");
    db->endTrn();
    tb->release();

}

/* isoration Level ISO_READ_COMMITED */
void testTransactionLockReadCommited(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb2 = openTable(db2);

    /* -------------------------------------------------*/
    /* Test single record lock Transaction and read     */
    /* -------------------------------------------------*/
    db->beginTrn(SINGLELOCK_READ_COMMITED);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");
    db->beginSnapshot();
    BOOST_CHECK_MESSAGE(STATUS_ALREADY_INTRANSACTION == db->stat(), "Invalid beginSnapshot");

    tb->setKeyNum(0);
    tb->seekFirst(); // lock(X)
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    // Try lock(X)
    tb2->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekFirst");

    // consistent read
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    // Unlock first record. And lock(X) second record
    tb->seekNext(); 

    // test unlocked first record
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->update");

    // The second record, consistent read
    tb2->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekNext");
    // Try lock(X) whith lock(IX)
    tb2->update();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->update stat = " << tb2->stat());

    /* ---------------------------------------------------------*/
    /* Test single record lock Transaction and Transaction lock */
    /* ---------------------------------------------------------*/
    db2->beginTrn();
    // Try lock(X)  
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    // Try lock(X)  
    tb2->seekNext();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekNext");
    db2->endTrn();
    db->endTrn();

    /* ------------------------------------------------------------*/
    /* Test multi record lock Transaction and non-transaction read */
    /* ------------------------------------------------------------*/
    db->beginTrn(MULTILOCK_READ_COMMITED);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");

    // lock(X) the first record
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    // Add lock(X) the second record
    tb->seekNext();

    // No transaction user read can read allways. Use consistent_read 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");

    tb2->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekNext");
    
    /* --------------------------------------*/
    /* Test unlock                           */
    /* --------------------------------------*/
    tb2->seekFirst();
    tb2->seekNext(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekNext");

    tb->unlock();
    // retry seekNext. Before operation is failed but do not lost currency.
    tb2->seekNext(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "unlock");
    tb2->seekNext();
    /* --------------------------------------*/
    /* Test undate record unlock             */
    /* --------------------------------------*/
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->unlock();// Can not unlock updated record
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->unlock");
    tb2->seekFirst();
    tb2->seekNext(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "undate unlock");

    /* ---------------------------------------------------------*/
    /* Test multi record lock Transaction and Transaction       */
    /* ---------------------------------------------------------*/
    db2->beginTrn();
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "beginTrn");

    // Try lock(X)
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");
    db2->endTrn();
    db->endTrn();

    /* -------------------------------------------------------------------*/
    /* Test multi record lock Transaction and non-transaction record lock */
    /* -------------------------------------------------------------------*/
    // lock(X) non-transaction
    tb2->seekFirst(ROW_LOCK_X);

    db->beginTrn(SINGLELOCK_READ_COMMITED);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");

    // Try lock(X)
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst");

    // Remove lock(X)
    tb2->seekFirst();

    // Retry lock(X)
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    // update in transaction
    tb->setFV(fdi_name, _T("ABC"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

    // move from first record.
    tb->seekNext();

    // No transaction read can read allways. Use consistent_read 
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->update();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->update stat = " << tb2->stat());

    db->endTrn();
    /* -------------------------------------------------*/
    /* Test phantom read                                */
    /* -------------------------------------------------*/
    db->beginTrn(MULTILOCK_READ_COMMITED);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");
    
    //read last row
    tb->seekLast();  //lock(X) last id = 30000
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLast");
    tb->seekPrev(); //Add lock(X)
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");
    int last2 = tb->getFVint(fdi_id);
    
    //insert test row
    tb2->setFV(fdi_id, 29999);
    tb2->insert(false);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->insert stat = " << tb2->stat());

    tb->seekLast();  
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLast");
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekPrev");
    BOOST_CHECK_MESSAGE(last2 != tb->getFVint(fdi_id), "phantom read id = " 
                        << tb->getFVint(fdi_id));
    db->endTrn();
    
    //cleanup
    tb2->del(); // last id = 29999
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->del");

    /* -------------------------------------------------*/
    /* TAbort test                                      */
    /* -------------------------------------------------*/
    db->beginTrn(SINGLELOCK_READ_COMMITED);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginTrn");

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->setFV(fdi_name, _T("EFG"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

    tb->seekNext();
    db->abortTrn();
    tb2->setKeyNum(0);
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(_tcscmp(tb2->getFVstr(fdi_name), _T("ABC")) == 0,
                        "tb->seekFirst");

    /* -------------------------------------------------*/
    /* Test Query and locks Single record lock          */
    /* -------------------------------------------------*/
    db->beginTrn(SINGLELOCK_READ_COMMITED);
    
    // Test find last record locked
    query q;
    q.where(_T("id"), _T("<="), _T("100"));
    tb->setQuery(&q);
    tb->setFV(fdi_id, 1);
    tb->find();
    while (tb->stat() == 0)
        tb->findNext();
    BOOST_CHECK_MESSAGE(100 == tb->getFVint(fdi_id), "find last id");
    
    // find read last is record of id = 101.
    // Would be difficult to identify the last 
    //  access to records at SINGLELOCK_READ_COMMITED.
    // No match records are unlocked.
    tb2->setFV(fdi_id, 100);
    tb2->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seek stat = " << tb2->stat());
    tb2->setFV(fdi_id, 101);
    tb2->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seek stat = " << tb2->stat());
    tb2->unlock();
    db->endTrn();

    /* -------------------------------------------------*/
    /* Test Query and locks Multi record lock           */
    /* -------------------------------------------------*/
    db->beginTrn(MULTILOCK_READ_COMMITED);
    
    // Test find records are lock.
    q.reset().where(_T("id"), _T("<="), 15).and_(_T("id"), _T("<>"), 13)
        .reject(0xFFFF);
    tb->setQuery(&q);
    tb->setFV(fdi_id, 12);
    tb->find();
    while (tb->stat() == 0)
        tb->findNext();
    BOOST_CHECK_MESSAGE(15 == tb->getFVint(fdi_id), "find last id");
    
    
    for (int i = 12 ; i <= 16; ++i)
    {
        tb2->setFV(fdi_id, i);
        tb2->seek(ROW_LOCK_X);
        if ((i == 16)|| (i == 13)) 
            BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seek");
        else
            BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seek");
    }
    db->endTrn();
    tb->release();
    tb2->release();
    database::destroy(db2);
}

void testRecordLock(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb2 = openTable(db2);

    tb->setKeyNum(0);
    tb2->setKeyNum(0);

    //Single record lock
    tb->seekFirst(ROW_LOCK_X); // lock(X)
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb2->seekFirst();  // Use consistent_read  
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    tb2->seekFirst(ROW_LOCK_X);  // Try lock(X) single
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekFirst");

    // try consistent_read. Check ended that before auto transaction  
    tb2->seekFirst();  
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    tb2->seekNext(ROW_LOCK_X);  // lock(X) second
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    tb2->seekNext(ROW_LOCK_X);  // lock(X) third  second lock freed
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    
    tb->seekNext(); // nobody lock second. but REPEATABLE_READ tb2 lock all(no unlock)
    if (db->trxIsolationServer() == SRV_ISO_REPEATABLE_READ)
        BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst stat = "
                            <<  tb->stat() );
    else
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst stat = " <<  
                            tb->stat() );
    tb->seekNext(ROW_LOCK_X); // Try lock(X) third
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst");

    //Update test change third with lock(X)
    tb2->setFV(fdi_name, _T("The 3rd"));
    tb2->update(); // auto trn commit and unlock all locks
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update");
    tb2->seekNext(ROW_LOCK_X); // lock(X) 4th
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    tb2->setFV(fdi_name, _T("The 4th"));
    tb2->update(); // auto trn commit and unlock all locks

    // Test unlock all locks, after update
    tb->seekFirst(ROW_LOCK_X); // lock(X) first
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    tb->seekNext(ROW_LOCK_X); // lock(X) second
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekNext");
    tb->seekNext(ROW_LOCK_X); // lock(X) third
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekNext");
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fdi_name), _T("The 3rd")) == 0,
                        "tb->seekNext");
    //Test Insert, After record lock  operation 
    tb->setFV(fdi_id, 21000);
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->insert");
    tb->setFV(fdi_id, 21000);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seek stat = " <<  tb->stat() );
    tb->del();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->del stat = " <<  tb->stat() );

    /* ---------   Unlock test ----------------------------*/
    // 1 unlock()
    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    tb->unlock();

    tb2->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->unlock();

    //2 auto tran ended
    table* tb3 = openTable(db2);
    tb2->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    
    tb3->seekLast(); //This operation is another table handle, then auto tran ended 
    BOOST_CHECK_MESSAGE(0 == tb3->stat(), "tb3->seekLast");

    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->unlock();

    // begin trn
    tb3->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb3->stat(), "tb3->seekFirst");

    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst");
    db2->beginTrn();

    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    db2->endTrn();
    tb->unlock();
    // begin snapshot
    tb3->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb3->stat(), "tb3->seekFirst");

    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->seekFirst");
    db2->beginSnapshot();
    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    db2->endSnapshot();
    tb->unlock();
     // close Table
    tb->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

    tb2->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekFirst");
    tb->release();
    tb2->seekFirst(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->unlock();
    /* ---------   End Unlock test ----------------------------*/

    /* ---------   Invalid lock type test ----------------------------*/
    tb2->seekFirst(ROW_LOCK_S);
    BOOST_CHECK_MESSAGE(STATUS_INVALID_LOCKTYPE == tb2->stat(), "tb2->seekFirst");

    /* ---------   Invalid unlock  test ----------------------------*/
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->unlock();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    tb2->release();
    tb3->release();
    database::destroy(db2);
    
}

bool isMySQL5_7(database* db)
{
    btrVersions vv;
    db->getBtrVersion(&vv);
    return (db->stat() == 0) && 
        ((5 == vv.versions[1].majorVersion) &&
        (7 == vv.versions[1].minorVersion));
  
}
#if defined(__BCPLUSPLUS__)
#pragma warn -8004
#endif
void testExclusive()
{
   
    // db mode exclusive
    database* db = database::create();
    /* -------------------------------------------------*/
    /*  database WRITE EXCLUSIVE                        */
    /* -------------------------------------------------*/
    table* tb = openTable(db, TD_OPEN_EXCLUSIVE);//DB TD_OPEN_EXCLUSIVE
    BOOST_CHECK_MESSAGE(0 == db->stat(), "Exclusive opened 1 ");

    // Can not open another connections.
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF);
    //database open error. Check database::stat()
    BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat(),
                        "open db2->stat = " << db2->stat());
    dbdef* def = db->dbDef();
    tabledef* td = def->tableDefs(1);
    td->iconIndex = 3;
    def->updateTableDef(1);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef");
    tb->release();
    db->close();
    db2->close();

    /* -------------------------------------------------*/
    /*  database READ EXCLUSIVE                        */
    /* -------------------------------------------------*/
    tb = openTable(db, TD_OPEN_READONLY_EXCLUSIVE);

    // read mysql version
    bool MySQL5_7 = isMySQL5_7(db);

    // Read only open
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), 
                    "read only open " << db2->stat());
    db2->close();
    
    // Normal open
    /*  Since MySQL 5.7 : D_OPEN_READONLY_EXCLUSIVE + TD_OPEN_NORMAL is fail,
        It's correct.
    */

    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);

    if (MySQL5_7)
        BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat() , "Normal open");
    else
        BOOST_CHECK_MESSAGE(0 == db2->stat(), "Normal open");
                                    
    db2->close();

    // Write Exclusive open
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), 
            TYPE_SCHEMA_BDF, TD_OPEN_EXCLUSIVE);
    BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat()
                                    , "Write Exclusive open");
    db2->close();

    // Read Exclusive open
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), 
            TYPE_SCHEMA_BDF, TD_OPEN_READONLY_EXCLUSIVE);
    BOOST_CHECK_MESSAGE(0 == db2->stat()
                                    , "Read Exclusive open");
    db2->close();
    tb->release();
    db->close();

    /* -------------------------------------------------*/
    /*  Nnomal and Exclusive opend tables mix use       */
    /* -------------------------------------------------*/
    tb = openTable(db, TD_OPEN_NORMAL);
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "db2 open stat = " << db2->stat());
            
    table* tb2 = db->openTable(_T("group"), TD_OPEN_EXCLUSIVE);
    //Check tb2 Exclusive
    table* tb3 = db2->openTable(_T("group"), TD_OPEN_NORMAL);
    if (tb3) //For don't use tb3;
        BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat()
                                    , "Write Exclusive open" << db2->stat());
    else
        BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat()
                                    , "Write Exclusive open" << db2->stat());
    //if (tb2->recordCount(false) == 0)
    {
        for (int i = 1 ; i < 5 ; ++i)
        {
            tb2->setFV(fdi_id, i + 1);
            tb2->setFV(fdi_name, i + 1);
            tb2->insert();
            BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->insert");
        }
    }
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb2->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekLast");
    tb->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb2->seekLast");
    // Normal close first
    tb->close();
    tb2->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekLast");
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");

    //Reopen Normal
    tb = db->openTable(_T("user"));
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb2->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekLast");
    tb->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb2->seekLast");
    // Exclusive close first
    tb2->close();
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->seekLast();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb2->seekLast");

    /* ---------------------------------------------------*/
    /*  Nnomal and Exclusive opend tables mix transaction */
    /* ---------------------------------------------------*/
    tb2 = db->openTable(_T("group"), TD_OPEN_EXCLUSIVE);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open group stat = " << db->stat()) ;
    //Check tb2 Exclusive

    tb3 = db2->openTable(_T("group"), TD_OPEN_NORMAL);

    BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat()
                                    , "Write Exclusive open");
    db->beginTrn();
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->setFV(fdi_name, _T("mix trn"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->update");

    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->setFV(fdi_name, _T("first mix trn tb2"));
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update");

    tb2->seekNext();
    tb2->setFV(fdi_name, _T("second mix trn tb2"));
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update");
    db->endTrn();
    tb2->seekFirst();
    _tstring v =  tb2->getFVstr(fdi_name);
    BOOST_CHECK_MESSAGE(v == _T("first mix trn tb2"), "check first");
    tb2->seekNext();
    v =  tb2->getFVstr(fdi_name);
    BOOST_CHECK_MESSAGE(v == _T("second mix trn tb2"), "check second");

    database::destroy(db);
    database::destroy(db2);
}
#if defined(__BCPLUSPLUS__)
#pragma warn .8004
#endif
/* Multi database */
void testMultiDatabase(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF); // not new connection
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "db2->open");
    table* tb2 = db2->openTable(_T("group"));

    db->beginTrn();
    db2->beginTrn();

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst");
    _tstring v = tb->getFVstr(fdi_name);
    tb->setFV(fdi_name, _T("MultiDatabase"));
    tb->update();

    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "seekFirst");
    tb2->setFV(fdi_name, _T("MultiDatabase"));
    tb2->update();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "update");
    db2->endTrn();
    db->abortTrn();

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst");
    _tstring v2 = tb->getFVstr(fdi_name);
    BOOST_CHECK_MESSAGE(v == v2, "check value");
    
    tb->release();
    tb2->release();
    database::destroy(db2);
}

class worker
{
    table* m_tb;
public:
    worker(table* tb):m_tb(tb){}
    void run(){m_tb->seekLessThan(false, ROW_LOCK_X);}
};
/* Getting missing value by lock wait */
void testMissingUpdate(database* db)
{
    table* tb = openTable(db);
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF); 
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "db2->open stat = " << db2->stat());
    table* tb2 = db2->openTable(_T("user"));
    {

        boost::scoped_ptr<worker> w(new worker(tb2));
#ifdef TEST_SEEK_RETRY    
        // Inserting  target, The InnoDB is good!
        tb->setFV(fdi_id, 300000);
        tb2->setFV(fdi_id, 300000);
        tb->seekLessThan(false, ROW_LOCK_X);
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLessThan");
        if (tb->stat() == 0)
        {
            tb2->seekLessThan(false, ROW_LOCK_X);
            BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->seekLessThan");
            // Get lock(X) same record in parallel. The InnoDB is good!
            boost::scoped_ptr<boost::thread> t(new boost::thread(boost::bind(&worker::run, w.get())));
            Sleep(100);
            int v = tb->getFVint(fdi_id);//v = 30000
            tb->setFV(fdi_id, ++v);      //v = 30001
            tb->insert();
            Sleep(1);
            t->join();
            Sleep(1);
            if (db->trxIsolationServer() == SRV_ISO_REPEATABLE_READ)
            {   /* When SRV_ISO_REPEATABLE_READ tb2 get gap lock first,
                   tb can not insert, it is dedlock! 
                */
                BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb->stat(), "tb->insert stat= "
                                    << tb->stat());
            }
            else
            {   /* When SRV_ISO_READ_COMMITED, tb2 get lock after insert. 
                   But no retry loop then lock id = 30000 !!!!!!!. Oh no!
                   This is not READ_COMMITED !. 
                */
                BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekLessThan  stat= " 
                                    << tb2->stat());
                int v2 = tb2->getFVint(fdi_id);
                BOOST_CHECK_MESSAGE(v2 == v-1 , "value v-1 = " << v-1 << " bad = " 
                                    << v2);
                
                //cleanup
                tb->setFV(fdi_id, v);
                tb->seek();
                BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seek");
                tb->del();
                BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->del");
            }
                    
            tb2->unlock();
        }
#endif
        
        // Deleting  target, The InnoDB is good!
        tb->setFV(fdi_id, 300000);
        tb2->setFV(fdi_id, 300000);
        tb->seekLessThan(false, ROW_LOCK_X);
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekLessThan");
        if (tb->stat() == 0)
        {
            // Get lock(X) same record in parallel.
            boost::scoped_ptr<boost::thread> t(new boost::thread(boost::bind(&worker::run, w.get())));
            Sleep(5);
            int v = tb->getFVint(fdi_id);
            tb->del();
            t->join();
            BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->insert");
            BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekLessThan");
            int v2 = tb2->getFVint(fdi_id);
            BOOST_CHECK_MESSAGE(v != v2 , "value v = " << v
                    << " bad = " << v2);
            tb2->unlock();
        }
    }
    tb->release();
    tb2->release();
    database::destroy(db2);
}

void testInsert2(database* db)
{
    table* tb = openTable(db);
    int v = 40000;
    db->beginTrn();
    tb->clearBuffer();
    tb->setFV(fdi_id, v);
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert");
    v = 10;
    tb->clearBuffer();
    tb->setFV((short)0, v);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual Ins");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(11 == tb->getFVint(fdi_id), "GetEqual InsNext");
    db->endTrn();
    tb->release();
}

void testDelete(database* db)
{
    table* tb = openTable(db);

    // estimate number
    int count = tb->recordCount(true);
    bool c = (abs(count - 20002) < 5000);
    BOOST_CHECK_MESSAGE(c == true, "RecordCount1");
    if (!c)
    {
        char tmp[256];
        sprintf_s(
            tmp, 256,
            "true record count = 20002 as estimate recordCount count = %d ",
            count);
        BOOST_CHECK_MESSAGE(false, tmp);
    }
    // true number
    uint_td v = tb->recordCount(false);
    BOOST_CHECK_MESSAGE(20002 == v,
                        "RecordCount2 count = " << v);
    int vv = 15001;
    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetEqual");
    tb->del();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "Delete");
    tb->setFV((short)0, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(4 == tb->stat(), "GetEqual");

    // check update in key
    vv = 8;
    tb->setFV(fdi_id, vv);
    tb->del(table::inkey);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "delete changeInKey");

    tb->clearBuffer();
    tb->setFV(fdi_id, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(tb->stat() == STATUS_NOT_FOUND_TI,
                        "delete changeInKey2");

    db->beginTrn();
    tb->stepFirst();
    while (tb->stat() == 0)
    {
        tb->del();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "Delete");
        tb->stepNext();
    }
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "StepNext");
    db->endTrn();
    BOOST_CHECK_MESSAGE((uint_td)0 == tb->recordCount(false), "RecordCount");
    tb->release();
}

void testSetOwner(database* db)
{
    table* tb = openTable(db);
    tb->setOwnerName(_T("ABCDEFG"));
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "SetOwner stat = " << tb->stat());
    tb->clearOwnerName();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "SetOwner stat = " << tb->stat());
    tb->release();
}

void testReconnect(database* db)
{
    table* tb = openTable(db);

    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF); 
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "db2->open");
    table* tb2 = db2->openTable(_T("user"));
    
    //lock row
    tb->setFV(fdi_id, 10);
    tb->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seek stat = " << tb->stat());
    db->disconnectForReconnectTest();
    db->reconnect();
    
    //Check restore lock
    tb2->setFV(fdi_id, 10);
    tb2->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seek stat = " << tb2->stat()); //0

    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seek stat = " << tb->stat());//8
    BOOST_CHECK_MESSAGE(11 == tb->getFVint(fdi_id), "getFVint 11 bad = " << tb->getFVint(fdi_id));//10

    tb2->setFV(fdi_id, 11);
    tb2->seek(ROW_LOCK_X);
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seek stat = " << tb2->stat());

    tb->release();
    tb2->release();
    database::destroy(db2);
}

void testCreateIndex(database* db)
{
    table* tb = openTable(db);
    dbdef* def = db->dbDef();
    if (def)
    {
        const tabledef* td = tb->tableDef();
        keydef* kd = def->insertKey(td->id, td->keyCount);
        kd->segments[0].fieldNum = fdi_name; //name
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segments[0].flags.bit0 = 1; // duplicatable
        kd->segmentCount = 1;
        // assign keynumber 
        kd->keyNumber = 5;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "CreateIndex updateTableDef stat = " << def->stat());
    }
    tb->setKeyNum(tb->tableDef()->keyCount-1);
    tb->createIndex(true);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "CreateIndex");
    tb->release();

    //test not mysql grant
    db->aclReload();
    BOOST_CHECK_MESSAGE(STATUS_DB_YET_OPEN == db->stat(),
                        "bad grantReload db->stat() = " << db->stat());
}

void testDropIndex(database* db)
{
    table* tb = openTable(db);
    tb->dropIndex(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "DropIndex stat = " << tb->stat());
    tb->release();
}

void testLogin(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    db->connect(makeUri(PROTOCOL, HOSTNAME, _T("")));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "connect");
    if (db->stat() == 0)
    {
        // second connection
        database* db2 = database::create();
        db2->connect(makeUri(PROTOCOL, HOSTNAME, _T("")), true);
        BOOST_CHECK_MESSAGE(
            0 == db2->stat(),
            "new connection connect  db->stat() = " << db->stat());
        database::destroy(db2);

        db->disconnect();
        BOOST_CHECK_MESSAGE(0 == db->stat(),
                            "disconnect  db->stat() = " << db->stat());
    }
    // invalid host name
    db->connect(makeUri(PROTOCOL, _T("localhost123"), _T("")));
    bool f = (db->stat() == ERROR_TD_INVALID_CLINETHOST) ||
             (db->stat() == ERROR_TD_HOSTNAME_NOT_FOUND);
    BOOST_CHECK_MESSAGE(f, "bad host stat =" << db->stat());
    if (!f)
    {

#ifndef _UNICODE
        TCHAR buf[256];
        sprintf_s(buf, 256, "bad host db->stat()=%d", db->stat());
        BOOST_TEST_MESSAGE(buf);
#endif
    }

    db->open(makeUri(PROTOCOL, _T("localhost123"), DBNAME, BDFNAME),
             TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);
    f = (db->stat() == ERROR_TD_INVALID_CLINETHOST) ||
        (db->stat() == ERROR_TD_HOSTNAME_NOT_FOUND);
    BOOST_CHECK_MESSAGE(f, "bad host stat =" << db->stat());

    testCreateNewDataBase(db); //with open
    db->close(); // disconnected
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "databese close db->stat() = " << db->stat());

    // true database name
    db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "databese  connect db->stat() = " << db->stat());
    if (db->stat() == 0)
    {
        db->disconnect();
        BOOST_CHECK_MESSAGE(0 == db->stat(),
                            "databese disconnect db->stat() = " << db->stat());
    }
    // invalid database name
    testDropDatabase(db);
    db->disconnect();
    BOOST_CHECK_MESSAGE(STATUS_PROGRAM_ERROR == db->stat(),
                        "databese disconnect db->stat() = " << db->stat());

    db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(ERROR_NO_DATABASE == db->stat(),
                        "databese connect db->stat() = " << db->stat());

    //connect is failed, no need disconnet.
    db->disconnect();
    BOOST_CHECK_MESSAGE(STATUS_PROGRAM_ERROR == db->stat(),
                        "databese disconnect db->stat() = " << db->stat());
}

void testGrantReload(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, _T("mysql"), TRANSACTD_SCHEMANAME),
             TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "open mysql db->stat() = " << db->stat());
    db->aclReload();
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "grantReload db->stat() = " << db->stat());
}
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
void doCreateVarTable(database* db, int id, const _TCHAR* name, char fieldType,
                      int charset)
{
    // create table
    dbdef* def = db->dbDef();
    tabledef td;
    memset(&td, 0, sizeof(td));
    td.setTableName(name);
    _TCHAR buf[267];
    _tcscpy_s(buf, 100, name);
    _tcscat_s(buf, 100, _T(".dat"));
    td.setFileName(buf);
    td.id = id;
    td.keyCount = 0;
    td.fieldCount = 0;
    td.flags.all = 0;

    td.primaryKeyNum = -1;
    td.parentKeyNum = -1;
    td.replicaKeyNum = -1;

    td.pageSize = 2048;

    td.charsetIndex = charset;

    def->insertTable(&td);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable stat = " << def->stat());

    fielddef* fd = def->insertField(id, 0);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    fd = def->insertField(id, 1);
    fd->setName(_T("name"));
    fd->type = fieldType;
    if (fieldType == ft_mywvarchar)
        fd->len = (ushort_td)1 +
                  mysql::charsize(CHARSET_UTF16LE) * 3; // max 3 char len byte
    else if (fieldType == ft_mywvarbinary)
        fd->len = (ushort_td)1 +
                  mysql::charsize(CHARSET_UTF16LE) * 3; // max 6 char len byte
    else if (fieldType == ft_myvarchar)
    {
        if (charset == CHARSET_CP932)
            fd->len = (ushort_td)1 +
                      mysql::charsize(CHARSET_CP932) * 3; // max 6 char len byte
        else if (charset == CHARSET_UTF8B4)
            fd->len =
                (ushort_td)1 +
                mysql::charsize(CHARSET_UTF8B4) * 3; // max 6 char len byte
    }
    else
        fd->len = (ushort_td)7; // max 6 char len byte

    fd = def->insertField(id, 2);
    fd->setName(_T("groupid"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    keydef* kd = def->insertKey(id, 0);

    kd->segments[0].fieldNum = 0;
    kd->segments[0].flags.bit8 = 1; // extended key type
    kd->segments[0].flags.bit1 = 1; // changeable
    kd->segmentCount = 1;

    kd = def->insertKey(id, 1);

    kd->segments[0].fieldNum = 1;
    kd->segments[0].flags.bit8 = 1; // extended key type
    kd->segments[0].flags.bit1 = 1; // changeable
    kd->segments[0].flags.bit0 = 1; // duplicateable
    kd->segments[0].flags.bit4 = 1; // not last segmnet
    kd->segments[1].fieldNum = 2;
    kd->segments[1].flags.bit8 = 1; // extended key type
    kd->segments[1].flags.bit1 = 1; // changeable
    kd->segments[1].flags.bit0 = 1; // duplicateable
    kd->segmentCount = 2;

    def->updateTableDef(id);

    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 4");
    table* tb = db->openTable(id);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    if (tb)
        tb->release();
}

bool isUtf16leSupport(database* db)
{
    btrVersions vv;
    db->getBtrVersion(&vv);
    if ((int)'M' == (int)vv.versions[1].type)
    {
        if (vv.versions[1].majorVersion > 5)
            return true;
        if (vv.versions[1].minorVersion > 5)
            return true;
        return false;
    }
    return true;
}

void testCreateDataBaseVar(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    if (db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME)))
    {
        db->drop();
        BOOST_CHECK_MESSAGE(0 == db->stat(), "drop testvar db stat = " << db->stat());
    }
    db->create(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "create testvar db stat = " << db->stat());
    if (0 == db->stat())
    {
        db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME), 0, 0);
        BOOST_CHECK_MESSAGE(0 == db->stat(), "open testvar db stat = " << db->stat());

        if (0 == db->stat())
        {
            doCreateVarTable(db, 1, _T("user1"), ft_myvarchar, CHARSET_CP932);
            doCreateVarTable(db, 2, _T("user2"), ft_myvarbinary, CHARSET_CP932);
            if (isUtf16leSupport(db))
                doCreateVarTable(db, 3, _T("user3"), ft_mywvarchar,
                                 CHARSET_CP932);
            doCreateVarTable(db, 4, _T("user4"), ft_mywvarbinary,
                             CHARSET_CP932);
            doCreateVarTable(db, 5, _T("user5"), ft_myvarchar, CHARSET_UTF8B4);
            db->close();
            db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"),
                             TRANSACTD_SCHEMANAME),
                     0, 0);
        }
    }
}

void testDropDataBaseVar(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    if (0 == db->stat())
    {
        db->drop();
        BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDataBaseVar");
    }
}

void dump(const char* p, int size)
{
    std::string s;
    char tmp[100];
    for (int i = 0; i < size; i += 16)
    {
        for (int j = 0; j < 16; j++)
        {
            printf(tmp, "%02X ", *((unsigned char*)(p + i + j)));
            s.append(tmp);
        }
        s.append(" ");

        for (int j = 0; j < 16; j++)
        {
            printf(tmp, "%c", *((unsigned char*)(p + i + j)));
            s.append(tmp);
        }
        s.append("\n");
    }
}

void doTestverField(table* tb, bool unicodeField, bool varCharField)
{
// Set Wide Get Wide
#ifdef _WIN32
    tb->setFVW(2, L"68");
#else
    tb->setFV(2, "68");
#endif

#ifdef _WIN32
    // too long string
    tb->setFVW(1, L"1234567");
    if (varCharField)
    {
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"123"),
                            "Get Set W1");
        if (wstring(tb->getFVWstr(1)) != wstring(L"123"))
            dump((const char*)tb->getFVWstr(1), 7);
    }
    else
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"123456"),
                            "Get Set W1");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 1");
    // short
    tb->setFVW(1, L"12 ");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"12 "),
                        "Get Set W2");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 2");
    // too long lanji

    if (unicodeField)
    {

        tb->setFVW(1, L"あいうえお\xD867\xDE3D"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"あいう"),
                                "Get Set W3");
        else
            BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) ==
                                    wstring(L"あいうえお"),
                                "Get Set W3");
    }
    else
    {
        tb->setFVW(1, L"0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"0松本"),
                            "Get Set W3");
    }
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 2");
#endif

    // Set Ansi Get Wide
    // too long string
    tb->setFVA(1, "1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"),
                            "Get Set A1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"),
                            "Get Set A1");

#ifdef _WIN32
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 1");
#else
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 1");
#endif
    // short string
    tb->setFVA(1, "13 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("13 "),
                        "Get Set A2");
#ifdef _WIN32
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 2");
#else
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");
#endif
    // too long lanji

    if (unicodeField)
    {
#ifdef LINUX
        tb->setFVA(1, "あいうえお𩸽"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"),
                                "Get Set A3");
        else
            BOOST_CHECK_MESSAGE(
                string(tb->getFVAstr(1)) == string("あいうえお"), "Get Set A3");
#endif
    }
    else
    {
        tb->setFVA(1, "0松本市"); // kanji that "matumostoshi"
        bool f = string(tb->getFVAstr(1)) == string("0松本");
        BOOST_CHECK_MESSAGE(f, "Get Set A3");
        if (!f)
            BOOST_TEST_MESSAGE(tb->getFVAstr(1));
    }
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");

// Set Wide Get Ansi
#ifdef _WIN32
    // too long string
    tb->setFVW(1, L"1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"),
                            "GetA Set W1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"),
                            "GetA Set W1");

    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 1");

    // short string
    tb->setFVW(1, L"23 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("23 "),
                        "GetA Set W2");

    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 2");

    // too long lanji
    if (unicodeField)
    {

        tb->setFVW(1, L"あいうえお\xD867\xDE3D"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"),
                                "GetA Set W3");
        else
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) ==
                                    string("あいうえお"),
                                "GetA Set W3");
    }
    else
    {
        tb->setFVW(1, L"0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("0松本"),
                            "GetA Set W3");
    }
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"),
                        "Orverrun 2");
#endif
    // Set Ansi Get Ansi
    // too long string
    tb->setFVA(1, "1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"),
                            "GetA Set A1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"),
                            "GetA Set A1");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 1");
    // short string
    tb->setFVA(1, "13 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("13 "),
                        "GetA Set A2");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");

    // too long lanji
    if (unicodeField)
    {
#ifdef LINUX
        tb->setFVA(1, "あいうえお𩸽"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"),
                                "Get Set A3");
        else
            BOOST_CHECK_MESSAGE(
                string(tb->getFVAstr(1)) == string("あいうえお"), "Get Set A3");
#endif
    }
    else
    {
        tb->setFVA(1, "0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("0松本"),
                            "GetA Set A3");
    }
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");
}

void testVarField(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1");
    table* tb = db->openTable(_T("user1"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
    BOOST_TEST_MESSAGE("Start acp varchar");
    doTestverField(tb, false, true);
    tb->release();

    tb = db->openTable(_T("user2"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable2");
    BOOST_TEST_MESSAGE("Start acp varbinary");
    doTestverField(tb, false, false);
    tb->release();

    if (isUtf16leSupport(db))
    {
        tb = db->openTable(_T("user3"));
        BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable3");
        BOOST_TEST_MESSAGE("Start unicode varchar");
        doTestverField(tb, true, true);
        tb->release();
    }
    tb = db->openTable(_T("user4"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable4");
    BOOST_TEST_MESSAGE("Start unicode varbinary");
    doTestverField(tb, true, false);
    tb->release();

    tb = db->openTable(_T("user5"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable5");
    BOOST_TEST_MESSAGE("Start utf8 varchar");
    doTestverField(tb, true, true);

    tb->release();
}

void doVarInsert(database* db, const _TCHAR* name, unsigned int codePage,
                 const _TCHAR* str, int start, int end, bool bulk)
{
    _TCHAR buf[256];
    table* tb = db->openTable(name);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
    if (bulk)
        tb->beginBulkInsert(BULKBUFSIZE);
    int v;

    for (int i = start; i <= end; i++)
    {
        tb->clearBuffer();
        tb->setFV((short)0, i);
        _stprintf_s(buf, 256, _T("%s%d"), str, i);
        tb->setFV((short)1, buf);
        v = i + 10;
        tb->setFV((short)2, v);
        tb->insert();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert");
    }
    if (bulk)
        tb->commitBulkInsert();
    tb->release();
}

void testVarInsert(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    int start = 1;
    bool bulk = false;
    const _TCHAR* str = _T("漢字文字のテスト"); // too long kanji
    const _TCHAR* str2 = _T("123");

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1 stat = " << db->stat());
    if (0 == db->stat())
    {
        bool utf16leSupport = isUtf16leSupport(db);

        doVarInsert(db, _T("user1"), CP_ACP, str, start, start, bulk);
        doVarInsert(db, _T("user2"), CP_ACP, str, start, start, bulk);
        if (utf16leSupport)
            doVarInsert(db, _T("user3"), CP_ACP, str, start, start, bulk);
        doVarInsert(db, _T("user4"), CP_ACP, str, start, start, bulk);
        doVarInsert(db, _T("user5"), CP_UTF8, str, start, start, bulk);

        ++start;
        doVarInsert(db, _T("user1"), CP_ACP, str2, start, start, bulk);
        doVarInsert(db, _T("user2"), CP_ACP, str2, start, start, bulk);
        if (utf16leSupport)
            doVarInsert(db, _T("user3"), CP_ACP, str2, start, start, bulk);
        doVarInsert(db, _T("user4"), CP_ACP, str2, start, start, bulk);
        doVarInsert(db, _T("user5"), CP_UTF8, str2, start, start, bulk);

        ++start;
        bulk = true;
        int end = 1000;
        doVarInsert(db, _T("user1"), CP_ACP, _T(""), start, end, bulk);
        doVarInsert(db, _T("user2"), CP_ACP, _T(""), start, end, bulk);
        if (utf16leSupport)
            doVarInsert(db, _T("user3"), CP_ACP, _T(""), start, end, bulk);
        doVarInsert(db, _T("user4"), CP_ACP, _T(""), start, end, bulk);
        doVarInsert(db, _T("user5"), CP_UTF8, _T(""), start, end, bulk);
    }
}

void doVarRead(database* db, const _TCHAR* name, unsigned int codePage,
               const _TCHAR* str, int num, char_td key)
{

    table* tb = db->openTable(name);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    tb->clearBuffer();
    tb->setKeyNum(key);

    if (key == 0)
        tb->setFV((short)0, num);
    else
    {
        int v = num + 10;
        tb->setFV((short)1, str);
        tb->setFV((short)2, v);
    }
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetEqual var stat");

    // test read of var field
    bool f = _tstring(str) == _tstring(tb->getFVstr(1));
    BOOST_CHECK_MESSAGE(f, "GetEqual var field1");

    // test read of second field
    BOOST_CHECK_MESSAGE((int)(num + 10) == tb->getFVint(2),
                        "GetEqual var field2");

    tb->release();
}

void testVarRead(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    const _TCHAR* str = _T("漢字文");
    const _TCHAR* str3 = _T("漢字文字のテ");
    const _TCHAR* str2 = _T("123");
    const _TCHAR* str4 = _T("1232");

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    if (0 == db->stat())
    {
        bool utf16leSupport = isUtf16leSupport(db);
        int num = 1;
        char_td key = 0;
        // too long string

        doVarRead(db, _T("user1"), CP_ACP, str, num, key);
        doVarRead(db, _T("user2"), CP_ACP, str, num, key);
        if (utf16leSupport)
            doVarRead(db, _T("user3"), CP_ACP, str, num, key);
        doVarRead(db, _T("user4"), CP_ACP, str3, num, key);
        doVarRead(db, _T("user5"), CP_UTF8, str, num, key);
        // short string
        ++num;
        doVarRead(db, _T("user1"), CP_ACP, str2, num, key);
        doVarRead(db, _T("user2"), CP_ACP, str4, num, key);
        if (utf16leSupport)
            doVarRead(db, _T("user3"), CP_ACP, str2, num, key);
        doVarRead(db, _T("user4"), CP_ACP, str4, num, key);
        doVarRead(db, _T("user5"), CP_UTF8, str2, num, key);

        key = 1;
        doVarRead(db, _T("user1"), CP_ACP, _T("120"), 120, key);
        doVarRead(db, _T("user2"), CP_ACP, _T("120"), 120, key);
        if (utf16leSupport)
            doVarRead(db, _T("user3"), CP_ACP, _T("120"), 120, key);
        doVarRead(db, _T("user4"), CP_ACP, _T("120"), 120, key);
        doVarRead(db, _T("user5"), CP_UTF8, _T("120"), 120, key);
    }
}

void doVarFilter(database* db, const _TCHAR* name, unsigned int codePage,
                 const _TCHAR* str, int num, char_td key)
{
    table* tb = db->openTable(name);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    tb->clearBuffer();
    tb->setKeyNum(key);

    if (key == 0)
    {
        _TCHAR buf[120];
        _stprintf_s(buf, 120, _T("id > %d and id <= %d"), num, num + 10);
        tb->setFilter(buf, 0, 10);

        // find forword
        tb->setFV((short)0, num);
        tb->seekGreater(true);
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetGreater var stat");
        for (int i = num + 1; i <= num + 10; i++)
        {
            tb->findNext();
            BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetGreater var stat");
            // test read of var field
            BOOST_CHECK_MESSAGE(i == tb->getFVint(1), "findNext");
            // test read of second field
            BOOST_CHECK_MESSAGE((int)(i + 10) == tb->getFVint(2),
                                "GetEqual var field2");
        }

        // find previous
        int v = num + 10;
        tb->setFilter(buf, 0, 10);
        tb->setFV(fdi_id, v);
        tb->seekLessThan(true);
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "GetPrevious var stat");
        BOOST_CHECK_MESSAGE(v == tb->getFVint(fdi_id), "GetPrevious");
        for (int i = num + 10; i > num; i--)
        {
            tb->findPrev(false);
            BOOST_CHECK_MESSAGE(0 == tb->stat(), "FindPrev var stat");
            // test read of var field
            BOOST_CHECK_MESSAGE(i == tb->getFVint(1), "FindPrev");
            // test read of second field
            BOOST_CHECK_MESSAGE((int)(i + 10) == tb->getFVint(2),
                                "FindPrev var field2");
        }

        // test record count
        BOOST_CHECK_MESSAGE((uint_td)10 == tb->recordCount(),
                            "GetEqual var field2");
    }
    else
    {
        int v = num + 10;
        tb->setFV((short)1, str);
        tb->setFV((short)2, v);
    }
    tb->release();
}

void testFilterVar(database* db)
{
    if (_tcscmp(PROTOCOL, _T("tdap")) != 0)
        return;

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    if (0 == db->stat())
    {
        const _TCHAR* str = _T("漢字文");
        const _TCHAR* str3 = _T("漢字文字のテ");
        bool utf16leSupport = isUtf16leSupport(db);

        int num = 10;
        char_td key = 0;
        doVarFilter(db, _T("user1"), CP_ACP, str, num, key);
        doVarFilter(db, _T("user2"), CP_ACP, str, num, key);
        if (utf16leSupport)
            doVarFilter(db, _T("user3"), CP_ACP, str, num, key);
        doVarFilter(db, _T("user4"), CP_ACP, str3, num, key);
        doVarFilter(db, _T("user5"), CP_UTF8, str, num, key);

#ifdef _UNICODE
        // short string
        const _TCHAR* str2 = _T("123");
        const _TCHAR* str4 = _T("1232");
        ++num;
        doVarFilter(db, L"user1", CP_ACP, str2, num, key);
        doVarFilter(db, L"user2", CP_ACP, str4, num, key);
        if (utf16leSupport)
            doVarFilter(db, L"user3", CP_ACP, str2, num, key);
        doVarFilter(db, L"user4", CP_ACP, str4, num, key);
        doVarFilter(db, L"user5", CP_UTF8, str2, num, key);

#endif

        key = 1;
        doVarFilter(db, _T("user1"), CP_ACP, _T("120"), 120, key);
        doVarFilter(db, _T("user2"), CP_ACP, _T("120"), 120, key);
        if (utf16leSupport)
            doVarFilter(db, _T("user3"), CP_ACP, _T("120"), 120, key);
        doVarFilter(db, _T("user4"), CP_ACP, _T("120"), 120, key);
        doVarFilter(db, _T("user5"), CP_UTF8, _T("120"), 120, key);
    }
}
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------

void stringFileterCreateTable(database* db, int id, const _TCHAR* name,
                              uchar_td type, uchar_td type2)
{
    // create table

    dbdef* def = db->dbDef();
    tabledef td;
    memset(&td, 0, sizeof(td));
    td.setTableName(name);
    _TCHAR buf[267];
    _tcscpy_s(buf, 100, name);
    _tcscat_s(buf, 100, _T(".dat"));
    td.setFileName(buf);
    td.id = id;
    td.primaryKeyNum = -1;
    td.parentKeyNum = -1;
    td.replicaKeyNum = -1;
    td.pageSize = 2048;
    td.charsetIndex = CHARSET_UTF8B4;
    // td.charsetIndex = CHARSET_CP932;

    def->insertTable(&td);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable stat = " << def->stat());

    fielddef* fd = def->insertField(id, 0);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(),
                        "updateTableDef 1 stat = " << def->stat());

    fd = def->insertField(id, 1);
    fd->setName(_T("name"));
    fd->type = type;
    fd->len = 44;
    if (fd->varLenBytes())
    {
        fd->len = fd->varLenBytes() + 44;
        fd->keylen = fd->len;
    }
    if (fd->blobLenBytes())
    {
        fd->len = 12; // 8+4
    }

    fd->keylen = fd->len;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(),
                        "updateTableDef 2 stat = " << def->stat());

    fd = def->insertField(id, 2);
    fd->setName(_T("namew"));
    fd->type = type2;
    fd->len = 44;
    if (fd->varLenBytes())
    {
        fd->len = fd->varLenBytes() + 44;
        fd->keylen = fd->len;
    }
    if (fd->blobLenBytes())
    {
        fd->len = 12; // 8+4
    }
    fd->keylen = fd->len;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 3");

    keydef* kd = def->insertKey(id, 0);
    kd->segments[0].fieldNum = 0;
    kd->segments[0].flags.bit8 = 1; // extended key type
    kd->segments[0].flags.bit1 = 1; // changeable
    kd->segmentCount = 1;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 4");

    kd = def->insertKey(id, 1);
    kd->segments[0].fieldNum = 1;
    kd->segments[0].flags.bit8 = 1; // extended key type
    kd->segments[0].flags.bit1 = 1; // changeable
    kd->segments[0].flags.bit0 = 1; // duplicateable
    kd->segmentCount = 1;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 5");

    kd = def->insertKey(id, 2);
    kd->segments[0].fieldNum = 2;
    kd->segments[0].flags.bit8 = 1; // extended key type
    kd->segments[0].flags.bit1 = 1; // changeable
    kd->segments[0].flags.bit0 = 1; // duplicateable
    kd->segmentCount = 1;
    def->updateTableDef(id);
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 6");
}

void doInsertStringFileter(table* tb)
{

    tb->clearBuffer();
    int id = 1;
    tb->setFV(_T("id"), id);
    tb->setFV(_T("name"), _T("あいうえおかきくこ"));
    tb->setFV(_T("namew"), _T("あいうえおかきくこ"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 1 stst = " << tb->stat());

    tb->clearBuffer();
    id = 2;
    tb->setFV(_T("id"), id);
    tb->setFV(_T("name"), _T("A123456"));
    tb->setFV(_T("namew"), _T("A123456"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 2");

    tb->beginBulkInsert(BULKBUFSIZE);
    tb->clearBuffer();
    id = 3;
    tb->setFV(_T("id"), id);
    tb->setFV(_T("name"), _T("あいがあればOKです"));
    tb->setFV(_T("namew"), _T("あいがあればOKです"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 3");

    tb->clearBuffer();
    id = 4;
    tb->setFV(_T("id"), id);
    tb->setFV(_T("name"), _T("おはようございます"));
    tb->setFV(_T("namew"), _T("おはようございます"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 3");

    tb->clearBuffer();
    id = 5;
    tb->setFV(_T("id"), id);
    tb->setFV(_T("name"), _T("おめでとうございます。"));
    tb->setFV(_T("namew"), _T("おめでとうございます。"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 4");
    int n = tb->commitBulkInsert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 5");
    BOOST_CHECK_MESSAGE(n == 3, "InsertStringFileter 5");
}

void doTestReadSF(table* tb)
{
    tb->setKeyNum(0);
    tb->clearBuffer();
    int id = 1;
    tb->setFV(_T("id"), id);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) ==
                            _tstring(tb->getFVstr(1)),
                        "doTestReadSF2");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) ==
                            _tstring(tb->getFVstr(1)),
                        "doTestReadSF2b");

    id = 3;
    tb->setFV(_T("id"), id);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF3");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいがあればOKです")) ==
                            _tstring(tb->getFVstr(1)),
                        "doTestReadSF4");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいがあればOKです")) ==
                            _tstring(tb->getFVstr(2)),
                        "doTestReadSF4b");

    tb->setKeyNum(1);
    tb->clearBuffer();
    tb->setFV(_T("name"), _T("A123456"));
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF5");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(1)),
                        "doTestReadSF6");

    tb->setKeyNum(2);
    tb->clearBuffer();
    tb->setFV(_T("namew"), _T("A123456"));
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF5");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)),
                        "doTestReadSF6");
}

void doTestSF(table* tb)
{
    tb->setKeyNum(0);
    tb->clearBuffer();

    tb->setFilter(_T("name = 'あい*'"), 0, 10);
    
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    /*
        If this point segmentation fult. Then drop database teststring. 
    */

    tb->findNext(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) ==
                            _tstring(tb->getFVstr(1)),
                        "doTestReadSF2");
    BOOST_CHECK_MESSAGE(2 == (int)tb->recordCount(), "doTestReadSF2");

    tb->setFilter(_T("name <> 'あい*'"), 0, 10);
    BOOST_CHECK_MESSAGE(3 == (int)tb->recordCount(), "doTestReadSF2");
    tb->clearBuffer();
    tb->setFilter(_T("name <> 'あい*'"), 0, 10);
    tb->seekFirst();
    tb->findNext(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1 stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");

    tb->findNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おはようございます")) ==
                            _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");

    tb->findNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おめでとうございます。")) ==
                            _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");
    tb->findNext();
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestReadSF1");

    tb->clearBuffer();
    tb->seekLast();
    tb->findPrev(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おめでとうございます。")) ==
                            _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");

    tb->findPrev();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おはようございます")) ==
                            _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");

    tb->findPrev(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)),
                        "doTestReadSF1");

    tb->findPrev();
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestReadSF1");

    tb->setFilter(_T("name = 'あい'"), 0, 10);
    BOOST_CHECK_MESSAGE(0 == (int)tb->recordCount(), "doTestReadSF2");

    tb->setFilter(_T("name <> ''"), 0, 10);
    BOOST_CHECK_MESSAGE(5 == (int)tb->recordCount(), "doTestReadSF2");

    // test setFilter don't change field value
    tb->clearBuffer();
    tb->setFV(_T("name"), _T("ABCDE"));
    tb->setFilter(_T("name = 'あい'"), 0, 10);
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE")) == _tstring(tb->getFVstr(1)),
                        "doTestReadSF2 field value");
}

void doTestUpdateSF(table* tb)
{

    tb->setKeyNum(0);
    tb->clearBuffer();
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());
    tb->setFV(_T("name"), _T("ABCDE"));
    tb->setFV(_T("namew"), _T("ABCDEW"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());

    tb->setFV(_T("name"), _T("ABCDE2"));
    tb->setFV(_T("namew"), _T("ABCDEW2"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE")) == _tstring(tb->getFVstr(1)),
                        "doTestUpdateSF");
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDEW")) == _tstring(tb->getFVstr(2)),
                        "doTestUpdateSF");
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(),
                        "doTestUpdateSF stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE2")) == _tstring(tb->getFVstr(1)),
                        "doTestUpdateSF");
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDEW2")) == _tstring(tb->getFVstr(2)),
                        "doTestUpdateSF");
}

void doTestStringFileter(database* db, int id, const _TCHAR* name,
                         uchar_td type, uchar_td type2)
{

    stringFileterCreateTable(db, id, name, type, type2);
    table* tb = db->openTable(id);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");

    doInsertStringFileter(tb);
    doTestReadSF(tb);
    doTestSF(tb);
    doTestUpdateSF(tb);
    tb->release();
}

void testDropDataBaseStr(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME), 0, 0);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open stat = " << db->stat());
    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "DropDataBaseTestString stat=" << db->stat());
}

void testStringFileter(database* db)
{
    db->create(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        testDropDataBaseStr(db);
        db->create(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME));
    }
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase");

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME), 0, 0);
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createNewDataBase 1 stat = " << db->stat());

    doTestStringFileter(db, 1, _T("zstring"), ft_zstring, ft_wzstring);
    if (isUtf16leSupport(db))
        doTestStringFileter(db, 2, _T("myvarchar"), ft_myvarchar,
                            ft_mywvarchar);
    else
        doTestStringFileter(db, 2, _T("myvarchar"), ft_myvarchar, ft_myvarchar);

    doTestStringFileter(db, 3, _T("mytext"), ft_mytext, ft_myblob);

    db->close();
}


// ------------------------------------------------------------------------

_TCHAR dbNmae[50] = { _T("テスト") };
_TCHAR bdfNmae[50] = { _T("構成.bdf") };
_TCHAR tableNmae[50] = { _T("漢字テーブル") };
_TCHAR fdName1[50] = { _T("番号") };
_TCHAR fdName2[50] = { _T("名前") };

bool nameInited = false;

void initKanjiName()
{

#if (!defined(_UNICODE) && defined(_WIN32))
    if (!nameInited)
    {
        wchar_t tmp[50];
        MultiByteToWideChar(932, MB_PRECOMPOSED, dbNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, dbNmae, 50, NULL, NULL);
        MultiByteToWideChar(932, MB_PRECOMPOSED, bdfNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, bdfNmae, 50, NULL, NULL);
        MultiByteToWideChar(932, MB_PRECOMPOSED, tableNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, tableNmae, 50, NULL, NULL);
        MultiByteToWideChar(932, MB_PRECOMPOSED, fdName1, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, fdName1, 50, NULL, NULL);
        MultiByteToWideChar(932, MB_PRECOMPOSED, fdName2, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, fdName2, 50, NULL, NULL);
        nameInited = true;
    }
#endif
}

void testDropDatabaseKanji(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "db->open stat = " << db->stat());

    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "db->drop() stat = " << db->stat());
}

void testKnajiCreateSchema(database* db)
{

    initKanjiName();
    db->create(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        testDropDatabaseKanji(db);
        db->create(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae));
    }
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createKanjiDatabase stat = " << db->stat());
    // create table
    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createKanjiDatabase 1 stat = " << db->stat());

    dbdef* def = db->dbDef();
    if (def)
    {
        tabledef td;
        memset(&td, 0, sizeof(tabledef));
#ifndef _UNICODE
        td.schemaCodePage = CP_UTF8;
        td.charsetIndex = CHARSET_UTF8;
#else
        td.schemaCodePage = CP_UTF8;
        td.charsetIndex = CHARSET_CP932;
#endif
        td.setTableName(tableNmae);
        td.setFileName(tableNmae);
        td.id = 1;
        td.primaryKeyNum = -1;
        td.parentKeyNum = -1;
        td.replicaKeyNum = -1;
        td.pageSize = 2048;

        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "insertTable stat = " << def->stat());

        fielddef* fd = def->insertField(1, 0);
        fd->setName(fdName1);
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 1 stat = " << def->stat());

        fd = def->insertField(1, 1);
        fd->setName(fdName2);
        fd->type = ft_zstring;
        fd->len = (ushort_td)33;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 2 stat = " << def->stat());

        keydef* kd = def->insertKey(1, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 3 stat = " << def->stat());
    }
}

table* openKnajiTable(database* db)
{

    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openKnajiTable 1 stat = " << db->stat());
    table* tb = db->openTable(tableNmae);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openKnajiTable 2 stat = " << db->stat());
    return tb;
}

void testInsertKanji(database* db)
{

    table* tb = openKnajiTable(db);

    tb->clearBuffer();
    tb->setFV(fdName1, _T("1"));
    tb->setFV(fdName2, _T("小坂"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert 1");

    tb->clearBuffer();
    tb->setFV(fdName1, _T("2"));
    tb->setFV(fdName2, _T("矢口"));
    tb->insert();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert 2");
    tb->release();
}

void testGetEqualKanji(database* db)
{

    table* tb = openKnajiTable(db);
    tb->clearBuffer();
    tb->setFV((short)0, 1);
    tb->seek();
    BOOST_CHECK_MESSAGE(1 == tb->getFVint(fdName1), "GetEqual id 1");
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fdName2), _T("小坂")) == 0,
                        "GetEqual name 2");

    tb->setFV((short)0, 2);
    tb->seek();
    BOOST_CHECK_MESSAGE(2 == tb->getFVint(fdName1), "GetEqual id 2");
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fdName2), _T("矢口")) == 0,
                        "GetEqual name 2");
    tb->release();
}

// ------------------------------------------------------------------------
void testResultField(database* db)
{
    table* tb = openTable(db);
    resultField rf = {0, 0};
    rf.setParam(tb, _T("name"));

    BOOST_CHECK_MESSAGE(rf.len == 33, " resultField.setParam");
    BOOST_CHECK_MESSAGE(rf.pos == 4, " resultField.setParam");

    size_t len = rf.size();
    BOOST_CHECK_MESSAGE(len == 4, " resultField.writeBuffer");
    tb->release();
}

void testResultDef()
{
    resultDef rd;

    rd.reset();
    BOOST_CHECK_MESSAGE(rd.maxRows == 0, " resultDef.maxRows");
    BOOST_CHECK_MESSAGE(rd.fieldCount == 0, " resultDef.fieldCount");

    size_t len = rd.size();
    BOOST_CHECK_MESSAGE(len == 4, " resultDef.writeBuffer");
}

void testLogic(database* db)
{
    table* tb = openTable(db);
    logic lc;
    db->dbDef()->tableDefs(1)->fieldDefs[1].type = ft_zstring;

    lc.setParam(tb, _T("name"), _T("="), _T("abc"), eCend);

    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1, " logic.logType");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    int len = lc.size();
    BOOST_CHECK_MESSAGE(lc.len == 33, " logic.len");
    BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "abc") == 0, " logic.data");
    BOOST_CHECK_MESSAGE(len == 7 + 33, " logic.writeBuffer");
   
   
    

    // compField invalid filed name
    bool ret = lc.setParam(tb, _T("name"), _T("="), _T("1"), eCend, true);
    BOOST_CHECK_MESSAGE(ret == false, " logic invalid filed name");

    // compField
    ret = lc.setParam(tb, _T("name"), _T("="), _T("id"), eCend, true);
    BOOST_CHECK_MESSAGE(ret == true, " logic filed name");
    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    BOOST_CHECK_MESSAGE(lc.len == 33, " logic.len");
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1 + CMPLOGICAL_FIELD,
                        " logic.logType compField");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    BOOST_CHECK_MESSAGE(*((short*)lc.data) == 0, " logic.data");
    len = lc.size();
    BOOST_CHECK_MESSAGE(len == 7 + 2, " logic.writeBuffer");

    // invalid filed name
    ret = lc.setParam(tb, _T("name1"), _T("="), _T("id"), eCend, true);
    BOOST_CHECK_MESSAGE(ret == false, " logic invalid filed name2");

    // wildcard
    lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCend, false);
    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    BOOST_CHECK_MESSAGE(lc.len == 3, " logic.len");
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1, " logic.logType");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "abc") == 0, " logic.data");

    len = lc.size();
    BOOST_CHECK_MESSAGE(len == 7 + 3, " logic.writeBuffer");

    lc.setParam(tb, _T("name"), _T("="), _T("漢字*"), eCend, false);
    BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "漢字") == 0, " logic.data");

    len = lc.size();
    BOOST_CHECK_MESSAGE(len == (int)(7 + (_tcslen(_T("漢字")) * sizeof(_TCHAR))),
                        " logic.writeBuffer len =" << len);

    // combine
    lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCor, false);
    BOOST_CHECK_MESSAGE(lc.opr == 2, " logic.opr or");
    lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCand, false);
    BOOST_CHECK_MESSAGE(lc.opr == 1, " logic.opr and");

    // logType
    ret = lc.setParam(tb, _T("name"), _T("!="), _T("abc*"), eCend, false);
    BOOST_CHECK_MESSAGE(lc.logType == 255, " logic.logType !=");
    BOOST_CHECK_MESSAGE(ret == false, " logic invalid logType");

    // canJoin

    // zstring is cannot join
    lc.setParam(tb, _T("name"), _T("="), _T("1"), eCand, false);
    BOOST_CHECK_MESSAGE(lc.canJoin(false) == false, " logic canJoin");
    BOOST_CHECK_MESSAGE(lc.canJoin(true) == false, " logic canJoin");

    lc.setParam(tb, _T("id"), _T("="), _T("1"), eCand, false);
    BOOST_CHECK_MESSAGE(lc.canJoin(false) == true, " logic canJoin");
    BOOST_CHECK_MESSAGE(lc.canJoin(true) == true, " logic canJoin");
    lc.opr = eCend;
    BOOST_CHECK_MESSAGE(lc.canJoin(true) == false, " logic canJoin");
    BOOST_CHECK_MESSAGE(lc.canJoin(false) == true, " logic canJoin");

    lc.opr = eCor;
    BOOST_CHECK_MESSAGE(lc.canJoin(true) == false, " logic canJoin");
    BOOST_CHECK_MESSAGE(lc.canJoin(false) == true, " logic canJoin");

    lc.opr = eCand;

    logic lc2;
    lc2.setParam(tb, _T("id"), _T("="), _T("1"), eCend, false);
    lc2.pos = 3;

    lc.isNextFiled(&lc2);
    BOOST_CHECK_MESSAGE(lc.isNextFiled(&lc2) == false, " logic isNextFiled");
    lc2.pos = 4;
    BOOST_CHECK_MESSAGE(lc.isNextFiled(&lc2) == true, " logic isNextFiled");

    // join
    lc.joinAfter(&lc2);
    BOOST_CHECK_MESSAGE(lc.len == 8, " logic joinAfter");

    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic joinAfter");

    // placeHolder
    lc.setParam(tb, _T("name"), _T("="), _T("?"), eCand);
    BOOST_CHECK_MESSAGE(lc.placeHolder == true, " logic placeHolder");

    lc.setValue(tb, _T("abc*"));
    BOOST_CHECK_MESSAGE(strcmp((const char*)lc.data, "abc") == 0, "logic setValue");
    BOOST_CHECK_MESSAGE(lc.len == 3, "logic setValue");

    header hd;
    len = hd.size();
    BOOST_CHECK_MESSAGE(len == 8, " header.writeBuffer");
    tb->release();
}

void testQuery()
{
    queryBase q;
    q.queryString(_T("id = 0 and name = 'Abc efg'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("id = '0' and name = 'Abc efg'"),
                        "queryString");

    q.queryString(_T(""));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("and"), _T("name"), _T("="), _T("Abc efg"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("id = '0' and name = 'Abc efg'"),
                        "queryString");

    q.queryString(_T("select id,name id = 0 AND name = 'Abc&' efg'"));
    BOOST_CHECK_MESSAGE(
        _tstring(q.toString()) ==
            _T("select id,name id = '0' AND name = 'Abc&' efg'"),
        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("AND"), _T("name"), _T("="), _T("Abc' efg"));
    BOOST_CHECK_MESSAGE(
        _tstring(q.toString()) ==
            _T("select id,name id = '0' AND name = 'Abc&' efg'"),
        "queryString");

    q.queryString(_T("select id,name id = 0 AND name = 'Abc&& efg'"));
    BOOST_CHECK_MESSAGE(
        _tstring(q.toString()) ==
            _T("select id,name id = '0' AND name = 'Abc&& efg'"),
        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("AND"), _T("name"), _T("="), _T("Abc& efg"));
    BOOST_CHECK_MESSAGE(
        _tstring(q.toString()) ==
            _T("select id,name id = '0' AND name = 'Abc&& efg'"),
        "queryString");

    q.queryString(_T("*"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("*"), "queryString");

    q.queryString(_T(""));
    q.all();
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("*"), "queryString");

    q.queryString(_T("Select id,name id = 2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '2'"),
                        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '2'"),
                        "queryString");

    q.queryString(_T("SELECT id,name,fc id = 2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc id = '2'"),
                        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc id = '2'"),
                        "queryString");

    q.queryString(_T("select id,name,fc id = 2 and name = '3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc id = '2' and name = '3'"),
                        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    q.addLogic(_T("and"), _T("name"), _T("="), _T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc id = '2' and name = '3'"),
                        "queryString");

    // IN include
    q.queryString(_T("select id,name,fc IN '1','2','3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc in '1','2','3'"),
                        "queryString");

    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addSeekKeyValue(_T("1"));
    q.addSeekKeyValue(_T("2"));
    q.addSeekKeyValue(_T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) ==
                            _T("select id,name,fc in '1','2','3'"),
                        "queryString");

    q.queryString(_T("IN '1','2','3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'"),
                        "queryString");

    q.queryString(_T("IN 1,2,3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'"),
                        "queryString");

    q.queryString(_T(""));
    q.addSeekKeyValue(_T("1"));
    q.addSeekKeyValue(_T("2"));
    q.addSeekKeyValue(_T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'"),
                        "queryString");

    // special field name
    q.queryString(_T("select = 1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select = '1'"),
                        "queryString");

    q.queryString(_T(""));
    q.addLogic(_T("select"), _T("="), _T("1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select = '1'"),
                        "queryString");

    q.queryString(_T("in <> 1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in <> '1'"),
                        "queryString");

    q.queryString(_T(""));
    q.addLogic(_T("in"), _T("<>"), _T("1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in <> '1'"),
                        "queryString");

    // test auto_escape
    q.queryString(_T("code = ab'c"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&'c'"),
                        "queryString");

    q.queryString(_T("code = ab&c"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&&c'"),
                        "queryString");

    q.queryString(_T("code = abc&"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&'"),
                        "queryString");
    q.queryString(_T("code = abc&&"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&&&'"),
                        "queryString");

    q.queryString(_T("code = 'abc&'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&'"),
                        "queryString");
    q.queryString(_T("code = 'abc&&'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&&&'"),
                        "queryString");

    q.queryString(_T("code = 'ab'c'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&'c'"),
                        "queryString");

    q.queryString(_T("code = 'abc''"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&''"),
                        "queryString");

    q.queryString(_T("code = abc'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&''"),
                        "queryString");

    // Invalid end no close '
    q.queryString(_T("code = 'abc"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc'"),
                        "queryString");

    q.queryString(_T("code = &abc"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = '&&abc'"),
                        "queryString");
}

void teetNewDelete(database* db)
{
    // printf("new delete start \n");
    for (int i = 0; i < 500; ++i)
    {
        queryBase qb;
        qb.reset();

        query q;
        q.reset();

        recordsetQuery rq;
        rq.reset();

        groupQuery gq;
        gq.reset();

        fieldNames f;
        f.addValue(_T("abc"));

        activeTable atu(db, _T("user"));
        atu.index(0);
        activeTable atg(db, _T("groups"));
        atg.index(0);
        fieldNames fns;
        fns.keyField(_T("a"));

        client::sum s(fns);
        s.reset();

        client::count c(_T("a"));
        c.reset();

        client::avg a(fns);
        a.reset();
        client::min mi(fns);
        mi.reset();
        client::max ma(fns);
        ma.reset();

        recordset rs;
        rs.clear();

#ifndef BCB_64
        queryBase* nqb1 = new queryBase(); // bcb64 bad
        delete nqb1;
#endif
        queryBase* nqb = queryBase::create(); // All OK
        nqb->release();

        query* nqq = query::create(); // All OK
        nqq->release();

#ifndef BCB_64
        query* nqq1 = new query(); // bcb64 bad
        delete nqq1;
#endif
        recordsetQuery* nrq = recordsetQuery::create(); // All OK
        nrq->release();

#ifndef BCB_64
        recordsetQuery* nrqq = new recordsetQuery(); // bcb64 bad
        delete nrqq;

        groupQuery* ngq1 = new groupQuery(); // bcb64 bad
        delete ngq1;
#endif
        groupQuery* ngq = groupQuery::create(); // All OK
        ngq->release();

        fieldNames* nfn = fieldNames::create(); // All OK
        nfn->release();

#ifndef BCB_64
        fieldNames* nfn1 = new fieldNames(); // bcb64 bad
        delete nfn1;
#endif

        activeTable* at = new activeTable(db, _T("user")); // All OK
        activeTable* atg1 = new activeTable(db, _T("groups")); // All OK
        delete atg1;
        delete at;

#ifndef BCB_64

        client::sum* ns1 = new sum(fns); // bcb64 bad
        delete ns1;

        client::count* nc1 = new client::count(_T("a")); // bcb64 bad
        delete nc1;

        client::avg* na1 = new client::avg(fns); // bcb64 bad
        delete na1;

        client::min* nmin1 = new client::min(fns); // bcb64 bad
        delete nmin1;

        client::max* nmax1 = new client::max(fns); // bcb64 bad
        delete nmax1;

#endif

        client::sum* ns = sum::create(fns); // All OK
        ns->release();

        client::count* nc = client::count::create(_T("a")); // All OK
        nc->release();

        client::avg* na = client::avg::create(fns); // All OK
        na->release();

        client::min* nmin = client::min::create(fns); // All OK
        nmin->release();

        client::max* nmax = client::max::create(fns); // All OK
        nmax->release();

        recordset* r = new recordset(); // All OK

        recordset* rc(r->clone()); // All OK
        // delete rc;                          //All bad
        rc->release();

        delete r; // All OK
    }

    //activeTable releaseTable
    activeTable* at = new activeTable(db, _T("user"));
    at->releaseTable();
    BOOST_CHECK_MESSAGE(at->table() == NULL, " activeTable::releaseTable");
    delete at;
}

void testRecordsetClone(database* db)
{

#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    //activeTable atg(db, _T("groups"));
    activeTable ate(db, _T("extention"));
    recordset* rs = recordset::create();
    query q;

    atu.alias(fd_name, _T("name"));
    q.select(_T("id"), _T("name"), _T("group"))
        .where(_T("id"), _T("<="), 15000);
    atu.index(0).keyValue(1).read(*rs, q);
    BOOST_CHECK_MESSAGE(rs->size() == 15000, " rs.size() 15000 bad = " << rs->size());
    BOOST_CHECK_MESSAGE(rs->fieldDefs()->size() == 3, " rs.fieldDefs()->size() 3 bad = " << rs->fieldDefs()->size());

    // Join extention::comment
    q.reset();
    ate.index(0).join(
        *rs, q.select(_T("comment")).optimize(queryBase::joinHasOneOrHasMany),
        _T("id"));
    BOOST_CHECK_MESSAGE(rs->size() == 15000, "join  rs.size() 15000 bad = " << rs->size());
    BOOST_CHECK_MESSAGE(rs->fieldDefs()->size() == 4, " rs.fieldDefs()->size() 4 bad = " << rs->fieldDefs()->size());

    recordset* rs2 = rs->clone();
    rs->release();
    rs2->release();
    
}

void testJoin(database* db)
{

#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    activeTable atg(db, _T("groups"));
    activeTable ate(db, _T("extention"));
    recordset rs;
    query q;

    atu.alias(fd_name, _T("name"));
    q.select(_T("id"), _T("name"), _T("group"))
        .where(_T("id"), _T("<="), 15000);
    atu.index(0).keyValue(1).read(rs, q);
    BOOST_CHECK_MESSAGE(rs.size() == 15000, " rs.size() 15000 bad = " << rs.size());
    BOOST_CHECK_MESSAGE(rs.fieldDefs()->size() == 3, " rs.fieldDefs()->size() 3 bad = " << rs.fieldDefs()->size());

    // Join extention::comment
    q.reset();
    ate.index(0).join(
        rs, q.select(_T("comment")).optimize(queryBase::joinHasOneOrHasMany),
        _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join  rs.size() 15000 bad = " << rs.size());
    BOOST_CHECK_MESSAGE(rs.fieldDefs()->size() == 4, " rs.fieldDefs()->size() 4 bad = " << rs.fieldDefs()->size());

    // test reverse

    row& last = rs.reverse().first();
    BOOST_CHECK_MESSAGE(last[_T("id")].i() == 15000, "last field id == 15000");
    BOOST_CHECK_MESSAGE(_tstring(last[_T("comment")].c_str()) ==
                            _tstring(_T("15000 comment")),
                        "last field comment");

    // Join group::name
    q.reset();
    atg.alias(_T("name"), _T("group_name"));
    atg.index(0).join(rs, q.select(_T("group_name")), _T("group"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join2  rs.size()== 15000");
    row& first = rs.last();

    BOOST_CHECK_MESSAGE(first[_T("id")].i() == 1, "first field id == 1");
    BOOST_CHECK_MESSAGE(_tstring(first[_T("comment")].c_str()) ==
                            _tstring(_T("1 comment")),
                        "first field comment");

    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    // row_ptr row = rs[15000 - 9];
    row& rec = rs[15000 - 9];
    BOOST_CHECK_MESSAGE(
        _tstring(rec[_T("group_name")].c_str()) == _tstring(_T("4 group")),
        "group_name = 4 group " << string((rec)[_T("group_name")].a_str()));

    // Test orderby
    rs.orderBy(_T("group_name"));
    // rec = rs[(size_t)0];
    BOOST_CHECK_MESSAGE(_tstring(rs[(size_t)0][_T("group_name")].c_str()) ==
                            _tstring(_T("1 group")),
                        "group_name = 1 group "
                            << string(rs[(size_t)0][_T("group_name")].a_str()));

    sortFields orderRv;
    orderRv.add(_T("group_name"), false);
    rs.orderBy(orderRv);

    sortFields order;
    order.add(_T("group_name"), true);
    rs.orderBy(order);
    BOOST_CHECK_MESSAGE(_tstring(rs[(size_t)0][_T("group_name")].c_str()) ==
                            _tstring(_T("1 group")),
                        "group_name = 1 group "
                            << string(rs[(size_t)0][_T("group_name")].a_str()));

    // test union
    recordset rs2;
    atu.alias(fd_name, _T("name"));

    q.reset().select(_T("id"), _T("name"), _T("group")).where(_T("id"),
                                                              _T("<="), 16000);
    atu.index(0).keyValue(15001).read(rs2, q);
    ate.index(0).join(rs2, q.reset().select(_T("comment")).optimize(
                               queryBase::joinHasOneOrHasMany),
                      _T("id"));

    atg.alias(_T("name"), _T("group_name"));
    atg.index(0).join(rs2, q.reset().select(_T("group_name")), _T("group"));
    BOOST_CHECK_MESSAGE(rs2.size() == 1000, "join2  rs2.size()== 1000");

    rs += rs2;
    BOOST_CHECK_MESSAGE(rs.size() == 16000, "union  rs.size()== 16000");
    // row = rs[15000];
    BOOST_CHECK_MESSAGE(rs[15000][_T("id")].i() == 15001, "id = 15001");
    // row = rs.last();
    BOOST_CHECK_MESSAGE(rs.last()[_T("id")].i() == 16000, "id = 16000");

    // test group by
    groupQuery gq;
    gq.keyField(_T("group"), _T("id"));

    client::count count1(_T("count"));
    gq.addFunction(&count1);

    client::count count2(_T("gropu1_count"));
    count2.when(_T("group"), _T("="), 1);

    gq.addFunction(&count2);
    rs.groupBy(gq);
    BOOST_CHECK_MESSAGE(rs.size() == 16000, "group by  rs.size()== 16000");
    int v = rs[0][_T("gropu1_count")].i();
    BOOST_CHECK_MESSAGE(v == 1, "gropu1_count = " << v);

    // clone
    recordset* rsv(rs.clone());

    gq.reset();
    client::count count3(_T("count"));
    gq.addFunction(&count3).keyField(_T("group")); //.resultField(_T("count"));
    rs.groupBy(gq);
    BOOST_CHECK_MESSAGE(rs.size() == 5,
                        "group by2  rs.size()==" << rsv->size());

    // having
    recordsetQuery rq;
    rq.when(_T("gropu1_count"), _T("="), 1).or_(_T("gropu1_count"), _T("="), 2);
    rsv->matchBy(rq);
    BOOST_CHECK_MESSAGE(rsv->size() == 3200,
                        "matchBy  rsv.size() ==" << rsv->size());
    rsv->release();
    // top
    recordset rs3;
    rs.top(rs3, 10);
    BOOST_CHECK_MESSAGE(rs3.size() == 5, "top 10  rs3.size()== 5");

    // query new / delete
    recordsetQuery* q1 = recordsetQuery::create();
    q1->when(_T("gropu1_count"), _T("="), 1)
        .or_(_T("gropu1_count"), _T("="), 2);
    q1->release();

    query* q2 = query::create();
    q2->where(_T("gropu1_count"), _T("="), 1)
        .or_(_T("gropu1_count"), _T("="), 2);
    q2->release();

    groupQuery* q3 = groupQuery::create();
    q3->keyField(_T("group"), _T("id"));
    q3->release();
}

void testPrepareJoin(database* db)
{
#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    atu.alias(fd_name, _T("name"));

    activeTable atg(db, _T("groups"));
    atg.alias(_T("name"), _T("group_name"));

    activeTable ate(db, _T("extention"));
    recordset rs;
    query q;

    
    q.select(_T("id"), _T("name"), _T("group")).where(_T("id"), _T("<="), _T("?"));
    pq_handle pq = atu.prepare(q);
    atu.index(0).keyValue(1).read(rs, pq, 15000);
    BOOST_CHECK_MESSAGE(rs.size() == 15000, " rs.size() 15000 bad = " << rs.size());
    BOOST_CHECK_MESSAGE(rs.fieldDefs()->size() == 3, " rs.fieldDefs()->size() 3 bad = " << rs.fieldDefs()->size());

    // Join extention::comment
    q.reset().select(_T("comment")).optimize(queryBase::joinHasOneOrHasMany);
    pq = ate.prepare(q);
    ate.index(0).join(rs, pq, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join  rs.size() 15000 bad = " << rs.size());
    BOOST_CHECK_MESSAGE(rs.fieldDefs()->size() == 4, " rs.fieldDefs()->size() 4 bad = " << rs.fieldDefs()->size());

    // test reverse
    row& last = rs.reverse().first();
    BOOST_CHECK_MESSAGE(last[_T("id")].i() == 15000, "last field id == 15000");
    BOOST_CHECK_MESSAGE(_tstring(last[_T("comment")].c_str()) ==
                            _tstring(_T("15000 comment")),
                        "last field comment");
    // Join group::name
    q.reset().select(_T("group_name"));
    pq = atg.prepare(q);
    atg.index(0).join(rs, pq, _T("group"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join2  rs.size()== 15000");
    row& first = rs.last();

    BOOST_CHECK_MESSAGE(first[_T("id")].i() == 1, "first field id == 1");
    BOOST_CHECK_MESSAGE(_tstring(first[_T("comment")].c_str()) ==
                            _tstring(_T("1 comment")),
                        "first field comment");

    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    // row_ptr row = rs[15000 - 9];
    row& rec = rs[15000 - 9];
    BOOST_CHECK_MESSAGE(
        _tstring(rec[_T("group_name")].c_str()) == _tstring(_T("4 group")),
        "group_name = 4 group " << string((rec)[_T("group_name")].a_str()));

}

void testServerPrepareJoin(database* db)
{
#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    activeTable atg(db, _T("groups"));
    activeTable ate(db, _T("extention"));
    atu.alias(fd_name, _T("name"));
    atg.alias(_T("name"), _T("group_name"));
    query q;
    q.select(_T("id"), _T("name"), _T("group"))
        .where(_T("id"), _T("<="), _T("?"));
    pq_handle stmt1 = atu.prepare(q, true);
    BOOST_CHECK_MESSAGE(stmt1 != NULL, " stmt1");
    
    q.reset().select(_T("comment")).optimize(queryBase::joinHasOneOrHasMany);
    pq_handle stmt2 = ate.prepare(q, true);
    BOOST_CHECK_MESSAGE(stmt2 != NULL, " stmt2");

    q.reset().select(_T("group_name"));
    pq_handle stmt3 = atg.prepare(q, true);
    BOOST_CHECK_MESSAGE(stmt3 != NULL, " stmt3");

    recordset rs;
    atu.index(0).keyValue(1).read(rs, stmt1, 15000);
    BOOST_CHECK_MESSAGE(rs.size() == 15000, " rs.size()== 15000 bad " << rs.size());

    // Join extention::comment
    ate.index(0).join(rs, stmt2, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join  rs.size()== 15000");

    // test reverse
    row& last = rs.reverse().first();
    BOOST_CHECK_MESSAGE(last[_T("id")].i() == 15000, "last field id == 15000");
    BOOST_CHECK_MESSAGE(_tstring(last[_T("comment")].c_str()) ==
                            _tstring(_T("15000 comment")),
                        "last field comment");

    // Join group::name
    atg.index(0).join(rs, stmt3, _T("group"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join2  rs.size()== 15000");
    row& first = rs.last();

    BOOST_CHECK_MESSAGE(first[_T("id")].i() == 1, "first field id == 1");
    BOOST_CHECK_MESSAGE(_tstring(first[_T("comment")].c_str()) ==
                            _tstring(_T("1 comment")),
                        "first field comment");

    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    BOOST_CHECK_MESSAGE(
        _tstring(first[_T("group_name")].c_str()) == _tstring(_T("1 group")),
        "first field group_name " << string(first[_T("group_name")].a_str()));
    // row_ptr row = rs[15000 - 9];
    row& rec = rs[15000 - 9];
    BOOST_CHECK_MESSAGE(
        _tstring(rec[_T("group_name")].c_str()) == _tstring(_T("4 group")),
        "group_name = 4 group " << string((rec)[_T("group_name")].a_str()));

    // Test orderby
    rs.orderBy(_T("group_name"));
    // rec = rs[(size_t)0];
    BOOST_CHECK_MESSAGE(_tstring(rs[(size_t)0][_T("group_name")].c_str()) ==
                            _tstring(_T("1 group")),
                        "group_name = 1 group "
                            << string(rs[(size_t)0][_T("group_name")].a_str()));

    sortFields orderRv;
    orderRv.add(_T("group_name"), false);
    rs.orderBy(orderRv);

    sortFields order;
    order.add(_T("group_name"), true);
    rs.orderBy(order);
    BOOST_CHECK_MESSAGE(_tstring(rs[(size_t)0][_T("group_name")].c_str()) ==
                            _tstring(_T("1 group")),
                        "group_name = 1 group "
                            << string(rs[(size_t)0][_T("group_name")].a_str()));

    //All fields
    rs.clear();
    q.reset().all();
    q.where(_T("id"), _T("<="), _T("?"));
    stmt1 = atu.prepare(q, true);
    atu.keyValue(1).read(rs, stmt1, 15000);
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "read  rs.size()== 15000");
    if (rs.size() == 15000)
    {
        for (int i=0;i<15000;++i)
            BOOST_CHECK_MESSAGE(rs[i][_T("id")].i() == i+1, "All fields field Value");
    }

    ate.join(rs, stmt2, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join  rs.size()== 15000");
    atg.join(rs, stmt3, _T("group"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join2  rs.size()== 15000");

    // OuterJoin
    #define NO_RECORD_ID 5 
    table_ptr tb = ate.table();
    tb->setFV(_T("id"), NO_RECORD_ID);
    tb->seek();
    if (tb->stat() == 0)
        tb->del();
    BOOST_CHECK_MESSAGE(tb->stat()  == 0, "ate delete id = 5");
    q.reset().select(_T("comment"), _T("blob")).optimize(queryBase::joinHasOneOrHasMany);
    stmt2 = ate.prepare(q, true);
    
    // Join is remove record(s) no join target record.
    rs.clear();
    atu.keyValue(1).read(rs, stmt1, 15000);
    ate.join(rs, stmt2, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 14999, "join  rs.size()== 14999");
    BOOST_CHECK_MESSAGE(rs[NO_RECORD_ID-1][_T("comment")].i() == NO_RECORD_ID+1, "row of 5 : '6 comment'");
    const _TCHAR* vs = rs[NO_RECORD_ID-1][_T("blob")].c_str();
    bool ret = _tcscmp(vs, _T("6 blob")) == 0;
    BOOST_CHECK_MESSAGE(ret == true, "row of 5 : '6 blob'" );

    // OuterJoin is no remove record(s) no join target record.
    rs.clear();
    atu.keyValue(1).read(rs, stmt1, 15000);
    ate.outerJoin(rs, stmt2, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "outerJoin  rs.size()== 15000");
    atg.outerJoin(rs, stmt3, _T("group"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "join2  rs.size()== 15000");

    BOOST_CHECK_MESSAGE(rs[NO_RECORD_ID-1].isInvalidRecord() == true, "outerJoin isInvalidRecord");
    BOOST_CHECK_MESSAGE(rs[NO_RECORD_ID][_T("comment")].i() == NO_RECORD_ID+1, "row of 6 = '6 comment'");
    vs = rs[NO_RECORD_ID][_T("blob")].c_str();
    ret = _tcscmp(vs, _T("6 blob")) == 0; 
    BOOST_CHECK_MESSAGE(ret == true, "row of 6 = '6 blob'");

    // OuterJoin All Join fields
    q.reset().optimize(queryBase::joinHasOneOrHasMany).all();
    stmt2 = ate.prepare(q, true);
    rs.clear();
    atu.keyValue(1).read(rs, stmt1, 15000);
    ate.outerJoin(rs, stmt2, _T("id"));
    BOOST_CHECK_MESSAGE(rs.size() == 15000, "outerJoin  rs.size()== 15000");
    BOOST_CHECK_MESSAGE(rs[NO_RECORD_ID-1].isInvalidRecord() == true, "outerJoin isInvalidRecord");
    BOOST_CHECK_MESSAGE(rs[NO_RECORD_ID][_T("comment")].i() == NO_RECORD_ID+1, "row of 6 = '6 comment'");
    vs = rs[NO_RECORD_ID][_T("blob")].c_str();
    ret = _tcscmp(vs, _T("6 blob")) == 0; 
    BOOST_CHECK_MESSAGE(ret == true, "row of 6 = '6 blob'");
    field fd = rs[NO_RECORD_ID][_T("binary")];
    ret = compBlobField(NO_RECORD_ID + 1, fd);
    BOOST_CHECK_MESSAGE(ret == true, "row of 6 = 'binary 256 byte'");


    // Test clone blob field
    recordset& rs2 = *rs.clone();
    BOOST_CHECK_MESSAGE(rs2.size() == 15000, "outerJoin  rs2.size()== 15000");
    BOOST_CHECK_MESSAGE(rs2[NO_RECORD_ID-1].isInvalidRecord() == true, "outerJoin isInvalidRecord");
    BOOST_CHECK_MESSAGE(rs2[NO_RECORD_ID][_T("comment")].i() == NO_RECORD_ID+1, "row of 6 = '6 comment'");
    vs = rs2[NO_RECORD_ID][_T("blob")].c_str();
    ret = _tcscmp(vs, _T("6 blob")) == 0; 
    BOOST_CHECK_MESSAGE(ret == true, "row of 6 = '6 blob'");




    //hasManyJoin inner
    rs.clear();
    q.reset().reject(0xFFFF).limit(0).all();
    atg.keyValue(1).read(rs, q);
    BOOST_CHECK_MESSAGE(rs.size() == 100, "hasManyJoin  rs.size()== 100");
    q.all().optimize(queryBase::joinHasOneOrHasMany);
    atu.index(1).join(rs, q, _T("code"));
    BOOST_CHECK_MESSAGE(rs.size() == 20000, "hasManyJoin  rs.size()== 20000 size = " << rs.size());

    //hasManyJoin outer
    rs.clear();
    q.reset().reject(0xFFFF).limit(0).all();
    atg.keyValue(1).read(rs, q);
    BOOST_CHECK_MESSAGE(rs.size() == 100, "hasManyJoin  rs.size()== 100");
    q.all().optimize(queryBase::joinHasOneOrHasMany);
    atu.index(1).outerJoin(rs, q, _T("code"));
    BOOST_CHECK_MESSAGE(rs.size() == 20095, "hasManyJoin  rs.size()== 20095 size = " << rs.size());


    // restore record
    unsigned char bin[256];
    tb->clearBuffer();
    tb->setFV(_T("id"), NO_RECORD_ID);
    tb->setFV(_T("comment"), _T("5 comment"));
    tb->setFV(_T("blob"), _T("5 blob"));
    fillBlobField(3, NO_RECORD_ID, tb.get(), bin);
    tb->insert();
    BOOST_CHECK_MESSAGE(tb->stat()  == 0, "ate insert id = 5");
    if (tb->stat())
    {
        atu.release();
        atg.release();
        ate.release();
        db->drop();
    }
}

void testReadMore(database* db)
{
#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    atu.alias(fd_name, _T("name"));
    query q;
    q.select(_T("id"), _T("name"), _T("group"))
        .where(_T("name"), _T("="), _T("1*"))
        .reject(70).limit(8).stopAtLimit(true);
    pq_handle stmt1 = atu.prepare(q, true);
    BOOST_CHECK_MESSAGE(stmt1 != NULL, " stmt1");
    
    recordset rs;
    atu.index(0).keyValue(18).read(rs, stmt1);
    BOOST_CHECK_MESSAGE(rs.size() == 2, "read");

    recordset rs2;
    atu.readMore(rs2);
    BOOST_CHECK_MESSAGE(rs2.size() == 8, "readMore");

    rs += rs2;
    BOOST_CHECK_MESSAGE(rs.size() == 10, "union");

}

void testFirstLastGroupFunction(database* db)
{
#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));
    atu.alias(fd_name, _T("name"));
    query q;
    q.select(_T("id"), _T("name"), _T("group"))
        .where(_T("name"), _T("="), _T("1*"))
        .reject(70).limit(8).stopAtLimit(true);
    pq_handle stmt1 = atu.prepare(q, true);
    BOOST_CHECK_MESSAGE(stmt1 != NULL, " stmt1");
    
    recordset rs;
    atu.index(0).keyValue(0).read(rs, stmt1);
    BOOST_CHECK_MESSAGE(rs.size() == 8, "read");

    groupQuery gq;
    fieldNames target;
    target.addValue(_T("name"));
    client::last last(target, _T("last_rec_name"));
    client::first first(target, _T("first_rec_name"));
    gq.addFunction(&last);
    gq.addFunction(&first);

    rs.groupBy(gq);
    BOOST_CHECK_MESSAGE(rs.size() == 1, "read");
    
    BOOST_CHECK_MESSAGE(rs[0][_T("first_rec_name")].c_str() == std::_tstring(_T("1 user")), "first_rec_name");
    BOOST_CHECK_MESSAGE(rs[0][_T("last_rec_name")].c_str() == std::_tstring(_T("16 user")), "last_rec_name");

}

void testBlobOnlyTable(database* db)
{
    // table access test
    table* tb = db->openTable(_T("blobonly"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    tb->clearBuffer();
    static const int id = 1;
    tb->setFV((short)0, id);
    tb->seek();
    field fd = tb->fields()[_T("binary")];

    bool ret = compBlobField(id, fd);
    BOOST_CHECK_MESSAGE(ret == true, "tb->seek binary 256 byte");
    tb->release();

    // activeTable test
    activeTable at(db, _T("blobonly"));
    recordset rs;
    client::query q;
    q.where(_T("id"), _T("<"), 10);
    at.index(0).keyValue(1).read(rs, q);
    BOOST_CHECK_MESSAGE(rs.size() == 9, " rs.size() 9 bad = " << rs.size());
    for (int i= 0; i < 9; ++i)
    {
        fd = rs[i][_T("binary")];
        ret = compBlobField(i+1, fd);
        BOOST_CHECK_MESSAGE(ret == true, "rs[n][binary] binary 256 byte");
    }   
}

void testWirtableRecord(database* db)
{

#ifdef LINUX
    const char* fd_name = "名前";
#else
#ifdef _UNICODE
    const wchar_t fd_name[] = { L"名前" };
#else
    char fd_name[30];
    WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
#endif

    activeTable atu(db, _T("user"));

    writableRecord& rec = atu.index(0).getWritableRecord();

    rec[_T("id")] = 120000;
    rec[fd_name] = _T("aiba");
    rec.save();

    rec.clear();
    rec[_T("id")] = 120000;
    rec.read();
    BOOST_CHECK_MESSAGE(_tstring(rec[fd_name].c_str()) == _tstring(_T("aiba")),
                        "rec 120000 name ");

    rec.clear();
    rec[_T("id")] = 120001;
    bool r = rec.read();
    rec[fd_name] = _T("oono");
    if (!r)
        rec.insert();
    else
        rec.update();

    rec.clear();
    rec[_T("id")] = 120001;
    rec.read();

    BOOST_CHECK_MESSAGE(_tstring(rec[1].c_str()) == _tstring(_T("oono")),
                        "rec 120001 name ");
    // update changed filed only
    rec.clear();
    rec[_T("id")] = 120001;
    rec[fd_name] = _T("matsumoto");
    rec.update();

    rec.clear();
    rec[_T("id")] = 120001;
    rec.read();
    BOOST_CHECK_MESSAGE(_tstring(rec[fd_name].c_str()) ==
                            _tstring(_T("matsumoto")),
                        "rec 120001 update name ");

    rec.del();
    rec[_T("id")] = 120000;
    rec.del();

    rec.clear();
    rec[_T("id")] = 120001;
    bool ret = rec.read();
    BOOST_CHECK_MESSAGE(ret == false, "rec 120001 delete ");

    rec.clear();
    rec[_T("id")] = 120000;
    ret = rec.read();
    BOOST_CHECK_MESSAGE(ret == false, "rec 120000 delete ");
}

void testDbPool()
{
    pooledDbManager poolMgr;
    pooledDbManager::setMaxConnections(4);

    connectParams pm(PROTOCOL, HOSTNAME, _T("querytest"), DBNAME, g_userName, g_password);
    poolMgr.use(&pm);
    BOOST_CHECK_MESSAGE(1 == poolMgr.usingCount(), "usingCount 1");
    poolMgr.use(&pm);
    BOOST_CHECK_MESSAGE(2 == poolMgr.usingCount(), "usingCount 2");
    poolMgr.use(&pm);
    BOOST_CHECK_MESSAGE(3 == poolMgr.usingCount(), "usingCount 3");
    poolMgr.unUse();
    BOOST_CHECK_MESSAGE(0 == poolMgr.usingCount(), "usingCount 0");

    //snapshot method
    {
        poolMgr.use(&pm);
        table_ptr tb2 = openTable(poolMgr.db(), _T("user"));

        poolMgr.use(&pm);
        table_ptr tb = openTable(poolMgr.db(), _T("user"));


        poolMgr.beginSnapshot(MULTILOCK_GAP_SHARE);
        tb->seekFirst();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

        tb2->seekFirst(ROW_LOCK_X);
        BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), 
                    "tb2->seekFirst tb2 stat = " << tb2->stat());
    
        poolMgr.endSnapshot();
    
        tb2->seekFirst(ROW_LOCK_X);
        BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    }
    poolMgr.endSnapshot();

}

//--------------------------------------------------------------------------------------
//   filter test
//--------------------------------------------------------------------------------------
#define FILTER_DB _T("filter_test")

const _TCHAR* fdf_names[] = 
{
    _T("ft_string"),
    _T("ft_wstring"),
    _T("ft_zstring"),
    _T("ft_wzstring"),
    _T("ft_mychar"),
    _T("ft_mywchar"),
    _T("ft_myvarchar"),
    _T("ft_mywvarchar"),
    _T("ft_myvarbinary"),
    _T("ft_mywvarbinary"),
};

char fdf_types[] = 
{
    ft_string,
    ft_wstring,
    ft_zstring,
    ft_wzstring,
    ft_mychar,
    ft_mywchar,
    ft_myvarchar,
    ft_mywvarchar,
    ft_myvarbinary,
    ft_mywvarbinary,
};

#define FILTER_RECORDS 15
const _TCHAR* fd_values[15] = 
{
    _T("090-xxxx-xxx"),
    _T("090-xxxx-xxx"),
    _T(" "),
    _T("090-xxxx-xxx"),
    _T("080-xxxx-xxx"),
    _T("0"),
    _T(""),
    _T(""),
    _T("09"),
    _T("070"),
    _T(""),
    _T("090-xxxx-xxx"),
    _T("a90-xxxx-xxx"),
    _T("Aa0-xxxx-xxx"),
    _T("A90-xxxx-xxx"),
};

void intFieldTypes(database* db)
{
    if (!isUtf16leSupport(db))
    {
        for (int i = 0; i < 10; i++) 
        {
            if ((i % 2) == 1)
                fdf_types[i] = fdf_types[i-1];
        }
    }
}

void inserFilterTestRecords(database* db)
{


    table* tb = db->openTable(_T("user"), TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable stat = " << db->stat());
    db->beginTrn();
    for (int i = 0; i < FILTER_RECORDS; ++i)
    {
        tb->clearBuffer();
        tb->setFV((short)0, i);
        //set AllFields smae value
        for (int j=0;j<10;++j)
            tb->setFV(j+1, fd_values[i]);
        tb->insert();
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "insert stat = " << tb->stat());

    }
    db->endTrn();
    tb->release();
}

void createFilterTestDb(database* db)
{

    db->create(makeUri(PROTOCOL, HOSTNAME, FILTER_DB, BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        db->open(makeUri(PROTOCOL, HOSTNAME, FILTER_DB, BDFNAME), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);
        BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "open stat = " << db->stat());
        db->drop();
        BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "drop stat = " << db->stat());
        db->create(makeUri(PROTOCOL, HOSTNAME, FILTER_DB, BDFNAME));
    }
    
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createFilterTestDb stat = " << db->stat());
    // create table
    db->open(makeUri(PROTOCOL, HOSTNAME, FILTER_DB, BDFNAME), TYPE_SCHEMA_BDF,
             TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(),
                        "createFilterTestDb 1 stat = " << db->stat());

    intFieldTypes(db);

    dbdef* def = db->dbDef();
    if (def)
    {
        /*  user table */
        tabledef td;
        memset(&td, 0, sizeof(tabledef));
        td.setTableName(_T("user"));
        td.setFileName(_T("user.dat"));
        td.id = 1;
        /*td.primaryKeyNum = -1;
        td.parentKeyNum = -1;
        td.replicaKeyNum = -1;
        td.pageSize = 2048;*/
#ifdef _WIN32
        td.charsetIndex = CHARSET_CP932;
#else
        td.charsetIndex = CHARSET_UTF8;
#endif

        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "insertTable stat = " << def->stat());

        fielddef* fd = def->insertField(1, 0);
        fd->setName(_T("id"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;

        for (int i=0;i<10;++i)
        {
            fielddef* fd = def->insertField(1, i+1);
            fd->setName(fdf_names[i]);
            fd->type = fdf_types[i];
            fd->setLenByCharnum(20);
        }
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 1 stat = " << def->stat());

        keydef* kd = def->insertKey(1, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        for (int i=0;i<10;++i)
        {
            kd = def->insertKey(1, i+1);
            kd->segments[0].fieldNum = i+1;
            kd->segments[0].flags.bit0 = 1; // duplicate
            kd->segments[0].flags.bit8 = 1; // extended key type
            kd->segments[0].flags.bit1 = 1; // changeable
            kd->segmentCount = 1;
        }

        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(),
                            "updateTableDef 3 stat = " << def->stat());
        
        inserFilterTestRecords(db);
    }
}

void setReject(query& q)
{
    q.reject(0).limit(0);
}

void setReject(pq_handle& q)
{
    
}

template<class Q>
void doTestReadByQuery(int num, activeTable& at, recordset& rs, Q& q, 
                    int compSize, const char* msg)
{
    setReject(q);
    at.index(0).keyValue(0).read(rs, q);
    BOOST_CHECK_MESSAGE(compSize == (int)rs.size(),
            num << " " << msg << " " << compSize << " --> bad rs.size() = " << rs.size());
}

void testFilterOfServer(database* db)
{
    intFieldTypes(db);

    {
        activeTable atu(db, _T("user"));
        recordset rs;
        query q;
        for (int i = 0; i < 10; ++i)
        {
            // empty string
            
            q.reset().where(fdf_names[i], _T("="), _T(""));
            int n = 3;
            fielddef* fd = &atu.table()->tableDef()->fieldDefs[i+1];
            if (fd->isUsePadChar())
                n += 1;
            doTestReadByQuery(i, atu, rs, q, n, "");
            q.where(fdf_names[i], _T("=i"), _T(""));
            doTestReadByQuery(i, atu, rs, q, n, "=i");

            // match complate 
            q.where(fdf_names[i], _T("="), _T("070"));
            doTestReadByQuery(i, atu, rs, q, 1, "= 070");

            q.where(fdf_names[i], _T("=i"), _T("070"));
            doTestReadByQuery(i, atu, rs, q, 1, "=i 070");

            // match complate
            q.where(fdf_names[i], _T("<"), _T("09"));
            doTestReadByQuery(i, atu, rs, q, 7, "< 09");

            q.where(fdf_names[i], _T("<i"), _T("09"));
            doTestReadByQuery(i, atu, rs, q, 7, "<i 09");

            // wildcard and prerare
            // 0:noprepare 1:prepare 2:prepareServer
            for (int j = 0 ; j < 3; ++j)
            {
                q.reset().reject(0).limit(0);
                if (j > 0)
                {
                    q.where(fdf_names[i], _T("="), _T("?"));
                    pq_handle pq = atu.prepare(q, (j == 2));
                    supplyValue(pq, 0, _T("09*"));
                    doTestReadByQuery(i, atu, rs, pq, 5, "prepare = 09*");

                    q.where(fdf_names[i], _T("=i"), _T("?"));
                    pq_handle pq1 = atu.prepare(q, (j == 2));
                    supplyValue(pq1, 0, _T("09*"));
                    doTestReadByQuery(i, atu, rs, pq1, 5, "prepare =i 09*");

                }else
                {
                    q.where(fdf_names[i], _T("="), _T("09*"));
                    doTestReadByQuery(i, atu, rs, q, 5, "= 09*");

                    q.where(fdf_names[i], _T("=i"), _T("09*"));
                    doTestReadByQuery(i, atu, rs, q, 5, "=i 09*");
                }
            }

            // ascii
            q.where(fdf_names[i], _T("="), _T("a*"));
            doTestReadByQuery(i, atu, rs, q, 1, " = a*");

            q.where(fdf_names[i], _T("=i"), _T("a*"));
            doTestReadByQuery(i, atu, rs, q, 3, " =i a*");

            q.where(fdf_names[i], _T("="), _T("A*"));
            doTestReadByQuery(i, atu, rs, q, 2, " = A*");

            q.where(fdf_names[i], _T("=i"), _T("A*"));
            doTestReadByQuery(i, atu, rs, q, 3, " =i A*");

            q.where(fdf_names[i], _T("="), _T("AA0*"));
            doTestReadByQuery(i, atu, rs, q, 0, " = AA0*");

            q.where(fdf_names[i], _T("=i"), _T("AA0*"));
            doTestReadByQuery(i, atu, rs, q, 1, " =i Aa0*");

            //case in-sencitive index no jaudge
            
            for (int i = 0 ; i < 10 ; ++i)
            {
                q.where(fdf_names[i], _T("="), _T("A*"));
                setReject(q);
                atu.index(i+1).keyValue(_T("A")).read(rs, q);
                BOOST_CHECK_MESSAGE(2 == rs.size(), 
                    i << " " <<  "jaudge = A*" << " rs.size() = " << rs.size());
                BOOST_CHECK_MESSAGE(atu.table()->statReasonOfFind() == STATUS_REACHED_FILTER_COND, 
                    i << " " <<  "jaudge = A* FILTER_COND");

                q.where(fdf_names[i], _T("=i"), _T("A*"));
                setReject(q);
                atu.index(i+1).keyValue(_T("A")).read(rs, q);
                BOOST_CHECK_MESSAGE(3 == rs.size(), 
                    i << " " <<  "jaudge = A*" << " rs.size() = " << rs.size());
                BOOST_CHECK_MESSAGE(atu.table()->statReasonOfFind() == STATUS_EOF, 
                    i << " " <<  "jaudge = A* STATUS_EOF");

            }
        }
    }
}

void doTestMatchBy(int num, recordset& rs, recordsetQuery& rq, int compSize, const _TCHAR* msg)
{
    recordset* rss = rs.clone();
    rss->matchBy(rq);
    BOOST_CHECK_MESSAGE(compSize == (int)rss->size(),
                num << msg << _T(" rss->size = ") << rss->size());
    rss->release();
}

void testFilterOfMatchBy(database* db)
{
    intFieldTypes(db);
    {
        activeTable atu(db, _T("user"));
        query q;
        recordset rs;
        atu.index(0).keyValue(0).read(rs, q.all());
        BOOST_CHECK_MESSAGE(FILTER_RECORDS == rs.size(), " rs.size() = " << rs.size());
        for (int i = 0; i < 10; ++i)
        {
            // empty string
            recordsetQuery rq;
            rq.when(fdf_names[i], _T("="), _T(""));
            int n = 3;
            if (atu.table()->tableDef()->fieldDefs[i+1].isUsePadChar())
                n += 1;
            doTestMatchBy(i, rs, rq, n, _T(" = "));
            rq.reset().when(fdf_names[i], _T("=i"), _T(""));
            doTestMatchBy(i, rs, rq, n, _T(" =i "));

            
            // wildcard
            rq.reset().when(fdf_names[i], _T("="), _T("09*"));
            doTestMatchBy(i, rs, rq, 5, _T(" = 09*"));
            rq.reset().when(fdf_names[i], _T("=i"), _T("09*"));
            doTestMatchBy(i, rs, rq, 5, _T(" =i 09*"));
            
            // match complate
            rq.reset().when(fdf_names[i], _T("="), _T("070"));
            doTestMatchBy(i, rs, rq, 1, _T(" = 070"));
            rq.reset().when(fdf_names[i], _T("=i"), _T("070"));
            doTestMatchBy(i, rs, rq, 1, _T(" =i 070"));

            // match complate
            rq.reset().when(fdf_names[i], _T("<"), _T("09"));
            doTestMatchBy(i, rs, rq, 7, _T(" < 09"));

            rq.reset().when(fdf_names[i], _T("<i"), _T("09"));
            doTestMatchBy(i, rs, rq, 7, _T(" <i 09"));

            // ascii
            rq.reset().when(fdf_names[i], _T("="), _T("a*"));
            doTestMatchBy(i, rs, rq, 1, _T(" = a*"));

            rq.reset().when(fdf_names[i], _T("=i"), _T("a*"));
            doTestMatchBy(i, rs, rq, 3, _T(" =i a*"));

            rq.reset().when(fdf_names[i], _T("=i"), _T("A*"));
            doTestMatchBy(i, rs, rq, 3, _T(" =i A*"));

            rq.reset().when(fdf_names[i], _T("="), _T("AA0*"));
            doTestMatchBy(i, rs, rq, 0, _T(" = AA0*"));

            rq.reset().when(fdf_names[i], _T("=i"), _T("AA0*"));
            doTestMatchBy(i, rs, rq, 1, _T(" =i Aa0*"));

            
            BOOST_CHECK_MESSAGE(FILTER_RECORDS == rs.size(), " rs.size() = " << rs.size());
        }
    }
    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "drop stat = " << db->stat());
}

unsigned char field_types[] = 
{
    ft_myvarbinary,
    ft_mywvarbinary,
    ft_myvarchar,
    ft_mywvarchar,
    ft_string,
    ft_wstring,
    ft_lstring,
    ft_lvar,
    ft_myblob,
    ft_mytext
};

void testBinaryField()
{
    unsigned char data[255], buf[255];
    for (int i = 0; i < 255 ;++i)
        data[i] = i;
    
    fielddef fdd;
    fdd.type = ft_string;
    fdd.len = 255;
    fdd.pos = 0;

    client::field fd(buf, fdd, NULL/*, NULL, 0*/);
    for (int i = 0 ; i < 10; ++i)
    {
        fdd.type = field_types[i];
        int len = fdd.len - fdd.varLenBytes(); 
        if (i > 7)
        {
            fdd.len = 9; //blob type
            len = 255;
        } 
        fd.setBin(data, len);
        uint_td size;
        void* p = fd.getBin(size);
        BOOST_CHECK_MESSAGE(len == (int)size, "binary size type = " << i);

        BOOST_CHECK_MESSAGE(p != NULL, "binary ret type = " << i);
        int ret = memcmp(p, data, len);
        BOOST_CHECK_MESSAGE(0 == ret, "binary memcmp type = " << i);
    }
}


// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(btrv_nativ)

BOOST_FIXTURE_TEST_CASE(createNewDataBase, fixture)
{
    connectParams cp(PROTOCOL, HOSTNAME, DBNAME, BDFNAME, g_userName);
    _tprintf(_T("URI = %s\n"), cp.uri());
    bool ret = db()->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    if (ret)
    {
        db()->drop();
        if (db()->stat())
        {
            printf("test database drop error No.%d\nTest is stopped !" , db()->stat());
            exit(1);
        }
    }
    testCreateNewDataBase(db());
}
BOOST_FIXTURE_TEST_CASE(clone, fixture)
{
    testClone(db());
}

BOOST_FIXTURE_TEST_CASE(version, fixture)
{
    testVersion(db());
}

BOOST_FIXTURE_TEST_CASE(insert, fixture)
{
    testInsert(db());
}

BOOST_FIXTURE_TEST_CASE(find, fixture)
{
    testFind(db());
}

BOOST_FIXTURE_TEST_CASE(findNext, fixture)
{
    testFindNext(db());
    testFindIn(db());
    testPrepare(db());
    testPrepareServer(db());
}

BOOST_FIXTURE_TEST_CASE(getPercentage, fixture)
{
    testGetPercentage(db());
}

BOOST_FIXTURE_TEST_CASE(movePercentage, fixture)
{
    testMovePercentage(db());
}

BOOST_FIXTURE_TEST_CASE(getEqual, fixture)
{
    testGetEqual(db());
}

BOOST_FIXTURE_TEST_CASE(getNext, fixture)
{
    testGetNext(db());
}

BOOST_FIXTURE_TEST_CASE(getPrevious, fixture)
{
    testGetPrevious(db());
}

BOOST_FIXTURE_TEST_CASE(getGreater, fixture)
{
    testGetGreater(db());
}

BOOST_FIXTURE_TEST_CASE(getLessThan, fixture)
{
    testGetLessThan(db());
}

BOOST_FIXTURE_TEST_CASE(getFirst, fixture)
{
    testGetFirst(db());
}

BOOST_FIXTURE_TEST_CASE(getLast, fixture)
{
    testGetLast(db());
}

BOOST_FIXTURE_TEST_CASE(movePosition, fixture)
{
    testMovePosition(db());
}

BOOST_FIXTURE_TEST_CASE(update, fixture)
{
    testUpdate(db());
}

BOOST_FIXTURE_TEST_CASE(snapShot, fixture)
{
    testSnapshot(db());
}

BOOST_FIXTURE_TEST_CASE(conflict, fixture)
{
    testConflict(db());
}

BOOST_FIXTURE_TEST_CASE(transactionLockRepeatable, fixture)
{
    testTransactionLockRepeatable(db());
}

BOOST_FIXTURE_TEST_CASE(bug_015, fixture)
{
    testBug_015(db());
}

BOOST_FIXTURE_TEST_CASE(issue_016, fixture)
{
    testIssue_016(db());
}


BOOST_FIXTURE_TEST_CASE(transactionLock, fixture)
{
    testTransactionLockReadCommited(db());
}

BOOST_FIXTURE_TEST_CASE(RecordLock, fixture)
{
    testRecordLock(db());
}

BOOST_FIXTURE_TEST_CASE(MultiDatabase, fixture)
{
    testMultiDatabase(db());
}

BOOST_AUTO_TEST_CASE(Exclusive)
{
    testExclusive();
}

BOOST_FIXTURE_TEST_CASE(MissingUpdate, fixture)
{
    testMissingUpdate(db());
}

BOOST_FIXTURE_TEST_CASE(reconnect, fixture)
{
    testReconnect(db());
}

BOOST_FIXTURE_TEST_CASE(insert2, fixture)
{
    testInsert2(db());
}

BOOST_FIXTURE_TEST_CASE(delete_, fixture)
{
    testDelete(db());
}

BOOST_FIXTURE_TEST_CASE(setOwner, fixture)
{
    testSetOwner(db());
}


BOOST_FIXTURE_TEST_CASE(createIndex, fixture)
{
    testCreateIndex(db());
}

BOOST_FIXTURE_TEST_CASE(dropIndex, fixture)
{
    testDropIndex(db());
}

BOOST_FIXTURE_TEST_CASE(dropDatabase, fixture)
{
    testDropDatabase(db());
}

BOOST_FIXTURE_TEST_CASE(grantReload, fixture)
{
    testGrantReload(db());
}


BOOST_FIXTURE_TEST_CASE(connect, fixture)
{
    testLogin(db());
}
BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>

BOOST_AUTO_TEST_SUITE(convert)
BOOST_AUTO_TEST_CASE(u8tombc)
{
    char mbc[256];
    char u8[256] = "123";

    bzs::env::u8tombc(u8, -1, mbc, 256);
    BOOST_CHECK_MESSAGE(!strcmp(mbc, u8), u8);

    strcpy(u8, "漢字");
    unsigned char mbcKanji[20] = { 0x8A, 0xBF, 0x8E, 0x9A, 0x00 };
    bzs::env::u8tombc(u8, -1, mbc, 256);
    BOOST_CHECK_MESSAGE(!strcmp(mbc, (const char*)mbcKanji), u8);

    memset(u8, 0, 256);
    bzs::env::mbctou8(mbc, -1, u8, 256);
    BOOST_CHECK_MESSAGE(!strcmp(u8, "漢字"), "漢字2");
}

BOOST_AUTO_TEST_SUITE_END()
#endif
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(var_field)

BOOST_FIXTURE_TEST_CASE(createDataBaseVar, fixtureKanji)
{
    testCreateDataBaseVar(db());
}

BOOST_FIXTURE_TEST_CASE(varField, fixtureKanji)
{
    testVarField(db());
}

BOOST_FIXTURE_TEST_CASE(varInsert, fixtureKanji)
{
    testVarInsert(db());
}

BOOST_FIXTURE_TEST_CASE(varRead, fixtureKanji)
{
    testVarRead(db());
}

BOOST_FIXTURE_TEST_CASE(filterVar, fixtureKanji)
{
    testFilterVar(db());
}

BOOST_FIXTURE_TEST_CASE(dropDataBaseVar, fixtureKanji)
{
    testDropDataBaseVar(db());
}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------
#ifdef TDAP
// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)

BOOST_FIXTURE_TEST_CASE(stringFileter, fixtureKanji)
{
    testStringFileter(db());
}

BOOST_FIXTURE_TEST_CASE(dropDataBaseStr, fixtureKanji)
{
    testDropDataBaseStr(db());
}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------
#endif
// ------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(kanjiSchema)

BOOST_FIXTURE_TEST_CASE(knajiCreateSchema, fixtureKanji)
{
    testKnajiCreateSchema(db());
}

BOOST_FIXTURE_TEST_CASE(insertKanji, fixtureKanji)
{
    testInsertKanji(db());
}

BOOST_FIXTURE_TEST_CASE(getEqualKanji, fixtureKanji)
{
    testGetEqualKanji(db());
}

BOOST_FIXTURE_TEST_CASE(dropDatabaseKanji, fixtureKanji)
{
    testDropDatabaseKanji(db());
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)

BOOST_FIXTURE_TEST_CASE(resultField, fixture)
{

    if (db()->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME)))
        db()->drop();
    testCreateNewDataBase(db());
    testResultField(db());
    testResultDef();
    testLogic(db());
    testQuery();
}
BOOST_FIXTURE_TEST_CASE(drop, fixture)
{
    testDropDatabase(db());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(query)

BOOST_FIXTURE_TEST_CASE(new_delete, fixtureQuery)
{
    teetNewDelete(db());
}

BOOST_FIXTURE_TEST_CASE(join, fixtureQuery)
{
    testJoin(db());
    //testRecordsetClone(db());
    testPrepareJoin(db());
    testServerPrepareJoin(db());
    testWirtableRecord(db());
}

BOOST_FIXTURE_TEST_CASE(readMore, fixtureQuery)
{
    testReadMore(db());
}

BOOST_FIXTURE_TEST_CASE(firstLastGropuFunction, fixtureQuery)
{
    testFirstLastGroupFunction(db());
}

BOOST_FIXTURE_TEST_CASE(blobOnly, fixtureQuery)
{
    testBlobOnlyTable(db());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(dbPool)

BOOST_AUTO_TEST_CASE(pool)
{
    testDbPool();
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(filter)

BOOST_FIXTURE_TEST_CASE(serverFilter, fixture)
{
    createFilterTestDb(db());
    testFilterOfServer(db());
    testFilterOfMatchBy(db());
   
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(field)

BOOST_AUTO_TEST_CASE(binary)
{
    testBinaryField();    
}

BOOST_AUTO_TEST_CASE(fuga)
{
    BOOST_CHECK_EQUAL(2 * 3, 6);
}

BOOST_AUTO_TEST_SUITE_END()
    

// ------------------------------------------------------------------------
