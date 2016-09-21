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
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>
#include <bzs/db/protocol/tdap/client/stringConverter.h>
#include <limits.h>
#include <stdlib.h>

#define DBNAMEV3 _T("test_v3")

const char* sql = "CREATE TABLE `setenumbit` ("
  "`id` int(11) NOT NULL AUTO_INCREMENT,"
  "`set5` set('A','B','C','D','E') DEFAULT '',"
  "`set64` set('a0', 'a1', 'a2', 'a3', 'a4', 'a5', 'a6', 'a7', 'a8', 'a9', 'b0', 'b1', 'b2', 'b3', 'b4', 'b5', 'b6', 'b7', 'b8', 'b9', 'c0', 'c1', 'c2', 'c3', 'c4', 'c5', 'c6', 'c7', 'c8', 'c9', 'd0', 'd1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7', 'd8', 'd9', 'e0', 'e1', 'e2', 'e3', 'e4', 'e5', 'e6', 'e7', 'e8', 'e9', 'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'g0', 'g1', 'g2', 'g3') DEFAULT '',"
  "`enum2` enum('Y','N') DEFAULT 'N', "
  "`enum260` enum('a0', 'a1', 'a2', 'a3', 'a4', 'a5', 'a6', 'a7', 'a8', 'a9', 'b0', 'b1', 'b2', 'b3', 'b4', 'b5', 'b6', 'b7', 'b8', 'b9', 'c0', 'c1', 'c2', 'c3', 'c4', 'c5', 'c6', 'c7', 'c8', 'c9', 'd0', 'd1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7', 'd8', 'd9', 'e0', 'e1', 'e2', 'e3', 'e4', 'e5', 'e6', 'e7', 'e8', 'e9', 'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'g0', 'g1', 'g2', 'g3', 'g4', 'g5', 'g6', 'g7', 'g8', 'g9', 'h0', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'h7', 'h8', 'h9', 'i0', 'i1', 'i2', 'i3', 'i4', 'i5', 'i6', 'i7', 'i8', 'i9', 'j0', 'j1', 'j2', 'j3', 'j4', 'j5', 'j6', 'j7', 'j8', 'j9', 'k0', 'k1', 'k2', 'k3', 'k4', 'k5', 'k6', 'k7', 'k8', 'k9', 'l0', 'l1', 'l2', 'l3', 'l4', 'l5', 'l6', 'l7', 'l8', 'l9', 'm0', 'm1', 'm2', 'm3', 'm4', 'm5', 'm6', 'm7', 'm8', 'm9', 'n0', 'n1', 'n2', 'n3', 'n4', 'n5', 'n6', 'n7', 'n8', 'n9', 'o0', 'o1', 'o2', 'o3', 'o4', 'o5', 'o6', 'o7', 'o8', 'o9', 'p0', 'p1', 'p2', 'p3', 'p4', 'p5', 'p6', 'p7', 'p8', 'p9', 'q0', 'q1', 'q2', 'q3', 'q4', 'q5', 'q6', 'q7', 'q8', 'q9', 'r0', 'r1', 'r2', 'r3', 'r4', 'r5', 'r6', 'r7', 'r8', 'r9', 's0', 's1', 's2', 's3', 's4', 's5', 's6', 's7', 's8', 's9', 't0', 't1', 't2', 't3', 't4', 't5', 't6', 't7', 't8', 't9', 'u0', 'u1', 'u2', 'u3', 'u4', 'u5', 'u6', 'u7', 'u8', 'u9', 'v0', 'v1', 'v2', 'v3', 'v4', 'v5', 'v6', 'v7', 'v8', 'v9', 'w0', 'w1', 'w2', 'w3', 'w4', 'w5', 'w6', 'w7', 'w8', 'w9', 'x0', 'x1', 'x2', 'x3', 'x4', 'x5', 'x6', 'x7', 'x8', 'x9', 'y0', 'y1', 'y2', 'y3', 'y4', 'y5', 'y6', 'y7', 'y8', 'y9', 'z0', 'z1', 'z2', 'z3', 'z4', 'z5', 'z6', 'z7', 'z8', 'z9') DEFAULT 'a0',"
  "`bit1` bit(1) DEFAULT b'0',"
  "`bit8` bit(8) DEFAULT b'0',"
  "`bit32` bit(32) DEFAULT b'0',"
  "`bit64` bit(64) DEFAULT b'0',"
  "PRIMARY KEY (`id`) "
") ENGINE=InnoDB DEFAULT CHARSET=utf8;"; 


const char* test_records = "INSERT INTO `setenumbit` (`id`, `set5`, `set64`, `enum2`, `enum260`, `bit1`, `bit8`, `bit32`, `bit64`) VALUES"
  "(1, 'A', 'a0', 'N', 'a0', b'1', b'1', b'1', b'1'),"
  "(2, 'A,B,C,D,E', 'a0,g3', 'Y', 'z9', b'1', b'11111111', b'11111111111111111111111111111111', b'1111111111111111111111111111111111111111111111111111111111111111'),"
  "(3, '', '', '0', '0', b'0', b'00000000', b'00000000', b'00000000'),"
  "(4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);";

const char* test_view = "create view idlessthan5 as select * from scores where id < 5";

