#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief Insert records example

This program inserts some records to "user" and "group1" table.

user table
----------------------------------------
 id | name      |group| tel
----------------------------------------
 1  | "akio"    | 1   | "81-3-2222-3569"
 2  | "yoko"    | 2   | "81-263-80-5555"
 3  | "satoshi" | 1   | "81-3-1111-1234"
 4  | "keiko"   | 2   | "81-26-222-3569"
 5  | "john"    | 3   | "81-26-222-3565"
-------------------------------------

 gropu1 table
----------------------------------------
 id | name      |
----------------------------------------
 1  | "develop" |
 2  | "sales"   |
 3  | "finance" |
----------------------------------------

picture table
----------------------------------------
 type | id |   picture
----------------------------------------
 1    | 1  |  dummy picture (this program bianry image)
 ----------------------------------------

Please execute the "create database" and "change schema" example
	before execute this example.

*/



static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;

static const short fieldnum_pic_type = 0;
static const short fieldnum_pic_id = 1;
static const short fieldnum_pic_pic = 2;


void insertUser(fields& fds, int id, const _TCHAR* name, int groupid
													, const _TCHAR* tel)
{

	fds.clear();
	fds[fieldnum_id] = id;
	fds[fieldnum_name] = name;
	fds[fieldnum_group] = groupid;
	fds[fieldnum_tel] = tel;

	insertRecord(fds);
}

void insertUsers(table_ptr tb)
{
	fields fds(tb);
	insertUser(fds, 1, _T("akio")   , 1, _T("81-3-2222-3569"));
	insertUser(fds, 2, _T("yoko")   , 2, _T("81-263-80-5555"));
	insertUser(fds, 3, _T("satoshi"), 1, _T("81-3-1111-1234"));
	insertUser(fds, 4, _T("keiko")  , 2, _T("81-26-222-3569"));
	insertUser(fds, 5, _T("john")  ,  3, _T("81-26-222-3565"));

}


void insertGroup(fields& fds, int id, const _TCHAR* name)
{

	fds.clear();
	fds[fieldnum_id] = id;
	fds[fieldnum_name] = name;
	insertRecord(fds);
}

void insertGroups(table_ptr tb)
{
	fields fds(tb);
	insertGroup(fds, 1, _T("develop"));
	insertGroup(fds, 2, _T("sales"));
	insertGroup(fds, 3, _T("finance"));
}


void insertPicure(table_ptr tb, short type, int id, const void* img, size_t size)
{
	fields fds(tb);
	fds.clear();
	fds[fieldnum_pic_type] = type;
	fds[fieldnum_pic_id] = id;
	fds[fieldnum_pic_pic].setBin(img, (int)size);
	insertRecord(fds);

}
void readImage(const _TCHAR* path, std::vector<char>& s)
{
	std::ifstream ifs(path, std::ios::in | std::ios::binary );

	ifs.seekg(0, std::ios::end);
	s.resize(ifs.tellg());

	ifs.seekg(0, std::ios::beg);
	ifs.read(&s[0], s.size());
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatabaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		openDatabase(db, param);

		table_ptr tb = openTable(db, _T("group1"));
		insertGroups(tb);

		tb = openTable(db, _T("user"));
		insertUsers(tb);

		tb = openTable(db, _T("picture"));

		std::vector<char> s;
		readImage(argv[0], s);
		insertPicure(tb, 1, 1, &s[0], s.size());

		std::cout << "Insert records success." << std::endl;
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
