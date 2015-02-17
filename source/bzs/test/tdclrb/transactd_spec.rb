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

require 'rbconfig'
IS_WINDOWS = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/)

def getHost()
  hostname = 'localhost/'
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
DBNAME_QT = 'querytest'
TABLENAME = 'user'
PROTOCOL = 'tdap://'
BDFNAME = '?dbfile=test.bdf'
URL = PROTOCOL + HOSTNAME + DBNAME + BDFNAME
URL_VAR = PROTOCOL + HOSTNAME + DBNAME_VAR + BDFNAME
URL_SF = PROTOCOL + HOSTNAME + DBNAME_SF + BDFNAME
URL_QT = PROTOCOL + HOSTNAME + DBNAME_QT + BDFNAME
FDI_ID = 0
FDI_NAME = 1
FDI_GROUP = 2
FDI_NAMEW = 2

BULKBUFSIZE = 65535 - 1000
TEST_COUNT = 20000
FIVE_PERCENT_OF_TEST_COUNT = TEST_COUNT / 20
ALLOWABLE_ERROR_DISTANCE_IN_ESTIMATE_COUNT = TEST_COUNT / 4

NO_RECORD_ID = 5

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
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
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
  
  fd = dbdef.insertField(1, 2)
  fd.setName('select')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  
  fd = dbdef.insertField(1, 3)
  fd.setName('in')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  
  kd = dbdef.insertKey(1, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  
  # group table
  td = Transactd::Tabledef.new()
  td.setTableName('group')
  td.setFileName('group.dat')
  table_id = 2
  td.id = table_id
  td.pageSize = 2048
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  
  fd = dbdef.insertField(table_id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(table_id)
  expect(dbdef.stat()).to eq 0
  
  fd = dbdef.insertField(table_id, 1)
  fd.setName('name')
  fd.type = Transactd::Ft_zstring
  fd.len = 33
  dbdef.updateTableDef(table_id)
  expect(dbdef.stat()).to eq 0
  
  kd = dbdef.insertKey(table_id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  dbdef.updateTableDef(table_id)
  expect(dbdef.stat()).to eq 0
end

def testOpenTable(db)
  testOpenDatabase(db)
  tb = db.openTable(TABLENAME)
  expect(db.stat()).to eq 0
  return tb
end

def testClone()
  db = Transactd::Database.new()
  db.open(URL)
  expect(db.stat()).to eq 0
  expect(db.isOpened()).to eq true
  db2 = db.clone
  expect(db2.stat).to eq 0
  expect(db2.isOpened()).to eq true
  db2.close
  expect(db2.stat).to eq 0
  expect(db2.isOpened()).to eq false
  db2 = nil
  expect(db.stat).to eq 0
  expect(db.isOpened()).to eq true
  db.close
end

def testVersion()
  db = Transactd::Database.new()
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
  my5x = (server_ver.majorVersion == 5) && (server_ver.minorVersion >= 5)
  maria10 = (server_ver.majorVersion == 10) && (server_ver.minorVersion == 0)
  expect(my5x || maria10).to be true
  expect(server_ver.type.chr).to eq 'M'
  expect(engine_ver.majorVersion.to_s).to eq Transactd::TRANSACTD_VER_MAJOR.to_s
  expect(engine_ver.minorVersion.to_s).to eq Transactd::TRANSACTD_VER_MINOR.to_s
  expect(engine_ver.type.chr).to eq 'T'
  db.close()
end

def testInsert()
  db = Transactd::Database.new()
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
  db.close()
end

def testFind()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  tb.setKeyNum(0)
  tb.clearBuffer()
  tb.setFilter('id >= 10 and id < ' + TEST_COUNT.to_s, 1, 0)
  v = 10
  tb.setFV(FDI_ID, v)
  tb.find(Transactd::Table::FindForword)
  i = v
  while i < TEST_COUNT do
    expect(tb.stat()).to eq 0
    break unless tb.stat() == 0
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
    break unless tb.stat() == 0
    expect(tb.getFVint(FDI_ID)).to eq i
    tb.findPrev(true) # 11 - 19
    i = i - 1
  end
  # out of filter range (EOF)
  tb.clearBuffer()
  v = TEST_COUNT
  tb.setFV(FDI_ID, v)
  tb.find(Transactd::Table::FindForword)
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  tb.close()
  db.close()
end

def testFindNext()
  db = Transactd::Database.new()
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
    break unless tb.stat() == 0
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq i
  end
  tb.close()
  db.close()
end

def testFindIn()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.setKeyNum(0)
  tb.clearBuffer()
  q = Transactd::Query.new()
  q.addSeekKeyValue('10', true)
  q.addSeekKeyValue('300000')
  q.addSeekKeyValue('50')
  q.addSeekKeyValue('-1')
  q.addSeekKeyValue('80')
  q.addSeekKeyValue('5000')
  
  tb.setQuery(q)
  expect(tb.stat()).to eq 0
  tb.find()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).to eq 10
  tb.findNext()
  expect(tb.stat()).to eq Transactd::STATUS_NOT_FOUND_TI
  
  msg = tb.keyValueDescription()
  expect(msg).to eq "table:user\nstat:4\nid = 300000\n"
 
  tb.findNext()
  expect(tb.getFVint(FDI_ID)).to eq 50
  tb.findNext()
  expect(tb.stat()).to eq Transactd::STATUS_NOT_FOUND_TI
  
  msg = tb.keyValueDescription()
  expect(msg).to eq "table:user\nstat:4\nid = -1\n"
 
  tb.findNext()
  expect(tb.getFVint(FDI_ID)).to eq 80
  tb.findNext()
  expect(tb.getFVint(FDI_ID)).to eq 5000
  tb.findNext()
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  
  # Many params
  1.upto(10000) do |i|
    q.addSeekKeyValue(i.to_s, (i == 1))
  end
  tb.setQuery(q)
  expect(tb.stat()).to eq 0
  
  tb.find()
  i = 0
  while tb.stat() == 0 do
    i = i + 1
    expect(tb.getFVint(FDI_ID)).to eq i
    tb.findNext(true)
  end
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  expect(i).to eq 10000
  
  # LogicalCountLimit
  q.select('id')
  tb.setQuery(q)
  
  tb.find()
  i = 0
  while tb.stat() == 0 do
    i = i + 1
    expect(tb.getFVint(FDI_ID)).to eq i
    tb.findNext(true)
  end
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  expect(i).to eq 10000
  
  tb.close()
  db.close()
end

def testGetPercentage()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.clearBuffer()
  vv = TEST_COUNT / 2 + 1
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.stat()).to eq 0
  per = tb.getPercentage()
  expect((5000 - per).abs).to be < 500 # 500 = 5%
  tb.close()
  db.close()
end

def testMovePercentage()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekByPercentage(5000) # 50%
  expect(tb.stat()).to eq 0
  v = tb.getFVint(FDI_ID)
  expect(tb.stat()).to eq 0
  expect((TEST_COUNT / 2 + 1 - v).abs).to be < FIVE_PERCENT_OF_TEST_COUNT
  tb.close()
  db.close()
end

def testGetEqual()
  db = Transactd::Database.new()
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
  db.close()
end

def testGetNext()
  db = Transactd::Database.new()
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
    break unless tb.getFVint(FDI_ID) == i
  end
  db.endSnapshot()
  tb.close()
  db.close()
end

def testGetPrevious()
  db = Transactd::Database.new()
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
    break unless tb.getFVint(FDI_ID) == i
  end
  tb.seekPrev()
  expect(tb.getFVstr(FDI_NAME)).to eq 'kosaka'
  db.endSnapshot()
  # without snapshot
  vv = TEST_COUNT + 1
  tb.clearBuffer()
  tb.setFV(FDI_ID, vv)
  tb.seek()
  expect(tb.getFVint(FDI_ID)).to eq vv
  for i in TEST_COUNT.downto(2) do
    tb.seekPrev()
    expect(tb.getFVint(FDI_ID)).to eq i
    break unless tb.getFVint(FDI_ID) == i
  end
  tb.seekPrev()
  expect(tb.getFVstr(FDI_NAME)).to eq 'kosaka'
  tb.close()
  db.close()
end

def testGetGreater()
  db = Transactd::Database.new()
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
  db.close()
end

def testGetLessThan()
  db = Transactd::Database.new()
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
  db.close()
end

def testGetFirst()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekFirst()
  expect(tb.getFVstr(FDI_NAME)).to eq 'kosaka'
  tb.close()
  db.close()
end

def testGetLast()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.clearBuffer()
  tb.seekLast()
  expect(tb.getFVint(FDI_ID)).to eq TEST_COUNT + 2
  tb.close()
  db.close()
end

def testMovePosition()
  db = Transactd::Database.new()
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
  db.close()
end

def testUpdate()
  db = Transactd::Database.new()
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
  db.close()
end

def testSnapshot()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  tbg = db.openTable('group')
  expect(db.stat()).to eq 0
  expect(tbg).not_to be nil
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME , true)
  expect(db2.stat()).to eq 0
  tb2 = testOpenTable(db2)
  expect(tb2).not_to be nil
  tbg2 = db2.openTable('group')
  expect(db2.stat()).to eq 0
  expect(tbg2).not_to be nil
  
  # No locking repeatable read
  # ----------------------------------------------------
  db.beginSnapshot() # CONSISTENT_READ is default
  expect(db.stat()).to eq 0
  db.beginTrn()
  expect(db.stat()).to eq Transactd::STATUS_ALREADY_INSNAPSHOT
  
  tb.setKeyNum(0)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  firstValue = tb.getFVint(FDI_NAME)
  tb.seekNext()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).to eq 2
  tbg.seekFirst()
  expect(tbg.stat()).to eq Transactd::STATUS_EOF
  expect(tbg.recordCount(false)).to eq 0
  
  # Change data on 2 tables by another connection
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, tb2.getFVint(FDI_ID) + 1)
  tb2.update() # Change success
  expect(tb2.stat()).to eq 0
  tbg2.setFV(FDI_ID, 1)
  tbg2.setFV(FDI_NAME, 'ABC')
  tbg2.insert()
  expect(tbg2.stat()).to eq 0
  
  # in-snapshot repeatable read check same value
  tb.seekFirst()
  secondValue = tb.getFVint(FDI_NAME)
  expect(tb.stat()).to eq 0
  expect(secondValue).to eq firstValue
  
  tbg.seekFirst()
  expect(tbg.stat()).to eq Transactd::STATUS_EOF
  expect(tbg.recordCount(false)).to eq 0
  
  # in-snapshot update
  tb.update()
  expect(tb.stat()).to eq Transactd::STATUS_INVALID_LOCKTYPE
  
  # in-snapshot insert
  tb.setFV(FDI_ID, 0)
  tb.insert()
  expect(tb.stat()).to eq Transactd::STATUS_INVALID_LOCKTYPE
  
  # phantom read
  tb2.setFV(FDI_ID, 29999)
  tb2.insert()
  expect(tb2.stat()).to eq 0
  tb.setFV(FDI_ID, 29999)
  tb.seek()
  expect(tb.stat()).to eq Transactd::STATUS_NOT_FOUND_TI
  
  # clean up
  tb2.setFV(FDI_ID, 29999)
  tb2.seek()
  expect(tb2.stat()).to eq 0
  tb2.del()
  expect(tb2.stat()).to eq 0
  
  db.endSnapshot()
  expect(db.stat()).to eq 0
  
  # After snapshot, db can read new versions.
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).to eq 1
  tbg.seekFirst()
  expect(tbg.stat()).to eq 0
  expect(tbg.recordCount(false)).to eq 1
  
  # gap lock
  db.beginSnapshot(Transactd::MULTILOCK_GAP_SHARE)
  tb.seekLast() # id = 30000
  expect(tb.stat()).to eq 0
  tb.seekPrev() # id = 20002
  expect(tb.stat()).to eq 0
  tb.seekPrev() # id = 20001
  expect(tb.stat()).to eq 0
  
  tb2.setFV(FDI_ID, 29999)
  tb2.insert()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  db.endSnapshot()
  
  # gap lock
  db.beginSnapshot(Transactd::MULTILOCK_NOGAP_SHARE)
  tb.seekLast() # id = 30000
  expect(tb.stat()).to eq 0
  tb.seekPrev() # id = 20002
  expect(tb.stat()).to eq 0
  tb.seekPrev() # id = 20001
  expect(tb.stat()).to eq 0
  
  tb2.setFV(FDI_ID, 20002)
  tb2.seek(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  tb2.seekLast(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  db.endSnapshot()
  
  tbg2.close()
  tbg.close()
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

# isolation Level ISO_REPEATABLE_READ
def testTransactionLockRepeatable()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  expect(db2.stat()).to eq 0
  tb2 = testOpenTable(db2)
  expect(tb2).not_to be nil
  
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  # Test Invalid operation
  db.beginSnapshot()
  expect(db.stat()).to eq Transactd::STATUS_ALREADY_INTRANSACTION
  
  # ----------------------------------------------------
  # Test Read with lock
  # ----------------------------------------------------
  # lock(X) the first record
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  
  # Add lock(X) the second record
  tb.seekNext()
  
  # No transaction user can read allways. Use consistent_read
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  
  tb2.seekNext()
  expect(tb2.stat()).to eq 0
  
  # The second transaction user can not lock same record.
  db2.beginTrn()
  tb2.setKeyNum(0)
  
  # Try lock(X)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test single record lock and Transaction lock
  # ----------------------------------------------------
  # lock(X) non-transaction
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  
  # Try lock(X)
  tb.seekFirst()
  expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # Remove lock(X)
  tb2.seekFirst()
  
  # Retry lock(X)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  
  tb.setFV(FDI_NAME, 'ABC')
  tb.update()
  expect(tb.stat()).to eq 0
  
  # No transaction user can read allways. Use consistent_read
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  expect(tb2.getFVstr(FDI_NAME)).not_to eq 'ABC'
  
  # ----------------------------------------------------
  # Test Transaction lock and Transaction lock
  # ----------------------------------------------------
  db2.beginTrn()
  expect(db2.stat()).to eq 0
  
  # try lock(X)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # Try unlock updated record. Can not unlock updated record.
  tb.unlock()
  
  # try lock(X)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test phantom read
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  
  # read last row
  tb.seekLast() # lock(X) last id = 30000
  expect(tb.stat()).to eq 0
  tb.seekPrev() # Add lock(X)
  expect(tb.stat()).to eq 0
  last2 = tb.getFVint(FDI_ID)
  
  # insert test row
  tb2.setFV(FDI_ID, 29999)
  tb2.insert() # Can not insert by gap lock
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  tb.seekLast()
  expect(tb.stat()).to eq 0
  tb.seekPrev()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).to eq last2
  db.endTrn()
  
  # ----------------------------------------------------
  # Test use shared lock option
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  
  db2.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db2.stat()).to eq 0
  
  tb.seekLast(Transactd::ROW_LOCK_S)
  expect(tb.stat()).to eq 0
  tb2.seekLast(Transactd::ROW_LOCK_S)
  expect(tb2.stat()).to eq 0
  
  tb.seekPrev() # Lock(X)
  expect(tb.stat()).to eq 0
  
  tb2.seekPrev(Transactd::ROW_LOCK_S)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  tb.seekPrev(Transactd::ROW_LOCK_S)
  expect(tb.stat()).to eq 0
  id = tb.getFVint(FDI_ID)
  
  tb2.setFV(FDI_ID, id)
  tb2.seek(Transactd::ROW_LOCK_S)
  expect(tb2.stat()).to eq 0
  
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test use shared lock option
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(0).to eq db.stat()
  
  db2.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(0).to eq db2.stat()
  
  tb.seekLast(Transactd::ROW_LOCK_S)
  expect(0).to eq tb.stat()
  tb2.seekLast(Transactd::ROW_LOCK_S)
  expect(0).to eq tb2.stat()
  
  tb.seekPrev() # Lock(X)
  expect(0).to eq tb.stat()
  
  tb2.seekPrev(Transactd::ROW_LOCK_S)
  expect(Transactd::STATUS_LOCK_ERROR).to eq tb2.stat()
  
  tb.seekPrev(Transactd::ROW_LOCK_S)
  expect(0).to eq tb.stat()
  id = tb.getFVint(FDI_ID)
  
  tb2.setFV(FDI_ID, id)
  tb2.seek(Transactd::ROW_LOCK_S)
  expect(0).to eq tb2.stat()
  
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test Abort
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  
  # lock(X)
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
  
  # ----------------------------------------------------
  # Test Query and locks Multi record lock
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_REPEATABLE_READ)
  expect(db.stat()).to eq 0
  
  # Test find records are lock.
  q = Transactd::Query.new()
  q.where('id', '<=', 15).and_('id', '<>', 13).reject(0xFFFF)
  tb.setQuery(q)
  tb.setFV(FDI_ID, 12)
  tb.find()
  while (tb.stat() == 0) do
    tb.findNext()
  end
  expect(tb.getFVint(FDI_ID)).to eq 15
  
  # all records locked
  for i in 12..16 do
    tb2.setFV(FDI_ID, i)
    tb2.seek(Transactd::ROW_LOCK_X)
    expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  end
  db.endTrn()
  
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

