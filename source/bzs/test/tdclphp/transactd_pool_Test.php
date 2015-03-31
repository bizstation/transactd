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

function getHost()
{
    $host = getenv('TRANSACTD_PHPUNIT_HOST');
    if (strlen($host) == 0)
    {
        $host = '127.0.0.1';
    }
    return $host;
}

define("PROTOCOL", 'tdap');
define("HOSTNAME", getHost());
define("USERNAME", getenv('TRANSACTD_PHPUNIT_USER'));
define("USERPART", strlen(USERNAME) == 0 ? '' : USERNAME . '@');
define("PASSWORD", getenv('TRANSACTD_PHPUNIT_PASS'));
define("PASSPART", strlen(PASSWORD) == 0 ? '' : '&pwd=' . PASSWORD);
define("DBNAME", 'querytest');
define("SCHEMANAME", 'test');
define("BDFNAME", '?dbfile=' . SCHEMANAME . '.bdf');
define("URL", PROTOCOL . '://' . USERPART . HOSTNAME . '/' . DBNAME . BDFNAME . PASSPART);
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
        $cp = new Bz\connectParams(PROTOCOL, HOSTNAME, DBNAME, SCHEMANAME, USERNAME, PASSWORD);
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
        $dbm1->unUse();
        $dbm2->unUse();
        $dbm3->unUse();
        $dbm4->unUse();
        $dbm5->unUse();
    }
    public function testConnect()
    {
        Bz\pooledDbManager::setMaxConnections(3);
        $cp = new Bz\connectParams(URL);
        $this->assertEquals($cp->uri(), URL);
        $dbm = new Bz\pooledDbManager($cp);
        $atu = new Bz\ActiveTable($dbm, 'user');
        $q = new Bz\query();
        $atu->alias('名前', 'name');
        $q->select('id', 'name', 'group')->where('id', '<=', 15000);
        $rs = $atu->index(0)->keyValue(1)->read($q);
        $this->assertEquals($rs->size(), 15000);
        $this->assertEquals($rs[0]['id'], 1);
        $this->assertEquals($rs[0][0], 1);
        $this->assertEquals($rs[0]['name'], '1 user');
        $this->assertEquals($rs[0][1], '1 user');
        $this->assertEquals($rs[0][2], 1);
        $dbm->unUse();
    }
    public function testMultiThreads()
    {
        if(! class_exists('Thread')){
            echo(' * class Thread not found! * ');
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
