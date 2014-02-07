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

class transactdDatetimeTest extends PHPUnit_Framework_TestCase
{
    public function testGetBtrdate()
    {
        $i_nowdate = transactd::getNowDate(); // get today as integer
        $s_i_nowdate  = transactd::btrdtoa($i_nowdate);
        $s_i_nowdate2 = transactd::btrdtoa($i_nowdate, true);
        //print_r($i_nowdate);
        //print_r($s_i_nowdate);
        //print_r($s_i_nowdate2);
        $nowdate = new btrDate();
        $nowdate->i = $i_nowdate;              // get today as BtrDate
        $s_nowdate  = transactd::btrdtoa($nowdate);
        $s_nowdate2 = transactd::btrdtoa($nowdate, true);
        $cs_nowdate = transactd::c_str($nowdate);
        //print_r($nowdate);
        //print_r($s_nowdate);
        //print_r($s_nowdate2);
        //print_r($cs_nowdate);
        $this->assertEquals($s_i_nowdate, $s_nowdate);
        $this->assertEquals($s_i_nowdate2, $s_nowdate2);
        $this->assertEquals($cs_nowdate, $s_nowdate);
    }
    public function testGetBtrtime()
    {
        $i_nowtime = transactd::getNowTime(); // get now time as integer
        $s_i_nowtime  = transactd::btrttoa($i_nowtime);
        $s_i_nowtime2 = transactd::btrttoa($i_nowtime, true);
        //print_r($i_nowtime);
        //print_r($s_i_nowtime);
        //print_r($s_i_nowtime2);
        $nowtime = new btrTime();
        $nowtime->i = $i_nowtime;  // get now time as BtrTime
        $s_nowtime  = transactd::btrttoa($nowtime);
        $s_nowtime2 = transactd::btrttoa($nowtime, true);
        $cs_nowtime = transactd::c_str($nowtime);
        //print_r($nowtime);
        //print_r($s_nowtime);
        //print_r($s_nowtime2);
        //print_r($cs_nowtime);
        $this->assertEquals($s_i_nowtime, $s_nowtime);
        $this->assertEquals($s_i_nowtime2, $s_nowtime2);
        $this->assertEquals($cs_nowtime, $s_nowtime);
    }
    public function testGetBtrdatetime()
    {
        $d = transactd::atobtrd("2012-08-22");
        //print_r($d);
        $s_date = transactd::btrdtoa($d);
        $this->assertEquals($s_date, '2012/08/22');
        //print_r($s_date);
        $t = transactd::atobtrt("15:37:00");
        $s_time = transactd::btrttoa($t);
        $this->assertEquals($s_time, '15:37:00');
        //print_r($t);
        //print_r($s_time);
        $dt = transactd::atobtrs('2012-08-22 15:37:00');
        $s_datetime  = transactd::btrstoa($dt);
        $s_datetime2 = transactd::btrstoa($dt, true);
        $this->assertEquals($s_datetime, '2012/08/22 15:37:00');
        $this->assertEquals($s_datetime2, '2012-08-22T15:37:00');
        //print_r($dt);
        //print_r($s_datetime);
        //print_r($s_datetime2);
        $s_datetime_d = transactd::btrdtoa($dt->date);
        $s_datetime_t = transactd::btrttoa($dt->time);
        $this->assertEquals($s_datetime_d . ' ' . $s_datetime_t, '2012/08/22 15:37:00');
        //print_r($s_datetime_d . ' ' . $s_datetime_t);
    }
    public function testGetBdate()
    {
        $bd = transactd::atobtrd('2012-08-22');
        $bdate  = new bdate($bd->i);
        $bdate2 = new bdate(transactd::btrdtoa($bd));
        //print_r($bdate);
        //print_r($bdate2);
        $btrdate  = $bdate->btr_date();
        $btrdate2 = $bdate2->btr_date();
        //print_r($btrdate);
        //print_r($btrdate2);
        $s_bdate  = $bdate->c_str();
        $s_bdate2 = $bdate2->c_str();
        $this->assertEquals($s_bdate, $s_bdate2);
        //print_r($s_bdate);
        //print_r($s_bdate2);
        $this->assertEquals($bdate->year(), 2012);
        $this->assertEquals($bdate->month(), 8);
        $this->assertEquals($bdate->date(), 22);
        $this->assertEquals($bdate->year_str(), '2012');
        $this->assertEquals($bdate->month_str(), '8');
        $this->assertEquals($bdate->date_str(), '22');
        //print_r($bdate->year());
        //print_r($bdate->month());
        //print_r($bdate->date());
        //print_r($bdate->year_str());
        //print_r($bdate->month_str());
        //print_r($bdate->date_str());
    }
    public function testGetBtrtimestampFromString()
    {
        $d = transactd::atobtrd('2012-08-22');
        $t = transactd::atobtrt('15:37:00');
        $btrts  = new btrTimeStamp('2012-08-22 15:37:00');
        $btrts2 = new btrTimeStamp($d, $t);
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
        $i_nowdate = transactd::getNowDate();
        $nowdate = new btrDate();
        $nowdate->i = $i_nowdate;
        $i_nowtime = transactd::getNowTime();
        $nowtime = new btrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new btrTimeStamp($nowdate, $nowtime);
        $s_nowdate  = transactd::btrdtoa($nowdate);
        $s_nowtime  = transactd::btrttoa($nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $this->assertEquals($s_nowdatetime, $s_nowdate . ' ' . $s_nowtime);
        //print_r($nowdatetime);
        //print_r($s_nowdatetime);
        //print_r($s_nowdate . ' ' . $s_nowtime);
    }
    public function testLastYear()
    {
        $i_nowdate = transactd::getNowDate();
        $nowdate = new btrDate();
        $nowdate->i = $i_nowdate;
        $nowyear_yy = $nowdate->yy;
        $i_nowtime = transactd::getNowTime();
        $nowtime = new btrTime();
        $nowtime->i = $i_nowtime;
        $nowdatetime = new btrTimeStamp($nowdate, $nowtime);
        $s_nowdatetime = $nowdatetime->toString();
        $lastyear = $nowdate;
        $lastyear->yy = $lastyear->yy - 1;
        $lastyear_yy = $lastyear->yy;
        $s_lastyear = transactd::btrdtoa($lastyear);
        $lastyeardatetime = new btrTimeStamp($lastyear, $nowtime);
        $s_lastyeardatetime = $lastyeardatetime->toString();
        $this->assertEquals($nowyear_yy - 1, $lastyear_yy);
        $this->assertEquals($s_lastyeardatetime, str_replace($nowyear_yy, $lastyear_yy, $s_nowdatetime));
        //print_r($lastyeardatetime);
        //print_r($s_lastyear);
        //print_r($s_lastyeardatetime);
    }
    public function testTypename()
    {
        $typename = transactd::getTypeName(transactd::ft_integer);
        //print_r($typename);
        $this->assertEquals($typename, 'Integer');
    }
}
