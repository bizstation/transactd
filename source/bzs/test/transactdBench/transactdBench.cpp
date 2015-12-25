/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
=================================================================*/

#pragma hdrstop

#include <tchar.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <time.h>
#include <bzs/rtl/benchmark.h>
#include <boost/bind.hpp>
#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS

static const int TYPE_BDF = 0;
static const bool AUTO_CREATE_TABLE = true;
static const short fn_id = 0;
static const short fn_name = 1;
static const int trans_bias = PARALLEL_TRN + LOCK_SINGLE_NOWAIT;

static const int USE_NORMAL = 0;
static const int USE_TRANS = 1;
static const int USE_BALKINS = 2;
static const int USE_SNAPSHOT = 4;

using namespace bzs::rtl;
using namespace bzs::db::protocol::tdap;

/* --------------------------------------------------------------------------------
 */
void showTableError(client::table* tb, const _TCHAR* description)
{
    if (tb->stat() != 0)
        _tprintf(_T("%s error %s:No.%d\r\n"), description,
                 tb->tableDef()->fileName(), tb->stat());
}

/* --------------------------------------------------------------------------------
 */
void showEnginError(client::database* db, const _TCHAR* tableName)
{
    if (db->stat() != 0)
        _tprintf(_T("%s error No.%d\r\n"), tableName, db->stat());
}

/* --------------------------------------------------------------------------------
 */
client::table* openTable(client::database* db, const _TCHAR* tableName,
                         short mode)
{
    client::table* tb = db->openTable(tableName, mode, AUTO_CREATE_TABLE);
    if (tb == NULL)
        showEnginError(db, tableName);
    return tb;
}

/* --------------------------------------------------------------------------------
 */
bool createDataBase(client::database* db, const _TCHAR* uri)
{
    db->create(uri);
    return (db->stat() == 0);
}

/* --------------------------------------------------------------------------------
 */
bool write(client::table* tb, int start, int end)
{
    tb->setKeyNum(0);
    for (int i = start; i < end; i++)
    {
        tb->clearBuffer();
        tb->setFV(fn_id, i);
        tb->setFV(fn_name, i);
        tb->insert();
        if (tb->stat() != 0)
        {
            showTableError(tb, _T("write"));
            return false;
        }
    }
    return true;
}

/* --------------------------------------------------------------------------------
 */
bool deleteAll(client::database* db, client::table* tb, int start, int end)
{
    db->beginTrn(trans_bias);
    tb->clearBuffer();
    for (int i = start; i < end; i++)
    {
        tb->setFV(fn_id, i);
        tb->seek();
        if (tb->stat() == 0)
        {
            tb->del();
            if (tb->stat() != 0)
            {
                showTableError(tb, _T("deleteAll"));
                db->endTrn();
                return false;
            }
        }
    }
    db->endTrn();
    return true;
}

/* --------------------------------------------------------------------------------
 */
bool Inserts(client::database* db, client::table* tb, int start, int end,
             int mode, int unit)
{
    bool ret = true;
    int st = start;
    int en = st;
    while (en != end)
    {
        en = st + unit;
        if (mode == USE_TRANS)
            db->beginTrn(trans_bias);
        if (mode == USE_BALKINS)
            tb->beginBulkInsert(BULKBUFSIZE);
        ret = write(tb, st, en);
        if (mode == USE_BALKINS)
            tb->commitBulkInsert();
        if (mode == USE_TRANS)
            db->endTrn();
        if (ret == false)
            break;
        st = en;
    }
    return ret;
}

/* --------------------------------------------------------------------------------
 */
bool Read(client::database* db, client::table* tb, int start, int end,
          int shapshot)
{
    bool ret = true;
    tb->clearBuffer();
    if (shapshot == USE_SNAPSHOT)
        db->beginSnapshot();
    for (int i = start; i < end; i++)
    {
        tb->setFV(fn_id, i);
        tb->seek();
        if ((tb->stat() != 0) || (tb->getFVlng(fn_id) != i))
        {
            printf("GetEqual Error stat() = %d  Value %d = %d\r\n", tb->stat(),
                   i, tb->getFVlng(fn_id));
            ret = false;
            break;
        }
    }
    if (shapshot == USE_SNAPSHOT)
        db->endSnapshot();

    return ret;
}

