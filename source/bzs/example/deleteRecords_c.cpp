#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief read and delete example

This program deletes one record of a "user" table.


Please execute "create database" , "change schema" and "insert records" example
	before execute this example.

*/



static const short fieldnum_id = 0;
static const char_td keynum_id = 0;

void deleteUser(table_ptr tb)
{
	//Seek record that user id = 3 "satoshi"
	indexIterator it = readIndex_v(tb, eSeekEqual, keynum_id, 3);

	if (!it.isEnd())
		deleteRecord(it);  //delete id = 3
	else
	   THROW_BZS_ERROR_WITH_MSG(_T("User id = 3 was not found"));

}


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatabaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		openDatabase(db, param);

		table_ptr tb = openTable(db, _T("user"));
		deleteUser(tb);
		std::cout << "Delete records success." << std::endl;
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
