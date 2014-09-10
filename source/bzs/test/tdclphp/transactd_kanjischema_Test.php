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
define("URL", "tdap://" . HOSTNAME . "test?dbfile=test.bdf");
define("URL_KANJI", "tdap://" . HOSTNAME . "テスト?dbfile=構成.bdf");
define("FDI_ID", 0);
define("FDN_ID", "番号");
define("FDI_NAME", 1);
define("FDN_NAME", "名前");

class transactdKanjiSchemaTest extends PHPUnit_Framework_TestCase
{
    private function dropDatabase($db, $url)
    {
        $db->open($url);
        $this->assertEquals($db->stat(), 0);
        $db->drop();
        $this->assertEquals($db->stat(), 0);
    }
    private function createDatabase($db, $url)
    {
        $db->create($url);
        if ($db->stat() == Bz\transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->dropDatabase($db, $url);
            $db->create($url);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db, $url)
    {
        $db->open($url, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
    }
    private function createTable($db, $tableid, $tablename)
    {
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, NULL);
        $td = new Bz\tabledef();
        /* Set table schema codepage to UTF-8
             - codepage for field NAME and tableNAME */
        $td->schemaCodePage = Bz\transactd::CP_UTF8;
        $td->setTableName($tablename);
        $td->setFileName($tablename . '.dat');
        /* Set table default charaset index
              - default charset for field VALUE */
        $td->charsetIndex = Bz\transactd::charsetIndex(Bz\transactd::CP_UTF8);
        //
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($tableid, FDI_ID);
        $fd->setName(FDN_ID);
        $fd->type = Bz\transactd::ft_integer;
        $fd->len = 4;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        $fd = $dbdef->insertField($tableid, FDI_NAME);
        $fd->setName(FDN_NAME);
        $fd->type = Bz\transactd::ft_zstring;
        $fd->len = 33;
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
        /* Set field charset index
              - charset for each field VALUE
          $fd->setCharsetIndex(Bz\transactd::charsetIndex(Bz\transactd::CP_UTF8)) */
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
    private function insert($db, $tablename)
    {
        $tb = $this->openTable($db, $tablename);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->setFV(FDN_ID, 1);
        $tb->setFV(FDN_NAME, '小坂');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDN_ID, 2);
        $tb->setFV(FDN_NAME, '矢口');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
        $tb->clearBuffer();
        $tb->setFV(FDN_ID, 3);
        $tb->setFV(FDN_NAME, 'ビズステーション');
        $tb->insert();
        $this->assertEquals($tb->stat(), 0);
    }
    private function getEqual($db, $tablename)
    {
        $tb = $this->openTable($db, $tablename);
        $this->assertNotEquals($tb, NULL);
        $tb->clearBuffer();
        $tb->setFV(FDN_ID, 1);
        $tb->seek();
        $this->assertEquals($tb->getFVstr(FDN_NAME), '小坂');
    }
    private function find($db, $tablename)
    {
        $tb = $this->openTable($db, $tablename);
        $this->assertNotEquals($tb, NULL);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $tb->setFilter('番号 >= 1 and 番号 < 3', 1, 0);
        $tb->setFV(FDN_ID, 1);
        $tb->find(Bz\table::findForword);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDN_ID), 1);
        $this->assertEquals($tb->getFVstr(FDN_NAME), '小坂');
        $tb->findNext(true);
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVint(FDN_ID), 2);
        $this->assertEquals($tb->getFVstr(FDN_NAME), '矢口');
        $tb->findNext(true);
        $this->assertEquals($tb->stat(), Bz\transactd::STATUS_EOF);
    }
    private function doWhole($db, $tableid, $tablename, $url)
    {
        $this->openDatabase($db, $url);
        $this->createTable($db, $tableid, $tablename);
        $tb = $this->openTable($db, $tablename);
        $tb->release();
        $this->insert($db, $tablename);
        $this->getEqual($db, $tablename);
        $this->find($db, $tablename);
    }
    
    public function testCreateDatabase()
    {
        $db = new Bz\database();
        $this->createDatabase($db, URL);
    }
    public function testTableWhichHasKanjiNamedField()
    {
        $db = new Bz\database();
        $this->doWhole($db, 1, 'kanji-field', URL);
    }
    public function testKanjiNamedTable()
    {
        $db = new Bz\database();
        $this->doWhole($db, 2, '漢字テーブル', URL);
    }
    public function testCreateKanjiNamedDatabase()
    {
        $db = new Bz\database();
        $this->createDatabase($db, URL_KANJI); // URL must be UTF-8
    }
    public function testTableWhichHasKanjiNamedFieldInKanjiNamedDatabase()
    {
        $db = new Bz\database();
        $this->doWhole($db, 1, 'kanji-field', URL_KANJI);
    }
    public function testKanjiNamedTableInKanjiNamedDatabase()
    {
        $db = new Bz\database();
        $this->doWhole($db, 2, '漢字テーブル', URL_KANJI);
    }
    public function testDropDatabase()
    {
        $db = new Bz\database();
        $this->dropDatabase($db, URL_KANJI);
    }
}