def testTransactionLockReadCommited()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  expect(db2.stat()).to eq 0
  tb2 = testOpenTable(db2)
  expect(tb2).not_to be nil
  
  # ----------------------------------------------------
  # Test single record lock Transaction and read
  # ----------------------------------------------------
  db.beginTrn(Transactd::SINGLELOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  # Test Invalid operation
  db.beginSnapshot()
  expect(db.stat()).to eq Transactd::STATUS_ALREADY_INTRANSACTION
  
  tb.setKeyNum(0)
  tb.seekFirst() # lock(X)
  expect(tb.stat()).to eq 0
  
  # Try lock(X)
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # consistent read
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  
  # Unlock first record. And lock(X) second record
  tb.seekNext()
  
  # test unlocked first record
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.update()
  expect(tb2.stat()).to eq 0
  
  # The second record, consistent read
  tb2.seekNext()
  expect(tb2.stat()).to eq 0
  # Try lock(X) whith lock(IX)
  tb2.update()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # ----------------------------------------------------
  # Test single record lock Transaction and Transaction lock
  # ----------------------------------------------------
  db2.beginTrn()
  # Try lock(X)
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  # Try lock(X)
  tb2.seekNext()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test multi record lock Transaction and non-transaction read
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  # lock(X) the first record
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  
  # Add lock(X) the second record
  tb.seekNext()
  
  # No transaction user read can read allways. Use consistent_read 
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  
  tb2.seekNext()
  expect(tb2.stat()).to eq 0
  
  # ----------------------------------------------------
  # Test unlock
  # ----------------------------------------------------
  tb2.seekFirst()
  tb2.seekNext(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  tb.unlock()
  # retry seekNext. Before operation is failed but do not lost currency.
  tb2.seekNext(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  tb2.seekNext()
  # ----------------------------------------------------
  # Test undate record unlock
  # ----------------------------------------------------
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.stat()).to eq 0
  tb.update()
  expect(tb.stat()).to eq 0
  tb.unlock() # Can not unlock updated record
  expect(tb.stat()).to eq 0
  tb2.seekFirst()
  tb2.seekNext(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # ----------------------------------------------------
  # Test undate record unlock
  # ----------------------------------------------------
  db2.beginTrn()
  expect(db2.stat()).to eq 0
  
  # Try lock(X)
  tb2.seekFirst()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.endTrn()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test multi record lock Transaction and non-transaction record lock
  # ----------------------------------------------------
  # lock(X) non-transaction
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  
  db.beginTrn(Transactd::SINGLELOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  # Try lock(X)
  tb.seekFirst()
  expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # Remove lock(X)
  tb2.seekFirst()
  
  # Retry lock(X)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  
  # update in transaction
  tb.setFV(FDI_NAME, 'ABC')
  tb.update()
  expect(tb.stat()).to eq 0
  
  # move from first record.
  tb.seekNext()
  
  # No transaction read can read allways. Use consistent_read 
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.update()
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  db.endTrn()
  
  # ----------------------------------------------------
  # Test phantom read
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  # read last row
  tb.seekLast() # lock(X) last id = 30000
  expect(tb.stat()).to eq 0
  tb.seekPrev() # Add lock(X)
  expect(tb.stat()).to eq 0
  last2 = tb.getFVint(FDI_ID)
  
  # insert test row
  tb2.setFV(FDI_ID, 29999)
  tb2.insert()
  expect(tb2.stat()).to eq 0
  
  tb.seekLast()
  expect(tb.stat()).to eq 0
  tb.seekPrev()
  expect(tb.stat()).to eq 0
  expect(tb.getFVint(FDI_ID)).not_to eq last2
  db.endTrn()
  
  # cleanup
  tb2.del() # last id = 29999
  expect(tb.stat()).to eq 0
  
  # ----------------------------------------------------
  # Abort test
  # ----------------------------------------------------
  db.beginTrn(Transactd::SINGLELOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_NAME, 'EFG')
  tb.update()
  expect(tb.stat()).to eq 0
  
  tb.seekNext()
  db.abortTrn()
  tb2.setKeyNum(0)
  tb2.seekFirst()
  expect(tb2.getFVstr(FDI_NAME)).to eq 'ABC'
  
  # ----------------------------------------------------
  # Test Query and locks Single record lock
  # ----------------------------------------------------
  db.beginTrn(Transactd::SINGLELOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  # Test find last record locked
  q = Transactd::Query.new()
  q.where('id', '<=', '100')
  tb.setQuery(q)
  tb.setFV(FDI_ID, 1)
  tb.find()
  while (tb.stat() == 0) do
    tb.findNext()
  end
  expect(tb.getFVint(FDI_ID)).to eq 100
  
  # find read last is record of id = 101.
  # Would be difficult to identify the last 
  #  access to records at SINGLELOCK_READ_COMMITED.
  # No match records are unlocked.
  tb2.setFV(FDI_ID, 100)
  tb2.seek(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_ID, 101)
  tb2.seek(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  tb2.unlock()
  db.endTrn()
  
  # ----------------------------------------------------
  # Test Query and locks Multi record lock
  # ----------------------------------------------------
  db.beginTrn(Transactd::MULTILOCK_READ_COMMITED)
  expect(db.stat()).to eq 0
  
  # Test find records are lock.
  q.reset().where('id', '<=', 15).and_('id', '<>', 13).reject(0xFFFF)
  tb.setQuery(q)
  tb.setFV(FDI_ID, 12)
  tb.find()
  while (tb.stat() == 0) do
    tb.findNext()
  end
  expect(tb.getFVint(FDI_ID)).to eq 15
  
  for i in 12..16 do
    tb2.setFV(FDI_ID, i)
    tb2.seek(Transactd::ROW_LOCK_X)
    if ((i == 16) || (i == 13)) then
      expect(tb2.stat()).to eq 0
    else
      expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
    end
  end
  db.endTrn()
  
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

def testRecordLock()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  expect(db2.stat()).to eq 0
  tb2 = testOpenTable(db2)
  expect(tb2).not_to be nil
  
  tb.setKeyNum(0)
  tb2.setKeyNum(0)
  
  # Single record lock
  tb.seekFirst(Transactd::ROW_LOCK_X) # lock(X)
  expect(tb.stat()).to eq 0
  tb2.seekFirst() # Use consistent_read
  expect(tb2.stat()).to eq 0
  
  tb2.seekFirst(Transactd::ROW_LOCK_X) # Try lock(X) single
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # try consistent_read. Check ended that before auto transaction
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  
  tb2.seekNext(Transactd::ROW_LOCK_X) # lock(X) second
  expect(tb2.stat()).to eq 0
  
  tb2.seekNext(Transactd::ROW_LOCK_X) # lock(X) third, second lock freed
  expect(tb2.stat()).to eq 0
  
  tb.seekNext() # nobody lock second. But REPEATABLE_READ tb2 lock all(no unlock)
  if (db.trxIsolationServer() == Transactd::SRV_ISO_REPEATABLE_READ)
     expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  else
     expect(tb.stat()).to eq 0
  end
  tb.seekNext(Transactd::ROW_LOCK_X) # Try lock(X) third
  expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  
  # Update test change third with lock(X)
  tb2.setFV(FDI_NAME, 'The 3rd')
  tb2.update() # auto trn commit and unlock all locks
  expect(tb2.stat()).to eq 0
  tb2.seekNext(Transactd::ROW_LOCK_X) # lock(X) 4th
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, 'The 4th')
  tb2.update() # auto trn commit and unlock all locks
  
  # Test unlock all locks, after update
  tb.seekFirst(Transactd::ROW_LOCK_X) # lock(X) first
  expect(tb2.stat()).to eq 0
  tb.seekNext(Transactd::ROW_LOCK_X) # lock(X) second
  expect(tb2.stat()).to eq 0
  tb.seekNext(Transactd::ROW_LOCK_X) # lock(X) third
  expect(tb2.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'The 3rd'
  
  # Test Insert, After record lock  operation
  tb.setFV(FDI_ID, 21000)
  tb.insert()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_ID, 21000)
  tb.seek()
  expect(tb.stat()).to eq 0
  tb.del()
  expect(tb.stat()).to eq 0
  
  # --------- Unlock test ------------------------------
  # 1 unlock()
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq 0
  
  tb.unlock()
  
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  tb2.unlock()
  
  # 2 auto tran ended
  tb3 = testOpenTable(db2)
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  
  tb3.seekLast() #This operation is another table handle, then auto tran ended
  expect(tb3.stat()).to eq 0
  
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq 0
  tb.unlock()
  
  # begin trn
  tb3.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb3.stat()).to eq 0
  
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.beginTrn()
  
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq 0
  db2.endTrn()
  tb.unlock()
  # begin snapshot
  tb3.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb3.stat()).to eq 0
  
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
  db2.beginSnapshot()
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq 0
  db2.endSnapshot()
  tb.unlock()
  # close Table
  tb.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb.stat()).to eq 0
  
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq Transactd::STATUS_LOCK_ERROR
  tb.close()
  tb2.seekFirst(Transactd::ROW_LOCK_X)
  expect(tb2.stat()).to eq 0
  tb2.unlock()
  # --------- End Unlock test --------------------------
  
  # --------- Invalid lock type test ----------------
  tb2.seekFirst(Transactd::ROW_LOCK_S)
  expect(tb2.stat()).to eq Transactd::STATUS_INVALID_LOCKTYPE
  
  tb3.close()
  tb2.close()
  db2.close()
  db.close()
end

def testConflict()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME , true)
  expect(db2.stat()).to eq 0
  expect(tb).not_to be nil
  tb2 = testOpenTable(db2)
  expect(tb2).not_to be nil
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
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

def testExclusive()
  # db mode exclusive
  db = Transactd::Database.new()
  # ------------------------------------------------------
  # database WRITE EXCLUSIVE
  # ------------------------------------------------------
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_EXCLUSIVE)
  expect(db.stat()).to eq 0
  tb = db.openTable(TABLENAME)
  expect(db.stat()).to eq 0
  
  # Can not open database from other connections.
  db2 = Transactd::Database.new()
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  expect(db2.stat()).to eq 0
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF)
  # database open error. Check database::stat()
  expect(db2.stat()).to eq Transactd::STATUS_CANNOT_LOCK_TABLE
  tb.close()
  db.close()
  db2.close()
  
  # ------------------------------------------------------
  # database READ EXCLUSIVE
  # ------------------------------------------------------
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_READONLY_EXCLUSIVE)
  expect(db.stat()).to eq 0
  tb = db.openTable(TABLENAME, Transactd::TD_OPEN_READONLY_EXCLUSIVE)
  expect(db.stat()).to eq 0
  
  # Read only open
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF)
  expect(db2.stat()).to eq 0
  db2.close()
  
  # Normal open
  db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db2.stat()).to eq 0
  db2.close()
  
  # Write Exclusive open
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_EXCLUSIVE)
  expect(db2.stat()).to eq Transactd::STATUS_CANNOT_LOCK_TABLE
  db2.close()
  
  # Read Exclusive open
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_READONLY_EXCLUSIVE)
  expect(db2.stat()).to eq 0
  db2.close()
  tb.close()
  db.close()
  
  # ------------------------------------------------------
  # Normal and Exclusive open tables mix use
  # ------------------------------------------------------
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  tb = db.openTable(TABLENAME, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  db2.open(URL, Transactd::TYPE_SCHEMA_BDF)
  expect(db2.stat()).to eq 0
  
  tb2 = db.openTable('group', Transactd::TD_OPEN_EXCLUSIVE)
  expect(db.stat()).to eq 0
  
  # Check tb2 Exclusive
  tb3 = db2.openTable('group', Transactd::TD_OPEN_NORMAL)
  expect(db2.stat()).to eq Transactd::STATUS_CANNOT_LOCK_TABLE
  for i in 1..4 do
    tb2.setFV(FDI_ID, i + 1)
    tb2.setFV(FDI_NAME, i + 1)
    tb2.insert()
    expect(tb2.stat()).to eq 0
  end
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb2.seekLast()
  expect(tb2.stat()).to eq 0
  tb.seekLast()
  expect(tb.stat()).to eq 0
  # Normal close first
  tb.close()
  tb2.seekLast()
  expect(tb2.stat()).to eq 0
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  
  # Reopen Normal
  tb = db.openTable('user')
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb2.seekLast()
  expect(tb2.stat()).to eq 0
  tb.seekLast()
  expect(tb.stat()).to eq 0
  # Exclusive close first
  tb2.close()
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.seekLast()
  expect(tb.stat()).to eq 0
  
  tb2.close()
  
  # ------------------------------------------------------
  # Normal and Exclusive open tables mix transaction
  # ------------------------------------------------------
  tb2 = db.openTable('group', Transactd::TD_OPEN_EXCLUSIVE)
  expect(db.stat()).to eq 0
  # Check tb2 Exclusive
  tb3 = db2.openTable('group', Transactd::TD_OPEN_NORMAL)
  expect(db2.stat()).to eq Transactd::STATUS_CANNOT_LOCK_TABLE
  
  db.beginTrn()
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.setFV(FDI_NAME, 'mix trn')
  tb.update()
  expect(tb.stat()).to eq 0
  
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, 'first mix trn tb2')
  tb2.update()
  expect(tb2.stat()).to eq 0
  
  tb2.seekNext()
  tb2.setFV(FDI_NAME, 'second mix trn tb2')
  tb2.update()
  expect(tb2.stat()).to eq 0
  db.endTrn()
  tb2.seekFirst()
  v = tb2.getFVstr(FDI_NAME)
  expect(v).to eq 'first mix trn tb2'
  tb2.seekNext()
  v = tb2.getFVstr(FDI_NAME)
  expect(v).to eq 'second mix trn tb2'
  
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

