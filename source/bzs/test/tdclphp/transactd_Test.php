<?php
/* ================================================================
   Copyright (C) 2013,2016 BizStation Corp All rights reserved.

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

require("transactd.php");

use BizStation\Transactd\Transactd;
use BizStation\Transactd\PooledDbManager;
use BizStation\Transactd\ConnectParams;
use BizStation\Transactd\Tabledef;
use BizStation\Transactd\Database;
use BizStation\Transactd\BtrVersions;
use BizStation\Transactd\Nstable;
use BizStation\Transactd\Table;
use BizStation\Transactd\QueryBase;
use BizStation\Transactd\Query;
use BizStation\Transactd\RecordsetQuery;
use BizStation\Transactd\GroupQuery;
use BizStation\Transactd\FieldNames;
use BizStation\Transactd\ActiveTable;
use BizStation\Transactd\Sum;
use BizStation\Transactd\Count;
use BizStation\Transactd\Avg;
use BizStation\Transactd\Min;
use BizStation\Transactd\Max;
use BizStation\Transactd\Last;
use BizStation\Transactd\First;
use BizStation\Transactd\Recordset;

Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE);

function getHost()
{
    $host = getenv('TRANSACTD_PHPUNIT_HOST');
    if (strlen($host) == 0) {
        $host = '127.0.0.1/';
    }
    if ($host[strlen($host) - 1] != '/') {
        $host = $host . '/';
    }
    return $host;
}

define("HOSTNAME", getHost());
define("USERNAME", getenv('TRANSACTD_PHPUNIT_USER'));
define("USERPART", strlen(USERNAME) == 0 ? '' : USERNAME . '@');
define("PASSWORD", getenv('TRANSACTD_PHPUNIT_PASS'));
define("PASSPART", strlen(PASSWORD) == 0 ? '' : '&pwd=' . PASSWORD);
define("PASSPART2", strlen(PASSWORD) == 0 ? '' : '?pwd=' . PASSWORD);
define("DBNAME", "test");
define("DBNAME_VAR", "testvar");
define("DBNAME_SF", "testString");
define("DBNAME_QT", "querytest");
define("TABLENAME", "user");
define("PROTOCOL", "tdap://");
define("BDFNAME", "?dbfile=test.bdf");
define("URL", PROTOCOL . USERPART . HOSTNAME . DBNAME . BDFNAME . PASSPART);
define("URL_VAR", PROTOCOL . USERPART . HOSTNAME . DBNAME_VAR . BDFNAME . PASSPART);
define("URL_SF", PROTOCOL . USERPART . HOSTNAME . DBNAME_SF . BDFNAME . PASSPART);
define("URL_QT", PROTOCOL . USERPART . HOSTNAME . DBNAME_QT . BDFNAME . PASSPART);
define("URL_HOST", PROTOCOL . USERPART . HOSTNAME . PASSPART2);
define("URL_DB", PROTOCOL . USERPART . HOSTNAME . DBNAME . PASSPART2);
define("FDI_ID", 0);
define("FDI_NAME", 1);
define("FDI_GROUP", 2);
define("FDI_NAMEW", 2);

define("BULKBUFSIZE", 65535 - 1000);
define("TEST_COUNT", 20000);
define("FIVE_PERCENT_OF_TEST_COUNT", TEST_COUNT / 20);

// multi thread test if `php_pthreads` exists.
if (class_exists('Thread')) {
    class SeekLessThanWorker extends Thread
    {
        public function __construct()
        {
            $this->value = -1;
        }
        public function run()
        {
            $dbm = new PooledDbManager(new ConnectParams(URL));
            $tb = $dbm->table('user');
            $tb->setFV(FDI_ID, 300000);
            $tb->seekLessThan(false, Transactd::ROW_LOCK_X);
            $this->value = $tb->getFVint(FDI_ID);
            $tb->unlock();
            $tb->close();
            $dbm->unUse();
        }
        public function getResult()
        {
            return $this->value;
        }
    }
}

class TransactdTest extends PHPUnit_Framework_TestCase
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
        if ($db->stat() == Transactd::STATUS_TABLE_EXISTS_ERROR) {
            $this->dropDatabase($db);
            $db->create(URL);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db)
    {
        $db->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
    }
    private function createTable($db)
    {
        $this->openDatabase($db);
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
        // Set table schema codepage to UTF-8
        //     - codepage for field NAME and tableNAME
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName(TABLENAME);
        $td->setFileName(TABLENAME . '.dat');
        // Set table default charaset index
        //    - default charset for field VALUE
        $td->charsetIndex = Transactd::charsetIndex(Transactd::CP_UTF8);
        $tableid = 1;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 0);
        $fd->setName('id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 1);
        $fd->setName('name');
        $fd->len = 33;
        
        //test padChar only string or wstring
        $fd->type = Transactd::ft_string;
        $fd->setPadCharSettings(true, false);
        $this->assertEquals($fd->isUsePadChar(), true);
        $this->assertEquals($fd->isTrimPadChar(), false);
        $fd->setPadCharSettings(false, true);
        $this->assertEquals($fd->isUsePadChar(), false);
        $this->assertEquals($fd->isTrimPadChar(), true);
        
        $fd->type = Transactd::ft_zstring;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // Set field charset index
        //    - charset for each field VALUE
        //  $fd->setCharsetIndex(Transactd::charsetIndex(Transactd::CP_UTF8))

        $fd = $dbdef->insertField($tableid, 2);
        $fd->setName('select');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 3);
        $fd->setName('in');
        $fd->type = Transactd::ft_integer;
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
        
        // group table
        $td = new Tabledef();
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName('group');
        $td->setFileName('group.dat');
        $tableid = 2;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 0);
        $fd->setName('id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fd = $dbdef->insertField($tableid, 1);
        $fd->setName('name');
        $fd->type = Transactd::ft_zstring;
        $fd->len = 33;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        //test statMsg
        $this->assertEquals($dbdef->statMsg(), '');
        
        $kd = $dbdef->insertKey($tableid, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        $this->assertEquals($dbdef->validateTableDef($tableid), 0);
    }
    private function openTable($db)
    {
        $this->openDatabase($db);
        $tb = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        //test statMsg
        $this->assertEquals($db->statMsg(), '');
        return $tb;
    }
    
    public function testCreateDatabase()
    {
        $db = new Database();
        $this->createDatabase($db);
    }
    public function testCreateTable()
    {
        $db = new Database();
        $this->createTable($db);
    }
    // open database, not call close explicitly
    public function testOpenDatabase()
    {
        $db = new Database();
        $db->open(URL);
    }
    // open database, call close explicitly
    public function testCloseDatabase()
    {
        $db = new Database();
        $db->open(URL);
        $db->close();
    }
    // open database, open table, not call close explicitly
    public function testOpenA()
    {
        $db = new Database();
        $db->open(URL);
        $this->openTable($db);
    }
    // open database, open table, call close explicitly
    public function testOpenB()
    {
        $db = new Database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->close();
        $db->close();
    }
    // open database, open table, call database::close explicitly
    public function testOpenC()
    {
        $db = new Database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $db->close();
    }
    // open database, open table, call table::close explicitly
    public function testOpenD()
    {
        $db = new Database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->close();
    }
    // open database, open table, call table::release explicitly
    public function testOpenE()
    {
        $db = new Database();
        $db->open(URL);
        $tb = $this->openTable($db);
        $tb->release();
    }
    public function testClone()
    {
        $db = new Database();
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
        $db = new Database();
        $db->connect(URL_HOST);
        $this->assertEquals($db->stat(), 0);
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $this->assertEquals($db->stat(), 0);
        $client_ver = $vv->version(0);
        $server_ver = $vv->version(1);
        $engine_ver = $vv->version(2);
        $this->assertEquals($client_ver->majorVersion, Transactd::CPP_INTERFACE_VER_MAJOR);
        $this->assertEquals($client_ver->minorVersion, Transactd::CPP_INTERFACE_VER_MINOR);
        $this->assertEquals(chr($client_ver->type), 'N');
        $my5x = ($server_ver->majorVersion == 5) && ($server_ver->minorVersion >= 5);
        $maria10 = ($server_ver->majorVersion == 10) && ($server_ver->minorVersion <= 2);
        $this->assertTrue($my5x || $maria10);
        $tmp = (chr($server_ver->type) == 'M') || (chr($server_ver->type) == 'A');
        $this->assertTrue($tmp);
        $this->assertEquals($engine_ver->majorVersion, Transactd::TRANSACTD_VER_MAJOR);
        $this->assertEquals($engine_ver->minorVersion, Transactd::TRANSACTD_VER_MINOR);
        $this->assertEquals(chr($engine_ver->type), 'T');
    }
    public function testReadDatabaseDirectory()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $s = $db->readDatabaseDirectory();
        $this->assertNotEquals($s, '');
    }
    public function testGetFileName()
    {
        $s = '';
        if (PHP_OS == 'WIN32' || PHP_OS == 'WINNT') {
            $s = Nstable::getFileName('test\abcdefghijklnmopqrstuvwxyz1234567890.txt');
        } else {
            $s = Nstable::getFileName('test/abcdefghijklnmopqrstuvwxyz1234567890.txt');
        }
        $this->assertEquals($s, 'abcdefghijklnmopqrstuvwxyz1234567890.txt');
    }
    public function testGetDirURI()
    {
        $s = Nstable::getDirURI('tdap://localhost/test?dbfile=test.bdf');
        $this->assertEquals($s, 'tdap://localhost/test?dbfile=');
    }
    public function testInsert()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $this->assertEquals($tb->recordCount(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 1);
        $tb->setFV(FDI_NAME, 'kosaka');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        //test statMsg
        $this->assertEquals($tb->statMsg(), '');

        $db->beginTrn();
        $n = 1;
        $tb->seekLast();
        if ($tb->stat() == 0) {
            $n = $tb->getFVint(FDI_ID) + 1;
        }
        $tb->beginBulkInsert(BULKBUFSIZE);
        for ($i = $n; $i <= (TEST_COUNT + $n); $i++) {
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 10 and id < ' . TEST_COUNT, 1, 0);
        $v = 10;
        $tb->setFV(FDI_ID, $v);
        $tb->find(Table::findForword);
        $i = $v;
        while ($i < TEST_COUNT) {
            $this->assertEquals($tb->stat(), 0);
            if ($tb->stat() != 0) {
                break;
            }
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true); // 11 - 19
            $i = $i + 1;
        }
        // backforword
        $tb->clearBuffer();
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->find(Table::findBackForword);
        $i = $v;
        while ($i >= 10) {
            $this->assertEquals($tb->stat(), 0);
            if ($tb->stat() != 0) {
                break;
            }
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findPrev(true); // 11 - 19
            $i = $i - 1;
        }
        // out of filter range (EOF)
        $tb->clearBuffer();
        $v = TEST_COUNT;
        $tb->setFV(FDI_ID, $v);
        $tb->find(Table::findForword);
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
    }
    public function testFindNext()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 10 and id < ' . TEST_COUNT, 1, 0);
        $v = 10;
        $tb->setFV(FDI_ID, $v);
        $tb->seekGreater(true);
        $this->assertEquals($tb->getFVint(FDI_ID), $v);
        for ($i = $v + 1; $i <= (TEST_COUNT - 1); $i++) {
            $tb->findNext(true); // 11 - 19
            if ($tb->stat() != 0) {
                break;
            }
            $this->assertEquals($tb->stat(), 0);
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
    }
    public function testFindIn()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $q = new Query();
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = 300000\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 50);
        $tb->findNext();
        $this->assertEquals($tb->stat(), Transactd::STATUS_NOT_FOUND_TI);
        
        $msg = $tb->keyValueDescription();
        $this->assertEquals($msg, "table:user\nstat:4\nid = -1\n");
       
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 80);
        $tb->findNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 5000);
        $tb->findNext();
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        
        // Many params
        for ($i = 1; $i <= 10000; $i++) {
            $q->addSeekKeyValue(strval($i), ($i == 1)); // reset
        }
        $tb->setQuery($q);
        $this->assertEquals($tb->stat(), 0);
        
        $tb->find();
        $i = 0;
        while ($tb->stat() == 0) {
            $i++;
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true);
        }
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        $this->assertEquals($i, 10000);
        
        //LogicalCountLimit
        $q->select('id');
        $tb->setQuery($q);
        
        $tb->find();
        $i = 0;
        while ($tb->stat() == 0) {
            $i++;
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            $tb->findNext(true);
        }
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        $this->assertEquals($i, 10000);
    }
    public function testGetPercentage()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->clearBuffer();
        $tb->seekByPercentage(5000); // 50%
        $this->assertEquals($tb->stat(), 0);
        $v = $tb->getFVint(FDI_ID);
        $this->assertEquals($tb->stat(), 0);
        $this->assertTrue(abs(TEST_COUNT / 2 + 1 - $v) < FIVE_PERCENT_OF_TEST_COUNT);
    }
    
    public function testGetEqual()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db->beginSnapshot();
        for ($i = 2; $i <= (TEST_COUNT + 1); $i++) {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->seek();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
        }
        $db->endSnapshot();
    }
    public function testGetNext()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db->beginSnapshot();
        $vv = 2;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        for ($i = 3; $i <= (TEST_COUNT + 1); $i++) {
            $tb->seekNext();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            if ($i != $tb->getFVint(FDI_ID)) {
                break;
            }
        }
        $db->endSnapshot();
    }
    public function testGetPrevious()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db->beginSnapshot();
        $vv = TEST_COUNT + 1;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        for ($i = TEST_COUNT; $i >= 2; $i--) {
            $tb->seekPrev();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            if ($i != $tb->getFVint(FDI_ID)) {
                break;
            }
        }
        $tb->seekPrev();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
        $db->endSnapshot();
        // without snapshot
        $vv = TEST_COUNT + 1;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $vv);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $vv);
        for ($i = TEST_COUNT; $i > 1; $i--) {
            $tb->seekPrev();
            $this->assertEquals($tb->getFVint(FDI_ID), $i);
            if ($i != $tb->getFVint(FDI_ID)) {
                break;
            }
        }
        $tb->seekPrev();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
    }
    public function testGetGreater()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->clearBuffer();
        $tb->seekFirst();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'kosaka');
    }
    public function testGetLast()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->clearBuffer();
        $tb->seekLast();
        $this->assertEquals($tb->getFVstr(FDI_NAME), '' . (TEST_COUNT + 2));
    }
    public function testMovePosition()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db->beginTrn();
        // test of ncc
        $v = 5;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $v = TEST_COUNT + TEST_COUNT / 2;
        $tb->setFV(FDI_ID, $v);
        $tb->update(Table::changeCurrentNcc); // 5 . 30000 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext(); // next 5
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), $v);
        $v = 5;
        $tb->setFV(FDI_ID, $v);
        $tb->update(Table::changeCurrentCc);  // 19999 . 5 cur 5
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 6);
        $v = TEST_COUNT - 1;
        $tb->setFV(FDI_ID, $v);
        $tb->update(Table::changeCurrentCc); // 6 . 19999 cur 19999
        $tb->seekPrev(); // prev 19999
        $this->assertEquals($tb->getFVint(FDI_ID), $v -1);
        $v = 10;
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 11);
        for ($i = 10; $i <= (TEST_COUNT - 2); $i++) {
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
        $tb->update(Table::changeInKey);
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $v);
        $tb->seek();
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'ABC');
    }
    public function testSnapshot()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tbg = $db->openTable('group');
        $this->assertEquals($db->stat(), 0);
        $this->assertNotEquals($tbg, null);
        $db2 = new Database();
        $this->assertEquals($db2->stat(), 0);
        $db2->connect(URL_DB, true);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, null);
        $tbg2 = $db2->openTable('group');
        $this->assertEquals($db2->stat(), 0);
        $this->assertNotEquals($tbg2, null);
        
        // No locking repeatable read
        // ----------------------------------------------------
        $db->beginSnapshot(); // CONSISTENT_READ is default
        $this->assertEquals($db->stat(), 0);
        $db->beginTrn();
        $this->assertEquals($db->stat(), Transactd::STATUS_ALREADY_INSNAPSHOT);
        
        $tb->setKeyNum(0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $firstValue = $tb->getFVstr(FDI_NAME);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $tbg->seekFirst();
        $this->assertEquals($tbg->stat(), Transactd::STATUS_EOF);
        $this->assertEquals($tbg->recordCount(false), 0);
        
        // Change data on 2 tables by another connection
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, $tb2->getFVint(FDI_ID) + 1);
        $tb2->update(); //Change success
        $this->assertEquals($tb2->stat(), 0);
        $tbg2->setFV(FDI_ID, 1);
        $tbg2->setFV(FDI_NAME, 'ABC');
        $tbg2->insert();
        $this->assertEquals($tbg2->stat(), 0);
        
        // in-snapshot repeatable read check same value
        $tb->seekFirst();
        $secondValue = $tb->getFVstr(FDI_NAME);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($secondValue, $firstValue, "$firstValue != $secondValue");
        
        $tbg->seekFirst();
        $this->assertEquals($tbg->stat(), Transactd::STATUS_EOF);
        $this->assertEquals($tbg->recordCount(false), 0);
        
        // in-snapshot update
        $tb->update();
        $this->assertEquals($tb->stat(), Transactd::STATUS_INVALID_LOCKTYPE);
        
        // in-snapshot insert
        $tb->setFV(FDI_ID, 0);
        $tb->insert();
        $this->assertEquals($tb->stat(), Transactd::STATUS_INVALID_LOCKTYPE);
        
        // phantom read
        $tb2->setFV(FDI_ID, 29999);
        $tb2->insert();
        $this->assertEquals($tb2->stat(), 0);
        $tb->setFV(FDI_ID, 29999);
        $tb->seek();
        $this->assertEquals($tb->stat(), Transactd::STATUS_NOT_FOUND_TI);
        
        // clean up
        $tb2->setFV(FDI_ID, 29999);
        $tb2->seek();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->del();
        $this->assertEquals($tb2->stat(), 0);
        
        $db->endSnapshot();
        $this->assertEquals($db->stat(), 0);
        
        // After snapshot, db can read new versions.
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $tbg->seekFirst();
        $this->assertEquals($tbg->stat(), 0);
        $this->assertEquals($tbg->recordCount(false), 1);
        
        // gap lock
        $db->beginSnapshot(Transactd::MULTILOCK_GAP_SHARE);
        $tb->seekLast(); // id = 30000
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // id = 20002
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // id = 20001
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->setFV(FDI_ID, 29999);
        $tb2->insert();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $db->endSnapshot();
        
        // gap lock
        $db->beginSnapshot(Transactd::MULTILOCK_NOGAP_SHARE);
        $tb->seekLast(); // id = 30000
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // id = 20002
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // id = 20001
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->setFV(FDI_ID, 20002);
        $tb2->seek(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $tb2->seekLast(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $db->endSnapshot();
    }
    public function testConflict()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $db2 = new Database();
        $db2->connect(URL_DB, true);
        $this->assertEquals($db2->stat(), 0);
        $this->assertNotEquals($tb, null);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, null);
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_CHANGE_CONFLICT);
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_CHANGE_CONFLICT);
        // ----------------------------------------------------
        $tb2->release();
        $tb->release();
        unset($db2);
        unset($db);
    }
    // isoration Level ISO_REPEATABLE_READ
    public function testTransactionLockRepeatable()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db2 = new Database();
        $db2->connect(URL_DB, true);
        $this->assertEquals($db2->stat(), 0);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, null);
        
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        // Test Invalid operation
        $db->beginSnapshot();
        $this->assertEquals($db->stat(), Transactd::STATUS_ALREADY_INTRANSACTION);
        
        // ------------------------------------------------------
        // Test Read with lock
        // ------------------------------------------------------
        // lock(X) the first record
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        // Add lock(X) the second record
        $tb->seekNext();
        
        // No transaction user can read allways. Use consistent_read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekNext();
        $this->assertEquals($tb2->stat(), 0);
        
        // The second transaction user can not lock same record.
        $db2->beginTrn();
        $tb2->setKeyNum(0);
        
        // Try lock(X)
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        
        // ------------------------------------------------------
        // Test single record lock and Transaction lock
        // ------------------------------------------------------
        // lock(X) non-transaction
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        
        // Try lock(X)
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // Remove lock(X)
        $tb2->seekFirst();
        
        // Retry lock(X)
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->setFV(FDI_NAME, 'ABC');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        // No transaction user can read allways. Use consistent_read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $this->assertNotEquals($tb2->getFVstr(FDI_NAME), 'ABC');
        
        // ------------------------------------------------------
        // Test Transaction lock and Transaction lock
        // ------------------------------------------------------
        $db2->beginTrn();
        $this->assertEquals($db2->stat(), 0);
        
        // try lock(X)
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // Try unlock updated record. Can not unlock updated record.
        $tb->unlock();
        
        // try lock(X)
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $db2->endTrn();
        $db->endTrn();
        
        // ----------------------------------------------------
        //  Test phantom read
        // ----------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        
        // read last row
        $tb->seekLast(); // lock(X) last id = 30000
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // Add lock(X)
        $this->assertEquals($tb->stat(), 0);
        $last2 = $tb->getFVint(FDI_ID);
        
        // insert test row
        $tb2->setFV(FDI_ID, 29999);
        $tb2->insert(); // Can not insert by gap lock
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), $last2);
        $db->endTrn();
        
        // ----------------------------------------------------
        //  Test use shared lock option
        // ----------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals(0, $db->stat());
        
        $db2->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals(0, $db2->stat());
        
        $tb->seekLast(Transactd::ROW_LOCK_S);
        $this->assertEquals(0, $tb->stat());
        $tb2->seekLast(Transactd::ROW_LOCK_S);
        $this->assertEquals(0, $tb2->stat());
        
        $tb->seekPrev(); // Lock(X)
        $this->assertEquals(0, $tb->stat());
        
        $tb2->seekPrev(Transactd::ROW_LOCK_S);
        $this->assertEquals(Transactd::STATUS_LOCK_ERROR, $tb2->stat());
        
        $tb->seekPrev(Transactd::ROW_LOCK_S);
        $this->assertEquals(0, $tb->stat());
        $id = $tb->getFVint(FDI_ID);
        
        $tb2->setFV(FDI_ID, $id);
        $tb2->seek(Transactd::ROW_LOCK_S);
        $this->assertEquals(0, $tb2->stat());
        
        $db2->endTrn();
        $db->endTrn();
        
        // ----------------------------------------------------
        //  Test Abort
        // ----------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        
        // lock(X)
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
        
        // ----------------------------------------------------
        //  Test Query and locks Multi record lock
        // ----------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        
        // Test find records are lock.
        $q = new Query();
        $q->where('id', '<=', 15)->and_('id', '<>', 13)->reject(0xFFFF);
        $tb->setQuery($q);
        $tb->setFV(FDI_ID, 12);
        $tb->find();
        while ($tb->stat() == 0) {
            $tb->findNext();
        }
        $this->assertEquals($tb->getFVint(FDI_ID), 15);
        
        // all records locked
        for ($i = 12; $i <= 16; $i++) {
            $tb2->setFV(FDI_ID, $i);
            $tb2->seek(Transactd::ROW_LOCK_X);
            $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        }
        $db->endTrn();
    }
    public function testTransactionLockReadCommited()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db2 = new Database();
        $db2->connect(URL_DB, true);
        $this->assertEquals($db2->stat(), 0);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, null);
        
        // ------------------------------------------------------
        // Test single record lock Transaction and read
        // ------------------------------------------------------
        $db->beginTrn(Transactd::SINGLELOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        // Test Invalid operation
        $db->beginSnapshot();
        $this->assertEquals($db->stat(), Transactd::STATUS_ALREADY_INTRANSACTION);
        
        $tb->setKeyNum(0);
        $tb->seekFirst(); // lock(X)
        $this->assertEquals($tb->stat(), 0);
        
        // Try lock(X)
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // consistent read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        
        // Unlock first record. And lock(X) second record
        $tb->seekNext();
        
        // test unlocked first record
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        
        // The second record, consistent read
        $tb2->seekNext();
        $this->assertEquals($tb2->stat(), 0);
        // Try lock(X) whith lock(IX)
        $tb2->update();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // ------------------------------------------------------
        // Test single record lock Transaction and Transaction lock
        // ------------------------------------------------------
        $db2->beginTrn();
        // Try lock(X)
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        // Try lock(X)
        $tb2->seekNext();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        
        // ------------------------------------------------------
        // Test multi record lock Transaction and non-transaction read
        // ------------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        // lock(X) the first record
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        // Add lock(X) the second record
        $tb->seekNext();
        
        // No transaction user read can read allways. Use consistent_read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekNext();
        $this->assertEquals($tb2->stat(), 0);
        
        // ------------------------------------------------------
        // Test unlock
        // ------------------------------------------------------
        $tb2->seekFirst();
        $tb2->seekNext(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $tb->unlock();
        // retry seekNext. Before operation is failed but do not lost currency.
        $tb2->seekNext(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), 0);
        $tb2->seekNext();
        // ------------------------------------------------------
        // Test undate record unlock
        // ------------------------------------------------------
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekNext();
        $this->assertEquals($tb->stat(), 0);
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb->unlock(); // Can not unlock updated record
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $tb2->seekNext(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // ------------------------------------------------------
        // Test multi record lock Transaction and Transaction
        // ------------------------------------------------------
        $db2->beginTrn();
        $this->assertEquals($db2->stat(), 0);
        
        // Try lock(X)
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        $db2->endTrn();
        $db->endTrn();
        
        // ------------------------------------------------------
        // Test multi record lock Transaction and non-transaction record lock
        // ------------------------------------------------------
        // lock(X) non-transaction
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        
        $db->beginTrn(Transactd::SINGLELOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        // Try lock(X)
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // Remove lock(X)
        $tb2->seekFirst();
        
        // Retry lock(X)
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        // update in transaction
        $tb->setFV(FDI_NAME, 'ABC');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        // move from first record.
        $tb->seekNext();
        
        // No transaction read can read allways. Use consistent_read
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->update();
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $db->endTrn();
        // ------------------------------------------------------
        // Test phantom read
        // ------------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        // read last row
        $tb->seekLast(); // lock(X) last id = 30000
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev(); // Add lock(X)
        $this->assertEquals($tb->stat(), 0);
        $last2 = $tb->getFVint(FDI_ID);
        
        //insert test row
        $tb2->setFV(FDI_ID, 29999);
        $tb2->insert();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekPrev();
        $this->assertEquals($tb->stat(), 0);
        $this->assertNotEquals($tb->getFVint(FDI_ID), $last2);
        $db->endTrn();
        
        // cleanup
        $tb2->del(); // last id = 29999
        $this->assertEquals($tb2->stat(), 0);
        
        // ------------------------------------------------------
        // Test use shared lock option
        // ------------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db->stat(), 0);
        
        $db2->beginTrn(Transactd::MULTILOCK_REPEATABLE_READ);
        $this->assertEquals($db2->stat(), 0);
        
        $tb->seekLast(Transactd::ROW_LOCK_S);
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekLast(Transactd::ROW_LOCK_S);
        $this->assertEquals($tb2->stat(), 0);
        
        $tb->seekPrev(); // Lock(X)
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->seekPrev(Transactd::ROW_LOCK_S);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        $tb->seekPrev(Transactd::ROW_LOCK_S);
        $this->assertEquals($tb->stat(), 0);
        $id = $tb->getFVint(FDI_ID);
        
        $tb2->setFV(FDI_ID, $id);
        $tb2->seek(Transactd::ROW_LOCK_S);
        $this->assertEquals($tb2->stat(), 0);
        
        $db2->endTrn();
        $db->endTrn();
        
        // ------------------------------------------------------
        // Abort test
        // ------------------------------------------------------
        $db->beginTrn(Transactd::SINGLELOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_NAME, 'EFG');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->seekNext();
        $db->abortTrn();
        $tb2->setKeyNum(0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->getFVstr(FDI_NAME), 'ABC');
        
        // ------------------------------------------------------
        // Test Query and locks Single record lock
        // ------------------------------------------------------
        $db->beginTrn(Transactd::SINGLELOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        // Test find last record locked
        $q = new Query();
        $q->where('id', '<=', '100');
        $tb->setQuery($q);
        $tb->setFV(FDI_ID, 1);
        $tb->find();
        while ($tb->stat() == 0) {
            $tb->findNext();
        }
        $this->assertEquals($tb->getFVint(FDI_ID), 100);
        
        // find read last is record of id = 101.
        // Would be difficult to identify the last
        //  access to records at SINGLELOCK_READ_COMMITED.
        // No match records are unlocked.
        $tb2->setFV(FDI_ID, 100);
        $tb2->seek(Transactd::ROW_LOCK_X);
        $this->assertEquals(0, $tb2->stat());
        $tb2->setFV(FDI_ID, 101);
        $tb2->seek(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), 0);
        $tb2->unlock();
        $db->endTrn();
        
        // ------------------------------------------------------
        // Test Query and locks Multi record lock
        // ------------------------------------------------------
        $db->beginTrn(Transactd::MULTILOCK_READ_COMMITED);
        $this->assertEquals($db->stat(), 0);
        
        // Test find records are lock.
        $q->reset()->where('id', '<=', 15)->and_('id', '<>', 13)->reject(0xFFFF);
        $tb->setQuery($q);
        $tb->setFV(FDI_ID, 12);
        $tb->find();
        while ($tb->stat() == 0) {
            $tb->findNext();
        }
        $this->assertEquals($tb->getFVint(FDI_ID), 15);
        
        for ($i = 12; $i <= 16; $i++) {
            $tb2->setFV(FDI_ID, $i);
            $tb2->seek(Transactd::ROW_LOCK_X);
            if (($i == 16) || ($i == 13)) {
                $this->assertEquals($tb2->stat(), 0);
            } else {
                $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
            }
        }
        $db->endTrn();
    }
    public function testRecordLock()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db2 = new Database();
        $db2->connect(URL_DB, true);
        $this->assertEquals($db2->stat(), 0);
        $tb2 = $this->openTable($db2);
        $this->assertNotEquals($tb2, null);
        
        $tb->setKeyNum(0);
        $tb2->setKeyNum(0);
        
        // Single record lock
        $tb->seekFirst(Transactd::ROW_LOCK_X); // lock(X)
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst(); // Use consistent_read
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekFirst(Transactd::ROW_LOCK_X); // Try lock(X) single
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // try consistent_read. Check ended that before auto transaction
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekNext(Transactd::ROW_LOCK_X); // lock(X) second
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekNext(Transactd::ROW_LOCK_X); // lock(X) third, second lock freed
        $this->assertEquals($tb2->stat(), 0);
        
        $tb->seekNext(); // nobody lock second. But REPEATABLE_READ tb2 lock all(no unlock)
        if ($db->trxIsolationServer() == Transactd::SRV_ISO_REPEATABLE_READ) {
            $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        } else {
            $this->assertEquals($tb->stat(), 0);
        }
        $tb->seekNext(Transactd::ROW_LOCK_X); // Try lock(X) third
        $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        
        // Update test change third with lock(X)
        $tb2->setFV(FDI_NAME, 'The 3rd');
        $tb2->update(); // auto trn commit and unlock all locks
        $this->assertEquals($tb2->stat(), 0);
        $tb2->seekNext(Transactd::ROW_LOCK_X); // lock(X) 4th
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, 'The 4th');
        $tb2->update(); // auto trn commit and unlock all locks

        // Test unlock all locks, after update
        $tb->seekFirst(Transactd::ROW_LOCK_X); // lock(X) first
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekNext(Transactd::ROW_LOCK_X); // lock(X) second
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekNext(Transactd::ROW_LOCK_X); // lock(X) third
        $this->assertEquals($tb2->stat(), 0);
        $this->assertEquals($tb->getFVstr(FDI_NAME), 'The 3rd');
        
        // Test Insert, After record lock  operation
        $tb->setFV(FDI_ID, 21000);
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        
        //cleanup
        $tb->setFV(FDI_ID, 21000);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $tb->del();
        $this->assertEquals($tb->stat(), 0);
        
        // --------- Unlock test ---------------------------
        // 1 unlock()
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        
        $tb->unlock();
        
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), 0);
        $tb2->unlock();
        
        // 2 auto tran ended
        $tb3 = $this->openTable($db2);
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), 0);
        
        $tb3->seekLast(); //This operation is another table handle, then auto tran ended
        $this->assertEquals($tb3->stat(), 0);
        
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        $tb->unlock();
        
        // begin trn
        $tb3->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb3->stat(), 0);
        
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        $db2->beginTrn();
        
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        $db2->endTrn();
        $tb->unlock();
        // begin snapshot
        $tb3->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb3->stat(), 0);
        
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
        $db2->beginSnapshot();
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        $db2->endSnapshot();
        $tb->unlock();
        // close Table
        $tb->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_LOCK_ERROR);
        $tb->release();
        $tb2->seekFirst(Transactd::ROW_LOCK_X);
        $this->assertEquals($tb2->stat(), 0);
        $tb2->unlock();
        // --------- End Unlock test -----------------------

        // --------- Invalid lock type test ----------------
        $tb2->seekFirst(Transactd::ROW_LOCK_S);
        $this->assertEquals(Transactd::STATUS_INVALID_LOCKTYPE, $tb2->stat());
    }
    private function isMySQL5_7($db)
    {
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) &&
            ((5 == $server_ver->majorVersion) &&
            (7 == $server_ver->minorVersion));
    }
    public function testExclusive()
    {
        // db mode exclusive
        $db = new Database();
        // ------------------------------------------------------
        // database WRITE EXCLUSIVE
        // ------------------------------------------------------
        $db->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable(TABLENAME);
        $this->assertEquals($db->stat(), 0);
        
        // Can not open database from other connections.
        $db2 = new Database();
        $db2->connect(URL_DB, true);
        $this->assertEquals($db2->stat(), 0);
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF);
        // database open error. Check database::stat()
        $this->assertEquals($db2->stat(), Transactd::STATUS_CANNOT_LOCK_TABLE);
        $tb->close();
        $db->close();
        $db2->close();
        
        // ------------------------------------------------------
        // database READ EXCLUSIVE
        // ------------------------------------------------------
        $db->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_READONLY_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable(TABLENAME, Transactd::TD_OPEN_READONLY_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        
        $mysql5_7 = $this->isMySQL5_7($db);
        
        // Read only open
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF);
        $this->assertEquals($db2->stat(), 0);
        $db2->close();
        
        // Normal open
        //      Since MySQL 5.7 : D_OPEN_READONLY_EXCLUSIVE + TD_OPEN_NORMAL is fail,
        //      It's correct.
        //

        $db2->connect(URL_DB, true);
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        if ($mysql5_7 == true) {
            $this->assertEquals($db2->stat(), Transactd::STATUS_CANNOT_LOCK_TABLE);
        } else {
            $this->assertEquals($db2->stat(), 0);
        }
        $db2->close();
        
        // Write Exclusive open
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db2->stat(), Transactd::STATUS_CANNOT_LOCK_TABLE);
        $db2->close();
        
        // Read Exclusive open
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_READONLY_EXCLUSIVE);
        $this->assertEquals($db2->stat(), 0);
        $db2->close();
        $tb->close();
        $db->close();
        
        // ------------------------------------------------------
        // Normal and Exclusive open tables mix use
        // ------------------------------------------------------
        $db->open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable(TABLENAME, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $db2->open(URL, Transactd::TYPE_SCHEMA_BDF);
        $this->assertEquals($db2->stat(), 0);
        
        $tb2 = $db->openTable('group', Transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        
        // Check tb2 Exclusive
        $tb3 = $db2->openTable('group', Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db2->stat(), Transactd::STATUS_CANNOT_LOCK_TABLE);
        for ($i = 1; $i < 5; $i++) {
            $tb2->setFV(FDI_ID, $i + 1);
            $tb2->setFV(FDI_NAME, $i + 1);
            $tb2->insert();
            $this->assertEquals($tb2->stat(), 0);
        }
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekLast();
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0);
        // Normal close first
        $tb->close();
        $tb2->seekLast();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        
        // Reopen Normal
        $tb = $db->openTable('user');
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekLast();
        $this->assertEquals($tb2->stat(), 0);
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0);
        // Exclusive close first
        $tb2->close();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0);
        
        // ------------------------------------------------------
        // Normal and Exclusive open tables mix transaction
        // ------------------------------------------------------
        $tb2 = $db->openTable('group', Transactd::TD_OPEN_EXCLUSIVE);
        $this->assertEquals($db->stat(), 0);
        // Check tb2 Exclusive
        $tb3 = $db2->openTable('group', Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db2->stat(), Transactd::STATUS_CANNOT_LOCK_TABLE);
        
        $db->beginTrn();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_NAME, 'mix trn');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, 'first mix trn tb2');
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        
        $tb2->seekNext();
        $tb2->setFV(FDI_NAME, 'second mix trn tb2');
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        $db->endTrn();
        $tb2->seekFirst();
        $v = $tb2->getFVstr(FDI_NAME);
        $this->assertEquals($v, 'first mix trn tb2');
        $tb2->seekNext();
        $v = $tb2->getFVstr(FDI_NAME);
        $this->assertEquals($v, 'second mix trn tb2');
        
        $tb2->close();
        $tb->close();
    }
    public function testMultiDatabase()
    {
        $db  = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $db2 = new Database();
        $this->openDatabase($db2);
        $tb2 = $db2->openTable('group');
        $this->assertEquals($db2->stat(), 0);
        $this->assertNotEquals($tb2, null);
        
        $db->beginTrn();
        $db2->beginTrn();
        
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $v = $tb->getFVstr(FDI_NAME);
        $tb->setFV(FDI_NAME, 'MultiDatabase');
        $tb->update();
        
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb2->setFV(FDI_NAME, 'MultiDatabase');
        $tb2->update();
        $this->assertEquals($tb2->stat(), 0);
        $db2->endTrn();
        $db->abortTrn();
        
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $v2 = $tb->getFVstr(FDI_NAME);
        $this->assertEquals($v, $v2);
    }
    public function testMissingUpdate()
    {
        if (! class_exists('Thread')) {
            echo(' * class Thread not found! * ');
            return;
        }
        $db = new Database();
        $tb = $this->openTable($db);
        PooledDbManager::setMaxConnections(3);
        // Lock last record and insert to next of it
        /*$w = new SeekLessThanWorker();
        $tb->setFV(FDI_ID, 300000);
        $tb->seekLessThan(false, Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        if ($tb->stat() == 0)
        {
            // Get lock(X) same record in parallel.
            $w->start();
            usleep(5000);
            $v = $tb->getFVint(FDI_ID);
            $tb->setFV(FDI_ID, ++$v);
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
            $w->join();
            $v2 = $w->getResult();

            if ($db->trxIsolationServer() == Transactd::SRV_ISO_REPEATABLE_READ)
            {   // $tb can not insert because $tb2 got gap lock with SRV_ISO_REPEATABLE_READ.
                // It is deadlock!
                $this->assertEquals($tb->stat(), Transactd::STATUS_LOCK_ERROR);
            }
            else
            {   // When SRV_ISO_READ_COMMITED set, $tb2 get lock after $tb->insert.
                // But this is not READ_COMMITED !
                $this->assertEquals($tb->stat(), 0);
                $this->assertEquals($v2, $v - 1);
                // cleanup
                $tb->setFV(FDI_ID, $v);
                $tb->seek();
                $this->assertEquals($tb->stat(), 0);
                $tb->del();
                $this->assertEquals($tb->stat(), 0);
            }
        }*/
        // Lock last record and delete it
        $w = new SeekLessThanWorker();
        $tb->setFV(FDI_ID, 300000);
        $tb->seekLessThan(false, Transactd::ROW_LOCK_X);
        $this->assertEquals($tb->stat(), 0);
        if ($tb->stat() == 0) {
            // Get lock(X) same record in parallel.
            $w->start();
            usleep(5000);
            $v = $tb->getFVint(FDI_ID);
            $tb->del();
            $this->assertEquals($tb->stat(), 0);
            $w->join();
            $v2 = $w->getResult();
            $this->assertNotEquals($v, $v2);
        }
        (new PooledDbManager())->reset(0);
    }
    public function testInsert2()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $expected_count = 20002;
        if (! class_exists('Thread')) {
            $expected_count = $expected_count + 1;
        }
        // estimate count
        $count = $tb->recordCount(true);
        $this->assertEquals($tb->stat(), 0);
        $is_valid_count = (abs($count - $expected_count) < 10000);
        $this->assertTrue($is_valid_count);
        if (! $is_valid_count) {
            print("true record count = " . $expected_count . " and estimate recordCount count = " . $count);
        }
        $this->assertEquals($tb->recordCount(false), $expected_count); // true count
        $this->assertEquals($tb->stat(), 0);
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_NOT_FOUND_TI);
        $db->beginTrn();
        $tb->stepFirst();
        while ($tb->stat() == 0) {
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
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->setOwnerName("ABCDEFG");
        $this->assertEquals($tb->stat(), 0);
        $tb->clearOwnerName();
        $this->assertEquals($tb->stat(), 0);
    }
    public function testDropIndex()
    {
        $db = new Database();
        $tb = $this->openTable($db);
        $this->assertNotEquals($tb, null);
        $tb->dropIndex(false);
        $this->assertEquals($tb->stat(), 0);
    }
    public function testDropDatabase()
    {
        $db = new Database();
        $this->dropDatabase($db);
    }
    public function testLogin()
    {
        $db = new Database();
        $db->connect(URL_HOST);
        $this->assertEquals($db->stat(), 0);
        if ($db->stat() == 0) {
            // second connection
            $db2 = new Database();
            $db2->connect(URL_HOST, true);
            $this->assertEquals($db2->stat(), 0);
            unset($db2);
            $db->disconnect();
            $this->assertEquals($db->stat(), 0);
        }
        // invalid host name
        $db->connect(PROTOCOL . 'localhost123/');
        $is_valid_stat = ($db->stat() == Transactd::ERROR_TD_INVALID_CLINETHOST) ||
                         ($db->stat() == Transactd::ERROR_TD_HOSTNAME_NOT_FOUND);
        $this->assertTrue($is_valid_stat);
        if (! $is_valid_stat) {
            print('bad host $db->stat() = ' . $db->stat());
        }
        $this->createDatabase($db);
        $this->createTable($db);
        $db->close();
        $this->assertEquals($db->stat(), 0);
        // true database name
        $db->connect(URL_DB);
        $this->assertEquals($db->stat(), 0);
        if ($db->stat() == 0) {
            $db->disconnect();
            $this->assertEquals($db->stat(), 0);
        }
        // invalid database name
        $this->dropDatabase($db);
        $db->disconnect();
        $this->assertEquals($db->stat(), 1);
        $db->connect(URL_DB);
        $this->assertEquals($db->stat(), Transactd::ERROR_NO_DATABASE);
        $db->disconnect();
        $this->assertEquals($db->stat(), 1);
    }
    
    //-----------------------------------------------------
    //    transactd var tables
    //-----------------------------------------------------

    private function isWindows()
    {
        return (strtolower(substr(PHP_OS, 0, 3)) == 'win');
    }
    private function isUtf16leSupport($db)
    {
        // CHARSET_UTF16LE supported on MySQL 5.6 or later
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        if ('M' == chr($server_ver->type) || 'A' == chr($server_ver->type)) {
            if ($server_ver->majorVersion <= 4) {
                return false;
            } elseif ($server_ver->majorVersion == 5 && $server_ver->minorVersion <= 5) {
                return false;
            }
            return true;
        }
        return false;
    }
    private function createVarTable($db, $id, $name, $fieldType, $charset)
    {
        // create table
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
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
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // name
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = $fieldType;
        if ($fieldType == Transactd::ft_mywvarchar) {
            $fd->len = 1 + Transactd::charsize(Transactd::CHARSET_UTF16LE) * 3;
        } // max 3 char len byte
        elseif ($fieldType == Transactd::ft_mywvarbinary) {
            $fd->len = 1 + Transactd::charsize(Transactd::CHARSET_UTF16LE) * 3;
        } // max 6 char len byte
        elseif ($fieldType == Transactd::ft_myvarchar) {
            if ($charset == Transactd::CHARSET_CP932) {
                $fd->len = 1 + Transactd::charsize(Transactd::CHARSET_CP932) * 3;
            }  // max 6 char len byte
            elseif ($charset == Transactd::CHARSET_UTF8B4) {
                $fd->len = 1 + Transactd::charsize(Transactd::CHARSET_UTF8B4) * 3;
            } // max 6 char len byte
        } else {
            $fd->len = 7;
        } // max 6 char len byte
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        // groupid
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('groupid');
        $fd->type = Transactd::ft_integer;
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
        $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
    }
    
    public function testCreateDatabaseVar()
    {
        $db = new Database();
        $db->create(URL_VAR);
        if ($db->stat() == Transactd::STATUS_TABLE_EXISTS_ERROR) {
            $this->testDropDatabaseVar();
            $db->create(URL_VAR);
        }
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat()) {
            $db->open(URL_VAR, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
            $this->assertEquals($db->stat(), 0);
        }
        if (0 == $db->stat()) {
            $this->createVarTable($db, 1, 'user1', Transactd::ft_myvarchar,   Transactd::CHARSET_CP932);
            $this->createVarTable($db, 2, 'user2', Transactd::ft_myvarbinary, Transactd::CHARSET_CP932);
            if ($this->isUtf16leSupport($db)) {
                $this->createVarTable($db, 3, 'user3', Transactd::ft_mywvarchar,  Transactd::CHARSET_CP932);
            }
            $this->createVarTable($db, 4, 'user4', Transactd::ft_mywvarbinary,    Transactd::CHARSET_CP932);
            $this->createVarTable($db, 5, 'user5', Transactd::ft_myvarchar,       Transactd::CHARSET_UTF8B4);
            $db->close();
            $db->open(URL_VAR);
            $this->assertEquals($db->stat(), 0);
        }
        $db->close();
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
        $tb->setFV(FDI_NAME, '1234567');
        if ($varCharField) {
            $this->assertEquals($tb->getFVstr(FDI_NAME), '123');
        } else {
            $this->assertEquals($tb->getFVstr(FDI_NAME), '123456');
        }
        //if ($this->isWindows())
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //else
            $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        // short string
        $tb->setFV(FDI_NAME, '13 ');
        $this->assertEquals($tb->getFVstr(FDI_NAME), '13 ');
        //if ($this->isWindows())
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //else
            $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        // too long kanji
        if ($unicodeField) {
            if ($this->isWindows()) {
                $tb->setFV(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
                if ($varCharField) {
                    $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいう');
                } else {
                    $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいうえお');
                }
            }
        } else {
            $tb->setFV(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
            $is_valid_value = ($tb->getFVstr(FDI_NAME) == '0松本');
            $this->assertTrue($is_valid_value);
            if (! $is_valid_value) {
                print_r($tb->getFVstr(FDI_NAME));
            }
        }
        $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        //// Set Wide Get Ansi
        //if ($this->isWindows())
        //{
        //    // too long string
        //    $tb->setFVW(FDI_NAME, '1234567');
        //    if ($varCharField)
        //        $this->assertEquals($tb->getFVstr(FDI_NAME), '123');
        //    else
        //        $this->assertEquals($tb->getFVstr(FDI_NAME), '123456');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // short string
        //    $tb->setFVW(1, '23 ');
        //    $this->assertEquals($tb->getFVstr(FDI_NAME), '23 ');
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //    // too long kanji
        //    if ($unicodeField)
        //    {
        //        $tb->setFVW(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
        //        if ($varCharField)
        //            $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいう');
        //        else
        //            $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいうえお');
        //    }
        //    else
        //    {
        //        $tb->setFVW(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
        //        $this->assertEquals($tb->getFVstr(FDI_NAME), '0松本');
        //    }
        //    $this->assertEquals($tb->getFVWstr(FDI_GROUP), '68');
        //}
        // Set Ansi Get Ansi
        // too long string
        $tb->setFV(FDI_NAME, '1234567');
        if ($varCharField) {
            $this->assertEquals($tb->getFVstr(FDI_NAME), '123');
        } else {
            $this->assertEquals($tb->getFVstr(FDI_NAME), '123456');
        }
        $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        // short string
        $tb->setFV(FDI_NAME, '13 ');
        $this->assertEquals($tb->getFVstr(FDI_NAME), '13 ');
        $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
        // too long lanji
        if ($unicodeField) {
            if ($this->isWindows()) {
                $tb->setFV(FDI_NAME, 'あいうえお𩸽'); // hiragana 'aiueo' kanji 'hokke'
                if ($varCharField) {
                    $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいう');
                } else {
                    $this->assertEquals($tb->getFVstr(FDI_NAME), 'あいうえお');
                }
            }
        } else {
            $tb->setFV(FDI_NAME, '0松本市'); // numeric '0' kanji 'matumostoshi'
            $this->assertEquals($tb->getFVstr(FDI_NAME), '0松本');
        }
        $this->assertEquals($tb->getFVstr(FDI_GROUP), '68');
    }
    public function testVarField()
    {
        $db = new Database();
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
        if ($this->isUtf16leSupport($db)) {
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
        $db->close();
    }
    private function doVarInsert($db, $name, $codePage, $str, $startid, $endid, $bulk)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        if ($bulk) {
            $tb->beginBulkInsert(BULKBUFSIZE);
        }
        for ($i = $startid; $i <= $endid; $i++) {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, $str . $i);
            $tb->setFV(FDI_GROUP, "" . ($i + 10));
            $tb->insert();
        }
        if ($bulk) {
            $tb->commitBulkInsert();
        }
        $this->assertEquals($tb->stat(), 0);
    }
    public function testVarInsert()
    {
        $db = new Database();
        $startid = 1;
        $bulk = false;
        $str = '漢字文字のテスト'; // too long kanji
        $str2 = '123';
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat()) {
            $utf16leSupport = $this->isUtf16leSupport($db);
            $this->doVarInsert($db, 'user1', Transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', Transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            if ($utf16leSupport) {
                $this->doVarInsert($db, 'user3', Transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            }
            $this->doVarInsert($db, 'user4', Transactd::CP_ACP,   $str, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', Transactd::CP_UTF8,  $str, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $this->doVarInsert($db, 'user1', Transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user2', Transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            if ($utf16leSupport) {
                $this->doVarInsert($db, 'user3', Transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            }
            $this->doVarInsert($db, 'user4', Transactd::CP_ACP,   $str2, $startid, $startid, $bulk);
            $this->doVarInsert($db, 'user5', Transactd::CP_UTF8,  $str2, $startid, $startid, $bulk);
            $startid = $startid + 1;
            $bulk = true;
            $endid = 1000;
            $this->doVarInsert($db, 'user1', Transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user2', Transactd::CP_ACP,   '', $startid, $endid, $bulk);
            if ($utf16leSupport) {
                $this->doVarInsert($db, 'user3', Transactd::CP_ACP,   '', $startid, $endid, $bulk);
            }
            $this->doVarInsert($db, 'user4', Transactd::CP_ACP,   '', $startid, $endid, $bulk);
            $this->doVarInsert($db, 'user5', Transactd::CP_UTF8,  '', $startid, $endid, $bulk);
        }
        $db->close();
    }
    private function doVarRead($db, $name, $codePage, $str, $num, $ky)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        $tb->setKeyNum($ky);
        if ($ky == 0) {
            $tb->setFV(FDI_ID, $num);
        } else {
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
        $db = new Database();
        $str = '漢字文';
        $str3 = '漢字文字のテ';
        $str2 ='123';
        $str4 ='1232';
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat()) {
            $utf16leSupport = $this->isUtf16leSupport($db);
            $num = 1;
            $ky = 0;
            // too long string
            $this->doVarRead($db, 'user1', Transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarRead($db, 'user2', Transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport) {
                $this->doVarRead($db, 'user3', Transactd::CP_ACP,   $str,  $num, $ky);
            }
            $this->doVarRead($db, 'user4', Transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarRead($db, 'user5', Transactd::CP_UTF8,  $str,  $num, $ky);
            // short string
            $num = $num + 1;
            $this->doVarRead($db, 'user1', Transactd::CP_ACP,   $str2, $num, $ky);
            $this->doVarRead($db, 'user2', Transactd::CP_ACP,   $str4, $num, $ky);
            if ($utf16leSupport) {
                $this->doVarRead($db, 'user3', Transactd::CP_ACP,   $str2, $num, $ky);
            }
            $this->doVarRead($db, 'user4', Transactd::CP_ACP,   $str4, $num, $ky);
            $this->doVarRead($db, 'user5', Transactd::CP_UTF8,  $str2, $num, $ky);
            $ky = 1;
            $this->doVarRead($db, 'user1', Transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user2', Transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport) {
                $this->doVarRead($db, 'user3', Transactd::CP_ACP,   '120', 120, $ky);
            }
            $this->doVarRead($db, 'user4', Transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarRead($db, 'user5', Transactd::CP_UTF8,  '120', 120, $ky);
        }
        $db->close();
    }
    private function doVarFilter($db, $name, $codePage, $str, $num, $ky)
    {
        $tb = $db->openTable($name);
        $this->assertEquals($db->stat(), 0);
        $tb->clearBuffer();
        $tb->setKeyNum($ky);
        if ($ky == 0) {
            $buf = 'id > ' . $num . ' and id <= ' . ($num + 10);
            $tb->setFilter($buf, 0, 10);
            // find forword
            $tb->setFV(FDI_ID, $num);
            $tb->seekGreater(true);
            $this->assertEquals($tb->stat(), 0);
            for ($i = ($num + 1); $i <= ($num + 10); $i++) {
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
            for ($i = $num + 10; $i <= $num + 1; $i--) {
                $tb->findPrev(false);
                $this->assertEquals($tb->stat(), 0);
                // test read of var field
                $this->assertEquals($tb->getFVint(FDI_NAME), $i);
                // test read of second field
                $this->assertEquals($tb->getFVint(FDI_GROUP), $i + 10);
            }
            // test record count
            $this->assertEquals($tb->recordCount(), 10);
        } else {
            $v = $num + 10;
            $tb->setFV(FDI_NAME, $str);
            $tb->setFV(FDI_GROUP, $v);
        }
    }
    public function testFilterVar()
    {
        $db = new Database();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        if (0 == $db->stat()) {
            $str = '漢字文';
            $str3 = '漢字文字のテ';
            $utf16leSupport = $this->isUtf16leSupport($db);
            $num = 10;
            $ky = 0;
            $this->doVarFilter($db, 'user1', Transactd::CP_ACP,   $str,  $num, $ky);
            $this->doVarFilter($db, 'user2', Transactd::CP_ACP,   $str,  $num, $ky);
            if ($utf16leSupport) {
                $this->doVarFilter($db, 'user3', Transactd::CP_ACP,   $str,  $num, $ky);
            }
            $this->doVarFilter($db, 'user4', Transactd::CP_ACP,   $str3, $num, $ky);
            $this->doVarFilter($db, 'user5', Transactd::CP_UTF8,  $str,  $num, $ky);
            //if (UNICODE)
            //{
            //    // short string
            //    $num = $num + 1;
            //    $this->doVarFilter($db, 'user1', Transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user2', Transactd::CP_ACP,  $str4, $num, $ky);
            //    if ($utf16leSupport)
            //        $this->doVarFilter($db, 'user3', Transactd::CP_ACP,  $str2, $num, $ky);
            //    $this->doVarFilter($db, 'user4', Transactd::CP_ACP,  $str4, $num, $ky);
            //    $this->doVarFilter($db, 'user5', Transactd::CP_UTF8, $str2, $num, $ky);
            //}
            $ky = 1;
            $this->doVarFilter($db, 'user1', Transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user2', Transactd::CP_ACP,   '120', 120, $ky);
            if ($utf16leSupport) {
                $this->doVarFilter($db, 'user3', Transactd::CP_ACP,   '120', 120, $ky);
            }
            $this->doVarFilter($db, 'user4', Transactd::CP_ACP,   '120', 120, $ky);
            $this->doVarFilter($db, 'user5', Transactd::CP_UTF8,  '120', 120, $ky);
        }
        $db->close();
    }
    public function testDropDatabaseVar()
    {
        $db = new Database();
        $db->open(URL_VAR);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
        $db->close();
    }
    
    //-----------------------------------------------------
    //    transactd StringFilter
    //-----------------------------------------------------
    private function varLenBytes($fd)
    {
        if ((($fd->type >= ft_myvarchar) && ($fd->type <= ft_mywvarbinary)) || $fd->type == ft_lstring) {
            return $fd->len < 256 ? 1 : 2;
        } elseif ($fd->type == ft_lvar) {
            return 2;
        }
        return 0;
    }

    private function blobLenBytes($fd)
    {
        if (($fd->type== ft_myblob) || ($fd->type == ft_mytext)) {
            return $fd->len - 8;
        }
        return 0;
    }
    
    private function createTableStringFilter($db, $id, $name, $type, $type2)
    {
        // create table
        $dbdef = $db->dbDef();
        $td = new Tabledef();
        $td->setTableName($name);
        $td->setFileName($name . '.dat');
        $td->id = $id;
        $td->pageSize = 2048;
        $td->charsetIndex = Transactd::CHARSET_UTF8B4;
        // $td->charsetIndex = Transactd::CHARSET_CP932;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = $type;
        $fd->len = 44;
        if ($this->varLenBytes($fd) != 0) {
            $fd->len = $this->varLenBytes($fd) + 44;
            $fd->keylen = $fd->len;
        }
        if ($this->blobLenBytes($fd) != 0) {
            $fd->len = 12;
        } // 8+4
        $fd->keylen = $fd->len;
        $dbdef->updateTableDef($id);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('namew');
        $fd->type = $type2;
        $fd->len = 44;
        if ($this->varLenBytes($fd) != 0) {
            $fd->len = $this->varLenBytes($fd) + 44;
            $fd->keylen = $fd->len;
        }
        if ($this->blobLenBytes($fd) != 0) {
            $fd->len = 12;
        } // 8+4
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        
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
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        
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
        $db = new Database();
        $db->create(URL_SF);
        if ($db->stat() == Transactd::STATUS_TABLE_EXISTS_ERROR) {
            $this->testDropDatabaseStringFilter();
            $db->create(URL_SF);
        }
        $this->assertEquals($db->stat(), 0);
        $db->open(URL_SF, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $this->doTestStringFilter($db, 1, 'zstring', Transactd::ft_zstring, Transactd::ft_wzstring);
        if ($this->isUtf16leSupport($db)) {
            $this->doTestStringFilter($db, 2, 'myvarchar', Transactd::ft_myvarchar, Transactd::ft_mywvarchar);
        } else {
            $this->doTestStringFilter($db, 2, 'myvarchar', Transactd::ft_myvarchar, Transactd::ft_myvarchar);
        }
        $this->doTestStringFilter($db, 3, 'mytext', Transactd::ft_mytext, Transactd::ft_myblob);
        $db->close();
    }
    
    public function testDropDatabaseStringFilter()
    {
        $db = new Database();
        $db->open(URL_SF);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
        $db->close();
    }
    
    public function testQuery()
    {
        $q = new Query();
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
    
    //-----------------------------------------------------
    //    ActiveTable
    //-----------------------------------------------------

    private function createQTuser($db)
    {
        $dbdef = $db->dbDef();
        $td = new Tabledef();
        $td->setTableName('user');
        $td->setFileName('user.dat');
        $id = 1;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Transactd::ft_autoinc;
        $fd->len = 4;
        // 名前 field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('名前');
        $fd->type = Transactd::ft_myvarchar;
        $fd->setLenByCharnum(20);
        // group field
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('group');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        // tel field
        $fd = $dbdef->insertField($id, 3);
        $fd->setName('tel');
        $fd->type = Transactd::ft_myvarchar;
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
        $td = new Tabledef();
        $td->setTableName('groups');
        $td->setFileName('groups.dat');
        $id = 2;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // code field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('code');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        // name field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('name');
        $fd->type = Transactd::ft_myvarbinary;
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
        $db->openTable($id);
        $this->assertEquals($db->stat(), 0);
        return true;
    }
    private function createQTextention($db)
    {
        $dbdef = $db->dbDef();
        $td = new Tabledef();
        $td->setTableName('extention');
        $td->setFileName('extention.dat');
        $id = 3;
        $td->id = $id;
        $td->pageSize = 2048;
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id field
        $fd = $dbdef->insertField($id, 0);
        $fd->setName('id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        // comment field
        $fd = $dbdef->insertField($id, 1);
        $fd->setName('comment');
        $fd->type = Transactd::ft_myvarchar;
        $fd->setLenByCharnum(60);
        // blob field
        $fd = $dbdef->insertField($id, 2);
        $fd->setName('blob');
        $fd->type = Transactd::ft_myblob;
        $fd->len = 10;
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
        // insert user data
        $tb = $db->openTable('user', Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $db->beginTrn();
        $tb->clearBuffer();
        for ($i = 1; $i <= $maxId; $i++) {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i user");
            $tb->setFV('group', (($i - 1) % 5) + 1);
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $db->endTrn();
        $tb->close();
        // insert groups data
        $tb = $db->openTable('groups', Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $db->beginTrn();
        $tb->clearBuffer();
        for ($i = 1; $i <= 100; $i++) {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i group");
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $db->endTrn();
        $tb->close();
        // insert extention data
        $tb = $db->openTable('extention', Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $db->beginTrn();
        $tb->clearBuffer();
        for ($i = 1; $i <= $maxId; $i++) {
            $tb->setFV(0, $i);
            $tb->setFV(1, "$i comment");
            $tb->setFV(2, "$i blob");
            $tb->insert();
            $this->assertEquals($tb->stat(), 0);
        }
        $db->endTrn();
    }
    
    public function testCreateQueryTest()
    {
        $db = new Database();
        // check database existence
        $db->open(URL_QT, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        if ($db->stat() !== 0) {
            echo("\nDatabase " . DBNAME_QT . " not found");
        } else {
            $def = $db->dbDef();
            $td = $def->tableDefs(3);
            if ($td != null && $td->fieldCount === 3) {
                $tb = $db->openTable('extention');
                if ($db->stat() === 0 && $tb->recordCount(false) === TEST_COUNT) {
                    return;
                }
                $tb->close();
            }
            $db->drop();
        }
        echo("\nCreate database " . DBNAME_QT . "\n");
        $db->create(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $db->open(URL_QT, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
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
        $db = new Database();
        $db->open(URL_QT);
        for ($i = 0; $i < 500; $i++) {
            $q  = new Query();
            $rq = new RecordsetQuery();
            $gq = new GroupQuery();
            $f  = new FieldNames();
            $f->addValue('abc');
            $atu = new ActiveTable($db, 'user');
            $atu->index(0);
            $atg = new ActiveTable($db, 'groups');
            $atg->index(0);
            $fns = new FieldNames();
            $fns->addValue('a');
            $s = new Sum($fns);
            $s = new Count('a');
            $s = new Avg($fns);
            $s = new Min($fns);
            $s = new Max($fns);
            $rs = new Recordset();
        }
    }
    public function testLoop()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new ActiveTable($db, 'user');
        $q = new Query();
        
        $atu->alias('名前', 'name');
        $q->where('id', '<=', 15000);
        $rs = $atu->index(0)->keyValue(1)->read($q);
        //
        // loop fielddefs
        //
        $fds = $rs->fielddefs();
        // for
        for ($field_id = 0; $field_id < count($fds); $field_id++) {
            $field_name = $fds[$field_id]->name();
            //echo("$field_id : $field_name\n");
        }
        // foreach
        $field_id = 0;
        foreach ($fds as $fd) {
            $field_name = $fd->name();
            //echo("$field_id : $field_name\n");
            $field_id++;
        }
        // foreach KeyValue
        foreach ($fds as $field_id => $fd) {
            $field_name = $fd->name();
            //echo("$field_id : $field_name\n");
        }
        // generator
        $field_id = 0;
        foreach ($fds->range() as $fd) {
            $field_name = $fd->name();
            //echo("$field_id : $field_name\n");
            $field_id++;
        }
        // generator with range
        $field_id = 1;
        foreach ($fds->range(1, 2) as $fd) {
            $field_name = $fd->name();
            //echo("$field_id : $field_name\n");
            $field_id++;
        }
        //
        // loop Recordset and Record
        //
        // for
        for ($row_id = 0; $row_id < count($rs); $row_id++) {
            $record = $rs->getRecord($row_id);
            // for loop Record
            for ($field_id = 0; $field_id < count($record); $field_id++) {
                $field_name = $fds[$field_id]->name();
                $field_value = $record[$field_id];
                $field_value = $record[$field_name];
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
            }
        }
        // foreach
        $row_id = 0;
        $rs->fetchMode = Transactd::FETCH_RECORD_INTO;
        foreach ($rs as $record) {
            $field_id = 0;
            foreach ($record as $field_value) {
                $field_name = $fds[$field_id]->name();
                $field_value = $record[$field_id];
                $field_value = $record[$field_name];
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
                $field_id++;
            }
            $row_id++;
        }
        // foreach KeyValue
        foreach ($rs as $row_id => $record) {
            $field_id = 0;
            foreach ($record as $field_name => $field_value) {
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
                $field_id++;
            }
        }
        // generator
        $row_id = 0;
        foreach ($rs->range() as $record) {
            // values generator
            $field_id = 0;
            foreach ($record as $field_value) {
                $field_name = $fds[$field_id]->name();
                $field_value = $record[$field_id];
                $field_value = $record[$field_name];
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
                $field_id++;
            }
            
            $field_id = 0;
            foreach ($record->values() as $field_value) {
                $field_name = $fds[$field_id]->name();
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
                $field_id++;
            }
            $row_id++;
        }
        // generator with range
        $row_id = 3;
        foreach ($rs->range(3, 100) as $record) {
            // keys generator
            $field_id = 0;
            foreach ($record->keys() as $field_name) {
                $field_value = $record[$field_id];
                //if ($row_id < 5) { echo("rs[$row_id][$field_id:$field_name] $field_value\n"); }
                $field_id++;
            }
            $row_id++;
        }
    }
    public function testJoin()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new ActiveTable($db, 'user');
        $atg = new ActiveTable($db, 'groups');
        $ate = new ActiveTable($db, 'extention');
        $q = new Query();
        
        $atu->alias('名前', 'name');
        $q->select('id', 'name', 'group')->where('id', '<=', 15000);
        $rs = $atu->index(0)->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 3);
        
        // Join extention::comment
        $q->reset();
        $this->assertEquals($q->selectCount(), 0);
        
        $q->select('comment')->optimize(QueryBase::joinHasOneOrHasMany);
        $this->assertEquals($q->selectCount(), 1);
        $ate->index(0)->join($rs, $q, 'id');
        $this->assertEquals($q->selectCount(), 1);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 4);
        
        // reverse and get first (so it means 'get last')
        $last = $rs->reverse()->first();
        $this->assertEquals($rs->size(), 15000);
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
        for ($i = 0; $i < 15000 / 5; $i++) {
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
            $q->select('comment')->optimize(QueryBase::joinHasOneOrHasMany), 'id');
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
        $gq = new GroupQuery();
        $gq->keyField('group', 'id');
        $count1 = new Count('count');
        $gq->addFunction($count1);
        
        $count2 = new Count('group1_count');
        $count2->when('group', '=', 1);
        $gq->addFunction($count2);
        
        $rs->groupBy($gq);
        $this->assertEquals($rs->size(), 16000);
        $this->assertEquals($rs[0]['group1_count'], 1);
        
        // clone
        $rsv = clone $rs;
        $this->assertEquals($rsv->size(), 16000);
        $gq->reset();
        $count3 = new Count('count3');
        $gq->addFunction($count3)->keyField('group');
        $this->assertEquals($gq->functionCount(), 1);
        $this->assertEquals($gq->getKeyFields()->count(), 1);
        
        $rs->groupBy($gq);
        $this->assertEquals($rs->size(), 5);
        $this->assertEquals($rsv->size(), 16000);
        
        // having
        $rq = new RecordsetQuery();
        $rq->when('group1_count', '=', 1)->or_('group1_count', '=', 2);
        $rsv->matchBy($rq);
        $this->assertEquals($rsv->size(), 3200);
        $this->assertEquals(isset($rsv), true);
        unset($rsv);
        $this->assertEquals(isset($rsv), false);
        
        // top
        $rs3 = new Recordset();
        $rs->top($rs3, 10);
        $this->assertEquals($rs3->size(), 5);
        $rs->top($rs3, 3);
        $this->assertEquals($rs3->size(), 3);
        $this->assertEquals($rs->size(), 5);
        
        // query new / delete
        $q1 = new RecordsetQuery();
        $q1->when('group1_count', '=', 1)->or_('group1_count', '=', 2);
        unset($q1);
        
        $q2 = new Query();
        $q2->where('group1_count', '=', 1)->or_('group1_count', '=', 2);
        unset($q2);
        
        $q3 = new GroupQuery();
        $q3->keyField('group', 'id');
        unset($q3);
    }
    public function testPrepareJoin()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        
        $atu = new ActiveTable($db, 'user');
        $atu->alias('名前', 'name');
        $atg = new ActiveTable($db, 'groups');
        $atg->alias('name', 'group_name');
        $ate = new ActiveTable($db, 'extention');
        $q = new Query();
        
        $q->select('id', 'name', 'group')->where('id', '<=', '?');
        $pq = $atu->prepare($q);
        
        // int value
        $rs = $atu->index(0)->keyValue(1)->read($pq, 15000);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 3);
        // string value
        $rs = $atu->index(0)->keyValue(1)->read($pq, '15000');
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 3);
        // double value
        $rs = $atu->index(0)->keyValue(1)->read($pq, 15000.000);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 3);
        // Using supply value
        $pq->supplyValue(0, 15000);
        $rs = $atu->index(0)->keyValue(1)->read($pq);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 3);
         
        // Join extention::comment
        $q->reset();
        $this->assertEquals($q->selectCount(), 0);
        $q->select('comment')->optimize(QueryBase::joinHasOneOrHasMany);
        $this->assertEquals($q->selectCount(), 1);
        $pq = $ate->prepare($q);
        
        $ate->index(0)->join($rs, $pq, 'id');
        $this->assertEquals($q->selectCount(), 1);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->fieldDefs()->size(), 4);
        // reverse and get first (so it means 'get last')
        $last = $rs->reverse()->first();
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($last['id'], 15000);
        $this->assertEquals($last['comment'], '15000 comment');
        
        // Join group::name
        $q->reset()->select('group_name');
        $pq = $atg->prepare($q);
        $atg->index(0)->join($rs, $pq, 'group');
        $this->assertEquals($rs->size(), 15000);
        
        // get last (the rs is reversed, so it means 'get first')
        $first = $rs->last();
        $this->assertEquals($first['id'], 1);
        $this->assertEquals($first['comment'], '1 comment');
        $this->assertEquals($first['group_name'], '1 group');
        
        // row in rs[15000 - 9]
        $rec = $rs[15000 - 9];
        $this->assertEquals($rec['group_name'], '4 group');
    }
    public function testServerPrepareJoin()
    {
        define('NO_RECORD_ID', 5);
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        
        $atu = new ActiveTable($db, 'user');
        $atu->alias('名前', 'name');
        $atg = new ActiveTable($db, 'groups');
        $atg->alias('name', 'group_name');
        $ate = new ActiveTable($db, 'extention', Transactd::TD_OPEN_NORMAL);
        $q = new Query();
        
        $q->select('id', 'name', 'group')->where('id', '<=', '?');
        $stmt1 = $atu->prepare($q, true);
        $this->assertNotEquals($stmt1, null);
        
        $q->reset()->select('comment')->optimize(QueryBase::joinHasOneOrHasMany);
        $stmt2 = $ate->prepare($q, true);
        $this->assertNotEquals($stmt2, null);
        
        $q->reset()->select('group_name');
        $stmt3 = $atg->prepare($q, true);
        $this->assertNotEquals($stmt3, null);
        
        $rs = $atu->index(0)->keyValue(1)->read($stmt1, 15000);
        $this->assertEquals($rs->size(), 15000);
        
        // Join extention::comment
        $ate->index(0)->join($rs, $stmt2, 'id');
        $this->assertEquals($rs->size(), 15000);
        
        // test reverse
        $last = $rs->reverse()->first();
        $this->assertEquals($last['id'], 15000);
        $this->assertEquals($last['comment'], '15000 comment');
        
        // Join group::name
        $atg->index(0)->join($rs, $stmt3, 'group');
        $this->assertEquals($rs->size(), 15000);
        $first = $rs->last();
        
        $this->assertEquals($first['id'], 1);
        $this->assertEquals($first['comment'], '1 comment');
        $this->assertEquals($first['group_name'], '1 group');
        
        // $rs[15000 - 9];
        $rec = $rs[15000 - 9];
        $this->assertEquals($rec['group_name'], '4 group');
        
        // Test orderby
        $rs->orderBy('group_name');
        // $rs[0];
        $this->assertEquals($rs[0]['group_name'], '1 group');
        
        // All fields
        $rs->clear();
        $q->reset()->all();
        $q->where('id', '<=', '?');
        $stmt1 = $atu->prepare($q, true);
        $rs = $atu->keyValue(1)->read($stmt1, 15000);
        $this->assertEquals($rs->size(), 15000);
        if ($rs->size() == 15000) {
            for ($i = 0; $i < 15000; $i++) {
                $this->assertEquals($rs[$i]['id'], $i + 1);
            }
        }
        
        $ate->join($rs, $stmt2, 'id');
        $this->assertEquals($rs->size(), 15000);
        $atg->join($rs, $stmt3, 'group');
        $this->assertEquals($rs->size(), 15000);
        
        // OuterJoin
        $tb = $ate->table();
        $tb->setFV('id', NO_RECORD_ID);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        if ($tb->stat() == 0) {
            $tb->del();
        }
        $this->assertEquals($tb->stat(), 0);
        $q->reset()->select('comment', 'blob')->optimize(QueryBase::joinHasOneOrHasMany);
        $stmt2 = $ate->prepare($q, true);
        
        // Join is remove record(s) no join target record.
        $rs->clear();
        $rs = $atu->keyValue(1)->read($stmt1, 15000);
        $ate->join($rs, $stmt2, 'id');
        $this->assertEquals($rs->size(), 14999);
        $this->assertEquals($rs[NO_RECORD_ID - 1]['id'], NO_RECORD_ID + 1);
        $this->assertEquals($rs[NO_RECORD_ID - 1]['comment'], '' . (NO_RECORD_ID + 1) . ' comment');
        $this->assertEquals($rs[NO_RECORD_ID - 1]['blob'], '' . (NO_RECORD_ID + 1) . ' blob');
        
        // OuterJoin is no remove record(s) no join target record.
        $rs->clear();
        $rs = $atu->keyValue(1)->read($stmt1, 15000);
        $ate->outerJoin($rs, $stmt2, 'id');
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->getRecord(NO_RECORD_ID - 1)->isInvalidRecord(), true);
        $atg->outerJoin($rs, $stmt3, 'group');
        $this->assertEquals($rs->size(), 15000);
        
        $this->assertEquals($rs[NO_RECORD_ID]['comment'], '' . (NO_RECORD_ID + 1) . ' comment');
        $this->assertEquals($rs[NO_RECORD_ID]['blob'], '' . (NO_RECORD_ID + 1) . ' blob');
        
        // OuterJoin All Join fields
        $q->reset()->optimize(QueryBase::joinHasOneOrHasMany)->all();
        $stmt2 = $ate->prepare($q, true);
        $rs->clear();
        $rs = $atu->keyValue(1)->read($stmt1, 15000);
        $ate->outerJoin($rs, $stmt2, 'id');
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs->getRecord(NO_RECORD_ID - 1)->isInvalidRecord(), true);
        $this->assertEquals($rs->getRecord(NO_RECORD_ID - 1)->getField('comment')->isNull(), true);
        $this->assertEquals($rs[NO_RECORD_ID]['comment'], '' . (NO_RECORD_ID + 1) . ' comment');
        $this->assertEquals($rs[NO_RECORD_ID]['blob'], '' . (NO_RECORD_ID + 1) . ' blob');
        
        // Test clone blob field
        $rs2 = clone($rs);
        $this->assertEquals($rs2->size(), 15000);
        //$this->assertEquals($rs2[NO_RECORD_ID - 1]->isInvalidRecord(), true);
        $this->assertEquals($rs2->getRecord(NO_RECORD_ID - 1)->getField('comment')->isNull(), true);
        $this->assertEquals($rs2[NO_RECORD_ID]['comment'], '' . (NO_RECORD_ID + 1) . ' comment');
        $this->assertEquals($rs2[NO_RECORD_ID]['blob'], '' . (NO_RECORD_ID + 1) . ' blob');
        
        // hasManyJoin inner
        $rs->clear();
        $q->reset()->reject(0xFFFF)->limit(0)->all();
        $rs = $atg->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 100);
        $q->all()->optimize(QueryBase::joinHasOneOrHasMany);
        $atu->index(1)->join($rs, $q, 'code');
        $this->assertEquals($rs->size(), 20000);
        
        // hasManyJoin outer
        $rs->clear();
        $q->reset()->reject(0xFFFF)->limit(0)->all();
        $rs = $atg->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 100);
        $q->all()->optimize(QueryBase::joinHasOneOrHasMany);
        $atu->index(1)->outerJoin($rs, $q, 'code');
        $this->assertEquals($rs->size(), 20095);
        
        // restore record
        $tb->clearBuffer();
        $tb->setFV('id', NO_RECORD_ID);
        $tb->setFV('comment', '5 comment');
        $tb->setFV('blob', '5 blob');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        if ($tb->stat() != 0) {
            $db->drop();
        }
    }
    
    public function testReadMore()
    {
        $db = new Database();
        $db->open(URL_QT);

        $this->assertEquals($db->stat(), 0);
        $atu = new ActiveTable($db, 'user');
        $atu->alias('名前', 'name');
        $q = new Query();
        
        //isStopAtLimit
        $this->assertEquals($q->isStopAtLimit(), 0);
        $q->select('id', 'name', 'group')
            ->where('name', '=', '1*')
            ->reject(70)->limit(8)->stopAtLimit(true);
        $this->assertEquals($q->isStopAtLimit(), true);
        $stmt1 = $atu->prepare($q);
        $this->assertEquals($atu->table()->stat(), 0);
        $this->assertNotEquals($stmt1, null);
        $rs = $atu->index(0)->keyValue(18)->read($stmt1);
        $this->assertEquals($rs->size(), 2);
        
        //readMore
        $rs2 = $atu->readMore();
        $this->assertEquals($rs2->size(), 8);
        $rs->unionRecordset($rs2);
        $this->assertEquals($rs->size(), 10);
    }

    public function testFirstLastGroupFunction()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        
        $atu = new ActiveTable($db, 'user');
        $atu->alias('名前', 'name');
        $q = new Query();
        $q->select('id', 'name', 'group')
              ->where('name', '=', '1*')
              ->reject(70)->limit(8)->stopAtLimit(true);
        $stmt1 = $atu->prepare($q);
        $this->assertNotEquals($stmt1, null);
        
        $rs = $atu->index(0)->keyValue(0)->read($stmt1);
        $this->assertEquals($rs->size(), 8);
        
        #grouping first and last
        $gq = new GroupQuery();
        $target = new FieldNames();
        $target->addValue('name');
        $last = new Last($target, 'last_rec_name');
        $first = new First($target, 'first_rec_name');
        $gq->addFunction($last);
        $gq->addFunction($first);
        $rs->groupBy($gq);
        $this->assertEquals($rs[0]['first_rec_name'], "1 user");
        $this->assertEquals($rs[0]['last_rec_name'], "16 user");
    }

    public function testWritableRecord()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new ActiveTable($db, 'user', Transactd::TD_OPEN_NORMAL);
        
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
    
    public function testBookmark1()
    {
        $min_id = 5;
        $max_id = 15;
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $tb = $db->openTable('user');
        $this->assertEquals($db->stat(), 0);

        $tb->clearBuffer();
        $tb->setKeyNum(0);
        $tb->setFilter('id >= ' . $min_id . ' and id <= ' . $max_id, 0, 0);
        $cnt = $tb->recordCount();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($cnt, $tb->bookmarksCount());
        $tb->moveBookmarks($cnt - 1);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), $max_id);
        $tb->moveBookmarks(0);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), $min_id);
        
        $q = new Query();
        $q->where('id', '>=', $min_id)->and_('id', '<=', $max_id)->reject(0xFFFF);
        $tb->clearBuffer();
        $tb->setQuery($q->bookmarkAlso(true));
        $cnt = $tb->recordCount();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($cnt, $tb->bookmarksCount());
        $tb->moveBookmarks($cnt - 1);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), $max_id);
        $tb->moveBookmarks(0);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), $min_id);
        
        
        $atu = new ActiveTable($db, 'user');
        $qb = new Query();
        $len = $atu->table()->bookmarkLen();
        // Hold bookmark objects to reading.
        $bm1 = $tb->bookmarks(0);
        $bm2 = $tb->bookmarks(2);
        $bm3 = $tb->bookmarks(4);
        
        $qb->addSeekBookmark($bm1, $len);
        $qb->addSeekBookmark($bm2, $len);
        $qb->addSeekBookmark($bm3, $len);
        $this->assertEquals($qb->isSeekByBookmarks(), true);
        $rs = $atu->read($qb);
        
        $this->assertEquals($cnt, $tb->bookmarksCount());
        $this->assertEquals(count($rs), 3);
        $this->assertEquals($rs[0][FDI_ID], 5);
        $this->assertEquals($rs[1][FDI_ID], 7);
        $this->assertEquals($rs[2][FDI_ID], 9);

        //read by WritableRecord
        $rec = $atu->getWritableRecord();
        $rec->read($tb->bookmarks($cnt - 1));
        $this->assertEquals($rec[FDI_ID], $max_id);
        $rec->read($tb->bookmarks(0));
        $this->assertEquals($rec[FDI_ID], $min_id);

        $db->close();
    }
    
    public function testBookmark2()
    {
        $db = new Database();
        $db->open(URL_QT);
        $this->assertEquals($db->stat(), 0);
        $atu = new ActiveTable($db, 'user');
        
        $tb = $atu->table();
        $q = new Query();
        $tb->setQuery($q->all()->bookmarkAlso(true));
        $num = $tb->recordCount();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($num, $tb->bookmarksCount());
        
        $q->reset();
        $len = $tb->bookmarkLen();
        $this->assertEquals($len, 4);
        
        // Hold bookmark objects to reading.
        $bm1 = $tb->bookmarks(0);
        $bm2 = $tb->bookmarks(10);
        $bm3 = $tb->bookmarks(20);

        $q->addSeekBookmark($bm1, $len);
        $q->addSeekBookmark($bm2, $len);
        $q->addSeekBookmark($bm3, $len);
        $rs = $atu->read($q);
        $this->assertEquals($rs->count(), 3);
        
        $this->assertEquals($rs[0][FDI_ID], 1);
        $this->assertEquals($rs[1][FDI_ID], 11);
        $this->assertEquals($rs[2][FDI_ID], 21);
        
        //Read by table
        $tb->setQuery($q, false);
        $tb->find();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $tb->findNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 11);
        $tb->findNext();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 21);
        $tb->findNext();
        $this->assertEquals($tb->stat(), 9);
    }
}
