#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief Insert records example

This program inserts some records to "user" and "group1" table.

user table
----------------------------------------
 id | name      |group| tel
----------------------------------------
 1  | "akio"    | 1   | "81-3-2222-3569"
 2  | "yoko"    | 2   | "81-263-80-5555"
 3  | "satoshi" | 1   | "81-3-1111-1234"
 4  | "keiko"   | 2   | "81-26-222-3569"
 5  | "john"    | 3   | "81-26-222-3565"
-------------------------------------

 gropu1 table
----------------------------------------
 id | name      |
----------------------------------------
 1  | "develop" |
 2  | "sales"   |
 3  | "finance" |
----------------------------------------

picture table
----------------------------------------
 type | id |   picture
----------------------------------------
 1    | 1  |  dummy picture (this program bianry image)
 ----------------------------------------


Please execute the "create database" and "change schema" example
        before execute this example.

*/

static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;

static const short fieldnum_pic_type = 0;
static const short fieldnum_pic_id = 1;
static const short fieldnum_pic_pic = 2;

/** show database operation error
*/
void showError(const _TCHAR* caption, const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024] = { 0x00 };
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("[ERROR] %s No.%ld %s\n"), caption, statusCode, tmp);
}

bool insertUser(table* tb, int id, const _TCHAR* name, int groupid,
                const _TCHAR* tel)
{
    tb->clearBuffer();
    tb->setFV(fieldnum_id, id);
    tb->setFV(fieldnum_name, name);
    tb->setFV(fieldnum_group, groupid);
    tb->setFV(fieldnum_tel, tel);
    tb->insert();
    if (tb->stat() != 0)
        showError(_T("insert user record"), tb->tableDef()->tableName(),
                  tb->stat());
    return (tb->stat() == 0);
}

bool insertUsers(table* tb)
{
    bool ret = insertUser(tb, 1, _T("akio"), 1, _T("81-3-2222-3569"));
    if (ret == false)
        return false;

    ret = insertUser(tb, 2, _T("yoko"), 2, _T("81-263-80-5555"));
    if (ret == false)
        return false;

    ret = insertUser(tb, 3, _T("satoshi"), 1, _T("81-3-1111-1234"));
    if (ret == false)
        return false;

    ret = insertUser(tb, 4, _T("keiko"), 2, _T("81-26-222-3569"));
    if (ret == false)
        return false;

    ret = insertUser(tb, 5, _T("john"), 3, _T("81-26-222-3565"));
    if (ret == false)
        return false;
    return true;
}

bool insertGroup(table* tb, int id, const _TCHAR* name)
{
    tb->clearBuffer();
    tb->setFV(fieldnum_id, id);
    tb->setFV(fieldnum_name, name);
    tb->insert();
    if (tb->stat() != 0)
        showError(_T("insert group1 record"), tb->tableDef()->tableName(),
                  tb->stat());
    return (tb->stat() == 0);
}

bool insertGroups(table* tb)
{
    bool ret = insertGroup(tb, 1, _T("develop"));
    if (ret == false)
        return false;

    ret = insertGroup(tb, 2, _T("sales"));
    if (ret == false)
        return false;

    ret = insertGroup(tb, 3, _T("finance"));
    if (ret == false)
        return false;
    return true;
}

bool insertPicure(table* tb, short type, int id, const void* img, size_t size)
{
    tb->clearBuffer();
    tb->setFV(fieldnum_pic_type, type);
    tb->setFV(fieldnum_pic_id, id);
    tb->setFV(fieldnum_pic_pic, img, (uint_td)size);
    tb->insert();

    if (tb->stat() != 0)
        showError(_T("insert picture record"), tb->tableDef()->tableName(),
                  tb->stat());
    return (tb->stat() == 0);
}

void readImage(const _TCHAR* path, std::vector<char>& s)
{
    std::ifstream ifs(path, std::ios::in | std::ios::binary);

    ifs.seekg(0, std::ios::end);
    s.resize((unsigned int)ifs.tellg());

    ifs.seekg(0, std::ios::beg);
    ifs.read(&s[0], s.size());
}

/** Open database
 */
bool openDatabase(database* db, const _TCHAR* uri)
{
    /******************************************************
     !!!  Important   !!!
     When using a multi-threaded, 
     please request a new connection for each database. 
    *******************************************************/
    // When using a multi-threaded, set to true.
    bool newConnection = false; 

    if (!db->connect(uri, newConnection))
    {
        showError(_T("connect daatabase"), NULL, db->stat());
        return false;
    }
    
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
        table* tbg = db->openTable(_T("group1"));
        if (db->stat() != 0)
            showError(_T("open group1 table"), NULL, db->stat());
        else
        {
            if (insertGroups(tbg))
            {
                table* tbu = db->openTable(_T("user"));
                if (db->stat() != 0)
                    showError(_T("open user table"), NULL, db->stat());
                else
                {
                    if (insertUsers(tbu))
                    {
                        table* tbp = db->openTable(_T("picture"));
                        if (db->stat() != 0)
                            showError(_T("open user table"), NULL, db->stat());
                        else
                        {
                            std::vector<char> s;
                            readImage(argv[0], s);
                            if (insertPicure(tbp, 1, 1, &s[0], s.size()))
                            {
                                _tprintf(_T("Insert records success. \n"));
                                result = 0;
                            }
                            tbp->release();
                        }
                    }
                    tbu->release();
                }
            }
            tbg->release();
        }
        db->close();
    }
    database::destroy(db);
    return result;
}
