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
//#define BOOST_TEST_MAIN
//#define BOOST_TEST_MODULE

#include <boost/test/included/unit_test.hpp>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/client/stringConverter.h>
#include <stdio.h>
#include <bzs/db/protocol/tdap/client/filter.h>
#include <bzs/example/queryData.h>
#include <bzs/db/protocol/tdap/client/memRecordset.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace std;

#define PROTOCOL _T("tdap")
static _TCHAR HOSTNAME[MAX_PATH] =
{_T("127.0.0.1")};
#define DBNAME	 _T("test")
#define BDFNAME _T("test.bdf")
// #define ISOLATION_REPEATABLE_READ
#define ISOLATION_READ_COMMITTED

static const short fdi_id = 0;
static const short fdi_name = 1;
boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]);

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (strstr(argv[i], "--host=") == argv[i])
        {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP, (CP_ACP == CP_UTF8) ? 0 : MB_PRECOMPOSED, argv[i] + 7, -1, HOSTNAME, MAX_PATH);
#else
            strcpy_s(HOSTNAME, MAX_PATH, argv[i] + 7);
#endif

        }
    }
    return 0;
}

static _TCHAR g_uri[MAX_PATH];

const _TCHAR* makeUri(const _TCHAR* protocol, const _TCHAR* host, const _TCHAR* dbname, const _TCHAR* dbfile = NULL)
{
    if (dbfile)
        _stprintf_s(g_uri, MAX_PATH, _T("%s://%s/%s?dbfile=%s"), protocol, host, dbname, dbfile);
    else
        _stprintf_s(g_uri, MAX_PATH, _T("%s://%s/%s"), protocol, host, dbname);
    return g_uri;

}

class fixture
{
    mutable database* m_db;

public:
    fixture() : m_db(NULL)
    {
		m_db = database::create();
		if (!m_db)
			printf("Error database::create()\n");
	}

	~fixture()
	{
		if (m_db)
			database::destroy(m_db);
    }

	::database* db() const {return m_db;}
};

table* openTable(database* db, short dbmode=TD_OPEN_NORMAL, short tbmode=TD_OPEN_NORMAL)
{

	db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF, dbmode);
	BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1" << db->stat());
	table* tb = db->openTable(_T("user"), tbmode);
	BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable" << db->stat());
	return tb;
}

void testDropDatabase(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDatabase 1");

    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "drop 2");
}

void testClone(database* db)
{
    database* db2 = db->clone();
    BOOST_CHECK_MESSAGE(db2 != NULL, "createNewDataBase stat = " << db->stat());
    if (db2)
		database::destroy(db2);
}

void testCreateNewDataBase(database* db)
{

	db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        testDropDatabase(db);
        db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    }
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase stat = " << db->stat());
    // create table
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1 stat = " << db->stat());

    dbdef* def = db->dbDef();
    if (def)
    {
        tabledef td;
        memset(&td, 0, sizeof(tabledef));
        td.setTableName(_T("user"));
        td.setFileName(_T("user.dat"));
        td.id = 1;
        td.primaryKeyNum = -1;
        td.parentKeyNum = -1;
        td.replicaKeyNum = -1;
        td.pageSize = 2048;
        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable stat = " << def->stat());

        fielddef* fd = def->insertField(1, 0);
        fd->setName(_T("id"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 1 stat = " << def->stat());

        fd = def->insertField(1, 1);
        fd->setName(_T("name"));
        fd->type = ft_zstring;
        fd->len = (ushort_td)33;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

        fd = def->insertField(1, 2);
        fd->setName(_T("select"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

        fd = def->insertField(1, 3);
        fd->setName(_T("in"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

        keydef* kd = def->insertKey(1, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 3 stat = " << def->stat());

    }

}

void testVersion(database* db)
{
	db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "Version connect stat = " << db->stat());
    if (0 == db->stat())
    {
        btrVersions vv;
        db->getBtrVersion(&vv);
        BOOST_CHECK_MESSAGE(0 == db->stat(), "Version");
        if (_tcscmp(PROTOCOL, _T("tdap")) == 0)
        {
            BOOST_CHECK_MESSAGE(atoi(CPP_INTERFACE_VER_MAJOR) == vv.versions[0].majorVersion,
                "clent_Major = " << vv.versions[0].majorVersion);
            BOOST_CHECK_MESSAGE(atoi(CPP_INTERFACE_VER_MINOR) == vv.versions[0].minorVersion,
                "clent_Miner = " << vv.versions[0].minorVersion);
            BOOST_CHECK_MESSAGE((int)'N' == (int)vv.versions[0].type, "clent_Type = " << vv.versions[0].type);

            BOOST_CHECK_MESSAGE(((5 == vv.versions[1].majorVersion)||(10 == vv.versions[1].majorVersion)),
                "mysql_server_Major = " << vv.versions[1].majorVersion);
            BOOST_CHECK_MESSAGE(((5 <= vv.versions[1].minorVersion)||(0 == vv.versions[1].minorVersion)),
                "mysql_server_Miner = " << vv.versions[1].minorVersion);
            BOOST_CHECK_MESSAGE((int)'M' == (int)vv.versions[1].type, "mysql_server_Type = " << vv.versions[1].type);

            BOOST_CHECK_MESSAGE(TRANSACTD_VER_MAJOR == vv.versions[2].majorVersion,
                "server_Major = " << vv.versions[2].majorVersion);
            BOOST_CHECK_MESSAGE(TRANSACTD_VER_MINOR == vv.versions[2].minorVersion,
                "server_Miner = " << vv.versions[2].minorVersion);
            BOOST_CHECK_MESSAGE((int)'T' == (int)vv.versions[2].type, "server_Type = " << vv.versions[2].type);
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
        if (i == 87170)
            i = 87170;
        tb->insert();

    }
    tb->commitBulkInsert();
    db->endTrn();

    BOOST_CHECK_MESSAGE(0 == tb->stat(), "Insert2");
	tb->release();
}

void testFind(database* db)
{

    table* tb = openTable(db);
    tb->setKeyNum(0);
    tb->clearBuffer();
    tb->setFilter(_T("id >= 10 and id < 20000"), 1, 0);
    int v = 10;
    tb->setFV((short)0, v);
    tb->find(table::findForword);
    int i = v;
    while (i < 20000)
    {
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "find stat");
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "find value " << i);
        tb->findNext(true); // 11 ～ 19
        ++i;
    }

    // backforword
    tb->clearBuffer();
    v = 19999;
    tb->setFV((short)0, v);
    tb->find(table::findBackForword);
    i = v;
    while (i >= 10)
    {
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "find stat");
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
        BOOST_CHECK_MESSAGE(0 == tb->stat(), "findNext stat()");
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "findNext value");

    }
	tb->release();
}

void testFindIn(database* db)
{

    table* tb = openTable(db);
    tb->setKeyNum(0);
    tb->clearBuffer();
    queryBase q;
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
	_ltot_s(1, buf, 20, 10);
    q.addSeekKeyValue(buf, true);

    for (int i=2;i<=10000;++i)
	{
        _ltot_s(i, buf, 20, 10);
		q.addSeekKeyValue(buf);
	}
	tb->setQuery(&q);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "find in stat = " << tb->stat());

    tb->find();
    int i = 0;
    while (0 == tb->stat())
    {

        BOOST_CHECK_MESSAGE(++i == tb->getFVint(fdi_id), "findNext in value");
        tb->findNext(true);
    }
    BOOST_CHECK_MESSAGE(i == 10000, "findNext in count");
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "find in end stat = " << tb->stat());

    //LogicalCountLimit
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
    tb->seekByPercentage(5000); // 50%
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "MovePercentage");
    // If mainus is less than 500 then ok.
    BOOST_CHECK_MESSAGE(true == (3000 > abs(10001 - tb->getFVint(fdi_id))), "MovePercentage 1");
	tb->release();
}

