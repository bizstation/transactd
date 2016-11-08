# coding : utf-8
=begin ============================================================
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
===================================================================
=end
require 'transactd'

def hhnnssuu2i(hh, nn, ss, uu)
  uu + ss * 256 + nn * 256**2 + hh * 256**3
end

def yymmdd2i(yy, mm, dd)
  dd + mm * 256 + yy * 256**2
end

describe Transactd, 'datetime' do
  it 'get BtrDate' do
    i_nowdate = Transactd::getNowDate() # get today as integer
    s_i_nowdate  = Transactd::btrdtoa(i_nowdate)
    s_i_nowdate2 = Transactd::btrdtoa(i_nowdate, true)
    #p i_nowdate
    #p s_i_nowdate + ' ' + s_i_nowdate.encoding.to_s
    #p s_i_nowdate2 + ' ' + s_i_nowdate2.encoding.to_s
    nowdate = Transactd::BtrDate.new()  # get today as BtrDate
    nowdate.i = i_nowdate
    s_nowdate  = Transactd::btrdtoa(nowdate)
    s_nowdate2 = Transactd::btrdtoa(nowdate, true)
    #p nowdate
    #p s_nowdate + ' ' + s_nowdate.encoding.to_s
    #p s_nowdate2 + ' ' + s_nowdate2.encoding.to_s
    expect(s_i_nowdate).to eq s_nowdate
    expect(s_i_nowdate2).to eq s_nowdate2
    expect(nowdate.yy.class).to eq Fixnum
    expect(nowdate.mm.class).to eq Fixnum
    expect(nowdate.dd.class).to eq Fixnum
    expect("#{format('%04d', nowdate.yy)}/#{format('%02d', nowdate.mm)}/#{format('%02d', nowdate.dd)}").to eq s_nowdate
    expect("#{format('%04d', nowdate.yy)}-#{format('%02d', nowdate.mm)}-#{format('%02d', nowdate.dd)}").to eq s_nowdate2
    expect(i_nowdate).to eq yymmdd2i(nowdate.yy, nowdate.mm, nowdate.dd)
    nowdate.yy = 2014
    nowdate.mm = 8
    nowdate.dd = 15
    expect(nowdate.yy).to eq 2014
    expect(nowdate.mm).to eq 8
    expect(nowdate.dd).to eq 15
    expect(nowdate.i).to eq yymmdd2i(2014, 8, 15)
  end
  
  it 'get BtrTime' do
    i_nowtime = Transactd::getNowTime()   # get now time as integer
    s_i_nowtime  = Transactd::btrttoa(i_nowtime)
    #p i_nowtime
    #p s_i_nowtime + ' ' + s_i_nowtime.encoding.to_s
    nowtime = Transactd::BtrTime.new()    # get now time as BtrTime
    nowtime.i = i_nowtime
    s_nowtime  = Transactd::btrttoa(nowtime)
    #p nowtime
    #p s_nowtime + ' ' + s_nowtime.encoding.to_s
    expect(s_i_nowtime).to eq s_nowtime
    expect(nowtime.hh.class).to eq Fixnum
    expect(nowtime.nn.class).to eq Fixnum
    expect(nowtime.ss.class).to eq Fixnum
    expect(nowtime.uu.class).to eq Fixnum
    expect("#{format('%02d', nowtime.hh)}:#{format('%02d', nowtime.nn)}:#{format('%02d', nowtime.ss)}").to eq s_nowtime
    expect(i_nowtime).to eq hhnnssuu2i(nowtime.hh, nowtime.nn, nowtime.ss, nowtime.uu)
    nowtime.hh = 1
    nowtime.nn = 2
    nowtime.ss = 31
    nowtime.uu = 32
    expect(nowtime.hh).to eq 1
    expect(nowtime.nn).to eq 2
    expect(nowtime.ss).to eq 31
    expect(nowtime.uu).to eq 32
    expect(nowtime.i).to eq hhnnssuu2i(1, 2, 31, 32)
  end
  
  it 'get BtrDateTime' do
    date = Transactd::atobtrd('2012-08-22')
    s_date = Transactd::btrdtoa(date)
    expect(s_date).to eq '2012/08/22'
    #p date
    #p s_date + ' ' + s_date.encoding.to_s
    time = Transactd::atobtrt('15:37:00')
    s_time = Transactd::btrttoa(time)
    expect(s_time).to eq '15:37:00'
    #p time
    #p s_time + ' ' + s_time.encoding.to_s
    datetime = Transactd::atobtrs('2012-08-22 15:37:00')
    s_datetime  = Transactd::btrstoa(datetime)
    s_datetime2 = Transactd::btrstoa(datetime, true)
    expect(s_datetime).to eq '2012/08/22 15:37:00'
    expect(s_datetime2).to eq '2012-08-22T15:37:00'
    #p datetime
    #p s_datetime + ' ' + s_datetime.encoding.to_s
    #p s_datetime2 + ' ' + s_datetime2.encoding.to_s
    s_datetime_d = Transactd::btrdtoa(datetime.date)
    s_datetime_t = Transactd::btrttoa(datetime.time)
    expect(s_datetime_d + ' ' + s_datetime_t).to eq '2012/08/22 15:37:00'
    #p s_datetime_d + ' ' + s_datetime_t
  end
  
  it 'get BtrTimeStamp from string' do
    date = Transactd::atobtrd('2012-08-22')
    time = Transactd::atobtrt('15:37:00')
    btrts  = Transactd::BtrTimeStamp.new('2012-08-22 15:37:00')
    btrts2 = Transactd::BtrTimeStamp.new(date, time)
    s_btrts  = btrts.toString()
    s_btrts2 = btrts2.toString()
    #p btrts, btrts2
    #p s_btrts, s_btrts2
    expect(s_btrts).to eq s_btrts2
    expect(s_btrts).to eq '2012/08/22 15:37:00'
  end
  
  it 'get BtrTimeStamp from BtrDate and BtrTime' do
    i_nowdate = Transactd::getNowDate()
    nowdate = Transactd::BtrDate.new()
    nowdate.i = i_nowdate
    i_nowtime = Transactd::getNowTime()
    nowtime = Transactd::BtrTime.new()
    nowtime.i = i_nowtime
    nowdatetime = Transactd::BtrTimeStamp.new(nowdate, nowtime)
    s_nowdate  = Transactd::btrdtoa(nowdate)
    s_nowtime  = Transactd::btrttoa(nowtime)
    s_nowdatetime = nowdatetime.toString()
    expect(s_nowdatetime).to eq s_nowdate + ' ' + s_nowtime
    #p nowdatetime
    #p s_nowdatetime + ' ' + s_nowdatetime.encoding.to_s()
    #p s_nowdate + ' ' + s_nowtime
  end
  
  it 'get last year' do
    i_nowdate = Transactd::getNowDate()
    nowdate = Transactd::BtrDate.new()
    nowdate.i = i_nowdate
    nowyear_yy = nowdate.yy
    i_nowtime = Transactd::getNowTime()
    nowtime = Transactd::BtrTime.new()
    nowtime.i = i_nowtime
    nowdatetime = Transactd::BtrTimeStamp.new(nowdate, nowtime)
    s_nowdatetime = nowdatetime.toString()
    lastyear = nowdate
    lastyear.yy = lastyear.yy - 1
    lastyear_yy = lastyear.yy
    s_lastyear = Transactd::btrdtoa(lastyear)
    lastyeardatetime = Transactd::BtrTimeStamp.new(lastyear, nowtime)
    s_lastyeardatetime = lastyeardatetime.toString()
    expect(nowyear_yy - 1).to eq lastyear_yy
    expect(s_nowdatetime.sub(nowyear_yy.to_s, lastyear_yy.to_s)).to eq s_lastyeardatetime
    #p lastyeardatetime
    #p s_lastyear + ' ' + s_lastyear.encoding.to_s()
    #p s_lastyeardatetime + ' ' + s_lastyeardatetime.encoding.to_s()
  end
  
  it 'get typename' do
    typename = Transactd::getTypeName(Transactd::Ft_integer)
    #p typename + ' ' + typename.encoding.to_s
    expect(typename).to eq 'Integer'
  end
end
