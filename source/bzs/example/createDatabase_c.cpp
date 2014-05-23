#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <bzs/env/tstring.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief create database example

This program  create "test" database and
"test" table. The table of "test" has schema data.
And it is called "schema table".


database name :  'test'

table 'user'  tableid = 1
   -------------------------------------------------
   No.| Filed name  | type    |  size
   -------------------------------------------------
   0  |  id         | integer | 4byte
   1  |  name       | var     | 97byte (32 charctors)
   2  |  groupid    | integer | 4byte
   -------------------------------------------------
   -------------------------------------------------
   index No.|  type  | segment0(size) | segment1(size)
   -------------------------------------------------
   0        | primary| id(4)          |  -
   1        |        | groupid(4)     |  -



table 'group1' tableid = 2
   -------------------------------------------------
   No.| Filed name  | type    |  size
   -------------------------------------------------
   0  |  id         | integer | 4byte
   1  |  name       | var     | 97byte (32 charctors)
   -------------------------------------------------
   -------------------------------------------------
   index No.|  type  | segment0(size) | segment1(size)
   -------------------------------------------------
   0        | primary| id(4)          |  -


table 'picture' tableid = 3
   -------------------------------------------------
   No.| Filed name  | type    |  size
   -------------------------------------------------
   0  |  type       | integer | 2byte
   0  |  id         | integer | 4byte
   1  |  picture    | myblob  | 11 MIDIUMBLOB
   -------------------------------------------------
   -------------------------------------------------
   index No.|  type  | segment0(size) | segment1(size)
   -------------------------------------------------
   0        | primary| type(2)        |  id(4)


*/


/** Create user table schema and write
 */
void createUserTableSchema(dbdef* def)
{
	//Insert table
	short tableid = 1;
	const _TCHAR* name = _T("user");
	insertTable(def, tableid, name, CHARSET_UTF8B4);

	//Insert 3 fields
	short fieldNum = 0;
	insertField(def, tableid, fieldNum, _T("id"), ft_integer, 4);

	int size = lenByCharnum(ft_myvarchar, CHARSET_UTF8B4, 32);
	insertField(def, tableid, ++fieldNum, _T("name"), ft_myvarchar, size);
	insertField(def, tableid, ++fieldNum, _T("group"), ft_integer, 4);

	//Insert index
	uchar_td keyNum = 0;
	keydef* kd = insertKey(def, tableid, keyNum);
	keySegment* ks = &kd->segments[0];
	ks->fieldNum = 0;//id
	ks->flags.bit8 = true;//extended type
	ks->flags.bit1 = true;//updateable
	kd->segmentCount = 1;
	def->tableDefs(tableid)->primaryKeyNum = keyNum;

	keyNum = 1;
	kd = insertKey(def, tableid, keyNum);
	ks = &kd->segments[0];
	ks->fieldNum = 2;//groupid
	ks->flags.bit8 = true;//extended type
	ks->flags.bit1 = true;//updateable
	ks->flags.bit0 = true;//duplicateable
	kd->segmentCount = 1;


	//write schema table
	updateTableDef(def, tableid);

}

/** Create group table schema and write
 */
void createGroupTableSchema(dbdef* def)
{
	//Insert table
	short tableid = 2;
	const _TCHAR* name = _T("group1");
	insertTable(def, tableid, name, CHARSET_UTF8B4);

	//Insert 2 fields
	short fieldNum = 0;
	insertField(def, tableid, fieldNum, _T("id"), ft_integer, 4);
	int size = lenByCharnum(ft_myvarchar, CHARSET_UTF8B4, 32);
	insertField(def, tableid, ++fieldNum, _T("name"), ft_myvarchar, size);

	//Insert index
	uchar_td keyNum = 0;
	keydef* kd = insertKey(def, tableid, keyNum);
	keySegment* ks = &kd->segments[0];
	ks->fieldNum = 0; //id
	ks->flags.bit8 = true;//extended type
	ks->flags.bit1 = true;//updateable
	kd->segmentCount = 1;
	def->tableDefs(tableid)->primaryKeyNum = keyNum;
	//write schema table
	updateTableDef(def, tableid);

}

/** Create picture table schema and write
 */
void createPictureTableSchema(dbdef* def)
{
	//Insert table
	short tableid = 3;
	const _TCHAR* name = _T("picture");
	insertTable(def, tableid, name, CHARSET_LATIN1);

	//Insert 3 fields
	short fieldNum = 0;

	insertField(def, tableid, fieldNum, _T("type"), ft_integer, 2);
	insertField(def, tableid, ++fieldNum, _T("id"), ft_integer, 4);
	insertField(def, tableid, ++fieldNum, _T("picture"), ft_myblob, 11);

	//Insert index
	uchar_td keyNum = 0;
	keydef* kd = insertKey(def, tableid, keyNum);
	keySegment* ks = &kd->segments[0];
	ks->fieldNum = 0; //type
	ks->flags.bit8 = true;//extended type
	ks->flags.bit1 = true;//updateable
	ks->flags.bit4  = true;//segment part

	ks = &kd->segments[1];
	ks->fieldNum = 1; //id
	ks->flags.bit8 = true;//extended type
	ks->flags.bit1 = true;//updateable

	kd->segmentCount = 2;
	def->tableDefs(tableid)->primaryKeyNum = keyNum;

	//write schema table
	updateTableDef(def, tableid);

}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatadaseObject();
	try
	{
		connectParams prams(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		prams.setMode(TD_OPEN_EXCLUSIVE);

		createDatabase(db, prams);

		openDatabase(db, prams);

		createUserTableSchema(db->dbDef());

		createGroupTableSchema(db->dbDef());

		createPictureTableSchema(db->dbDef());

		std::cout << "create databse success." << std::endl;
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
