#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbdef.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief drop database example

This program  drop "test" database.
*/



/** show database operation error
*/
void showError(const _TCHAR* caption,const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024]={0x00};
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("%s error No.%ld %s\n"),caption, statusCode, tmp);
}


/** Open database
 */
bool openDbExclusive(database* db, const _TCHAR* uri)
{
    db->open(uri, TYPE_SCHEMA_BDF, TD_OPEN_EXCLUSIVE);
    if (db->stat() != 0)
    {
        showError(_T("open daatabase"), NULL, db->stat());
        return false;
    }
    return true;
}


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    int result = 1;
    static const _TCHAR* uri = _T("tdap://localhost/test?dbfile=test.bdf");
    database* db = database::create();

    if (openDbExclusive(db, uri))
    {
        db->drop();
        if (db->stat() != 0)
            showError(_T("drop daatabase"), NULL, db->stat());
        result = db->stat();
        db->close();
    }
    database::destroy(db);
    return result;
}
