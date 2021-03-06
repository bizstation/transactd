<?php
/* ================================================================
   Copyright (C) 2015-2016 BizStation Corp All rights reserved.

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
use BizStation\Transactd\Fielddef;
use BizStation\Transactd\Database;
use BizStation\Transactd\BtrVersions;
use BizStation\Transactd\Nstable;
use BizStation\Transactd\Table;
use BizStation\Transactd\Query;
use BizStation\Transactd\RecordsetQuery;
use BizStation\Transactd\ActiveTable;
use BizStation\Transactd\Bitset;
use BizStation\Transactd\ConnMgr;
use BizStation\Transactd\HaNameResolver;

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

class Number
{
   
    
}

class Phone
{
    protected $number; 
    protected $number2 = null; // Mapped object is null
    public function __construct()
    {
        $this->number = new Number;
    }
    public function __get($name)
    {
        if ($name === 'number2') {
            return $this->number2;
        } elseif ($name === 'number') {
            return $this->number;
        }
    }
}

class UserT
{
    /**
     *
     * @var array field name => property name of object. Do not use __get __set magic method
     * The mapped variable is required. If the variable is null then field values are not read or write.
     */
    static protected $transferMap = ['tel' => ['phone','number']];
    protected $phone; 
    protected $phone2 = null;// Mapped object is null
     
    public function __construct()
    {
        $this->phone = new Phone;
    }
    
    static public function setTransferMap($type)
    {
        if ($type === 0) {
            self::$transferMap = ['tel' => ['phone','number']];
        } elseif ($type === 1) {
            self::$transferMap = ['tel' => ['phone2','number']]; 
        } elseif ($type === 2) {
            self::$transferMap = ['tel' => ['phone','number2']];
        }
    }
    
    public function __get($name)
    {
        if ($name === 'phone2') {
            return $this->phone2;
        } elseif ($name === 'phone') {
            return $this->phone;
        }
    }
}

class User
{
    public $a = "";
    public $b = "";
    public $c = "";

    public function __construct($_a, $_b, $_c)
    {
        $this->a = $_a;
        $this->b = $_b;
        $this->c = $_c;
    }
}

class PropertyTest
{
    private $tmp = 4;
    private $private = 1;
    protected $protected = 2 ;       
    public $public = 3;
    private $duplicate = 5;
    private $duplicateTmp = 6;
    
    public function __get($var)
    {
        if ($var === 'magic') {
            return $this->tmp;
        }
    }
    
    public function __set($name, $value)
    {
        if ($name === 'magic') {
            $this->tmp = $value;
        } elseif ($name === 'duplicate') {
            $this->duplicateTmp = $value;
        }
    }
    
    public function __isset($name)
    {
        if ($name === 'magic') return true;
    }
    
}