void testGetEqual(database* db)
{
    table* tb = openTable(db);
    db->beginSnapshot();
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
            tb->seekNext();
        db->endSnapshot();
    }
	tb->release();
}

void testGetPrevious(database* db)
{
    table* tb = openTable(db);
    db->beginSnapshot();
    int vv = 20001;
    tb->clearBuffer();
    tb->setFV((short)0, vv);
    tb->seek();
    BOOST_CHECK_MESSAGE(vv == tb->getFVint(fdi_id), "GetPrevious");
    for (int i = 20000; i > 1; i--)
    {
        tb->seekPrev();
        BOOST_CHECK_MESSAGE(i == tb->getFVint(fdi_id), "GetPrevious I");
    }
    tb->seekPrev();
    BOOST_CHECK_MESSAGE(_tstring(_T("kosaka")) == _tstring(tb->getFVstr(1)), "GetPrevious kosaka");

    db->endSnapshot();
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
    BOOST_CHECK_MESSAGE(vv - 2 == tb->getFVint(fdi_id), "GetLessThan GetPrevious");
	tb->release();
}

void testGetFirst(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(_tstring(_T("kosaka")) == _tstring(tb->getFVstr(1)), "GetFirst");
	tb->release();
}

void testGetLast(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();
    tb->seekLast();
    BOOST_CHECK_MESSAGE(20002 == tb->getFVint(fdi_id), "GetLast");
	tb->release();
}

void testMovePosition(database* db)
{
    table* tb = openTable(db);
    tb->clearBuffer();
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
    BOOST_CHECK_MESSAGE(vv - 2 == tb->getFVint(fdi_id), "GetLessThan GetPrevious");

    tb->seekByBookmark(pos);
    BOOST_CHECK_MESSAGE(15000 == tb->getFVint(fdi_id), "MovePosition");
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
    tb->seekPrev();                     // prev 19999
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
    BOOST_CHECK_MESSAGE(_tcscmp(_T("ABC"), tb->getFVstr(fdi_name)) == 0, "update changeInKey2");
	tb->release();
}

void testSnapShot(database* db)
{
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb = openTable(db);
    table* tb2 = openTable(db2);

    db->beginSnapshot();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "beginSnapShot");
    tb->setKeyNum(0);
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "seekFirst");
    int firstValue = tb->getFVint(fdi_name);
    tb->seekNext();
    /* -------------------------------------------------- */
    // Change data by another connection
    tb2->setKeyNum(0);
    tb2->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->seekFirst");
    tb2->setFV(fdi_name, tb2->getFVint(fdi_name) + 1);
    tb2->update();
#ifdef ISOLATION_READ_COMMITTED
    BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb2->update");
#else
#ifdef ISOLATION_REPEATABLE_READ
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb2->update(");
#endif
#endif
    /* -------------------------------------------------- */

    tb->seekFirst();
    int secondValue = tb->getFVint(fdi_name);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "secondValue");
    db->endSnapshot();

    BOOST_CHECK_MESSAGE(0 == db->stat(), "endSnapShot");
#ifdef ISOLATION_READ_COMMITTED
    BOOST_CHECK_MESSAGE(secondValue != firstValue, "repeatableRead");
#else
    BOOST_CHECK_MESSAGE(secondValue == firstValue, "repeatableRead");
#endif

    /* -------------------------------------------------- */
	tb->release();
	tb2->release();
    database::destroy(db2);
}

void testConflict(database* db)
{
    database* db2 = database::create();
    db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
    BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
    table* tb = openTable(db);
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

    /* --------------------------------------------------
     Change Non index field
     -------------------------------------------------- */
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

void testTransactionLock(database* db)
{

	database* db2 = database::create();
	db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
	BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
	table* tb = openTable(db);
	table* tb2 = openTable(db2);

	// ------------------------------------------------------
	// Read test that single record lock with read
	// ------------------------------------------------------
	db->beginTrn(LOCK_SINGLE_NOWAIT);
	tb->setKeyNum(0);
	tb->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

	// unlock first record.
	tb->seekNext();

	tb2->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");

	db2->beginTrn();
	tb2->setKeyNum(0);
	tb2->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb2->stat(), "tb->seekFirst");
    db2->endTrn();
	db->endTrn();

    // ------------------------------------------------------
    // Can't read test that multi record lock with read
	// ------------------------------------------------------
	db->beginTrn(LOCK_MULTI_NOWAIT);
    tb->setKeyNum(0);
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

	// move from first record.
    tb->seekNext();

    // The no transaction user can not read .
	tb2->seekFirst();
	BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");

    // The second transaction user can not lock same record.
    db2->beginTrn();
    tb2->setKeyNum(0);
	tb2->seekFirst();
    BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");
    db2->endTrn();
    db->endTrn();

	// ------------------------------------------------------
    // Can't read test that single record lock with change
    // ------------------------------------------------------
    db->beginTrn(LOCK_SINGLE_NOWAIT);
    tb->setKeyNum(0);
	tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");
    tb->setFV(fdi_name, _T("ABC"));
    tb->update();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

    // move from first record.
    tb->seekNext();

    tb2->seekFirst();
	BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");

    db2->beginTrn();
    tb2->setKeyNum(0);
	tb2->seekFirst();
	BOOST_CHECK_MESSAGE(STATUS_LOCK_ERROR == tb2->stat(), "tb->seekFirst");
    db2->endTrn();
    db->endTrn();

    // ------------------------------------------------------
	// Abort test that Single record lock transaction
    // ------------------------------------------------------
	db->beginTrn(LOCK_SINGLE_NOWAIT);
    tb->setKeyNum(0);
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
	BOOST_CHECK_MESSAGE(_tcscmp(tb2->getFVstr(fdi_name), _T("ABC")) == 0, "tb->seekFirst");

	tb->release();
	tb2->release();
	database::destroy(db2);
}



