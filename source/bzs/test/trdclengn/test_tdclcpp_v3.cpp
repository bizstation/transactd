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
#define SP_SCOPE_FIELD_TEST
#include "testField.h"
#include <bzs/db/protocol/tdap/fieldComp.h>

class fixture
{
    mutable database* m_db;

public:
    fixture() : m_db(NULL)
    {
        nsdatabase::setCheckTablePtr(true);
        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
    }

    ~fixture()
    {
        if (m_db)
            database::destroy(m_db);
    }

    ::database* db() const { return m_db; }
};

static bool g_db_created = false;


bool canDatatimeTimeStamp(table* tb, short fieldNum)
{
    const fielddef& fd = tb->tableDef()->fieldDefs[fieldNum]; 
    return !tb->tableDef()->isLegacyTimeFormat(fd);
}

short createTestDataBase(database* db)
{
    if (g_db_created == false)
    {
        db->create(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
        if (db->stat() == STATUS_TABLE_EXISTS_ERROR)
        {
            db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
            if (db->stat()) return db->stat();
            db->drop();
            db->create(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME));
        }
        if (db->stat()) return db->stat();
        
        g_db_created = true;
    }
    return 0;
}

/* Desc of nulltest table timestampNull = true

| Field | Type          | Null | Key | Default              | Extra
+-------+---------------+------+-----+----------------------+-------------------
| id    | int(11)       | NO   | PRI | NULL                 |
| fd1   | int(11)       | YES  | UNI | NULL                 |
| fd2   | int(11)       | YES  |     | NULL                 |
| fd3   | int(11)       | YES  |     | NULL                 |
| fd4   | int(11)       | YES  |     | NULL                 |
| fd5   | int(11)       | YES  |     | -1                   |
| fd6   | varchar(16)   | YES  |     | NULL                 |
| fd7   | varbinary(50) | YES  |     | NULL                 |
| fd8   | timestamp(6)  | YES  |     | NULL                 |
| fd9   | datetime(6)   | YES  |     | NULL                 |
| fd10  | timestamp(6)  | YES  |     | CURRENT_TIMESTAMP(6) | on update CURRENT_TIMESTAMP(6)
| fd11  | datetime(6)   | YES  |     | NULL                 |
+-------+---------------+------+-----+----------------------+-------------------

CREATE TABLE `nulltest` (`id` INT NOT NULL ,`fd1` INT NULL DEFAULT NULL,`fd2` INT NULL DEFAULT NULL,`fd3` INT NULL DEFAULT NULL,`fd4` INT NULL DEFAULT NULL,`fd5` INT NULL DEFAULT '-1',`fd6` VARCHAR(16) binary NULL DEFAULT '-123456',`fd7` VARBINARY(50) NULL DEFAULT NULL,`fd8` TIMESTAMP(6) NULL DEFAULT NULL,`fd9` DATETIME(2) NULL DEFAULT NULL,`fd10` TIMESTAMP(6) NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),`fd11` DATETIME(6) NULL DEFAULT NULL, UNIQUE key0(`id`), UNIQUE key1(`fd1`)) ENGINE=InnoDB default charset=utf8
*/
short createTestTable(database* db, bool timestampNull = true, bool supportDateTimeTimeStamp = true)
{
    try
    {
        openDatabase(db, makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
        dbdef* def = db->dbDef();
        short tableid = 1;
        db->dropTable(_T("nulltest"));
        def->deleteTable(tableid);
        
        short fieldnum = 0;
        insertTable(def, tableid,  _T("nulltest"), g_td_charsetIndex);

        insertField(def, tableid, fieldnum, _T("id"), ft_integer, 4);
        fielddef* fd = insertField(def, tableid, ++fieldnum, _T("fd1"), ft_integer, 4);
        fd->setNullable(true);
        //fd->nullbit = 100;
        //fd->nullbytes = 30;

        fd = insertField(def, tableid, ++fieldnum, _T("fd2"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("fd3"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("fd4"), ft_integer, 4);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("fd5"), ft_integer, 4);
        fd->setNullable(timestampNull, false);
        fd->setDefaultValue(-1LL);
        fd = insertField(def, tableid, ++fieldnum, _T("fd6"), ft_myvarchar, 49);
        fd->setNullable(true, false);
        fd->setDefaultValue(_T("-123456"));
        //fd->setDefaultValue(_T("漢字"));
        fd = insertField(def, tableid, ++fieldnum, _T("fd7"), ft_myvarbinary, 51);
        fd->setNullable(true);
        fd = insertField(def, tableid, ++fieldnum, _T("fd8"), ft_mytimestamp, 7);
        fd->setNullable(timestampNull);
        fd->decimals = 6;

        fd = insertField(def, tableid, ++fieldnum, _T("fd9"), ft_mydatetime, 6);
        fd->setNullable(true);
        fd->decimals = 2;

        fd = insertField(def, tableid, ++fieldnum, _T("fd10"), ft_mytimestamp, 7);
        fd->setNullable(true);
        fd->decimals = 6;
        if (timestampNull == false && supportDateTimeTimeStamp)
        {
            fd->setDefaultValue(DFV_TIMESTAMP_DEFAULT);
            fd->setTimeStampOnUpdate(true);
        }

        fd = insertField(def, tableid, ++fieldnum, _T("fd11"), ft_mydatetime, 8);
        fd->setNullable(timestampNull);
        fd->decimals = 6;
        if (timestampNull == false && supportDateTimeTimeStamp)
            fd->setTimeStampOnUpdate(true);

        fd = insertField(def, tableid, ++fieldnum, _T("fd12"), ft_myyear, 1);
        fd->setNullable(true);

        keydef* kd = insertKey(def, tableid, 0);
        kd->segments[0].fieldNum = 0;
        kd->segments[0].flags.bit8 = 1; // extended key type
        kd->segments[0].flags.bit1 = 1; // changeable
        kd->segmentCount = 1;
        
        kd = insertKey(def, tableid, 1);
        kd->segments[0].fieldNum = 1;
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

void testNoSchema(database* db)
{
    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3));
    if (db->stat()==0)
        db->drop();
    
    db->create(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3));
    BOOST_CHECK_MESSAGE(db->stat() == 0, "create stat = " << db->stat());

    db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3));
    BOOST_CHECK_MESSAGE(db->stat() == 0, "open stat = " << db->stat());
    if (db->stat()==0)
    {
        db->drop();
        BOOST_CHECK_MESSAGE(db->stat() == 0, "drop stat = " << db->stat());
    }
}