def testMultiDatabase()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  expect(tb).not_to be nil
  db2 = Transactd::Database.new()
  testOpenDatabase(db2)
  expect(db2.stat()).to eq 0
  tb2 = db2.openTable('group')
  expect(db2.stat()).to eq 0
  expect(tb2).not_to be nil
  
  db.beginTrn()
  db2.beginTrn()
  
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  v = tb.getFVstr(FDI_NAME)
  tb.setFV(FDI_NAME, 'MultiDatabase')
  tb.update()
  
  tb2.seekFirst()
  expect(tb2.stat()).to eq 0
  tb2.setFV(FDI_NAME, 'MultiDatabase')
  tb2.update()
  expect(tb2.stat()).to eq 0
  db2.endTrn()
  db.abortTrn()
  
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  v2 = tb.getFVstr(FDI_NAME)
  expect(v).to eq v2
  
  tb2.close()
  tb.close()
  db2.close()
  db.close()
end

def testMissingUpdate()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  # Lock last record and insert to next of it
  tb.setFV(FDI_ID, 300000)
  tb.seekLessThan(false, Transactd::ROW_LOCK_X)
  if (tb.stat() == 0) then
    # Get lock(X) same record in parallel.
    w = Thread.new {
      db2 = Transactd::Database.new()
      db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
      expect(db2.stat()).to eq 0
      db2.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
      expect(db2.stat()).to eq 0
      tb2 = db2.openTable('user')
      expect(db2.stat()).to eq 0
      tb2.setFV(FDI_ID, 300000)
      tb2.seekLessThan(false, Transactd::ROW_LOCK_X)
      v2 = tb2.getFVint(FDI_ID)
      tb2.unlock()
      tb2.close()
      db2.close()
      v2
    }
    sleep(0.5)
    v = tb.getFVint(FDI_ID)
    v = v + 1
    tb.setFV(FDI_ID, v)
    tb.insert()
    v2 = w.join().value
    if (db.trxIsolationServer() == Transactd::SRV_ISO_REPEATABLE_READ)
      # $tb can not insert because $tb2 got gap lock with SRV_ISO_REPEATABLE_READ.
      # It is deadlock!
      expect(tb.stat()).to eq Transactd::STATUS_LOCK_ERROR
    else
      # When SRV_ISO_READ_COMMITED set, $tb2 get lock after $tb->insert.
      # But this is not READ_COMMITED !
      expect(tb.stat()).to eq 0
      expect(v2).to eq (v - 1);
      # cleanup
      tb.setFV(FDI_ID, v)
      tb.seek()
      expect(tb.stat()).to eq 0
      tb.del()
      expect(tb.stat()).to eq 0
    end
  end
  # Lock last record and delete it
  tb.setFV(FDI_ID, 300000)
  tb.seekLessThan(false, Transactd::ROW_LOCK_X)
  if (tb.stat() == 0) then
    # Get lock(X) same record in parallel.
    w = Thread.new {
      db2 = Transactd::Database.new()
      db2.connect(PROTOCOL + HOSTNAME + DBNAME, true)
      expect(db2.stat()).to eq 0
      db2.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
      expect(db2.stat()).to eq 0
      tb2 = db2.openTable('user')
      expect(db2.stat()).to eq 0
      tb2.setFV(FDI_ID, 300000)
      tb2.seekLessThan(false, Transactd::ROW_LOCK_X)
      expect(tb2.stat()).to eq 0
      v2 = tb2.getFVint(FDI_ID)
      tb2.unlock()
      tb2.close()
      db2.close()
      v2
    }
    sleep(0.5)
    v = tb.getFVint(FDI_ID)
    tb.del()
    v2 = w.join().value
    expect(tb.stat()).to eq 0
    expect(v).not_to eq v2
  end
  tb.close()
  db.close()
