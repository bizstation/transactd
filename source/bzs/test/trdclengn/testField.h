#ifndef BZS_TEST_TRDCLENGN_TESTFIELD_H
#define BZS_TEST_TRDCLENGN_TESTFIELD_H
/* =================================================================
 Copyright (C) 2015 BizStation Corp All rights reserved.

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
 ================================================================= */
#include "testbase.h"
#include <limits.h>

short createFieldStoreDataBase(database* db)
{
    db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
        if (db->stat()) return db->stat();
        db->drop();
        db->create(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME));
    }
    if (db->stat()) return db->stat();
    return 0;
}

short createTestTable1(database* db)
{
    try
    {
        openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
        dbdef* def = db->dbDef();
        short tableid = 1;
        
        insertTable(def, tableid,  _T("fieldtest"), g_td_charsetIndex);
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        int lens[5] = {1, 2, 3, 4, 8};
        _TCHAR buf[50];
        //int
        for (int i=1; i < 6 ; ++i)
        {
            _stprintf_s(buf, 50, _T("int_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_integer, lens[i-1]);
        }
        
        //unsigned int
        for (int i=1; i < 6 ; ++i)
        {
            
            _stprintf_s(buf, 50, _T("uint_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_uinteger, lens[i-1]);
        }

        //myyear
        fd = insertField(def, tableid, ++fieldnum, _T("year"), ft_myyear, 1);
        
        //logical
        fd = insertField(def, tableid, ++fieldnum, _T("logical1"), ft_logical, 1);
        fd = insertField(def, tableid, ++fieldnum, _T("logical2"), ft_logical, 2);
        
        //mydate
        fd = insertField(def, tableid, ++fieldnum, _T("date"), ft_mydate, 3);
 
        //double
        fd = insertField(def, tableid, ++fieldnum, _T("double4.0"), ft_float, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("double4.4"), ft_float, 4);
        fd->decimals = 4;

        fd = insertField(def, tableid, ++fieldnum, _T("double8.0"), ft_float, 8);
        fd = insertField(def, tableid, ++fieldnum, _T("double8.5"), ft_float, 8);
        fd->decimals = 15;


        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1; // extended key type
        kd->segments[0].flags.kf_changeatable = 1; // changeable
        kd->segmentCount = 1;

        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        updateTableDef(def, tableid);
        return 0;
   
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestTableLegacyTimeTable(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 2;
        
        insertTable(def, tableid,  _T("timetestLegacy"), g_td_charsetIndex);
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        //time
        fd = insertField(def, tableid, ++fieldnum, _T("time"), ft_mytime, 3);

        //datetime
        fd = insertField(def, tableid, ++fieldnum, _T("datetime"), ft_mydatetime, 8);

        //timestamp
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp"), ft_mytimestamp, 4);

 
        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1; // extended key type
        kd->segments[0].flags.kf_changeatable = 1; // changeable
        kd->segmentCount = 1;

        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        updateTableDef(def, tableid);
        return 0;
   
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

/* If MySQL internal table flag has HA_OPTION_PACK_RECORD
    then null_bit is offset one.
*/
short createTestTableHA_OPTION_PACK_RECORD(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 3;
        
        insertTable(def, tableid,  _T("packrecord_test"), g_td_charsetIndex);
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        fd = insertField(def, tableid, ++fieldnum, _T("int"), ft_integer, 4);
        fd->setNullable(true, false);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1; // extended key type
        kd->segments[0].flags.kf_changeatable = 1; // changeable
        kd->segmentCount = 1;

        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        updateTableDef(def, tableid);
        return 0;
   
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestTableTime(database* db, bool isMariadb, uchar_td minorVersion, bool isSupportMultiTimeStamp)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 2;

        insertTable(def, tableid,  _T("timetest"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        td->setValidationTarget(isMariadb, minorVersion);

        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        //time
        fd = insertField(def, tableid, ++fieldnum, _T("time5"), ft_mytime, 5);
        fd->decimals = 3;
        fd = insertField(def, tableid, ++fieldnum, _T("time3"), ft_mytime, 3);
        fd = insertField(def, tableid, ++fieldnum, _T("time4"), ft_mytime, 4);
        fd->decimals = 2;
        fd = insertField(def, tableid, ++fieldnum, _T("time6"), ft_mytime, 6);
        fd->decimals = 6;

        //datetime
        fd = insertField(def, tableid, ++fieldnum, _T("datetime7"), ft_mydatetime, 7);
        fd->decimals = 3;
        fd = insertField(def, tableid, ++fieldnum, _T("datetime5"), ft_mydatetime, 5);
        fd = insertField(def, tableid, ++fieldnum, _T("datetime6"), ft_mydatetime, 6);
        fd->decimals = 2;
        fd = insertField(def, tableid, ++fieldnum, _T("datetime8"), ft_mydatetime, 8);
        fd->decimals = 6;

        //timestamp
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp6"), ft_mytimestamp, 6);
        fd->decimals = 3;
        
        if (isSupportMultiTimeStamp)
        {
            fd = insertField(def, tableid, ++fieldnum, _T("timestamp4"), ft_mytimestamp, 4);
            fd->setNullable(true);
            fd = insertField(def, tableid, ++fieldnum, _T("timestamp5"), ft_mytimestamp, 5);
            fd->decimals = 2;
            fd->setNullable(true);
            fd = insertField(def, tableid, ++fieldnum, _T("timestamp7"), ft_mytimestamp, 7);
            fd->decimals = 6;
            fd->setNullable(true);
        }
        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1; // extended key type
        kd->segments[0].flags.kf_changeatable = 1; // changeable
        kd->segmentCount = 1;

        
        updateTableDef(def, tableid);
        return 0;
   
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestInMany(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 4;

        insertTable(def, tableid,  _T("nullkey"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);

        //time
        fd = insertField(def, tableid, ++fieldnum, _T("id2"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("id3"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("name"), ft_myvarchar, 151);
        fd->setLenByCharnum(50);
        fd->setNullable(true);

        
        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 1);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segments[0].flags.kf_allseg_nullkey = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 2);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segments[1].fieldNum = 2;
        kd->segments[1].flags.kf_extend = 1;
        kd->segments[1].flags.kf_changeatable = 1;
        kd->segments[1].flags.kf_duplicatable = 1;
        kd->segmentCount = 2;
        updateTableDef(def, tableid);
        return 0;
   
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestNull(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 5;

        insertTable(def, tableid,  _T("nullvalue"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("id2"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("id3"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("name"), ft_myvarchar, 151);
        fd->setLenByCharnum(50);
        fd->setNullable(true);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 1);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 2);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segments[1].fieldNum = 2;
        kd->segments[1].flags.kf_extend = 1;
        kd->segments[1].flags.kf_changeatable = 1;
        kd->segments[1].flags.kf_duplicatable = 1;
        kd->segmentCount = 2;
        updateTableDef(def, tableid);
        return 0;
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestGroups(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 6;

        insertTable(def, tableid,  _T("groups"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("pri_id"), ft_autoinc, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("id"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("name"), ft_myvarchar, 151);
        fd->setLenByCharnum(50);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 1);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 2);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[1].fieldNum = 1;
        kd->segments[1].flags.kf_extend = 1;
        kd->segments[1].flags.kf_changeatable = 1;
        kd->segments[1].flags.kf_duplicatable = 1;
        kd->segmentCount = 2;
        updateTableDef(def, tableid);
        return 0;
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestUsers(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 7;

        insertTable(def, tableid,  _T("users"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("group"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("class"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("name"), ft_myvarchar, 151);
        fd->setLenByCharnum(50);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segmentCount = 1;

        kd = insertKey(def, tableid, 1);
        kd->segments[0].fieldNum = 1;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segments[0].flags.kf_duplicatable = 1;
        kd->segmentCount = 1;
        updateTableDef(def, tableid);
        return 0;
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

short createTestScores(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 8;

        insertTable(def, tableid,  _T("scores"), g_td_charsetIndex);
        tabledef* td = def->tableDefs(tableid);
        td->primaryKeyNum = 0;
        
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("subject"), ft_integer, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("score"), ft_integer, 4);
        fd->setNullable(true);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.kf_extend = 1;
        kd->segments[0].flags.kf_changeatable = 1;
        kd->segmentCount = 1;
        updateTableDef(def, tableid);
        return 0;
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
    return 1;
}

class fixtureFieldStore
{
    mutable database* m_db;
    btrVersion ver;

public:
    fixtureFieldStore() : m_db(NULL)
    {
        nsdatabase::setCheckTablePtr(true);
        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
        short ret = createFieldStoreDataBase(m_db);
        if (ret)
        {    
            printf("Error createDataBase\n");
            return;
        }else
        {
            ret = createTestTable1(m_db);
            if (ret == 0)
            {
                btrVersions vs;
                m_db->getBtrVersion(&vs);
                if (m_db->stat())
                {    
                    printf("Error getBtrVersion\n");
                    return;
                }
                ver = vs.versions[1];

                if (ret == 0)
                {   if (isLegacyTimeFormat())
                        ret = createTestTableLegacyTimeTable(m_db);
                    else 
                        ret = createTestTableTime(m_db, ver.isMariaDB(), (uchar_td)ver.minorVersion, 
                                    isSupportMultiTimeStamp());
                    if (ret == 0)
                        ret = createTestTableHA_OPTION_PACK_RECORD(m_db);
                    if (ret == 0)
                        ret = createTestInMany(m_db);
                    if (ret == 0)
                        ret = createTestNull(m_db);
                    if (ret == 0)
                        ret = createTestGroups(m_db);
                    if (ret == 0)
                        ret = createTestUsers(m_db);
                    if (ret == 0)
                        ret = createTestScores(m_db);

                    if (ret == 0)
                        m_db->open(makeUri(PROTOCOL, HOSTNAME, DBNAME, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
                }
            }
            ret = m_db->stat();
        }
        if (ret)
            printf("Error createTable\n");
    }

    ~fixtureFieldStore()
    {
        if (m_db)
            m_db->release();
    }
    ::database* db() const { return m_db; }

    bool isMysql56TimeFormat() const { return ver.isMysql56TimeFormat(); }
    bool isLegacyTimeFormat() const{ return ver.isFullLegacyTimeFormat(); }
    bool isSupportMultiTimeStamp() const { return ver.isSupportMultiTimeStamp(); }
};
#define TEST_DATE _T("2015-11-10")
#define TEST_DATEA "2015-11-10"

void testModeMacro()
{
    short mode = TD_OPEN_READONLY_EXCLUSIVE +  TD_OPEN_MASK_MYSQL_NULL +  TD_OPEN_MASK_GETSHCHEMA;
    BOOST_CHECK_MESSAGE(IS_MODE_READONLY(mode) == true, "IS_MODE_READONLY ");
    BOOST_CHECK_MESSAGE(IS_MODE_EXCLUSIVE(mode) == true, "IS_MODE_EXCLUSIVE ");
    BOOST_CHECK_MESSAGE(IS_MODE_MYSQL_NULL(mode) == true, "IS_MODE_MYSQL_NULL ");
    BOOST_CHECK_MESSAGE(IS_MODE_GETSCHEMA(mode) == true, "IS_MODE_GETSCHEMA ");

    mode = 0;
    BOOST_CHECK_MESSAGE(IS_MODE_READONLY(mode) == false, "IS_MODE_READONLY false");
    BOOST_CHECK_MESSAGE(IS_MODE_EXCLUSIVE(mode) == false, "IS_MODE_EXCLUSIVE false");
    BOOST_CHECK_MESSAGE(IS_MODE_MYSQL_NULL(mode) == false, "IS_MODE_MYSQL_NULL false");
    BOOST_CHECK_MESSAGE(IS_MODE_GETSCHEMA(mode) == false, "IS_MODE_GETSCHEMA false");
}
#pragma warning(disable : 4996)

#define MINT_MIN -8388608
#define UMINT_MAX 16777215
#define FLOAT_V1 -1234.0f
#define FLOAT_V2 1234.1234f

#define DOUBLE_V1 (double)-12345678.0f
#define DOUBLE_V2 (double)0.1234567890123f

void checkIntValue(table_ptr tb)
{
    short fieldnum = 0;
    // read by int64
    BOOST_CHECK(tb->getFVbyt(++fieldnum) == SCHAR_MAX);
    BOOST_CHECK(tb->getFVsht(++fieldnum) == SHRT_MAX);
    BOOST_CHECK(tb->getFVint(++fieldnum) == MINT_MIN);
    BOOST_CHECK(tb->getFVint(++fieldnum) == INT_MAX);
    BOOST_CHECK(tb->getFV64(++fieldnum) == LLONG_MAX);

    BOOST_CHECK(tb->getFVsht(++fieldnum) == UCHAR_MAX - 1);
    BOOST_CHECK(tb->getFVint(++fieldnum) == USHRT_MAX - 1);
    BOOST_CHECK(tb->getFVint(++fieldnum) == UMINT_MAX - 1);
    BOOST_CHECK(tb->getFV64(++fieldnum) == UINT_MAX - 1);
    BOOST_CHECK((unsigned __int64)tb->getFV64(++fieldnum) == ULLONG_MAX - 1); 

    BOOST_CHECK(tb->getFVsht(++fieldnum) == 2000); 
    BOOST_CHECK(tb->getFVbyt(++fieldnum) == 254);  //logi1
    BOOST_CHECK(tb->getFVint(++fieldnum) == 65000); //logi2
    myDate d;
    d = TEST_DATE;
    BOOST_CHECK(tb->getFVint(++fieldnum) == d.i); //date

    BOOST_CHECK(tb->getFVint(++fieldnum) == FLOAT_V1); //double
    ++fieldnum;//BOOST_CHECK(tb->getFVint(++fieldnum) == FLOAT_V2);
    BOOST_CHECK(tb->getFV64(++fieldnum) == DOUBLE_V1); 
    ++fieldnum;//BOOST_CHECK(tb->getFV64(++fieldnum) == DOUBLE_V2);


    // read by double
    BOOST_CHECK(tb->getFVdbl(1) == SCHAR_MAX);
    BOOST_CHECK(tb->getFVdbl(3) == MINT_MIN);
    double v = tb->getFVdbl(5);
    BOOST_CHECK(v == ((double)LLONG_MAX));
    BOOST_CHECK(tb->getFVdbl(6) == UCHAR_MAX - 1);
    BOOST_CHECK(tb->getFVdbl(8) == UMINT_MAX - 1);
    fieldnum = 9;
    v = tb->getFVdbl(++fieldnum);
    BOOST_CHECK(v == ((double)ULLONG_MAX - 1));
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == 2000);
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == 254);  //logi1
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == 65000); //logi2
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)d.i); //date
    
    float f = tb->getFVflt(++fieldnum);
    BOOST_CHECK(f == FLOAT_V1); //double
    f = tb->getFVflt(++fieldnum);
    BOOST_CHECK(f == FLOAT_V2);
    v = tb->getFVdbl(++fieldnum);
    BOOST_CHECK(v == DOUBLE_V1); 
    v = tb->getFVdbl(++fieldnum);
    BOOST_CHECK(v == DOUBLE_V2);


    //read by string
    _TCHAR buf[50];
    BOOST_CHECK(_tcscmp(tb->getFVstr(1), _ltot(SCHAR_MAX, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(3), _ltot(MINT_MIN, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(5), _i64tot(LLONG_MAX, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(6), _ultot(UCHAR_MAX - 1, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(8), _ultot(UMINT_MAX - 1, buf, 10)) == 0);
    fieldnum = 9;
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), _ui64tot(ULLONG_MAX - 1, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), _ltot(2000, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), _ltot(254, buf, 10)) == 0);   //logi1
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), _ltot(65000, buf, 10)) == 0); //logi2
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), TEST_DATE) == 0);             //date

    _TCHAR tmp[64];
    _stprintf(tmp, _T("%0.0f"), FLOAT_V1);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //double
    _stprintf(tmp, _T("%0.4f"), FLOAT_V2);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //double
    _stprintf(tmp, _T("%0.0f"), DOUBLE_V1);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //double
    _stprintf(tmp, _T("%0.15lf"), DOUBLE_V2);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //double

}

void testStoreInt(database* db)
{
    short tableid = 1;
    short fieldnum = 0;
    table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);

    //int64 store
    tb->clearBuffer();
    tb->setFV(_T("id"), 1);

    tb->setFV(++fieldnum, (short)SCHAR_MAX);
    tb->setFV(++fieldnum, (short)SHRT_MAX);
    tb->setFV(++fieldnum, (int)MINT_MIN);
    tb->setFV(++fieldnum, (int)INT_MAX);
    tb->setFV(++fieldnum, (__int64)LLONG_MAX);

    tb->setFV(++fieldnum, (short)(UCHAR_MAX - 1));
    tb->setFV(++fieldnum, (int)(USHRT_MAX - 1));
    tb->setFV(++fieldnum, (int)(UMINT_MAX - 1));
    tb->setFV(++fieldnum, (__int64)(UINT_MAX - 1));
    tb->setFV(++fieldnum, (__int64)(ULLONG_MAX - 1)); 
    tb->setFV(++fieldnum, 2000); 
    tb->setFV(++fieldnum, 254);   //logi1
    tb->setFV(++fieldnum, (int)65000); //logi2
    myDate d; d = TEST_DATE;
    tb->setFV(++fieldnum, d.i);
    tb->setFV(++fieldnum, (int)FLOAT_V1);
    tb->setFV(++fieldnum, FLOAT_V2);
    tb->setFV(++fieldnum, (__int64)DOUBLE_V1);
    tb->setFV(++fieldnum, DOUBLE_V2);
    tb->insert();

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);

    // char* or wchar* store
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    _TCHAR buf[50];
    fieldnum = 0;
    tb->setFV(++fieldnum, _ltot(SCHAR_MAX, buf, 10));
    tb->setFV(++fieldnum, _ltot(SHRT_MAX, buf, 10));
    tb->setFV(++fieldnum, _ltot(MINT_MIN, buf, 10));
    tb->setFV(++fieldnum, _ltot(INT_MAX, buf, 10));
    tb->setFV(++fieldnum, _i64tot(LLONG_MAX, buf, 10));

    tb->setFV(++fieldnum, _ltot(UCHAR_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(USHRT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(UMINT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ultot(UINT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ui64tot(ULLONG_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(2000, buf, 10));
    tb->setFV(++fieldnum, _ltot(254, buf, 10));  //logi1
    tb->setFV(++fieldnum, _ltot(65000, buf, 10)); //logi2
    tb->setFV(++fieldnum, TEST_DATE);

    _stprintf(buf, _T("%.0f"), FLOAT_V1);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.4f"), FLOAT_V2);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.0lf"), DOUBLE_V1);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.20lf"), DOUBLE_V2);
    tb->setFV(++fieldnum, buf);
   

    tb->insert();
        
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);

    // double store
    tb->clearBuffer();
    tb->setFV(_T("id"), 3);
    fieldnum = 0;
    tb->setFV(++fieldnum, (double)SCHAR_MAX);
    tb->setFV(++fieldnum, (double)SHRT_MAX);
    tb->setFV(++fieldnum, (double)MINT_MIN);
    tb->setFV(++fieldnum, (double)INT_MAX);
    // LLONG_MAX double store not support, because happen truncation
    //tb->setFV(++fieldnum, (double)LLONG_MAX);
    tb->setFV(++fieldnum, (__int64)LLONG_MAX);
    tb->setFV(++fieldnum, (double)(UCHAR_MAX - 1));
    tb->setFV(++fieldnum, (double)(USHRT_MAX - 1));
    tb->setFV(++fieldnum, (double)(UMINT_MAX - 1));
    tb->setFV(++fieldnum, (double)(UINT_MAX - 1));
    //tb->setFV(++fieldnum, (double)(ULLONG_MAX - 1));
    tb->setFV(++fieldnum, (__int64)(ULLONG_MAX - 1));
    tb->setFV(++fieldnum, (double)2000); 
    tb->setFV(++fieldnum, (double)254);   //logi1
    tb->setFV(++fieldnum, (double)65000); //logi2
    tb->setFV(++fieldnum, (double)d.i);
    tb->setFV(++fieldnum, FLOAT_V1);
    tb->setFV(++fieldnum, FLOAT_V2);
    tb->setFV(++fieldnum, DOUBLE_V1);
    tb->setFV(++fieldnum, DOUBLE_V2);


    tb->insert();
        
    tb->clearBuffer();
    tb->setFV(_T("id"), 3);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);


    // cleanup id 3
    tb->clearBuffer();
    tb->setFV(_T("id"), 3);
    tb->update();
    BOOST_CHECK(tb->stat() == 0);

    char buf2[50];
    std::string values;
    values += "1\t";
    values += _ltoa(SCHAR_MAX, buf2, 10);
    values += "\t2\t";
    values += _ltoa(SHRT_MAX, buf2, 10);
    values += "\t3\t";
    values += _ltoa(MINT_MIN, buf2, 10);
    values += "\t4\t";
    values += _ltoa(INT_MAX, buf2, 10);
    values += "\t5\t";
    values += _i64toa(LLONG_MAX, buf2, 10);
    values += "\t6\t";
    values += _ltoa(UCHAR_MAX - 1, buf2, 10);
    values += "\t7\t";
    values += _ltoa(USHRT_MAX - 1, buf2, 10);
    values += "\t8\t";
    values += _ltoa(UMINT_MAX - 1, buf2, 10);
    values += "\t9\t";
    values += _i64toa(UINT_MAX - 1, buf2, 10);
    values += "\t10\t";
    values += _ui64toa(ULLONG_MAX - 1, buf2, 10);
    values += "\t11\t";
    values += _ltoa(2000, buf2, 10);
    values += "\t12\t";
    values += _ltoa(254, buf2, 10);
    values += "\t13\t";
    values += _ltoa(65000, buf2, 10);
    values += "\t14\t";
    values += TEST_DATEA;
    values += "\t15\t";sprintf_s(buf2, 50, "%.0f", FLOAT_V1);
    values += buf2;
    values += "\t16\t";sprintf_s(buf2, 50, "%.4f", FLOAT_V2);
    values += buf2;
    values += "\t17\t";sprintf_s(buf2, 50, "%.0lf", DOUBLE_V1);
    values += buf2;
    values += "\t18\t";sprintf_s(buf2, 50, "%.20lf", DOUBLE_V2);
    values += buf2;
    

    tb->test_store(values.c_str());
    BOOST_CHECK(tb->stat() == 0);
    tb->clearBuffer();
    tb->setFV(_T("id"), 3);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);
}

#define TEST_TIME0 _T("23:41:12")
#define TEST_TIME0A "23:41:12"

#define TEST_DATETIME0 _T("2015-11-09 12:31:56")
#define TEST_DATETIME0A "2015-11-09 12:31:56"

const _TCHAR*  TEST_TIME[7] = {
    _T("23:41:12"),
    _T(""),
    _T("23:41:12.34"),
    _T("23:41:12.987"),
    _T(""),
    _T(""),
    _T("23:41:12.123456") 
};

const char* TEST_TIMEA[7] = {
    "23:41:12",
    "",
    "23:41:12.34",
    "23:41:12.987",
    "",
    "",
    "23:41:12.123456" 
};

const _TCHAR* TEST_DATETIME[7] = {
    _T("2015-11-09 12:31:56"),
    _T(""),
    _T("2015-11-09 12:31:56.98"),
    _T("2015-11-09 12:31:56.987"),
    _T(""),
    _T(""),
    _T("2015-11-09 12:31:56.123456")
};

const char* TEST_DATETIMEA[7] = {
    "2015-11-09 12:31:56",
    "",
    "2015-11-09 12:31:56.98",
    "2015-11-09 12:31:56.987",
    "",
    "",
    "2015-11-09 12:31:56.123456"
};


void checkLegacyTimeValue(table_ptr tb)
{
    short fieldnum = 0;
    // read by int64
    myTime t(0, false); t = TEST_TIME0;
    BOOST_CHECK(tb->getFV64(++fieldnum) == t.i64);

    myDateTime dt(0, false); dt = TEST_DATETIME0;
    BOOST_CHECK(tb->getFV64(++fieldnum) == dt.i64);

    myTimeStamp ts(0, false); ts = TEST_DATETIME0;
    BOOST_CHECK(tb->getFV64(++fieldnum) == ts.i64);

    // read by double
    fieldnum = 0;
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)t.i64);
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)dt.i64);
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)ts.i64);

    //read by string
    fieldnum = 0;
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), TEST_TIME0) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), TEST_DATETIME0) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), TEST_DATETIME0) == 0);

}