void testExclusive()
{

	//db mode exclusive
	database* db = database::create();
	table* tb = openTable(db, TD_OPEN_EXCLUSIVE);

	// Can not open another connections.
	database* db2 = database::create();
	db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
	BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
	db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF);
	BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat(), "open 1" << db->stat());

	table*  tb2 = db->openTable(_T("user"));
	BOOST_CHECK_MESSAGE(0 == db->stat(), "Exclusive opened 2");

	tb->setKeyNum(0);
	tb->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

	tb->setFV(fdi_name, _T("ABC123"));
	tb->update();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

	tb2->setKeyNum(0);
	tb2->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb2->stat(), "update");
	tb2->setFV(fdi_name, _T("ABC124"));
	tb2->update();
	BOOST_CHECK_MESSAGE(0 == tb2->stat(), "update");

	tb->release();
	tb2->release();
	database::destroy(db2);
	database::destroy(db);

	// table mode exclusive
	db = database::create();
	tb = openTable(db, TD_OPEN_READONLY, TD_OPEN_EXCLUSIVE);

	db2 = database::create();
	db2->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME), true);
	BOOST_CHECK_MESSAGE(0 == db2->stat(), "connect");
	db2->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF);
	BOOST_CHECK_MESSAGE(0 == db2->stat(), "open 1" << db->stat());

	// Can not open another connections.
	tb2 = db2->openTable(_T("user"));
	BOOST_CHECK_MESSAGE(STATUS_CANNOT_LOCK_TABLE == db2->stat(), "Exclusive opened 2");

	// Can open a same connection.
	tb2 = db->openTable(_T("user"));
	BOOST_CHECK_MESSAGE(0 == db->stat(), "Exclusive opened 2");

	tb->release();
	database::destroy(db2);
	database::destroy(db);

	//reopen and update
	db = database::create();
	tb = openTable(db);

	tb->setKeyNum(0);
	tb->seekFirst();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "tb->seekFirst");

	tb->setFV(fdi_name, _T("ABC123"));
	tb->update();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "update");

	tb->release();
	database::destroy(db);

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
    bool c = (abs(count - 20003) < 3000);
    BOOST_CHECK_MESSAGE(c == true, "RecordCount1");
    if (!c)
    {
        char tmp[256];
        sprintf_s(tmp, 256, "true record count = 20003 as estimate recordCount count = %d ", count);
        BOOST_CHECK_MESSAGE(false, tmp);
    }
    // true number
    BOOST_CHECK_MESSAGE((uint_td)20003 == tb->recordCount(false), "RecordCount2");
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
    BOOST_CHECK_MESSAGE(tb->stat() == STATUS_NOT_FOUND_TI, "delete changeInKey2");

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
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "SetOwner");
    tb->clearOwnerName();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "SetOwner");
	tb->release();
}

void testDropIndex(database* db)
{
    table* tb = openTable(db);
    tb->dropIndex(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "DropIndex");
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
        BOOST_CHECK_MESSAGE(0 == db->stat(), "new connection connect  db->stat() = " << db->stat());
        database::destroy(db2);

        db->disconnect(makeUri(PROTOCOL, HOSTNAME, _T("")));
        BOOST_CHECK_MESSAGE(0 == db->stat(), "disconnect  db->stat() = " << db->stat());

    }
    // invalid host name
    db->connect(makeUri(PROTOCOL, _T("localhost123"), _T("")));
    bool f = (db->stat() == ERROR_TD_INVALID_CLINETHOST) || (db->stat() == ERROR_TD_HOSTNAME_NOT_FOUND);
    BOOST_CHECK_MESSAGE(f, "bad host stat =" << db->stat());
    if (!f)
    {

#ifndef _UNICODE
        TCHAR buf[256];
        sprintf_s(buf, 256, "bad host db->stat()=%d", db->stat());
        BOOST_MESSAGE(buf);
#endif
    }
    testCreateNewDataBase(db);
    db->disconnect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "databese disconnect db->stat() = " << db->stat());

    // true database name
    db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "databese  connect db->stat() = " << db->stat());
    if (db->stat() == 0)
    {
        db->disconnect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
        BOOST_CHECK_MESSAGE(0 == db->stat(), "databese disconnect db->stat() = " << db->stat());
    }
    // invalid database name
    testDropDatabase(db);
    db->disconnect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "databese disconnect db->stat() = " << db->stat());

    db->connect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(25000 + 1049 == db->stat(), "databese connect db->stat() = " << db->stat());

    db->disconnect(makeUri(PROTOCOL, HOSTNAME, DBNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "databese disconnect db->stat() = " << db->stat());

}

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
void doCreateVarTable(database* db, int id, const _TCHAR* name, char fieldType, int charset)
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
    BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable");

    fielddef* fd = def->insertField(id, 0);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    fd = def->insertField(id, 1);
    fd->setName(_T("name"));
    fd->type = fieldType;
    if (fieldType == ft_mywvarchar)
        fd->len = (ushort_td)1 + mysql::charsize(CHARSET_UTF16LE) * 3; // max 3 char len byte
    else if (fieldType == ft_mywvarbinary)
        fd->len = (ushort_td)1 + mysql::charsize(CHARSET_UTF16LE) * 3; // max 6 char len byte
    else if (fieldType == ft_myvarchar)
    {
        if (charset == CHARSET_CP932)
            fd->len = (ushort_td)1 + mysql::charsize(CHARSET_CP932) * 3; // max 6 char len byte
        else if (charset == CHARSET_UTF8B4)
            fd->len = (ushort_td)1 + mysql::charsize(CHARSET_UTF8B4) * 3; // max 6 char len byte
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
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
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
        db->drop();

    db->create(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase stat = " << db->stat());
    if (0 == db->stat())
    {
        db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), BDFNAME), 0, 0);
        BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1");

        if (0 == db->stat())
        {
            doCreateVarTable(db, 1, _T("user1"), ft_myvarchar, CHARSET_CP932);
            doCreateVarTable(db, 2, _T("user2"), ft_myvarbinary, CHARSET_CP932);
            if (isUtf16leSupport(db))
                doCreateVarTable(db, 3, _T("user3"), ft_mywvarchar, CHARSET_CP932);
            doCreateVarTable(db, 4, _T("user4"), ft_mywvarbinary, CHARSET_CP932);
            doCreateVarTable(db, 5, _T("user5"), ft_myvarchar, CHARSET_UTF8B4);
            db->close();
            db->open(makeUri(PROTOCOL, HOSTNAME, _T("testvar"), TRANSACTD_SCHEMANAME), 0, 0);
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
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"123"), "Get Set W1");
        if (wstring(tb->getFVWstr(1)) != wstring(L"123"))
            dump((const char*)tb->getFVWstr(1), 7);
    }
    else
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"123456"), "Get Set W1");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 1");
    // short
    tb->setFVW(1, L"12 ");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"12 "), "Get Set W2");
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 2");
    // too long lanji

    if (unicodeField)
    {

        tb->setFVW(1, L"あいうえお\xD867\xDE3D"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"あいう"), "Get Set W3");
        else
            BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"あいうえお"), "Get Set W3");
    }
    else
    {
        tb->setFVW(1, L"0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(1)) == wstring(L"0松本"), "Get Set W3");
    }
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 2");
#endif

    // Set Ansi Get Wide
    // too long string
    tb->setFVA(1, "1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"), "Get Set A1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"), "Get Set A1");

