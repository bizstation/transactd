# coding : utf-8
=begin ============================================================
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
===================================================================
=end
require 'transactd'
require 'thwait'

Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)

def getEnv(valuename)
  return ENV[valuename] if ENV[valuename] != nil
  return ''
end

def getHost()
  hostname = getEnv('TRANSACTD_RSPEC_HOST')
  hostname = '127.0.0.1' if hostname == ''
  return hostname
end

PROTOCOL = 'tdap'
HOSTNAME = getHost()
USERNAME = getEnv('TRANSACTD_RSPEC_USER')
USERPART = USERNAME == '' ? '' : USERNAME + '@'
PASSWORD = getEnv('TRANSACTD_RSPEC_PASS')
PASSPART = PASSWORD == '' ? '' : '&pwd=' + PASSWORD
DBNAME = 'querytest'
SCHEMANAME = 'test'
BDFNAME = '?dbfile=' + SCHEMANAME + '.bdf'
URL = PROTOCOL + '://' + USERPART + HOSTNAME + '/' + DBNAME + BDFNAME + PASSPART
TABLENAME = 'user'

def getMode(mode)
  mode += 128 if mode <= -128
  mode += 64 if mode <= -64
  mode += 32 if mode <= -32
  mode += 16 if mode <= -16
  return mode
end

describe Transactd, 'pool' do
  it 'create ConnectParams' do
    cp = Transactd::ConnectParams.new(URL)
    expect(cp.uri()).to eq URL
    cp.setMode(Transactd::TD_OPEN_NORMAL)
    expect(getMode(cp.mode)).to eq Transactd::TD_OPEN_NORMAL
    cp.setMode(Transactd::TD_OPEN_READONLY)
    expect(getMode(cp.mode)).to eq Transactd::TD_OPEN_READONLY
    cp = Transactd::ConnectParams.new(PROTOCOL, HOSTNAME, DBNAME, SCHEMANAME, USERNAME, PASSWORD)
    expect(cp.uri()).to eq URL
  end
  
  it 'use connections' do
    Transactd::PooledDbManager::setMaxConnections(3)
    cp = Transactd::ConnectParams.new(URL)
    cp.setMode(Transactd::TD_OPEN_READONLY)
    expect(getMode(cp.mode)).to eq Transactd::TD_OPEN_READONLY
    expect(cp.uri()).to eq URL
    dbm1 = Transactd::PooledDbManager.new(cp)
    expect(getMode(dbm1.mode)).to eq Transactd::TD_OPEN_READONLY
    dbm2 = Transactd::PooledDbManager.new(cp)
    dbm3 = Transactd::PooledDbManager.new(cp)
    dbm1.unUse()
    dbm4 = Transactd::PooledDbManager.new(cp)
    dbm3.unUse()
    dbm5 = Transactd::PooledDbManager.new(cp)
    Transactd::PooledDbManager::setMaxConnections(5)
    dbm1 = Transactd::PooledDbManager.new(cp)
    dbm3 = Transactd::PooledDbManager.new(cp)
    dbm1.unUse()
    dbm2.unUse()
    dbm3.unUse()
    dbm4.unUse()
    dbm5.unUse()
    dbm1.reset(30)
  end
  
  it 'connect to table' do
    Transactd::PooledDbManager::setMaxConnections(3)
    cp = Transactd::ConnectParams.new(URL)
    cp.setMode(Transactd::TD_OPEN_NORMAL)
    expect(getMode(cp.mode)).to eq Transactd::TD_OPEN_NORMAL
    expect(cp.uri()).to eq URL
    dbm = Transactd::PooledDbManager.new(cp)
    expect(getMode(dbm.mode)).to eq Transactd::TD_OPEN_NORMAL
    atu = Transactd::ActiveTable.new(dbm, 'user')
    q = Transactd::Query.new()
    atu.alias('名前', 'name')
    q.select('id', 'name', 'group').where('id', '<=', '15000')
    rs = atu.index(0).keyValue(1).read(q)
    expect(rs.size()).to eq 15000
    expect(rs[0]['id']).to eq 1
    expect(rs[0][0]).to eq 1
    expect(rs[0]['name']).to eq '1 user'
    expect(rs[0][1]).to eq '1 user'
    expect(rs[0][2]).to eq 1
    atu.release()
    dbm.unUse()
    dbm.reset(30)
  end
  
  it 'can be used in MultiThreads' do
    Transactd::PooledDbManager::setMaxConnections(5)
    threads = [];
    for i in 1..12 do
      threads.push(Thread.new(i) { |i|
        #puts('... waiting to get dbm' + i.to_s + ' ...')
        Thread.pass
        dbm = Transactd::PooledDbManager.new(Transactd::ConnectParams.new(URL))
        #puts('GOT dbm' + i.to_s + ' !')
        sleep(rand(3) + 1)
        #puts('end dbm' + i.to_s)
        dbm.unUse()
      })
    end
    tw = ThreadsWait.new(*threads)
    tw.all_waits()
  end
end
