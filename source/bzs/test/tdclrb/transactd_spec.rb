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
Transactd::Nsdatabase::setExecCodePage(Transactd::CP_UTF8)

require 'rbconfig'
IS_WINDOWS = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/)

def getHost()
  hostname = '127.0.0.1/'
  if (ENV['TRANSACTD_RSPEC_HOST'] != nil && ENV['TRANSACTD_RSPEC_HOST'] != '')
    hostname = ENV['TRANSACTD_RSPEC_HOST']
  end
  hostname = hostname + '/' unless (hostname =~ /\/$/)
  return hostname
end

HOSTNAME = getHost()
DBNAME = 'test'
DBNAME_VAR = 'testvar'
DBNAME_SF = 'testString'
TABLENAME = 'user'
PROTOCOL = 'tdap://'
BDFNAME = '?dbfile=test.bdf'
URL = PROTOCOL + HOSTNAME + DBNAME + BDFNAME
URL_VAR = PROTOCOL + HOSTNAME + DBNAME_VAR + BDFNAME
URL_SF = PROTOCOL + HOSTNAME + DBNAME_SF + BDFNAME
FDI_ID = 0
FDI_NAME = 1
FDI_GROUP = 2
FDI_NAMEW = 2

BULKBUFSIZE = 65535 - 1000
TEST_COUNT = 20000
FIVE_PERCENT_OF_TEST_COUNT = TEST_COUNT / 20

TYPE_SCHEMA_BDF = 0

ISOLATION_READ_COMMITTED = true
ISOLATION_REPEATABLE_READ = false

def testDropDatabase(db)
  db.open(URL)
  expect(db.stat()).to eq 0
  db.drop()
  expect(db.stat()).to eq 0
end

def testCreateDatabase(db)
  db.create(URL)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    testDropDatabase(db)
    db.create(URL)
  end
  expect(db.stat()).to eq 0
end

def testOpenDatabase(db)
  db.open(URL, TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
end

def testCreateTable(db)
  testOpenDatabase(db)
  dbdef = db.dbDef()
  expect(dbdef).not_to be nil
  td = Transactd::Tabledef.new()
  td.setTableName(TABLENAME)
  td.setFileName(TABLENAME + '.dat')
  td.id = 1
  td.pageSize = 2048
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  fd = dbdef.insertField(1, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  fd = dbdef.insertField(1, 1)
  fd.setName('name')
  fd.type = Transactd::Ft_zstring
  fd.len = 33
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  kd = dbdef.insertKey(1,0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
end

def testOpenTable(db)
  testOpenDatabase(db)
  tb = db.openTable(TABLENAME)
  expect(db.stat()).to eq 0
  return tb
end

def testVersion(db)
  db.connect(PROTOCOL + HOSTNAME)
  expect(db.stat()).to eq 0
  vv = Transactd::BtrVersions.new()
  db.getBtrVersion(vv)
  expect(db.stat()).to eq 0
  client_ver = vv.version(0)
  server_ver = vv.version(1)
  engine_ver = vv.version(2)
  expect(client_ver.majorVersion.to_s).to eq Transactd::CPP_INTERFACE_VER_MAJOR.to_s
  expect(client_ver.minorVersion.to_s).to eq Transactd::CPP_INTERFACE_VER_MINOR.to_s
  expect(client_ver.type.chr).to eq 'N'
  expect(server_ver.majorVersion).to be >= 5
  expect(server_ver.minorVersion).to be >= 5
  expect(server_ver.type.chr).to eq 'M'
  expect(engine_ver.majorVersion.to_s).to eq Transactd::TRANSACTD_VER_MAJOR.to_s
  expect(engine_ver.minorVersion.to_s).to eq Transactd::TRANSACTD_VER_MINOR.to_s
  expect(engine_ver.type.chr).to eq 'T'
end

def testInsert(db)
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  if tb.recordCount() == 0
    tb.clearBuffer()
    tb.setFV(FDI_ID, 1)
    tb.setFV(FDI_NAME, 'kosaka')
    tb.insert()
    expect(tb.stat()).to eq 0
  end
  db.beginTrn()
  n = 1
  tb.seekLast()
  n = tb.getFVint(FDI_ID) + 1 if tb.stat()==0
  tb.beginBulkInsert(BULKBUFSIZE)
  for i in n..(TEST_COUNT + n) do
    tb.clearBuffer()
    tb.setFV(FDI_ID, i)
    tb.setFV(FDI_NAME, i.to_s)
    tb.insert()
  end
  tb.commitBulkInsert()
  db.endTrn()
  expect(tb.stat()).to eq 0
  tb.close()
end

def testFind(db)
  tb = testOpenTable(db)
  tb.setKeyNum(0)
  tb.clearBuffer()
  tb.setFilter('id >= 10 and id < ' + TEST_COUNT.to_s, 1, 0)
  v = 10
  tb.setFV(FDI_ID, v)
  tb.find(Transactd::Table::FindForword)
  i = v
  while i < TEST_COUNT do
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq i
    tb.findNext(true) # 11 - 19
    i = i + 1
  end
  # backforword
  tb.clearBuffer()
  v = TEST_COUNT - 1
  tb.setFV(FDI_ID, v)
  tb.find(Transactd::Table::FindBackForword)
  i = v
  while i >= 10 do
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq i
    tb.findPrev(true) # 11 - 19
    i = i - 1
  end
  # out of filter range (EOF)
  v = TEST_COUNT
  tb.setFV(FDI_ID, v)
  tb.find(Transactd::Table::FindForword)
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  tb.close()
end

def testFindNext(db)
  tb = testOpenTable(db)
  tb.setKeyNum(0)
  tb.clearBuffer()
  tb.setFilter('id >= 10 and id < ' + TEST_COUNT.to_s, 1, 0)
  v = 10
  tb.setFV(FDI_ID, v)
  tb.seekGreater(true)
  expect(tb.getFVint(FDI_ID)).to eq v
  for i in (v + 1)..(TEST_COUNT - 1) do
    tb.findNext(true) # 11 - 19
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq i
  end
  tb.close()
end

def testGetPercentage(db)
  tb = testOpenTable(db)
  tb.clearBuffer()
  vv = TEST_COUNT / 2 + 1
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.stat()).to eq 0
  per = tb.getPercentage()
  expect((5000 - per).abs).to be < 500 # 500 = 5%
  tb.close()
end

def testMovePercentage(db)
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekByPercentage(5000) # 50%
  expect(tb.stat()).to eq 0
  v = tb.getFVint(FDI_ID)
  expect(tb.stat()).to eq 0
  expect((TEST_COUNT / 2 + 1 - v).abs).to be < FIVE_PERCENT_OF_TEST_COUNT
  tb.close()
end

def testGetEqual(db)
  tb = testOpenTable(db)
  db.beginSnapshot()
  vv = 1
  for i in 2..(TEST_COUNT + 1) do
    tb.clearBuffer()
    tb.setFV(FDI_ID, i)
    tb.seek()
    expect(tb.getFVint(FDI_ID)).to eq i
  end
  db.endSnapshot()
  tb.close()
end

def testGetNext(db)
  tb = testOpenTable(db)
  db.beginSnapshot()
  vv = 2
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.getFVint(FDI_ID)).to eq vv
  for i in 3..(TEST_COUNT + 1)
    tb.seekNext()
    expect(tb.getFVint(FDI_ID)).to eq i
  end
  db.endSnapshot()
  tb.close()
end

def testGetPrevious(db)
  tb = testOpenTable(db)
  db.beginSnapshot()
  vv = TEST_COUNT + 1
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.getFVint(FDI_ID)).to eq vv
  for i in TEST_COUNT.downto(2) do
    tb.seekPrev()
    expect(tb.getFVint(FDI_ID)).to eq i
  end
  tb.seekPrev()
  expect(tb.getFVstr(FDI_NAME)).to eq 'kosaka'
  db.endSnapshot()
  tb.close()
