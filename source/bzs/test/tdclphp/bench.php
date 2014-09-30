<?php
/* ================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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

define('URL', 'tdap://localhost/test_bench?dbfile=test.bdf');
define('TABLENAME', 'users');
define('FDI_ID', 0);
define('FDI_NAME', 1);
define('BULKBUFSIZE', 65535 - 1000);

define('RECORD_COUNT', 20000);
define('RECORD_UNIT', 20);

function assertEquals($a, $b, $msg = '') {
    if ($a !== $b) {
        echo("$a !== $b $msg");
        return false;
    }
    return true;
}
function assertNotEquals($a, $b, $msg = '') {
    if ($a === $b) {
        echo("$a === $b $msg");
        return false;
    }
    return true;
}

function create($url) {
    // create database
    $db = new Bz\database();
    $db->create($url);
    if ($db->stat() == Bz\transactd::STATUS_TABLE_EXISTS_ERROR) {
        $db->open($url);
        assertEquals($db->stat(), 0);
        $db->drop();
        assertEquals($db->stat(), 0);
        $db->create($url);
    }
    assertEquals($db->stat(), 0);
    $db->open($url, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
    assertEquals($db->stat(), 0);
    // create table
    $dbdef = $db->dbDef();
    assertNotEquals($dbdef, NULL);
    $td = new Bz\tabledef();
    // Set table schema codepage to UTF-8
    //   - codepage for field NAME and tableNAME
    $td->schemaCodePage = Bz\transactd::CP_UTF8;
    $td->setTableName(TABLENAME);
    $td->setFileName(TABLENAME . '.dat');
    // Set table default charaset index
    //    - default charset for field VALUE
    $td->charsetIndex = Bz\transactd::charsetIndex(Bz\transactd::CP_UTF8);
    $td->id = 1;
    $td->pageSize = 2048;
    $dbdef->insertTable($td);
    assertEquals($dbdef->stat(), 0);
    // id
    $fd = $dbdef->insertField($td->id, FDI_ID);
    $fd->setName('id');
    $fd->type = Bz\transactd::ft_autoinc;
    $fd->len = 4;
    $dbdef->updateTableDef($td->id);
    assertEquals($dbdef->stat(), 0);
    // user_id
    $fd = $dbdef->insertField($td->id, FDI_NAME);
    $fd->setName('name');
    $fd->type = Bz\transactd::ft_myvarchar;
    $fd->len = 100;
    $dbdef->updateTableDef($td->id);
    assertEquals($dbdef->stat(), 0);
    // key
    $kd = $dbdef->insertKey($td->id, 0);
    $kd->segment(0)->fieldNum = 0;
    $kd->segment(0)->flags->bit8 = 1;
    $kd->segment(0)->flags->bit1 = 1;
    $kd->segmentCount = 1;
    $td->primaryKeyNum = 0;
    $dbdef->updateTableDef($td->id);
    assertEquals($dbdef->stat(), 0);
    // test open
    $tb = $db->openTable(TABLENAME);
    assertEquals($db->stat(), 0);
}

function deleteAll($db, $tb) {
    $db->beginTrn();
    $tb->clearBuffer();
    for ($i = 1; $i <= RECORD_COUNT; $i++) {
        $tb->setFV(FDI_ID, $i);
        $tb->seek();
        if ($tb->stat() === 0) {
            $tb->del();
            if (!assertEquals($tb->stat(), 0, 'deleteAll')) {
                $db->endTrn();
                return false;
            }
        }
    }
    $db->endTrn();
    return true;
}

function insert($db, $tb) {
    $tb->setKeyNum(0);
    for ($i = 1; $i <= RECORD_COUNT; $i++) {
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $i);
        $tb->setFV(FDI_NAME, $i);
        $tb->insert();
        if (!assertEquals($tb->stat(), 0, 'insert'))
            return false;
    }
    return true;
}

function insertTransaction($db, $tb) {
    $tb->setKeyNum(0);
    $start = 1;
    while ($start < RECORD_COUNT) {
        $db->beginTrn();
        for ($i = $start; $i < $start + RECORD_UNIT; $i++) {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, $i);
            $tb->insert();
            if (!assertEquals($tb->stat(), 0, 'insertTransaction')) {
                $db->endTrn();
                return false;
            }
        }
        $db->endTrn();
        $start += RECORD_UNIT;
    }
    return true;
}

function insertBulk($db, $tb) {
    $tb->setKeyNum(0);
    $start = 1;
    while ($start < RECORD_COUNT) {
        $tb->beginBulkInsert(BULKBUFSIZE);
        for ($i = $start; $i < $start + RECORD_UNIT; $i++) {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, $i);
            $tb->insert();
            if (!assertEquals($tb->stat(), 0, 'insertBulk')) {
                $tb->commitBulkInsert();
                return false;
            }
        }
        $tb->commitBulkInsert();
        $start += RECORD_UNIT;
    }
    return true;
}

function read($db, $tb) {
    $tb->setKeyNum(0);
    for ($i = 1; $i <= RECORD_COUNT; $i++) {
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $i);
        $tb->seek();
        if (!assertEquals($tb->stat(), 0, 'read') ||
            !assertEquals($tb->getFVint(FDI_ID), $i, 'read value')) {
            return false;
        }
    }
    return true;
}

function readSnapshot($db, $tb) {
    $ret = true;
    $tb->setKeyNum(0);
    $db->beginSnapshot();
    for ($i = 1; $i <= RECORD_COUNT; $i++) {
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $i);
        $tb->seek();
        if (!assertEquals($tb->stat(), 0, 'read') ||
            !assertEquals($tb->getFVint(FDI_ID), $i, 'read value')) {
            $ret = false;
            break;
        }
    }
    $db->endSnapshot();
    return true;
}

function readRange($db, $tb) {
    $tb->setKeyNum(0);
    $start = 1;
    while ($start < RECORD_COUNT) {
        $tb->clearBuffer();
        $tb->setFilter('*', 1, RECORD_UNIT);
        $tb->setFV(FDI_ID, $start);
        $tb->find(Bz\table::findForword);
        for ($i = $start; $i < $start + RECORD_UNIT; $i++) {
            if (!assertEquals($tb->stat(), 0, 'readRange') ||
                !assertEquals($tb->getFVint(FDI_ID), $i, 'readRange value')) {
                return false;
            }
            $tb->findNext();
        }
        $start += RECORD_UNIT;
    }
    return true;
}

function readRangeSnapshot($db, $tb) {
    $tb->setKeyNum(0);
    $db->beginSnapshot();
    $start = 1;
    while ($start < RECORD_COUNT) {
        $tb->clearBuffer();
        $tb->setFilter('*', 1, RECORD_UNIT);
        $tb->setFV(FDI_ID, $start);
        $tb->find(Bz\table::findForword);
        for ($i = $start; $i < $start + RECORD_UNIT; $i++) {
            if (!assertEquals($tb->stat(), 0, 'readRange snapshot') ||
                !assertEquals($tb->getFVint(FDI_ID), $i, 'readRange snapshot value')) {
                return false;
            }
            $tb->findNext();
        }
        $start += RECORD_UNIT;
    }
    $db->endSnapshot();
    return true;
}

function update($db, $tb) {
    $tb->setKeyNum(0);
    for ($i = 1; $i <= RECORD_COUNT; $i++) {
        $tb->clearBuffer();
        $tb->setFV(FDI_ID, $i);
        $tb->setFV(FDI_NAME, ($i + 1));
        $tb->update(Bz\table::changeInKey);
        if (!assertEquals($tb->stat(), 0, 'update'))
            return false;
    }
    return true;
}

function updateTransaction($db, $tb) {
    $tb->setKeyNum(0);
    $start = 1;
    while ($start < RECORD_COUNT) {
        $db->beginTrn();
        for ($i = $start; $i < $start + RECORD_UNIT; $i++) {
            $tb->clearBuffer();
            $tb->setFV(FDI_ID, $i);
            $tb->setFV(FDI_NAME, ($i + 2));
            $tb->update(Bz\table::changeInKey);
            if (!assertEquals($tb->stat(), 0, 'updateTransaction')) {
                $db->endTrn();
                return false;
            }
        }
        $db->endTrn();
        $start += RECORD_UNIT;
    }
    return true;
}

function main($url) {
    Bz\benchmark::start();
    create($url);
    Bz\benchmark::showTimeSec(true, ': create');
    
    $db = new Bz\database();
    $db->open($url, Bz\transactd::TYPE_SCHEMA_BDF, Bz\transactd::TD_OPEN_NORMAL);
    assertEquals($db->stat(), 0);
    $tb = $db->openTable(TABLENAME);
    assertEquals($db->stat(), 0);
    
    echo("--------------------------------\n");
    
    deleteAll($db, $tb);
    Bz\benchmark::start();
    $ret = insert($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': insert ' . RECORD_COUNT);
    
    echo("--------------------------------\n");
    
    deleteAll($db, $tb);
    Bz\benchmark::start();
    $ret = insertTransaction($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': insertTransaction ' . RECORD_COUNT . ' with transaction per ' . RECORD_UNIT);
    
    echo("--------------------------------\n");
    
    deleteAll($db, $tb);
    Bz\benchmark::start();
    $ret = insertBulk($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': insertBulk ' . RECORD_COUNT . ' with bulkmode per ' . RECORD_UNIT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = read($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': read ' . RECORD_COUNT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = readSnapshot($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': read with snapshot ' . RECORD_COUNT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = readRange($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': read ' . RECORD_COUNT . ' with range per ' . RECORD_UNIT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = readRangeSnapshot($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': read ' . RECORD_COUNT . ' with snapshot and range per ' . RECORD_UNIT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = update($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': update ' . RECORD_COUNT);
    
    echo("--------------------------------\n");
    
    Bz\benchmark::start();
    $ret = updateTransaction($db, $tb);
    Bz\benchmark::showTimeSec($ret, ': updateTransaction ' . RECORD_COUNT . ' with transaction per ' . RECORD_UNIT);
}

main(URL);
