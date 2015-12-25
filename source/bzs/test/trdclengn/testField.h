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

        int lens[4] = {1, 2, 4, 8};
        _TCHAR buf[50];
        //int
        for (int i=1; i < 5 ; ++i)
        {
            _stprintf_s(buf, _T("int_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_integer, lens[i-1]);
        }
        
        //unsigned int
        for (int i=1; i < 5 ; ++i)
        {
            
            _stprintf_s(buf, _T("uint_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_uinteger, lens[i-1]);
        }

        //myyear
        fd = insertField(def, tableid, ++fieldnum, _T("year"), ft_myyear, 1);
        
        //logical
        fd = insertField(def, tableid, ++fieldnum, _T("logical1"), ft_logical, 1);
        fd = insertField(def, tableid, ++fieldnum, _T("logical2"), ft_logical, 2);
        
        //mydate
        fd = insertField(def, tableid, ++fieldnum, _T("date"), ft_mydate, 3);
 
        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
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

short createTestTableLegacyTime(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 2;
        
        insertTable(def, tableid,  _T("timetest"), g_td_charsetIndex);
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
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
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
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
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

short createTestTableTime(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 2;
        
        insertTable(def, tableid,  _T("timetest"), g_td_charsetIndex);
        short fieldnum = 0;
        fielddef* fd = insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        //time
        fd = insertField(def, tableid, ++fieldnum, _T("time3"), ft_mytime, 3);
        fd = insertField(def, tableid, ++fieldnum, _T("time4"), ft_mytime, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("time5"), ft_mytime, 5);
        fd = insertField(def, tableid, ++fieldnum, _T("time6"), ft_mytime, 6);

        //datetime
        fd = insertField(def, tableid, ++fieldnum, _T("datetime5"), ft_mydatetime, 5);
        fd = insertField(def, tableid, ++fieldnum, _T("datetime6"), ft_mydatetime, 6);
        fd = insertField(def, tableid, ++fieldnum, _T("datetime7"), ft_mydatetime, 7);
        fd = insertField(def, tableid, ++fieldnum, _T("datetime8"), ft_mydatetime, 8);

        //timestamp
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp4"), ft_mytimestamp, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp5"), ft_mytimestamp, 5);
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp6"), ft_mytimestamp, 6);
        fd = insertField(def, tableid, ++fieldnum, _T("timestamp7"), ft_mytimestamp, 7);
 
        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
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


class fixtureFieldStore
{
    mutable database* m_db;
    bool mysql56TimeFormat;
    bool legacyTimeFormat;
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
                btrVersion& v = vs.versions[1];
                mysql56TimeFormat =  v.isMysql56TimeFormat();
                legacyTimeFormat = v.isFullLegacyTimeFormat();
                if (ret == 0)
                {   if (legacyTimeFormat)
                        ret = createTestTableLegacyTime(m_db);
                    else 
                        ret = createTestTableTime(m_db);
                    if (ret == 0)
                        ret = createTestTableHA_OPTION_PACK_RECORD(m_db);
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

    bool isMysql56TimeFormat() const {return mysql56TimeFormat;}
    bool isLegacyTimeFormat() const{return legacyTimeFormat;}
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
void checkIntValue(table_ptr tb)
{
    short fieldnum = 0;
    // read by int64
    BOOST_CHECK(tb->getFVbyt(++fieldnum) == SCHAR_MAX);
    BOOST_CHECK(tb->getFVsht(++fieldnum) == SHRT_MAX);
    BOOST_CHECK(tb->getFVint(++fieldnum) == INT_MAX);
    BOOST_CHECK(tb->getFV64(++fieldnum) == LLONG_MAX);

    BOOST_CHECK(tb->getFVsht(++fieldnum) == UCHAR_MAX - 1);
    BOOST_CHECK(tb->getFVint(++fieldnum) == USHRT_MAX - 1);
    BOOST_CHECK(tb->getFV64(++fieldnum) == UINT_MAX - 1);
    BOOST_CHECK((unsigned __int64)tb->getFV64(++fieldnum) == ULLONG_MAX - 1); 

    BOOST_CHECK(tb->getFVsht(++fieldnum) == 2000); 
    BOOST_CHECK(tb->getFVbyt(++fieldnum) == 254);  //logi1
    BOOST_CHECK(tb->getFVint(++fieldnum) == 65000); //logi2
    myDate d;
    d = TEST_DATE;
    BOOST_CHECK(tb->getFVint(++fieldnum) == d.i); //date

    // read by double
    BOOST_CHECK(tb->getFVdbl(1) == SCHAR_MAX);
    BOOST_CHECK(tb->getFVdbl(4) == LLONG_MAX);
    BOOST_CHECK(tb->getFVdbl(5) == UCHAR_MAX - 1);
    BOOST_CHECK(tb->getFVdbl(8) == ULLONG_MAX - 1);
    BOOST_CHECK(tb->getFVdbl(9) == 2000);
    BOOST_CHECK(tb->getFVdbl(10) == 254);  //logi1
    BOOST_CHECK(tb->getFVdbl(11) == 65000); //logi2
    BOOST_CHECK(tb->getFVdbl(12) == (double)d.i); //date

    //read by string
    _TCHAR buf[50];
    BOOST_CHECK(_tcscmp(tb->getFVstr(1), _ltot(SCHAR_MAX, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(4), _i64tot(LLONG_MAX, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(5), _ultot(UCHAR_MAX - 1, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(8), _ui64tot(ULLONG_MAX - 1, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(9), _ltot(2000, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(10), _ltot(254, buf, 10)) == 0);   //logi1
    BOOST_CHECK(_tcscmp(tb->getFVstr(11), _ltot(65000, buf, 10)) == 0); //logi2
    BOOST_CHECK(_tcscmp(tb->getFVstr(12), TEST_DATE) == 0);             //date

}

void testStoreInt(database* db)
{
    short tableid = 1;
    short fieldnum = 0;
    table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
    tb->clearBuffer();
    tb->setFV(_T("id"), 1);

    tb->setFV(++fieldnum, (short)SCHAR_MAX);
    tb->setFV(++fieldnum, (short)SHRT_MAX);
    tb->setFV(++fieldnum, (int)INT_MAX);
    tb->setFV(++fieldnum, (__int64)LLONG_MAX);

    tb->setFV(++fieldnum, (short)UCHAR_MAX - 1);
    tb->setFV(++fieldnum, (int)USHRT_MAX - 1);
    tb->setFV(++fieldnum, (__int64)UINT_MAX - 1);
    tb->setFV(++fieldnum, (__int64)ULLONG_MAX - 1); 
    tb->setFV(++fieldnum, 2000); 
    tb->setFV(++fieldnum, 254);   //logi1
    tb->setFV(++fieldnum, (int)65000); //logi2
    myDate d; d = TEST_DATE;
    tb->setFV(++fieldnum, d.i);
    tb->insert();

    tb->clearBuffer();
    tb->setFV(_T("id"), 1);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);

    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    _TCHAR buf[50];
    fieldnum = 0;
    tb->setFV(++fieldnum, _ltot(SCHAR_MAX, buf, 10));
    tb->setFV(++fieldnum, _ltot(SHRT_MAX, buf, 10));
    tb->setFV(++fieldnum, _ltot(INT_MAX, buf, 10));
    tb->setFV(++fieldnum, _i64tot(LLONG_MAX, buf, 10));

    tb->setFV(++fieldnum, _ltot(UCHAR_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(USHRT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ultot(UINT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ui64tot(ULLONG_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(2000, buf, 10));
    tb->setFV(++fieldnum, _ltot(254, buf, 10));  //logi1
    tb->setFV(++fieldnum, _ltot(65000, buf, 10)); //logi2
    tb->setFV(++fieldnum, TEST_DATE); 
    tb->insert();
        
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);

    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->update();
    BOOST_CHECK(tb->stat() == 0);

    char buf2[50];
    std::string values;
    values += "1\t";
    values += ltoa(SCHAR_MAX, buf2, 10);
    values += "\t2\t";
    values += ltoa(SHRT_MAX, buf2, 10);
    values += "\t3\t";
    values += ltoa(INT_MAX, buf2, 10);
    values += "\t4\t";
    values += _i64toa(LLONG_MAX, buf2, 10);
    values += "\t5\t";
    values += ltoa(UCHAR_MAX - 1, buf2, 10);
    values += "\t6\t";
    values += ltoa(USHRT_MAX - 1, buf2, 10);
    values += "\t7\t";
    values += _i64toa(UINT_MAX - 1, buf2, 10);
    values += "\t8\t";
    values += _ui64toa(ULLONG_MAX - 1, buf2, 10);
    values += "\t9\t";
    values += ltoa(2000, buf2, 10);
    values += "\t10\t";
    values += ltoa(254, buf2, 10);
    values += "\t11\t";
    values += ltoa(65000, buf2, 10);
    values += "\t12\t";
    values += TEST_DATEA;

    tb->test_store(values.c_str());
    BOOST_CHECK(tb->stat() == 0);
    tb->clearBuffer();
    tb->setFV(_T("id"), 2);
    tb->seek();
    BOOST_CHECK(tb->stat() == 0);
    checkIntValue(tb);
}

#define TEST_TIME0 _T("23:41:12")
#define TEST_TIME0A "23:41:12"

#define TEST_DATETIME0 _T("2015-11-09 12:31:56")
#define TEST_DATETIME0A "2015-11-09 12:31:56"

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
    ++fieldnum;
    // read by double
    fieldnum = 0;
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)t.i64);
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)dt.i64);
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == (double)ts.i64);
    ++fieldnum;
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

void testStoreTime(database* db)
{

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

#pragma warning(default : 4996) 

#endif // BZS_TEST_TRDCLENGN_TESTFIELD_H