void checkAutoTimeStamp(__int64& oldValue, __int64 newValue)
{
    BOOST_CHECK(oldValue != newValue);
    _TCHAR tmp[30];
    myTimeStamp ts_auto(0, false);
    ts_auto.i64 = newValue;
    BOOST_CHECK(_tcscmp(btrdtoa(getNowDate(),(_TCHAR*)NULL, true), ts_auto.dateStr(tmp)) == 0);

    oldValue = newValue;
}

void testStoreLegacyTime(database* db)
{
    short tableid = 2;
    short fieldnum = 0;

    table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
    tb->setTimestampMode(TIMESTAMP_VALUE_CONTROL);
    BOOST_CHECK(tb->stat() == 0);

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);

    myTime t(0, false); t = TEST_TIME0;
    tb->setFV(++fieldnum, t.i64);

    myDateTime dt(0, false); dt = TEST_DATETIME0;
    tb->setFV(++fieldnum, dt.i64);

    myTimeStamp ts(0, false); ts = TEST_DATETIME0;
    tb->setFV(++fieldnum, ts.i64);
    tb->insert();

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkLegacyTimeValue(tb);
    

    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    fieldnum = 0;
    tb->setFV(++fieldnum, TEST_TIME0);
    tb->setFV(++fieldnum, TEST_DATETIME0);
    tb->setFV(++fieldnum, TEST_DATETIME0);
    tb->insert();
        
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkLegacyTimeValue(tb);
    __int64 ts_auto_i = tb->getFV64(3);

    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->update();
    BOOST_CHECK(tb->stat() == 0);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkAutoTimeStamp(ts_auto_i, tb->getFV64(3));


    std::string values;
    values += "1\t";
    values += TEST_TIME0A;
    values += "\t2\t";
    values += TEST_DATETIME0A;
    values += "\t3\t";
    values += TEST_DATETIME0A;

    tb->test_store(values.c_str());
    BOOST_CHECK(tb->stat() == 0);
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkLegacyTimeValue(tb);
}