end

def testGetGreater(db)
  tb = testOpenTable(db)
  vv = TEST_COUNT * 3 / 4
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekGreater(true)
  expect(tb.getFVint(FDI_ID)).to eq vv
  tb.seekNext()
  expect(tb.getFVint(FDI_ID)).to eq vv + 1
  vv = vv - FIVE_PERCENT_OF_TEST_COUNT
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekGreater(false)
  expect(tb.getFVint(FDI_ID)).to eq vv + 1
  tb.seekPrev()
  expect(tb.getFVint(FDI_ID)).to eq vv
  tb.close()
end

def testGetLessThan(db)
  tb = testOpenTable(db)
  vv = TEST_COUNT * 3 / 4
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekLessThan(true)
  expect(tb.getFVint(FDI_ID)).to eq vv
  tb.seekNext()
  expect(tb.getFVint(FDI_ID)).to eq vv + 1
  vv = vv - FIVE_PERCENT_OF_TEST_COUNT
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekLessThan(false)
  expect(tb.getFVint(FDI_ID)).to eq vv - 1
  tb.seekPrev()
  expect(tb.getFVint(FDI_ID)).to eq vv - 2
  tb.close()
end

def testGetFirst(db)
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekFirst()
  expect(tb.getFVstr(FDI_NAME)).to eq 'kosaka'
  tb.close()
end

def testGetLast(db)
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekLast()
  expect(tb.getFVint(FDI_ID)).to eq TEST_COUNT + 2
  tb.close()
end

def testMovePosition(db)
  tb = testOpenTable(db)
  tb.clearBuffer()
  vv = TEST_COUNT * 3 / 4
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekLessThan(true)
  expect(tb.getFVint(FDI_ID)).to eq vv
  pos =  tb.bookmark()
  pos_vv = vv
  expect(tb.stat()).to eq 0
  vv = vv - FIVE_PERCENT_OF_TEST_COUNT
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seekLessThan(false)
  expect(tb.getFVint(FDI_ID)).to eq vv - 1
  tb.seekPrev()
  expect(tb.getFVint(FDI_ID)).to eq vv - 2
  tb.seekByBookmark(pos)
  expect(tb.getFVint(FDI_ID)).to eq pos_vv
  tb.close()
end

def testUpdate(db)
  tb = testOpenTable(db)
  db.beginTrn()
  # test of ncc
  v = 5
  tb.clearBuffer()
  tb.setFV(FDI_ID, v)
  tb.seek()
  expect(tb.stat()).to eq 0
  v = TEST_COUNT + TEST_COUNT / 2
  tb.setFV(FDI_ID, v)
  tb.update(Transactd::Table::ChangeCurrentNcc) # 5 . 30000 cur 5
  expect(tb.stat()).to eq 0
  tb.seekNext() # next 5
  expect(tb.getFVint(FDI_ID)).to eq 6
  v = TEST_COUNT - 1
  tb.setFV(FDI_ID, v)
  tb.seek()
  expect(tb.getFVint(FDI_ID)).to eq v
  v = 5
  tb.setFV(FDI_ID, v)
  tb.update(Transactd::Table::ChangeCurrentCc)  # 19999 . 5 cur 5
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).to eq 6
  v = TEST_COUNT - 1
  tb.setFV(FDI_ID, v)
  tb.update(Transactd::Table::ChangeCurrentCc)  # 6 . 19999 cur 19999
  tb.seekPrev() # prev 19999
  expect(tb.getFVint(FDI_ID)).to eq v - 1
  v = 10
  tb.clearBuffer()
  tb.setFV(FDI_ID, v)
  tb.seek()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.getFVint(FDI_ID)).to eq 11
  for i in 10..(TEST_COUNT - 2)
    tb.clearBuffer()
    tb.setFV(FDI_ID, i)
    tb.seek()
    expect(tb.stat()).to eq 0
    v = i + 1
    tb.setFV(FDI_NAME, v)
    tb.update()
    expect(tb.stat()).to eq 0
  end
  db.endTrn()
  # check update in key
  v = 8
  tb.setFV(FDI_ID, v)
  tb.setFV(FDI_NAME, 'ABC')
  tb.update(Transactd::Table::ChangeInKey)
  expect(tb.stat()).to eq 0
  tb.clearBuffer()
  tb.setFV(FDI_ID, v)
  tb.seek()
  expect(tb.getFVstr(FDI_NAME)).to eq 'ABC'
  tb.close()
