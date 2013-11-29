#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief update records in transaction example

This program updates some records of a "user" table.
Change group of user in group 1 to 3

Please execute "create database" , "change schema" and "insert records" example
    before execute this example.

*/


static const short fieldnum_id = 0;
static const short fieldnum_group = 2;
static const char_td keynum_group = 1;

int isGroupOne(const fields& fds)
{
    return  (fds[fieldnum_group].i() == 1)? filter_validate_value
                                          : filter_invalidate_value;
}

void changeGroup1To3(const fields& fds)
{
    fds[fieldnum_group] = 3;
    updateRecord(fds);
}

void updateUsers(table_ptr tb)
{
    // Execute seekGreaterOrEqual with key number and key value
    indexIterator it = readIndex_v(tb, eSeekGreaterOrEqual, keynum_group, 1);

    // Wrap filtered iterator
    filterdIndexIterator itf(it, isGroupOne);

    //loop each group=1 records and update
    for_each(itf, changeGroup1To3);


}


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    database_ptr db = createDatadaseObject();
    try
    {
        connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
        openDatabase(db, param);

        table_ptr tb = openTable(db, _T("user"));
        transaction trn(db);

        //start transaction
        trn.begin();

        updateUsers(tb);

        //Commit transaction. If an error throwed then abort transaction automaticaly.
        trn.end();

        std::cout << "Update records success." << std::endl;
        return 0;
    }

    catch(bzs::rtl::exception& e)
    {
        std::tcout << *bzs::rtl::getMsg(e) << std::endl;
    }
    return 1;
}
