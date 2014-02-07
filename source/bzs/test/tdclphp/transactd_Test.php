<?php
/* ================================================================
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
================================================================ */
mb_internal_encoding('UTF-8');

require_once("transactd.php");

define("HOSTNAME", "localhost/");
define("DBNAME", "test");
define("DBNAME_VAR", "testvar");
define("DBNAME_SF", "testString");
define("TABLENAME", "user");
define("PROTOCOL", "tdap://");
define("BDFNAME", "?dbfile=test.bdf");
define("URL", PROTOCOL . HOSTNAME . DBNAME . BDFNAME);
define("URL_VAR", PROTOCOL . HOSTNAME . DBNAME_VAR . BDFNAME);
define("URL_SF", PROTOCOL . HOSTNAME . DBNAME_SF . BDFNAME);
define("FDI_ID", 0);
define("FDI_NAME", 1);
define("FDI_GROUP", 2);
define("FDI_NAMEW", 2);

define("BULKBUFSIZE", 65535 - 1000);
define("TEST_COUNT", 20000);
define("FIVE_PERCENT_OF_TEST_COUNT", TEST_COUNT / 20);

define("TYPE_SCHEMA_BDF", 0);

define("ISOLATION_READ_COMMITTED", true);
define("ISOLATION_REPEATABLE_READ", false);

class transactdTest extends PHPUnit_Framework_TestCase
{
    private function getDbObj()
    {
        return database::createObject();
    }
    private function deleteDbObj($db)
    {
        $db->close();
        $db = NULL;
    }
    private function dropDatabase($db)
    {
        $db->open(URL);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
    }
    private function createDatabase($db)
    {
        $db->create(URL);
        if ($db->stat() == transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->dropDatabase($db);
            $db->create(URL);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db)
    {
        $db->open(URL, TYPE_SCHEMA_BDF, transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
    }
    private function createTable($db)
    {
        $this->openDatabase($db);
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, NULL);
        $td = new tabledef();
        // Set table schema codepage to UTF-8
        //     - codepage for field NAME and tableNAME
        $td->schemaCodePage = transactd::CP_UTF8;
        $td->setTableName(TABLENAME);
        $td->setFileName(TABLENAME . '.dat');
        // Set table default charaset index
        //    - default charset for field VALUE
          $td->charsetIndex = transactd::charsetIndex(transactd::CP_UTF8);
        //
        $tableid = 1;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 0);
        $fd->setName("id");
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 1);
        $fd->setName("name");
        $fd->type = transactd::ft_zstring;
        $fd->len = 33;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // Set field charset index
        //    - charset for each field VALUE
        //  $fd->setCharsetIndex(transactd::charsetIndex(transactd::CP_UTF8))
        
