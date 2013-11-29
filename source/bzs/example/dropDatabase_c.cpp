#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief drop database example

This program  drop "test" database.
*/


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    database_ptr db = createDatadaseObject();
    try
    {
        connectParams prams(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
        prams.setMode(TD_OPEN_EXCLUSIVE);

        openDatabase(db, prams);
        dropDatabase(db);

        std::cout << "Drop databse success." << std::endl;
        return 0;
    }
    catch(bzs::rtl::exception& e)
    {
        std::tcout << *bzs::rtl::getMsg(e) << std::endl;
    }
    return 1;
}
