#include <stdio.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <vector>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief read records and manual O/R mapping example

This program read records of the "user" table where group = 3.
And O/R mapping to the user class.
This program use the filter operation on server side.

Please execute "create database" , "change schema" and "insert records" example
        before execute this example.

*/

static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;
static const char_td keynum_group = 1;

/** User class */
class user
{
public:
    int id;
    std::_tstring name;
    int group;
    std::_tstring tel;
};

/** dump user to screen */
void dumpUser(const user* user)
{
    _tprintf(_T(" id           %d\n"), user->id);
    _tprintf(_T(" name         %s\n"), user->name.c_str());
    _tprintf(_T(" group        %d\n"), user->group);
    _tprintf(_T(" tel          %s\n\n"), user->tel.c_str());
}

/** O/R mapping functional object*/
class userMappper
{
    std::vector<user*>& m_users;

public:
    userMappper(std::vector<user*>& users) : m_users(users) {}
    void operator()(table* tb)
    {
        user* u = new user();
        u->id = tb->getFVint(fieldnum_id);
        u->name = tb->getFVstr(fieldnum_name);
        u->group = tb->getFVint(fieldnum_group);
        u->tel = tb->getFVstr(fieldnum_tel);
        m_users.push_back(u);
    }
};

/** show database operation error
*/
void showError(const _TCHAR* caption, const _TCHAR* tableName, short statusCode)
{
    _TCHAR tmp[1024] = { 0x00 };
    nstable::tdapErr(0x00, statusCode, tableName, tmp);
    _tprintf(_T("%s error No.%ld %s\n"), caption, statusCode, tmp);
}

bool readUsers(table* tb, std::vector<user*>& users)
{
    userMappper ormap(users);
    tb->clearBuffer();
    tb->setKeyNum(keynum_group); // use group key
    tb->setFV(fieldnum_group, 3); // set group = 3;
    tb->seekGreater(true /*orEqual*/);
    if (tb->stat() == 0)
        tb->setFilter(_T("group = 3"), 1 /*rejectCount*/, 0 /*max records*/);
    while (1)
    {
        if (tb->stat() != 0)
        {
            if (tb->stat() == STATUS_EOF)
                return true;
            showError(_T("read user"), tb->tableDef()->tableName(), tb->stat());
            return false;
        }
        ormap(tb);
        tb->findNext();
    }
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
            std::vector<user*> users;
            if (readUsers(tbu, users))
            {
                _tprintf(_T("Read records success.\nRecord count = %d\n"),
                         users.size());
                std::for_each(users.begin(), users.end(), dumpUser);
            }

            for (size_t i = 0; i < users.size(); ++i)
                delete users[i];

            tbu->release();
        }
        db->close();
    }
    database::destroy(db);
    return result;
}