void checkTimeValue(table_ptr tb, short fieldIndex, int decimals, bool mySql56timeFormat,
        bool isSupportMultiTimeStamp)
{
    // read by int64
    myTime t(decimals, true); 
    t = TEST_TIME[decimals]; 
    BOOST_CHECK_MESSAGE(tb->getFV64(fieldIndex) == t.i64, "myTime  decimals = " << decimals);
    myDateTime dt(decimals, true); 
    maDateTime dta(decimals, true); 
    if (mySql56timeFormat)
    {
        dt = TEST_DATETIME[decimals]; 
        BOOST_CHECK_MESSAGE(tb->getFV64(fieldIndex + 4) == dt.i64, "myDateTime decimals = " << decimals);
    }
    else
    {
        dta = TEST_DATETIME[decimals]; 
        BOOST_CHECK_MESSAGE(tb->getFV64(fieldIndex + 4) == dta.i64, "maDateTime decimals = " << decimals);
    }
    if (isSupportMultiTimeStamp || decimals == 3)
    {
        myTimeStamp ts(decimals, true); 
        ts = TEST_DATETIME[decimals]; 
        BOOST_CHECK_MESSAGE(tb->getFV64(fieldIndex + 8) == ts.i64, "myTimeStamp decimals = " << decimals);
    }
    //read by string
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fieldIndex), TEST_TIME[decimals]) == 0, "myTime  decimals = " << decimals);
    BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fieldIndex + 4), TEST_DATETIME[decimals]) == 0, "dateTime  decimals = " << decimals);
    if (isSupportMultiTimeStamp || decimals == 3)
        BOOST_CHECK_MESSAGE(_tcscmp(tb->getFVstr(fieldIndex + 8), TEST_DATETIME[decimals]) == 0, "myTimeStamp  decimals = " << decimals);
}

