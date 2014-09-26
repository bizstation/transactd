#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief read and update records example

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
    return (fds[fieldnum_group].i() == 1) ? filter_validate_value
                                          : filter_invalidate_value;
}

void changeGroupTo3(const fields& fds)
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

    // loop each group=1 records and update to 3
    for_each(itf, changeGroupTo3);
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    database_ptr db = createDatabaseObject();
    try
    {
        connectParams param(_T("tdap"), _T("localhost"), _T("test"),
                            _T("test"));
        openDatabase(db, param);

        table_ptr tb = openTable(db, _T("user"));
        updateUsers(tb);
        std::cout << "Insert records success." << std::endl;
        return 0;
    }

    catch (bzs::rtl::exception& e)
    {
        std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
    }
    return 1;
}
