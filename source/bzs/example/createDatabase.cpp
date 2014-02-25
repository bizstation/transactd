#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


/**
@brief create database example

This program  create "test" database and
"test" table. The table of "test" has schema data.
And it call "schema table".

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


/** show database operation error
*/
void showError(const _TCHAR* caption,const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024]={0x00};
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("%s error No.%ld %s\n"),caption, statusCode, tmp);
}

/** Create mysql database and empty schema table
 */
bool createBatabase(database* db, const _TCHAR* uri)
{
    db->create(uri);
    if (db->stat() != 0)
    {
        showError(_T("createDatabse"), NULL, db->stat());
        return false;
    }
    return true;
}

/** Open database and empty schema table
 */
bool openDbExclusive(database* db, const _TCHAR* uri)
{
    db->open(uri, TYPE_SCHEMA_BDF, TD_OPEN_EXCLUSIVE);
    if (db->stat() != 0)
    {
        showError(_T("openBatabase"), NULL, db->stat());
        return false;
    }
    return true;
}

/** Create user table schema and write
 */
bool createUserTableSchema(dbdef* def)
{

    //Insert table
    tabledef td;
    td.setTableName(_T("user"));
    td.setFileName(_T("user.dat"));
    td.id =1;
    td.charsetIndex = CHARSET_UTF8B4;
    def->insertTable(&td);
    if (def->stat() != 0)
    {
        showError(_T("create schema table user"), NULL, def->stat());
        return false;
    }

    //Insert field
    short fieldNum = 0;
    fielddef* fd =  def->insertField(td.id, fieldNum);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    fd =  def->insertField(td.id, ++fieldNum);
    fd->setName(_T("name"));
    fd->type = ft_myvarchar;
    fd->setLenByCharnum(32);

    fd =  def->insertField(td.id, ++fieldNum);
    fd->setName(_T("group"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    //Insert index
    uchar_td keyNum = 0;
    keydef* kd = def->insertKey(td.id, keyNum);
    kd->segments[0].fieldNum = 0;
    kd->segments[0].flags.bit8 = true;//extended type
    kd->segments[0].flags.bit1 = true;//updateable
    kd->segmentCount = 1;
    def->tableDefs(td.id)->primaryKeyNum = keyNum;

    keyNum = 1;
    kd = def->insertKey(td.id, keyNum);
    kd->segments[0].fieldNum = 2;//groupid
    kd->segments[0].flags.bit8 = true;//extended type
    kd->segments[0].flags.bit1 = true;//updateable
    kd->segments[0].flags.bit0 = true;//duplicateable
    kd->segmentCount = 1;

    //write schema table
    def->updateTableDef(td.id);
    if (def->stat() != 0)
    {
        showError(_T("edit schema table"), NULL, def->stat());
        return false;
    }
    return true;

}

/** Create group table schema and write
 */
bool createGroupTableSchema(dbdef* def)
{

    //Insert table
    tabledef td;
    td.setTableName(_T("group1"));
    td.setFileName(_T("group1.dat"));
    td.id = 2;
    td.charsetIndex = CHARSET_UTF8B4;
    def->insertTable(&td);
    if (def->stat() != 0)
    {
        showError(_T("create schema table group1"), NULL, def->stat());
        return false;
    }

    //Insert field
    short fieldNum = 0;
    fielddef* fd =  def->insertField(td.id, fieldNum);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;

    fd =  def->insertField(td.id, ++fieldNum);
    fd->setName(_T("name"));
    fd->type = ft_myvarchar;
    fd->setLenByCharnum(32);

    //Insert index
    uchar_td keyNum = 0;
    keydef* kd = def->insertKey(td.id, keyNum);
    kd->segments[0].fieldNum = 0;
    kd->segments[0].flags.bit8 = true;//extended type
    kd->segments[0].flags.bit1 = true;//updateable
    kd->segmentCount = 1;
    def->tableDefs(td.id)->primaryKeyNum = keyNum;

    //write schema table
    def->updateTableDef(td.id);
    if (def->stat() != 0)
    {
        showError(_T("edit schema table"), NULL, def->stat());
        return false;
    }
    return true;

}

/** Create picture table schema and write
 */
bool createPictureTableSchema(dbdef* def)
{
    //Insert table
    tabledef td;
    td.setTableName(_T("picture"));
    td.setFileName(_T("picture.dat"));
    td.id = 3;
    td.charsetIndex = CHARSET_LATIN1;
    def->insertTable(&td);
    if (def->stat() != 0)
    {
        showError(_T("create schema table picture"), NULL, def->stat());
        return false;
    }

    //Insert 3 fields
    short fieldNum = 0;
    fielddef* fd =  def->insertField(td.id, fieldNum);
    fd->setName(_T("type"));
    fd->type = ft_integer;
    fd->len = (ushort_td)2;

    fd =  def->insertField(td.id, ++fieldNum);
    fd->setName(_T("id"));
    fd->type = ft_integer;
    fd->len = (ushort_td)4;


    fd =  def->insertField(td.id, ++fieldNum);
    fd->setName(_T("picture"));
    fd->type = ft_myblob;
    fd->len = (ushort_td)11;


    //Insert index
    uchar_td keyNum = 0;
    keydef* kd = def->insertKey(td.id, keyNum);
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
    def->tableDefs(td.id)->primaryKeyNum = keyNum;

     //write schema table
    def->updateTableDef(td.id);
    if (def->stat() != 0)
    {
        showError(_T("edit schema table"), NULL, def->stat());
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
    if (createBatabase(db, uri))
    {
        if (openDbExclusive(db, uri))
        {
            if (createUserTableSchema(db->dbDef()))
            {
                if (createGroupTableSchema(db->dbDef()))
                {
                    if (createPictureTableSchema(db->dbDef()))
                    {
                        _tprintf(_T("create databse success. \n"));
                        result = 0;
                    }
                }
            }
            db->close();
        }
    }
    database::destroy(db);
    return result;
}