void checkTimeValues(table_ptr tb, bool mySql56timeFormat, bool isSupportMultiTimeStamp)
{
    short fieldnum = 0;
    checkTimeValue(tb, ++fieldnum, 3, mySql56timeFormat, isSupportMultiTimeStamp); //3 decimals
    checkTimeValue(tb, ++fieldnum, 0, mySql56timeFormat, isSupportMultiTimeStamp); //0 decimal
    checkTimeValue(tb, ++fieldnum, 2, mySql56timeFormat, isSupportMultiTimeStamp); //2 decimals
    checkTimeValue(tb, ++fieldnum, 6, mySql56timeFormat, isSupportMultiTimeStamp); //6 decimals 
}

void setTimeValue64(table_ptr tb, short fieldIndex, int decimals, bool mySql56timeFormat, 
            bool isSupportMultiTimeStamp)
{
    myTime t(decimals, true); 
    t = TEST_TIME[decimals];  tb->setFV(fieldIndex, t.i64);
    myDateTime dt(decimals, true); 
    maDateTime dta(decimals, true); 
    if (mySql56timeFormat)
    {
        dt = TEST_DATETIME[decimals]; tb->setFV(fieldIndex + 4, dt.i64);
    }
    else
    {
        dta = TEST_DATETIME[decimals]; tb->setFV(fieldIndex + 4, dta.i64);
    }
    if (!isSupportMultiTimeStamp && decimals != 3) return;

    myTimeStamp ts(decimals, true); 
    ts = TEST_DATETIME[decimals]; tb->setFV(fieldIndex + 8, ts.i64);
}

