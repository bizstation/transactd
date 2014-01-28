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

void doTestFilterBug(table* tb)
{
    tb->setKeyNum(0);
    tb->clearBuffer();

	tb->setFilter(_T("id >= 1 and id < 3"), 1, 0);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFilterBug - setFilter");

    tb->setFV(FDI_ID, 1);
	tb->find(table::findForword);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFilterBug - find");
    BOOST_CHECK_MESSAGE(1 == tb->getFVint(FDI_ID), "doTestFilterBug - getFVint 1");

	tb->findNext(true);
    BOOST_CHECK_MESSAGE(0 == tb->stat(), "doTestFilterBug - findNext");
	std::cout << tb->getFVint(FDI_ID) << std::endl;
    BOOST_CHECK_MESSAGE(2 == tb->getFVint(FDI_ID), "doTestFilterBug - getFVint 2");
	std::cout << tb->getFVstr(FDI_BODY) << std::endl;
	BOOST_CHECK_MESSAGE(
		_tstring(_T("2\ntest\nテスト\n\nあいうえおあいうえおb")) == _tstring(tb->getFVstr(FDI_BODY)),
		"doTestFilterBug - getFVstr 2");
    tb->findNext(true);
    BOOST_CHECK_MESSAGE(9 == tb->stat(), "doTestFilterBug - findNext");
}

void createDatabase(database* db)
{
    db->create(URL);
	if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
	{
		db->open(URL, 0, 0);
		BOOST_CHECK_MESSAGE(0 == db->stat(), "createNewDataBase 1");
		db->drop();
		BOOST_CHECK_MESSAGE(0 == db->stat(), "DropDataBaseTestString stat=" << db->stat());
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
	doTestFilterBug(tb);
	
	tb->release();

    db->close();
}


// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)

    BOOST_FIXTURE_TEST_CASE(stringFileter, fixture) {testStringFileter(db());}

BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------