short createFieldStoreDataBase(database* db)
{
    db->create(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
    if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
    {
        db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
        if (db->stat()) return db->stat();
        db->drop();
        if (db->stat()) return db->stat();
        db->create(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
    }
    if (db->stat()) return db->stat();
    return 0;
}

short createTestTable1(database* db)
{
    try
    {
        openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
        dbdef* def = db->dbDef();
        short tableid = 1;
        
        insertTable(def, tableid,  _T("fieldtest"), g_td_charsetIndex);
        short fieldnum = 0;
        fielddef* fd;
        insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        int lens[5] = {1, 2, 3, 4, 8};
        _TCHAR buf[50];
        //int
        for (int i=1; i < 6 ; ++i)
        {
            _stprintf_s(buf, 50, _T("int_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_integer, lens[i-1]);
            if (i==4)
                fd->setDefaultValue((__int64)INT_MIN);
            if (i==5)
                fd->setDefaultValue(LLONG_MIN);

        }

        //unsigned int
        for (int i=1; i < 6 ; ++i)
        {
            _stprintf_s(buf, 50, _T("uint_%d_byte"), lens[i-1]);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_uinteger, lens[i-1]);
            if (i==4)
                fd->setDefaultValue((__int64)UINT_MAX);
            if (i==5)
                fd->setDefaultValue((__int64)ULLONG_MAX);
        }

        //myyear
        insertField(def, tableid, ++fieldnum, _T("year"), ft_myyear, 1);

        //logical
        insertField(def, tableid, ++fieldnum, _T("logical1"), ft_logical, 1);
        insertField(def, tableid, ++fieldnum, _T("logical2"), ft_logical, 2);

        //mydate
        insertField(def, tableid, ++fieldnum, _T("date"), ft_mydate, 3);
 
        //double
        insertField(def, tableid, ++fieldnum, _T("double4.0"), ft_float, 4);
        fd = insertField(def, tableid, ++fieldnum, _T("double4.4"), ft_float, 4);
        fd->decimals = 4;

        insertField(def, tableid, ++fieldnum, _T("double8.0"), ft_float, 8);
        fd = insertField(def, tableid, ++fieldnum, _T("double8.5"), ft_float, 8);
        fd->decimals = 15;

        //decimal
        /*for (int i=1; i < 66 ; ++i)
        {
            _stprintf_s(buf, 50, _T("dec_%d_digits"), i);
            fd = insertField(def, tableid, ++fieldnum, buf, ft_mydecimal, 0);
            fd->setLenByDecimal(i,  (i-1) % 30);
        }*/
        fd = insertField(def, tableid, ++fieldnum, _T("dec1"), ft_mydecimal, 1);
        fd->setDecimalDigits(15, 5);

        fd = insertField(def, tableid, ++fieldnum, _T("dec2"), ft_mydecimal, 1);
        fd->setDecimalDigits(65, 30);

        fd = insertField(def, tableid, ++fieldnum, _T("dec3"), ft_mydecimal, 1);
        fd->setDecimalDigits(1, 0);

        fd = insertField(def, tableid, ++fieldnum, _T("dec4"), ft_mydecimal, 1);
        fd->setDecimalDigits(11, 11);

        fd = insertField(def, tableid, ++fieldnum, _T("dec5"), ft_mydecimal, 1);
        fd->setDecimalDigits(15, 5);

        fd = insertField(def, tableid, ++fieldnum, _T("dec6"), ft_mydecimal, 1);
        fd->setDecimalDigits(65, 30);

        fd = insertField(def, tableid, ++fieldnum, _T("dec7"), ft_mydecimal, 1);
        fd->setDecimalDigits(1, 0);

        fd = insertField(def, tableid, ++fieldnum, _T("dec8"), ft_mydecimal, 1);
        fd->setDecimalDigits(11, 11);

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
        insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        //time
        insertField(def, tableid, ++fieldnum, _T("time"), ft_mytime, 3);

        //datetime
        insertField(def, tableid, ++fieldnum, _T("datetime"), ft_mydatetime, 8);

        //timestamp
        insertField(def, tableid, ++fieldnum, _T("timestamp"), ft_mytimestamp, 4);

 
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
        insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);

        //time
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("time5"), ft_mytime, 5);
        fd->decimals = 3;
        insertField(def, tableid, ++fieldnum, _T("time3"), ft_mytime, 3);
        fd = insertField(def, tableid, ++fieldnum, _T("time4"), ft_mytime, 4);
        fd->decimals = 2;
        fd = insertField(def, tableid, ++fieldnum, _T("time6"), ft_mytime, 6);
        fd->decimals = 6;

        //datetime
        fd = insertField(def, tableid, ++fieldnum, _T("datetime7"), ft_mydatetime, 7);
        fd->decimals = 3;
        insertField(def, tableid, ++fieldnum, _T("datetime5"), ft_mydatetime, 5);
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
        insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);

        //time
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("id2"), ft_integer, 4);
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
        insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("id2"), ft_integer, 4);
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
        insertField(def, tableid, fieldnum, _T("pri_id"), ft_autoinc, 4);
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("id"), ft_integer, 4);
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
        insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("group"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("class"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("name"), ft_myvarchar, 151);
        fd->setLenByCharnum(50);
        fd = insertField(def, tableid, ++fieldnum, _T("blob"), ft_myblob, 9);
        fd->setCharsetIndex(CHARSET_BIN);
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
        insertField(def, tableid, fieldnum, _T("id"), ft_autoinc, 4);
        insertField(def, tableid, ++fieldnum, _T("subject"), ft_integer, 4);
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("score"), ft_integer, 4);
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

#define exec_sql createTable //exec sql for no result recordset. like a createTable.

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
                        m_db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
                    if (m_db->stat() == 0)
                    {
                        m_db->createTable(sql); //  This table is not listed in test.bdf
                        if (m_db->stat() == 0)
                            m_db->exec_sql(test_records);
                    }
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

#define DOUBLE_V1 (double)-12345678.0
#define DOUBLE_V2 (double)0.1234567890123

#define DEC_V1 (double)1234567890.12345
#define DEC_V1LL 1234567890LL

#define DEC_V2 (double)5.0000010000500001
#define DEC_V2LL 5LL
#define DEC_V2SA "5.000001000050000100000000000000"
#define DEC_V2S _T(DEC_V2SA)

#define DEC_V3 (double)3.0
#define DEC_V3LL 3LL

#define DEC_V4 (double)0.23456789010
#define DEC_V4LL 0LL

#define DEC_V5 (double)-1234567890.12345
#define DEC_V5LL -1234567890LL

#define DEC_V6 (double)-5.0000010000500001
#define DEC_V6LL -5LL
#define DEC_V6SA "-5.000001000050000100000000000000"
#define DEC_V6S _T(DEC_V6SA)

#define DEC_V7 (double)-3.0
#define DEC_V7LL -3LL