#ifdef _WIN32
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 1");
#else
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 1");
#endif
    // short string
    tb->setFVA(1, "13 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("13 "), "Get Set A2");
#ifdef _WIN32
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 2");
#else
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");
#endif
    // too long lanji

    if (unicodeField)
    {
#ifdef LINUX
        tb->setFVA(1, "あいうえお𩸽"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"), "Get Set A3");
        else
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいうえお"), "Get Set A3");
#endif
    }
    else
    {
        tb->setFVA(1, "0松本市"); // kanji that "matumostoshi"
        bool f = string(tb->getFVAstr(1)) == string("0松本");
        BOOST_CHECK_MESSAGE(f, "Get Set A3");
        if (!f)
            BOOST_MESSAGE(tb->getFVAstr(1));

    }
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");

    // Set Wide Get Ansi
#ifdef _WIN32
    // too long string
    tb->setFVW(1, L"1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"), "GetA Set W1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"), "GetA Set W1");

    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 1");

    // short string
    tb->setFVW(1, L"23 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("23 "), "GetA Set W2");

    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 2");

    // too long lanji
    if (unicodeField)
    {

        tb->setFVW(1, L"あいうえお\xD867\xDE3D"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"), "GetA Set W3");
        else
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいうえお"), "GetA Set W3");
    }
    else
    {
        tb->setFVW(1, L"0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("0松本"), "GetA Set W3");
    }
    BOOST_CHECK_MESSAGE(wstring(tb->getFVWstr(2)) == wstring(L"68"), "Orverrun 2");
#endif
    // Set Ansi Get Ansi
    // too long string
    tb->setFVA(1, "1234567");
    if (varCharField)
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123"), "GetA Set A1");
    else
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("123456"), "GetA Set A1");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 1");
    // short string
    tb->setFVA(1, "13 ");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("13 "), "GetA Set A2");
    BOOST_CHECK_MESSAGE(string(tb->getFVAstr(2)) == string("68"), "Orverrun 2");

    // too long lanji
    if (unicodeField)
    {
#ifdef LINUX
        tb->setFVA(1, "あいうえお𩸽"); // kanji that "aiueohokke"
        if (varCharField)
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいう"), "Get Set A3");
        else
            BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("あいうえお"), "Get Set A3");
#endif
    }
    else
    {
        tb->setFVA(1, "0松本市"); // kanji that "matumostoshi"
        BOOST_CHECK_MESSAGE(string(tb->getFVAstr(1)) == string("0松本"), "GetA Set A3");
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
    BOOST_MESSAGE("Start acp varchar");
    doTestverField(tb, false, true);
    tb->release();

    tb = db->openTable(_T("user2"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable2");
    BOOST_MESSAGE("Start acp varbinary");
    doTestverField(tb, false, false);
    tb->release();

    if (isUtf16leSupport(db))
    {
        tb = db->openTable(_T("user3"));
        BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable3");
        BOOST_MESSAGE("Start unicode varchar");
        doTestverField(tb, true, true);
        tb->release();
    }
    tb = db->openTable(_T("user4"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable4");
    BOOST_MESSAGE("Start unicode varbinary");
    doTestverField(tb, true, false);
    tb->release();

    tb = db->openTable(_T("user5"));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable5");
    BOOST_MESSAGE("Start utf8 varchar");
    doTestverField(tb, true, true);

    tb->release();

}

void doVarInsert(database* db, const _TCHAR* name, unsigned int codePage, const _TCHAR* str, int start, int end,
    bool bulk)
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
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1");
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

void doVarRead(database* db, const _TCHAR* name, unsigned int codePage, const _TCHAR* str, int num, char_td key)
{

    table* tb = db->openTable(name);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
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
    BOOST_CHECK_MESSAGE((int)(num + 10) == tb->getFVint(2), "GetEqual var field2");

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
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1");
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

void doVarFilter(database* db, const _TCHAR* name, unsigned int codePage, const _TCHAR* str, int num, char_td key)
{
    table* tb = db->openTable(name);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
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
            BOOST_CHECK_MESSAGE((int)(i + 10) == tb->getFVint(2), "GetEqual var field2");
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
            BOOST_CHECK_MESSAGE((int)(i + 10) == tb->getFVint(2), "FindPrev var field2");
        }

        // test record count
        BOOST_CHECK_MESSAGE((uint_td)10 == tb->recordCount(), "GetEqual var field2");
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
    BOOST_CHECK_MESSAGE(0 == db->stat(), "open 1");
    if (0 == db->stat())
    {
        const _TCHAR* str = _T("漢字文");
        const _TCHAR* str3 = _T("漢字文字のテ");
        const _TCHAR* str2 = _T("123");
        const _TCHAR* str4 = _T("1232");
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

void stringFileterCreateTable(database* db, int id, const _TCHAR* name, uchar_td type, uchar_td type2)
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
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 1 stat = " << def->stat());

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
    BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

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
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 1");

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
	tb->commitBulkInsert();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "InsertStringFileter 5");

}

void doTestReadSF(table* tb)
{
    tb->setKeyNum(0);
    tb->clearBuffer();
    int id = 1;
    tb->setFV(_T("id"), id);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) == _tstring(tb->getFVstr(1)), "doTestReadSF2");
	BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) == _tstring(tb->getFVstr(1)), "doTestReadSF2b");

	id = 3;
    tb->setFV(_T("id"), id);
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF3");
	BOOST_CHECK_MESSAGE(_tstring(_T("あいがあればOKです")) == _tstring(tb->getFVstr(1)), "doTestReadSF4");
	BOOST_CHECK_MESSAGE(_tstring(_T("あいがあればOKです")) == _tstring(tb->getFVstr(2)), "doTestReadSF4b");

	tb->setKeyNum(1);
    tb->clearBuffer();
    tb->setFV(_T("name"), _T("A123456"));
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF5");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(1)), "doTestReadSF6");

    tb->setKeyNum(2);
    tb->clearBuffer();
    tb->setFV(_T("namew"), _T("A123456"));
    tb->seek();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF5");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)), "doTestReadSF6");

}