end

def testInsert2()
  db = Transactd::Database.new()
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
  db.close()
end

def testDelete()
  expected_count = 20003
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  # estimate count
  count = tb.recordCount(true)
  is_valid_count = ((count - expected_count).abs < 5000)
  expect(is_valid_count).to be true
  if !is_valid_count
    puts "true record count = #{expected_count.to_s} and estimate recordCount count = #{count.to_s}"
  end
  expect(tb.recordCount(false)).to eq expected_count # true count
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
  db.close()
end

def testSetOwner()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.setOwnerName('ABCDEFG')
  expect(tb.stat()).to eq 0
  tb.clearOwnerName()
  expect(tb.stat()).to eq 0
  tb.close()
  db.close()
end

def testDropIndex()
  db = Transactd::Database.new()
  tb = testOpenTable(db)
  tb.dropIndex(false)
  expect(tb.stat()).to eq 0
  tb.close()
  db.close()
end

def testLogin()
  db = Transactd::Database.new()
  db.connect(PROTOCOL + HOSTNAME)
  expect(db.stat()).to eq 0
  if db.stat() == 0
    # second connection
    db2 = Transactd::Database.new()
    db2.connect(PROTOCOL + HOSTNAME, true)
    expect(db2.stat()).to eq 0
    db2.disconnect()
    db.disconnect()
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
  db.close()
  expect(db.stat()).to eq 0
  # true database name
  db.connect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq 0
  if (db.stat() == 0)
    db.disconnect()
    expect(db.stat()).to eq 0
  end
  # invalid database name
  testDropDatabase(db)
  db.disconnect()
  expect(db.stat()).to eq 0
  db.connect(PROTOCOL + HOSTNAME + DBNAME)
  expect(db.stat()).to eq (Transactd::ERROR_NO_DATABASE)
  db.disconnect()
  expect(db.stat()).to eq 1
  db.close()
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