void testFielddefs(database* db)
{
    short ret = createTestDataBase(db);
    BOOST_CHECK_MESSAGE(0 == ret, "createTestDataBase stat = " << ret);
    if (ret) return;
    ret = createTestTable(db);
    BOOST_CHECK_MESSAGE(0 == ret, "createTestTable stat = " << ret);
    if (ret) return;
    try
    {
        short tableid = 1;
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
        
        //Check nullbit
        const tabledef* td = tb->tableDef();
        const fielddef* fd = &td->fieldDefs[1];
        BOOST_CHECK_MESSAGE(fd->nullbit() == 0, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 2, "Invalid nullbytes = " << (int)fd->nullbytes());

        fd = &td->fieldDefs[8]; //fd8
        BOOST_CHECK_MESSAGE(fd->nullbit() == 7, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 2, "Invalid nullbytes = " << (int)fd->nullbytes());
        //fielddefs copy test
        fielddefs& fds = *fielddefs::create();
        fds.addAllFileds(tb->tableDef());
        fd = &fds[1];
        BOOST_CHECK_MESSAGE(fd->nullbit() == 0, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 2, "Invalid nullbytes = " << (int)fd->nullbytes());

        // Append join field, nullbytes and nullbit specify only append fields.
        query q;
        q.select(_T("fd8"));
        tb->setQuery(&q);
        fds.addSelectedFields(tb.get());
        fd = &fds[(int)fds.size() -1];
        BOOST_CHECK_MESSAGE(fd->nullbit() == 0, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 1, "Invalid nullbytes = " << (int)fd->nullbytes());
        
        //One more join 
        q.reset().select(_T("fd2"), _T("fd3"), _T("fd4"), _T("fd5"), _T("fd6"));
        tb->setQuery(&q);
        fds.addSelectedFields(tb.get());
        fd = &fds[(int)fds.size() -1];
        BOOST_CHECK_MESSAGE(fd->nullbit() == 4, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 1, "Invalid nullbytes = " << (int)fd->nullbytes());

        //One more join 
        q.reset().select(_T("fd1"), _T("fd2"), _T("fd3"), _T("fd4"), _T("fd5"), _T("fd6"), _T("fd7"), _T("fd8"));
        tb->setQuery(&q);
        fds.addSelectedFields(tb.get());
        fd = &fds[(int)fds.size() -1];
        BOOST_CHECK_MESSAGE(fd->nullbit() == 7, "Invalid nullbit = " << (int)fd->nullbit());
        BOOST_CHECK_MESSAGE(fd->nullbytes() == 1, "Invalid nullbytes = " << (int)fd->nullbytes());
        fds.release();
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testFieldValue(database* db)
{
    try
    {
        short tableid = 1;
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
        tb->clearBuffer();
        for (int i = 1; i < tb->tableDef()->fieldCount; ++i)
        {
            tb->setFVNull(i, true);
            BOOST_CHECK_MESSAGE(tb->getFVNull(i) == true, "Invalid getFVNull i = " << i);
            tb->setFVNull(i, false);
            BOOST_CHECK_MESSAGE(tb->getFVNull(i) == false, "Invalid getFVNull i = " << i);
        }

        // field 0 is not nullable
        short index = 0;
        tb->setFVNull(index, true);
        BOOST_CHECK_MESSAGE(tb->getFVNull(index) == false, "Invalid getFVNull(0) ");
        tb->setFVNull(index, false);
        BOOST_CHECK_MESSAGE(tb->getFVNull(index) == false, "Invalid getFVNull(0) ");
 
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testWriatbleRecordFieldValue(database* db)
{
    try
    {
        activeTable tb(db, _T("nulltest"));
        writableRecord& wr = tb.getWritableRecord();
        wr.clear();
        for (int i = 1; i < (int)wr.fieldDefs()->size(); ++i)
        {
            wr[i].setNull(true);
            BOOST_CHECK_MESSAGE(wr[i].isNull() == true, "Invalid isNull i = " << i);
            wr[i].setNull(false);
            BOOST_CHECK_MESSAGE(wr[i].isNull() == false, "Invalid isNull i = " << i);
        }

        // field 0 is not nullable
        short index = 0;
        wr[index].setNull(true);
        BOOST_CHECK_MESSAGE(wr[index].isNull() == false, "Invalid isNull ");
        wr[index].setNull(false);
        BOOST_CHECK_MESSAGE(wr[index].isNull() == false, "Invalid isNull ");
 
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

// ------------------------------------------------------------------------
class fixtureStore
{
    mutable database* m_db;
public:
    fixtureStore() : m_db(NULL)
    {
        nsdatabase::setCheckTablePtr(true);
        bool db_created = g_db_created;

        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
        short ret = createTestDataBase(m_db);
        if (ret)
        {    
            printf("Error createDataBase\n");
            return;
        }
        if (!db_created)
            ret = createTestTable(m_db);
        else
        {
             m_db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, BDFNAME), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
             ret = m_db->stat();
        }
        if (ret)
            printf("Error createTable\n");
    }

    ~fixtureStore()
    {
        if (m_db)
            m_db->release();
    }
    ::database* db() const { return m_db; }
};

void testDefaultValue(database* db, bool timestampNull)
{
    try
    {
        short tableid = 1;
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL+TD_OPEN_MASK_MYSQL_NULL);
        tb->setKeyNum(0);
        tb->clearBuffer(table::defaultNull);
        fields& fds = tb->fields();
        if (timestampNull)
        {
            for (short i = 0 ;i < (short)fds.size(); ++i)
            {
                bool v = true;
                int dv = 0;
                if (i == 0 || i == 5 || i == 6) v = false;
                BOOST_CHECK_MESSAGE(fds[i].isNull() == v, "testDefaultValue isNull field num = " << i);
                if (i == 5) dv = -1;
                    
                if (i == 6) dv = -123456;
                BOOST_CHECK_MESSAGE(fds[i].i() == dv, "testDefaultValue defaultValue field num = " 
                        << i << " " << fds[i].i());
            }
        }else
        {
            int dfs[13] = {0, 0, 0, 0, 0, -1, -123456, 0, 3, 0, 0, 1, 0};
            for (short i = 1 ;i < (short)fds.size(); ++i)
            {
                BOOST_CHECK_MESSAGE(fds[i].isNull() == (dfs[i] == 0), "testDefaultValue isNull field num = " << i);
                if ((dfs[i] < 0) || i == 12)
                    BOOST_CHECK_MESSAGE(fds[i].i() == dfs[i], "testDefaultValue i() field num = " << i);
                else if ((dfs[i]  < 3))
                    BOOST_CHECK_MESSAGE(fds[i].i64() == 0, "testDefaultValue i64() field num = " << i);
            }
        }
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}


void testWRDefaultValue(database* db, bool timestampNull)
{
    try
    {
        activeTable tb(db, _T("nulltest"));
        writableRecord& wr = tb.getWritableRecord();
        wr.clear();
        if (timestampNull)
        {
            for (short i = 0 ;i < (short)wr.size(); ++i)
            {
                bool v = true;
                int dv = 0;
                if (i == 0 || i == 5 || i == 6) v = false;
                BOOST_CHECK_MESSAGE(wr[i].isNull() == v, "testWRDefaultValue isNull field num = " << i);
                if (i == 5) dv = -1;
                    
                if (i == 6) dv = -123456;
                BOOST_CHECK_MESSAGE(wr[i].i() == dv, "testWRDefaultValue defaultValue field num = " 
                        << i << " " << wr[i].i());
            }
        }else
        {
            int dfs[13] = {0, 0, 0, 0, 0, -1, -123456, 0, 3, 0, 0, 1, 0};
            for (short i = 1 ;i < (short)wr.size(); ++i)
            {
                BOOST_CHECK_MESSAGE(wr[i].isNull() == (dfs[i] == 0), "testWRDefaultValue isNull field num = " << i);
                if ((dfs[i] < 0))
                    BOOST_CHECK_MESSAGE(wr[i].i() == dfs[i], "testWRDefaultValue isNull field num = " << i);
                else if ((dfs[i]  < 3))
                    BOOST_CHECK_MESSAGE(wr[i].i64() == 0, "testWRDefaultValue isNull field num = " << i);
            }
        }
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testTableStore(database* db)
{
    try
    {
        // All null test
        short tableid = 1;
        table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL);
        tb->setKeyNum(0);
        tb->clearBuffer();
        fields& fds = tb->fields();
        
        fds[(short)0] = 1;
        for (short i = 1 ;i < (short)fds.size(); ++i)
        {
            fds[i] = _T("123");
            fds[i].setNull(true);
        }
        tb->insert();
        BOOST_CHECK_MESSAGE(tb->stat() == 0, "testStore insert stat = " << tb->stat());
        
        tb->clearBuffer();
        fds[(short)0] = 1;
        tb->seek();
        BOOST_CHECK_MESSAGE(tb->stat() == 0, "testStore seek stat = " << tb->stat());
        for (short i = 1 ;i < (short)fds.size(); ++i)
            BOOST_CHECK_MESSAGE(fds[i].isNull() == true, "testStore isNull field num = " << i);

        // All not null test
        tb->clearBuffer(table::defaultNull);
        fds[(short)0] = 2;
        for (short i = 1 ;i < (short)fds.size(); ++i)
            fds[i].setNull(false);
        tb->insert();
        BOOST_CHECK_MESSAGE(tb->stat() == 0, "testStore insert stat = " << tb->stat());

        tb->clearBuffer(table::defaultNull);
        
        fds[(short)0] = 2;
        tb->seek();
        BOOST_CHECK_MESSAGE(tb->stat() == 0, "testStore seek stat = " << tb->stat());
        for (short i = 1 ;i < (short)fds.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(fds[i].isNull() == false, "testStore isNull field num = " << i);
            //Test Default value
            __int64 dv = 0;
            if (i == 5) dv = -1;
            if (i == 6) dv = -123456;
            if (i != 10) //ignore timestamp
                BOOST_CHECK_MESSAGE(fds[i].i64() == dv, "testStore defaultValue field num = " 
                    << i << " " << fds[i].i64());
        }
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testWRStore(database* db)
{
    try
    {
        // All null test
        activeTable tb(db, _T("nulltest"));
        writableRecord& wr = tb.getWritableRecord();
        wr.clear();
        
        wr[(short)0] = 3;
        for (short i = 1 ;i < (short)wr.size(); ++i)
        {
            wr[i] = _T("123");
            wr[i].setNull(true);
        }
        wr.save();
       
        wr.clear();
        wr[(short)0] = 3;
        bool ret = wr.read();
        BOOST_CHECK_MESSAGE(ret == true, "testWRStore read ");
        for (short i = 1 ;i < (short)wr.size(); ++i)
            BOOST_CHECK_MESSAGE(wr[i].isNull() == true, "testWRStore isNull field num = " << i);

        // All not null test
        wr.clear();
        wr[(short)0] = 4;
        wr[1] = 2;
        for (short i = 1 ;i < (short)wr.size(); ++i)
            wr[i].setNull(false);
        wr.save();

        wr.clear();
        wr[(short)0] = 4;
        ret = wr.read();
        BOOST_CHECK_MESSAGE(ret == true, "testWRStore read");
        for (short i = 1 ;i < (short)wr.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(wr[i].isNull() == false, "testWRStore isNull field num = " << i);
            //Test Default value
            __int64 dv = 0;
            if (i == 1) dv = 2;
            if (i == 5) dv = -1;
            if (i == 6) dv = -123456;
            if (i != 10) //ignore timestamp
                BOOST_CHECK_MESSAGE(wr[i].i64() == dv, "testWRStore defaultValue field num = " 
                    << i << " " << wr[i].i64());
        }
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testAutoNotNull(database* db)
{
    activeTable tb(db, _T("nulltest"));
    writableRecord& wr = tb.getWritableRecord();
    wr.clear();
        
    wr[(short)0] = 3;
    for (short i = 1 ;i < (short)wr.size(); ++i)
    {
        wr[i].setNull(true);
        wr[i] = 1;// setNotNull automaticaly
        BOOST_CHECK_MESSAGE(wr[i].isNull() == false, "testAutoNotNull isNull field num = " << i);
    }
}

void testSchemaSync(database* db)
{
    try
    {
        dbdef* def = db->dbDef();
        short tableid = 1;
        def->pushBackup(tableid);
        def->deleteField(tableid, 10);
        updateTableDef(def, tableid);
        synchronizeSeverSchema(def, tableid);
        if (def->compAsBackup(tableid))
            BOOST_CHECK_MESSAGE(false, "testSchemaSync");
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("testSchemaSync Error ! %s\n"), (*getMsg(e)).c_str());
    }
}

void testUnuseSchema(database* db)
{
    try
    {
        db->close();
        bool ret = db->open(makeUri(PROTOCOL, HOSTNAME, DBNAMEV3, _T("")), TYPE_SCHEMA_BDF,TD_OPEN_NORMAL);
        BOOST_CHECK_MESSAGE(ret == true, "db open stat = " << db->stat());
        table_ptr tb = openTable(db, _T("nulltest"), TD_OPEN_NORMAL);
        tb->setKeyNum(0);
        tb->clearBuffer();
        fields& fds = tb->fields();
        fds[(short)0] = 1;
        tb->seek();
        BOOST_CHECK_MESSAGE(tb->stat() == 0, "UnuseSchema seek stat = " << tb->stat());
        for (short i = 1 ;i < (short)fds.size(); ++i)
            BOOST_CHECK_MESSAGE(fds[i].isNull() == true, "UnuseSchema isNull field num = " << i);

        //open second table
        table_ptr tb2 = openTable(db, _T("nulltest"), TD_OPEN_NORMAL);
        BOOST_CHECK_MESSAGE(db->stat() == 0, "UnuseSchema openTable stat = " << db->stat());

        //shared tabledef
        BOOST_CHECK_MESSAGE(db->dbDef()->tableCount() == 1, "tableCount = " << db->dbDef()->tableCount());

        tb2->setKeyNum(0);
        tb2->clearBuffer(table::defaultNull);

        //default values
        fields& fds2 = tb2->fields();
        for (short i = 1 ;i < (short)fds.size(); ++i)
        {
            bool v = true;
            int dv = 0;
            if (i == 0 || i == 5 || i == 6) v = false;
            BOOST_CHECK_MESSAGE(fds2[i].isNull() == v, "defaultValue isNull field num = " << i);
            if (i == 5) dv = -1;
                    
            if (i == 6) dv = -123456;
            BOOST_CHECK_MESSAGE(fds2[i].i() == dv, "defaultValue defaultValue field num = " 
                    << i << " " << fds2[i].i());
        }
    
        fds2[(short)0] = 1;
        tb2->seek();
        BOOST_CHECK_MESSAGE(tb2->stat() == 0, "UnuseSchema seek stat = " << tb2->stat());
        for (short i = 1 ;i < (short)fds2.size(); ++i)
            BOOST_CHECK_MESSAGE(fds2[i].isNull() == true, "UnuseSchema isNull field num = " << i);
    
        db->openTable(_T("abc"));
        BOOST_CHECK_MESSAGE(db->stat() == STATUS_TABLE_NOTOPEN, "openTable stat = " << db->stat());
    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("testUnuseSchema Error ! %s\n"), (*getMsg(e)).c_str());
    }
}
// ------------------------------------------------------------------------
class fixtureTimestamp
{
    mutable database* m_db;
    bool supportDateTimeTimeStamp;
public:
    fixtureTimestamp() : m_db(NULL)
    {
        nsdatabase::setCheckTablePtr(true);
        m_db = database::create();
        if (!m_db)
            printf("Error database::create()\n");
        short ret = createTestDataBase(m_db);
        if (ret)
        {    
            printf("Error createDataBase\n");
            return;
        }
        m_db->connect(makeUri(PROTOCOL, HOSTNAME, _T("")));
        if (m_db->stat())
        {    
            printf("Error db connect\n");
            return;
        }
        btrVersions vs;
        m_db->getBtrVersion(&vs);
        if (m_db->stat())
        {    
            printf("Error getBtrVersion\n");
            return;
        }
        btrVersion& v = vs.versions[1];
        supportDateTimeTimeStamp =  v.isSupportDateTimeTimeStamp();
        
        ret = createTestTable(m_db, false, supportDateTimeTimeStamp);
        if (ret)
            printf("Error createTable\n");
    }

    ~fixtureTimestamp()
    {
        if (m_db)
            m_db->release();
    }
    ::database* db() const { return m_db; }
};

/* Desc of nulltest table timestampNull = false

| Field | Type          | Null | Key | Default              | Extra
+-------+---------------+------+-----+----------------------+-------------------
| id    | int(11)       | NO   | PRI | NULL                 |
| fd1   | int(11)       | YES  | UNI | NULL                 |
| fd2   | int(11)       | YES  |     | NULL                 |
| fd3   | int(11)       | YES  |     | NULL                 |
| fd4   | int(11)       | YES  |     | NULL                 |
| fd5   | int(11)       | NO   |     | -1                   |
| fd6   | varchar(16)   | YES  |     | NULL                 |
| fd7   | varbinary(50) | YES  |     | NULL                 |
| fd8   | timestamp(6)  | NO   |     | CURRENT_TIMESTAMP(6) | on update CURRENT_TIMESTAMP(6) |
| fd9   | datetime(6)   | YES  |     | NULL                 |
| fd10  | timestamp(6)  | YES  |     | CURRENT_TIMESTAMP(6) | on update CURRENT_TIMESTAMP(6) |
| fd11  | datetime(6)   | NO   |     | NULL                 | on update CURRENT_TIMESTAMP(6) |
+-------+---------------+------+-----+----------------------+-------------------
*/
void testTimestamp(database* db)
{
    try
    {
        short tableid = 1;
        {
            table_ptr tb = openTable(db, tableid, TD_OPEN_NORMAL+TD_OPEN_MASK_MYSQL_NULL);
            tb->setKeyNum(0);
            tb->clearBuffer();
            fields& fds = tb->fields();
            fds[(short)0] = 1;
            for (short i = 1 ;i < (short)fds.size(); ++i)
                fds[i].setNull(true);
            tb->insert();
            BOOST_CHECK_MESSAGE(tb->stat() == 0, "testTimestamp insert stat = " << tb->stat());
        
            tb->clearBuffer();
            fds[(short)0] = 1;
            tb->seek();
            BOOST_CHECK_MESSAGE(tb->stat() == 0, "testTimestamp seek stat = " << tb->stat());

            __int64 v = fds[_T("fd8")].i64();
            BOOST_CHECK_MESSAGE(v != 0, "Timestamp is 0 ");
            
            BOOST_CHECK_MESSAGE(fds[_T("fd10")].isNull() == true, "Timestamp2 not null" );

            Sleep(1);
            tb->update();
            BOOST_CHECK_MESSAGE(tb->stat() == 0, "testTimestamp update stat = " << tb->stat());
            tb->seek();

            if (canDatatimeTimeStamp(tb.get(), 8))
                BOOST_CHECK_MESSAGE(fds[_T("fd8")].i64() != v, "Timestamp not udapte ");
            BOOST_CHECK_MESSAGE(fds[_T("fd8")].i64() != 0, "Timestamp is zero ");

        }

    }
    catch (bzs::rtl::exception& e)
    {
        _tprintf(_T("Error ! %s\n"), (*getMsg(e)).c_str());
    }
}
// ------------------------------------------------------------------------



void testMyDateTimeStore()
{
    __int64 tmp;

    fielddefs* fds = fielddefs::create();
    fielddef f;
    memset(&f, 0, sizeof(fielddef));
    f.len = 8;
    f.pos = 0;

    //timestamp
    f.type = ft_mytimestamp;
    f.decimals = 6;
    field fd((unsigned char* )&tmp, f, fds);
    myTimeStamp dt(f.decimals, true);
    dt = "2010-10-10 00:00:00.123456";
    fd = dt.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt.i64, "ft_mytimestamp7 value = " << fd.i64());

        //Legacy format
    myTimeStamp dtl(f.decimals, false);
    dtl = "2010-10-10 00:00:00";
    fd = dtl.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dtl.i64, "ft_mytimestamp legacy value = " << fd.i64());

    
    f.decimals = 4; f.len = 7;
    myTimeStamp dt2(f.decimals, true);
    dt2 = "2010-10-10 00:00:12.9988";
    fd = dt2.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt2.i64, "ft_mytimestamp6 value = " << fd.i64());

    f.decimals = 2; f.len = 6;
    myTimeStamp dt3(f.decimals, true);
    dt3 = "2010-10-10 00:00:12.23";
    fd = dt3.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt3.i64, "ft_mytimestamp5 value = " << fd.i64());

    f.decimals = 0; f.len = 5;
    myTimeStamp dt4(f.decimals, true);
    dt4 = "2010-10-10 00:00:12";
    fd = dt4.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt4.i64, "ft_mytimestamp4 value = " << fd.i64());

    //datetime
    f.decimals = 6; f.len = 8;
    f.type = ft_mydatetime;
    myDateTime dt5(f.decimals, true);
    dt5 = "2015-10-10 00:00:12.445566";
    fd = dt5.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt5.i64, "ft_mydatetime8 value = " << fd.i64());

        //Legacy format
    myDateTime dt5l(f.decimals, true);
    dt5l = "2015-10-10 00:00:12";
    fd = dt5l.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt5l.i64, "ft_mydatetime Legacy value = " << fd.i64());

    f.decimals = 4; f.len = 7;
    myDateTime dt6(f.decimals, true);
    dt6 = "2015-10-10 00:00:12.7788";
    fd = dt6.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt6.i64, "ft_mydatetime7 value = " << fd.i64());

    f.decimals = 2; f.len = 6;
    myDateTime dt7(f.decimals, true);
    dt7 = "2015-10-10 00:00:12.00";
    fd = dt7.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt7.i64, "ft_mydatetime6 value = " << fd.i64());

    f.decimals = 0; f.len = 5;
    myDateTime dt71(f.decimals, true);
    dt71 = "2015-10-10 00:00:12";
    fd = dt71.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt71.i64, "ft_mydatetime5 value = " << fd.i64());

    //mariadb datetime
    f.setOptions(FIELD_OPTION_MARIADB);
    f.decimals = 6; f.len = 8;
    f.type = ft_mydatetime;
    maDateTime dta(f.decimals, true);
    dta = "2015-10-10 00:00:12.445566";
    fd = dta.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta.i64, "ft_mydatetime8 maridb value = " << fd.i64());

    f.decimals = 4; f.len = 7;
    maDateTime dta1(f.decimals, true);
    dta1 = "2015-10-10 00:00:12.7788";
    fd = dta1.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta1.i64, "ft_mydatetime7 maridb value = " << fd.i64());

    f.decimals = 2; f.len = 6;
    maDateTime dta2(f.decimals, true);
    dta2 = "2015-10-10 00:00:12.00";
    fd = dta2.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta2.i64, "ft_mydatetime6 maridb value = " << fd.i64());

    f.decimals = 0; f.len = 5;
    maDateTime dta20(f.decimals, true);
    dta20 = "2015-10-10 00:00:12";
    fd = dta20.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta20.i64, "ft_mydatetime6 maridb value = " << fd.i64());

    // mariadb time
    f.decimals = 6; f.len = 6;
    f.type = ft_mytime;
    maTime dtma1(f.decimals, true);
    dtma1 = "00:00:12.123456";
    fd = dtma1.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dtma1.i64, "ft_mytime6 maridb value = " << fd.i64());

    f.decimals = 4; f.len = 5;
    maTime dtma2(f.decimals, true);
    dtma2 = "00:00:12.1234";
    fd = dtma2.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dtma2.i64, "ft_mytime5 maridb value = " << fd.i64());

    f.decimals = 2; f.len = 4;
    maTime dta3(f.decimals, true);
    dta3 = "00:00:12.12";
    fd = dta3.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta3.i64, "ft_mytime4 maridb value = " << fd.i64());

    f.decimals = 0; f.len = 3;
    maTime dta4(f.decimals, true);
    dta4 = "00:00:12";
    fd = dta4.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta4.i64, "ft_mytime3 maridb value = " << fd.i64());

    maTime dta5(f.decimals, false);
    dta5 = "00:00:12";
    fd = dta5.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dta5.i64, "ft_mytime Legacy maridb value = " << fd.i64());


    // MySQl time
    f.setOptions(0);
    f.decimals = 6; f.len = 6;
    f.type = ft_mytime;
    myTime dt10(f.decimals, true);
    dt10 = "00:00:12.123456";
    fd = dt10.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt10.i64, "ft_mytime6 value = " << fd.i64());

    f.decimals = 4; f.len = 5;
    myTime dt11(f.decimals, true);
    dt11 = "00:00:12.1234";
    fd = dt11.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt11.i64, "ft_mytime5 value = " << fd.i64());

    f.decimals = 2; f.len = 4;
    myTime dt12(f.decimals, true);
    dt12 = "00:00:12.12";
    fd = dt12.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt12.i64, "ft_mytime4 value = " << fd.i64());

    f.decimals = 0; f.len = 3;
    myTime dt13(f.decimals, true);
    dt13 = "00:00:12";
    fd = dt13.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt13.i64, "ft_mytime3 value = " << fd.i64());

    myTime dt13l(f.decimals, false);
    dt13l = "00:00:12";
    fd = dt13l.i64;
    BOOST_CHECK_MESSAGE(fd.i64() == dt13l.i64, "ft_mytime Legacy value = " << fd.i64());

    //print
    f.type = ft_mydatetime;
    f.decimals = 6; f.len = 8;
    fd = _T("2015-10-01 20:50:36.002000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.002000"))
                            , "ft_mydatetime string8 = " << fd.c_str());

    f.decimals = 4; f.len = 7;
    fd = _T("2015-10-01 20:50:36.002000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.0020"))
                            , "ft_mydatetime string7 = " << fd.c_str());

    f.decimals = 2; f.len = 6;
    fd = _T("2015-10-01 20:50:36.052000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.05"))
                            , "ft_mydatetime string6 = " << fd.c_str());

    f.decimals = 0; f.len = 5;
    fd = _T("2015-10-01 20:50:36.002000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36"))
                            , "ft_mydatetime string5 = " << fd.c_str());

    f.type = ft_mytimestamp;
    f.decimals = 6; f.len = 7;
    fd = _T("2015-10-01 20:50:36.052001");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.052001"))
                            , "ft_mytimestamp string7 = " << fd.c_str());

    f.decimals = 4; f.len = 6;
    fd = _T("2015-10-01 20:50:36.052001");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.0520"))
                            , "ft_mytimestamp string6 = " << fd.c_str());

    f.decimals = 2; f.len = 5;
    fd = _T("2015-10-01 20:50:36.052000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36.05"))
                            , "ft_mytimestamp string5 = " << fd.c_str());
    f.decimals = 0; f.len = 4;
    fd = _T("2015-10-01 20:50:36.952000");
    BOOST_CHECK_MESSAGE(fd.c_str() == _tstring(_T("2015-10-01 20:50:36"))
                            , "ft_mytimestamp string4 = " << fd.c_str());

    fds->release();
}


