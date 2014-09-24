#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief read and delete example

This program deletes one record of a "user" table.

Please execute "create database" , "change schema" and "insert records" example
        before execute this example.

*/

static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;
static const char_td keynum_id = 0;

/** show database operation error
*/
void showError(const _TCHAR* caption, const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024] = { 0x00 };
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("[ERROR] %s No.%ld %s\n"), caption, statusCode, tmp);
}

bool deleteUser(table* tb)
{
    tb->clearBuffer();
    tb->setKeyNum(keynum_id); // use id key
    tb->setFV(fieldnum_id, 3); // id=3 satoshi
    tb->seek();
    if (tb->stat() == 0)
        tb->del();
    if (tb->stat() != 0)
        showError(_T("update user"), tb->tableDef()->tableName(), tb->stat());
    return (tb->stat() == 0);
}

/** Open database
 */
bool openDatabase(database* db, const _TCHAR* uri)
{
    db->open(uri, TYPE_SCHEMA_BDF);
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
    int result = 0;
    static const _TCHAR* uri = _T("tdap://localhost/test?dbfile=test.bdf");
    database* db = database::create();

    if (openDatabase(db, uri))
    {
        table* tbu = db->openTable(_T("user"));
        if (db->stat() != 0)
            showError(_T("open user table"), NULL, db->stat());
        else
        {
            if (deleteUser(tbu))
                _tprintf(_T("Delete records success. \n"));
            tbu->release();
        }
        db->close();
    }
    database::destroy(db);
    return result;
}