#define DEC_V8 (double)-0.23456789010
#define DEC_V8LL -0LL

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
    ++fieldnum;
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V1LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V2LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V3LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V4LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V5LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V6LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V7LL); //decimal
    BOOST_CHECK(tb->getFV64(++fieldnum) == DEC_V8LL); //decimal

    
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
    if (DOUBLE_V2 > v)
        v = DOUBLE_V2 - v;
    else
        v = v - DOUBLE_V2;

    BOOST_CHECK(v < 0.000000001f);
    
    //decimal
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V1); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V2); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V3); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V4); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V5); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V6); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V7); 
    BOOST_CHECK(tb->getFVdbl(++fieldnum) == DEC_V8); 

    //read by string
    _TCHAR buf[50];
    BOOST_CHECK(_tcscmp(tb->getFVstr(1), _ltot(SCHAR_MAX, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(3), _ltot(MINT_MIN, buf, 10)) == 0);
    _i64tot_s(LLONG_MAX, buf, 50, 10);
    BOOST_CHECK(_tcscmp(tb->getFVstr(5), buf) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(6), _ultot(UCHAR_MAX - 1, buf, 10)) == 0);
    BOOST_CHECK(_tcscmp(tb->getFVstr(8), _ultot(UMINT_MAX - 1, buf, 10)) == 0);
    fieldnum = 9;
    _ui64tot_s(ULLONG_MAX - 1, buf, 50, 10);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), buf) == 0);
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

    _stprintf(tmp, _T("%0.5lf"), DEC_V1);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%s"), DEC_V2S);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%0.0lf"), DEC_V3);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%0.11lf"), DEC_V4);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal

    _stprintf(tmp, _T("%0.5lf"), DEC_V5);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%s"), DEC_V6S);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%0.0lf"), DEC_V7);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal
    _stprintf(tmp, _T("%0.11lf"), DEC_V8);
    BOOST_CHECK(_tcscmp(tb->getFVstr(++fieldnum), tmp) == 0); //decimal

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

    BOOST_CHECK(tb->getFVint(++fieldnum) == INT_MIN);// check default value
    tb->setFV(fieldnum, (int)INT_MAX);
    BOOST_CHECK(tb->getFV64(++fieldnum) == LLONG_MIN);// check default value
    tb->setFV(fieldnum, (__int64)LLONG_MAX);

    tb->setFV(++fieldnum, (short)(UCHAR_MAX - 1));
    tb->setFV(++fieldnum, (int)(USHRT_MAX - 1));
    tb->setFV(++fieldnum, (int)(UMINT_MAX - 1));
    BOOST_CHECK(tb->getFV64(++fieldnum) == UINT_MAX);// check default value
    tb->setFV(fieldnum, (__int64)(UINT_MAX - 1));
    BOOST_CHECK((unsigned __int64)tb->getFV64(++fieldnum) == ULLONG_MAX);// check default value
    tb->setFV(fieldnum, (__int64)(ULLONG_MAX - 1)); 
    tb->setFV(++fieldnum, 2000); 
    tb->setFV(++fieldnum, 254);   //logi1
    tb->setFV(++fieldnum, (int)65000); //logi2
    myDate d; d = TEST_DATE;
    tb->setFV(++fieldnum, d.i);
    tb->setFV(++fieldnum, (int)FLOAT_V1);
    tb->setFV(++fieldnum, FLOAT_V2);
    tb->setFV(++fieldnum, (__int64)DOUBLE_V1);
    tb->setFV(++fieldnum, DOUBLE_V2);
    tb->setFV(++fieldnum, DEC_V1);
    tb->setFV(++fieldnum, DEC_V2S);
    tb->setFV(++fieldnum, DEC_V3);
    tb->setFV(++fieldnum, DEC_V4);
    tb->setFV(++fieldnum, DEC_V5);
    tb->setFV(++fieldnum, DEC_V6S);
    tb->setFV(++fieldnum, DEC_V7);
    tb->setFV(++fieldnum, DEC_V8);
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
    _i64tot_s(LLONG_MAX, buf, 50, 10);
    tb->setFV(++fieldnum, buf);

    tb->setFV(++fieldnum, _ltot(UCHAR_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(USHRT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ltot(UMINT_MAX - 1, buf, 10));
    tb->setFV(++fieldnum, _ultot(UINT_MAX - 1, buf, 10));
    _ui64tot_s(ULLONG_MAX - 1, buf, 50, 10);
    tb->setFV(++fieldnum, buf);
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
    _stprintf(buf, _T("%.5lf"), DEC_V1);
    tb->setFV(++fieldnum, buf);
#if (__BCPLUSPLUS__)
    tb->setFV(++fieldnum, DEC_V2S);
#else
    _stprintf(buf, _T("%.16lf"), DEC_V2);
    tb->setFV(++fieldnum, buf);
#endif
    _stprintf(buf, _T("%.1lf"), DEC_V3);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.11lf"), DEC_V4);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.5lf"), DEC_V5);
    tb->setFV(++fieldnum, buf);
#if (__BCPLUSPLUS__)
    tb->setFV(++fieldnum, DEC_V6S);
#else
    _stprintf(buf, _T("%.16lf"), DEC_V6);
    tb->setFV(++fieldnum, buf);
#endif
    _stprintf(buf, _T("%.1lf"), DEC_V7);
    tb->setFV(++fieldnum, buf);
    _stprintf(buf, _T("%.11lf"), DEC_V8);
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
    tb->setFV(++fieldnum, DEC_V1);
    tb->setFV(++fieldnum, DEC_V2S);
    tb->setFV(++fieldnum, DEC_V3);
    tb->setFV(++fieldnum, DEC_V4);
    tb->setFV(++fieldnum, DEC_V5);
    tb->setFV(++fieldnum, DEC_V6S);
    tb->setFV(++fieldnum, DEC_V7);
    tb->setFV(++fieldnum, DEC_V8);
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
    _i64toa_s(LLONG_MAX, buf2, 50, 10);
    values += buf2;
    values += "\t6\t";
    values += _ltoa(UCHAR_MAX - 1, buf2, 10);
    values += "\t7\t";
    values += _ltoa(USHRT_MAX - 1, buf2, 10);
    values += "\t8\t";
    values += _ltoa(UMINT_MAX - 1, buf2, 10);
    values += "\t9\t";
    _i64toa_s(UINT_MAX - 1, buf2, 50, 10);
    values += buf2;
    values += "\t10\t";
    _ui64toa_s(ULLONG_MAX - 1, buf2, 50, 10);
    values += buf2;
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
    values += "\t19\t";sprintf_s(buf2, 50, "%.5lf", DEC_V1);
    values += buf2;
    values += "\t20\t";
    values += DEC_V2SA;
    values += "\t21\t";sprintf_s(buf2, 50, "%.1lf", DEC_V3);
    values += buf2;
    values += "\t22\t";sprintf_s(buf2, 50, "%.11lf", DEC_V4);
    values += buf2;
    values += "\t23\t";sprintf_s(buf2, 50, "%.5lf", DEC_V5);
    values += buf2;
    values += "\t24\t";
    values += DEC_V6SA;
    values += "\t25\t";sprintf_s(buf2, 50, "%.1lf", DEC_V7);
    values += buf2;
    values += "\t26\t";sprintf_s(buf2, 50, "%.11lf", DEC_V8);
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
    _TCHAR tmp[70];
    myTimeStamp ts_auto(0, false);
    ts_auto.i64 = newValue;
    BOOST_CHECK(_tcscmp(btrdtoa(getNowDate(),(_TCHAR*)NULL, true), ts_auto.dateStr(tmp, 70)) == 0);

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

#define SEB_DB_TABLE_NAME _T("setenumbit")

void testSetEnumBit()
{
    database* db = database::create();
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3));
    
    activeTable ats(db, SEB_DB_TABLE_NAME);
    ats.index(0);
    recordset rs;
    query q;
    q.reset().all();
    ats.keyValue((char *)NULL).read(rs, q);
#ifdef _DEBUG
    //rs.dump();
