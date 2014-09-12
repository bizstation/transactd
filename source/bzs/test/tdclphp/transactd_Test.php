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
use BizStation\Transactd as Bz;

define("HOSTNAME", "localhost/");
define("DBNAME", "test");
define("DBNAME_VAR", "testvar");
define("DBNAME_SF", "testString");
define("DBNAME_QT", "querytest");
define("TABLENAME", "user");
define("PROTOCOL", "tdap://");
define("BDFNAME", "?dbfile=test.bdf");
define("URL", PROTOCOL . HOSTNAME . DBNAME . BDFNAME);
define("URL_VAR", PROTOCOL . HOSTNAME . DBNAME_VAR . BDFNAME);
define("URL_SF", PROTOCOL . HOSTNAME . DBNAME_SF . BDFNAME);
define("URL_QT", PROTOCOL . HOSTNAME . DBNAME_QT . BDFNAME);
define("FDI_ID", 0);
define("FDI_NAME", 1);
define("FDI_GROUP", 2);
define("FDI_NAMEW", 2);

define("BULKBUFSIZE", 65535 - 1000);
define("TEST_COUNT", 20000);
define("FIVE_PERCENT_OF_TEST_COUNT", TEST_COUNT / 20);

define("ISOLATION_READ_COMMITTED", true);
define("ISOLATION_REPEATABLE_READ", false);