end

def testSnapShot(db)
  db2 = Transactd::Database::createObject()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME , true)
  expect(db2.stat()).to eq 0
  tb = testOpenTable(db)
  tb2 = testOpenTable(db2)
  db.beginSnapshot()
  expect(db.stat()).to eq 0
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  firstValue = tb.getFVint(FDI_NAME)
  tb.seekNext()
  # ----------------------------------------------------
  #   Change data by another connection
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, tb2.getFVint(FDI_ID) + 1)
  tb2.update()
  if ISOLATION_READ_COMMITTED
    expect(tb2.stat()).to eq 0
  elsif ISOLATION_REPEATABLE_READ
    expect(tb2.stat()).to eq STATUS_LOCK_ERROR
  end
  # ----------------------------------------------------
  tb.seekFirst()
  secondValue = tb.getFVint(FDI_NAME)
  expect(tb.stat()).to eq 0
  db.endSnapshot()
  expect(db.stat()).to eq 0
  if ISOLATION_READ_COMMITTED
    expect(secondValue).not_to eq firstValue
  else
    expect(secondValue).to eq firstValue
  end
  # ----------------------------------------------------
  tb.close()
  tb2.close()
  db2.close()
end

def testTransactionLock(db)
  db2 = Transactd::Database::createObject()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  expect(db2.stat()).to eq 0
  tb = testOpenTable(db)
  tb2 = testOpenTable(db2)
  # ----------------------------------------------------
  #   Read test that single record lock with read
  # ----------------------------------------------------
  db.beginTrn(Transactd::LOCK_SINGLE_NOWAIT)
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  # unlock first record.
  tb.seekNext()
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  db2.beginTrn()
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  db2.endTrn()
  db.endTrn()
  # ----------------------------------------------------
  #   Can't read test that multi record lock with read
  # ----------------------------------------------------
  db.beginTrn(Transactd::LOCK_MULTI_NOWAIT)
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  # move from first record.
  tb.seekNext()
  # not transactional user can not read
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  # The second transactional user can not lock same record
  db2.beginTrn()
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.endTrn()
  db.endTrn()
  # ----------------------------------------------------
  #   Can't read test that single record lock with change
  # ----------------------------------------------------
  db.beginTrn(Transactd::LOCK_SINGLE_NOWAIT)
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_NAME, 'ABC')
  tb.update()
  expect(tb.stat()).to eq 0
  # move from first record.
  tb.seekNext()
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.beginTrn()
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.endTrn()
  db.endTrn()
  # ----------------------------------------------------
  #   Abort test that Single record lock transaction
  # ----------------------------------------------------
  db.beginTrn(Transactd::LOCK_SINGLE_NOWAIT)
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_NAME, 'EFG')
  tb.update()
  expect(tb.stat()).to eq 0
  # move from first record.
  tb.seekNext()
  db.abortTrn()
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.getFVstr(FDI_NAME)).to eq 'ABC'
  tb.close()
  tb2.close()
  db2.close()
end

def testConflict(db)
  db2 = Transactd::Database::createObject()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME , true)
  expect(db2.stat()).to eq 0
  tb = testOpenTable(db)
  tb2 = testOpenTable(db2)
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  # ----------------------------------------------------
  #   Change Index field
  # ----------------------------------------------------
  # Change data by another connection
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_ID, tb2.getFVint(FDI_ID) - 10)
  tb2.update()
  expect(tb2.stat()).to eq 0
  # ----------------------------------------------------
  # Change same record data by original connection
  tb.setFV(FDI_ID, tb.getFVint(FDI_ID) - 8)
  tb.update()
  expect(tb.stat()).to eq Transactd::STATUS_CHANGE_CONFLICT
  # ----------------------------------------------------
  #   Change Non index field
  # ----------------------------------------------------
  # Change data by another connection
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, tb2.getFVint(FDI_ID) - 10)
  tb2.update()
  expect(tb2.stat()).to eq 0
  # ----------------------------------------------------
  # Change same record data by original connection
  tb.setFV(FDI_NAME, tb.getFVint(FDI_NAME) - 8)
  tb.update()
  expect(tb.stat()).to eq Transactd::STATUS_CHANGE_CONFLICT
  # ----------------------------------------------------
  tb.close()
  tb2.close()
  db2.close()
end

def testInsert2(db)
  tb = testOpenTable(db)
  v = TEST_COUNT * 2
  db.beginTrn()
  tb.clearBuffer()
  tb.setFV(FDI_ID, v)
  tb.insert()
  expect(tb.stat()).to eq 0
  v = 10
  tb.clearBuffer()
  tb.setFV(FDI_ID, v)
  tb.seek()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.getFVint(FDI_ID)).to eq 11
  db.endTrn()
  tb.close()
end