#endif

    BOOST_CHECK(rs.size() == 4);
    
    BOOST_CHECK(rs[0][_T("id")] == 1);
    BOOST_CHECK(rs[0][_T("set5")].i64() == 1);
    BOOST_CHECK(rs[0][_T("set64")].i64() == 1);
    BOOST_CHECK(rs[0][_T("enum2")].i64() == 2);
    BOOST_CHECK(rs[0][_T("enum260")].i64() == 1);
    BOOST_CHECK(rs[0][_T("bit1")].i64() == 1);
    BOOST_CHECK(rs[0][_T("bit8")].i64() == 1);
    BOOST_CHECK(rs[0][_T("bit32")].i64() == 1);
    BOOST_CHECK(rs[0][_T("bit64")].i64() == 1);

    BOOST_CHECK(rs[1][_T("id")] == 2);
    BOOST_CHECK(rs[1][_T("set5")].i64() == 31);
    BOOST_CHECK(rs[1][_T("set64")].i64() == (__int64)0x8000000000000001);
    BOOST_CHECK(rs[1][_T("enum2")].i64() == 1);
    BOOST_CHECK(rs[1][_T("enum260")].i64() == 260);
    BOOST_CHECK(rs[1][_T("bit1")].i64() == 1);
    BOOST_CHECK(rs[1][_T("bit8")].i64() == 0xFF);
    BOOST_CHECK(rs[1][_T("bit32")].i64() == 0xFFFFFFFF);
    BOOST_CHECK(rs[1][_T("bit64")].i64() == (__int64)0xFFFFFFFFFFFFFFFF);

    BOOST_CHECK(rs[2][_T("id")] == 3);
    BOOST_CHECK(rs[2][_T("set5")].i64() == 0);
    BOOST_CHECK(rs[2][_T("set64")].i64() == 0);
    BOOST_CHECK(rs[2][_T("enum2")].i64() == 0);
    BOOST_CHECK(rs[2][_T("enum260")].i64() == 0);
    BOOST_CHECK(rs[2][_T("bit1")].i64() == 0);
    BOOST_CHECK(rs[2][_T("bit8")].i64() == 0);
    BOOST_CHECK(rs[2][_T("bit32")].i64() == 0);
    BOOST_CHECK(rs[2][_T("bit64")].i64() == 0);

    BOOST_CHECK(rs[3][_T("id")] == 4);
    BOOST_CHECK(rs[3][_T("set5")].isNull() == true);
    BOOST_CHECK(rs[3][_T("set64")].isNull() == true);
    BOOST_CHECK(rs[3][_T("enum2")].isNull() == true);
    BOOST_CHECK(rs[3][_T("enum260")].isNull() == true);
    BOOST_CHECK(rs[3][_T("bit1")].isNull() == true);
    BOOST_CHECK(rs[3][_T("bit8")].isNull() == true);
    BOOST_CHECK(rs[3][_T("bit32")].isNull() == true);
    BOOST_CHECK(rs[3][_T("bit64")].isNull() == true);
    db->release();
}

#include <bzs/db/protocol/tdap/fieldComp.h>
void testCompInt()
{
    for (ushort_td len = 1; len < 6; ++len)
    {
        if (len == 5) len = 8;
        comp1Func compFunc = getCompFunc(ft_integer, len, eEqual, 0);
        __int64 l; __int64 r;
        l = 0; r = 1; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = 0; r = 0; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 1; r = 0; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = -1;r = 0; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = -1;r = -1; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = -1;r = -2; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
    }
}

void testCompUint()
{
    for (ushort_td len = 1; len < 6; ++len)
    {
        if (len == 5) len = 8;
        comp1Func compFunc = getCompFunc(ft_uinteger, len, eEqual, 0);
        unsigned __int64 l; unsigned __int64 r;
        l = 0; r = ULLONG_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = 0; r = 0;          BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 1; r = 0;          BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = ULLONG_MAX;  r = 0;  BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = ULLONG_MAX;  r = ULLONG_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = ULLONG_MAX-1;r = ULLONG_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
    }
}
void testCompDouble()
{
    ushort_td len = 4;
    comp1Func compFunc = getCompFunc(ft_float, len, eEqual, 0);
    {
        float l; float r;
        l = (float)-0.2; r = 0;    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = (float)-0.2; r = (float)-0.2; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = (float)0.2; r = 0;    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
    }
    {
        double l; double r;
        len = 8;
        compFunc = getCompFunc(ft_float, len, eEqual, 0);
        l = -0.000000000000002; r = 0;    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = -0.000000000000002; r = -0.000000000000002; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 0.000000000000002; r = 0;    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
    }
}

void testCompBit()
{
    for (ushort_td len = 1; len < 8; ++len)
    {
        // equal comp
        comp1Func compFunc = getCompFunc(ft_bit, len, eEqual, 0);
        unsigned __int64 l; unsigned __int64 r;

        l = 0; r = 0xF0; l = changeEndian(l, len);r = changeEndian(r, len);
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);

        l = 0; r = 0; l = changeEndian(l, len);r = changeEndian(r, len);          
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        
        l = 1; r = 0; l = changeEndian(l, len);r = changeEndian(r, len);          
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        
        l = 0xF0;  r = 0; l = changeEndian(l, len);r = changeEndian(r, len);  
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        
        l = 0xF0;  r = 0xF0; l = changeEndian(l, len);r = changeEndian(r, len);
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        
        l = 0xF0-1;r = 0xF0; l = changeEndian(l, len);r = changeEndian(r, len);
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);

        // bit comp
        compFunc = getCompFunc(ft_bit, len, eBitAnd, 0);
        l = 0xFF; r = 0xF0; l = changeEndian(l, len);r = changeEndian(r, len);
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 0xF0; r = 0xF; l = changeEndian(l, len);r = changeEndian(r, len);
        int v = compFunc((const char*)&l, (const char*)&r, len);
        BOOST_CHECK(v > 0);

        compFunc = getCompFunc(ft_bit, len, eNotBitAnd, 0);
        l = 0xFF; r = 0xF0; l = changeEndian(l, len);r = changeEndian(r, len);
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
    }
}

void testCompSet()
{
    for (ushort_td len = 1; len < 5; ++len)
    {
        if (len == 5) len = 8;
        // equal comp
        comp1Func compFunc = getCompFunc(ft_set, len, eEqual, 0);
        unsigned __int64 l; unsigned __int64 r;
        l = 0; r = 0xF0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = 0; r = 0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 1; r = 0; 
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = 0xF0;  r = 0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = 0xF0;  r = 0xF0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 0xF0-1;r = 0xF0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);

        // bit comp
        compFunc = getCompFunc(ft_set, len, eBitAnd, 0);
        l = 0xFF; r = 0xF0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 0xF0; r = 0xF;
        int v = compFunc((const char*)&l, (const char*)&r, len);
        BOOST_CHECK(v > 0);
        compFunc = getCompFunc(ft_set, len, eNotBitAnd, 0);
        l = 0xFF; r = 0xF0;
        BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
    }
}

void testCompEnum()
{
    for (ushort_td len = 1; len < 3; ++len)
    {
        comp1Func compFunc = getCompFunc(ft_enum, len, eEqual, 0);
        unsigned short l; unsigned short r;
        l = 0; r = USHRT_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
        l = 0; r = 0;          BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = 1; r = 0;          BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = USHRT_MAX;  r = 0;  BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        l = USHRT_MAX;  r = USHRT_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
        l = USHRT_MAX-1;r = USHRT_MAX; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
    }
}

void testCompYear()
{
    ushort_td len = 1;
    comp1Func compFunc = getCompFunc(ft_myyear, len, eEqual, 0);
    unsigned char l; unsigned char r;
    l = 0; r = 0XFF; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
    l = 0; r = 0; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
    l = 0XFF; r = 0; BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
}

