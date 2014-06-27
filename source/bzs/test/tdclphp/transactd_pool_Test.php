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

define("PROTOCOL", 'tdap');
define("HOSTNAME", 'localhost');
define("DBNAME", 'querytest');
define("SCHEMANAME", 'test');
define("BDFNAME", '?dbfile=' . SCHEMANAME . '.bdf');
define("URL", PROTOCOL . '://' . HOSTNAME . '/' . DBNAME . BDFNAME);
define("TABLENAME", 'user');

// multi thread test if `php_pthreads` exists.
if(class_exists('Thread')){
    class SimpleWorker extends Thread
    {
        public function __construct($name, $url, $sleep=0)
        {
            $this->name = $name;
            $this->url = $url;
            $this->sleep = $sleep;
        }
        public function run()
        {
            //echo ('... waiting to get ' . $this->name . " ...\n");
            $dbm = new Bz\pooledDbManager(new Bz\connectParams($this->url));
            //echo ('GOT ' . $this->name . ' !  sleep ' . $this->sleep . "sec ...\n");
            sleep($this->sleep);
            $dbm->unUse();
            //echo ('end ' . $this->name . "\n");
        }
    }
}

class transactdPoolTest extends PHPUnit_Framework_TestCase
{
    public function testConnectParams()
    {
        $cp = new Bz\connectParams(URL);
        $this->assertEquals($cp->uri(), URL);
        $cp = new Bz\connectParams(PROTOCOL, HOSTNAME, DBNAME, SCHEMANAME);
        $this->assertEquals($cp->uri(), URL);
    }
    public function testUse()
    {
        Bz\pooledDbManager::setMaxConnections(3);
        $cp = new Bz\connectParams(URL);
        $this->assertEquals($cp->uri(), URL);
        $dbm1 = new Bz\pooledDbManager($cp);
        $dbm2 = new Bz\pooledDbManager($cp);
        $dbm3 = new Bz\pooledDbManager($cp);
        $dbm1->unUse();
        $dbm4 = new Bz\pooledDbManager($cp);
        $dbm3->unUse();
        $dbm5 = new Bz\pooledDbManager($cp);
        Bz\pooledDbManager::setMaxConnections(5);
        $dbm1 = new Bz\pooledDbManager($cp);
        $dbm3 = new Bz\pooledDbManager($cp);
    }
    public function testConnect()
    {
        Bz\pooledDbManager::setMaxConnections(3);
        $cp = new Bz\connectParams(URL);
        $this->assertEquals($cp->uri(), URL);
        $dbm = new Bz\pooledDbManager($cp);
        $atu = new Bz\ActiveTable($dbm, 'user');
        $q = new Bz\queryBase();
        $rs = new Bz\RecordSet();
        $atu->alias('名前', 'name');
        $q->select('id', 'name', 'group')->where('id', '<=', 15000);
        $atu->index(0)->keyValue(1)->read($rs, $q);
        $this->assertEquals($rs->size(), 15000);
    }
    public function testMultiThreads()
    {
        if(! class_exists('Thread')){
            return;
        }
        Bz\pooledDbManager::setMaxConnections(5);
        $t = array();
        for ($i = 1; $i <= 12; $i++)
        {
            $t[] = new SimpleWorker('dbm' . $i, URL, rand(1, 3));
            $t[$i - 1]->start();
        }
        $this->assertEquals(true, true);
    }
}
