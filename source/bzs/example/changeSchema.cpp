#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <boost/bind.hpp>
using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/** @brief change schema and  convert table example

This program  change "user" table.
The "name" field is made into 64 characters from 32 characters.
And add The "tel" field.

Please execute the "create database" example before execute this example.

*/

static const short tablenum_user = 1;
static const short fieldnum_name = 1;


/** show database operation error
*/
void showError(const _TCHAR* caption,const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024]={0x00};
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("%s error No.%ld %s\n"),caption, statusCode, tmp);
}

/** Change user table schema
 */
bool changeUserTable(dbdef* def)
{

    //change name size
    tabledef** td = def->tableDefPtr(tablenum_user);
    fielddef* fd = &(*td)->fieldDefs[fieldnum_name];
    fd->setLenByCharnum(64);

    //add tel field
    fd = def->insertField((*td)->id, (*td)->fieldCount);
    fd->setName(_T("tel"));
    fd->type = ft_mychar;
    fd->setCharsetIndex( CHARSET_LATIN1);
    fd->setLenByCharnum(16);

    //write user table schema
    def->updateTableDef((*td)->id);
    if (def->stat() != 0)
    {
        showError(_T("edit schema table"), NULL, def->stat());
        return false;
    }
    return true;
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

void __STDCALL onCopyData(database* db, int recordCount, int count, bool &cancel)
{
     if (count == 0)
         _tprintf(_T("\n"));
     _tprintf(_T("."));

}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    int result = 0;
    static const _TCHAR* uri = _T("tdap://localhost/test?dbfile=test.bdf");
    database* db = database::create();

    if (openDbExclusive(db, uri))
    {
        //backup current user table schema
        db->dbDef()->pushBackup(tablenum_user);

        if (changeUserTable(db->dbDef()))
        {
            //convert table if table exist;
            if (db->existsTableFile(tablenum_user, NULL))
            {
                db->setOnCopyData(onCopyData);

                db->convertTable(tablenum_user, false, NULL);
                if (db->stat())
                    showError(_T("convert table"), NULL, db->stat());
            }
        }

        if (db->stat())
        {
            result = db->stat();
            //restore user table schema
            db->dbDef()->popBackup(tablenum_user);
        }else
            _tprintf(_T("\nchage table success. \n"));
        db->close();
    }
    database::destroy(db);
    return result;
}