void testStoreTime(database* db, bool mySql56timeFormat, bool isSupportMultiTimeStamp)
{
    short tableid = 2;
    short fieldnum = 0;


#if (defined(_DEBUG) && defined(_WIN32))
    char sqlTemp[10240];
    uint_td datalen = 10240;
    db->getSqlStringForCreateTable(_T("timetest"), sqlTemp, &datalen);
    OutputDebugStringA(sqlTemp);
#endif    
    table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
    tb->setTimestampMode(TIMESTAMP_VALUE_CONTROL);
    BOOST_CHECK(tb->stat() == 0);

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);

    setTimeValue64(tb, ++fieldnum, 3, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 0, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 2, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 6, mySql56timeFormat, isSupportMultiTimeStamp);
    tb->insert();

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkTimeValues(tb, mySql56timeFormat, isSupportMultiTimeStamp);
    
 
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    fieldnum = 0;
    setTimeValue64(tb, ++fieldnum, 3, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 0, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 2, mySql56timeFormat, isSupportMultiTimeStamp);
    setTimeValue64(tb, ++fieldnum, 6, mySql56timeFormat, isSupportMultiTimeStamp);
    tb->insert();
        
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkTimeValues(tb, mySql56timeFormat, isSupportMultiTimeStamp);
    __int64 ts_auto_i = tb->getFV64(9);

    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->update();
    BOOST_CHECK(tb->stat() == 0);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkAutoTimeStamp(ts_auto_i, tb->getFV64(9));
    

    std::string values;
    values += "1\t";
    values += TEST_TIMEA[3];
    values += "\t2\t";
    values += TEST_TIMEA[0];
    values += "\t3\t";
    values += TEST_TIMEA[2];
    values += "\t4\t";
    values += TEST_TIMEA[6];
    
    values += "\t5\t";
    values += TEST_DATETIMEA[3];
    values += "\t6\t";
    values += TEST_DATETIMEA[0];
    values += "\t7\t";
    values += TEST_DATETIMEA[2];
    values += "\t8\t";
    values += TEST_DATETIMEA[6];
    
    values += "\t9\t";
    values += TEST_DATETIMEA[3];
    if (isSupportMultiTimeStamp)
    {
        values += "\t10\t";
        values += TEST_DATETIMEA[0];
        values += "\t11\t";
        values += TEST_DATETIMEA[2];
        values += "\t12\t";
        values += TEST_DATETIMEA[6];
    }

    tb->test_store(values.c_str());
    BOOST_CHECK(tb->stat() == 0);
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkTimeValues(tb, mySql56timeFormat, isSupportMultiTimeStamp);
}

void test_NOT_HA_OPTION_PACK_RECORD(database* db)
{
    short tableid = 3;
    short fieldnum = 0;
    table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
    tb->clearBuffer();
    BOOST_CHECK(tb->getFVNull(1) == false);

    tb->setFV(fieldnum++, 1);
    tb->setFV(fieldnum++, 10);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldnum = 0;
    tb->clearBuffer();
    tb->setFV(fieldnum++, 2);
    tb->setFVNull(fieldnum++, true);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldnum = 0;
    tb->clearBuffer();
    tb->setFV(fieldnum++, 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    BOOST_CHECK(tb->getFVNull(fieldnum) == true);

}

void insertInManyData(table_ptr tb)
{
    short fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, _T("test"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, _T("test2"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T(""));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 4);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T("test4"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 4);
    tb->setFV(++fieldNum, 5);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
}

void insertGroupsData(table_ptr tb)
{
    short fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, _T("Administrators"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, _T("Users"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, _T("Guests"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T("Unknowns"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
}

void insertUsersData(table_ptr tb)
{
    short fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, _T("John"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T("Alice"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, _T("Pochi"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, _T("Bob"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, _T("Tama"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 4);
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, _T("Microsoft"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T("Susie"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, _T("Taro"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 4);
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, _T("Google"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, _T("Lee"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->setFV(++fieldNum, _T("Hanako"));
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
}

void insertScoresData(table_ptr tb)
{
    short fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 80);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 70);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 50);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);

    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, (_TCHAR*)NULL);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, 90);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, 85);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 80);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 70);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 50);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 0);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 1);
    tb->setFV(++fieldNum, 60);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 2);
    tb->setFV(++fieldNum, 87);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
    
    fieldNum = 0;
    tb->clearBuffer();
    tb->setFV(++fieldNum, 3);
    tb->setFV(++fieldNum, 90);
    tb->insert();
    BOOST_CHECK(tb->stat() == 0);
}