void doTestSF(table* tb)
{
    tb->setKeyNum(0);
    tb->clearBuffer();



    tb->setFilter(_T("name = 'あい*'"), 0, 10);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    tb->findNext(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("あいうえおかきくこ")) == _tstring(tb->getFVstr(1)), "doTestReadSF2");
    BOOST_CHECK_MESSAGE(2 == (int)tb->recordCount(), "doTestReadSF2");


    tb->setFilter(_T("name <> 'あい*'"), 0, 10);
    BOOST_CHECK_MESSAGE(3 == (int)tb->recordCount(), "doTestReadSF2");
    tb->clearBuffer();
	tb->setFilter(_T("name <> 'あい*'"), 0, 10);
    tb->seekFirst();
    tb->findNext(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1 stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");

    tb->findNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おはようございます")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");

    tb->findNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おめでとうございます。")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");
    tb->findNext();
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestReadSF1");

    tb->clearBuffer();
    tb->seekLast();
    tb->findPrev(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おめでとうございます。")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");

    tb->findPrev();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("おはようございます")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");

    tb->findPrev(false);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestReadSF1");
    BOOST_CHECK_MESSAGE(_tstring(_T("A123456")) == _tstring(tb->getFVstr(2)), "doTestReadSF1");

    tb->findPrev();
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestReadSF1");

    tb->setFilter(_T("name = 'あい'"), 0, 10);
    BOOST_CHECK_MESSAGE(0 == (int)tb->recordCount(), "doTestReadSF2");

    tb->setFilter(_T("name <> ''"), 0, 10);
    BOOST_CHECK_MESSAGE(5 == (int)tb->recordCount(), "doTestReadSF2");

    //test setFilter don't change field value
    tb->clearBuffer();
    tb->setFV(_T("name"), _T("ABCDE"));
    tb->setFilter(_T("name = 'あい'"), 0, 10);
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE")) == _tstring(tb->getFVstr(1)), "doTestReadSF2 field value");

}

void doTestUpdateSF(table* tb)
{

    tb->setKeyNum(0);
    tb->clearBuffer();
    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());
    tb->setFV(_T("name"), _T("ABCDE"));
    tb->setFV(_T("namew"), _T("ABCDEW"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());

    tb->setFV(_T("name"), _T("ABCDE2"));
    tb->setFV(_T("namew"), _T("ABCDEW2"));
    tb->update();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());

    tb->seekFirst();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE")) == _tstring(tb->getFVstr(1)), "doTestUpdateSF");
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDEW")) == _tstring(tb->getFVstr(2)), "doTestUpdateSF" );
    tb->seekNext();
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdateSF stat = " << tb->stat());
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDE2")) == _tstring(tb->getFVstr(1)), "doTestUpdateSF");
    BOOST_CHECK_MESSAGE(_tstring(_T("ABCDEW2")) == _tstring(tb->getFVstr(2)), "doTestUpdateSF" );

}

void doTestStringFileter(database* db, int id, const _TCHAR* name, uchar_td type, uchar_td type2)
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

void testStringFileter(database* db)
{
    db->create(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase");

    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME), 0, 0);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1 stat = " << db->stat());

    doTestStringFileter(db, 1, _T("zstring"), ft_zstring, ft_wzstring);
    if (isUtf16leSupport(db))
        doTestStringFileter(db, 2, _T("myvarchar"), ft_myvarchar, ft_mywvarchar);
    else
        doTestStringFileter(db, 2, _T("myvarchar"), ft_myvarchar, ft_myvarchar);

    doTestStringFileter(db, 3, _T("mytext"), ft_mytext, ft_myblob);

    db->close();

}

void testDropDataBaseStr(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, _T("testString"), BDFNAME), 0, 0);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1");
    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDataBaseTestString stat=" << db->stat());

}
// ------------------------------------------------------------------------

_TCHAR dbNmae[50] =
{_T("テスト")};
_TCHAR bdfNmae[50] =
{_T("構成.bdf")};
_TCHAR tableNmae[50] =
{_T("漢字テーブル")};
_TCHAR fdName1[50] =
{_T("番号")};
_TCHAR fdName2[50] =
{_T("名前")};

bool nameInited = false;

void initKanjiName()
{

#if (!defined(_UNICODE) && defined(_WIN32))
    if (!nameInited)
    {
        wchar_t tmp[50];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, dbNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, dbNmae, 50, NULL, NULL);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, bdfNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, bdfNmae, 50, NULL, NULL);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tableNmae, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, tableNmae, 50, NULL, NULL);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fdName1, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, fdName1, 50, NULL, NULL);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fdName2, -1, tmp, 50);
        WideCharToMultiByte(CP_UTF8, 0, tmp, -1, fdName2, 50, NULL, NULL);
        nameInited = true;
    }
#endif

}

void testDropDatabaseKanji(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae));
    BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDataBaseKanji 1");

    db->drop();
    BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDataBaseKanji 2");
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
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createKanjiDatabase stat = " << db->stat());
    // create table
    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae), TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "createKanjiDatabase 1 stat = " << db->stat());

    dbdef* def = db->dbDef();
    if (def)
    {
        tabledef td;
        memset(&td, 0, sizeof(tabledef));
#ifndef _UNICODE
        td.schemaCodePage = CP_UTF8;
#endif
        td.setTableName(tableNmae);
        td.setFileName(tableNmae);
        td.id = 1;
        td.primaryKeyNum = -1;
        td.parentKeyNum = -1;
        td.replicaKeyNum = -1;
        td.pageSize = 2048;

        def->insertTable(&td);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable stat = " << def->stat());

        fielddef* fd = def->insertField(1, 0);
        fd->setName(fdName1);
        fd->type = ft_integer;
        fd->len = (ushort_td)4;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 1 stat = " << def->stat());

        fd = def->insertField(1, 1);
        fd->setName(fdName2);
        fd->type = ft_zstring;
        fd->len = (ushort_td)33;
        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

        keydef* kd = def->insertKey(1, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        def->updateTableDef(1);
        BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 3 stat = " << def->stat());

    }
}

table* openKnajiTable(database* db)
{

    db->open(makeUri(PROTOCOL, HOSTNAME, dbNmae, bdfNmae), TYPE_SCHEMA_BDF, TD_OPEN_NORMAL);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openKnajiTable 1");
    table* tb = db->openTable(tableNmae);
    BOOST_CHECK_MESSAGE(0 == db->stat(), "openKnajiTable 2");
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
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fdName2), _T("小坂")) == 0, "GetEqual name 2");

    tb->setFV((short)0, 2);
    tb->seek();
    BOOST_CHECK_MESSAGE(2 == tb->getFVint(fdName1), "GetEqual id 2");
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fdName2), _T("矢口")) == 0, "GetEqual name 2");
	tb->release();

}

