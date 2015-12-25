<?php
/* ================================================================
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
================================================================ */
mb_internal_encoding('UTF-8');

require_once("transactd.php");
use BizStation\Transactd as bz;

function getHost()
{
    $host = getenv('TRANSACTD_PHPUNIT_HOST');
    if (strlen($host) == 0)
    {
        $host = '127.0.0.1/';
    }
    if ($host[strlen($host) - 1] != '/')
    {
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
define("DBNAME", "testv3");
define("TABLENAME", "user");
define("PROTOCOL", "tdap://");
define("BDFNAME", "?dbfile=test.bdf");
define("URL", PROTOCOL . USERPART . HOSTNAME . DBNAME . BDFNAME . PASSPART);

// multi thread test if `php_pthreads` exists.
if(class_exists('Thread')){
    class SeekLessThanWorker extends Thread
    {
        public function __construct()
        {
            $this->value = -1;
        }
        public function run()
        {
            $dbm = new bz\pooledDbManager(new bz\connectParams(URL));
            $tb = $dbm->table('user');
            $tb->setFV(FDI_ID, 300000);
            $tb->seekLessThan(false, bz\transactd::ROW_LOCK_X);
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
        if ($db->stat() == bz\transactd::STATUS_TABLE_EXISTS_ERROR)
        {
            $this->dropDatabase($db);
            $db->create(URL);
        }
        $this->assertEquals($db->stat(), 0);
    }
    private function openDatabase($db)
    {
        return $db->open(URL, bz\transactd::TYPE_SCHEMA_BDF, bz\transactd::TD_OPEN_NORMAL);
    }
    private function isMySQL5_5($db)
    {
        $vv = new bz\btrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) && 
            ((5 == $server_ver->majorVersion) &&
            (5 == $server_ver->minorVersion));
    }
    private function isLegacyTimeFormat($db)
    {
        $vv = new bz\btrVersions();
        $db->getBtrVersion($vv);
        $server_ver = $vv->version(1);
        return ($db->stat() == 0) && 
            ((5 == $server_ver->majorVersion) &&
            (5 == $server_ver->minorVersion)) &&
            ($server_ver->type == bz\transactd::MYSQL_TYPE_MYSQL);
    }
    private function createUserTable($db)
    {
        
        $dbdef = $db->dbDef();
        $this->assertNotEquals($dbdef, NULL);
        $td = new bz\tabledef();
        $td->schemaCodePage = bz\transactd::CP_UTF8;
        $td->setTableName(TABLENAME);
        $td->setFileName(TABLENAME . '.dat');
        $td->charsetIndex = bz\transactd::CHARSET_UTF8;
        $tableid = 1;
        $td->id = $tableid;
        $td->pageSize = 2048;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fieldIndex = 0;
        $fd = $dbdef->insertField($tableid, $fieldIndex);
        $fd->setName('id');
        $fd->type = bz\transactd::ft_autoinc;
        $fd->len = 4;
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('名前');
        $fd->type = bz\transactd::ft_myvarchar;
        $fd->len = 2;
        $this->assertEquals($fd->isValidCharNum(), false);
        $fd->SetLenByCharnum(20);
        $this->assertEquals($fd->isValidCharNum(), true);
        $fd->setDefaultValue("John");
        $this->assertEquals($fd->isPadCharType(), false);
        $this->assertEquals($fd->isDateTimeType(), false);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('group');
        $fd->type = bz\transactd::ft_integer;
        $fd->len = 4;
        $fd->setNullable(true, false);
        $fd->setDefaultValue(10);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('tel');
        $fd->type = bz\transactd::ft_myvarchar;
        $fd->len = 4;
        $fd->setLenByCharnum(21);
        $fd->setNullable(true);
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('update_datetime');
        $fd->type = bz\transactd::ft_mytimestamp;
        $fd->len = 7;
        $fd->setDefaultValue(bz\transactd::DFV_TIMESTAMP_DEFAULT);
        $fd->setTimeStampOnUpdate(true);
        $this->assertEquals($fd->isTimeStampOnUpdate(), true);

        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('create_datetime');
        if ($this->isMySQL5_5($db))
        {
            $fd->type = bz\transactd::ft_mydatetime;
            $fd->len = 8;
        }
        else
        {
            $fd->type = bz\transactd::ft_mytimestamp;
            $fd->len = 4;
            $fd->setDefaultValue(bz\transactd::DFV_TIMESTAMP_DEFAULT);
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
        $this->assertNotEquals($dbdef, NULL);
        $td = new bz\tabledef();
        
        $td->schemaCodePage = bz\transactd::CP_UTF8;
        $td->setTableName("extention");
        $td->setFileName("extention");
        $td->charsetIndex = bz\transactd::CHARSET_UTF8;
        $tableid = 3;
        $td->id = $tableid;
        $dbdef->insertTable($td);
        $this->assertEquals($dbdef->stat(), 0);
        
        $fieldIndex = 0;
        $fd = $dbdef->insertField($tableid, $fieldIndex);
        $fd->setName('id');
        $fd->type = bz\transactd::ft_integer;
        $fd->len = 4;
        
        $fd = $dbdef->insertField($tableid, ++$fieldIndex);
        $fd->setName('comment');
        $fd->type = bz\transactd::ft_myvarchar;
        $fd->SetLenByCharnum(60);
        $fd->setNullable(true);
        $this->assertEquals($fd->isDefaultNull(), true);
        
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
        $tb = $db->openTable("user", bz\transactd::TD_OPEN_NORMAL); 
        $tb3 = $db->openTable("extention", bz\transactd::TD_OPEN_NORMAL); 
        
        try
        {
            $db->beginTrn();
            $tb->clearBuffer();
            for ($i= 1;$i<= 1000;++$i)
            {
                $tb->setFV(0,  $i);
                $tb->setFV(1, $i." user");
                $tb->setFV(2, (($i-1) % 5)+1);
                $tb->insert();
            }
            
            $tb3->clearBuffer();
            for ($i= 1;$i<= 1000;++$i)
            {
                $tb3->setFV(0,  $i);
                $tb3->setFV(1, $i." comment");
                $tb3->insert();
            }
            $db->endTrn();
        }
        catch(Exception $e)
        {
            $db->abortTrn();
            $this->assertEquals(true, false);
        }
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
        $db = new bz\database();

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
        
        $this->assertEquals($db::compatibleMode(), bz\database::CMP_MODE_MYSQL_NULL);
        
        bz\database::setCompatibleMode(bz\database::CMP_MODE_OLD_NULL);
        $this->assertEquals(bz\database::compatibleMode(), bz\database::CMP_MODE_OLD_NULL);
        
        bz\database::setCompatibleMode(bz\database::CMP_MODE_MYSQL_NULL);
        $this->assertEquals(bz\database::compatibleMode(), bz\database::CMP_MODE_MYSQL_NULL);
        
        $dbdef = $db->dbDef();
        $td = $dbdef->tableDefs(1);
        //isMysqlNullMode //size()
        $this->assertEquals($td->isMysqlNullMode() , true);
        
        //recordlen()
        $len = 145;
        if ($mysql_5_5)  $len += 4;
        if ($this->isLegacyTimeFormat($db)) $len -= 3;
        $this->assertEquals($td->recordlen() , $len);
        
        //size()
        $this->assertEquals($td->size() , 1184);

        //InUse
        $this->assertEquals($td->inUse() , 0);
        
        //nullfields
        $this->assertEquals($td->nullfields(), 2);
            
        
       
        //fieldNumByName
        $this->assertEquals($td->fieldNumByName("tel") , 3);
        
        //default value
        $fd = $td->fieldDef(1);
        $this->assertEquals($fd->defaultValue(), "John");
        $fd = $td->fieldDef(2);
        $this->assertEquals($fd->defaultValue(), 10);
        $fd = $td->fieldDef(3);
        $this->assertEquals($fd->isDefaultNull(), true);
        $fd = $td->fieldDef(4);
        $this->assertEquals($fd->defaultValue(), bz\transactd::DFV_TIMESTAMP_DEFAULT);
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
        $this->assertEquals($sql, 'CREATE TABLE `extention` (`id` INT NOT NULL ,`comment` VARCHAR(60) binary NULL DEFAULT NULL, UNIQUE key0(`id`)) ENGINE=InnoDB default charset=utf8');
        
        // setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
        $td = $dbdef->tableDefs(1);
        $td->setValidationTarget(true, 0);
        
        
        $q = new bz\query();
        $atu = new bz\activeTable($db, "user");
        
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
        return;
        $this->assertEquals($rs->count(), 10);
        $rec = $rs->first();
        $this->assertEquals($rec[3]->isNull(), true);
        $rec[3]->setNull(false);
        $this->assertEquals($rec[3]->isNull(), false);

        //Join null
        $q->reset();
        $ate = new bz\activeTable($db, "extention");
        $last = $ate->index(0)->join($rs, $q->select("comment")
            ->optimize(bz\query::joinHasOneOrHasMany), "id")->reverse()->first();
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
        $rq = new bz\recordsetQuery();
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
        
        $tb->clearBuffer(bz\table::clearNull);
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
        $date  = bz\transactd::btrdtoa(bz\transactd::getNowDate(), true);
        $this->assertEquals(mb_substr($tb->getFVstr("update_datetime"), 0, 10), $date);
        if ($mysql_5_5 == false)
             $this->assertEquals(mb_substr($tb->getFVstr("create_datetime"), 0, 10), $date);
        
        // setTimestampMode
        $tb->setTimestampMode(bz\transactd::TIMESTAMP_VALUE_CONTROL);
        $tb->setTimestampMode(bz\transactd::TIMESTAMP_ALWAYS);
        
        //isMysqlNullMode
        $this->assertEquals($tb->tableDef()->isMysqlNullMode(), true);
        $this->assertEquals($td->inUse() , 2);
        
        unset($atu);
        $this->assertEquals($td->inUse() , 1);
        $tb->release();
        $this->assertEquals($td->inUse() , 0);
        
    }
}