BOOST_AUTO_TEST_SUITE(test_v3)

BOOST_FIXTURE_TEST_CASE(noschema, fixture)
{
    testNoSchema(db());
}

BOOST_FIXTURE_TEST_CASE(nullbit, fixture)
{
    testFielddefs(db());
    testFieldValue(db());
    testWriatbleRecordFieldValue(db());
}

BOOST_FIXTURE_TEST_CASE(nullstore, fixtureStore)
{
    testDefaultValue(db(), true);
    testWRDefaultValue(db(), true);
    testTableStore(db());
    testWRStore(db());
    testAutoNotNull(db());
    testSchemaSync(db());
    testUnuseSchema(db());
}

BOOST_FIXTURE_TEST_CASE(timestamp, fixtureTimestamp)
{
    testDefaultValue(db(), false);
    testWRDefaultValue(db(), false);
    testTimestamp(db());
}

BOOST_AUTO_TEST_CASE(dateTimeStore)
{
    testModeMacro();
    testMyDateTimeStore();
}

BOOST_FIXTURE_TEST_CASE(fieldstore, fixtureFieldStore)
{
    testStoreInt(db());
    if (isLegacyTimeFormat())
        testStoreLegacyTime(db());
    else
        testStoreTime(db(), isMysql56TimeFormat(), isSupportMultiTimeStamp());
    test_NOT_HA_OPTION_PACK_RECORD(db());
    testInMany(db());
    testNullValue(db());
    testSetEnumBit();
}