// ------------------------------------------------------------------------
void testResultField(database* db)
{
    table* tb = openTable(db);
    resultField rf;
    rf.setParam(tb, _T("name"));

    BOOST_CHECK_MESSAGE(rf.len == 33, " resultField.setParam");
    BOOST_CHECK_MESSAGE(rf.pos == 4, " resultField.setParam");

    size_t len = rf.writeBuffer(0, true) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 4, " resultField.writeBuffer");

}

void testResultDef()
{
    resultDef rd;

    rd.reset();
    BOOST_CHECK_MESSAGE(rd.maxRows == 0, " resultDef.maxRows");
    BOOST_CHECK_MESSAGE(rd.fieldCount == 0, " resultDef.fieldCount");

    size_t len = rd.writeBuffer(0, true) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 4, " resultDef.writeBuffer");

 
}


void testLogic(database* db)
{
    table* tb = openTable(db);
    logic lc;

    lc.setParam(tb, _T("name"), _T("="), _T("abc"), eCend);


    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    BOOST_CHECK_MESSAGE(lc.len == 33, " logic.len");
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1, " logic.logType");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "abc")==0, " logic.data");

    size_t len = lc.writeBuffer(0, true, false) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 7+33, " logic.writeBuffer");

    //compField invalid filed name
    bool ret = lc.setParam(tb, _T("name"), _T("="), _T("1"), eCend, true);
    BOOST_CHECK_MESSAGE(ret == false, " logic invalid filed name");

    //compField
    ret = lc.setParam(tb, _T("name"), _T("="), _T("id"), eCend, true);
	BOOST_CHECK_MESSAGE(ret == true, " logic filed name");
    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    BOOST_CHECK_MESSAGE(lc.len == 33, " logic.len");
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1+CMPLOGICAL_FIELD, " logic.logType compField");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    BOOST_CHECK_MESSAGE(*((short*)lc.data) == 0, " logic.data");
    len = lc.writeBuffer(0, true, false) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 7+2, " logic.writeBuffer");

    //invalid filed name
	ret = lc.setParam(tb, _T("name1"), _T("="), _T("id"), eCend, true);
	BOOST_CHECK_MESSAGE(ret == false, " logic invalid filed name2");

	//wildcard
	lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCend, false);
    BOOST_CHECK_MESSAGE(lc.type == ft_zstring, " logic.type");
    BOOST_CHECK_MESSAGE(lc.len == 3, " logic.len");
    BOOST_CHECK_MESSAGE(lc.pos == 4, " logic.pos");
    BOOST_CHECK_MESSAGE(lc.logType == 1, " logic.logType");
    BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic.opr");
    BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "abc")==0, " logic.data");

    len = lc.writeBuffer(0, true, false) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 7+3, " logic.writeBuffer");

	lc.setParam(tb, _T("name"), _T("="), _T("漢字*"), eCend, false);
	BOOST_CHECK_MESSAGE(strcmp((char*)lc.data, "漢字")==0, " logic.data");

	len = lc.writeBuffer(0, true, false) - (unsigned char*)0;
	BOOST_CHECK_MESSAGE(len == 7 + (_tcslen(_T("漢字"))*sizeof(_TCHAR))
									, " logic.writeBuffer len =" << len);


	//combine
	lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCor, false);
	BOOST_CHECK_MESSAGE(lc.opr == 2, " logic.opr or");
	lc.setParam(tb, _T("name"), _T("="), _T("abc*"), eCand, false);
    BOOST_CHECK_MESSAGE(lc.opr == 1, " logic.opr and");

    //logType
    ret = lc.setParam(tb, _T("name"), _T("!="), _T("abc*"), eCend, false);
    BOOST_CHECK_MESSAGE(lc.logType == 255, " logic.logType !=");
	BOOST_CHECK_MESSAGE(ret == false, " logic invalid logType");

    //canJoin

    //zstring is cannot join
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

    //join
    lc.joinAfter(&lc2);
    BOOST_CHECK_MESSAGE(lc.len == 8, " logic joinAfter");

	BOOST_CHECK_MESSAGE(lc.opr == eCend, " logic joinAfter");

    header hd;
    len = hd.writeBuffer(0, true) - (unsigned char*)0;
    BOOST_CHECK_MESSAGE(len == 8, " header.writeBuffer");


}