def testDelete(db)
  tb = testOpenTable(db)
  # estimate count
  count = tb.recordCount(true)
  is_valid_count = ((count - TEST_COUNT - 3).abs < FIVE_PERCENT_OF_TEST_COUNT)
  expect(is_valid_count).to be true
  if !is_valid_count
    puts "true record count = #{(TEST_COUNT + 3).to_s} and estimate recordCount count = #{count.to_s}"
  end
  expect(tb.recordCount(false)).to eq TEST_COUNT + 3 # true count
  vv = TEST_COUNT * 3 / 4 + 1
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.getFVint(FDI_ID)).to eq vv
  tb.del()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.stat()).to eq 4
  # check update in key
  vv = 8
  tb.setFV(FDI_ID, vv)
  tb.del(Transactd::Table::Inkey)
  expect(tb.stat()).to eq 0
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.stat()).to eq Transactd::STATUS_NOT_FOUND_TI
  db.beginTrn()
  tb.stepFirst()
  while tb.stat() == 0
    tb.del()
    expect(tb.stat()).to eq 0
    tb.stepNext()
  end
  expect(tb.stat()).to eq 9
  db.endTrn()
  expect(tb.recordCount(false)).to eq 0
  tb.close()
end

def testSetOwner(db)
  tb = testOpenTable(db)
  tb.setOwnerName('ABCDEFG')
  expect(tb.stat()).to eq 0
  tb.clearOwnerName()
  expect(tb.stat()).to eq 0
  tb.close()
end

def testDropIndex(db)
  tb = testOpenTable(db)
  tb.dropIndex(false)
  expect(tb.stat()).to eq 0
  tb.close()
end

def testLogin(db)
  db.connect(PROTOCOL + HOSTNAME)
  expect(db.stat()).to eq 0
  if db.stat() == 0
    # second connection
    db2 = Transactd::Database::createObject()
    db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
    expect(db.stat()).to eq 0
    db2.close()
    db2 = nil
    db.disconnect(PROTOCOL + HOSTNAME)
    expect(db.stat()).to eq 0
  end
  # invalid host name
  db.connect(PROTOCOL + 'localhost123/')
  is_valid_stat = (db.stat() == Transactd::ERROR_TD_INVALID_CLINETHOST) || 
                  (db.stat() == Transactd::ERROR_TD_HOSTNAME_NOT_FOUND)
  expect(is_valid_stat).to be true
  if (!is_valid_stat)
    puts 'bad host db.stat() = ' + db.stat().to_s
  end
  testCreateDatabase(db)
  testCreateTable(db)
  db.disconnect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq 0
  # true database name
  db.connect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq 0
  if (db.stat() == 0)
    db.disconnect(PROTOCOL + HOSTNAME + DBNAME)
    expect(db.stat()).to eq 0
  end
  # invalid database name
  testDropDatabase(db)
  db.disconnect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq 0
  db.connect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq (25000 + 1049)
  db.disconnect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq 0
end

def isUtf16leSupport(db)
  # CHARSET_UTF16LE supported on MySQL 5.6 or later
  vv = Transactd::BtrVersions.new()
  db.getBtrVersion(vv)
  server_ver = vv.version(1)
  if ('M' == server_ver.type.chr)
    if (server_ver.majorVersion <= 4)
      return false
    elsif (server_ver.majorVersion == 5)
      return false if (server_ver.minorVersion <= 5)
    end
    return true
  end
  return false
end