BOOST_AUTO_TEST_CASE(null_comp)
{
    BOOST_CHECK_EQUAL(nullComp(true, true, (char)eIsNull),     0);
    BOOST_CHECK_EQUAL(nullComp(true, true, (char)eIsNotNull),  -1); 
    BOOST_CHECK_EQUAL(nullComp(true, false, (char)0),          -1); 
    BOOST_CHECK_EQUAL(nullComp(false, true, (char)eIsNull),    1); 
    BOOST_CHECK_EQUAL(nullComp(false, true, (char)eIsNotNull), 0); 
    BOOST_CHECK_EQUAL(nullComp(false, false, (char)0),         2); 
}

BOOST_AUTO_TEST_CASE(field_comp)
{
    testCompInt();
    testCompUint();
    testCompDouble();
    testCompBit();
    testCompSet();
    testCompEnum();
    testCompYear();
    testCompDate();
    testCompTime();
    testCompDateTime();
    testCompTimeStamp();
    testCompTimeMa();
    testCompDateTimeMa();
    testCompTimeStampMa();
    testCompString();
#ifdef _WIN32
    testCompWString();
#endif
    testCompBlob();
    testCompDecimal();
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(snapshot)
BOOST_AUTO_TEST_CASE(snapshot_binlog)
{
    testSnapshotWithbinlog();
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(tablelist)
BOOST_AUTO_TEST_CASE(tablelist)
{
    testTableList();
}
BOOST_AUTO_TEST_SUITE_END()
// ------------------------------------------------------------------------