void testQuery()
{
    queryBase q;
    q.queryString(_T("id = 0 and name = 'Abc efg'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("id = '0' and name = 'Abc efg'")
                          ,  "queryString");
    
    q.queryString(_T(""));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("and"), _T("name"), _T("="), _T("Abc efg"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("id = '0' and name = 'Abc efg'"), "queryString");
    
    q.queryString(_T("select id,name id = 0 AND name = 'Abc&' efg'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '0' AND name = 'Abc&' efg'")
                          ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("AND"), _T("name"), _T("="), _T("Abc' efg"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '0' AND name = 'Abc&' efg'")
                          ,  "queryString");
    
    q.queryString(_T("select id,name id = 0 AND name = 'Abc&& efg'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '0' AND name = 'Abc&& efg'")
                          ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("0"));
    q.addLogic(_T("AND"), _T("name"), _T("="), _T("Abc& efg"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '0' AND name = 'Abc&& efg'")
                          ,  "queryString");
    
    q.queryString(_T("*"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("*"),  "queryString");
    
    q.queryString(_T(""));
    q.all();
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("*"),  "queryString");
    
    q.queryString(_T("Select id,name id = 2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '2'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name id = '2'")
                            ,  "queryString");
    
    q.queryString(_T("SELECT id,name,fc id = 2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc id = '2'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc id = '2'")
                            ,  "queryString");
    
    q.queryString(_T("select id,name,fc id = 2 and name = '3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc id = '2' and name = '3'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addLogic(_T("id"), _T("="), _T("2"));
    q.addLogic(_T("and"), _T("name"), _T("="), _T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc id = '2' and name = '3'")
                            ,  "queryString");
    
    //IN include
    q.queryString(_T("select id,name,fc IN '1','2','3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc in '1','2','3'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addField(_T("id"));
    q.addField(_T("name"));
    q.addField(_T("fc"));
    q.addSeekKeyValue(_T("1"));
    q.addSeekKeyValue(_T("2"));
    q.addSeekKeyValue(_T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select id,name,fc in '1','2','3'")
                            ,  "queryString");
    
    q.queryString(_T("IN '1','2','3'"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'")
                            ,  "queryString");
    
    q.queryString(_T("IN 1,2,3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addSeekKeyValue(_T("1"));
    q.addSeekKeyValue(_T("2"));
    q.addSeekKeyValue(_T("3"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in '1','2','3'")
                            ,  "queryString");
    
    //special field name
    q.queryString(_T("select = 1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select = '1'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addLogic(_T("select"), _T("="), _T("1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("select = '1'")
                            ,  "queryString");
    
    q.queryString(_T("in <> 1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in <> '1'")
                            ,  "queryString");
    
    q.queryString(_T(""));
    q.addLogic(_T("in"), _T("<>"), _T("1"));
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("in <> '1'")
                            ,  "queryString");

    //test auto_escape
    q.queryString(_T("code = ab'c"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&'c'")
                            ,  "queryString");

    q.queryString(_T("code = ab&c"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&&c'")
                            ,  "queryString");

    q.queryString(_T("code = abc&"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&'")
                            ,  "queryString");
    q.queryString(_T("code = abc&&"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&&&'")
                            ,  "queryString");

    q.queryString(_T("code = 'abc&'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&'")
                            ,  "queryString");
    q.queryString(_T("code = 'abc&&'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&&&&'")
                            ,  "queryString");

    q.queryString(_T("code = 'ab'c'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'ab&'c'")
                            ,  "queryString");

    q.queryString(_T("code = 'abc''"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&''")
                            ,  "queryString");

    q.queryString(_T("code = abc'"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc&''")
                            ,  "queryString");

    // Invalid end no close '
    q.queryString(_T("code = 'abc"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = 'abc'")
                            ,  "queryString");

    q.queryString(_T("code = &abc"), true);
    BOOST_CHECK_MESSAGE(_tstring(q.toString()) == _T("code = '&&abc'")
							,  "queryString");

}

void testJoin(database* db)
{

#ifdef LINUX
	const char* fd_name = "名前";
#else 
	#ifdef _UNICODE
		const wchar_t* fd_name = L"名前";
	#else
		char fd_name[30];
		WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
	#endif
#endif

	queryTable atu(db, _T("user"));
	queryTable atg(db, _T("groups"));
	queryTable ate(db, _T("extention"));
	recordset rs;
	query q;

	atu.alias(fd_name, _T("name"));
	q.select(_T("id"), _T("name"),_T("group")).where(_T("id"), _T("<="), 15000);
	atu.index(0).keyValue(1).read(rs, q);
	BOOST_CHECK_MESSAGE(rs.size()== 15000, " rs.size()== 15000");

	//Join extention::comment
	q.reset();
	ate.index(0).join(rs, q.select(_T("comment")).optimize(queryBase::joinKeyValuesUnique), _T("id"));
	BOOST_CHECK_MESSAGE(rs.size()== 15000, "join  rs.size()== 15000");

	//test reverse
	row_ptr last = rs.reverse().first();
	BOOST_CHECK_MESSAGE((*last)[_T("id")].i() == 15000, "last field id == 15000");
	BOOST_CHECK_MESSAGE(_tstring((*last)[_T("comment")].c_str()) == _tstring(_T("15000 comment"))
				, "last field comment");

	//Join group::name
	q.reset();
	atg.alias(_T("name"), _T("group_name"));
	atg.index(0).join(rs, q.select(_T("group_name")), _T("group"));
	BOOST_CHECK_MESSAGE(rs.size()== 15000, "join2  rs.size()== 15000");
	row_ptr first = rs.last();

	BOOST_CHECK_MESSAGE((*first)[_T("id")].i() == 1, "first field id == 1");
	BOOST_CHECK_MESSAGE(_tstring((*first)[_T("comment")].c_str()) == _tstring(_T("1 comment"))
				, "first field comment");

	BOOST_CHECK_MESSAGE(_tstring((*first)[_T("group_name")].c_str()) == _tstring(_T("1 group"))
				, "first field group_name " << string((*first)[_T("group_name")].a_str()));
	BOOST_CHECK_MESSAGE(_tstring((*first)[_T("group_name")].c_str()) == _tstring(_T("1 group"))
				, "first field group_name " << string((*first)[_T("group_name")].a_str()));
	row_ptr row = rs[15000 - 9];
	BOOST_CHECK_MESSAGE(_tstring((*row)[_T("group_name")].c_str()) == _tstring(_T("9 group"))
				, "group_name = 9 group " << string((*row)[_T("group_name")].a_str()));

	//Test orderby
	rs.orderBy(_T("group_name"));
	first = rs[0];
	BOOST_CHECK_MESSAGE(_tstring((*first)[_T("group_name")].c_str()) == _tstring(_T("1 group"))
				, "group_name = 1 group " << string((*first)[_T("group_name")].a_str()));

	//test union
	recordset rs2;
	atu.alias(fd_name, _T("name"));

	q.reset().select(_T("id"), _T("name"),_T("group")).where(_T("id"), _T("<="), 16000);
	atu.index(0).keyValue(15001).read(rs2, q);
	ate.index(0).join(rs2, q.reset().select(_T("comment")).optimize(queryBase::joinKeyValuesUnique), _T("id"));

	atg.alias(_T("name"), _T("group_name"));
	atg.index(0).join(rs2, q.reset().select(_T("group_name")), _T("group"));
	BOOST_CHECK_MESSAGE(rs2.size()== 1000, "join2  rs2.size()== 1000");

	rs += rs2;
	BOOST_CHECK_MESSAGE(rs.size()== 16000, "union  rs.size()== 16000");
	row = rs[15000];
	BOOST_CHECK_MESSAGE((*row)[_T("id")].i() == 15001,"id = 15001");
	row = rs.last();
	BOOST_CHECK_MESSAGE((*row)[_T("id")].i() == 16000,"id = 16000");
	

	//test group by
	groupQuery gq;
	gq.keyField(_T("group"),_T("id")).resultField(_T("count"));
	rs.groupBy(gq, group_count());
	BOOST_CHECK_MESSAGE(rs.size()== 16000, "group by  rs.size()== 16000");

	gq.reset();
	gq.keyField(_T("group")).resultField(_T("count"));
	rs.groupBy(gq, group_count());
	BOOST_CHECK_MESSAGE(rs.size()== 100, "group by2  rs.size()== 100");

	//use sum function object by for_each
	group_sum f;
	f.setResultKey((int)rs.fieldDefs()->size()-1);
	f = std::for_each(rs.begin(),rs.end(), f);
	BOOST_CHECK_MESSAGE(f.result()== 16000, "sum ");

	//top
	recordset rs3;
	rs.top(rs3, 10);
	BOOST_CHECK_MESSAGE(rs3.size()== 10, "top 10  rs3.size()== 10");


}

void testWirtableRecord(database* db)
{

#ifdef LINUX
	const char* fd_name = "名前";
#else 
	#ifdef _UNICODE
		const wchar_t* fd_name = L"名前";
	#else
		char fd_name[30];
		WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
	#endif
#endif

	queryTable atu(db, _T("user"));

	writableRecord& rec = atu.index(0).getWritableRecord();

	rec[_T("id")] = 120000;
	rec[fd_name] = _T("aiba");
	rec.save();

	rec.clear();
	rec[_T("id")] = 120000;
	rec.read();
	BOOST_CHECK_MESSAGE(_tstring(rec[fd_name].c_str()) == _tstring(_T("aiba"))
					, "rec 120000 name ");

	rec.clear();
	rec[_T("id")] = 120001;
	rec[fd_name]  = _T("oono");
	if (!rec.read())
		rec.insert();

	rec.clear();
	rec[_T("id")] = 120001;
	rec.read();
	BOOST_CHECK_MESSAGE(_tstring(rec[fd_name].c_str()) == _tstring(_T("oono"))
					, "rec 120001 name ");

	//update changed filed only
	rec.clear();
	rec[_T("id")]  = 120001;
	rec[fd_name] = _T("matsumoto");
	rec.update();

	rec.clear();
	rec[_T("id")] = 120001;
	rec.read();
	BOOST_CHECK_MESSAGE(_tstring(rec[fd_name].c_str()) == _tstring(_T("matsumoto"))
					, "rec 120001 update name ");

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


// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(btrv_nativ)

    BOOST_FIXTURE_TEST_CASE(createNewDataBase, fixture)
    {
        const _TCHAR* uri = makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME);
		_tprintf(_T("URI = %s\n"), uri);
		if (db()->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME)))
            db()->drop();
        testCreateNewDataBase(db());
    }
    BOOST_FIXTURE_TEST_CASE(clone, fixture) {testClone(db());}

    BOOST_FIXTURE_TEST_CASE(version, fixture) {testVersion(db());}

    BOOST_FIXTURE_TEST_CASE(insert, fixture) {testInsert(db());}

    BOOST_FIXTURE_TEST_CASE(find, fixture) {testFind(db());}

    BOOST_FIXTURE_TEST_CASE(findNext, fixture)
	{
        testFindNext(db());
        testFindIn(db());
    }

    BOOST_FIXTURE_TEST_CASE(getPercentage, fixture) {testGetPercentage(db());}

    BOOST_FIXTURE_TEST_CASE(movePercentage, fixture) {testMovePercentage(db());}

    BOOST_FIXTURE_TEST_CASE(getEqual, fixture) {testGetEqual(db());}

    BOOST_FIXTURE_TEST_CASE(getNext, fixture) {testGetNext(db());}

	BOOST_FIXTURE_TEST_CASE(getPrevious, fixture) {testGetPrevious(db());}

    BOOST_FIXTURE_TEST_CASE(getGreater, fixture) {testGetGreater(db());}

    BOOST_FIXTURE_TEST_CASE(getLessThan, fixture) {testGetLessThan(db());}

	BOOST_FIXTURE_TEST_CASE(getFirst, fixture) {testGetFirst(db());}

	BOOST_FIXTURE_TEST_CASE(getLast, fixture) {testGetLast(db());}

	BOOST_FIXTURE_TEST_CASE(movePosition, fixture) {testMovePosition(db());}

	BOOST_FIXTURE_TEST_CASE(update, fixture) {testUpdate(db());}

	BOOST_FIXTURE_TEST_CASE(snapShot, fixture) {testSnapShot(db());}

	BOOST_FIXTURE_TEST_CASE(conflict, fixture) {testConflict(db());}

	BOOST_FIXTURE_TEST_CASE(transactionLock, fixture) {testTransactionLock(db());}

	BOOST_AUTO_TEST_CASE(Exclusive) {testExclusive();}

	BOOST_FIXTURE_TEST_CASE(insert2, fixture) {testInsert2(db());}

	BOOST_FIXTURE_TEST_CASE(delete_, fixture) {testDelete(db());}

	BOOST_FIXTURE_TEST_CASE(setOwner, fixture) {testSetOwner(db());}

	BOOST_FIXTURE_TEST_CASE(dropIndex, fixture) {testDropIndex(db());}

	BOOST_FIXTURE_TEST_CASE(dropDatabase, fixture) {testDropDatabase(db());}

	BOOST_FIXTURE_TEST_CASE(connect, fixture) {testLogin(db());}
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
        char mbcKanji[20] =
        {0x8A, 0xBF, 0x8E, 0x9A, 0x00};
		bzs::env::u8tombc(u8, -1, mbc, 256);
        BOOST_CHECK_MESSAGE(!strcmp(mbc, mbcKanji), u8);

        memset(u8, 0, 256);
        bzs::env::mbctou8(mbc, -1, u8, 256);
        BOOST_CHECK_MESSAGE(!strcmp(u8, "漢字"), "漢字2");
    }

BOOST_AUTO_TEST_SUITE_END()
#endif
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(var_field)

    BOOST_FIXTURE_TEST_CASE(createDataBaseVar, fixture) {testCreateDataBaseVar(db());}

    BOOST_FIXTURE_TEST_CASE(varField, fixture) {testVarField(db());}

    BOOST_FIXTURE_TEST_CASE(varInsert, fixture) {testVarInsert(db());}

    BOOST_FIXTURE_TEST_CASE(varRead, fixture) {testVarRead(db());}

    BOOST_FIXTURE_TEST_CASE(filterVar, fixture) {testFilterVar(db());}

    BOOST_FIXTURE_TEST_CASE(dropDataBaseVar, fixture) {testDropDataBaseVar(db());}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)

    BOOST_FIXTURE_TEST_CASE(stringFileter, fixture) {testStringFileter(db());}

    BOOST_FIXTURE_TEST_CASE(dropDataBaseStr, fixture) {testDropDataBaseStr(db());}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(kanjiSchema)

	BOOST_FIXTURE_TEST_CASE(knajiCreateSchema, fixture) {testKnajiCreateSchema(db());}

	BOOST_FIXTURE_TEST_CASE(insertKanji, fixture) {testInsertKanji(db());}

	BOOST_FIXTURE_TEST_CASE(getEqualKanji, fixture) {testGetEqualKanji(db());}

	BOOST_FIXTURE_TEST_CASE(dropDatabaseKanji, fixture) {testDropDatabaseKanji(db());}

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

	BOOST_AUTO_TEST_CASE(join)
	{
		database_ptr db = createDatadaseObject();
		connectParams param(PROTOCOL, HOSTNAME, _T("querytest"), _T("test.bdf"));
		param.setMode(TD_OPEN_NORMAL);
		int ret = prebuiltData(db, param);
		BOOST_CHECK_MESSAGE(0 == ret, "query data build");
		if (ret==0)
		{
			testJoin(db.get());
			testWirtableRecord(db.get());
		}
	}

	BOOST_AUTO_TEST_CASE(fuga)
	{
		BOOST_CHECK_EQUAL(2 * 3, 6);
	}
BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------