define("HOSTNAME", getHost());
define("USERNAME", getenv('TRANSACTD_PHPUNIT_USER'));
define("USERPART", strlen(USERNAME) == 0 ? '' : USERNAME . '@');
define("PASSWORD", getenv('TRANSACTD_PHPUNIT_PASS'));
define("PASSPART", strlen(PASSWORD) == 0 ? '' : '&pwd=' . PASSWORD);
define("PASSPART2", strlen(PASSWORD) == 0 ? '' : '?pwd=' . PASSWORD);
define("DBNAME", "test_v3");
define("TABLENAME", "user");
define("PROTOCOL", "tdap://");
define("BDFNAME", "?dbfile=test.bdf");
define("URI", PROTOCOL . USERPART . HOSTNAME . DBNAME . BDFNAME . PASSPART);

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
            $dbm = new PooledDbManager(new ConnectParams(URI));
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
        // Version 3.1 or later is support drop by uri.
        //$db->open(URI);
        //$this->assertEquals($db->stat(), 0);
        $db->drop(URI);
        $this->assertEquals($db->stat(), 0);
    }
    private function createDatabase($db)
    {
        $db->create(URI);
        if ($db->stat() == Transactd::STATUS_TABLE_EXISTS_ERROR) {
            $this->dropDatabase($db);
            $db->create(URI);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db)
    {
        return $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
    }
    private function isMySQL5_5($db)
    {
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) &&
            ((5 == $server_ver->majorVersion) &&
            (5 == $server_ver->minorVersion));
    }
    private function isMariaDBWithGtid($db)
    {
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) &&
            (10 == $server_ver->majorVersion) &&
            ($server_ver->type == Transactd::MYSQL_TYPE_MARIA);
    }
    private function isLegacyTimeFormat($db)
    {
        $vv = new BtrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) &&
            ((5 == $server_ver->majorVersion) &&
            (5 == $server_ver->minorVersion)) &&
            ($server_ver->type == Transactd::MYSQL_TYPE_MYSQL);
    }
    private function createUserTable($db)
    {
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName(TABLENAME);
        $td->setFileName(TABLENAME . '.dat');
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $tableid = 1;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fieldIndex = 0;
        $fd = $dbdef->insertField($tableid, $fieldIndex);
        $fd->setName('id');
        $fd->type = Transactd::ft_autoinc;
        $fd->len = 4;
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('名前');
        $fd->type = Transactd::ft_myvarchar;
        $fd->len = 2;
        $this->assertEquals($fd->isValidCharNum(), false);
        $fd->SetLenByCharnum(20);
        $this->assertEquals($fd->isValidCharNum(), true);
        $fd->setDefaultValue("John");
        $this->assertEquals($fd->isPadCharType(), false);
        $this->assertEquals($fd->isDateTimeType(), false);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('group');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $fd->setNullable(true, false);
        $fd->setDefaultValue(10);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('tel');
        $fd->type = Transactd::ft_myvarchar;
        $fd->len = 4;
        $fd->setLenByCharnum(21);
        $fd->setNullable(true);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('update_datetime');
        $fd->type = Transactd::ft_mytimestamp;
        $fd->len = 7;
        $fd->setDefaultValue(Transactd::DFV_TIMESTAMP_DEFAULT);
        $fd->setTimeStampOnUpdate(true);
        $this->assertEquals($fd->isTimeStampOnUpdate(), true);

        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('create_datetime');
        if ($this->isMySQL5_5($db)) {
            $fd->type = Transactd::ft_mydatetime;
            $fd->len = 8;
        } else {
            $fd->type = Transactd::ft_mytimestamp;
            $fd->len = 4;
            $fd->setDefaultValue(Transactd::DFV_TIMESTAMP_DEFAULT);
        }
        $fd->setTimeStampOnUpdate(false);
        $this->assertEquals($fd->isTimeStampOnUpdate(), false);
        $this->assertEquals($fd->isPadCharType(), false);
        $this->assertEquals($fd->isDateTimeType(), true);
        
        $keynum = 0;
        $kd = $dbdef->insertKey($tableid, $keynum);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        $td = $dbdef->tableDefs($tableid);
        $td->primaryKeyNum = $keynum;
        
        $kd = $dbdef->insertKey($tableid, ++$keynum);
        $kd->segment(0)->fieldNum = 2;
        $kd->segment(0)->flags->bit0 = 1; // key_duplicate
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
    }
    private function createUserExtTable($db)
    {
        $this->openDatabase($db);
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
        
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName("extention");
        $td->setFileName("extention");
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $tableid = 3;
        $td->id = $tableid;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fieldIndex = 0;
        $fd = $dbdef->insertField($tableid, $fieldIndex);
        $fd->setName('id');
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('comment');
        $fd->type = Transactd::ft_myvarchar;
        $fd->SetLenByCharnum(60);
        $fd->setNullable(true);
        $this->assertEquals($fd->isDefaultNull(), true);

        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('bits');
        $fd->type = Transactd::ft_integer;
        $fd->len = 8;
        $this->assertEquals($fd->isDefaultNull(), false);
        
        $keynum = 0;
        $kd = $dbdef->insertKey($tableid, $keynum);
        $kd->segment(0)->fieldNum = 0;
        $kd->segment(0)->flags->bit8 = 1;
        $kd->segment(0)->flags->bit1 = 1;
        $kd->segmentCount = 1;
        $td = $dbdef->tableDefs($tableid);
        $td->primaryKeyNum = $keynum;
        
        $dbdef->updateTableDef($tableid);
        $this->assertEquals($dbdef->stat(), 0);
    }
    
    private function insertData($db)
    {
        $tb = $db->openTable("user", Transactd::TD_OPEN_NORMAL);
        $tb3 = $db->openTable("extention", Transactd::TD_OPEN_NORMAL);
        
        try {
            $db->beginTrn();
            $tb->clearBuffer();
            for ($i= 1;$i<= 1000;++$i) {
                $tb->setFV(0,  $i);
                $tb->setFV(1, $i." user");
                $tb->setFV(2, (($i-1) % 5)+1);
                $tb->insert();
            }
            
            $tb3->clearBuffer();
            for ($i= 1;$i<= 1000;++$i) {
                $tb3->setFV(0,  $i);
                $tb3->setFV(1, $i." comment");
                $tb3->insert();
            }
            $db->endTrn();
        } catch (Exception $e) {
            $db->abortTrn();
            $this->assertEquals(true, false);
        }
        $tb->close();
        $tb3->close();
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
    
    public function test()
    {
        $db = new Database();
        
        $this->createDatabase($db);
        $this->openDatabase($db);
        $this->createUserTable($db);
        $this->createUserExtTable($db);
        $this->insertData($db);
        $mysql_5_5 = $this->isMySQL5_5($db);
        
        $db->setAutoSchemaUseNullkey(true);
        $this->assertEquals($db->autoSchemaUseNullkey(), true);
        $db->setAutoSchemaUseNullkey(false);
        $this->assertEquals($db->autoSchemaUseNullkey(), false);
        
        $this->assertEquals($db::compatibleMode(), Database::CMP_MODE_MYSQL_NULL);
        
        Database::setCompatibleMode(Database::CMP_MODE_OLD_NULL);
        $this->assertEquals(Database::compatibleMode(), Database::CMP_MODE_OLD_NULL);

        Database::setCompatibleMode(Database::CMP_MODE_BINFD_DEFAULT_STR);
        $this->assertEquals(Database::compatibleMode(), Database::CMP_MODE_BINFD_DEFAULT_STR);
        
        Database::setCompatibleMode(Database::CMP_MODE_MYSQL_NULL);
        $this->assertEquals(Database::compatibleMode(), Database::CMP_MODE_MYSQL_NULL);

        $dbdef = $db->dbDef();
        $td = $dbdef->tableDefs(1);
        //isMysqlNullMode //size()
        $this->assertEquals($td->isMysqlNullMode(), true);
        
        //recordlen()
        $len = 145;
        if ($mysql_5_5) {
            $len += 4;
        }
        if ($this->isLegacyTimeFormat($db)) {
            $len -= 3;
        }
        $this->assertEquals($td->recordlen(), $len);
        
        //size()
        $this->assertEquals($td->size(), 1184);
        
        //InUse
        $this->assertEquals($td->inUse(), 0);
        
        //nullfields
        $this->assertEquals($td->nullfields(), 2);
        
        //fieldNumByName
        $this->assertEquals($td->fieldNumByName("tel"), 3);
        
        //default value
        $fd = $td->fieldDef(1);
        $this->assertEquals($fd->defaultValue(), "John");
        $fd = $td->fieldDef(2);
        $this->assertEquals($fd->defaultValue(), 10);
        $fd = $td->fieldDef(3);
        $this->assertEquals($fd->isDefaultNull(), true);
        $fd = $td->fieldDef(4);
        $this->assertEquals($fd->defaultValue(), Transactd::DFV_TIMESTAMP_DEFAULT);
        $this->assertEquals($fd->isTimeStampOnUpdate(), true);
        $fd = $td->fieldDef(5);
        $this->assertEquals($fd->isTimeStampOnUpdate(), false);
        
        $fd = $td->fieldDef(1);
        // synchronizeSeverSchema
        $len = $fd->len;
        
        $fd->setLenByCharnum(19);
        $this->assertNotEquals($len, $fd->len);
        $dbdef->synchronizeSeverSchema(1);
        $td = $dbdef->tableDefs(1);
        $fd = $td->fieldDef(1);
        $this->assertEquals($len, $fd->len);
        
        // syncronize default value
        $fd = $td->fieldDef(1);
        $this->assertEquals($fd->defaultValue(), "John");
        $fd = $td->fieldDef(2);
        $this->assertEquals($fd->defaultValue(), 10);
        $fd = $td->fieldDef(3);
        $this->assertEquals($fd->isDefaultNull(), true);
        $fd = $td->fieldDef(4);
        $this->assertEquals($fd->isTimeStampOnUpdate(), true);
        $fd = $td->fieldDef(5);
        $this->assertEquals($fd->isTimeStampOnUpdate(), false);
        
        // nullable
        $fd = $td->fieldDef(3);
        $this->assertEquals($fd->isNullable(), true);
        
        // getSqlStringForCreateTable
        $sql = $db->getSqlStringForCreateTable("extention");
        $this->assertEquals($db->stat(), 0);
        $this->assertEquals($sql, 'CREATE TABLE `extention` (`id` INT NOT NULL ,`comment` VARCHAR(60) binary NULL DEFAULT NULL,`bits` BIGINT NOT NULL , PRIMARY KEY(`id`)) ENGINE=InnoDB default charset=utf8');
        
        // setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
        $td = $dbdef->tableDefs(1);
        $td->setValidationTarget(true, 0);
        
        
        $q = new Query();
        $atu = new ActiveTable($db, "user", Transactd::TD_OPEN_NORMAL);
        
        // segmentsSizeForInValue
        $this->assertEquals($q->segmentsForInValue(3)->getJoinKeySize(), 3);
        $q->reset();
        $this->assertEquals($q->getJoinKeySize(), 0);
        
        // keyValue null
        $tb1 = $atu->table();
        $tb1->setFV(2, null);
        $this->assertEquals($tb1->getFVNull(2), true);
        $tb1->setFVNull(2, false);
        $this->assertEquals($tb1->getFVNull(2), false);
        $atu->index(1)->keyValue(null);
        $this->assertEquals($tb1->getFVNull(2), true);
        
        // isNull setNull
        $atu->alias("名前", "name");

        $q->select("id", "name", "group", "tel")->where("id", "<=", 10);
        $rs = $atu->index(0)->keyValue(1)->read($q);
        $rs->fetchMode = Transactd::FETCH_RECORD_INTO;
        $this->assertEquals($rs->count(), 10);
        $rec = $rs->first();
        $this->assertEquals($rec[3]->isNull(), true);
        $rec[3]->setNull(false);
        $this->assertEquals($rec[3]->isNull(), false);

        //Join null
        $q->reset();
        $ate = new ActiveTable($db, "extention");
        $last = $ate->index(0)->join($rs, $q->select("comment")
            ->optimize(Query::joinHasOneOrHasMany), "id")->reverse()->first();
        $this->assertEquals($rs->count(), 10);
        $this->assertEquals($last["id"]->i(), 10);
        $this->assertEquals($last["id"]->i64(), 10);
        $this->assertEquals($last["id"]->d(), 10);
        $this->assertEquals($rec[4]->isNull(), false);
        $rec[4]->setNull(true);
        
        //setValue null
        $this->assertEquals($rec[4]->isNull(), true);
        $rec[4]->setNull(false);
        $this->assertEquals($rec[4]->isNull(), false);
        $rec[4]->setValue(null);
        $this->assertEquals($rec[4]->isNull(), true);
        
        //WritableRecord.clear()
        $wr = $atu->getWritableRecord();
        $wr->clear();
        $wr["id"]->setValue(5);
        $wr["tel"]->setValue("0236-99-9999");
        $wr->update();
        
        $wr->clear();
        $wr["id"]->setValue(5);
        $this->assertEquals($wr->read(), true);
        $this->assertEquals($wr["tel"]->str(), "0236-99-9999");
        
        //whereIsNull
        $q->reset();
        $q->select("id", "tel")->whereIsNull("tel")->reject(0xFFFF);
        $rs = $atu->index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 999);
        
        //whereIsNotNull
        $q->reset();
        $q->select("id", "tel")->whereIsNotNull("tel")->reject(0xFFFF);
        $rs = $atu->Index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 1);
        
        //AndIsNull
        $q->reset();
        $q->Select("id", "tel")->where("id", "<=", 10)->andIsNull("tel")->reject(0xFFFF);
        $rs = $atu->Index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 9);
        
        //AndIsNotNull
        $q->reset();
        $q->Select("id", "tel")->where("id", "<", 10)->andIsNotNull("tel")->reject(0xFFFF);
        $rs = $atu->Index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 1);
        
        //OrIsNull
        $q->reset();
        $q->Select("id", "tel")->where("id", "<=", 10)->orIsNull("tel")->reject(0xFFFF);
        $rs = $atu->Index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 1000);
        
        //OrIsNotNull
        $q->reset();
        $q->Select("id", "tel")->where("id", "<=", 10)->orIsNotNull("tel")->reject(0xFFFF);
        $rs = $atu->Index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 10);
        
        //test recordset query
        $q->reset();
        $q->select("id", "name", "group", "tel");
        $rs = $atu->index(0)->keyValue(0)->read($q);
        $this->assertEquals($rs->count(), 1000);
        
        // recordset whenIsNull
        $rq = new RecordsetQuery();
        $rq->whenIsNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 999);
        
        //recordset whenIsNotNull
        $rq->reset();
        $rq->whenIsNotNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 1);
        
        //recordset andIsNull
        $rq->reset();
        $rq->when("id", "<=", 10)->andIsNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 9);
        
        //recordset andIsNotNull
        $rq->reset();
        $rq->when("id", "<", 10)->andIsNotNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 1);
        
        // recordset orIsNull
        $rq->reset();
        $rq->when("id", "<=", 10)->orIsNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 1000);
        
        //recordset orIsNotNull
        $rq->reset();
        $rq->when("id", "<=", 10)->orIsNotNull("tel");
        $rs2 = clone $rs;
        $rs2 = $rs2->matchBy($rq);
        $this->assertEquals($rs2->count(), 10);
        
        //setBin bin
        $bin = "4321";
        $bin[0] = chr(0xFF);
        $bin[1] = chr(0x01);
        $bin[2] = chr(0xFF);
        $bin[3] = chr(0x02);
        $wr["tel"]->setBin($bin);
        $ret = $wr["tel"]->bin();
        $this->assertEquals(ord($ret[0]), 0xFF, "SetBin Bin");
        $this->assertEquals(ord($ret[1]), 0x01, "SetBin Bin");
        $this->assertEquals(ord($ret[2]), 0xFF, "SetBin Bin");
        $this->assertEquals(ord($ret[3]), 0x02, "SetBin Bin");
        
        // table::default NULL
        $tb = $db->openTable("user");
        $this->assertEquals($db->stat(), 0);
        $tb->setKeyNum(0);
        $tb->clearBuffer();
        $this->assertEquals($tb->getFVNull(3), true);
        
        $tb->clearBuffer(Table::clearNull);
        $this->assertEquals($tb->getFVNull(3), false);
        
        // table NULL
        $tb->setFv("id", 1);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $this->assertEquals($tb->getFVNull(3), true);
        $this->assertEquals($tb->getFVNull("tel"), true);
        $tb->setFVNull(3, false);
        $this->assertEquals($tb->getFVNull(3), false);
        
        //setFV null
        $tb->setFVNull("tel", true);
        $this->assertEquals($tb->getFVNull("tel"), true);
        $tb->setFVNull("tel", false);
        $this->assertEquals($tb->getFVNull("tel"), false);
        $tb->setFV("tel", null);
        $this->assertEquals($tb->getFVNull("tel"), true);
        
        //timestamp format
        $date  = Transactd::btrdtoa(Transactd::getNowDate(), true);
        $this->assertEquals(mb_substr($tb->getFVstr("update_datetime"), 0, 10), $date);
        if ($mysql_5_5 == false) {
            $this->assertEquals(mb_substr($tb->getFVstr("create_datetime"), 0, 10), $date);
        }
        
        // setTimestampMode
        $tb->setTimestampMode(Transactd::TIMESTAMP_VALUE_CONTROL);
        $tb->setTimestampMode(Transactd::TIMESTAMP_ALWAYS);
        
        //isMysqlNullMode
        $this->assertEquals($tb->tableDef()->isMysqlNullMode(), true);
        $this->assertEquals($td->inUse(), 2);
        
        unset($atu);
        $this->assertEquals($td->inUse(), 1);
        $tb->release();
        $this->assertEquals($td->inUse(), 0);
        
        $db->close();
    }
    
    public function testBit()
    {
        $db = new Database();
        $this->openDatabase($db);
        $tb = $db->openTable("extention", Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $tb->setKeyNum(0);
        $tb->setFV('id', 1);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $bits = new  Bitset();
        /*
        $bits->set(63, true);
        $bits->set(2, true);
        $bits->set(5, true);
        */
        $bits[63] = true;
        $bits[2] =  true;
        $bits[5] =  true;
        
        $tb->setFV('bits', $bits);
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        
        $q = new Query();
        $at = new ActiveTable($db, "extention", Transactd::TD_OPEN_NORMAL);
        $q->where('id', '=', 1);
        $rs = $at->index(0)->keyValue(1)->read($q);
        $rs->fetchMode = Transactd::FETCH_RECORD_INTO;
        $this->assertEquals($rs->size(), 1);
        $bits = $rs[0]['bits']->getBits();

        $this->assertEquals($bits->get(63), true);
        $this->assertEquals($bits->get(2), true);
        $this->assertEquals($bits->get(5), true);
        $this->assertEquals($bits->get(62), false);
        $this->assertEquals($bits->get(0), false);
        $this->assertEquals($bits->get(12), false);
        
        $this->assertEquals($bits[63], true);
        $this->assertEquals($bits[2], true);
        $this->assertEquals($bits[5], true);
        $this->assertEquals($bits[62], false);
        $this->assertEquals($bits[0], false);
        $this->assertEquals($bits[12], false);
    
        $wr = $at->getWritableRecord();
        $wr['id'] = 1;
        /*
        $bits->set(63, false);
        $bits->set(12, true);
        $bits->set(0, true);
        $bits->set(62, true);
        */
        $bits[63] = false;
        $bits[12] = true;
        $bits[0] =  true;
        $bits[62] =  true;

        $wr['bits'] = $bits;
        $wr->update();
        $tb->setFV('id', 1);
        $tb->seek();
        $this->assertEquals($tb->stat(), 0);
        $bits = $tb->getFVbits('bits');
        
        $this->assertEquals($bits->get(63), false);
        $this->assertEquals($bits->get(2), true);
        $this->assertEquals($bits->get(5), true);
        $this->assertEquals($bits->get(12), true);
        $this->assertEquals($bits->get(0), true);
        $this->assertEquals($bits->get(62), true);
        $this->assertEquals($bits->get(11), false);
        $this->assertEquals($bits->get(13), false);
        
        $this->assertEquals($bits[63], false);
        $this->assertEquals($bits[2], true);
        $this->assertEquals($bits[5], true);
        $this->assertEquals($bits[12], true);
        $this->assertEquals($bits[0], true);
        $this->assertEquals($bits[62], true);
        $this->assertEquals($bits[11], false);
        $this->assertEquals($bits[13], false);
        
        $tb->close();
        $db->close();
    }
    
    public function testBitset()
    {
        $bits1 = new Bitset();
        $bits2 = new Bitset();
        $bits1[0] = true;
        $bits1[1] = true;
        $bits1[63] = true;
        
        $bits2[0] = true;
        $bits2[1] = false;
        $bits2[63] = true;
        
        $this->assertEquals($bits1->equals($bits2), false);
        $this->assertEquals($bits1->contains($bits2), true);
        $this->assertEquals($bits2->contains($bits1), false);
        
        $all = false;
        $this->assertEquals($bits2->contains($bits1, $all), true);
    }
    
    public function testDecimal()
    {
        $db = new Database();
        $this->openDatabase($db);
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, null);
        $td = new Tabledef();
        
        $td->schemaCodePage = Transactd::CP_UTF8;
        $td->setTableName("decimal");
        $td->setFileName("decimal");
        $td->charsetIndex = Transactd::CHARSET_UTF8;
        $tableid = 10;
        $td->id = $tableid;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fieldIndex = 0;
        $fd = $dbdef->insertField($tableid, $fieldIndex);
        $fd->setName('id');
        $fd->type = Transactd::ft_mydecimal;
        $fd->setDecimalDigits(65, 30);
        $this->assertEquals($fd->digits, 65);
        $this->assertEquals($fd->decimals, 30);
        $this->assertEquals($fd->isIntegerType(), false);
        $this->assertEquals($fd->isNumericType(), true);
        
        $bits1 = new  Bitset();
        $bits1[2] = true;
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $fd->setDefaultValue($bits1);
        $this->assertEquals($fd->defaultValue(), '4');
        $db->close();
    }
    public function testSnapshot()
    {
        $db = new Database();
        $this->openDatabase($db);
        $bpos = $db->beginSnapshot(Transactd::CONSISTENT_READ_WITH_BINLOG_POS);
        if ($this->isMariaDBWithGtid($db)) {
            $this->assertEquals($bpos->type, Transactd::REPL_POSTYPE_MARIA_GTID);
        } else {
            $ret = ($bpos->type == Transactd::REPL_POSTYPE_POS) || ($bpos->type == Transactd::REPL_POSTYPE_GTID);
            $this->assertEquals($ret, true);
        }
        $this->assertNotEquals($bpos->pos, 0);
        $this->assertNotEquals($bpos->filename, "");
        //echo PHP_EOL.'binlog pos = '.$bpos->filename.':'.$bpos->pos.PHP_EOL;
        //echo 'gtid (set)= '.$bpos->gtid.PHP_EOL;
   
        //setGtid
        $bpos->gtid = "ABCD";
        $this->assertEquals($bpos->gtid, "ABCD");
        
        $db->endSnapshot();
        $db->close();
    }
    public function testGetSql()
    {
        $db = new Database();
        $this->openDatabase($db);
        $db->execSql("create view idlessthan5 as select * from user where id < 5");
        $view = $db->getCreateViewSql("idlessthan5");
        $this->assertEquals((strpos($view, "idlessthan5") !== false), true);
        //echo($view);
        $tb = $db->openTable("user");
        $this->assertEquals($db->stat(), 0);
        $sql = $tb->getCreateSql();
        //echo($sql);
        $this->assertEquals((strpos($sql, "CREATE TABLE") !== false), true);
        $this->assertEquals((strpos($sql, "名前") !== false), true);
        $tb->close();
        $db->close();
    }
    public function testCreateAssociate()
    {
        $db = new Database();
        $this->openDatabase($db);
        $dba = $db->createAssociate();
        $this->assertEquals($db->stat(), 0);
        $this->assertEquals($dba->isAssociate(), true);
        $dba->close();
        $db->close();
    }
    public function testConnMgr()
    {
        // other database connection
        $db_other = new Database();
        $this->openDatabase($db_other);
        $tb_other = $db_other->openTable("user");
        $this->assertEquals($db_other->stat(), 0);
        // connMgr connection
        $db = new Database();
        $mgr = new ConnMgr($db);
        $mgr->connect($db_other->uri());
        $this->assertEquals($mgr->stat(), 0);
        // connections
        $recs = $mgr->connections();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertEquals($recs->size(), 1);
        // InUseDatabases
        $recs = $mgr->inUseDatabases($recs[0]->conId);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertEquals($recs->size(), 1);
        // InUseTables
        $recs = $mgr->inUseTables($recs[0]->conId, $recs[0]->db);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertEquals($recs->size(), 2);
        // tables, views
        $recs = $mgr->tables("test_v3");
        $this->assertEquals($mgr->stat(), 0);
        $recs1 = $mgr->views("test_v3");
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertEquals($recs->size(), 3);
        $this->assertEquals($recs1->size(), count($recs1));
        $this->assertEquals($recs1->size(), 1);
        $this->assertEquals($recs1[0]->name, "idlessthan5");
        // schemaTables
        $recs = $mgr->schemaTables("test_v3");
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertEquals($recs->size(), 1);
        $this->assertEquals($recs[0]->name, "test");
        // databases
        $recs = $mgr->databases();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $size = $recs->size();
        $mgr->RemoveSystemDb($recs);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($recs->size(), count($recs));
        $this->assertNotEquals($recs->size(), $size);
        //sysvar
        $recs = $mgr->sysvars();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals(ConnMgr::sysvarName(0), "database_version");
        //statusvar
        $recs = $mgr->statusvars();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals(ConnMgr::statusvarName(0), "tcp_connections");
        //slaveStatus
        $recs = $mgr->slaveStatus("");
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($mgr->slaveStatusName(0), "Slave_IO_State");
        for ($i = 0; $i < $recs->size(); $i++) {
            //echo(PHP_EOL . $mgr->slaveStatusName($i) . "\t:" . $recs[$i]->value);
        }
        
        //extendedvars
        $recs = $mgr->extendedvars();
        $this->assertEquals($recs->size(), 4);
        $this->assertEquals($mgr->extendedVarName(0), "MySQL_Gtid_Mode");
        
        // record port
        $this->assertEquals($recs[0]->port, 0);
        
        //slaveHosts
        $recs = $mgr->slaveHosts();
        $this->assertEquals($mgr->stat(), 0);
        //channels
        $recs = $mgr->channels();
        $this->assertEquals($mgr->stat(), 0);
        //haLock
        $ret = $mgr->haLock();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($ret, true);
        //haUnlock
        $mgr->haUnlock();
        $this->assertEquals($mgr->stat(), 0);
        //setRole
        $ret = $mgr->setRole(0);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($ret, true);
        $ret = $mgr->setRole(1);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($ret, true);
        //setEnableFailover
        $ret = $mgr->setEnableFailover(false);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($ret, true);
        $ret = $mgr->setEnableFailover(true);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($ret, true);
        $this->assertEquals($mgr->isOpen(), true);
        //enableAutoReconnect
        $this->assertEquals($db->enableAutoReconnect(), false);
        $db->setenableAutoReconnect(true);
        $this->assertEquals($db->enableAutoReconnect(), true);
        $db->setenableAutoReconnect(false);
        $mgr->disconnect();
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($mgr->isOpen(), false);
        //haNameReslover
        $host = "localhost";
        $user = "root";
        $pwd = "";
        $ret = HaNameResolver::start("master123", "slave1, slave2", $host, 0, $user, $pwd);
        $this->assertEquals($ret, 1);
        //portMap
        HaNameResolver::addPortMap(3307, 8611);
        HaNameResolver::clearPortMap();
        //master slave name
        $this->assertEquals(HaNameResolver::master(), $host);
        $this->assertEquals(HaNameResolver::slave(), "-");
        //connect by master roll
        $mgr->connect("tdap://" . $user . "@master123/?pwd=" . $pwd);
        $this->assertEquals($mgr->stat(), 0);
        $this->assertEquals($mgr->isOpen(), true);
        $mgr->disconnect();
        $this->assertEquals($mgr->isOpen(), false);
        //stop
        HaNameResolver::stop();
        $mgr->connect("tdap://" . $user . "@master123/?pwd=" . $pwd);
        $this->assertEquals($mgr->stat(), ERROR_TD_HOSTNAME_NOT_FOUND);
        $tb_other->close();
        $db_other->close();
    }
    
    public function testV35Constant()
    {
        $this->assertEquals(Transactd::ERROR_TD_RECONNECTED_OFFSET, 1000);
        $this->assertEquals(Transactd::ERROR_TD_INVALID_SERVER_ROLE, 3812);
        $this->assertEquals(Transactd::ERROR_TD_RECONNECTED, 3900);
        $this->assertEquals(Transactd::MYSQL_ERROR_OFFSET, 25000);
        $this->assertEquals(Transactd::HA_ROLE_SLAVE, 0);
        $this->assertEquals(Transactd::HA_ROLE_MASTER, 1);
        $this->assertEquals(Transactd::HA_ROLE_NONE, 2);
        $this->assertEquals(Transactd::HA_RESTORE_ROLE, 4);
        $this->assertEquals(Transactd::HA_ENABLE_FAILOVER, 8);
    }
    
    public function testFetchMode()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $this->assertEquals($tb->stat(), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        //test fetch field type
        Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_OBJECT);
        $tb->fetchMode = Transactd::FETCH_RECORD_INTO;
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_RECORD_INTO);
        $rec = $tb->fields();
        $this->assertEquals($rec["id"]->i(), 1);
        
        Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE);
        $this->assertEquals($rec["id"], 1);
        
        $tb->fetchMode = Transactd::FETCH_VAL_NUM;
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_VAL_NUM);
        $rec = $tb->fields();
        $this->assertEquals($rec[0], 1);
        
        $tb->fetchMode = Transactd::FETCH_VAL_ASSOC;
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_VAL_ASSOC);
        $rec = $tb->fields();
        $this->assertEquals($rec["id"], 1);
        
        $tb->fetchMode = Transactd::FETCH_VAL_BOTH;
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_VAL_BOTH);
        $rec = $tb->fields();
        $this->assertEquals($rec[0], 1);
        $this->assertEquals($rec["id"], 1);
        
        
        $tb->fetchMode = Transactd::FETCH_OBJ;
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_OBJ);
        $usr = $tb->fields();
        $this->assertEquals($usr->id, 1);
        
        
        $tb->fetchMode = Transactd::FETCH_USR_CLASS;
        $tb->fetchClass = "User";
        $tb->ctorArgs = array("1","2","3");
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_USR_CLASS);
        $usr = $tb->fields();
        $this->assertEquals($usr->id, 1);
        $this->assertEquals($usr->a, "1");
        $this->assertEquals($usr->b, "2");
        $this->assertEquals($usr->c, "3");
        $tb->close();
        
        $at = new ActiveTable($db, "user");
        $q = new Query();
        $q->where("id", "<", 10);
        $rs = $at->index(0)->keyValue(0)->read($q);
        $rs->fetchMode = Transactd::FETCH_RECORD_INTO;
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_RECORD_INTO);
        Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_OBJECT);
        $this->assertEquals($rs[0]["id"]->i(), 1);
        Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE);
        $this->assertEquals($rs[0]["id"], 1);
        $this->assertEquals($rs->size(), 9);

        $rs->fetchMode = Transactd::FETCH_VAL_NUM;
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_VAL_NUM);
        $this->assertEquals($rs[0][0], 1);
        $this->assertEquals(count($rs), 9);

        $rs->fetchMode = Transactd::FETCH_VAL_ASSOC;
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_VAL_ASSOC);
        $this->assertEquals($rs[0]["id"], 1);
        $this->assertEquals(count($rs), 9);

        $rs->fetchMode = Transactd::FETCH_VAL_BOTH;
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_VAL_BOTH);
        $this->assertEquals($rs[0][0], 1);
        $this->assertEquals($rs[0]["id"], 1);
        $this->assertEquals(count($rs), 9);


        $rs->fetchMode = Transactd::FETCH_OBJ;
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_OBJ);
        $this->assertEquals($rs[0]->id, 1);
        $this->assertEquals(count($rs), 9);

        $rs->fetchMode = Transactd::FETCH_USR_CLASS;
        $rs->fetchClass = "User";
        $rs->ctorArgs = array("1","2","3");
        $this->assertEquals($rs->fetchMode, Transactd::FETCH_USR_CLASS);
        $this->assertEquals($rs[1]->id, 2);
        $this->assertEquals($rs[0]->a, "1");
        $this->assertEquals($rs[0]->b, "2");
        $this->assertEquals($rs[0]->c, "3");
        $this->assertEquals(count($rs), 9);
        
        $db->close();
    }
    
    public function testSetAlias()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $this->assertEquals($tb->stat(), 0);
        $tb->setAlias("名前", "name");
        $tb->setAlias("id", "user_id");
        $this->assertEquals($tb->fieldNumByName("user_id"), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);

        $tb->fetchMode = Transactd::FETCH_USR_CLASS;
        $tb->fetchClass = "User";
        $tb->ctorArgs = array("1","2","3");
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_USR_CLASS);
        $usr = $tb->fields();
        $this->assertEquals($usr->user_id, 1);
        $this->assertEquals($usr->name, "1 user");
        
        $q = new Query();
        $q->select("name")->where("id", "<", 10);
        $tb->setQuery($q);
        $tb->clearBuffer();
        $users = $tb->findAll();
        $this->assertEquals(count($users), 9);
        $this->assertEquals($users[0]->name, "1 user");
    }
    
    public function testCURDByObject()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $this->assertEquals($tb->stat(), 0);
        $tb->setAlias("名前", "name");
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb->fetchMode = Transactd::FETCH_USR_CLASS;
        $tb->fetchClass = "User";
        $tb->ctorArgs = array("1","2","3");
        $this->assertEquals($tb->fetchMode, Transactd::FETCH_USR_CLASS);
        
        //Insert
        $usr = $tb->fields();
        $usr->id = 0;
        $usr->name = 'test_insertObject';
        $this->assertEquals($tb->insertByObject($usr), true);
        $tb->seekLast();
        $usr = $tb->fields();
        $this->assertEquals($usr->name, 'test_insertObject');
        $this->assertEquals($usr->id, 1001);
        
        //Update
        $usr->name = 'test_UpdateObject';
        $this->assertEquals($tb->updateByObject($usr), true);
        $usr->name = '';
        
        //Read
        $this->assertEquals($tb->readByObject($usr), true);
        $this->assertEquals($usr->name, 'test_UpdateObject');
        $this->assertEquals($usr->id, 1001);
        
        $row = $tb->getRecord();
        $row->setValueByObject($usr);
        $usr2 = $tb->fields();
        $this->assertEquals($usr2, $usr);
        
        //Delete
        $this->assertEquals($tb->deleteByObject($usr), true);
        $tb->seekLast();
        $usr = $tb->fields();
        $this->assertEquals($usr->id, 1000); 
    }
    
    public function testUTCC()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $tb2 = $db->openTable("user");
        // test in changeCurrentCc or changeCurrentNcc

        $db->beginTrn();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->setFV("名前", 'John');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb2->setFV("名前", 'mike');
        $tb2->setUpdateConflictCheck(true);
        $this->assertEquals($tb2->updateConflictCheck(), true);
        $tb2->update(Nstable::changeCurrentCc);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_CHANGE_CONFLICT);
        $db->abortTrn();

        $db->beginTrn();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->setFV("名前", 'John');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb2->setFV("名前", 'mike');
        $tb2->setUpdateConflictCheck(false);
        $this->assertEquals($tb2->updateConflictCheck(), false);
        $tb2->update(Nstable::changeCurrentCc);
        $this->assertEquals($tb2->stat(), 0);
        $db->abortTrn();
        
        $db->beginTrn();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->setFV("名前", 'John');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb2->setFV("名前", 'mike');
        // test in changeInKey
        $tb2->setUpdateConflictCheck(true);
        $tb2->update(Nstable::changeInKey);
        $this->assertEquals($tb2->stat(), Transactd::STATUS_CHANGE_CONFLICT);
        $db->abortTrn();

        $db->beginTrn();
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        $tb2->seekFirst();
        $this->assertEquals($tb2->stat(), 0);
        $tb->setFV("名前", 'John');
        $tb->update();
        $this->assertEquals($tb->stat(), 0);
        $tb2->setFV("名前", 'mike');
        $tb2->setUpdateConflictCheck(false);
        $tb2->update(Nstable::changeInKey);
        $this->assertEquals($tb2->stat(), 0);
        $db->abortTrn();
    }
    
    const PROP_TEST_HAS = 1;
    const PROP_TEST_READ= 2;
    const PROP_TEST_WRITELONG= 3;

    public function testPropertyAccess()
    {
         /* To orm user
         To orm user,
         If you nothing to declared the private variable for a property, 
         and defined the __set and the  __get magic method, 
         The extension is used __set magic method in read from database.
         But the extension is not used __get magic method in write to database.
         You need prepared variable of field name for write operation.
         Even if you declrare both the private variable and __set methods, 
         the extension read or write private variable directly.  
         */
         $p = new PropertyTest();
         $this->assertEquals(test_property($p, 'private', self::PROP_TEST_HAS), true);
         $this->assertEquals(test_property($p, 'protected', self::PROP_TEST_HAS), true);
         $this->assertEquals(test_property($p, 'public', self::PROP_TEST_HAS), true);
         
         // Whne read data from object, use hasProperty function first. 
         // If hasProperty returns false, skip read this field.
         $this->assertEquals(test_property($p, 'magic', self::PROP_TEST_HAS), false);// Important
         
         $this->assertEquals(test_property($p, 'private', self::PROP_TEST_READ), 1);
         $this->assertEquals(test_property($p, 'protected', self::PROP_TEST_READ), 2);
         $this->assertEquals(test_property($p, 'public', self::PROP_TEST_READ), 3); 
         $this->assertEquals(test_property($p, 'magic', self::PROP_TEST_READ), 4);
         
         test_property($p, 'private', self::PROP_TEST_WRITELONG, 2);
         test_property($p, 'protected', self::PROP_TEST_WRITELONG, 3);
         test_property($p, 'public', self::PROP_TEST_WRITELONG, 4);
         test_property($p, 'magic', self::PROP_TEST_WRITELONG, 5);
         
         $this->assertEquals(test_property($p, 'private', self::PROP_TEST_READ), 2);
         $this->assertEquals(test_property($p, 'protected', self::PROP_TEST_READ), 3);
         $this->assertEquals(test_property($p, 'public', self::PROP_TEST_READ), 4);
         $this->assertEquals(test_property($p, 'magic', self::PROP_TEST_READ), 5);
         
         // write private variable directly. 
         test_property($p, 'duplicate', self::PROP_TEST_WRITELONG, 8);
         $this->assertEquals(test_property($p, 'duplicate', self::PROP_TEST_READ), 8);
    }
        
    public function testTransfer()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $this->assertEquals($tb->stat(), 0);
        $tb->seekFirst();
        $this->assertEquals($tb->stat(), 0);
        
        $tb->setAlias("名前", "name");
        $usr = new UserT();
        $usr->id = 5;
        $this->assertEquals($tb->readByObject($usr), true);
        $this->assertEquals($usr->id, 5);
        $this->assertEquals($usr->phone->number->tel, '0236-99-9999');
        UserT::setTransferMap(1);
        $usr->id = 5;
        $this->assertEquals($tb->readByObject($usr), true);// No error
        $this->assertEquals($usr->id, 5);
        $this->assertEquals($usr->phone2, null);
        
        UserT::setTransferMap(2);
        $usr->id = 5;
        $this->assertEquals($tb->readByObject($usr), true);// No error
        $this->assertEquals($usr->id, 5);
        $this->assertEquals($usr->phone->number2, null);
    }
    
    public function testByObjectOption()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $tb = $db->openTable("user");
        $this->assertEquals($tb->stat(), 0); 
        $tb->setAlias("名前", "name");
        $usr = new UserT();
        
        // read option (key number)
        $tb->setKeyNum(0);
        $usr->id = 5;
        $this->assertEquals($tb->readByObject($usr, 2), false);
        $this->assertEquals($tb->keyNum(), 2); 

        $this->assertEquals($tb->readByObject($usr), true);
        $this->assertEquals($tb->keyNum(), 0);
        
        //update option (eUpdateType)
        $this->assertEquals($tb->readByObject($usr, 2), false); // No currency
        $this->assertEquals($tb->updateByObject($usr, Nstable::changeCurrentCc), false);
        $this->assertEquals($tb->updateByObject($usr), true);  //default value  Nstable::changeInKey
        
        //delete option
        $usr->id = 0;
        $this->assertEquals($tb->insertByObject($usr), true); //Insert id = 1001
        $tb->fetchMode = Transactd::FETCH_USR_CLASS;
        $tb->fetchClass = "UserT";
        $tb->seekLast();
        $this->assertEquals($tb->stat(), 0); 
        $usr = $tb->getRow();
        $id = $usr->id;
        $usr->id = 1005;
        $this->assertEquals($tb->readByObject($usr), false); // No currency
        $usr->id = $id;
        $tb->setKeyNum(2);
        $this->assertEquals($tb->deleteByObject($usr, false/*inKey*/), false);
        $this->assertEquals($tb->deleteByObject($usr), true);  //default value true
    }
    
    public function testRecordsetJoin()
    {
        $db = new Database();
        $db->open(URI, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL);
        $this->assertEquals($db->stat(), 0);
        $at = new ActiveTable($db, "user", Transactd::TD_OPEN_READONLY);
        $ate = new ActiveTable($db, "extention", Transactd::TD_OPEN_READONLY);
        $q = new Query;
        $q->where('id','>=', 1)->and_('id','<=',10);
        $rs = $at->index(0)->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 10);
        $q->reset()->where('id','>=', 1)->and_('id','<=',5);
        $rse = $ate->index(0)->keyValue(1)->read($q);
        $this->assertEquals($rse->size(), 5);
        $rs1 = clone($rs);
        $rq = new RecordsetQuery;
        $rq->when('id', '=', 'id');
        $rs1->join($rse, $rq);
        $this->assertEquals($rs1->size(), 5);
        $rs->outerJoin($rse, $rq);
        $this->assertEquals($rs->size(), 10);
        
        // Recordset::appendField
        $n = $rs->fieldDefs()->size();
        $fd = new Fielddef;
        $fd->type = Transactd::ft_integer;
        $fd->len = 4;
        $fd->name = 'abc';
        $rs->appendField($fd);
        $this->assertEquals($n + 1, $rs->fieldDefs()->size());
        $rs->appendField($fd->name, $fd->type, $fd->len);
        $this->assertEquals($n + 2, $rs->fieldDefs()->size());
    }
}
