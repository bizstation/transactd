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
use BizStation\Transactd\Tabledef;
use BizStation\Transactd\Database;
use BizStation\Transactd\Table;
use BizStation\Transactd\Query;
use BizStation\Transactd\ActiveTable;

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
define("URL", "tdap://" . USERPART . HOSTNAME . "test_blob?dbfile=test.bdf" . PASSPART);
define("TABLENAME", "comments");
define("FDI_ID", 0);
define("FDI_USER_ID", 1);
define("FDI_BODY", 2);
define("FDI_IMAGE", 3);

class TransactdBlobTest extends PHPUnit_Framework_TestCase
{
    private function dropDatabase($db, $url)
    {
        //$db->open($url);
        //$this->assertEquals($db->stat(), 0);
        $db->drop($url);
        $this->assertEquals($db->stat(), 0);
    }
    private function createDatabase($db, $url)
    {
        $db->create($url);
        if ($db->stat() == Transactd::STATUS_TABLE_EXISTS_ERROR) {
            $this->dropDatabase($db, $url);
            $db->create($url);
            $this->assertEquals($db->stat(), 0);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db, $url)
    {
        $db->open($url, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
    }
    private function createTable($db, $tableid, $tablename)
    {
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
        // Set table schema codepage to UTF-8
        //   - codepage for field NAME and tableNAME
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName($tablename);
        $td->setFileName($tablename . '.dat');
        // Set table default charaset index
        //    - default charset for field VALUE
        $td->charsetIndex = Transactd::charsetIndex(Transactd::CP_UTF8);
        //
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        // id
        $fd = $dbdef->insertField($tableid, FDI_ID);
        $fd->setName('id');
        $fd->type = Transactd::ft_autoinc;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // user_id
        $fd = $dbdef->insertField($tableid, FDI_USER_ID);
        $fd->setName('user_id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // body
        $fd = $dbdef->insertField($tableid, FDI_BODY);
        $fd->setName('body');
        $fd->type = Transactd::ft_mytext;
        $fd->len = 10; // 9:TYNYTEXT 10:TEXT 11:MIDIUMTEXT 12:LONGTEXT
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // image
        $fd = $dbdef->insertField($tableid, FDI_IMAGE);
        $fd->setName('image');
        $fd->type = Transactd::ft_myblob;
        $fd->len = 10; // 9:TYNYBLOB 10:BLOB 11:MIDIUMBLOB 12:LONGBLOB
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        // key
        $kd = $dbdef->insertKey($tableid, 0);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
    }
    private function openTable($db, $tablename)
    {
        $tb = $db->openTable($tablename);
        $this->assertEquals($db->stat(), 0);
        return $tb;
    }
    private function getTestBinary()
    {
        $image_base64 = 'R0lGODdhEAAQAKEBAGZmZv///5mZmczMzCwAAAAAEAAQAAACRowzIgA6BxebTMAgG60nW5NM1kAZikGFHAmgYvYgJpW12FfTyLpJjz+IVSSXR4IlQCoUgCCG8ds0D5xZT3TJYS8IZiMJKQAAOw==';
        return base64_decode($image_base64);
    }
    
    public function testCreate()
    {
        $db = new Database();
        $this->createDatabase($db, URL);
        $this->openDatabase($db, URL);
        $this->createTable($db, 1, TABLENAME);
        $this->openTable($db, TABLENAME);
        $db->close();
    }
    public function testInsert()
    {
        $image = $this->getTestBinary();
        $db = new Database();
        $this->openDatabase($db, URL);
        $tb = $this->openTable($db, TABLENAME);
        $this->assertNotEquals($tb, null);
        // 1
        $tb->clearBuffer();
        $tb->setFV(FDI_USER_ID, 1);
        $tb->setFV(FDI_BODY, "1\ntest\nテスト\n\nあいうえおあいうえお");
        $tb->setFV(FDI_IMAGE, $image, strlen($image));
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        // 2
        $tb->clearBuffer();
        $tb->setFV('user_id', 1);
        $tb->setFV('body', "2\ntest\nテスト\n\nあいうえおあいうえお");
        $str = "2\ntest\nテスト\n\nあいうえおあいうえお";
        $tb->setFV('image', $str, strlen($str));
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        // 3
        $tb->clearBuffer();
        $tb->setFV(FDI_USER_ID, 2);
        $tb->setFV(FDI_BODY, "3\ntest\nテスト\n\nあいうえおあいうえお");
        $str = "3\ntest\nテスト\n\nあいうえおあいうえお";
        $tb->setFV(FDI_IMAGE, $str, strlen($str));
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        $db->close();
    }
    public function testSeek()
    {
        $db = new Database();
        $this->openDatabase($db, URL);
        $tb = $this->openTable($db, TABLENAME);
        $this->assertNotEquals($tb, null);
        // 1
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 1);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "1\ntest\nテスト\n\nあいうえおあいうえお");
        $image = $this->getTestBinary();
        $this->assertEquals($tb->getFVbin(FDI_IMAGE), $image);
        // 2
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "2\ntest\nテスト\n\nあいうえおあいうえお");
        $this->assertEquals($tb->getFVbin(FDI_IMAGE), "2\ntest\nテスト\n\nあいうえおあいうえお");
        // 3
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 3);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 2);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "3\ntest\nテスト\n\nあいうえおあいうえお");
        $this->assertEquals($tb->getFVbin(FDI_IMAGE), "3\ntest\nテスト\n\nあいうえおあいうえお");
        // 2
        $tb->seekPrev();
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "2\ntest\nテスト\n\nあいうえおあいうえお");
        $this->assertEquals($tb->getFVbin(FDI_IMAGE), "2\ntest\nテスト\n\nあいうえおあいうえお");
        $db->close();
    }
    public function testFind()
    {
        $db = new Database();
        $this->openDatabase($db, URL);
        $tb = $this->openTable($db, TABLENAME);
        $this->assertNotEquals($tb, null);
        // 1
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('id >= 1 and id < 3', 1, 0);
        $this->assertEquals($tb->stat(), 0);
        $tb->setFV(FDI_ID, 1);
        $tb->find(Table::findForword);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "1\ntest\nテスト\n\nあいうえおあいうえお");
        // 2
        $tb->findNext(true);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "2\ntest\nテスト\n\nあいうえおあいうえお");
        // 3... but not found because filtered
        $tb->findNext(true);
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        $db->close();
    }
    public function testUpdate()
    {
        $db = new Database();
        $this->openDatabase($db, URL);
        $tb = $this->openTable($db, TABLENAME);
        $this->assertNotEquals($tb, null);
        // select 1
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 1);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "1\ntest\nテスト\n\nあいうえおあいうえお");
        // update
        $tb->setFV(FDI_BODY, "1\nテスト\ntest\n\nABCDEFG");
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        // select 2
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "2\ntest\nテスト\n\nあいうえおあいうえお");
        // update
        $tb->setFV(FDI_BODY, "2\nテスト\ntest\n\nABCDEFG");
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        // check 1
        $tb->seekPrev();
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "1\nテスト\ntest\n\nABCDEFG");
        // check 2
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 2);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "2\nテスト\ntest\n\nABCDEFG");
        $db->close();
    }
    public function testDelete()
    {
        $db = new Database();
        $this->openDatabase($db, URL);
        $tb = $this->openTable($db, TABLENAME);
        $this->assertNotEquals($tb, null);
        // delete 2
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 2);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $tb->del();
        $this->assertEquals($tb->stat(), 0);
        // select 1
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, 1);
        $tb->seek();
        $this->assertEquals($tb->getFVint(FDI_ID), 1);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 1);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "1\nテスト\ntest\n\nABCDEFG");
        // next is 3
        $tb->seekNext();
        $this->assertEquals($tb->getFVint(FDI_ID), 3);
        $this->assertEquals($tb->getFVint(FDI_USER_ID), 2);
        $this->assertEquals($tb->getFVstr(FDI_BODY), "3\ntest\nテスト\n\nあいうえおあいうえお");
        // eof
        $tb->seekNext();
        $this->assertEquals($tb->stat(), Transactd::STATUS_EOF);
        $db->close();
    }
    public function testRecord()
    {
        $image = $this->getTestBinary();
        $db = new Database();
        $this->openDatabase($db, URL);
        $at = new ActiveTable($db, TABLENAME);
        $q = new Query();
        $q->where('id', '=', 1);
        $rs = $at->index(0)->keyValue(1)->read($q);
        $this->assertEquals(count($rs), 1);
        $f = $rs->getRecord(0)->getField(FDI_IMAGE);
        $this->assertEquals($f->getBin(), $image);
        $db->close();
    }
    public function testDrop()
    {
        $db = new Database();
        $this->dropDatabase($db, URL);
    }
}