class transactdTest extends PHPUnit_Framework_TestCase
{
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
        if ($db->stat() == Bz\transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->dropDatabase($db);
            $db->create(URL);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db)
    {
        $db->open(URL, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
    }
    private function createTable($db)
    {
        $this->openDatabase($db);
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, NULL);
        $td = new Bz\tabledef();
        // Set table schema codepage to UTF-8
        //     - codepage for field NAME and tableNAME
        $td->schemaCodePage = Bz\transactd::CP_UTF8;
        $td->setTableName(TABLENAME);
        $td->setFileName(TABLENAME . '.dat');
        // Set table default charaset index
        //    - default charset for field VALUE
          $td->charsetIndex = Bz\transactd::charsetIndex(Bz\transactd::CP_UTF8);
        //
        $tableid = 1;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 0);
        $fd->setName("id");
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 1);
        $fd->setName("name");
        $fd->type = Bz\transactd::ft_zstring;
        $fd->len = 33;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // Set field charset index
        //    - charset for each field VALUE
        //  $fd->setCharsetIndex(Bz\transactd::charsetIndex(Bz\transactd::CP_UTF8))
        
        $fd = $dbdef->insertField($tableid, 2);
        $fd->setName("select");
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 3);
        $fd->setName("in");
        $fd->type = Bz\transactd::ft_integer;
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
        $db = new Bz\database();
        $this->createDatabase($db);
    }
    public function testCreateTable()
    {
        $db = new Bz\database();
        $this->createTable($db);
    }
    // open database, not call close explicitly
    public function testOpenDatabase()
    {
        $db = new Bz\database();
        $db->open(URL);
    }
    // open database, call close explicitly
    public function testCloseDatabase()
    {
        $db = new Bz\database();
        $db->open(URL);
        $db->close();
    }
    // open database, open table, not call close explicitly
    public function testOpenA()
    {
        $db = new Bz\database();
        $db->open(URL);
        $tb = $this->openTable($db);
    }
    // open database, open table, call close explicitly
    public function testOpenB()
    {
        $db = new Bz\database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->close();
        $db->close();
    }
    // open database, open table, call database::close explicitly
    public function testOpenC()
    {
        $db = new Bz\database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $db->close();
    }
    // open database, open table, call table::close explicitly
    public function testOpenD()
    {
        $db = new Bz\database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->close();
    }
    // open database, open table, call table::release explicitly
    public function testOpenE()
    {
        $db = new Bz\database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->release();
    }
    public function testClone()
    {
        $db = new Bz\database();
        $db->open(URL);
        $this->assertEquals($db->stat(), 0);
        $this->assertEquals($db->isOpened(), true);
        $db2 = clone $db;
        $this->assertEquals($db2->stat(), 0);
        $this->assertEquals($db2->isOpened(), true);
        //echo("\ndb->_cPtr  " . $db->_cPtr  . "\ndb2->_cPtr " . $db2->_cPtr . "\n");
        $db2->close();
        $this->assertEquals($db2->stat(), 0);
        $this->assertEquals($db2->isOpened(), false);
        unset($db2);
        $this->assertEquals($db->stat(), 0);
        $this->assertEquals($db->isOpened(), true);
    }
    public function testVersion()
    {
        $db = new Bz\database();
        $db->connect(PROTOCOL . HOSTNAME);
        $this->assertEquals($db->stat(), 0);
        $vv = new Bz\btrVersions();
        $db->getBtrVersion($vv);
        $this->assertEquals($db->stat(), 0);
        $client_ver = $vv->version(0);
        $server_ver = $vv->version(1);
        $engine_ver = $vv->version(2);
        $this->assertEquals($client_ver->majorVersion, Bz\transactd::CPP_INTERFACE_VER_MAJOR);
        $this->assertEquals($client_ver->minorVersion, Bz\transactd::CPP_INTERFACE_VER_MINOR);
        $this->assertEquals(chr($client_ver->type), 'N');
        $this->assertTrue($server_ver->majorVersion >= 5);
        $this->assertTrue($server_ver->majorVersion != 5 || $server_ver->minorVersion >= 5);
        $this->assertEquals(chr($server_ver->type), 'M');
        $this->assertEquals($engine_ver->majorVersion, Bz\transactd::TRANSACTD_VER_MAJOR);
        $this->assertEquals($engine_ver->minorVersion, Bz\transactd::TRANSACTD_VER_MINOR);
        $this->assertEquals(chr($engine_ver->type), 'T');
    }
    public function testInsert()
    {
        $db = new Bz\database();
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
    }
    public function testFind()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 10 and id < ' . TEST_COUNT, 1, 0);
        $v = 10;
        $tb->setFV(FDI_ID, $v);
        $tb->find(Bz\table::findForword);
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
        $tb->find(Bz\table::findBackForword);
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
        $tb->find(Bz\table::findForword);
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
    }
    public function testFindNext()
    {
        $db = new Bz\database();
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
    }
    public function testFindIn()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $q = new Bz\query();
        $q->addSeekKeyValue('10', true);
        $q->addSeekKeyValue('300000');
        $q->addSeekKeyValue('50');
        $q->addSeekKeyValue('-1');
        $q->addSeekKeyValue('80');
        $q->addSeekKeyValue('5000');
        
        $tb->setQuery($q);
        $this->assertEquals($tb->stat(), 0);
        $tb->find();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 10);
        $tb->findNext();
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = 300000\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 50);
        $tb->findNext();
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = -1\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 80);
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 5000);
        $tb->findNext();
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
        
        // Many params
        $q->addSeekKeyValue('1', true);
        for($i = 2; $i <= 10000; $i++)
        {
            $q->addSeekKeyValue(strval($i));
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
        $this->assertEquals($i, 10000);
    }
    public function testGetPercentage()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $vv = TEST_COUNT / 2 + 1;
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $per = $tb->getPercentage();
        $this->assertTrue(abs(5000 - $per) < 500); // 500 = 5%
    }
    public function testMovePercentage()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekByPercentage(5000); // 50%
        $this->assertEquals($tb->stat(), 0);
        $v = $tb->getFVint(FDI_ID);
        $this->assertEquals($tb->stat(), 0);
        $this->assertTrue(abs(TEST_COUNT / 2 + 1 - $v) < FIVE_PERCENT_OF_TEST_COUNT);
    }
    public function testGetEqual()
    {
        $db = new Bz\database();
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
    }
    public function testGetNext()
    {
        $db = new Bz\database();
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
    }
    public function testGetPrevious()
    {
        $db = new Bz\database();
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
    }
    public function testGetGreater()
    {
        $db = new Bz\database();
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
    }
    public function testGetLessThan()
    {
        $db = new Bz\database();
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
    }
    public function testGetFirst()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekFirst();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
    }
    public function testGetLast()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->seekLast();
        $this->assertEquals($tb->getFVstr(FDI_NAME), '' . (TEST_COUNT + 2));
    }
    public function testMovePosition()
    {
        $db = new Bz\database();
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
    }
    public function testUpdate()
    {
        $db = new Bz\database();
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
        $tb->update(Bz\table::changeCurrentNcc); // 5 . 30000 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext(); // next 5
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $v);
        $v = 5;
        $tb->setFV(FDI_ID, $v);
        $tb->update(Bz\table::changeCurrentCc);  // 19999 . 5 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->update(Bz\table::changeCurrentCc); // 6 . 19999 cur 19999
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
        $tb->update(Bz\table::changeInKey);
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABC');
    }
    public function testSnapShot()
    {
        $db  = new Bz\database();
        $db2 = new Bz\database();
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
            $this->assertEquals($tb2->stat(), Bz\transactd::STATUS_LOCK_ERROR);
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
    }
    public function testConflict()
    {
        $db  = new Bz\database();
        $db2 = new Bz\database();
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_CHANGE_CONFLICT);
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_CHANGE_CONFLICT);
        // ----------------------------------------------------
        $tb2->release();
        $tb->release();
        unset($db2);
        unset($db);
    }
    public function testTransactionLock()
    {
        $db  = new Bz\database();
        $db2 = new Bz\database();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, NULL);
        // ----------------------------------------------------
        //  Read test that single record lock with read
        // ----------------------------------------------------
        $db->beginTrn(Bz\transactd::LOCK_SINGLE_NOWAIT);
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
        $db->beginTrn(Bz\transactd::LOCK_MULTI_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        // move from first record.
        $tb->seekNext();
        // not transactional user can not read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Bz\transactd::STATUS_LOCK_ERROR);
        // The second transactional user can not lock same record
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Bz\transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        // ----------------------------------------------------
        //  Can't read test that single record lock with change
        // ----------------------------------------------------
        $db->beginTrn(Bz\transactd::LOCK_SINGLE_NOWAIT);
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_NAME, 'ABC');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        // move from first record.
        $tb->seekNext();
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Bz\transactd::STATUS_LOCK_ERROR);
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Bz\transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        // ----------------------------------------------------
        //  Abort test that Single record lock transaction
        // ----------------------------------------------------
        $db->beginTrn(Bz\transactd::LOCK_SINGLE_NOWAIT);
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
    }
    public function testExclusive()
    {
        // db mode exclusive
        $db = new Bz\database();
        $db->open(URL, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        
        // Can not open database from other connections.
        $db2 = new Bz\database();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $db2->open(URL, Bz\transactd::TYPE_SCHEMA_BDF);
        $this->assertEquals($db2->stat(), Bz\transactd::STATUS_CANNOT_LOCK_TABLE);
        
        $tb2 = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->setFV(FDI_NAME, 'ABC123');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, 'ABC124');
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb->close();
        $tb2->close();
        $db->close();
        $db2->close();
        
        // table mode exclusive
        $db = new Bz\database();
        $db->open(URL, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_READONLY);
        $this->assertEquals($db->stat(), 0);
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable(TABLENAME, Bz\transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        
        $db2 = new Bz\database();
        $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
        $this->assertEquals($db2->stat(), 0);
        $db2->open(URL, Bz\transactd::TYPE_SCHEMA_BDF);
        $this->assertEquals($db2->stat(), 0);
        
        // Can not open table from other connections.
        unset($tb2); // UNSET $tb2 BEFORE REASSIGNMENT
        $tb2 = $db2->openTable(TABLENAME);
        $this->assertEquals($db2->stat(), Bz\transactd::STATUS_CANNOT_LOCK_TABLE);
        
        // Can open table from the same connection.
        $tb3 = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        
        $tb->close();
        if ($tb2 != NULL) { $tb2->close(); }
        $tb3->close();
        $db->close();
        $db2->close();
        
        // reopen and update
        $db = new Bz\database();
        $db->open(URL);
        $this->assertEquals($db->stat(), 0);
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->setFV(FDI_NAME, 'ABC123');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
    }
    public function testInsert2()
    {
        $db = new Bz\database();
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
    }
    public function testDelete()
    {
        $db = new Bz\database();
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_NOT_FOUND_TI);
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
    }
    public function testSetOwner()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->setOwnerName("ABCDEFG");
        $this->assertEquals($tb->stat(), 0);
        $tb->clearOwnerName();
        $this->assertEquals($tb->stat(), 0);
    }
    public function testDropIndex()
    {
        $db = new Bz\database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, NULL);
        $tb->dropIndex(false);
        $this->assertEquals($tb->stat(), 0);
    }
    public function testDropDatabase()
    {
        $db = new Bz\database();
        $this->dropDatabase($db);
    }
    public function testLogin()
    {
        $db = new Bz\database();
        $db->connect(PROTOCOL . HOSTNAME);
        $this->assertEquals($db->stat(), 0);
        if ($db->stat() == 0)
        {
            // second connection
            $db2 = new Bz\database();
            $db2->connect(PROTOCOL . HOSTNAME . DBNAME, true);
            $this->assertEquals($db->stat(), 0);
            $db->disconnect(PROTOCOL . HOSTNAME);
            $this->assertEquals($db->stat(), 0);
        }
        // invalid host name
        $db->connect(PROTOCOL . 'localhost123/');
        $is_valid_stat = ($db->stat() == Bz\transactd::ERROR_TD_INVALID_CLINETHOST) || 
                         ($db->stat() == Bz\transactd::ERROR_TD_HOSTNAME_NOT_FOUND);
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
        $vv = new Bz\btrVersions();
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
        $td = new Bz\tabledef();
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
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // name
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = $fieldType;
        if ($fieldType == Bz\transactd::ft_mywvarchar)
            $fd->len = 1 + Bz\transactd::charsize(Bz\transactd::CHARSET_UTF16LE) * 3; // max 3 char len byte
        elseif ($fieldType == Bz\transactd::ft_mywvarbinary)
            $fd->len = 1 + Bz\transactd::charsize(Bz\transactd::CHARSET_UTF16LE) * 3; // max 6 char len byte
        elseif ($fieldType == Bz\transactd::ft_myvarchar)
        {
            if ($charset == Bz\transactd::CHARSET_CP932)
                $fd->len = 1 + Bz\transactd::charsize(Bz\transactd::CHARSET_CP932) * 3;  // max 6 char len byte
            elseif($charset == Bz\transactd::CHARSET_UTF8B4)
                $fd->len = 1 + Bz\transactd::charsize(Bz\transactd::CHARSET_UTF8B4) * 3; // max 6 char len byte
        }
        else
            $fd->len = 7; // max 6 char len byte
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // groupid
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('groupid');
        $fd->type = Bz\transactd::ft_integer;
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
    }
    
    public function testCreateDatabaseVar()
    {
        $db = new Bz\database();
        $db->create(URL_VAR);
        if ($db->stat() == Bz\transactd::STATUS_TABLE_EXISTS_ERROR)
        {
          $this->testDropDatabaseVar();
          $db->create(URL_VAR);
        }
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $db->open(URL_VAR, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
            $this->assertEquals($db->stat(), 0);
        }
        if (0 == $db->stat())
        {
            $this->createVarTable($db, 1, 'user1', Bz\transactd::ft_myvarchar,   Bz\transactd::CHARSET_CP932);
            $this->createVarTable($db, 2, 'user2', Bz\transactd::ft_myvarbinary, Bz\transactd::CHARSET_CP932);
            if ($this->isUtf16leSupport($db))
                $this->createVarTable($db, 3, 'user3', Bz\transactd::ft_mywvarchar,  Bz\transactd::CHARSET_CP932);
            $this->createVarTable($db, 4, 'user4', Bz\transactd::ft_mywvarbinary,    Bz\transactd::CHARSET_CP932);
            $this->createVarTable($db, 5, 'user5', Bz\transactd::ft_myvarchar,       Bz\transactd::CHARSET_UTF8B4);
            $db->close();
            $db->open(URL_VAR);
            $this->assertEquals($db->stat(), 0);
        }
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
        $db = new Bz\database();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable('user1');
        $this->assertEquals($db->stat(), 0);
        // acp varchar
        $this->setGetVar($tb, false, true);
        $tb->close();
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable('user2');
        $this->assertEquals($db->stat(), 0);
        // acp varbinary
        $this->setGetVar($tb, false, false);
        $tb->close();
        if ($this->isUtf16leSupport($db))
        {
            unset($tb); // UNSET $tb BEFORE REASSIGNMENT
            $tb = $db->openTable('user3');
            $this->assertEquals($db->stat(), 0);
            // unicode varchar
            $this->setGetVar($tb, true, true);
            $tb->close();
        }
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable('user4');
        $this->assertEquals($db->stat(), 0);
        // unicode varbinary
        $this->setGetVar($tb, true, false);
        $tb->close();
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable('user5');
        $this->assertEquals($db->stat(), 0);
        // utf8 varchar
        $this->setGetVar($tb, true, true);
        $tb->close();
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
    }
    public function testVarInsert()
    {
        $db = new Bz\database();
        $startid = 1;
        $bulk = false;
        $str = '漢字文字のテスト'; // too long kanji
        $str2 = '123';
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat())
        {
            $utf16leSupport = $this->isUtf16leSupport($db);
            $this->doVarInsert($db, 'user1', Bz\transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', Bz\transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', Bz\transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user4', Bz\transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', Bz\transactd::CP_UTF8,  $str, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $this->doVarInsert($db, 'user1', Bz\transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', Bz\transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', Bz\transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user4', Bz\transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', Bz\transactd::CP_UTF8,  $str2, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $bulk = true;
            $endid = 1000;
            $this->doVarInsert($db, 'user1', Bz\transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user2', Bz\transactd::CP_ACP,   '', $startid, $endid, $bulk);
            if ($utf16leSupport)
                $this->doVarInsert($db, 'user3', Bz\transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user4', Bz\transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user5', Bz\transactd::CP_UTF8,  '', $startid, $endid, $bulk);
        }
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
    }
    public function testVarRead()
    {
        $db = new Bz\database();
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
            $this->doVarRead($db, 'user1', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarRead($db, 'user2', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarRead($db, 'user4', Bz\transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarRead($db, 'user5', Bz\transactd::CP_UTF8,  $str,  $num, $ky);
            // short string
            $num = $num + 1;
            $this->doVarRead($db, 'user1', Bz\transactd::CP_ACP,   $str2, $num, $ky);
            $this->doVarRead($db, 'user2', Bz\transactd::CP_ACP,   $str4, $num, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', Bz\transactd::CP_ACP,   $str2, $num, $ky);
            $this->doVarRead($db, 'user4', Bz\transactd::CP_ACP,   $str4, $num, $ky);
            $this->doVarRead($db, 'user5', Bz\transactd::CP_UTF8,  $str2, $num, $ky);
            $ky = 1;
            $this->doVarRead($db, 'user1', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user2', Bz\transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport)
                $this->doVarRead($db, 'user3', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user4', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user5', Bz\transactd::CP_UTF8,  '120', 120, $ky);
        }
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
    }
    public function testFilterVar()
    {
        $db = new Bz\database();
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
            $this->doVarFilter($db, 'user1', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarFilter($db, 'user2', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport)
                $this->doVarFilter($db, 'user3', Bz\transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarFilter($db, 'user4', Bz\transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarFilter($db, 'user5', Bz\transactd::CP_UTF8,  $str,  $num, $ky);
            //if (UNICODE)
            //{
            //    // short string
            //    $num = $num + 1;
            //    $this->doVarFilter($db, 'user1', Bz\transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user2', Bz\transactd::CP_ACP,  $str4, $num, $ky);
            //    if ($utf16leSupport)
            //        $this->doVarFilter($db, 'user3', Bz\transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user4', Bz\transactd::CP_ACP,  $str4, $num, $ky);
            //    $this->doVarFilter($db, 'user5', Bz\transactd::CP_UTF8, $str2, $num, $ky);
            //}
            $ky = 1;
            $this->doVarFilter($db, 'user1', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user2', Bz\transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport)
                $this->doVarFilter($db, 'user3', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user4', Bz\transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user5', Bz\transactd::CP_UTF8,  '120', 120, $ky);
        }
    }
    public function testDropDatabaseVar()
    {
        $db = new Bz\database();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
    }
    
    /* -----------------------------------------------------
        transactd StringFilter
    ----------------------------------------------------- */
    
    private function createTableStringFilter($db, $id, $name, $type, $type2)
    {
        // create table
        $dbdef = $db->dbDef();
        $td = new Bz\tabledef();
        $td->setTableName($name);
        $td->setFileName($name . '.dat');
        $td->id = $id;
        $td->pageSize = 2048;
        $td->charsetIndex = Bz\transactd::CHARSET_UTF8B4;
        // $td->charsetIndex = Bz\transactd::CHARSET_CP932;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Bz\transactd::ft_integer;
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
        
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
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
        
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
    }
    
    public function testStringFilter()
    {
        $db = new Bz\database();
        $db->create(URL_SF);
        if ($db->stat() == Bz\transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->testDropDatabaseStringFilter();
            $db->create(URL_SF);
        }
        $this->assertEquals($db->stat(), 0);
        $db->open(URL_SF, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $this->doTestStringFilter($db, 1, 'zstring', Bz\transactd::ft_zstring, Bz\transactd::ft_wzstring);
        if ($this->isUtf16leSupport($db))
            $this->doTestStringFilter($db, 2, 'myvarchar', Bz\transactd::ft_myvarchar, Bz\transactd::ft_mywvarchar);
        else
            $this->doTestStringFilter($db, 2, 'myvarchar', Bz\transactd::ft_myvarchar, Bz\transactd::ft_myvarchar);
        $this->doTestStringFilter($db, 3, 'mytext', Bz\transactd::ft_mytext, Bz\transactd::ft_myblob);
    }
    
    public function testDropDatabaseStringFilter()
    {
        $db = new Bz\database();
        $db->open(URL_SF);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
    }
    
    public function testQuery()
    {
        $q = new Bz\query();
        $q->queryString("id = 0 and name = 'Abc efg'");
        $this->assertEquals($q->toString(), "id = '0' and name = 'Abc efg'");
        
        $q->queryString('');
        $q->where('id', '=', '0')->and_('name', '=', 'Abc efg');
        $this->assertEquals($q->toString(), "id = '0' and name = 'Abc efg'");
        
        $q->queryString("select id,name id = 0 AND name = 'Abc&' efg'");
        $this->assertEquals($q->toString(), "select id,name id = '0' AND name = 'Abc&' efg'");
        
        $q->queryString('');
        $q->select('id', 'name')->where('id', '=', '0')->and_('name', '=', "Abc' efg");
        $this->assertEquals($q->toString(), "select id,name id = '0' and name = 'Abc&' efg'");
        
        $q->queryString("select id,name id = 0 AND name = 'Abc&& efg'");
        $this->assertEquals($q->toString(), "select id,name id = '0' AND name = 'Abc&& efg'");
        
        $q->queryString('');
        $q->select('id', 'name')->where('id', '=', '0')->and_('name', '=', 'Abc& efg');
        $this->assertEquals($q->toString(), "select id,name id = '0' and name = 'Abc&& efg'");
        
        $q->queryString('*');
        $this->assertEquals($q->toString(), '*');
        
        $q->queryString('');
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
        $q->select('id', 'name', 'fc')->where('id', '=', '2')->and_('name', '=', '3');
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
        
        // test auto_escape
        $q->queryString("code = ab'c", true);
        $this->assertEquals($q->toString(), "code = 'ab&'c'");
        
        $q->queryString("code = ab&c", true);
        $this->assertEquals($q->toString(), "code = 'ab&&c'");
        
        $q->queryString("code = abc&", true);
        $this->assertEquals($q->toString(), "code = 'abc&&'");
        $q->queryString("code = abc&&", true);
        $this->assertEquals($q->toString(), "code = 'abc&&&&'");
        
        $q->queryString("code = 'abc&'", true);
        $this->assertEquals($q->toString(), "code = 'abc&&'");
        $q->queryString("code = 'abc&&'", true);
        $this->assertEquals($q->toString(), "code = 'abc&&&&'");
        
        $q->queryString("code = 'ab'c'", true);
        $this->assertEquals($q->toString(), "code = 'ab&'c'");
        
        $q->queryString("code = 'abc''", true);
        $this->assertEquals($q->toString(), "code = 'abc&''");
        
        $q->queryString("code = abc'", true);
        $this->assertEquals($q->toString(), "code = 'abc&''");
        
        // Invalid single quote (') on the end of statement
        $q->queryString("code = 'abc", true);
        $this->assertEquals($q->toString(), "code = 'abc'");
        
        $q->queryString("code = &abc", true);
        $this->assertEquals($q->toString(), "code = '&&abc'");
    }
    
    /* -----------------------------------------------------
        ActiveTable
    ----------------------------------------------------- */
    
    private function createQTuser($db)
    {
        $dbdef = $db->dbDef();
        $td = new Bz\tabledef();
        $td->setTableName('user');
        $td->setFileName('user.dat');
        $id = 1;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Bz\transactd::CP_UTF8;
        $td->charsetIndex = Bz\transactd::CHARSET_UTF8B4;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Bz\transactd::ft_autoinc;
        $fd->len = 4;
        // 名前 field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('名前');
        $fd->type = Bz\transactd::ft_myvarchar;
        $fd->setLenByCharnum(20);
        // group field
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('group');
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        // tel field
        $fd = $dbdef->insertField($id, 3);
        $fd->setName('tel');
        $fd->type = Bz\transactd::ft_myvarchar;
        $fd->setLenByCharnum(20);
        // key 0 (primary) id
        $kd = $dbdef->insertKey($id, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1; // extended key type
        $kd->segment(0)->flags->bit1 = 1; // changeable
        $kd->segmentCount = 1;
        $td = $dbdef->tableDefs($id);
        $td->primaryKeyNum = 0;
        // key 1 group
        $kd = $dbdef->insertKey($id, 1);
        $kd->segment(0)->fieldNum = 2;
        $kd->segment(0)->flags->bit8 = 1; // extended key type
        $kd->segment(0)->flags->bit1 = 1; // changeable
        $kd->segment(0)->flags->bit0 = 1; // duplicatable
        $kd->segmentCount = 1;
        // update
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // open test
        $tb = $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        return true;
    }
    private function createQTgroups($db)
    {
        $dbdef = $db->dbDef();
        $td = new Bz\tabledef();
        $td->setTableName('groups');
        $td->setFileName('groups.dat');
        $id = 2;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Bz\transactd::CP_UTF8;
        $td->charsetIndex = Bz\transactd::CHARSET_UTF8B4;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // code field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('code');
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        // name field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = Bz\transactd::ft_myvarbinary;
        $fd->len = 33;
        // key 0 (primary) code
        $kd = $dbdef->insertKey($id, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segmentCount = 1;
        $td = $dbdef->tableDefs($id);
        $td->primaryKeyNum = 0;
        // update
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // open test
        $tb = $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        return true;
    }
    private function createQTextention($db)
    {
        $dbdef = $db->dbDef();
        $td = new Bz\tabledef();
        $td->setTableName('extention');
        $td->setFileName('extention.dat');
        $id = 3;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Bz\transactd::CP_UTF8;
        $td->charsetIndex = Bz\transactd::CHARSET_UTF8B4;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        // comment field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('comment');
        $fd->type = Bz\transactd::ft_myvarchar;
        $fd->setLenByCharnum(60);
        // key 0 (primary) id
        $kd = $dbdef->insertKey($id, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;  // extended key type
        $kd->segment(0)->flags->bit1 = 1;  // changeable
        $kd->segmentCount = 1;
        $td = $dbdef->tableDefs($id);
        $td->primaryKeyNum = 0;
        // update
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // open test
        $tb = $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        return true;
    }
    private function insertQT($db, $maxId)
    {
        $db->beginTrn();
        // insert user data
        $tb = $db->openTable('user', Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        for ($i = 1; $i <= $maxId; $i++)
        {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i user");
            $tb->setFV('group', (($i - 1) % 5) + 1);
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $tb->close();
        // insert groups data
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable('groups', Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        for ($i = 1; $i <= 100; $i++)
        {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i group");
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $tb->close();
        // insert extention data
        unset($tb); // UNSET $tb BEFORE REASSIGNMENT
        $tb = $db->openTable('extention', Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        for ($i = 1; $i <= $maxId; $i++)
        {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i comment");
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $tb->close();
        $db->endTrn();
    }
    
    public function testCreateQueryTest()
    {
        $db = new Bz\database();
        // check database existence
        $db->open(URL_QT, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
        if ($db->stat() === 0) {
            return;
        }
        echo("\nDatabase " . DBNAME_QT . " not found\n");
        $db->create(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $db->open(URL_QT, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        // create tables
        $this->createQTuser($db);
        $this->createQTgroups($db);
        $this->createQTextention($db);
        // insert data
        $this->insertQT($db, 20000);
    }
    public function testNewDelete()
    {
        $db = new Bz\database();
        $db->open(URL_QT);
        for ($i = 0; $i < 500; $i++)
        {
            $q  = new Bz\query();
            $rq = new Bz\recordsetQuery();
            $gq = new Bz\groupQuery();
            $f  = new Bz\fieldNames();
            $f->addValue('abc');
            $atu = new Bz\ActiveTable($db, 'user');
            $atu->index(0);
            $atg = new Bz\ActiveTable($db, 'groups');
            $atg->index(0);
            $fns = new Bz\fieldNames();
            $fns->addValue('a');
            $s = new Bz\sum($fns);
            $s = new Bz\count('a');
            $s = new Bz\avg($fns);
            $s = new Bz\min($fns);
            $s = new Bz\max($fns);
            $rs = new Bz\Recordset();
        }
    }
    public function testJoin()
    {
        $db = new Bz\database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new Bz\ActiveTable($db, 'user');
        $atg = new Bz\ActiveTable($db, 'groups');
        $ate = new Bz\ActiveTable($db, 'extention');
        $q = new Bz\query();
        
        $atu->alias('名前', 'name');
        $q->select('id', 'name', 'group')->where('id', '<=', 15000);
        $rs = $atu->index(0)->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 15000);
        
        // Join extention::comment
        $q->reset();
        $ate->index(0)->join($rs,
            $q->select('comment')->optimize(Bz\queryBase::joinHasOneOrHasMany), 'id');
        $this->assertEquals($rs->size(), 15000);
        
        // reverse and get first (so it means 'get last')
        $last = $rs->reverse()->first();
        $this->assertEquals($last['id'], 15000);
        $this->assertEquals($last['comment'], '15000 comment');
        
        // Join group::name
        $q->reset();
        $atg->alias('name', 'group_name');
        $atg->index(0)->join($rs, $q->select('group_name'), 'group');
        $this->assertEquals($rs->size(), 15000);
        
        // get last (the rs is reversed, so it means 'get first')
        $first = $rs->last();
        $this->assertEquals($first['id'], 1);
        $this->assertEquals($first['comment'], '1 comment');
        $this->assertEquals($first['group_name'], '1 group');
        
        // row in rs[15000 - 9]
        $rec = $rs[15000 - 9];
        $this->assertEquals($rec['group_name'], '4 group');
        
        // orderby
        $rs->orderBy('group_name');
        for ($i = 0; $i < 15000 / 5; $i++)
        {
            $this->assertEquals($rs[$i]['group_name'], '1 group');
        }
        $this->assertEquals($rs[15000 / 5]['group_name'], '2 group');
        $this->assertEquals($rs[(15000 / 5) * 2]['group_name'], '3 group');
        $this->assertEquals($rs[(15000 / 5) * 3]['group_name'], '4 group');
        $this->assertEquals($rs[(15000 / 5) * 4]['group_name'], '5 group');
        
        // union
        $q->reset();
        $q->select('id', 'name', 'group')->where('id', '<=', 16000);
        $rs2 = $atu->index(0)->keyValue(15001)->read($q);
        $this->assertEquals($rs2->size(), 1000);
        $q->reset();
        $ate->index(0)->join($rs2,
            $q->select('comment')->optimize(Bz\queryBase::joinHasOneOrHasMany), 'id');
        $this->assertEquals($rs2->size(), 1000);
        $q->reset();
        $atg->index(0)->join($rs2, $q->select('group_name'), 'group');
        $this->assertEquals($rs2->size(), 1000);
        $rs->unionRecordset($rs2);
        $this->assertEquals($rs->size(), 16000);
        // row in rs[15000]
        $this->assertEquals($rs[15000]['id'], 15001);
        // last
        $this->assertEquals($rs->last()['id'], 16000);
        
        // group by
        $gq = new Bz\groupQuery();
        $gq->keyField('group', 'id');
        $count1 = new Bz\count('count');
        $gq->addFunction($count1);
        
        $count2 = new Bz\count('group1_count');
        $count2->when('group', '=', 1);
        $gq->addFunction($count2);
        
        $rs->groupBy($gq);
        $this->assertEquals($rs->size(), 16000);
        $this->assertEquals($rs[0]['group1_count'], 1);
        
        // clone
        $rsv = clone $rs;
        $gq->reset();
        $count3 = new Bz\count('count3');
        $gq->addFunction($count3)->keyField('group');
        $rs->groupBy($gq);
        $this->assertEquals($rs->size(), 5);
        $this->assertEquals($rsv->size(), 16000);
        
        // having
        $rq = new Bz\recordsetQuery();
        $rq->when('group1_count', '=', 1)->or_('group1_count', '=', 2);
        $rsv->matchBy($rq);
        $this->assertEquals($rsv->size(), 3200);
        $this->assertEquals(isset($rsv), true);
        unset($rsv);
        $this->assertEquals(isset($rsv), false);
        
        // top
        $rs3 = new Bz\Recordset();
        $rs->top($rs3, 10);
        $this->assertEquals($rs3->size(), 5);
        $rs->top($rs3, 3);
        $this->assertEquals($rs3->size(), 3);
        $this->assertEquals($rs->size(), 5);
        
        // query new / delete
        $q1 = new Bz\recordsetQuery();
        $q1->when('group1_count', '=', 1)->or_('group1_count', '=', 2);
        unset($q1);
        
        $q2 = new Bz\query();
        $q2->where('group1_count', '=', 1)->or_('group1_count', '=', 2);
        unset($q2);
        
        $q3 = new Bz\groupQuery();
        $q3->keyField('group', 'id');
        unset($q3);
    }
    public function testWritableRecord()
    {
        $db = new Bz\database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new Bz\ActiveTable($db, 'user');
        
        $rec = $atu->index(0)->getWritableRecord();
        $rec['id'] = 120000;
        $rec['名前'] = 'aiba';
        $rec->save();
        
        $rec->clear();
        $this->assertNotEquals($rec['id'], 120000);
        $this->assertNotEquals($rec['名前'], 'aiba');
        $rec['id'] = 120000;
        $rec->read();
        $this->assertEquals($rec['id'], 120000);
        $this->assertEquals($rec['名前'], 'aiba');
        
        $rec->clear();
        $rec['id'] = 120001;
        $rec['名前'] = 'oono';
        if (! $rec->read()) {
            $rec->insert();
        }
        
        $rec->clear();
        $rec['id'] = 120001;
        $rec->read();
        $this->assertEquals($rec['id'], 120001);
        $this->assertEquals($rec['名前'], 'oono');
        
        // update only changed filed
        $rec->clear();
        $rec['id'] = 120001;
        $rec['名前'] = 'matsumoto';
        $rec->update();
        
        $rec->clear();
        $rec['id'] = 120001;
        $rec->read();
        $this->assertEquals($rec['id'], 120001);
        $this->assertEquals($rec['名前'], 'matsumoto');
        
        $rec->del();
        $rec['id'] = 120000;
        $rec->del();
        
        $rec->clear();
        $rec['id'] = 120001;
        $ret = $rec->read();
        $this->assertEquals($ret, false);
        
        $rec->clear();
        $rec['id'] = 120000;
        $ret = $rec->read();
        $this->assertEquals($ret, false);
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
            $ret = Bz\transactd::u8tombc($u8, -1, '', 256);
            $this->assertEquals($u8, $ret);
            
            $mbcKanji = [0x8A, 0xBF, 0x8E, 0x9A, 0x00];
            $u8 = mb_convert_encoding('漢字', $enc_u8);
            $ret = Bz\transactd::u8tombc($u8, -1, '', 256);
            for ($i = 0; $i < strlen($ret); $i++)
                $this->assertEquals(hexdec(bin2hex($ret{$i})), $mbcKanji[$i]);
            
            $mbc = $ret;
            $u8Kanji = [0xe6 ,0xbc ,0xa2 ,0xe5 ,0xad ,0x97];
            $ret = Bz\transactd::mbctou8($mbc, -1, '', 256);
            for ($i = 0; $i < strlen($ret); $i++)
                $this->assertEquals(hexdec(bin2hex($ret{$i})), $u8Kanji[$i]);
        }
    }
}
