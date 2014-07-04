#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief change schema and  convert table example

This program  change "user" table.
The "name" field is made into 64 characters from 32 characters.
And add The "tel" field.

Please execute the "create database" example before execute this example.

*/

static const short tablenum_user = 1;
static const short fieldnum_name = 1;



/** Change user table schema
 */
void changeUserTable(dbdef* def)
{
	//change name size
	tabledef* td = def->tableDefs(tablenum_user);
	fielddef* fd = &td->fieldDefs[fieldnum_name];
	fd->setLenByCharnum(64);

	//add tel field
	int size = lenByCharnum(ft_mychar, CHARSET_LATIN1, 16);
	fd = insertField(def, tablenum_user, td->fieldCount, _T("tel"), ft_mychar, size);
	fd->setCharsetIndex(CHARSET_LATIN1);
	//write user table schema
	updateTableDef(def, tablenum_user);
}

void __STDCALL onCopyData(database* db, int recordCount, int count, bool &cancel)
{
	 if (count == 0)
		 std::cout << std::endl;
	 std::cout << "." ;

}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatabaseObject();
	try
	{
		connectParams prams(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		prams.setMode(TD_OPEN_EXCLUSIVE);

		openDatabase(db, prams);

		//backup current user table schema
		db->dbDef()->pushBackup(tablenum_user);

		changeUserTable(db->dbDef());

		//convert table
		//If an error ouccered then restore the table schema automaticaly.
		convertTable(db, _T("user"), onCopyData);

		std::cout << "change schema success." << std::endl;
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