/* --------------------------------------------------------------------------------
 */
bool Reads(client::database* db, client::table* tb, int start, int end,
           int unit, int shapshot)
{
    bool ret = true;
    int st = start;
    int en = st;
    if (shapshot == USE_SNAPSHOT)
        db->beginSnapshot();
    tb->setKeyNum(0);
    tb->setFilter(_T("*"), 1, 20);
    tb->clearBuffer();
    tb->setFV(fn_id, st);
    tb->find(client::table::findForword);
    while (en != end)
    {
        en = st + unit;
        for (int i = st; i < en; i++)
        {
            if (tb->getFVlng(fn_id) != i)
            {
                printf("findNext Error stat() = %d  Value %d = %d\r\n",
                       tb->stat(), i, tb->getFVlng(fn_id));
                ret = false;
                break;
            }
            tb->findNext();
        }
        if (ret == false)
            break;
        st = en;
    }
    if (shapshot == USE_SNAPSHOT)
        db->endSnapshot();
    return ret;
}
/* --------------------------------------------------------------------------------
 */
bool Updates(client::database* db, client::table* tb, int start, int end,
             int tran, int unit)
{
    bool ret = true;
    _TCHAR buf[30];
    tb->setKeyNum(0);

    int st = start;
    int en = st;
    while (en != end)
    {
        en = st + unit;

        if (tran == USE_TRANS)
            db->beginTrn(trans_bias);
        for (int i = st; i < en; i++)
        {
            tb->setFV(fn_id, i);
            _ltot_s(i + 1 + tran, buf, 30, 10);
            tb->setFV(fn_name, buf);
            tb->update(client::nstable::changeInKey);
            if (tb->stat() != 0)
            {
                ret = false;
                break;
            }
        }
        if (tran == USE_TRANS)
            db->endTrn();
        if (ret == false)
            break;
        st = en;
    }
    return ret;
}

/* --------------------------------------------------------------------------------
 */