def testCreateDatabaseVar()
  db = Transactd::Database.new()
  db.create(URL_VAR)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    testDropDatabaseVar(db)
    db.create(URL_VAR)
  end
  expect(db.stat()).to eq 0
  if (0 == db.stat())
    db.open(URL_VAR, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    expect(db.stat()).to eq 0
  end
  if (0 == db.stat())
    testCreateVarTable(db, 1, 'user1', Transactd::Ft_myvarchar,   Transactd::CHARSET_CP932)
    testCreateVarTable(db, 2, 'user2', Transactd::Ft_myvarbinary, Transactd::CHARSET_CP932)
    if isUtf16leSupport(db)
      testCreateVarTable(db, 3, 'user3', Transactd::Ft_mywvarchar,    Transactd::CHARSET_CP932)
    end
    testCreateVarTable(db, 4, 'user4', Transactd::Ft_mywvarbinary,  Transactd::CHARSET_CP932)
    testCreateVarTable(db, 5, 'user5', Transactd::Ft_myvarchar,     Transactd::CHARSET_UTF8B4)
    db.close()
    db.open(PROTOCOL + HOSTNAME + DBNAME_VAR + '?dbfile=transactd_schemaname')
    expect(db.stat()).to eq 0
  end
  db.close()
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
  tb.setFV(FDI_NAME, '1234567')
  if (varCharField)
    expect(tb.getFVstr(FDI_NAME)).to eq '123'
  else
    expect(tb.getFVstr(FDI_NAME)).to eq '123456'
  end
  #if IS_WINDOWS
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #else
    expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  #end
  # short string
  tb.setFV(FDI_NAME, '13 ')
  expect(tb.getFVstr(FDI_NAME)).to eq '13 '
  #if IS_WINDOWS
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #else
    expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  #end
  # too long kanji
  if (unicodeField)
    if !IS_WINDOWS
      tb.setFV(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
      if (varCharField)
        expect(tb.getFVstr(FDI_NAME)).to eq 'あいう'
      else
        expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえお'
      end
    end
  else
    tb.setFV(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
    is_valid_value = tb.getFVstr(FDI_NAME) == '0松本'
    expect(is_valid_value).to be true
    puts tb.getFVstr(FDI_NAME) if (!is_valid_value)
  end
  expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  ### Set Wide Get Ansi
  #if IS_WINDOWS
  #  # too long string
  #  tb.setFVW(FDI_NAME, '1234567')
  #  if (varCharField)
  #    expect(tb.getFVstr(FDI_NAME)).to eq '123'
  #  else
  #    expect(tb.getFVstr(FDI_NAME)).to eq '123456'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # short string
  #  tb.setFVW(1, '23 ')
  #  expect(tb.getFVstr(FDI_NAME)).to eq '23 '
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #  # too long kanji
  #  if (unicodeField)
  #    tb.setFVW(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
  #    if (varCharField)
  #      expect(tb.getFVstr(FDI_NAME)).to eq 'あいう'
  #    else
  #      expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえお'
  #    end
  #  else
  #    tb.setFVW(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
  #    expect(tb.getFVstr(FDI_NAME)).to eq '0松本'
  #  end
  #  expect(tb.getFVWstr(FDI_GROUP)).to eq '68'
  #end
  ### Set Ansi Get Ansi
  # too long string
  tb.setFV(FDI_NAME, '1234567')
  if (varCharField)
    expect(tb.getFVstr(FDI_NAME)).to eq '123'
  else
    expect(tb.getFVstr(FDI_NAME)).to eq '123456'
  end
  expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  # short string
  tb.setFV(FDI_NAME, '13 ')
  expect(tb.getFVstr(FDI_NAME)).to eq '13 '
  expect(tb.getFVstr(FDI_GROUP)).to eq '68'
  # too long lanji
  if (unicodeField)
    if !IS_WINDOWS
      tb.setFV(FDI_NAME, 'あいうえお𩸽') # hiragana 'aiueo' kanji 'hokke'
      if (varCharField)
        expect(tb.getFVstr(FDI_NAME)).to eq 'あいう'
      else
        expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえお'
      end
    end
  else
    tb.setFV(FDI_NAME, '0松本市') # numeric '0' kanji 'matumostoshi'
    expect(tb.getFVstr(FDI_NAME)).to eq '0松本'
  end
  expect(tb.getFVstr(FDI_GROUP)).to eq '68'
end

def testVarField()
  db = Transactd::Database.new()
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
  db.close()
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

def testVarInsert()
  startid = 1
  bulk = false
  str = '漢字文字のテスト' # too long kanji
  str2 = '123'
  db = Transactd::Database.new()
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
  db.close()
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

def testVarRead()
  str = '漢字文'
  str3 = '漢字文字のテ'
  str2 ='123'
  str4 ='1232'
  db = Transactd::Database.new()
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
  db.close()
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

def testFilterVar()
  db = Transactd::Database.new()
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
  db.close()
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
  tb.beginBulkInsert(BULKBUFSIZE)
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
  tb.commitBulkInsert()
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
  expect(tb.stat()).to eq 0
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.findNext(false)
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'あいうえおかきくこ'
  expect(tb.recordCount()).to eq 2
  
  tb.setFilter("name <> 'あい*'", 0, 10)
  expect(tb.recordCount()).to eq 3
  tb.clearBuffer()
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.findNext(false)
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'A123456'
  
  tb.findNext()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'おはようございます'
  
  tb.findNext()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'おめでとうございます。'
  
  tb.findNext()
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  
  tb.clearBuffer()
  tb.seekLast()
  tb.findPrev(false)
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'おめでとうございます。'
  
  tb.findPrev()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'おはようございます'
  
  tb.findPrev(false)
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'A123456'
  
  tb.findPrev()
  expect(tb.stat()).to eq Transactd::STATUS_EOF
  
  tb.setFilter("name = 'あい'", 0, 10)
  expect(tb.recordCount()).to eq 0
  
  tb.setFilter("name <> ''", 0, 10)
  expect(tb.recordCount()).to eq 5
  
  # testing that setFilter don't change field value
  tb.clearBuffer()
  tb.setFV('name', 'ABCDE')
  tb.setFilter("name = 'あい'", 0, 10)
  expect(tb.getFVstr(FDI_NAME)).to eq 'ABCDE'
end

def doTestUpdateStringFilter(tb)
  tb.setKeyNum(0)
  tb.clearBuffer()
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.setFV('name', 'ABCDE')
  tb.setFV('namew', 'ABCDEW')
  tb.update()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.stat()).to eq 0
  
  tb.setFV('name', 'ABCDE2')
  tb.setFV('namew', 'ABCDEW2')
  tb.update()
  expect(tb.stat()).to eq 0
  
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'ABCDE'
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'ABCDEW'
  tb.seekNext()
  expect(tb.stat()).to eq 0
  expect(tb.getFVstr(FDI_NAME)).to eq 'ABCDE2'
  expect(tb.getFVstr(FDI_NAMEW)).to eq 'ABCDEW2'
end

def doTestStringFilter(db, id, name, type, type2)
  testCreateTableStringFilter(db, id, name, type, type2)
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  doTestInsertStringFilter(tb)
  doTestReadStringFilter(tb)
  doTestSetStringFilter(tb)
  doTestUpdateStringFilter(tb)
  tb.close()
end

def testStringFilter()
  db = Transactd::Database.new()
  db.create(URL_SF)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    testDropDatabaseStringFilter(db)
    db.create(URL_SF)
  end
  expect(db.stat()).to eq 0
  db.open(URL_SF, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  doTestStringFilter(   db, 1, 'zstring',    Transactd::Ft_zstring,    Transactd::Ft_wzstring)
  if (isUtf16leSupport(db))
    doTestStringFilter( db, 2, 'myvarchar',  Transactd::Ft_myvarchar,  Transactd::Ft_mywvarchar)
  else
    doTestStringFilter( db, 2, 'myvarchar',  Transactd::Ft_myvarchar,  Transactd::Ft_myvarchar)
  end
  doTestStringFilter(   db, 3, 'mytext',     Transactd::Ft_mytext,     Transactd::Ft_myblob)
  db.close()
end

def testDropDatabaseStringFilter(db)
  db.open(URL_SF)
  expect(db.stat()).to eq 0
  db.drop()
  expect(db.stat()).to eq 0
end

def testQuery()
  q = Transactd::Query.new()
  q.queryString("id = 0 and name = 'Abc efg'")
  expect(q.toString()).to eq "id = '0' and name = 'Abc efg'"
  
  q.queryString('')
  q.where('id', '=', '0').and_('name', '=', 'Abc efg')
  expect(q.toString()).to eq "id = '0' and name = 'Abc efg'"
  
  q.queryString("select id,name id = 0 AND name = 'Abc&' efg'")
  expect(q.toString()).to eq "select id,name id = '0' AND name = 'Abc&' efg'"
  
  q.queryString('')
  q.select('id', 'name').where('id', '=', '0').and_('name', '=', "Abc' efg")
  expect(q.toString()).to eq "select id,name id = '0' and name = 'Abc&' efg'"
  
  q.queryString("select id,name id = 0 AND name = 'Abc&& efg'")
  expect(q.toString()).to eq "select id,name id = '0' AND name = 'Abc&& efg'"
  
  q.queryString('')
  q.select('id', 'name').where('id', '=', '0').and_('name', '=', 'Abc& efg')
  expect(q.toString()).to eq "select id,name id = '0' and name = 'Abc&& efg'"
  
  q.queryString('*')
  expect(q.toString()).to eq '*'
  
  q.queryString('')
  q.all()
  expect(q.toString()).to eq '*'
  
  q.queryString('Select id,name id = 2')
  expect(q.toString()).to eq "select id,name id = '2'"
  
  q.queryString('')
  q.select('id', 'name').where('id', '=', '2')
  expect(q.toString()).to eq "select id,name id = '2'"
  
  q.queryString('SELECT id,name,fc id = 2')
  expect(q.toString()).to eq "select id,name,fc id = '2'"
  
  q.queryString('')
  q.select('id', 'name', 'fc').where('id', '=', '2')
  expect(q.toString()).to eq "select id,name,fc id = '2'"
  
  q.queryString("select id,name,fc id = 2 and name = '3'")
  expect(q.toString()).to eq "select id,name,fc id = '2' and name = '3'"
  
  q.queryString('')
  q.select('id', 'name', 'fc').where('id', '=', '2').and_('name', '=', '3')
  expect(q.toString()).to eq "select id,name,fc id = '2' and name = '3'"
  
  #  IN include
  q.queryString("select id,name,fc IN '1','2','3'")
  expect(q.toString()).to eq "select id,name,fc in '1','2','3'"
  
  q.queryString('')
  q.select('id', 'name', 'fc').in('1', '2', '3')
  expect(q.toString()).to eq "select id,name,fc in '1','2','3'"
  
  q.queryString("IN '1','2','3'")
  expect(q.toString()).to eq "in '1','2','3'"
  
  q.queryString('IN 1,2,3')
  expect(q.toString()).to eq "in '1','2','3'"
  
  q.queryString('')
  q.in('1', '2', '3')
  expect(q.toString()).to eq "in '1','2','3'"
  
  # special field name
  q.queryString('select = 1')
  expect(q.toString()).to eq "select = '1'"
  
  q.queryString('')
  q.where('select', '=', '1')
  expect(q.toString()).to eq "select = '1'"
  
  q.queryString('in <> 1')
  expect(q.toString()).to eq "in <> '1'"
  
  q.queryString('')
  q.where('in', '<>', '1')
  expect(q.toString()).to eq "in <> '1'"
  
  # test auto_escape
  q.queryString("code = ab'c", true)
  expect(q.toString()).to eq "code = 'ab&'c'"
  
  q.queryString("code = ab&c", true)
  expect(q.toString()).to eq "code = 'ab&&c'"
  
  q.queryString("code = abc&", true)
  expect(q.toString()).to eq "code = 'abc&&'"
  q.queryString("code = abc&&", true)
  expect(q.toString()).to eq "code = 'abc&&&&'"
  
  q.queryString("code = 'abc&'", true)
  expect(q.toString()).to eq "code = 'abc&&'"
  q.queryString("code = 'abc&&'", true)
  expect(q.toString()).to eq "code = 'abc&&&&'"
  
  q.queryString("code = 'ab'c'", true)
  expect(q.toString()).to eq "code = 'ab&'c'"
  
  q.queryString("code = 'abc''", true)
  expect(q.toString()).to eq "code = 'abc&''"
  
  q.queryString("code = abc'", true)
  expect(q.toString()).to eq "code = 'abc&''"
  
  # Invalid single quote (') on the end of statement
  q.queryString("code = 'abc", true)
  expect(q.toString()).to eq "code = 'abc'"
  
  q.queryString("code = &abc", true)
  expect(q.toString()).to eq "code = '&&abc'"
end

def createQTuser(db)
  dbdef = db.dbDef()
  td = Transactd::Tabledef.new()
  td.setTableName('user')
  td.setFileName('user.dat')
  id = 1
  td.id = id
  td.pageSize = 2048
  td.schemaCodePage = Transactd::CP_UTF8
  td.charsetIndex = Transactd::CHARSET_UTF8
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # id field
  fd = dbdef.insertField(id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_autoinc
  fd.len = 4
  # 名前 field
  fd = dbdef.insertField(id, 1)
  fd.setName('名前')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(20)
  # group field
  fd = dbdef.insertField(id, 2)
  fd.setName('group')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # tel field
  fd = dbdef.insertField(id, 3)
  fd.setName('tel')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(20)
  # key 0 (primary) id
  kd = dbdef.insertKey(id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1 # extended key type
  kd.segment(0).flags.bit1 = 1 # changeable
  kd.segmentCount = 1
  td = dbdef.tableDefs(id)
  td.primaryKeyNum = 0
  # key 1 group
  kd = dbdef.insertKey(id, 1)
  kd.segment(0).fieldNum = 2
  kd.segment(0).flags.bit8 = 1 # extended key type
  kd.segment(0).flags.bit1 = 1 # changeable
  kd.segment(0).flags.bit0 = 1 # duplicatable
  kd.segmentCount = 1
  # update
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # open test
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  tb.close() if tb != nil
  return true
end

def createQTgroups(db)
  dbdef = db.dbDef()
  td = Transactd::Tabledef.new()
  td.setTableName('groups')
  td.setFileName('groups.dat')
  id = 2
  td.id = id
  td.pageSize = 2048
  td.schemaCodePage = Transactd::CP_UTF8
  td.charsetIndex = Transactd::CHARSET_UTF8
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # code field
  fd = dbdef.insertField(id, 0)
  fd.setName('code')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # name field
  fd = dbdef.insertField(id, 1)
  fd.setName('name')
  fd.type = Transactd::Ft_myvarbinary
  fd.len = 33
  # key 0 (primary) code
  kd = dbdef.insertKey(id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segmentCount = 1
  td = dbdef.tableDefs(id)
  td.primaryKeyNum = 0
  # update
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # open test
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  tb.close() if tb != nil
  return true
end

def createQTextention(db)
  dbdef = db.dbDef()
  td = Transactd::Tabledef.new()
  td.setTableName('extention')
  td.setFileName('extention.dat')
  id = 3
  td.id = id
  td.pageSize = 2048
  td.schemaCodePage = Transactd::CP_UTF8
  td.charsetIndex = Transactd::CHARSET_UTF8
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # id field
  fd = dbdef.insertField(id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # comment field
  fd = dbdef.insertField(id, 1)
  fd.setName('comment')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(60)
  # blob field
  fd = dbdef.insertField(id, 2)
  fd.setName('blob')
  fd.type = Transactd::Ft_myblob
  fd.len = 10
  # key 0 (primary) id
  kd = dbdef.insertKey(id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1  # extended key type
  kd.segment(0).flags.bit1 = 1  # changeable
  kd.segmentCount = 1
  td = dbdef.tableDefs(id)
  td.primaryKeyNum = 0
  # update
  dbdef.updateTableDef(id)
  expect(dbdef.stat()).to eq 0
  # open test
  tb = db.openTable(id)
  expect(db.stat()).to eq 0
  tb.close() if tb != nil
  return true
end

def insertQT(db, maxId)
  db.beginTrn()
  # insert user data
  tb = db.openTable('user', Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  expect(tb).not_to be nil
  tb.clearBuffer()
  for i in 1..maxId
    tb.setFV(0, i)
    tb.setFV(1, "#{i} user")
    tb.setFV('group', ((i - 1) % 5) + 1)
    tb.insert()
    expect(tb.stat()).to eq 0
  end
  tb.close()
  # insert groups data
  tb = db.openTable('groups', Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  expect(tb).not_to be nil
  tb.clearBuffer()
  for i in 1..100
    tb.setFV(0, i)
    tb.setFV(1, "#{i} group")
    tb.insert()
    expect(tb.stat()).to eq 0
  end
  tb.close()
  # insert extention data
  tb = db.openTable('extention', Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  expect(tb).not_to be nil
  tb.clearBuffer()
  for i in 1..maxId
    tb.setFV(0, i)
    tb.setFV(1, "#{i} comment")
    tb.setFV(2, "#{i} blob")
    tb.insert()
    expect(tb.stat()).to eq 0
  end
  tb.close()
  db.endTrn()
end

def testCreateQueryTest()
  # check database existence
  db = Transactd::Database.new()
  db.open(URL_QT, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  if (db.stat() != 0) then
    puts "\nDatabase " + DBNAME_QT + " not found\n"
  else
    dbdef = db.dbDef()
    td = dbdef.tableDefs(3)
    if (td != nil && td.fieldCount == 3) then
      tb = db.openTable('extention')
      if (db.stat() == 0 && tb.recordCount(false) == TEST_COUNT)
        tb.close()
        db.close()
        return
      end
      tb.close()
    end
    db.drop()
  end
  puts "\nCreate database " + DBNAME_QT + "\n"
  db.create(URL_QT)
  expect(db.stat()).to eq 0
  db.open(URL_QT, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  # create tables
  createQTuser(db)
  createQTgroups(db)
  createQTextention(db)
  # insert data
  insertQT(db, 20000)
  db.close()
end

def testNewDelete()
  db = Transactd::Database.new()
  db.open(URL_QT)
  for i in 0..499
    q  = Transactd::Query.new()
    rq = Transactd::RecordsetQuery.new()
    gq = Transactd::GroupQuery.new()
    f  = Transactd::FieldNames.new()
    f.addValue('abc')
    atu = Transactd::ActiveTable.new(db, 'user')
    atu.index(0)
    atg = Transactd::ActiveTable.new(db, 'groups')
    atg.index(0)
    fns = Transactd::FieldNames.new()
    fns.keyField('a')
    s = Transactd::Sum.new(fns)
    c = Transactd::Count.new('a')
    a = Transactd::Avg.new(fns)
    mi = Transactd::Min.new(fns)
    ma = Transactd::Max.new(fns)
    rs = Transactd::Recordset.new()
    # have to explicitly release
    atu.release()
    atg.release()
  end
  db.close()
end

def testJoin()
  db = Transactd::Database.new()
  db.open(URL_QT)
  expect(db.stat()).to eq 0
  atu = Transactd::ActiveTable.new(db, 'user')
  atg = Transactd::ActiveTable.new(db, 'groups')
  ate = Transactd::ActiveTable.new(db, 'extention')
  q = Transactd::Query.new()
  
  atu.alias('名前', 'name')
  q.select('id', 'name', 'group').where('id', '<=', '15000')
  rs = atu.index(0).keyValue('1').read(q)
  expect(rs.size()).to eq 15000
  
  # Join extention::comment
  q.reset()
  ate.index(0).join(rs,
    q.select('comment').optimize(Transactd::QueryBase::JoinHasOneOrHasMany), 'id')
  expect(rs.size()).to eq 15000
  
  # reverse and get first (so it means 'get last')
  last = rs.reverse().first()
  expect(last['id']).to eq 15000
  expect(last['comment']).to eq '15000 comment'
  
  # Join group::name
  q.reset()
  atg.alias('name', 'group_name')
  atg.index(0).join(rs, q.select('group_name'), 'group')
  expect(rs.size()).to eq 15000
  
  # get last (the rs is reversed, so it means 'get first')
  first = rs.last()
  expect(first['id']).to eq 1
  expect(first['comment']).to eq '1 comment'
  expect(first['group_name']).to eq '1 group'
  
  # row in rs[15000 - 9]
  rec = rs[15000 - 9]
  expect(rec['group_name']).to eq '4 group'
  
  # orderby
  rs.orderBy('group_name')
  for i in 0..(15000 / 5 - 1)
    expect(rs[i]['group_name']).to eq '1 group'
  end
  expect(rs[15000 / 5]['group_name']).to eq '2 group'
  expect(rs[(15000 / 5) * 2]['group_name']).to eq '3 group'
  expect(rs[(15000 / 5) * 3]['group_name']).to eq '4 group'
  expect(rs[(15000 / 5) * 4]['group_name']).to eq '5 group'
  
  # union
  q.reset()
  q.select('id', 'name', 'group').where('id', '<=', '16000')
  rs2 = atu.index(0).keyValue('15001').read(q)
  expect(rs2.size()).to eq 1000
  q.reset()
  ate.index(0).join(rs2,
    q.select('comment').optimize(Transactd::QueryBase::JoinHasOneOrHasMany), 'id')
  expect(rs2.size()).to eq 1000
  q.reset()
  atg.index(0).join(rs2, q.select('group_name'), 'group')
  expect(rs2.size()).to eq 1000
  rs.unionRecordset(rs2)
  expect(rs.size()).to eq 16000
  # row in rs[15000]
  expect(rs[15000]['id']).to eq 15001
  # last
  expect(rs.last()['id']).to eq 16000
  
  # group by
  gq = Transactd::GroupQuery.new()
  gq.keyField('group', 'id')
  count1 = Transactd::Count.new('count')
  gq.addFunction(count1)
  
  count2 = Transactd::Count.new('group1_count')
  count2.when('group', '=', '1')
  gq.addFunction(count2)
  
  rs.groupBy(gq)
  expect(rs.size()).to eq 16000
  expect(rs[0]['group1_count']).to eq 1
  
  # clone
  rsv = rs.clone
  gq.reset()
  count3 = Transactd::Count.new('count')
  gq.addFunction(count3).keyField('group')
  rs.groupBy(gq)
  expect(rs.size()).to eq 5
  expect(rsv.size()).to eq 16000
  
  # having
  rq = Transactd::RecordsetQuery.new()
  rq.when('group1_count', '=', '1').or_('group1_count', '=', '2')
  rsv.matchBy(rq)
  expect(rsv.size()).to eq 3200
  expect(rsv).not_to be nil
  rsv = nil
  expect(rsv).to be nil
  
  # top
  rs3 = Transactd::Recordset.new()
  rs.top(rs3, 10)
  expect(rs3.size()).to eq 5
  rs.top(rs3, 3)
  expect(rs3.size()).to eq 3
  expect(rs.size()).to eq 5
  
  # query new / delete
  q1 = Transactd::RecordsetQuery.new()
  q1.when('group1_count', '=', '1').or_('group1_count', '=', '2')
  q1 = nil
  
  q2 = Transactd::Query.new()
  q2.where('group1_count', '=', '1').or_('group1_count', '=', '2')
  q2 = nil
  
  q3 = Transactd::GroupQuery.new()
  q3.keyField('group', 'id')
  q3 = nil
  
  atu.release()
  atg.release()
  ate.release()
  db.close()
end

def testPrepareJoin()
  db = Transactd::Database.new()
  db.open(URL_QT)
  expect(db.stat()).to eq 0
  
  atu = Transactd::ActiveTable.new(db, 'user')
  atu.alias('名前', 'name')
  atg = Transactd::ActiveTable.new(db, 'groups')
  atg.alias('name', 'group_name')
  ate = Transactd::ActiveTable.new(db, 'extention')
  q = Transactd::Query.new()
  
  q.select('id', 'name', 'group').where('id', '<=', '?')
  pq = atu.prepare(q)
  
  # integer value
  rs = atu.index(0).keyValue('1').read(pq, 15000)
  expect(rs.size()).to eq 15000
  # float value
  rs = atu.index(0).keyValue('1').read(pq, 15000.000)
  expect(rs.size()).to eq 15000
  # String value
  rs = atu.index(0).keyValue('1').read(pq, '15000')
  expect(rs.size()).to eq 15000
  # Using supply value
  pq.supplyValue(0, 15000)
  rs = atu.index(0).keyValue('1').read(pq)
  expect(rs.size()).to eq 15000
  
  # Join extention::comment
  q.reset().select('comment').optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
  pq = ate.prepare(q)
  ate.index(0).join(rs, pq, 'id')
  expect(rs.size()).to eq 15000
  
  # reverse and get first (so it means 'get last')
  last = rs.reverse().first()
  expect(last['id']).to eq 15000
  expect(last['comment']).to eq '15000 comment'
  
  # Join group::name
  q.reset().select('group_name')
  pq = atg.prepare(q)
  atg.index(0).join(rs, pq, 'group')
  expect(rs.size()).to eq 15000
  
  # get last (the rs is reversed, so it means 'get first')
  first = rs.last()
  expect(first['id']).to eq 1
  expect(first['comment']).to eq '1 comment'
  expect(first['group_name']).to eq '1 group'
  
  # row in rs[15000 - 9]
  rec = rs[15000 - 9]
  expect(rec['group_name']).to eq '4 group'
  
  atu.release()
  atg.release()
  ate.release()
  db.close()
end

def testServerPrepareJoin()
  db = Transactd::Database.new()
  db.open(URL_QT)
  expect(db.stat()).to eq 0
  
  atu = Transactd::ActiveTable.new(db, 'user')
  atu.alias('名前', 'name')
  atg = Transactd::ActiveTable.new(db, 'groups')
  atg.alias('name', 'group_name')
  ate = Transactd::ActiveTable.new(db, 'extention')
  q = Transactd::Query.new()
  
  q.select('id', 'name', 'group').where('id', '<=', '?')
  stmt1 = atu.prepare(q, true)
  expect(stmt1).not_to eq nil
  
  q.reset().select('comment').optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
  stmt2 = ate.prepare(q, true)
  expect(stmt2).not_to eq nil
  
  q.reset().select('group_name')
  stmt3 = atg.prepare(q, true)
  expect(stmt3).not_to eq nil
  
  rs = atu.index(0).keyValue(1).read(stmt1, 15000)
  expect(rs.size()).to eq 15000
  
  # Join extention::comment
  ate.index(0).join(rs, stmt2, 'id')
  expect(rs.size()).to eq 15000
  
  # test reverse
  last = rs.reverse().first()
  expect(last['id']).to eq 15000
  expect(last['comment']).to eq '15000 comment'
  
  # Join group::name
  atg.index(0).join(rs, stmt3, 'group')
  expect(rs.size()).to eq 15000
  first = rs.last()
  
  expect(first['id']).to eq 1
  expect(first['comment']).to eq '1 comment'
  expect(first['group_name']).to eq '1 group'
  
  # rs[15000 - 9]
  rec = rs[15000 - 9]
  expect(rec['group_name']).to eq '4 group'
  
  # Test orderby
  rs.orderBy('group_name')
  # rs[0]
  expect(rs[0]['group_name']).to eq '1 group'
  
  # All fields
  rs.clear()
  q.reset().all()
  q.where('id', '<=', '?')
  stmt1 = atu.prepare(q, true)
  rs = atu.keyValue(1).read(stmt1, 15000)
  expect(rs.size()).to eq 15000
  if (rs.size() == 15000) then
    for i in 0..14999 do
      expect(rs[i]['id']).to eq i + 1
    end
  end
  
  ate.join(rs, stmt2, 'id')
  expect(rs.size()).to eq 15000
  atg.join(rs, stmt3, 'group')
  expect(rs.size()).to eq 15000
  
  # OuterJoin
  tb = ate.table()
  tb.setFV('id', NO_RECORD_ID)
  tb.seek()
  expect(tb.stat()).to eq 0
  tb.del() if (tb.stat() == 0)
  expect(tb.stat()).to eq 0
  q.reset().select('comment', 'blob').optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
  stmt2 = ate.prepare(q, true)
  
  # Join is remove record(s) no join target record.
  rs.clear()
  rs = atu.keyValue(1).read(stmt1, 15000)
  ate.join(rs, stmt2, 'id')
  expect(rs.size()).to eq 14999
  expect(rs[NO_RECORD_ID - 1]['id']).to eq (NO_RECORD_ID + 1)
  expect(rs[NO_RECORD_ID - 1]['comment']).to eq "#{NO_RECORD_ID + 1} comment"
  expect(rs[NO_RECORD_ID - 1]['blob']).to eq "#{NO_RECORD_ID + 1} blob"
  
  # OuterJoin is no remove record(s) no join target record.
  rs.clear()
  rs = atu.keyValue(1).read(stmt1, 15000)
  ate.outerJoin(rs, stmt2, 'id')
  expect(rs.size()).to eq 15000
  atg.outerJoin(rs, stmt3, 'group')
  expect(rs.size()).to eq 15000
  
  expect(rs[NO_RECORD_ID - 1].isInvalidRecord()).to eq true
  expect(rs[NO_RECORD_ID]['comment']).to eq "#{NO_RECORD_ID + 1} comment"
  expect(rs[NO_RECORD_ID]['blob']).to eq "#{NO_RECORD_ID + 1} blob"
  
  # OuterJoin All Join fields
  q.reset().optimize(Transactd::QueryBase::JoinHasOneOrHasMany).all()
  stmt2 = ate.prepare(q, true)
  rs.clear()
  rs = atu.keyValue(1).read(stmt1, 15000)
  ate.outerJoin(rs, stmt2, 'id')
  expect(rs.size()).to eq 15000
  expect(rs[NO_RECORD_ID - 1].isInvalidRecord()).to eq true
  expect(rs[NO_RECORD_ID]['comment']).to eq "#{NO_RECORD_ID + 1} comment"
  expect(rs[NO_RECORD_ID]['blob']).to eq "#{NO_RECORD_ID + 1} blob"
  
  # Test clone blob field
  rs2 = rs.clone()
  expect(rs2.size()).to eq 15000
  expect(rs2[NO_RECORD_ID - 1].isInvalidRecord()).to eq true
  expect(rs2[NO_RECORD_ID]['comment']).to eq "#{NO_RECORD_ID + 1} comment"
  expect(rs2[NO_RECORD_ID]['blob']).to eq "#{NO_RECORD_ID + 1} blob"
  
  # hasManyJoin inner
  rs.clear()
  q.reset().reject(0xFFFF).limit(0).all()
  rs = atg.keyValue(1).read(q)
  expect(rs.size()).to eq 100
  q.all().optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
  atu.index(1).join(rs, q, 'code')
  expect(rs.size()).to eq 20000
  
  # hasManyJoin outer
  rs.clear()
  q.reset().reject(0xFFFF).limit(0).all()
  rs = atg.keyValue(1).read(q)
  expect(rs.size()).to eq 100
  q.all().optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
  atu.index(1).outerJoin(rs, q, 'code')
  expect(rs.size()).to eq 20095
  
  # restore record
  tb.clearBuffer()
  tb.setFV('id', NO_RECORD_ID)
  tb.setFV('comment', '5 comment')
  tb.setFV('blob', '5 blob')
  tb.insert()
  expect(tb.stat()).to eq 0
  status = tb.stat()
  atu.release()
  atg.release()
  ate.release()
  db.drop() unless (status == 0)
  db.close()
end

def testWirtableRecord()
  db = Transactd::Database.new()
  db.open(URL_QT)
  expect(db.stat()).to eq 0
  atu = Transactd::ActiveTable.new(db, 'user')
  
  rec = atu.index(0).getWritableRecord()
  rec['id'] = 120000
  rec['名前'] = 'aiba'
  rec.save()
  
  rec.clear()
  expect(rec['id']).not_to eq 120000
  expect(rec['名前']).not_to eq 'aiba'
  rec['id'] = 120000
  rec.read()
  expect(rec['id']).to eq 120000
  expect(rec['名前']).to eq 'aiba'
  
  rec.clear()
  rec['id'] = 120001
  rec['名前'] = 'oono'
  rec.insert() unless rec.read()
  
  rec.clear()
  rec['id'] = 120001
  rec.read()
  expect(rec['id']).to eq 120001
  expect(rec['名前']).to eq 'oono'
  
  # update only changed filed
  rec.clear()
  rec['id'] = 120001
  rec['名前'] = 'matsumoto'
  rec.update()
  
  rec.clear()
  rec['id'] = 120001
  rec.read()
  expect(rec['id']).to eq 120001
  expect(rec['名前']).to eq 'matsumoto'
  
  rec.del()
  rec['id'] = 120000
  rec.del()
  
  rec.clear()
  rec['id'] = 120001
  ret = rec.read()
  expect(ret).to eq false
  
  rec.clear()
  rec['id'] = 120000
  ret = rec.read()
  expect(ret).to eq false
  
  atu.release()
  db.close()
end

describe Transactd do
  it 'create database' do
    db = Transactd::Database.new()
    testCreateDatabase(db)
    db.close()
  end
  it 'create table' do
    db = Transactd::Database.new()
    testCreateDatabase(db)
    testCreateTable(db)
    db.close()
  end
  it 'open table' do
    db = Transactd::Database.new()
    testOpenTable(db)
    db.close()
  end
  it 'clone db object' do
    testClone()
  end
  it 'version' do
    testVersion()
  end
  it 'insert' do
    testInsert()
  end
  it 'find' do
    testFind()
  end
  it 'findNext' do
    testFindNext()
  end
  it 'findIn' do
    testFindIn()
  end
  it 'get percentage' do
    testGetPercentage()
  end
  it 'move percentage' do
    testMovePercentage()
  end
  it 'get equal' do
    testGetEqual()
  end
  it 'get next' do
    testGetNext()
  end
  it 'get previous' do
    testGetPrevious()
  end
  it 'get greater' do
    testGetGreater()
  end
  it 'get less than' do
    testGetLessThan()
  end
  it 'get first' do
    testGetFirst()
  end
  it 'get last' do
    testGetLast()
  end
  it 'move position' do
    testMovePosition()
  end
  it 'update' do
    testUpdate()
  end
  it 'snapshot' do
    testSnapshot()
  end
  it 'send conflict error' do
    testConflict()
  end
  it 'transaction (REPEATABLE_READ)' do
    testTransactionLockRepeatable()
  end
  it 'transaction (READ_COMMITED)' do
    testTransactionLockReadCommited()
  end
  it 'record lock' do
    testRecordLock()
  end
  it 'exclusive' do
    testExclusive()
  end
  it 'multi database' do
    testMultiDatabase()
  end
  it 'missing update' do
    testMissingUpdate()
  end
  it 'insert2' do
    testInsert2()
  end
  it 'delete' do
    testDelete()
  end
  it 'set owner' do
    testSetOwner()
  end
  it 'drop index' do
    testDropIndex()
  end
  it 'drop database' do
    db = Transactd::Database.new()
    testDropDatabase(db)
    db.close()
  end
  it 'login' do
    testLogin()
  end
  it 'query' do
    testQuery()
  end
  it 'create querytest db' do
    testCreateQueryTest()
  end
  it 'new and delete objects' do
    testNewDelete()
  end
  it 'activetable and join' do
    testJoin()
  end
  it 'activetable and prepare' do
    testPrepareJoin()
  end
  it 'activetable and prepare (server)' do
    testServerPrepareJoin()
  end
  it 'write with writableRecord' do
    testWirtableRecord()
  end
end

describe Transactd, 'var tables' do
  it 'create var database' do
    testCreateDatabaseVar()
  end
  it 'set kanji char to field' do
    testVarField()
  end
  it 'insert kanji char to field' do
    testVarInsert()
  end
  it 'read kanji char from field' do
    testVarRead()
  end
  it 'filter' do
    testFilterVar()
  end
  it 'drop var database' do
    db = Transactd::Database.new()
    testDropDatabaseVar(db)
  end
end

describe Transactd, 'StringFilter' do
  it 'string filter' do
    testStringFilter()
  end
  it 'drop database' do
    db = Transactd::Database.new()
    testDropDatabaseStringFilter(db)
  end
end