def testCreateVarTable(db, id, name, fieldType, charset)
  # create table
  dbdef = db.dbDef()
  expect(dbdef).not_to be nil
  td = Transactd::Tabledef.new()
  td.setTableName(name)
  td.setFileName(name + '.dat')
  td.id =id
  td.keyCount = 0
  td.fieldCount = 0
  td.flags.all = 0
  td.pageSize = 2048
  td.charsetIndex = charset
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # id
  fd = dbdef.insertField(id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # name
  fd = dbdef.insertField(id, 1)
  fd.setName('name')
  fd.type = fieldType
  if (fieldType == Transactd::Ft_mywvarchar)
    fd.len = 1 + Transactd::charsize(Transactd::CHARSET_UTF16LE) * 3  # max 3 char len byte
  elsif (fieldType == Transactd::Ft_mywvarbinary)
    fd.len = 1 + Transactd::charsize(Transactd::CHARSET_UTF16LE) * 3  # max 6 char len byte
  elsif (fieldType == Transactd::Ft_myvarchar)
    if (charset == Transactd::CHARSET_CP932)
      fd.len = 1 + Transactd::charsize(Transactd::CHARSET_CP932) * 3  # max 6 char len byte
    elsif(charset == Transactd::CHARSET_UTF8B4)
      fd.len = 1 + Transactd::charsize(Transactd::CHARSET_UTF8B4) * 3 # max 6 char len byte
    end
  else
    fd.len = 7;  # max 6 char len byte
  end
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # groupid
  fd = dbdef.insertField(id, 2)
  fd.setName('groupid')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # key 1
  kd = dbdef.insertKey(id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segmentCount = 1
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # key 2
  kd = dbdef.insertKey(id, 1)
  kd.segment(0).fieldNum = 1
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segment(0).flags.bit0 = 1  # duplicateable
  kd.segment(0).flags.bit4 = 1  # not last segmnet
  kd.segment(1).fieldNum = 2
  kd.segment(1).flags.bit8 = 1  # extended key type
  kd.segment(1).flags.bit1 = 1  # changeable
  kd.segment(1).flags.bit0 = 1  # duplicateable
  kd.segmentCount = 2
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # open
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  tb.close() if tb != nil
end

def testCreateDatabaseVar(db)
  db.create(URL_VAR)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    testDropDatabaseVar(db)
    db.create(URL_VAR)
  end
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    db.open(URL_VAR, 0, 0)
    expect(db.stat()).to eq 0
  end
  if (0 == db.stat())
    testCreateVarTable(db, 1, 'user1', Transactd::Ft_myvarchar,   Transactd::CHARSET_CP932)
    testCreateVarTable(db, 2, 'user2', Transactd::Ft_myvarbinary, Transactd::CHARSET_CP932)
    if isUtf16leSupport(db)
      testCreateVarTable(db, 3, 'user3', Transactd::Ft_mywvarchar,    Transactd::CHARSET_CP932)
      testCreateVarTable(db, 4, 'user4', Transactd::Ft_mywvarbinary,  Transactd::CHARSET_CP932)
      testCreateVarTable(db, 5, 'user5', Transactd::Ft_myvarchar,     Transactd::CHARSET_UTF8B4)
      db.close()
      db.open(PROTOCOL + HOSTNAME + DBNAME_VAR + '?dbfile=transactd_schemaname', 0, 0)
      expect(db.stat()).to eq 0
    end
  end
end

def testDropDatabaseVar(db)
  db.open(URL_VAR)
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    db.drop()
    expect(db.stat()).to eq 0
  end
end

def dump(str, size)
  p str.bytes.to_a
end

def testSetGetVar(tb, unicodeField, varCharField)
  ### Set Wide Get Wide
  #if IS_WINDOWS
  #  tb.setFVW(FDI_GROUP, '68')
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #else
    tb.setFV(FDI_GROUP, '68')
    expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  #end
  #if IS_WINDOWS
  #  # too long string
  #  tb.setFVW(FDI_NAME, '1234567')
  #  if (varCharField)
  #    expect(tb.getFVWstr(FDI_NAME)).to eq '123'
  #    dump(tb.getFVWstr(FDI_NAME), 7) if (tb.getFVWstr(FDI_NAME) != '123')
  #  else
  #    expect(tb.getFVWstr(FDI_NAME)).to eq '123456'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # short string
  #  tb.setFVW(FDI_NAME, '12 ')
  #  expect(tb.getFVWstr(FDI_NAME)).to eq '12 '
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # too long kanji
  #  if (unicodeField)
  #    tb.setFVW(1, 'あいうえお𩸽') # hiragana 'aiueo' + kanji 'hokke'
  #    if (varCharField)
  #      expect(tb.getFVWstr(FDI_NAME)).to eq 'あいう'
  #    else
  #      expect(tb.getFVWstr(FDI_NAME)).to eq 'あいうえお'
  #    end
  #  else
  #    tb.setFVW(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
  #    expect(tb.getFVWstr(FDI_NAME)).to eq '0松本'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #end
  ### Set Ansi Get Wide
  # too long string
  tb.setFVA(FDI_NAME, '1234567')
  if (varCharField)
    expect(tb.getFVAstr(FDI_NAME)).to eq '123'
  else
    expect(tb.getFVAstr(FDI_NAME)).to eq '123456'
  end
  #if IS_WINDOWS
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #else
    expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
  #end
  # short string
  tb.setFVA(FDI_NAME, '13 ')
  expect(tb.getFVAstr(FDI_NAME)).to eq '13 '
  #if IS_WINDOWS
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #else
    expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
  #end
  # too long kanji
  if (unicodeField)
    if !IS_WINDOWS
      tb.setFVA(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
      if (varCharField)
        expect(tb.getFVAstr(FDI_NAME)).to eq 'あいう'
      else
        expect(tb.getFVAstr(FDI_NAME)).to eq 'あいうえお'
      end
    end
  else
    tb.setFVA(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
    is_valid_value = tb.getFVAstr(FDI_NAME) == '0松本'
    expect(is_valid_value).to be true
    puts tb.getFVAstr(FDI_NAME) if (!is_valid_value)
  end
  expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
  ### Set Wide Get Ansi
  #if IS_WINDOWS
  #  # too long string
  #  tb.setFVW(FDI_NAME, '1234567')
  #  if (varCharField)
  #    expect(tb.getFVAstr(FDI_NAME)).to eq '123'
  #  else
  #    expect(tb.getFVAstr(FDI_NAME)).to eq '123456'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # short string
  #  tb.setFVW(1, '23 ')
  #  expect(tb.getFVAstr(FDI_NAME)).to eq '23 '
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # too long kanji
  #  if (unicodeField)
  #    tb.setFVW(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
  #    if (varCharField)
  #      expect(tb.getFVAstr(FDI_NAME)).to eq 'あいう'
  #    else
  #      expect(tb.getFVAstr(FDI_NAME)).to eq 'あいうえお'
  #    end
  #  else
  #    tb.setFVW(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
  #    expect(tb.getFVAstr(FDI_NAME)).to eq '0松本'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #end
  ### Set Ansi Get Ansi
  # too long string
  tb.setFVA(FDI_NAME, '1234567')
  if (varCharField)
    expect(tb.getFVAstr(FDI_NAME)).to eq '123'
  else
    expect(tb.getFVAstr(FDI_NAME)).to eq '123456'
  end
  expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
  # short string
  tb.setFVA(FDI_NAME, '13 ')
  expect(tb.getFVAstr(FDI_NAME)).to eq '13 '
  expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
  # too long lanji
  if (unicodeField)
    if !IS_WINDOWS
      tb.setFVA(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
      if (varCharField)
        expect(tb.getFVAstr(FDI_NAME)).to eq 'あいう'
      else
        expect(tb.getFVAstr(FDI_NAME)).to eq 'あいうえお'
      end
    end
  else
    tb.setFVA(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
    expect(tb.getFVAstr(FDI_NAME)).to eq '0松本'
  end
  expect(tb.getFVAstr(FDI_GROUP)).to eq '68'
end

def testVarField(db)
  db.open(URL_VAR)
  expect(db.stat()).to eq 0
  tb = db.openTable('user1')
  expect(db.stat()).to eq 0
  # acp varchar
  testSetGetVar(tb, false, true)
  tb.close()
  tb = db.openTable('user2')
  expect(db.stat()).to eq 0
  # acp varbinary
  testSetGetVar(tb, false, false)
  tb.close()
  if (isUtf16leSupport(db))
    tb = db.openTable('user3')
    expect(db.stat()).to eq 0
    # unicode varchar
    testSetGetVar(tb, true, true)
    tb.close()
  end
  tb = db.openTable('user4')
  expect(db.stat()).to eq 0
  # unicode varbinary'
  testSetGetVar(tb, true, false)
  tb.close()
  tb = db.openTable('user5')
  expect(db.stat()).to eq 0
  # utf8 varchar
  testSetGetVar(tb, true, true)
  tb.close()
end

def doVarInsert(db, name, codePage, str, startid, endid, bulk)
  tb = db.openTable(name)
  expect(db.stat()).to eq 0
  tb.beginBulkInsert(BULKBUFSIZE) if (bulk)
  for i in startid..endid do
    tb.clearBuffer()
    tb.setFV(FDI_ID, i)
    tb.setFV(FDI_NAME, str + i.to_s)
    tb.setFV(FDI_GROUP, (i + 10))
    tb.insert()
  end
  tb.commitBulkInsert() if (bulk)
  tb.close()
end

def testVarInsert(db)
  startid = 1
  bulk = false
  str = '漢字文字のテスト' # too long kanji
  str2 = '123'
  db.open(URL_VAR)
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    utf16leSupport = isUtf16leSupport(db)
    doVarInsert(db, 'user1', Transactd::CP_ACP,   str, startid, startid, bulk)
    doVarInsert(db, 'user2', Transactd::CP_ACP,   str, startid, startid, bulk)
    doVarInsert(db, 'user3', Transactd::CP_ACP,   str, startid, startid, bulk) if (utf16leSupport)
    doVarInsert(db, 'user4', Transactd::CP_ACP,   str, startid, startid, bulk)
    doVarInsert(db, 'user5', Transactd::CP_UTF8,  str, startid, startid, bulk)
    startid = startid + 1
    doVarInsert(db, 'user1', Transactd::CP_ACP,   str2, startid, startid, bulk)
    doVarInsert(db, 'user2', Transactd::CP_ACP,   str2, startid, startid, bulk)
    doVarInsert(db, 'user3', Transactd::CP_ACP,   str2, startid, startid, bulk) if (utf16leSupport)
    doVarInsert(db, 'user4', Transactd::CP_ACP,   str2, startid, startid, bulk)
    doVarInsert(db, 'user5', Transactd::CP_UTF8,  str2, startid, startid, bulk)
    startid = startid + 1
    bulk = true
    endid = 1000
    doVarInsert(db, 'user1', Transactd::CP_ACP,   '', startid, endid, bulk)
    doVarInsert(db, 'user2', Transactd::CP_ACP,   '', startid, endid, bulk)
    doVarInsert(db, 'user3', Transactd::CP_ACP,   '', startid, endid, bulk) if (utf16leSupport)
    doVarInsert(db, 'user4', Transactd::CP_ACP,   '', startid, endid, bulk)
    doVarInsert(db, 'user5', Transactd::CP_UTF8,  '', startid, endid, bulk)
  end
end

def doVarRead(db, name, codePage, str, num, key)
  tb = db.openTable(name)
  expect(db.stat()).to eq 0
  tb.clearBuffer()
  tb.setKeyNum(key)
  if (key == 0)
    tb.setFV(FDI_ID, num)
  else
    v = num + 10
    tb.setFV(FDI_NAME, str)
    tb.setFV(FDI_GROUP, v)
  end
  tb.seek()
  expect(tb.stat()).to eq 0
  # test read of var field
  is_valid_value = (str == tb.getFVstr(FDI_NAME))
  expect(is_valid_value).to be true
  # test read of second field
  expect(tb.getFVint(FDI_GROUP)).to eq (num + 10)
  tb.close()
end

def testVarRead(db)
  str = '漢字文'
  str3 = '漢字文字のテ'
  str2 ='123'
  str4 ='1232'
  db.open(URL_VAR)
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    utf16leSupport = isUtf16leSupport(db)
    num = 1
    key = 0
    # too long string
    doVarRead(db, 'user1', Transactd::CP_ACP,   str,  num, key)
    doVarRead(db, 'user2', Transactd::CP_ACP,   str,  num, key)
    doVarRead(db, 'user3', Transactd::CP_ACP,   str,  num, key) if (utf16leSupport)
    doVarRead(db, 'user4', Transactd::CP_ACP,   str3, num, key)
    doVarRead(db, 'user5', Transactd::CP_UTF8,  str,  num, key)
    # short string
    num = num + 1
    doVarRead(db, 'user1', Transactd::CP_ACP,   str2, num, key)
    doVarRead(db, 'user2', Transactd::CP_ACP,   str4, num, key)
    doVarRead(db, 'user3', Transactd::CP_ACP,   str2, num, key) if (utf16leSupport)
    doVarRead(db, 'user4', Transactd::CP_ACP,   str4, num, key)
    doVarRead(db, 'user5', Transactd::CP_UTF8,  str2, num, key)
    key = 1
    doVarRead(db, 'user1', Transactd::CP_ACP,   '120', 120, key)
    doVarRead(db, 'user2', Transactd::CP_ACP,   '120', 120, key)
    doVarRead(db, 'user3', Transactd::CP_ACP,   '120', 120, key) if (utf16leSupport)
    doVarRead(db, 'user4', Transactd::CP_ACP,   '120', 120, key)
    doVarRead(db, 'user5', Transactd::CP_UTF8,  '120', 120, key)
  end
end

def doVarFilter(db, name, codePage, str, num, key)
  tb = db.openTable(name)
  expect(db.stat()).to eq 0
  tb.clearBuffer()
  tb.setKeyNum(key)
  if (key == 0)
    buf = 'id > ' + num.to_s + ' and id <= ' + (num + 10).to_s
    tb.setFilter(buf, 0, 10)
    # find forword
    tb.setFV(FDI_ID, num)
    tb.seekGreater(true)
    expect(tb.stat()).to eq 0
    for i in (num + 1)..(num + 10)
      tb.findNext()
      expect(tb.stat()).to eq 0
      # test read of var field
      expect(tb.getFVint(FDI_NAME)).to eq i
      # test read of second field
      expect(tb.getFVint(FDI_GROUP)).to eq (i + 10)
    end
    # find previous
    v = num + 10
    tb.setFilter(buf, 0, 10)
    tb.setFV(FDI_ID, v)
    tb.seekLessThan(true)
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq v
    for i in (num + 10).downto(num + 1) do
      tb.findPrev(false)
      expect(tb.stat()).to eq 0
      # test read of var field
      expect(tb.getFVint(FDI_NAME)).to eq i
      # test read of second field
      expect(tb.getFVint(FDI_GROUP)).to eq (i + 10)
    end
    # test record count
    expect(tb.recordCount()).to eq 10
  else
    v = num + 10
    tb.setFV(FDI_NAME, str)
    tb.setFV(FDI_GROUP, v)
  end
  tb.close()
end

def testFilterVar(db)
  db.open(URL_VAR)
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    str = '漢字文'
    str3 = '漢字文字のテ'
    str2 = '123'
    str4 = '1232'
    utf16leSupport = isUtf16leSupport(db)
    num = 10
    key = 0
    doVarFilter(db, 'user1', Transactd::CP_ACP,   str,  num, key)
    doVarFilter(db, 'user2', Transactd::CP_ACP,   str,  num, key)
    doVarFilter(db, 'user3', Transactd::CP_ACP,   str,  num, key) if (utf16leSupport)
    doVarFilter(db, 'user4', Transactd::CP_ACP,   str3, num, key)
    doVarFilter(db, 'user5', Transactd::CP_UTF8,  str,  num, key)
    #ifdef _UNICODE
    #  # short string
    #  num = num + 1
    #  doVarFilter(db, 'user1', Transactd::CP_ACP,  str2, num, key)
    #  doVarFilter(db, 'user2', Transactd::CP_ACP,  str4, num, key)
    #  doVarFilter(db, 'user3', Transactd::CP_ACP,  str2, num, key) if (utf16leSupport)
    #  doVarFilter(db, 'user4', Transactd::CP_ACP,  str4, num, key)
    #  doVarFilter(db, 'user5', Transactd::CP_UTF8, str2, num, key)
    #endif
    key = 1
    doVarFilter(db, 'user1', Transactd::CP_ACP,   '120', 120, key)
    doVarFilter(db, 'user2', Transactd::CP_ACP,   '120', 120, key)
    doVarFilter(db, 'user3', Transactd::CP_ACP,   '120', 120, key) if (utf16leSupport)
    doVarFilter(db, 'user4', Transactd::CP_ACP,   '120', 120, key)
    doVarFilter(db, 'user5', Transactd::CP_UTF8,  '120', 120, key)
  end
end


def testCreateTableStringFilter(db, id, name, type, type2)
  # create table
  dbdef = db.dbDef()
  td = Transactd::Tabledef.new()
  td.setTableName(name)
  td.setFileName(name + '.dat')
  td.id =id
  td.pageSize = 2048
  td.charsetIndex = Transactd::CHARSET_UTF8B4
  # td.charsetIndex = Transactd::CHARSET_CP932
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  fd = dbdef.insertField(id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  fd = dbdef.insertField(id, 1)
  fd.setName('name')
  fd.type = type
  fd.len = 44
  if (fd.varLenBytes() != 0)
    fd.len = fd.varLenBytes() + 44
    fd.keylen = fd.len
  end
  if (fd.blobLenBytes() != 0)
    fd.len = 12 # 8+4
  end
  fd.keylen = fd.len
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  fd = dbdef.insertField(id, 2)
  fd.setName('namew')
  fd.type = type2
  fd.len = 44
  if (fd.varLenBytes() != 0)
    fd.len = fd.varLenBytes() + 44
    fd.keylen = fd.len
  end
  if (fd.blobLenBytes() != 0)
    fd.len = 12 # 8+4
  end
  fd.keylen = fd.len
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  kd = dbdef.insertKey(id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segmentCount = 1
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  kd = dbdef.insertKey(id, 1)
  kd.segment(0).fieldNum = 1
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segment(0).flags.bit0 = 1  # duplicateable
  kd.segmentCount = 1
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  kd = dbdef.insertKey(id, 2)
  kd.segment(0).fieldNum = 2
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segment(0).flags.bit0 = 1  # duplicateable
  kd.segmentCount = 1
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
end

def doTestInsertStringFilter(tb)
  tb.clearBuffer()
  id = 1
  tb.setFV('id', id)
  tb.setFV('name', 'あいうえおかきくこ')
  tb.setFV('namew', 'あいうえおかきくこ')
  tb.insert()
  tb.clearBuffer()
  id = 2
  tb.setFV('id', id)
  tb.setFV('name', 'A123456')
  tb.setFV('namew', 'A123456')
  tb.insert()
  tb.clearBuffer()
  id = 3
  tb.setFV('id', id)
  tb.setFV('name', 'あいがあればOKです')
  tb.setFV('namew', 'あいがあればOKです')
  tb.insert()
  tb.clearBuffer()
  id = 4
  tb.setFV('id', id)
  tb.setFV('name', 'おはようございます')
  tb.setFV('namew', 'おはようございます')
  tb.insert()
  tb.clearBuffer()
  id = 5
  tb.setFV('id', id)
  tb.setFV('name', 'おめでとうございます。')
  tb.setFV('namew', 'おめでとうございます。')
  tb.insert()
end

def doTestReadStringFilter(tb)
  tb.setKeyNum(0)
  tb.clearBuffer()
  id = 1
  tb.setFV('id', id)
  tb.seek()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえおかきくこ'
  id =3
  tb.setFV('id', id)
  tb.seek()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'あいがあればOKです'
  tb.setKeyNum(1)
  tb.clearBuffer()
  tb.setFV('name', 'A123456')
  tb.seek()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'A123456'
  tb.setKeyNum(2)
  tb.clearBuffer()
  tb.setFV('namew', 'A123456')
  tb.seek()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'A123456'
end

def doTestSetStringFilter(tb)
  tb.setKeyNum(0)
  tb.clearBuffer()
  tb.setFilter("name = 'あい*'", 0, 10)
  tb.seekFirst()
  tb.findNext(false)
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえおかきくこ'
  expect(tb.recordCount()).to eq 2
  tb.setFilter("name <> 'あい*'", 0, 10)
  expect(tb.recordCount()).to eq 3
  tb.setFilter("name = 'あい'", 0, 10)
  expect(tb.recordCount()).to eq 0
  tb.setFilter("name <> ''", 0, 10)
  expect(tb.recordCount()).to eq 5
end

def doTestStringFilter(db, id, name, type, type2)
  testCreateTableStringFilter(db, id, name, type, type2)
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  doTestInsertStringFilter(tb)
  doTestReadStringFilter(tb)
  doTestSetStringFilter(tb)
  tb.close()
end

def testStringFilter(db)
  db.create(URL_SF)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    testDropDatabaseStringFilter(db)
    db.create(URL_SF)
  end
  expect(db.stat()).to eq 0
  db.open(URL_SF, 0, 0)
  expect(db.stat()).to eq 0
  doTestStringFilter(   db, 1, 'zstring',    Transactd::Ft_zstring,    Transactd::Ft_wzstring)
  if (isUtf16leSupport(db))
    doTestStringFilter( db, 2, 'myvarchar',  Transactd::Ft_myvarchar,  Transactd::Ft_mywvarchar)
  else
    doTestStringFilter( db, 2, 'myvarchar',  Transactd::Ft_myvarchar,  Transactd::Ft_myvarchar)
  end
  doTestStringFilter(   db, 3, 'mytext',     Transactd::Ft_mytext,     Transactd::Ft_myblob)
end

def testDropDatabaseStringFilter(db)
  db.open(URL_SF, 0, 0)
  expect(db.stat()).to eq 0
  db.drop()
  expect(db.stat()).to eq 0
end


describe Transactd do
  before :each do
    @db = Transactd::Database.createObject()
  end
  after :each do
    @db.close()
    @db = nil
  end
  it 'create database' do
    testCreateDatabase(@db)
  end
  it 'create table' do
    testCreateDatabase(@db)
    testCreateTable(@db)
  end
  it 'open table' do
    testOpenTable(@db)
  end
  it 'version' do
    testVersion(@db)
  end
  it 'insert' do
    testInsert(@db)
  end
  it 'find' do
    testFind(@db)
  end
  it 'findNext' do
    testFindNext(@db)
  end
  it 'get percentage' do
    testGetPercentage(@db)
  end
  it 'move percentage' do
    testMovePercentage(@db)
  end
  it 'get equal' do
    testGetEqual(@db)
  end
  it 'get next' do
    testGetNext(@db)
  end
  it 'get previous' do
    testGetPrevious(@db)
  end
  it 'get greater' do
    testGetGreater(@db)
  end
  it 'get less than' do
    testGetLessThan(@db)
  end
  it 'get first' do
    testGetFirst(@db)
  end
  it 'get last' do
    testGetLast(@db)
  end
  it 'move position' do
    testMovePosition(@db)
  end
  it 'update' do
    testUpdate(@db)
  end
  it 'snapshot' do
    testSnapShot(@db)
  end
  it 'send conflict error' do
    testConflict(@db)
  end
  it 'transaction' do
    testTransactionLock(@db)
  end
  it 'insert2' do
    testInsert2(@db)
  end
  it 'delete' do
    testDelete(@db)
  end
  it 'set owner' do
    testSetOwner(@db)
  end
  it 'drop index' do
    testDropIndex(@db)
  end
  it 'drop database' do
    testDropDatabase(@db)
  end
  it 'login' do
    testLogin(@db)
  end
end

describe Transactd, 'var tables' do
  before :each do
    @db = Transactd::Database.createObject()
  end
  after :each do
    @db.close()
    @db = nil
  end
  it 'create var database' do
    testCreateDatabaseVar(@db)
  end
  it 'set kanji char to field' do
    testVarField(@db)
  end
  it 'insert kanji char to field' do
    testVarInsert(@db)
  end
  it 'read kanji char from field' do
    testVarRead(@db)
  end
  it 'filter' do
    testFilterVar(@db)
  end
  it 'drop var database' do
    testDropDatabaseVar(@db)
  end
end

describe Transactd, 'StringFilter' do
  before :each do
    @db = Transactd::Database.createObject()
  end
  after :each do
    @db.close()
    @db = nil
  end
  it 'string filter' do
    testStringFilter(@db)
  end
  it 'drop database' do
    testDropDatabaseStringFilter(@db)
  end
end

describe Transactd, 'convert' do
  if !IS_WINDOWS
    it 'convert char' do
      dummy = ''
      u8 = '123'
      mbcArray = '123'.encode('Shift_JIS').bytes.to_a # bites Shift-JIS '漢字'
      ret = Transactd::u8tombc(u8, -1, dummy, 256)
      mbc = ret[1].force_encoding('Shift_JIS')
      expect(mbc.bytes.to_a).to eq mbcArray
      
      u8 = '漢字'
      mbcArray = '漢字'.encode('Shift_JIS').bytes.to_a # bites Shift-JIS '漢字'
      ret = Transactd::u8tombc(u8, -1, dummy, 256)
      mbc = ret[1].force_encoding('Shift_JIS')
      expect(mbc.bytes.to_a).to eq mbcArray
      
      u8Array = '漢字'.encode('UTF-8').bytes.to_a # bites UTF-8 '漢字'
      ret = Transactd::mbctou8(mbc, -1, dummy , 256)
      u8 = ret[1].force_encoding('UTF-8')
      expect(u8.bytes.to_a).to eq u8Array
    end
  end
end