bool createTestDataBase(client::database* db, const _TCHAR* uri)
{
    db->create(uri);
    if (db->stat() != 0)
    {
        _tprintf(_T("createTestDataBase erorr:No.%d %s\r\n"), db->stat(), uri);
        return false;
    }
    if (db->open(uri, TYPE_BDF, TD_OPEN_NORMAL, _T(""), _T("")))
    {

        client::dbdef* def = db->dbDef();

        tabledef td;
        td.setTableName(_T("user"));
        td.setFileName(_T("user.dat"));
        td.id = 1;
        def->insertTable(&td);

        fielddef* fd = def->insertField(td.id, 0);
        fd->setName(_T("id"));
        fd->type = ft_integer;
        fd->len = (ushort_td)4;

        fd = def->insertField(td.id, 1);
        fd->setName(_T("name"));
        if (db->isUseTransactd())
            fd->type = ft_myvarchar;
        else
            fd->type = ft_zstring;

        fd->setLenByCharnum(49);
        keydef* kd = def->insertKey(td.id, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extend key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;

        td.primaryKeyNum = 0;
        def->updateTableDef(td.id);
        if (def->stat())
            printf("crate daatabse erorr No:%d\r\n", def->stat());
        return def->stat() == 0;
    }
    else
        printf("open daatabse erorr No:%d\r\n", db->stat());
    return false;
}

/* --------------------------------------------------------------------------------
 */
void printDateTime()
{
    time_t timer;
#ifdef LINUX
    time(&timer);
#else
    timer = time(NULL);
#endif
#pragma warning(disable : 4996)
    printf("%s", ctime(&timer));
#pragma warning(default : 4996)
}

/* --------------------------------------------------------------------------------
 */
void printHeader(const _TCHAR* uri, int count)
{
    printf("Start Bench mark Insert Items = %d\r\n", count);
    printDateTime();
    _tprintf(_T("%s\r\n"), uri);
    printf("BOOST_VERSION = %s\r\n", BOOST_LIB_VERSION);
    printf("----------------------------------------\r\n");
}

/* --------------------------------------------------------------------------------
 */
void printTail()
{
    printf("----------------------------------------\r\n");
}

/* --------------------------------------------------------------------------------
 */
#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 4)
    {
        printf("usage: bench_tdclcpp_bcb32(64) databaseUri processNumber "
               "functionNumber\n "
               "\t --- Below is list of functionNumber  ---\n"
               "\t-1: all function\n"
               "\t 0: Insert\n"
               "\t 1: Insert in transaction. 20rec x 1000times\n"
               "\t 2: Insert by bulkmode. 20rec x 1000times\n"
               "\t 3: read each record\n"
               "\t 4: read each record with snapshpot\n"
               "\t 5: read range. 20rec x 1000times\n"
               "\t 6: read range with snapshpot. 20rec x 1000times\n"
               "\t 7: update\n"
               "\t 8: update in transaction. 20rec x 1000times\n"
               "exsample : bench_tdclcpp_bcb32(64) "
               "\"tdap://localhost/test?dbfile=test.bdf\" 0 -1\n");
        return 0;
    }
    const _TCHAR* uri = argv[1]; // "tdap://localhost/test?dbfile=test.bdf";
    int procID = _ttol(argv[2]); // 0
    int count = 20000;
    int start = procID * count + 1;
    int end = start + count;
    int exeType = _ttol(argv[3]); // -1
    bool insertBeforeNoDelete = 0;
    if (argc > 4)
        insertBeforeNoDelete = (_ttol(argv[4]) != 0);

    client::database* db = client::database::create();
    if (db->open(uri, TYPE_BDF, TD_OPEN_NORMAL, _T(""), _T("")) == false)
    {
        if (!createTestDataBase(db, uri))
        {
            client::database::destroy(db);
            return 1;
        }
        printf("CreateDataBase success.\r\n");
    }
    printHeader(uri, count);

    if (!db->open(uri, TYPE_BDF, TD_OPEN_NORMAL, _T(""), _T("")))
        printf("open database erorr No:%d\r\n", db->stat());
    else
    {
        client::table* tb = openTable(db, _T("user"), TD_OPEN_NORMAL);
        if (tb)
        {
            if ((exeType == -1) || (exeType == 0))
            {
                if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
                    benchmark::report(
                        boost::bind(Inserts, db, tb, start, end, USE_NORMAL, 1),
                        ": Insert");
                else
                    printf("deleteAll erorr No:%d\r\n", tb->stat());
            }
            if ((exeType == -1) || (exeType == 1))
            {
                if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
                    benchmark::report(
                        boost::bind(Inserts, db, tb, start, end, USE_TRANS, 20),
                        ": Insert in transaction. 20rec x 1000times.");
                else
                    printf("deleteAll erorr No:%d\r\n", tb->stat());
            }
            if ((exeType == -1) || (exeType == 2))
            {
                if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
                    benchmark::report(
                        boost::bind(Inserts, db, tb, start, end, USE_BALKINS, 20),
                        ": Insert by bulkmode. 20rec x 1000times.");
                else
                    printf("deleteAll erorr No:%d\r\n", tb->stat());
            }
            if ((exeType == -1) || (exeType == 3))
                benchmark::report(boost::bind(Read, db, tb, start, end, USE_NORMAL),
                                  ": read each record.");
            if ((exeType == -1) || (exeType == 4))
                benchmark::report(
                    boost::bind(Read, db, tb, start, end, USE_SNAPSHOT),
                    ": read each record with snapshpot.");
            if ((exeType == -1) || (exeType == 5))
                benchmark::report(
                    boost::bind(Reads, db, tb, start, end, 20, USE_NORMAL),
                    ": read range. 20rec x 1000times.");
            if ((exeType == -1) || (exeType == 6))
                benchmark::report(
                    boost::bind(Reads, db, tb, start, end, 20, USE_SNAPSHOT),
                    ": read range with snapshpot. 20rec x 1000times.");
            if ((exeType == -1) || (exeType == 7))
                benchmark::report(
                    boost::bind(Updates, db, tb, start, end, USE_NORMAL, 1),
                    ": update.");
            if ((exeType == -1) || (exeType == 8))
                benchmark::report(
                    boost::bind(Updates, db, tb, start, end, USE_TRANS, 20),
                    ": update in transaction. 20rec x 1000times.");
        }else
            printf("open table erorr No:%d\r\n", db->stat());
    }
    client::database::destroy(db);
    printTail();
    return 0;
}