        $fd = $dbdef->insertField($tableid, 2);
        $fd->setName("select");
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 3);
        $fd->setName("in");
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $kd = $dbdef->insertKey($tableid, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
    }
    private function openTable($db)
    {
        $this->openDatabase($db);
        $tb = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        return $tb;
    }
    
    public function testCreateDatabase()
    {
        $db = $this->getDbObj();
        $this->createDatabase($db);
        $this->deleteDbObj($db);
    }
    public function testCreateTable()
    {
        $db = $this->getDbObj();
        $this->createTable($db);
        $this->deleteDbObj($db);
    }
    public function testOpenTable()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testVersion()
    {
        $db = $this->getDbObj();
        $db->connect(PROTOCOL . HOSTNAME);
        $this->assertEquals($db->stat(), 0);
        $vv = new btrVersions();
        $db->getBtrVersion($vv);
        $this->assertEquals($db->stat(), 0);
        $client_ver = $vv->version(0);
        $server_ver = $vv->version(1);
        $engine_ver = $vv->version(2);
        $this->assertEquals($client_ver->majorVersion, transactd::CPP_INTERFACE_VER_MAJOR);
        $this->assertEquals($client_ver->minorVersion, transactd::CPP_INTERFACE_VER_MINOR);
        $this->assertEquals(chr($client_ver->type), 'N');
        $this->assertTrue($server_ver->majorVersion >= 5);
        $this->assertTrue($server_ver->majorVersion != 5 || $server_ver->minorVersion >= 5);
        $this->assertEquals(chr($server_ver->type), 'M');
        $this->assertEquals($engine_ver->majorVersion, transactd::TRANSACTD_VER_MAJOR);
        $this->assertEquals($engine_ver->minorVersion, transactd::TRANSACTD_VER_MINOR);
        $this->assertEquals(chr($engine_ver->type), 'T');
    }
    public function testInsert()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $this->assertEquals($tb->recordCount(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 1);
        $tb->setFV(FDI_NAME, 'kosaka');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        $db->beginTrn();
        $n = 1;
        $tb->seekLast();
        if ($tb->stat() == 0)
        { 
            $n = $tb->getFVint(FDI_ID) + 1;
        }
        $tb->beginBulkInsert(BULKBUFSIZE);
        for ($i = $n; $i <= (TEST_COUNT + $n); $i++)
        {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, "" . $i);
            $tb->insert();
        }
        $tb->commitBulkInsert();
        $this->assertEquals($tb->stat(), 0);
        $db->endTrn();
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testFind()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 10 and id < ' . TEST_COUNT, 1, 0);
        $v = 10;
        $tb->setFV(FDI_ID, $v);
        $tb->find(table::findForword);
        $i = $v;
        while ($i < TEST_COUNT)
        {
            $this->assertEquals($tb->stat(), 0);
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true); // 11 - 19
            $i = $i + 1;
        }
        // backforword
        $tb->clearBuffer();
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->find(table::findBackForword);
        $i = $v;
        while ($i >= 10)
        {
            $this->assertEquals($tb->stat(), 0);
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findPrev(true); // 11 - 19
            $i = $i - 1;
        }
        // out of filter range (EOF)
        $tb->clearBuffer();
        $v = TEST_COUNT;
        $tb->setFV(FDI_ID, $v);
        $tb->find(table::findForword);
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testFindNext()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 10 and id < ' . TEST_COUNT, 1, 0);
        $v = 10;
        $tb->setFV(FDI_ID, $v);
        $tb->seekGreater(true);
        $this->assertEquals($tb->getFVint(FDI_ID), $v);
        for ($i = $v + 1; $i <= (TEST_COUNT - 1); $i++)
        {
            $tb->findNext(true); // 11 - 19
            $this->assertEquals($tb->stat(), 0);
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testFindIn()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $q = new queryBase();
        $q->addInValue('10', true);
        $q->addInValue('300000');
        $q->addInValue('50');
        $q->addInValue('-1');
        $q->addInValue('80');
        $q->addInValue('5000');
        
        $tb->setQuery($q);
        $this->assertEquals($tb->stat(), 0);
        $tb->find();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 10);
        $tb->findNext();
        $this->assertEquals($tb->stat(), transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = 300000\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 50);
        $tb->findNext();
        $this->assertEquals($tb->stat(), transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = -1\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 80);
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 5000);
        $tb->findNext();
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        
        // Many params
        $q->addInValue('1', true);
        for($i = 2; $i <= 10000; $i++)
        {
            $q->addInValue(strval($i));
        }
        $tb->setQuery($q);
        $this->assertEquals($tb->stat(), 0);
        
        $tb->find();
        $i = 0;
        while($tb->stat() == 0)
        {
            $i++;
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true);
        }
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        $this->assertEquals($i, 10000);
        
        //LogicalCountLimit
        $q->select('id');
        $tb->setQuery($q);
        
        $tb->find();
        $i = 0;
        while ($tb->stat() == 0)
        {
            $i++;
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true);
        }
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        $this->assertEquals($i, 10000);
        
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetPercentage()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $vv = TEST_COUNT / 2 + 1;
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $per = $tb->getPercentage();
        $this->assertTrue(abs(5000 - $per) < 500); // 500 = 5%
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testMovePercentage()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekByPercentage(5000); // 50%
        $this->assertEquals($tb->stat(), 0);
        $v = $tb->getFVint(FDI_ID);
        $this->assertEquals($tb->stat(), 0);
        $this->assertTrue(abs(TEST_COUNT / 2 + 1 - $v) < FIVE_PERCENT_OF_TEST_COUNT);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetEqual()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $db->beginSnapshot();
        for ($i = 2; $i <= (TEST_COUNT + 1); $i++)
        {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->seek();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
        $db->endSnapshot();
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetNext()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $db->beginSnapshot();
        $vv = 2;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        for ($i = 3; $i <= (TEST_COUNT + 1); $i++)
        {
            $tb->seekNext();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
        $db->endSnapshot();
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetPrevious()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $db->beginSnapshot();
        $vv = TEST_COUNT + 1;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        for ($i = TEST_COUNT; $i >= 2; $i--)
        {
            $tb->seekPrev();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
        $tb->seekPrev();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
        $db->endSnapshot();
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetGreater()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $vv = TEST_COUNT * 3 / 4;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekGreater(true);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv + 1);
        $vv = $vv - FIVE_PERCENT_OF_TEST_COUNT;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekGreater(false);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv + 1);
        $tb->seekPrev();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetLessThan()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $vv = TEST_COUNT * 3 / 4;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekLessThan(true);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv + 1);
        $vv = $vv - FIVE_PERCENT_OF_TEST_COUNT;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekLessThan(false);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv - 1);
        $tb->seekPrev();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv - 2);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetFirst()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekFirst();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testGetLast()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekLast();
        $this->assertEquals($tb->getFVstr(FDI_NAME), '' . (TEST_COUNT + 2));
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testMovePosition()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $vv = TEST_COUNT * 3 / 4;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekLessThan(true);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        $ps = $tb->bookmark();
        $ps_vv = $vv;
        $this->assertEquals($tb->stat(), 0);
        $vv = $vv - FIVE_PERCENT_OF_TEST_COUNT;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seekLessThan(false);
        $this->assertEquals($tb->getFVint(FDI_ID), $vv - 1);
        $tb->seekPrev();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv - 2);
        $tb->seekByBookmark($ps);
        $this->assertEquals($tb->getFVint(FDI_ID), $ps_vv);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testUpdate()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $db->beginTrn();
        // test of ncc
        $v = 5;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $v = TEST_COUNT + TEST_COUNT / 2;
        $tb->setFV(FDI_ID, $v);
        $tb->update(table::changeCurrentNcc); // 5 . 30000 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext(); // next 5
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $v);
        $v = 5;
        $tb->setFV(FDI_ID, $v);
        $tb->update(table::changeCurrentCc);  // 19999 . 5 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->update(table::changeCurrentCc); // 6 . 19999 cur 19999
        $tb->seekPrev(); // prev 19999
        $this->assertEquals($tb->getFVint(FDI_ID), $v -1);
        $v = 10;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 11);
        for ($i = 10; $i <= (TEST_COUNT - 2); $i++)
        {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->seek();
            $this->assertEquals($tb->stat(), 0);
            $v = $i + 1;
            $tb->setFV(FDI_NAME, $v);
            $tb->update();
            $this->assertEquals($tb->stat(), 0);
        }
        $db->endTrn();
        // check update in key;
        $v = 8;
        $tb->setFV(FDI_ID, $v);
        $tb->setFV(FDI_NAME, 'ABC');
        $tb->update(table::changeInKey);
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABC');
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testSnapShot()
    {
        $db  = $this->getDbObj();
        $db2 = $this->getDbObj();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, NULL);
        $db->beginSnapshot();
        $this->assertEquals($db->stat(), 0);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $firstValue = $tb->getFVstr(FDI_NAME);
        $tb->seekNext();
        // ----------------------------------------------------
        //   Change data by another connection
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, $tb2->getFVint(FDI_ID) + 1);
        $tb2->update();
        if (ISOLATION_READ_COMMITTED)
            $this->assertEquals($tb2->stat(), 0);
        elseif (ISOLATION_REPEATABLE_READ)
            $this->assertEquals($tb2->stat(), transactd::STATUS_LOCK_ERROR);
        // ----------------------------------------------------
        $tb->seekFirst();
        $secondValue = $tb->getFVstr(FDI_NAME);
        $this->assertEquals($tb->stat(), 0);
        $db->endSnapshot();
        $this->assertEquals($tb->stat(), 0);
        if (ISOLATION_READ_COMMITTED)
          $this->assertNotEquals($secondValue, $firstValue);
        else
          $this->assertEquals($secondValue, $firstValue);
        // ----------------------------------------------------
        $tb->close();
        $tb2->close();
        $this->deleteDbObj($db);
        $this->deleteDbObj($db2);
    }
    public function testTransactionLock()
    {
        $db  = $this->getDbObj();
        $db2 = $this->getDbObj();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, NULL);
        // ----------------------------------------------------
        //  Read test that single record lock with read
        // ----------------------------------------------------
        $db->beginTrn(transactd::LOCK_SINGLE_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        // unlock first record
        $tb->seekNext();
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $db2->endTrn();
        $db->endTrn();
        // ----------------------------------------------------
        //  Can't read test that multi record lock with read
        // ----------------------------------------------------
        $db->beginTrn(transactd::LOCK_MULTI_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        // move from first record.
        $tb->seekNext();
        // not transactional user can not read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), transactd::STATUS_LOCK_ERROR);
        // The second transactional user can not lock same record
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        // ----------------------------------------------------
        //  Can't read test that single record lock with change
        // ----------------------------------------------------
        $db->beginTrn(transactd::LOCK_SINGLE_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_NAME, 'ABC');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        // move from first record.
        $tb->seekNext();
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), transactd::STATUS_LOCK_ERROR);
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        // ----------------------------------------------------
        //  Abort test that Single record lock transaction
        // ----------------------------------------------------
        $db->beginTrn(transactd::LOCK_SINGLE_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_NAME, 'EFG');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        // move from first record.
        $tb->seekNext();
        $db->abortTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->getFVstr(FDI_NAME), 'ABC');
        $tb->close();
        $tb2->close();
        $this->deleteDbObj($db);
        $this->deleteDbObj($db2);
    }
    public function testConflict()
    {
        $db  = $this->getDbObj();
        $db2 = $this->getDbObj();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, NULL);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        // ----------------------------------------------------
        //  Change Index field
        // ----------------------------------------------------
        // Change data by another connection
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_ID, $tb2->getFVint(FDI_ID) - 10);
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        // ----------------------------------------------------
        // Change same record data by original connection
        $tb->setFV(FDI_ID, $tb->getFVint(FDI_ID) - 8);
        $tb->update();
        $this->assertEquals($tb->stat(), transactd::STATUS_CHANGE_CONFLICT);
        // ----------------------------------------------------
        //  Change Non index field
        // ----------------------------------------------------
        // Change data by another connection
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, $tb2->getFVint(FDI_ID) - 10);
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        // ----------------------------------------------------
        // Change same record data by original connection
        $tb->setFV(FDI_NAME, $tb->getFVint(FDI_NAME) - 8);
        $tb->update();
        $this->assertEquals($tb->stat(), transactd::STATUS_CHANGE_CONFLICT);
        // ----------------------------------------------------
        $tb->close();
        $tb2->close();
        $this->deleteDbObj($db);
        $this->deleteDbObj($db2);
    }
    public function testInsert2()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $v = TEST_COUNT * 2;
        $db->beginTrn();
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        $v = 10;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 11);
        $db->endTrn();
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testDelete()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        // estimate count
        $count = $tb->recordCount(true);
        $is_valid_count = (abs($count - TEST_COUNT - 3) < FIVE_PERCENT_OF_TEST_COUNT);
        $this->assertTrue($is_valid_count);
        if (! $is_valid_count)
          print("true record count = " . (TEST_COUNT + 3) . " and estimate recordCount count = " . $count);
        $this->assertEquals($tb->recordCount(false), TEST_COUNT + 3); // true count
        $vv = TEST_COUNT * 3 / 4 + 1;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        $tb->del();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->stat(), 4);
        // check update in key
        $vv = 8;
        $tb->setFV(FDI_ID, $vv);
        $tb->del(true);
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->stat(), transactd::STATUS_NOT_FOUND_TI);
        $db->beginTrn();
        $tb->stepFirst();
        while ($tb->stat() == 0)
        {
            $tb->del();
            $this->assertEquals($tb->stat(), 0);
            $tb->stepNext();
        }
        $this->assertEquals($tb->stat(), 9);
        $db->endTrn();
        $this->assertEquals($tb->recordCount(false), 0);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testSetOwner()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setOwnerName("ABCDEFG");
        $this->assertEquals($tb->stat(), 0);
        $tb->clearOwnerName();
        $this->assertEquals($tb->stat(), 0);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testDropIndex()
    {
        $db = $this->getDbObj();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->dropIndex(false);
        $this->assertEquals($tb->stat(), 0);
        $tb->close();
        $this->deleteDbObj($db);
    }
    public function testDropDatabase()
    {
        $db = $this->getDbObj();
        $this->dropDatabase($db);
        $this->deleteDbObj($db);
    }
    public function testLogin()
    {
        $db = $this->getDbObj();
        $db->connect(PROTOCOL . HOSTNAME);
        $this->assertEquals($db->stat(), 0);
        if ($db->stat() == 0)
        {
            // second connection
            $db2 = $this->getDbObj();
            $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
            $this->assertEquals($db->stat(), 0);
            $this->deleteDbObj($db2);
            $db->disconnect(PROTOCOL . HOSTNAME);
            $this->assertEquals($db->stat(), 0);
        }
        // invalid host name
        $db->connect(PROTOCOL . 'localhost123/');
        $is_valid_stat = ($db->stat() == transactd::ERROR_TD_INVALID_CLINETHOST) || 
                         ($db->stat() == transactd::ERROR_TD_HOSTNAME_NOT_FOUND);
        $this->assertTrue($is_valid_stat);
        if (! $is_valid_stat)
            print('bad host $db->stat() = ' . $db->stat());
        $this->createDatabase($db);
        $this->createTable($db);
        $db->disconnect(PROTOCOL . HOSTNAME . DBNAME);
        $this->assertEquals($db->stat(), 0);
        // true database name
        $db->connect(PROTOCOL . HOSTNAME . DBNAME);
        $this->assertEquals($db->stat(), 0);
        if ($db->stat() == 0)
        {
            $db->disconnect(PROTOCOL . HOSTNAME . DBNAME);
            $this->assertEquals($db->stat(), 0);
        }
        // invalid database name
        $this->dropDatabase($db);
        $db->disconnect(PROTOCOL . HOSTNAME . DBNAME);
        $this->assertEquals($db->stat(), 0);
        $db->connect(PROTOCOL . HOSTNAME . DBNAME);
        $this->assertEquals($db->stat(), 25000 + 1049);
        $db->disconnect(PROTOCOL . HOSTNAME . DBNAME);
        $this->assertEquals($db->stat(), 0);
        $this->deleteDbObj($db);
    }
    
    /* -----------------------------------------------------
        transactd var tables
    ----------------------------------------------------- */
    
    private function isWindows()
    {
        return (strtolower(substr(PHP_OS, 0, 3)) == 'win');
    }
    private function isUtf16leSupport($db)
    {
        // CHARSET_UTF16LE supported on MySQL 5.6 or later
        $vv = new btrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        if ('M' == chr($server_ver->type))
        {
          if ($server_ver->majorVersion <= 4)
            return false;
          elseif ($server_ver->majorVersion == 5 && $server_ver->minorVersion <= 5)
            return false;
          return true;
        }
        return false;
    }
    private function createVarTable($db, $id, $name, $fieldType, $charset)
    {
        // create table
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, NULL);
        $td = new tabledef();
        $td->setTableName($name);
        $td->setFileName($name . '.dat');
        $td->id = $id;
        $td->keyCount = 0;
        $td->fieldCount = 0;
        $td->flags->all = 0;
        $td->pageSize = 2048;
        $td->charsetIndex = $charset;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // name
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = $fieldType;
        if ($fieldType == transactd::ft_mywvarchar)
            $fd->len = 1 + transactd::charsize(transactd::CHARSET_UTF16LE) * 3; // max 3 char len byte
        elseif ($fieldType == transactd::ft_mywvarbinary)
            $fd->len = 1 + transactd::charsize(transactd::CHARSET_UTF16LE) * 3; // max 6 char len byte
        elseif ($fieldType == transactd::ft_myvarchar)
        {
            if ($charset == transactd::CHARSET_CP932)
                $fd->len = 1 + transactd::charsize(transactd::CHARSET_CP932) * 3;  // max 6 char len byte
            elseif($charset == transactd::CHARSET_UTF8B4)
                $fd->len = 1 + transactd::charsize(transactd::CHARSET_UTF8B4) * 3; // max 6 char len byte
        }
        else
            $fd->len = 7; // max 6 char len byte
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // groupid
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('groupid');
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // key 1
        $kd = $dbdef->insertKey($id, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // key 2
        $kd = $dbdef->insertKey($id, 1);
        $kd->segment(0)->fieldNum = 1;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segment(0)->flags->bit0 = 1;  // duplicateable
        $kd->segment(0)->flags->bit4 = 1;  // not last segmnet
        $kd->segment(1)->fieldNum = 2;
        $kd->segment(1)->flags->bit8 = 1;  // extended key type
        $kd->segment(1)->flags->bit1 = 1;  // changeable
        $kd->segment(1)->flags->bit0 = 1;  // duplicateable
        $kd->segmentCount = 2;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // open
        $tb = $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        $tb->close();
    }
    
    public function testCreateDatabaseVar()
    {
        $db = $this->getDbObj();
        $db->create(URL_VAR);
        if ($db->stat() == transactd::STATUS_TABLE_EXISTS_ERROR)
        {
          $this->testDropDatabaseVar();
          $db->create(URL_VAR);
        }
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $db->open(URL_VAR, 0, 0);
            $this->assertEquals($db->stat(), 0);
        }
        if (0 == $db->stat())
        {
            $this->createVarTable($db, 1, 'user1', transactd::ft_myvarchar,   transactd::CHARSET_CP932);
            $this->createVarTable($db, 2, 'user2', transactd::ft_myvarbinary, transactd::CHARSET_CP932);
            if ($this->isUtf16leSupport($db))
                $this->createVarTable($db, 3, 'user3', transactd::ft_mywvarchar,  transactd::CHARSET_CP932);
            $this->createVarTable($db, 4, 'user4', transactd::ft_mywvarbinary,    transactd::CHARSET_CP932);
            $this->createVarTable($db, 5, 'user5', transactd::ft_myvarchar,       transactd::CHARSET_UTF8B4);
            $db->close();
            $db->open(URL_VAR);
            $this->assertEquals($db->stat(), 0);
        }
        $this->deleteDbObj($db);
    }
    private function setGetVar($tb, $unicodeField, $varCharField)
    {
        //// Set Wide Get Wide
        //if ($this->isWindows())
        //{
        //    $tb->setFVW(FDI_GROUP, '68');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //}
        //else
        //{
            $tb->setFV(FDI_GROUP, '68');
            $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        //}
        //if ($this->isWindows())
        //{
        //    // too long string
        //    $tb->setFVW(FDI_NAME, '1234567');
        //    if ($varCharField)
        //    {
        //        $this->assertEquals($tb->getFVWstr(FDI_NAME), '123');
        //        if ($tb->getFVWstr(FDI_NAME) != '123')
        //            print_r($tb->getFVWstr(FDI_NAME));
        //    }
        //    else
        //    {
        //        $this->assertEquals($tb->getFVWstr(FDI_NAME), '123456');
        //    }
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // short string;
        //    $tb->setFVW(FDI_NAME, '12 ');
        //    $this->assertEquals($tb->getFVWstr(FDI_NAME), '12 ');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // too long kanji;
        //    if ($unicodeField)
        //    {
        //        $tb->setFVW(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' + kanji 'hokke'
        //        if ($varCharField)
        //            $this->assertEquals($tb->getFVWstr(FDI_NAME), 'あいう');
        //        else
        //            $this->assertEquals($tb->getFVWstr(FDI_NAME), 'あいうえお');
        //    }
        //    else
        //    {
        //        $tb->setFVW(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
        //        $this->assertEquals($tb->getFVWstr(FDI_NAME), '0松本');
        //    }
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //}
        // Set Ansi Get Wide
        // too long string
        $tb->setFVA(FDI_NAME, '1234567');
        if ($varCharField)
            $this->assertEquals($tb->getFVAstr(FDI_NAME), '123');
        else
            $this->assertEquals($tb->getFVAstr(FDI_NAME), '123456');
        //if ($this->isWindows())
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //else
            $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
        // short string
        $tb->setFVA(FDI_NAME, '13 ');
        $this->assertEquals($tb->getFVAstr(FDI_NAME), '13 ');
        //if ($this->isWindows())
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //else
            $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
        // too long kanji
        if ($unicodeField)
        {
            if ($this->isWindows())
            {
                $tb->setFVA(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
                if ($varCharField)
                    $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいう');
                else
                    $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいうえお');
            }
        }
        else
        {
            $tb->setFVA(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
            $is_valid_value = ($tb->getFVAstr(FDI_NAME) == '0松本');
            $this->assertTrue($is_valid_value);
            if (! $is_valid_value)
                print_r($tb->getFVAstr(FDI_NAME));
        }
        $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
        //// Set Wide Get Ansi
        //if ($this->isWindows())
        //{
        //    // too long string
        //    $tb->setFVW(FDI_NAME, '1234567');
        //    if ($varCharField)
        //        $this->assertEquals($tb->getFVAstr(FDI_NAME), '123');
        //    else
        //        $this->assertEquals($tb->getFVAstr(FDI_NAME), '123456');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // short string
        //    $tb->setFVW(1, '23 ');
        //    $this->assertEquals($tb->getFVAstr(FDI_NAME), '23 ');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // too long kanji
        //    if ($unicodeField)
        //    {
        //        $tb->setFVW(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
        //        if ($varCharField)
        //            $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいう');
        //        else
        //            $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいうえお');
        //    }
        //    else
        //    {
        //        $tb->setFVW(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
        //        $this->assertEquals($tb->getFVAstr(FDI_NAME), '0松本');
        //    }
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //}
        // Set Ansi Get Ansi
        // too long string
        $tb->setFVA(FDI_NAME, '1234567');
        if ($varCharField)
            $this->assertEquals($tb->getFVAstr(FDI_NAME), '123');
        else
            $this->assertEquals($tb->getFVAstr(FDI_NAME), '123456');
        $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
        // short string
        $tb->setFVA(FDI_NAME, '13 ');
        $this->assertEquals($tb->getFVAstr(FDI_NAME), '13 ');
        $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
        // too long lanji
        if ($unicodeField)
        {
            if ($this->isWindows())
            {
                $tb->setFVA(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
                if ($varCharField)
                    $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいう');
                else
                    $this->assertEquals($tb->getFVAstr(FDI_NAME), 'あいうえお');
            }
        }
        else
        {
            $tb->setFVA(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
            $this->assertEquals($tb->getFVAstr(FDI_NAME), '0松本');
        }
        $this->assertEquals($tb->getFVAstr(FDI_GROUP), '68');
    }
    public function testVarField()
    {
        $db = $this->getDbObj();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable('user1');
        $this->assertEquals($db->stat(), 0);
        // acp varchar
        $this->setGetVar($tb, false, true);
        $tb->close();
        $tb = $db->openTable('user2');
        $this->assertEquals($db->stat(), 0);
        // acp varbinary
        $this->setGetVar($tb, false, false);
        $tb->close();
        if ($this->isUtf16leSupport($db))
        {
            $tb = $db->openTable('user3');
            $this->assertEquals($db->stat(), 0);
            // unicode varchar
            $this->setGetVar($tb, true, true);
            $tb->close();
        }
        $tb = $db->openTable('user4');
        $this->assertEquals($db->stat(), 0);
        // unicode varbinary
        $this->setGetVar($tb, true, false);
        $tb->close();
        $tb = $db->openTable('user5');
        $this->assertEquals($db->stat(), 0);
        // utf8 varchar
        $this->setGetVar($tb, true, true);
        $tb->close();
        $this->deleteDbObj($db);
    }
    private function doVarInsert($db, $name, $codePage, $str, $startid, $endid, $bulk)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        if ($bulk)
            $tb->beginBulkInsert(BULKBUFSIZE);
        for ($i = $startid; $i <= $endid; $i++)
        {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, $str . $i);
            $tb->setFV(FDI_GROUP, "" . ($i + 10));
            $tb->insert();
        }
        if ($bulk)
            $tb->commitBulkInsert();
        $this->assertEquals($tb->stat(), 0);
        $tb->close();
    }
    public function testVarInsert()
    {
        $db = $this->getDbObj();
        $startid = 1;
        $bulk = false;
        $str = '漢字文字のテスト'; // too long kanji
        $str2 = '123';
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $utf16leSupport = $this->isUtf16leSupport($db);
            $this->doVarInsert($db, 'user1', transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user4', transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', transactd::CP_UTF8,  $str, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $this->doVarInsert($db, 'user1', transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user4', transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', transactd::CP_UTF8,  $str2, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $bulk = true;
            $endid = 1000;
            $this->doVarInsert($db, 'user1', transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user2', transactd::CP_ACP,   '', $startid, $endid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user4', transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user5', transactd::CP_UTF8,  '', $startid, $endid, $bulk);
        }
        $this->deleteDbObj($db);
    }
    private function doVarRead($db, $name, $codePage, $str, $num, $ky)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        $tb->setKeyNum($ky);
        if ($ky == 0)
        {
            $tb->setFV(FDI_ID, $num);
        }
        else
        {
            $v = $num + 10;
            $tb->setFV(FDI_NAME, $str);
            $tb->setFV(FDI_GROUP, $v);
        }
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        // test read of var field
        $is_valid_value = ($str == $tb->getFVstr(FDI_NAME));
        $this->assertTrue($is_valid_value);
        // test read of second field
        $this->assertEquals($tb->getFVint(FDI_GROUP), ($num + 10));
        $tb->close();
    }
    public function testVarRead()
    {
        $db = $this->getDbObj();
        $str = '漢字文';
        $str3 = '漢字文字のテ';
        $str2 ='123';
        $str4 ='1232';
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $utf16leSupport = $this->isUtf16leSupport($db);
            $num = 1;
            $ky = 0;
            // too long string
            $this->doVarRead($db, 'user1', transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarRead($db, 'user2', transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarRead($db, 'user4', transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarRead($db, 'user5', transactd::CP_UTF8,  $str,  $num, $ky);
            // short string
            $num = $num + 1;
            $this->doVarRead($db, 'user1', transactd::CP_ACP,   $str2, $num, $ky);
            $this->doVarRead($db, 'user2', transactd::CP_ACP,   $str4, $num, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', transactd::CP_ACP,   $str2, $num, $ky);
            $this->doVarRead($db, 'user4', transactd::CP_ACP,   $str4, $num, $ky);
            $this->doVarRead($db, 'user5', transactd::CP_UTF8,  $str2, $num, $ky);
            $ky = 1;
            $this->doVarRead($db, 'user1', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user2', transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user4', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user5', transactd::CP_UTF8,  '120', 120, $ky);
        }
        $this->deleteDbObj($db);
    }
    private function doVarFilter($db, $name, $codePage, $str, $num, $ky)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        $tb->setKeyNum($ky);
        if ($ky == 0)
        {
            $buf = 'id > ' . $num . ' and id <= ' . ($num + 10);
            $tb->setFilter($buf, 0, 10);
            // find forword
            $tb->setFV(FDI_ID, $num);
            $tb->seekGreater(true);
            $this->assertEquals($tb->stat(), 0);
            for ($i = ($num + 1); $i <= ($num + 10); $i++)
            {
                $tb->findNext();
                $this->assertEquals($tb->stat(), 0);
                // test read of var field
                $this->assertEquals($tb->getFVint(FDI_NAME), $i);
                // test read of second field
                $this->assertEquals($tb->getFVint(FDI_GROUP), $i + 10);
            }
            // find previous
            $v = $num + 10;
            $tb->setFilter($buf, 0, 10);
            $tb->setFV(FDI_ID, $v);
            $tb->seekLessThan(true);
            $this->assertEquals($tb->stat(), 0);
            $this->assertEquals($tb->getFVint(FDI_ID), $v);
            for ($i = $num + 10; $i <= $num + 1; $i--)
            {
                $tb->findPrev(false);
                $this->assertEquals($tb->stat(), 0);
                // test read of var field
                $this->assertEquals($tb->getFVint(FDI_NAME), $i);
                // test read of second field
                $this->assertEquals($tb->getFVint(FDI_GROUP), $i + 10);
            }
            // test record count
            $this->assertEquals($tb->recordCount(), 10);
        }
        else
        {
            $v = $num + 10;
            $tb->setFV(FDI_NAME, $str);
            $tb->setFV(FDI_GROUP, $v);
        }
        $tb->close();
    }
    public function testFilterVar()
    {
        $db = $this->getDbObj();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $str = '漢字文';
            $str3 = '漢字文字のテ';
            $str2 = '123';
            $str4 = '1232';
            $utf16leSupport = $this->isUtf16leSupport($db);
            $num = 10;
            $ky = 0;
            $this->doVarFilter($db, 'user1', transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarFilter($db, 'user2', transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport)
                $this->doVarFilter($db, 'user3', transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarFilter($db, 'user4', transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarFilter($db, 'user5', transactd::CP_UTF8,  $str,  $num, $ky);
            //if (UNICODE)
            //{
            //    // short string
            //    $num = $num + 1;
            //    $this->doVarFilter($db, 'user1', transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user2', transactd::CP_ACP,  $str4, $num, $ky);
            //    if ($utf16leSupport)
            //        $this->doVarFilter($db, 'user3', transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user4', transactd::CP_ACP,  $str4, $num, $ky);
            //    $this->doVarFilter($db, 'user5', transactd::CP_UTF8, $str2, $num, $ky);
            //}
            $ky = 1;
            $this->doVarFilter($db, 'user1', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user2', transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport)
                $this->doVarFilter($db, 'user3', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user4', transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user5', transactd::CP_UTF8,  '120', 120, $ky);
        }
        $this->deleteDbObj($db);
    }
    public function testDropDatabaseVar()
    {
        $db = $this->getDbObj();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
        $this->deleteDbObj($db);
    }
    
    /* -----------------------------------------------------
        transactd StringFilter
    ----------------------------------------------------- */
    
    private function createTableStringFilter($db, $id, $name, $type, $type2)
    {
        // create table
        $dbdef = $db->dbDef();
        $td = new tabledef();
        $td->setTableName($name);
        $td->setFileName($name . '.dat');
        $td->id = $id;
        $td->pageSize = 2048;
        $td->charsetIndex = transactd::CHARSET_UTF8B4;
        // $td->charsetIndex = transactd::CHARSET_CP932;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = $type;
        $fd->len = 44;
        if ($fd->varLenBytes() != 0)
        {
            $fd->len = $fd->varLenBytes() + 44;
            $fd->keylen = $fd->len;
        }
        if ($fd->blobLenBytes() != 0)
            $fd->len = 12; // 8+4
        $fd->keylen = $fd->len;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('namew');
        $fd->type = $type2;
        $fd->len = 44;
        if ($fd->varLenBytes() != 0)
        {
            $fd->len = $fd->varLenBytes() + 44;
            $fd->keylen = $fd->len;
        }
        if ($fd->blobLenBytes() != 0)
            $fd->len = 12; // 8+4
        $fd->keylen = $fd->len;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $kd = $dbdef->insertKey($id, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $kd = $dbdef->insertKey($id, 1);
        $kd->segment(0)->fieldNum = 1;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segment(0)->flags->bit0 = 1;  // duplicateable
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $kd = $dbdef->insertKey($id, 2);
        $kd->segment(0)->fieldNum = 2;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segment(0)->flags->bit0 = 1;  // duplicateable
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
    }
    private function doTestInsertStringFilter($tb)
    {
        $tb->beginBulkInsert(BULKBUFSIZE);
        $tb->clearBuffer();
        $id = 1;
        $tb->setFV('id', $id);
        $tb->setFV('name', 'あいうえおかきくこ');
        $tb->setFV('namew', 'あいうえおかきくこ');
        $tb->insert();
        $tb->clearBuffer();
        $id = 2;
        $tb->setFV('id', $id);
        $tb->setFV('name', 'A123456');
        $tb->setFV('namew', 'A123456');
        $tb->insert();
        $tb->clearBuffer();
        $id = 3;
        $tb->setFV('id', $id);
        $tb->setFV('name', 'あいがあればOKです');
        $tb->setFV('namew', 'あいがあればOKです');
        $tb->insert();
        $tb->clearBuffer();
        $id = 4;
        $tb->setFV('id', $id);
        $tb->setFV('name', 'おはようございます');
        $tb->setFV('namew', 'おはようございます');
        $tb->insert();
        $tb->clearBuffer();
        $id = 5;
        $tb->setFV('id', $id);
        $tb->setFV('name', 'おめでとうございます。');
        $tb->setFV('namew', 'おめでとうございます。');
        $tb->insert();
        $tb->commitBulkInsert();
    }
    private function doTestReadStringFilter($tb)
    {
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $id = 1;
        $tb->setFV('id', $id);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいうえおかきくこ');
        $id =3;
        $tb->setFV('id', $id);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいがあればOKです');
        $tb->setKeyNum(1);
        $tb->clearBuffer();
        $tb->setFV('name', 'A123456');
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'A123456');
        $tb->setKeyNum(2);
        $tb->clearBuffer();
        $tb->setFV('namew', 'A123456');
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'A123456');
    }
    private function doTestSetStringFilter($tb)
    {
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        
        $tb->setFilter("name = 'あい*'", 0, 10);
        $this->assertEquals($tb->stat(), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->findNext(false);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいうえおかきくこ');
        $this->assertEquals($tb->recordCount(), 2);
        
        $tb->setFilter("name <> 'あい*'", 0, 10);
        $this->assertEquals($tb->recordCount(), 3);
        $tb->clearBuffer();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->findNext(false);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'A123456');
        
        $tb->findNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'おはようございます');
        
        $tb->findNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'おめでとうございます。');
        
        $tb->findNext();
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        
        $tb->clearBuffer();
        $tb->seekLast();
        $tb->findPrev(false);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'おめでとうございます。');
        
        $tb->findPrev();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'おはようございます');
        
        $tb->findPrev(false);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'A123456');
        
        $tb->findPrev();
        $this->assertEquals($tb->stat(), transactd::STATUS_EOF);
        
        $tb->setFilter("name = 'あい'", 0, 10);
        $this->assertEquals($tb->recordCount(), 0);
        
        $tb->setFilter("name <> ''", 0, 10);
        $this->assertEquals($tb->recordCount(), 5);
        
        // testing that setFilter don't change field value
        $tb->clearBuffer();
        $tb->setFV('name', 'ABCDE');
        $tb->setFilter("name = 'あい'", 0, 10);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABCDE');
    }
    private function doTestUpdateStringFilter($tb)
    {
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV('name', 'ABCDE');
        $tb->setFV('namew', 'ABCDEW');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->setFV('name', 'ABCDE2');
        $tb->setFV('namew', 'ABCDEW2');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABCDE');
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'ABCDEW');
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABCDE2');
        $this->assertEquals($tb->getFVstr(FDI_NAMEW), 'ABCDEW2');
    }
    private function doTestStringFilter($db, $id, $name, $type, $type2)
    {
        $this->createTableStringFilter($db, $id, $name, $type, $type2);
        $tb = $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        $this->doTestInsertStringFilter($tb);
        $this->doTestReadStringFilter($tb);
        $this->doTestSetStringFilter($tb);
        $this->doTestUpdateStringFilter($tb);
        $tb->close();
    }
    
    public function testStringFilter()
    {
        $db = $this->getDbObj();
        $db->create(URL_SF);
        if ($db->stat() == transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->testDropDatabaseStringFilter();
            $db->create(URL_SF);
        }
        $this->assertEquals($db->stat(), 0);
        $db->open(URL_SF, 0, 0);
        $this->assertEquals($db->stat(), 0);
        $this->doTestStringFilter($db, 1, 'zstring', transactd::ft_zstring, transactd::ft_wzstring);
        if ($this->isUtf16leSupport($db))
            $this->doTestStringFilter($db, 2, 'myvarchar', transactd::ft_myvarchar, transactd::ft_mywvarchar);
        else
            $this->doTestStringFilter($db, 2, 'myvarchar', transactd::ft_myvarchar, transactd::ft_myvarchar);
        $this->doTestStringFilter($db, 3, 'mytext', transactd::ft_mytext, transactd::ft_myblob);
        $this->deleteDbObj($db);
    }
    
    public function testDropDatabaseStringFilter()
    {
        $db = $this->getDbObj();
        $db->open(URL_SF);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
        $this->deleteDbObj($db);
    }
    
    public function testQuery()
    {
        $q = new queryBase();
        $q->queryString("id = 0 and name = 'Abc efg'");
        $this->assertEquals($q->toString(), "id = '0' and name = 'Abc efg'");
        
        $q->queryString('');
        $q->where('id', '=', '0')->andWhere('name', '=', 'Abc efg');
        $this->assertEquals($q->toString(), "id = '0' and name = 'Abc efg'");
        
        $q->queryString("select id,name id = 0 AND name = 'Abc&' efg'");
        $this->assertEquals($q->toString(), "select id,name id = '0' AND name = 'Abc&' efg'");
        
        $q->queryString('');
        $q->select('id', 'name')->where('id', '=', '0')->andWhere('name', '=', "Abc' efg");
        $this->assertEquals($q->toString(), "select id,name id = '0' and name = 'Abc&' efg'");
        
        $q->queryString("select id,name id = 0 AND name = 'Abc&& efg'");
        $this->assertEquals($q->toString(), "select id,name id = '0' AND name = 'Abc&& efg'");
        
        $q->queryString('');
        $q->select('id', 'name')->where('id', '=', '0')->andWhere('name', '=', 'Abc& efg');
        $this->assertEquals($q->toString(), "select id,name id = '0' and name = 'Abc&& efg'");
        
        $q->queryString('*');
        $this->assertEquals($q->toString(), '*');
        
        $q->all();
        $this->assertEquals($q->toString(), '*');
        
        $q->queryString('Select id,name id = 2');
        $this->assertEquals($q->toString(), "select id,name id = '2'");
        
        $q->queryString('');
        $q->select('id', 'name')->where('id', '=', '2');
        $this->assertEquals($q->toString(), "select id,name id = '2'");
        
        $q->queryString('SELECT id,name,fc id = 2');
        $this->assertEquals($q->toString(), "select id,name,fc id = '2'");
        
        $q->queryString('');
        $q->select('id', 'name', 'fc')->where('id', '=', '2');
        $this->assertEquals($q->toString(), "select id,name,fc id = '2'");
        
        $q->queryString("select id,name,fc id = 2 and name = '3'");
        $this->assertEquals($q->toString(), "select id,name,fc id = '2' and name = '3'");
        
        $q->queryString('');
        $q->select('id', 'name', 'fc')->where('id', '=', '2')->andWhere('name', '=', '3');
        $this->assertEquals($q->toString(), "select id,name,fc id = '2' and name = '3'");
        
        // IN include
        $q->queryString("select id,name,fc IN '1','2','3'");
        $this->assertEquals($q->toString(), "select id,name,fc in '1','2','3'");
        
        $q->queryString('');
        $q->select('id', 'name', 'fc')->In('1', '2', '3');
        $this->assertEquals($q->toString(), "select id,name,fc in '1','2','3'");
        
        $q->queryString("IN '1','2','3'");
        $this->assertEquals($q->toString(), "in '1','2','3'");
        
        $q->queryString('IN 1,2,3');
        $this->assertEquals($q->toString(), "in '1','2','3'");
        
        $q->queryString('');
        $q->In('1', '2', '3');
        $this->assertEquals($q->toString(), "in '1','2','3'");
        
        //special field name
        $q->queryString('select = 1');
        $this->assertEquals($q->toString(), "select = '1'");
        
        $q->queryString('');
        $q->where('select', '=', '1');
        $this->assertEquals($q->toString(), "select = '1'");
        
        $q->queryString('in <> 1');
        $this->assertEquals($q->toString(), "in <> '1'");
        
        $q->queryString('');
        $q->where('in', '<>', '1');
        $this->assertEquals($q->toString(), "in <> '1'");
    }
    
    /* -----------------------------------------------------
        transactd convert
    ----------------------------------------------------- */
    
    public function testConvert()
    {
        if (! $this->isWindows())
        {
            $enc_u8 = 'UTF-8';
            
            $u8 = mb_convert_encoding('123', $enc_u8);
            $ret = transactd::u8tombc($u8, -1, '', 256);
            $this->assertEquals($u8, $ret);
            
            $mbcKanji = [0x8A, 0xBF, 0x8E, 0x9A, 0x00];
            $u8 = mb_convert_encoding('漢字', $enc_u8);
            $ret = transactd::u8tombc($u8, -1, '', 256);
            for ($i = 0; $i < strlen($ret); $i++)
                $this->assertEquals(hexdec(bin2hex($ret{$i})), $mbcKanji[$i]);
            
            $mbc = $ret;
            $u8Kanji = [0xe6 ,0xbc ,0xa2 ,0xe5 ,0xad ,0x97];
            $ret = transactd::mbctou8($mbc, -1, '', 256);
            for ($i = 0; $i < strlen($ret); $i++)
                $this->assertEquals(hexdec(bin2hex($ret{$i})), $u8Kanji[$i]);
        }
    }
    
}
