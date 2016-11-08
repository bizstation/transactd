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
use BizStation\Transactd\BtrTime;
use BizStation\Transactd\BtrDate;
use BizStation\Transactd\BtrTimeStamp;

class TransactdDatetimeTest extends PHPUnit_Framework_TestCase
{
    public function testGetBtrdate()
    {
        $i_nowdate = Transactd::getNowDate(); // get today as integer
        $s_i_nowdate  = Transactd::btrdtoa($i_nowdate);
        $s_i_nowdate2 = Transactd::btrdtoa($i_nowdate, true);
        //print_r($i_nowdate);
        //print_r($s_i_nowdate);
        //print_r($s_i_nowdate2);
        $nowdate = new BtrDate();
        $nowdate->i = $i_nowdate;              // get today as BtrDate
        $s_nowdate  = Transactd::btrdtoa($nowdate);
        $s_nowdate2 = Transactd::btrdtoa($nowdate, true);
        //print_r($nowdate);
        //print_r($s_nowdate);
        //print_r($s_nowdate2);
        $this->assertEquals($s_i_nowdate, $s_nowdate);
        $this->assertEquals($s_i_nowdate2, $s_nowdate2);
    }
    public function testGetBtrtime()
    {
        $i_nowtime = Transactd::getNowTime(); // get now time as integer
        $s_i_nowtime  = Transactd::btrttoa($i_nowtime);
        //print_r($i_nowtime);
        //print_r($s_i_nowtime);
        $nowtime = new BtrTime();
        $nowtime->i = $i_nowtime;  // get now time as BtrTime
        $s_nowtime  = Transactd::btrttoa($nowtime);
        //print_r($nowtime);
        //print_r($s_nowtime);
        $this->assertEquals($s_i_nowtime, $s_nowtime);
    }
    public function testGetBtrdatetime()
    {
        $d = Transactd::atobtrd("2012-08-22");
        //print_r($d);
        $s_date = Transactd::btrdtoa($d);
        $this->assertEquals($s_date, '2012/08/22');
        //print_r($s_date);
        $t = Transactd::atobtrt("15:37:00");
        $s_time = Transactd::btrttoa($t);
        $this->assertEquals($s_time, '15:37:00');
        //print_r($t);
        //print_r($s_time);
        $dt = Transactd::atobtrs('2012-08-22 15:37:00');
        $s_datetime  = Transactd::btrstoa($dt);
        $s_datetime2 = Transactd::btrstoa($dt, true);
        $this->assertEquals($s_datetime, '2012/08/22 15:37:00');
        $this->assertEquals($s_datetime2, '2012-08-22T15:37:00');
        //print_r($dt);
        //print_r($s_datetime);
        //print_r($s_datetime2);
        $s_datetime_d = Transactd::btrdtoa($dt->date);
        $s_datetime_t = Transactd::btrttoa($dt->time);
        $this->assertEquals($s_datetime_d . ' ' . $s_datetime_t, '2012/08/22 15:37:00');
        //print_r($s_datetime_d . ' ' . $s_datetime_t);
    }
    public function testGetBtrtimestampFromString()
    {
        $d = Transactd::atobtrd('2012-08-22');
        $t = Transactd::atobtrt('15:37:00');
        $btrts  = new BtrTimeStamp('2012-08-22 15:37:00');
        $btrts2 = new BtrTimeStamp($d, $t);
        $s_btrts  = $btrts->toString();
        $s_btrts2 = $btrts2->toString();
        //print_r($btrts);
        //print_r($btrts2);
        //print_r($s_btrts);
        //print_r($s_btrts2);
        $this->assertEquals($s_btrts, $s_btrts2);
        $this->assertEquals($s_btrts, '2012/08/22 15:37:00');
    }
    public function testGetBtrtimestampFromBtrdateAndBtrtime()
    {
        $i_nowdate = Transactd::getNowDate();
        $nowdate = new BtrDate();
        $nowdate->i = $i_nowdate;
        $i_nowtime = Transactd::getNowTime();
        $nowtime = new BtrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new BtrTimeStamp($nowdate, $nowtime);
        $s_nowdate  = Transactd::btrdtoa($nowdate);
        $s_nowtime  = Transactd::btrttoa($nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $this->assertEquals($s_nowdatetime, $s_nowdate . ' ' . $s_nowtime);
        //print_r($nowdatetime);
        //print_r($s_nowdatetime);
        //print_r($s_nowdate . ' ' . $s_nowtime);
    }
    public function testLastYear()
    {
        $i_nowdate = Transactd::getNowDate();
        $nowdate = new BtrDate();
        $nowdate->i = $i_nowdate;
        $nowyear_yy = $nowdate->yy;
        $i_nowtime = Transactd::getNowTime();
        $nowtime = new BtrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new BtrTimeStamp($nowdate, $nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $lastyear = $nowdate;
        $lastyear->yy = $lastyear->yy - 1;
        $lastyear_yy = $lastyear->yy;
        $s_lastyear = Transactd::btrdtoa($lastyear);
        $lastyeardatetime = new BtrTimeStamp($lastyear, $nowtime);
        $s_lastyeardatetime = $lastyeardatetime->toString();
        $this->assertEquals($nowyear_yy - 1, $lastyear_yy);
        $this->assertEquals($s_lastyeardatetime, str_replace($nowyear_yy, $lastyear_yy, $s_nowdatetime));
        //print_r($lastyeardatetime);
        //print_r($s_lastyear);
        //print_r($s_lastyeardatetime);
    }
    public function testTypename()
    {
        $typename = Transactd::getTypeName(Transactd::ft_integer);
        //print_r($typename);
        $this->assertEquals($typename, 'Integer');
    }
}