void testInMany(database* db)
{
    short tableid = 4;
    {
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
        insertInManyData(tb);
    } 
    activeTable atv(db, _T("nullkey"));
    atv.index(1);
    query q;
    recordset rs;
    q.in(1, 2, 3);
    atv.read(rs, q);
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1].isInvalidRecord() == true);
    BOOST_CHECK(rs[2][_T("id")] == 3);

    // read by nullkey
    q.reset();
    atv.index(1).keyValue(0).read(rs, q);
    BOOST_CHECK(rs.size() == 4);

    // read records all, that include null value
    atv.index(2).keyValue((const _TCHAR*)NULL).read(rs, q);
    BOOST_CHECK(rs.size() == 6);
    rs.orderBy(_T("id2"));
    BOOST_CHECK(rs[0][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[1][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[2][_T("id2")].isNull() == false);

    sortFields sf;
    sf.add(_T("id2"), false);
    rs.orderBy(sf);
    BOOST_CHECK(rs[5][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[4][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[3][_T("id2")].isNull() == false);

    recordset& rs2 = *rs.clone();
    recordsetQuery rq;
    rq.whenIsNotNull(_T("id2"));
    rs2.matchBy(rq);
    BOOST_CHECK(rs2.size() == 4);
    //rs2.release();

    rs2 = *rs.clone();
    rq.whenIsNull(_T("id2"));
    rs2.matchBy(rq);
    BOOST_CHECK(rs2.size() == 2);
    //rs2.release();

    rs2 = *rs.clone();
    rq.when(_T("id2"), _T("="), 4);
    rs2.matchBy(rq);
    BOOST_CHECK(rs2.size() == 2);

    rs2 = *rs.clone();
    rq.when(_T("id2"), _T("<"), 3);
    rs2.matchBy(rq);
    BOOST_CHECK(rs2.size() == 1);

    rs2.release();

    // read records after null.
    atv.index(2).keyValue(0).read(rs, q);
    BOOST_CHECK(rs.size() == 4);

    // read in has_many.
    q.reset().segmentsForInValue(1).in(4);
    atv.index(2).read(rs, q);
    BOOST_CHECK(rs.size() == 2);
    BOOST_CHECK(rs[0][_T("id")] == 4);
    BOOST_CHECK(rs[1][_T("id")] == 5);
}

void testNullValue(database* db)
{
    {
        short tableid = 5;
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
        insertInManyData(tb);
        tableid = 6;
        table_ptr tbg = openTable(db, tableid, TD_OPEN_NORMAL);
        insertGroupsData(tbg);
        tableid = 7;
        table_ptr tbu = openTable(db, tableid, TD_OPEN_NORMAL);
        insertUsersData(tbu);
        tableid = 8;
        table_ptr tbs = openTable(db, tableid, TD_OPEN_NORMAL);
        insertScoresData(tbs);
    }
    activeTable atv(db, _T("nullvalue"));
    atv.index(1);
    activeTable atg(db, _T("groups"));
    atg.alias(_T("id"), _T("group_id")).alias(_T("name"), _T("group_name"));
    atg.index(1);
    activeTable atu(db, _T("users"));
    atu.index(0);
    activeTable ats(db, _T("scores"));
    ats.index(0);
    query q;
    recordsetQuery rq;
    groupQuery gq;
    recordset rs;
    fieldNames fns;
    fns.keyField(_T("score"));
    bzs::db::protocol::tdap::client::count countFunc(_T("row_count"));
    bzs::db::protocol::tdap::client::count  countField(fns, _T("valid_row"));
    bzs::db::protocol::tdap::client::sum    sumField(fns, _T("sum"));
    bzs::db::protocol::tdap::client::avg    avgField(fns, _T("avg"));
    bzs::db::protocol::tdap::client::min    minField(fns, _T("min"));
    bzs::db::protocol::tdap::client::max    maxField(fns, _T("max"));
    
    // [1] WHERE id2 = 0
    // query
    rs.clear();
    q.reset().where(_T("id2"), _T("="), 0).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 0);
    // query reverse
    rs.clear();
    q.reset().where(_T("id2"), _T("="), 0).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 0);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("id2"), _T("="), 0);
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 0);
    
    // [2] WHERE id2 <> 0
    // query
    rs.clear();
    q.reset().where(_T("id2"), _T("<>"), 0).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    // query reverse
    rs.clear();
    q.reset().where(_T("id2"), _T("<>"), 0).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 5);
    BOOST_CHECK(rs[1][_T("id")] == 4);
    BOOST_CHECK(rs[2][_T("id")] == 3);
    BOOST_CHECK(rs[3][_T("id")] == 1);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("id2"), _T("<>"), 0);
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    
    // [3] WHERE id2 >= 0
    // query
    rs.clear();
    q.reset().where(_T("id2"), _T(">="), 0).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    // query reverse
    rs.clear();
    q.reset().where(_T("id2"), _T(">="), 0).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 5);
    BOOST_CHECK(rs[1][_T("id")] == 4);
    BOOST_CHECK(rs[2][_T("id")] == 3);
    BOOST_CHECK(rs[3][_T("id")] == 1);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("id2"), _T(">="), 0);
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    
    // [4] WHERE id2 <= 5
    // query
    rs.clear();
    q.reset().where(_T("id2"), _T("<="), 5).reject(0);
    atv.keyValue(0).read(rs, q); // set keyValue as 0, CAN NOT USE NULL IN THIS OPERATION.
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    // query reverse
    rs.clear();
    q.reset().where(_T("id2"), _T("<="), 5).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 5);
    BOOST_CHECK(rs[1][_T("id")] == 4);
    BOOST_CHECK(rs[2][_T("id")] == 3);
    BOOST_CHECK(rs[3][_T("id")] == 1);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("id2"), _T("<="), 5);
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    
    // [5] WHERE name like 'test%'
    // query
    rs.clear();
    q.reset().where(_T("name"), _T("="), _T("test*")).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 2);
    BOOST_CHECK(rs[1][_T("id")] == 1);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    // query reverse
    rs.clear();
    q.reset().where(_T("name"), _T("="), _T("test*")).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 4);
    BOOST_CHECK(rs[1][_T("id")] == 1);
    BOOST_CHECK(rs[2][_T("id")] == 2);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("name"), _T("="), _T("test*"));
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 2);
    BOOST_CHECK(rs[1][_T("id")] == 1);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    
    // [6] WHERE name not like 'test%'
    // query
    rs.clear();
    q.reset().where(_T("name"), _T("<>"), _T("test*")).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 1);
    BOOST_CHECK(rs[0][_T("id")] == 3);
    // query reverse
    rs.clear();
    q.reset().where(_T("name"), _T("<>"), _T("test*")).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 1);
    BOOST_CHECK(rs[0][_T("id")] == 3);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.when(_T("name"), _T("<>"), _T("test*"));
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 1);
    BOOST_CHECK(rs[0][_T("id")] == 3);
    
    // [7] WHERE id2 IS NULL
    // query
    rs.clear();
    q.reset().whereIsNull(_T("id2")).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 2);
    BOOST_CHECK(rs[0][_T("id")] == 2);
    BOOST_CHECK(rs[1][_T("id")] == 6);
    // query reverse
    rs.clear();
    q.reset().whereIsNull(_T("id2")).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 2);
    BOOST_CHECK(rs[0][_T("id")] == 6);
    BOOST_CHECK(rs[1][_T("id")] == 2);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.whenIsNull(_T("id2"));
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 2);
    BOOST_CHECK(rs[0][_T("id")] == 2);
    BOOST_CHECK(rs[1][_T("id")] == 6);
    
    // [8] WHERE id2 IS NOT NULL
    // query
    rs.clear();
    q.reset().whereIsNotNull(_T("id2")).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    // query reverse
    rs.clear();
    q.reset().whereIsNotNull(_T("id2")).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 5);
    BOOST_CHECK(rs[1][_T("id")] == 4);
    BOOST_CHECK(rs[2][_T("id")] == 3);
    BOOST_CHECK(rs[3][_T("id")] == 1);
    // recordsetQuery
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    rq.reset();
    rq.whenIsNotNull(_T("id2"));
    rs.matchBy(rq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 4);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("id")] == 5);
    
    // [9] WHERE id2 IN (1, 2, 3
    // query
    rs.clear();
    q.reset().in(1, 2, 3).reject(0);
    atv.keyValue((char *)NULL).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1].isInvalidRecord() == true);
    BOOST_CHECK(rs[2][_T("id")] == 3);
    // query reverse
    rs.clear();
    q.reset().in(1, 2, 3).reject(0).direction(table::findBackForword);
    atv.keyValue(100).read(rs, q);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[1].isInvalidRecord() == true);
    BOOST_CHECK(rs[2][_T("id")] == 3);

    // [10] select * from `values` join `groups` on `values`.id2 = `groups`.id;
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    atg.join(rs, q, _T("id2"));
    //rs.dump();
    BOOST_CHECK(rs.size() == 2);
    BOOST_CHECK(rs[0][_T("id2")] == 1);
    BOOST_CHECK(rs[0][_T("group_name")] == _T("Administrators"));
    BOOST_CHECK(rs[1][_T("id2")] == 3);
    BOOST_CHECK(rs[1][_T("group_name")] == _T("Guests"));
    
    // [11] select * from `values` join `groups` on `values`.id2 = `groups`.pri_id and `values`.id3 = `groups`.id;
    atv.index(2);
    atg.index(2);
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    atg.join(rs, q, _T("id2"), _T("id3"));
    atv.index(1);
    atg.index(1);
    //rs.dump();
    BOOST_CHECK(rs.size() == 1);
    BOOST_CHECK(rs[0][_T("id2")] == 1);
    BOOST_CHECK(rs[0][_T("group_name")] == _T("Administrators"));
    
    // [12] select * from `values` left outer join `groups` on `values`.id2 = `groups`.id;
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL).read(rs, q);
    atg.outerJoin(rs, q, _T("id2"));
    //rs.dump();
    BOOST_CHECK(rs.size() == 6);
    BOOST_CHECK(rs[0].isInvalidRecord() == true);
    BOOST_CHECK(rs[1].isInvalidRecord() == true);
    BOOST_CHECK(rs[2][_T("id2")] == 1);
    BOOST_CHECK(rs[2][_T("group_name")] == _T("Administrators"));
    BOOST_CHECK(rs[3][_T("id2")] == 3);
    BOOST_CHECK(rs[3][_T("group_name")] == _T("Guests"));
    BOOST_CHECK(rs[4].isInvalidRecord() == true);
    BOOST_CHECK(rs[5].isInvalidRecord() == true);
    
    // [13] select * from `values` left outer join `groups` on `values`.id2 = `groups`.pri_id and `values`.id3 = `groups`.id;
    atv.index(2);
    atg.index(2);
    rs.clear();
    q.reset().all();
    atv.keyValue((char *)NULL, (char *)NULL).read(rs, q);
    atg.outerJoin(rs, q, _T("id2"), _T("id3"));
    atv.index(1);
    atg.index(1);
    //rs.dump();
    BOOST_CHECK(rs.size() == 6);
    BOOST_CHECK(rs[0].isInvalidRecord() == true);
    BOOST_CHECK(rs[1].isInvalidRecord() == true);
    BOOST_CHECK(rs[2][_T("id2")] == 1);
    BOOST_CHECK(rs[2][_T("group_name")] == _T("Administrators"));
    BOOST_CHECK(rs[3].isInvalidRecord() == true);
    BOOST_CHECK(rs[4].isInvalidRecord() == true);
    BOOST_CHECK(rs[5].isInvalidRecord() == true);

    // [14] select id2 from `values` order by id2;
    rs.clear();
    q.reset().select(_T("id2"));
    atv.keyValue((char *)NULL).read(rs, q);
    rs.orderBy(_T("id2"));
    //rs.dump();
    BOOST_CHECK(rs.size() == 6);
    BOOST_CHECK(rs[0][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[1][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[2][_T("id2")] == 1);
    BOOST_CHECK(rs[3][_T("id2")] == 3);
    BOOST_CHECK(rs[4][_T("id2")] == 4);
    BOOST_CHECK(rs[5][_T("id2")] == 4);
    
    // [15] select id2 from `values` order by id2 DESC;
    rs.clear();
    q.reset().select(_T("id2"));
    atv.keyValue((char *)NULL).read(rs, q);
    sortFields orders;
    orders.add(_T("id2"), false); // DESC
    rs.orderBy(orders);
    //rs.dump();
    BOOST_CHECK(rs.size() == 6);
    BOOST_CHECK(rs[0][_T("id2")] == 4);
    BOOST_CHECK(rs[1][_T("id2")] == 4);
    BOOST_CHECK(rs[2][_T("id2")] == 3);
    BOOST_CHECK(rs[3][_T("id2")] == 1);
    BOOST_CHECK(rs[4][_T("id2")].isNull() == true);
    BOOST_CHECK(rs[5][_T("id2")].isNull() == true);
    
    // [16] select * from users2 group by `group`;
    rs.clear();
    q.reset().all();
    atu.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.keyField(_T("group"));
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 5);
    BOOST_CHECK(rs[0][_T("group")].isNull() == true);
    BOOST_CHECK(rs[1][_T("group")] == 1);
    BOOST_CHECK(rs[2][_T("group")] == 2);
    BOOST_CHECK(rs[3][_T("group")] == 3);
    BOOST_CHECK(rs[4][_T("group")] == 4);
    
    // [17] select * from users2 group by `group`, `class`;
    rs.clear();
    q.reset().all();
    atu.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.keyField(_T("group"), _T("class"));
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 9);
    BOOST_CHECK(rs[0][_T("group")].isNull() == true);
    BOOST_CHECK(rs[0][_T("class")] == 2);
    BOOST_CHECK(rs[1][_T("group")].isNull() == true);
    BOOST_CHECK(rs[1][_T("class")] == 3);
    BOOST_CHECK(rs[2][_T("group")] == 1);
    BOOST_CHECK(rs[2][_T("class")] == 1);
    BOOST_CHECK(rs[3][_T("group")] == 1);
    BOOST_CHECK(rs[3][_T("class")] == 3);
    BOOST_CHECK(rs[4][_T("group")] == 2);
    BOOST_CHECK(rs[4][_T("class")].isNull() == true);
    BOOST_CHECK(rs[5][_T("group")] == 2);
    BOOST_CHECK(rs[5][_T("class")] == 1);
    BOOST_CHECK(rs[6][_T("group")] == 3);
    BOOST_CHECK(rs[6][_T("class")].isNull() == true);
    BOOST_CHECK(rs[7][_T("group")] == 3);
    BOOST_CHECK(rs[7][_T("class")] == 1);
    BOOST_CHECK(rs[8][_T("group")] == 4);
    BOOST_CHECK(rs[8][_T("class")] == 3);
    
    // [18] select `group`, count(*) from users2 group by `group`;
    rs.clear();
    q.reset().all();
    atu.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.keyField(_T("group"));
    gq.addFunction(&countFunc);
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 5);
    BOOST_CHECK(rs[0][_T("group")].isNull() == true);
    BOOST_CHECK(rs[0][_T("row_count")] == 2);
    BOOST_CHECK(rs[1][_T("group")] == 1);
    BOOST_CHECK(rs[1][_T("row_count")] == 2);
    BOOST_CHECK(rs[2][_T("group")] == 2);
    BOOST_CHECK(rs[2][_T("row_count")] == 3);
    BOOST_CHECK(rs[3][_T("group")] == 3);
    BOOST_CHECK(rs[3][_T("row_count")] == 2);
    BOOST_CHECK(rs[4][_T("group")] == 4);
    BOOST_CHECK(rs[4][_T("row_count")] == 2);
    
    // [19] select `group`, `class`, count(*) from users2 group by `group`, `class`;
    rs.clear();
    q.reset().all();
    atu.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.keyField(_T("group"), _T("class"));
    gq.addFunction(&countFunc);
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 9);
    BOOST_CHECK(rs[0][_T("group")].isNull() == true);
    BOOST_CHECK(rs[0][_T("class")] == 2);
    BOOST_CHECK(rs[0][_T("row_count")] == 1);
    BOOST_CHECK(rs[1][_T("group")].isNull() == true);
    BOOST_CHECK(rs[1][_T("class")] == 3);
    BOOST_CHECK(rs[1][_T("row_count")] == 1);
    BOOST_CHECK(rs[2][_T("group")] == 1);
    BOOST_CHECK(rs[2][_T("class")] == 1);
    BOOST_CHECK(rs[2][_T("row_count")] == 1);
    BOOST_CHECK(rs[3][_T("group")] == 1);
    BOOST_CHECK(rs[3][_T("class")] == 3);
    BOOST_CHECK(rs[3][_T("row_count")] == 1);
    BOOST_CHECK(rs[4][_T("group")] == 2);
    BOOST_CHECK(rs[4][_T("class")].isNull() == true);
    BOOST_CHECK(rs[4][_T("row_count")] == 2);
    BOOST_CHECK(rs[5][_T("group")] == 2);
    BOOST_CHECK(rs[5][_T("class")] == 1);
    BOOST_CHECK(rs[5][_T("row_count")] == 1);
    BOOST_CHECK(rs[6][_T("group")] == 3);
    BOOST_CHECK(rs[6][_T("class")].isNull() == true);
    BOOST_CHECK(rs[6][_T("row_count")] == 1);
    BOOST_CHECK(rs[7][_T("group")] == 3);
    BOOST_CHECK(rs[7][_T("class")] == 1);
    BOOST_CHECK(rs[7][_T("row_count")] == 1);
    BOOST_CHECK(rs[8][_T("group")] == 4);
    BOOST_CHECK(rs[8][_T("class")] == 3);
    BOOST_CHECK(rs[8][_T("row_count")] == 2);

    // [20] select count(*) as cnt1, count(score) as cnt2, sum(score) as sum, avg(score) as avg, min(score) as min, max(score) as max from subjectscores;
    rs.clear();
    q.reset().all();
    ats.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.addFunction(&countFunc);
    gq.addFunction(&countField);
    gq.addFunction(&sumField);
    gq.addFunction(&avgField);
    gq.addFunction(&minField);
    gq.addFunction(&maxField);
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 1);
    BOOST_CHECK(rs[0][_T("row_count")] == 13);
    BOOST_CHECK(rs[0][_T("valid_row")] == 12);
    BOOST_CHECK(rs[0][_T("sum")] == 812);
    BOOST_CHECK(rs[0][_T("avg")] == (812/12));
    BOOST_CHECK(rs[0][_T("min")] == 0);
    BOOST_CHECK(rs[0][_T("max")] == 90);
    
    // [21] select count(*) as cnt1, count(score) as cnt2, sum(score) as sum, avg(score) as avg, min(score) as min, max(score) as max from subjectscores group by subject;
    rs.clear();
    q.reset().all();
    ats.keyValue((char *)NULL).read(rs, q);
    gq.reset();
    gq.addFunction(&countFunc);
    gq.addFunction(&countField);
    gq.addFunction(&sumField);
    gq.addFunction(&avgField);
    gq.addFunction(&minField);
    gq.addFunction(&maxField);
    gq.keyField(_T("subject"));
    rs.groupBy(gq);
    //rs.dump();
    BOOST_CHECK(rs.size() == 3);
    BOOST_CHECK(rs[0][_T("subject")] == 1);
    BOOST_CHECK(rs[0][_T("row_count")] == 5);
    BOOST_CHECK(rs[0][_T("valid_row")] == 4);
    BOOST_CHECK(rs[0][_T("sum")] == 260);
    BOOST_CHECK(rs[0][_T("avg")] == (260/4));
    BOOST_CHECK(rs[0][_T("min")] == 50);
    BOOST_CHECK(rs[0][_T("max")] == 80);
    BOOST_CHECK(rs[1][_T("subject")] == 2);
    BOOST_CHECK(rs[1][_T("row_count")] == 3);
    BOOST_CHECK(rs[1][_T("valid_row")] == 3);
    BOOST_CHECK(rs[1][_T("sum")] == 262);
    BOOST_CHECK(rs[1][_T("avg")] == (262/3));
    BOOST_CHECK(rs[1][_T("min")] == 85);
    BOOST_CHECK(rs[1][_T("max")] == 90);
    BOOST_CHECK(rs[2][_T("subject")] == 3);
    BOOST_CHECK(rs[2][_T("row_count")] == 5);
    BOOST_CHECK(rs[2][_T("valid_row")] == 5);
    BOOST_CHECK(rs[2][_T("sum")] == 290);
    BOOST_CHECK(rs[2][_T("avg")] == (290/5));
    BOOST_CHECK(rs[2][_T("min")] == 0);
    BOOST_CHECK(rs[2][_T("max")] == 90);
}

#pragma warning(default : 4996) 

#endif // BZS_TEST_TRDCLENGN_TESTFIELD_H