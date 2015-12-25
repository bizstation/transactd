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

class transactdDatetimeTest extends PHPUnit_Framework_TestCase
{
    public function testGetBtrdate()
    {
        $i_nowdate = Bz\transactd::getNowDate(); // get today as integer
        $s_i_nowdate  = Bz\transactd::btrdtoa($i_nowdate);
        $s_i_nowdate2 = Bz\transactd::btrdtoa($i_nowdate, true);
        //print_r($i_nowdate);
        //print_r($s_i_nowdate);
        //print_r($s_i_nowdate2);
        $nowdate = new Bz\btrDate();
        $nowdate->i = $i_nowdate;              // get today as BtrDate
        $s_nowdate  = Bz\transactd::btrdtoa($nowdate);
        $s_nowdate2 = Bz\transactd::btrdtoa($nowdate, true);
        //print_r($nowdate);
        //print_r($s_nowdate);
        //print_r($s_nowdate2);
        $this->assertEquals($s_i_nowdate, $s_nowdate);
        $this->assertEquals($s_i_nowdate2, $s_nowdate2);
    }
    public function testGetBtrtime()
    {
        $i_nowtime = Bz\transactd::getNowTime(); // get now time as integer
        $s_i_nowtime  = Bz\transactd::btrttoa($i_nowtime);
        //print_r($i_nowtime);
        //print_r($s_i_nowtime);
        $nowtime = new Bz\btrTime();
        $nowtime->i = $i_nowtime;  // get now time as BtrTime
        $s_nowtime  = Bz\transactd::btrttoa($nowtime);
        //print_r($nowtime);
        //print_r($s_nowtime);
        $this->assertEquals($s_i_nowtime, $s_nowtime);
    }
    public function testGetBtrdatetime()
    {
        $d = Bz\transactd::atobtrd("2012-08-22");
        //print_r($d);
        $s_date = Bz\transactd::btrdtoa($d);
        $this->assertEquals($s_date, '2012/08/22');
        //print_r($s_date);
        $t = Bz\transactd::atobtrt("15:37:00");
        $s_time = Bz\transactd::btrttoa($t);
        $this->assertEquals($s_time, '15:37:00');
        //print_r($t);
        //print_r($s_time);
        $dt = Bz\transactd::atobtrs('2012-08-22 15:37:00');
        $s_datetime  = Bz\transactd::btrstoa($dt);
        $s_datetime2 = Bz\transactd::btrstoa($dt, true);
        $this->assertEquals($s_datetime, '2012/08/22 15:37:00');
        $this->assertEquals($s_datetime2, '2012-08-22T15:37:00');
        //print_r($dt);
        //print_r($s_datetime);
        //print_r($s_datetime2);
        $s_datetime_d = Bz\transactd::btrdtoa($dt->date);
        $s_datetime_t = Bz\transactd::btrttoa($dt->time);
        $this->assertEquals($s_datetime_d . ' ' . $s_datetime_t, '2012/08/22 15:37:00');
        //print_r($s_datetime_d . ' ' . $s_datetime_t);
    }
    public function testGetBtrtimestampFromString()
    {
        $d = Bz\transactd::atobtrd('2012-08-22');
        $t = Bz\transactd::atobtrt('15:37:00');
        $btrts  = new Bz\btrTimeStamp('2012-08-22 15:37:00');
        $btrts2 = new Bz\btrTimeStamp($d, $t);
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
        $i_nowdate = Bz\transactd::getNowDate();
        $nowdate = new Bz\btrDate();
        $nowdate->i = $i_nowdate;
        $i_nowtime = Bz\transactd::getNowTime();
        $nowtime = new Bz\btrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new Bz\btrTimeStamp($nowdate, $nowtime);
        $s_nowdate  = Bz\transactd::btrdtoa($nowdate);
        $s_nowtime  = Bz\transactd::btrttoa($nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $this->assertEquals($s_nowdatetime, $s_nowdate . ' ' . $s_nowtime);
        //print_r($nowdatetime);
        //print_r($s_nowdatetime);
        //print_r($s_nowdate . ' ' . $s_nowtime);
    }
    public function testLastYear()
    {
        $i_nowdate = Bz\transactd::getNowDate();
        $nowdate = new Bz\btrDate();
        $nowdate->i = $i_nowdate;
        $nowyear_yy = $nowdate->yy;
        $i_nowtime = Bz\transactd::getNowTime();
        $nowtime = new Bz\btrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new Bz\btrTimeStamp($nowdate, $nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $lastyear = $nowdate;
        $lastyear->yy = $lastyear->yy - 1;
        $lastyear_yy = $lastyear->yy;
        $s_lastyear = Bz\transactd::btrdtoa($lastyear);
        $lastyeardatetime = new Bz\btrTimeStamp($lastyear, $nowtime);
        $s_lastyeardatetime = $lastyeardatetime->toString();
        $this->assertEquals($nowyear_yy - 1, $lastyear_yy);
        $this->assertEquals($s_lastyeardatetime, str_replace($nowyear_yy, $lastyear_yy, $s_nowdatetime));
        //print_r($lastyeardatetime);
        //print_r($s_lastyear);
        //print_r($s_lastyeardatetime);
    }
    public function testTypename()
    {
        $typename = Bz\transactd::getTypeName(Bz\transactd::ft_integer);
        //print_r($typename);
        $this->assertEquals($typename, 'Integer');
    }
}