void testCompDate()
{
    ushort_td len = 3;
    comp1Func compFunc = getCompFunc(ft_mydate, len, eEqual, 0);
    myDate ld, rd;
    int l, r;

    ld = _T("1900-01-01"); rd = _T("1900-01-02"); l = ld.getValue(); r = rd.getValue();
    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
    ld = _T("1900-01-01"); rd = _T("1900-01-01"); l = ld.getValue(); r = rd.getValue();
    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
    ld = _T("1900-01-02"); rd = _T("1900-01-01"); l = ld.getValue(); r = rd.getValue();
    BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
    
}

void testCompTime()
{
    __int64 l, r;
    {
        ushort_td len = 3;
        {
            comp1Func compFunc = getCompFunc(ft_mytime_num_cmp, len, eEqual, 0);
            myTime ld(0, false), rd(0, false);
            ld = _T("00:00:59"); rd = _T("16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:01"); rd = _T("00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("16:59:00"); rd = _T("00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(0, true), rd(0, true);
            ld = _T("00:00:59"); rd = _T("16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:01"); rd = _T("00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("16:59:00"); rd = _T("00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 4;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(1, true), rd(1, true);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(2, true), rd(2, true);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(3, true), rd(3, true);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(4, true), rd(4, true);
            ld = _T("00:00:59.0000"); rd = _T("00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.0001"); rd = _T("00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.0001"); rd = _T("00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(5, true), rd(5, true);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            myTime ld(6, true), rd(6, true);
            ld = _T("01:00:59.999999"); rd = _T("10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.000001"); rd = _T("00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("10:00:59.000000"); rd = _T("10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void testCompDateTime()
{
    __int64 l, r;
    {
        ushort_td len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(0, true), rd(0, true);
            ld = _T("1900-01-02 00:00:59"); rd = _T("1900-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:01"); rd = _T("1900-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 16:59:00"); rd = _T("1900-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(1, true), rd(1, true);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(2, true), rd(2, true);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 7;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(3, true), rd(3, true);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(4, true), rd(4, true);
            ld = _T("1900-01-02 00:00:59.0000"); rd = _T("1900-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.0001"); rd = _T("1900-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.0001"); rd = _T("1900-01-02 00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 8;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(5, true), rd(5, true);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            myDateTime ld(6, true), rd(6, true);
            ld = _T("1900-01-02 01:00:59.999999"); rd = _T("1900-01-02 10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.000001"); rd = _T("1900-01-02 00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 10:00:59.000000"); rd = _T("1900-01-02 10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime_num_cmp, len, eEqual, 0);
            myDateTime ld(0, false), rd(0, false);
            ld = _T("1900-01-02 00:00:59"); rd = _T("1900-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:01"); rd = _T("1900-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 16:59:00"); rd = _T("1900-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void testCompTimeStamp()
{
    __int64 l, r;
    {
        ushort_td len = 4;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(0, true), rd(0, true);
            ld = _T("1970-01-02 00:00:59"); rd = _T("1970-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:01"); rd = _T("1970-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 16:59:00"); rd = _T("1970-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp_num_cmp, len, eEqual, 0);
            myTimeStamp ld(0, false), rd(0, false);
            ld = _T("1970-01-02 00:00:59"); rd = _T("1970-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:01"); rd = _T("1970-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 16:59:00"); rd = _T("1970-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(1, true), rd(1, true);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(2, true), rd(2, true);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(3, true), rd(3, true);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(4, true), rd(4, true);
            ld = _T("1970-01-02 00:00:59.0000"); rd = _T("1970-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.0001"); rd = _T("1970-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.0001"); rd = _T("1970-01-02 00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 7;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(5, true), rd(5, true);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            myTimeStamp ld(6, true), rd(6, true);
            ld = _T("1970-01-02 01:00:59.999999"); rd = _T("1970-01-02 10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.000001"); rd = _T("1970-01-02 00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 10:00:59.000000"); rd = _T("1970-01-02 10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void testCompTimeMa()
{
    __int64 l, r;
    {
        ushort_td len = 3;
        {
            comp1Func compFunc = getCompFunc(ft_mytime_num_cmp, len, eEqual, 0);
            maTime ld(0, false), rd(0, false);
            ld = _T("00:00:59"); rd = _T("16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:01"); rd = _T("00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("16:59:00"); rd = _T("00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(0, true), rd(0, true);
            ld = _T("00:00:59"); rd = _T("16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:01"); rd = _T("00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("16:59:00"); rd = _T("00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 4;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(1, true), rd(1, true);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.1"); rd = _T("00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(2, true), rd(2, true);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.01"); rd = _T("00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(3, true), rd(3, true);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.001"); rd = _T("00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(4, true), rd(4, true);
            ld = _T("00:00:59.0000"); rd = _T("00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.0001"); rd = _T("00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.0001"); rd = _T("00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(5, true), rd(5, true);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("00:00:59.00001"); rd = _T("00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytime, len, eEqual, 0);
            maTime ld(6, true), rd(6, true);
            ld = _T("01:00:59.999999"); rd = _T("10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("00:00:59.000001"); rd = _T("00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("10:00:59.000000"); rd = _T("10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void testCompDateTimeMa()
{
    __int64 l, r;
    {
        ushort_td len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(0, true), rd(0, true);
            ld = _T("1900-01-02 00:00:59"); rd = _T("1900-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:01"); rd = _T("1900-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 16:59:00"); rd = _T("1900-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(1, true), rd(1, true);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.1"); rd = _T("1900-01-02 00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(2, true), rd(2, true);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.01"); rd = _T("1900-01-02 00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 7;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(3, true), rd(3, true);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.001"); rd = _T("1900-01-02 00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(4, true), rd(4, true);
            ld = _T("1900-01-02 00:00:59.0000"); rd = _T("1900-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.0001"); rd = _T("1900-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.0001"); rd = _T("1900-01-02 00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 8;
        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(5, true), rd(5, true);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 00:00:59.00001"); rd = _T("1900-01-02 00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime, len, eEqual, 0);
            maDateTime ld(6, true), rd(6, true);
            ld = _T("1900-01-02 01:00:59.999999"); rd = _T("1900-01-02 10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:59.000001"); rd = _T("1900-01-02 00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 10:00:59.000000"); rd = _T("1900-01-02 10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mydatetime_num_cmp, len, eEqual, 0);
            maDateTime ld(0, false), rd(0, false);
            ld = _T("1900-01-02 00:00:59"); rd = _T("1900-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1900-01-02 00:00:01"); rd = _T("1900-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1900-01-02 16:59:00"); rd = _T("1900-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void testCompTimeStampMa()
{
    __int64 l, r;
    {
        ushort_td len = 4;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(0, true), rd(0, true);
            ld = _T("1970-01-02 00:00:59"); rd = _T("1970-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:01"); rd = _T("1970-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 16:59:00"); rd = _T("1970-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp_num_cmp, len, eEqual, 0);
            maTimeStamp ld(0, false), rd(0, false);
            ld = _T("1970-01-02 00:00:59"); rd = _T("1970-01-02 16:59:00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:01"); rd = _T("1970-01-02 00:00:01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 16:59:00"); rd = _T("1970-01-02 00:00:59"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 5;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(1, true), rd(1, true);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.2"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.1"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.1"); rd = _T("1970-01-02 00:00:59.0"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(2, true), rd(2, true);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.02"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.01"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.01"); rd = _T("1970-01-02 00:00:59.00"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        len = 6;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(3, true), rd(3, true);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) < 0);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.001"); rd = _T("1970-01-02 00:00:59.000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(4, true), rd(4, true);
            ld = _T("1970-01-02 00:00:59.0000"); rd = _T("1970-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == -1);
            ld = _T("1970-01-02 00:00:59.0001"); rd = _T("1970-01-02 00:00:59.0001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.0001"); rd = _T("1970-01-02 00:00:59.0000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
        
        len = 7;
        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(5, true), rd(5, true);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00002"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == -1);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 00:00:59.00001"); rd = _T("1970-01-02 00:00:59.00000"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }

        {
            comp1Func compFunc = getCompFunc(ft_mytimestamp, len, eEqual, 0);
            maTimeStamp ld(6, true), rd(6, true);
            ld = _T("1970-01-02 01:00:59.999999"); rd = _T("1970-01-02 10:00:59.100001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == -1);
            ld = _T("1970-01-02 00:00:59.000001"); rd = _T("1970-01-02 00:00:59.000001"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) == 0);
            ld = _T("1970-01-02 10:00:59.000000"); rd = _T("1970-01-02 10:00:58.999999"); l = ld.getValue(); r = rd.getValue();
            BOOST_CHECK(compFunc((const char*)&l, (const char*)&r, len) > 0);
        }
    }
}

void doTestCompStringOne(const char* lt , const char* rt, int ret, int len,
                    int sizeByte, comp1Func compFunc)
{
    char l[128] = {0x00};
    char r[128] = {0x00};
    strcpy(l + sizeByte, lt); strcpy(r + sizeByte, rt);
    if (sizeByte)
    {
        l[0] = (char)strlen(lt);
        r[0] = (char)len;
    }
    int v = compFunc((const char*)l, (const char*)r, len);
    if (ret > 0)
        BOOST_CHECK( v > 0);
    else if (ret == 0)
        BOOST_CHECK( v == 0);
    else
        BOOST_CHECK( v < 0);
}

void doTestCompString(uchar_td type, int sizeByte)
{
    uchar_td logType = eEqual; 
    ushort_td len = 2;
    comp1Func compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompStringOne("abcc", "ac",  -1, len, sizeByte, compFunc);
    doTestCompStringOne("a", "ac",     -1, len, sizeByte, compFunc);
    doTestCompStringOne("ab99", "abcc", 0, len, sizeByte, compFunc);
    doTestCompStringOne("accc", "ab0",  1, len, sizeByte, compFunc);

    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);

    doTestCompStringOne("Abc", "ac", -1, len, sizeByte, compFunc);
    doTestCompStringOne("Abc", "aBc", 0, len, sizeByte, compFunc);
    doTestCompStringOne("acc", "Abc", 1, len, sizeByte, compFunc);

    logType = eEqual | CMPLOGICAL_VAR_COMP_ALL;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompStringOne("ab", "ac", -1, len, sizeByte, compFunc);
    doTestCompStringOne("ab", "ab", 0, len, sizeByte, compFunc);
    doTestCompStringOne("ac", "ab", 1, len, sizeByte, compFunc);

    
    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompStringOne("Ab", "ac", -1, len, sizeByte, compFunc);
    doTestCompStringOne("Ab", "aB", 0, len, sizeByte, compFunc);
    doTestCompStringOne("ac", "Ab", 1, len, sizeByte, compFunc);

}

void testCompString()
{
    doTestCompString(ft_mychar, 0);
    doTestCompString(ft_string, 0);
    doTestCompString(ft_zstring, 0);
    doTestCompString(ft_note, 0);
    doTestCompString(ft_myvarchar, 1);
    doTestCompString(ft_myvarchar, 2);
    doTestCompString(ft_myvarbinary, 1);
    doTestCompString(ft_myvarbinary, 2);
    doTestCompString(ft_lstring, 1);
    doTestCompString(ft_lstring, 2);
}

#ifdef _WIN32
void doTestCompWStringOne(const wchar_t* lt , const wchar_t* rt, int ret, int len,
                    int sizeByte, comp1Func compFunc)
{
    char l[128] = {0x00};
    char r[128] = {0x00};
    wcscpy((wchar_t*)(l + sizeByte), lt); wcscpy((wchar_t*)(r + sizeByte), rt);
    
    if (sizeByte)
    {
        l[0] = (char)wcslen(lt) * 2;
        r[0] = (char)len;
    }
    int v = compFunc((const char*)l, (const char*)r, len);
    if (ret > 0)
        BOOST_CHECK( v > 0);
    else if (ret == 0)
        BOOST_CHECK( v == 0);
    else
        BOOST_CHECK( v < 0);
}

void doTestCompWString(uchar_td type, int sizeByte)
{
    uchar_td logType = eEqual; 
    ushort_td len = 4;
    comp1Func compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompWStringOne(L"abcc", L"ac",  -1, len, sizeByte, compFunc);
    doTestCompWStringOne(L"a", L"ac",     -1, len, sizeByte, compFunc);
    doTestCompWStringOne(L"ab99", L"abcc", 0, len, sizeByte, compFunc);
    doTestCompWStringOne(L"accc", L"ab0",  1, len, sizeByte, compFunc);

    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);

    doTestCompWStringOne(L"Abc", L"ac", -1, len, sizeByte, compFunc);
    doTestCompWStringOne(L"Abc", L"aBc", 0, len, sizeByte, compFunc);
    doTestCompWStringOne(L"acc", L"Abc", 1, len, sizeByte, compFunc);

    logType = eEqual | CMPLOGICAL_VAR_COMP_ALL;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompWStringOne(L"ab", L"ac", -1, len, sizeByte, compFunc);
    doTestCompWStringOne(L"ab", L"ab", 0, len, sizeByte, compFunc);
    doTestCompWStringOne(L"ac", L"ab", 1, len, sizeByte, compFunc);

    
    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompWStringOne(L"Ab", L"ac", -1, len, sizeByte, compFunc);
    doTestCompWStringOne(L"Ab", L"aB", 0, len, sizeByte, compFunc);
    doTestCompWStringOne(L"ac", L"Ab", 1, len, sizeByte, compFunc);

}

void testCompWString()
{
    doTestCompWString(ft_mywchar, 0);
    doTestCompWString(ft_wstring, 0);
    doTestCompWString(ft_wzstring, 0);
    doTestCompWString(ft_mywvarchar, 1);
    doTestCompWString(ft_mywvarchar, 2);
    doTestCompWString(ft_mywvarbinary, 1);
    doTestCompWString(ft_mywvarbinary, 2);
}

#endif



void doTestCompBlobOne(const char* lt , const char* rt, int ret, int len,
                    int sizeByte, comp1Func compFunc)
{
    char l[128] = {0x00};
    char lb[128] = {0x00};
    char r[128] = {0x00};
    strcpy(lb, lt); strcpy(r + sizeByte, rt);
    l[0] = (char)strlen(lt);
    char** p = (char**)(l + sizeByte);
    *p = lb;
    
    r[0] = (char)len;
    int v = compFunc((const char*)l, (const char*)r, len);
    if (ret > 0)
        BOOST_CHECK( v > 0);
    else if (ret == 0)
        BOOST_CHECK( v == 0);
    else
        BOOST_CHECK( v < 0);
}

void doTestCompBlob(uchar_td type, int sizeByte)
{
    uchar_td logType = eEqual; 
    ushort_td len = 2;
    comp1Func compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompBlobOne("abcc", "ac",  -1, len, sizeByte, compFunc);
    doTestCompBlobOne("a", "ac",     -1, len, sizeByte, compFunc);
    doTestCompBlobOne("ab99", "abcc", 0, len, sizeByte, compFunc);
    doTestCompBlobOne("accc", "ab0",  1, len, sizeByte, compFunc);

    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);

    doTestCompBlobOne("Abc", "ac", -1, len, sizeByte, compFunc);
    doTestCompBlobOne("Abc", "aBc", 0, len, sizeByte, compFunc);
    doTestCompBlobOne("acc", "Abc", 1, len, sizeByte, compFunc);

    logType = eEqual | CMPLOGICAL_VAR_COMP_ALL;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompBlobOne("ab", "ac", -1, len, sizeByte, compFunc);
    doTestCompBlobOne("ab", "ab", 0, len, sizeByte, compFunc);
    doTestCompBlobOne("ac", "ab", 1, len, sizeByte, compFunc);

    
    logType |= CMPLOGICAL_CASEINSENSITIVE;
    compFunc = getCompFunc(type, 0, logType, sizeByte);
    doTestCompBlobOne("Ab", "ac", -1, len, sizeByte, compFunc);
    doTestCompBlobOne("Ab", "aB", 0, len, sizeByte, compFunc);
    doTestCompBlobOne("ac", "Ab", 1, len, sizeByte, compFunc);

}

void testCompBlob()
{
    doTestCompBlob(ft_mytext, 1);
    doTestCompBlob(ft_mytext, 2);
    doTestCompBlob(ft_mytext, 3);
    doTestCompBlob(ft_mytext, 4);
    doTestCompBlob(ft_myblob, 1);
    doTestCompBlob(ft_myblob, 2);
    doTestCompBlob(ft_myblob, 3);
    doTestCompBlob(ft_myblob, 4);
}

void testCompDecimal()
{
    comp1Func compFunc = getCompFunc(ft_mydecimal, 0, 0, 0);
    BOOST_CHECK(compFunc != NULL);

}

void testSnapshotWithbinlog()
{
    nsdatabase::setCheckTablePtr(true);
    database_ptr db = createDatabaseObject();
    openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_READONLY);
    BOOST_CHECK(db->stat() == 0);
    table* tb = db->openTable(1, TD_OPEN_READONLY);
    BOOST_CHECK(db->stat() == 0);

    btrVersions vs;
    db->getBtrVersion(&vs);
    BOOST_CHECK(db->stat() == 0);
    btrVersion ver = vs.versions[1];

    binlogPos bpos;
    db->beginSnapshot(CONSISTENT_READ_WITH_BINLOG_POS, &bpos);
    BOOST_CHECK_MESSAGE(db->stat() == 0, "stat = " << db->stat());
    BOOST_CHECK(strlen(bpos.filename) >= 5);
    BOOST_CHECK(bpos.pos != 0);
    if (ver.isMariaDB() && ver.majorVersion > 5)
    {
        BOOST_CHECK(bpos.type == REPL_POSTYPE_MARIA_GTID);
        BOOST_CHECK(strlen(bpos.gtid) >= 5);
    }
    else if (ver.minorVersion > 5)
    {   //mysql 5.6 5.7
        bool ret = (bpos.type == REPL_POSTYPE_POS || bpos.type == REPL_POSTYPE_GTID);
        BOOST_CHECK(ret);
        if (bpos.type == REPL_POSTYPE_GTID)
            BOOST_CHECK(strlen(bpos.gtid) >= 35);
    }
    else
        BOOST_CHECK(bpos.type == REPL_POSTYPE_POS);
    
    //Test invalid close 
    tb->close();
    BOOST_CHECK(tb->stat() == STATUS_ALREADY_INSNAPSHOT);

    db->endSnapshot();
    db->createTable(test_view);// create view for next test
    BOOST_CHECK(db->stat() == 0);
}


void testConnMgr()
{
    nsdatabase::setCheckTablePtr(true);
    database_ptr db = createDatabaseObject();
    connMgr_ptr mgr(createConnMgr(db.get()));
    mgr->connect(makeUri(PROTOCOL, HOSTNAME, _T("")));
    BOOST_CHECK(mgr->stat() == 0);
    {
        const connMgr::records& recs = mgr->tables(DBNAMEV3);
        BOOST_CHECK(mgr->stat() == 0);
        BOOST_CHECK(recs.size() == 10); //8 + setenumbit + test.bdf
    }
    {
        const connMgr::records& recs = mgr->views(DBNAMEV3);
        BOOST_CHECK(mgr->stat() == 0);
        BOOST_CHECK(recs.size() == 1);
        BOOST_CHECK(recs[0].name == std::string("idlessthan5"));
    }
    {
        const connMgr::records& recs = mgr->schemaTables(DBNAMEV3);
        BOOST_CHECK(mgr->stat() == 0);
        BOOST_CHECK(recs.size() == 1);
        BOOST_CHECK(recs[0].name == std::string("test"));
    }
    {
        mgr->slaveStatus();
        BOOST_CHECK_MESSAGE(mgr->stat() == 0, "stat = " << mgr->stat());
    }
    {
        const connMgr::records& recs = mgr->extendedvars();
        BOOST_CHECK(mgr->stat() == 0);
        BOOST_CHECK(recs.size() == TD_EXTENDED_VAR_SIZE);
        _tprintf(_T("\nSQL_GTID_MODE = %lld\n"), 
            recs[TD_EXTENDED_VAR_MYSQL_GTID_MODE].longValue);
    }
    {
        const connMgr::records& recs = mgr->slaveHosts();
        BOOST_CHECK(mgr->stat() == 0);
        for (int i=0;i<(int)recs.size();++i)
        {
            _TCHAR tmp[1024];
            recs[i].value(tmp, 1024);
            _tprintf(_T("slaveHosts = %u\t%u\t%s\n"), recs[i].id, recs[i].readCount, tmp);
        }
    }

    mgr->disconnect();

}

void testCreateInfo()
{
    nsdatabase::setCheckTablePtr(true);
    database_ptr db = createDatabaseObject();
    openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_READONLY);
    BOOST_CHECK(db->stat() == 0);

    char buf[2048];
    uint_td size = 2048;
    db->getCreateViewSql(_T("idlessthan5"), buf, &size);
    BOOST_CHECK(db->stat() == 0);
    BOOST_CHECK(size > 20);

    table* tb = db->openTable(1, TD_OPEN_READONLY);
    BOOST_CHECK(db->stat() == 0);
    size = 2048;
    tb->getCreateSql(buf, &size);
    BOOST_CHECK(db->stat() == 0);
    BOOST_CHECK(size > 1000);
    db->close();
    BOOST_CHECK(db->stat() == 0);
}

void testAlias()
{
    try
    {
        database_ptr db = createDatabaseObject();
        openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF);
        table_ptr tb = openTable(db, _T("users"));
        tb->setAlias(_T("name"), _T("name_alias"));
        tb->setAlias(_T("name"), _T("name_alias2"));
        tb->setKeyNum(0);
        tb->seekFirst();
        BOOST_CHECK(tb->stat() == 0);
        // access original 
        BOOST_CHECK(_tstring(tb->getFVstr(_T("name"))) == _tstring(_T("John")));
        // access alias 
        BOOST_CHECK(_tstring(tb->getFVstr(_T("name_alias"))) == _tstring(_T("John")));
        BOOST_CHECK(_tstring(tb->getFVstr(_T("name_alias2"))) == _tstring(_T("John")));
        //query
        query q;
        q.select(_T("name_alias2_"));
        tb->setQuery(&q);
        BOOST_CHECK(tb->stat() != 0);

        q.reset().select(_T("name_alias2"));
        tb->setQuery(&q);
        BOOST_CHECK(tb->stat() == 0);
        tb->clearBuffer();
        tb->find();
        if (tb->stat() == 0)
            BOOST_CHECK(_tstring(tb->getFVstr(_T("name_alias2"))) == _tstring(_T("John")));
        while (tb->stat() == 0)
        {
            BOOST_CHECK(_tstring(tb->getFVstr(_T("name"))) != _tstring(_T("")));
            tb->findNext();
        }
    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
}

void testAutoincWithBlob()
{
    try
    {
        database_ptr db = createDatabaseObject();
        openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF);
        table_ptr tb = openTable(db, _T("users"));
        
        tb->setKeyNum(0);
        tb->clearBuffer();
        tb->setFV(_T("blob"), "abc");
        tb->insert();
        BOOST_CHECK(tb->stat() == 0);
        BOOST_CHECK(tb->getFVint(_T("id")) > 0);
        BOOST_CHECK(strcmp(tb->getFVAstr(_T("blob")), "abc") == 0);

        tb->clearBuffer();
        tb->setFVNull(_T("blob"), true);
        tb->insert();
        BOOST_CHECK(tb->stat() == 0);
        BOOST_CHECK(tb->getFVint(_T("id")) > 0);
        BOOST_CHECK(tb->getFVNull(_T("blob")) == true);

    }
    catch (bzs::rtl::exception& e)
    {
        BOOST_CHECK(false);
        _tprintf(_T("Error! %s\n"), (*getMsg(e)).c_str());
    }
}

void testBinaryFieldConvert()
{
// Ansi
    stringConverter cv(CP_UTF8, CP_UTF8);
    //Unicode --> utf8
    bool ret = cv.isNeedConvert<WCHAR, char>() == true;
    BOOST_CHECK(ret);

    //utf8 --> utf8
    ret = cv.isNeedConvert<char, char>() == false;
    BOOST_CHECK(ret);
    
    //cp932 --> utf8
    cv.setCodePage(932);
    ret = cv.isNeedConvert<char, char>() == true;
    BOOST_CHECK(ret);

    //binnary --> utf8
    cv.setCodePage(0);
    ret = cv.isNeedConvert<char, char>() == false;
    BOOST_CHECK(ret);

    //binnary ? Unicode --> utf8,  Unicode fields are force convert
    ret = cv.isNeedConvert<WCHAR, char>() == true;
    BOOST_CHECK(ret);

// Unicode
    // Unicode --> Unicode
    cv.setCodePage(CP_UTF8);
    ret = cv.isNeedConvert<WCHAR, WCHAR>() == false;
    BOOST_CHECK(ret);

    //utf8 --> Unicode
    ret = cv.isNeedConvert<char, WCHAR>() == true;
    BOOST_CHECK(ret);
    
    //cp932 --> Unicode
    cv.setCodePage(932);
    ret = cv.isNeedConvert<char, WCHAR>() == true;
    BOOST_CHECK(ret);

    //binnary --> Unicode
    cv.setCodePage(0);
    ret = cv.isNeedConvert<char, WCHAR>() == false;
    BOOST_CHECK(ret);

    //binnary ? Unicode --> Unicode,  
    ret = cv.isNeedConvert<WCHAR, WCHAR>() == false;
    BOOST_CHECK(ret);

}

void testTableInvalidRecord()
{
    database_ptr db = createDatabaseObject();
    openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF);
    table_ptr tb = openTable(db, _T("users"));
    query q;
    q.in(1,2,-1,3,4,50000);
    tb->clearBuffer();
    tb->setKeyNum(0);
    tb->setQuery(&q);
    tb->find();
    int i = 0;
    int fdi = 0;
    while (tb->stat()==0 || tb->stat() == STATUS_NOT_FOUND_TI)
    {
        if (i == 2 || i == 5)
        {
            BOOST_CHECK(tb->fields().isInvalidRecord() == true);
            BOOST_CHECK(tb->fields()[fdi].isNull() == true);
        }
        else
        {
            BOOST_CHECK(tb->fields().isInvalidRecord() == false);
            BOOST_CHECK(tb->fields()[fdi].isNull() == false);
        }
        //printf("index %d = %s\n", i, tb->fields()[fdi].isNull() ? "NULL" : "NOT NULL");
        tb->findNext();
        ++i;
    }
    BOOST_CHECK(tb->fields().isInvalidRecord() == true);
    tb->clearBuffer();
    BOOST_CHECK(tb->fields().isInvalidRecord() == false);

    activeTable at(db, _T("users"));
    recordset rs;
    at.index(0).read(rs, q);
    BOOST_CHECK(rs.size() == 6);

    for  (int i = 0; i < (int)rs.size(); ++i)
    {
        if (i == 2 || i == 5)
        {
            BOOST_CHECK(rs[i].isInvalidRecord() == true);
            BOOST_CHECK(rs[i][fdi].isNull() == true);
        }
        else
        {
            BOOST_CHECK(rs[i].isInvalidRecord() == false);
            BOOST_CHECK(rs[i][fdi].isNull() == false);
        }
    }

}

#pragma warning(default : 4996) 

#endif // BZS_TEST_TRDCLENGN_TESTFIELD_H