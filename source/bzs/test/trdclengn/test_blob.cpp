/* =================================================================
 Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include <boost/test/included/unit_test.hpp>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/client/stringConverter.h>
#include <stdio.h>
#include <bzs/db/protocol/tdap/client/filter.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace std;
#define ISOLATION_READ_COMMITTED

#define URL _T("tdap://localhost/testString?dbfile=test.bdf")
#define TABLE_NAME _T("comments")
#define TABLE_ID 1

#define FDN_ID		_T("id")
#define FDN_USER	_T("user_id")
#define FDN_BODY	_T("body")
#define FDN_IMAGE	_T("image")
#define FDI_ID		(short)0
#define FDI_USER	(short)1
#define FDI_BODY	(short)2
#define FDI_IMAGE	(short)3

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]);

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
	return 0;
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

// ------------------------------------------------------------------------

void doInsertStringFileter(table* tb)
{
	tb->clearBuffer();
	tb->setFV(FDI_USER,  1);
	tb->setFV(FDI_BODY,	 _T("1\ntest\nテスト\n\nあいうえおあいうえおb"));
	tb->setFV(FDI_IMAGE, _T("1\ntest\nテスト\n\nあいうえおあいうえおi"));
	tb->insert();

	tb->clearBuffer();
	tb->setFV(FDI_USER,  1);
	tb->setFV(FDI_BODY,  _T("2\ntest\nテスト\n\nあいうえおあいうえおb"));
	tb->setFV(FDI_IMAGE, _T("2\ntest\nテスト\n\nあいうえおあいうえおi"));
	tb->insert();

	tb->clearBuffer();
	tb->setFV(FDI_USER,  2);
	tb->setFV(FDI_BODY,  _T("3\ntest\nテスト\n\nあいうえおあいうえおb"));
	tb->setFV(FDI_IMAGE, _T("3\ntest\nテスト\n\nあいうえおあいうえおi"));
	tb->insert();
}

void doTestSeek(table* tb)
{
	uint_td dummy = 0;
	// 1
	tb->clearBuffer();
	tb->setFV(FDI_ID, 1);
	tb->seek();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestSeek - stat 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 1, "doTestSeek - id 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 1, "doTestSeek - user_id 1");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("1\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestSeek - body 1");
	// 2
	tb->seekNext();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestSeek - stat 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 2, "doTestSeek - id 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 1, "doTestSeek - user_id 2");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("2\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestSeek - body 2");
	// 3
	tb->seekNext();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestSeek - stat 3");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 3, "doTestSeek - id 3");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 2, "doTestSeek - user_id 3");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("3\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestSeek - body 3");
	// 2
	tb->seekPrev();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestSeek - stat 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 2, "doTestSeek - id 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 1, "doTestSeek - user_id 2");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("2\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestSeek - body 2");
}

void doTestFind(table* tb)
{
	tb->setKeyNum(0);
	tb->clearBuffer();

	tb->setFilter(_T("id >= 1 and id < 3"), 1, 0);
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFind - setFilter");

	tb->setFV(FDI_ID, 1);
	tb->find(table::findForword);
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFind - find");
	BOOST_CHECK_MESSAGE(1 == tb->getFVint(FDI_ID), "doTestFind - getFVint 1");

	tb->findNext(true);
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFind - findNext");
	BOOST_CHECK_MESSAGE(2 == tb->getFVint(FDI_ID), "doTestFind - getFVint 2");
	BOOST_CHECK_MESSAGE(
		_tstring(_T("2\ntest\nテスト\n\nあいうえおあいうえおb")) == _tstring(tb->getFVstr(FDI_BODY)),
		"doTestFind - getFVstr 2");
	tb->findNext(true);
	BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestFind - findNext");
	// 2
	//tb->findPrev(true);
	//BOOST_CHECK_MESSAGE(tb->stat() == STATUS_PROGRAM_ERROR, "doTestFind - findPrev");
}

void doTestUpdate(table* tb)
{
	// select 1
	tb->clearBuffer();
	tb->setFV(FDI_ID, 1);
	tb->seek();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - stat 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 1, "doTestUpdate - id 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 1, "doTestUpdate - user_id 1");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("1\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestUpdate - body 1");
	// update
	tb->setFV(FDI_USER, 11);
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - setFV stat 1");
	tb->setFV(FDI_BODY, "1\nテスト\ntest\n\nABCDEFG");
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - setFV stat 1");
	tb->update();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - update stat 1");
	// select 2
	tb->seekNext();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - stat 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 2, "doTestUpdate - id 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 1, "doTestUpdate - user_id 2");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("2\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestUpdate - body 2");
	// update
	tb->setFV(FDI_USER, 12);
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - setFV stat 2");
	tb->setFV(FDI_BODY, "2\nテスト\ntest\n\nABCDEFG");
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - setFV stat 2");
	tb->update();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - update stat 2");
	// check 1
	tb->seekPrev();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - stat 2 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 1, "doTestUpdate - id 2 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 11, "doTestUpdate - user_id 2 1");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) ==
		_tstring(_T("1\nテスト\ntest\n\nABCDEFG")), "doTestUpdate - body 2 1");
	// check 2
	tb->seekNext();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestUpdate - stat 2 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 2, "doTestUpdate - id 2 2");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 12, "doTestUpdate - user_id 2 2");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) ==
		_tstring(_T("2\nテスト\ntest\n\nABCDEFG")), "doTestUpdate - body 2 2");
}

void doTestDelete(table* tb)
{
	// delete 2
	tb->clearBuffer();
	tb->setFV(FDI_ID, 2);
	tb->seek();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestDelete - seek 2");
	tb->del();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestDelete - delete 2");
	// select 1
	tb->clearBuffer();
	tb->setFV(FDI_ID, 1);
	tb->seek();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestDelete - stat 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 1, "doTestDelete - id 1");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 11, "doTestDelete - user_id 1");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("1\nテスト\ntest\n\nABCDEFG")), "doTestDelete - body 1");
	// next is 3
	tb->seekNext();
	BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestDelete - stat 3");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_ID) == 3, "doTestDelete - id 3");
	BOOST_CHECK_MESSAGE(tb->getFVint(FDI_USER) == 2, "doTestDelete - user_id 3");
	BOOST_CHECK_MESSAGE(_tstring(tb->getFVstr(FDI_BODY)) == 
		_tstring(_T("3\ntest\nテスト\n\nあいうえおあいうえおb")), "doTestDelete - body 3");
	// eof
	tb->seekNext();
	BOOST_CHECK_MESSAGE(tb->stat() == STATUS_EOF, "doTestDelete - eof 3");
}

void createDatabase(database* db)
{
	db->create(URL);
	if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
	{
		db->open(URL, 0, 0);
		BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1");
		db->drop();
		BOOST_CHECK_MESSAGE(0 == db->stat(), "drop stat=" << db->stat());
		db->create(URL);
	}
	BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase");
}

void createTable(database* db)
{
	// create table
	dbdef* def = db->dbDef();
	tabledef td;
	memset(&td, 0, sizeof(td));
	td.setTableName(TABLE_NAME);
	_TCHAR buf[267];
	_tcscpy_s(buf, 100, TABLE_NAME);
	_tcscat_s(buf, 100, _T(".dat"));
	td.setFileName(buf);
	td.id = TABLE_ID;
	td.primaryKeyNum = -1;
	td.parentKeyNum = -1;
	td.replicaKeyNum = -1;
	td.pageSize = 2048;
	td.charsetIndex = CHARSET_UTF8B4;

	def->insertTable(&td);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "insertTable stat = " << def->stat());

	fielddef* fd = def->insertField(TABLE_ID, FDI_ID);
	fd->setName(FDN_ID);
	fd->type = ft_autoinc;
	fd->len = (ushort_td)4;
	def->updateTableDef(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 1 stat = " << def->stat());
	
	fd = def->insertField(TABLE_ID, FDI_USER);
	fd->setName(FDN_USER);
	fd->type = ft_integer;
	fd->len = (ushort_td)4;
	def->updateTableDef(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 1 stat = " << def->stat());

	fd = def->insertField(TABLE_ID, FDI_BODY);
	fd->setName(FDN_BODY);
	fd->type = ft_mytext;
	fd->len = 10;
	def->updateTableDef(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 2 stat = " << def->stat());

	fd = def->insertField(TABLE_ID, FDI_IMAGE);
	fd->setName(FDN_IMAGE);
	fd->type = ft_myblob;
	fd->len = 10;
	def->updateTableDef(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 3");

	keydef* kd = def->insertKey(TABLE_ID, 0);
	kd->segments[0].fieldNum = 0;
	kd->segments[0].flags.bit8 = 1; // extended key type
	kd->segments[0].flags.bit1 = 1; // changeable
	kd->segmentCount = 1;
	def->updateTableDef(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == def->stat(), "updateTableDef 4");
}

void testStringFileter(database* db)
{
	createDatabase(db);

	db->open(URL, 0, 0);
	BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1 stat = " << db->stat());

	createTable(db);

	table* tb = db->openTable(TABLE_ID);
	BOOST_CHECK_MESSAGE(0 == db->stat(), "openTable");
	
	doInsertStringFileter(tb);
	doTestSeek(tb);
	doTestFind(tb);
	doTestUpdate(tb);
	doTestDelete(tb);
	
	tb->release();

	db->close();

	db->open(URL, 0, 0);
	BOOST_CHECK_MESSAGE(0 == db->stat(), "drop 1");
	db->drop();
	BOOST_CHECK_MESSAGE(0 == db->stat(), "drop stat=" << db->stat());
}


// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)

	BOOST_FIXTURE_TEST_CASE(stringFileter, fixture) {testStringFileter(db());}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------

