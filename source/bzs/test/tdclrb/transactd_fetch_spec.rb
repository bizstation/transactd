# coding : utf-8
=begin ============================================================
   Copyright (C) 2015 BizStation Corp All rights reserved.

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

def getEnv(valuename)
  return ENV[valuename] if ENV[valuename] != nil
  return ''
end

def getHost()
  hostname = getEnv('TRANSACTD_RSPEC_HOST')
  hostname = '127.0.0.1/' if hostname == ''
  hostname = hostname + '/' unless (hostname =~ /\/$/)
  return hostname
end

HOSTNAME = getHost()
USERNAME = getEnv('TRANSACTD_RSPEC_USER')
USERPART = USERNAME == '' ? '' : USERNAME + '@'
PASSWORD = getEnv('TRANSACTD_RSPEC_PASS')
PASSPART = PASSWORD == '' ? '' : '&pwd=' + PASSWORD
URL = 'tdap://' + USERPART + HOSTNAME + 'test_fetch?dbfile=transactd_schema' + PASSPART
TABLENAME = 'users'

FIELDS = ['id', 'name', 'age', 'group_id'];
ALIASED_FIELDS = ['id', 'name', 'age', 'group'];

MAX_ID = 2000
CHECK_MAX_ID = 10
FETCHMODES = [Transactd::FETCH_VAL_NUM, Transactd::FETCH_VAL_ASSOC, Transactd::FETCH_VAL_BOTH,
  Transactd::FETCH_OBJ, Transactd::FETCH_USR_CLASS, Transactd::FETCH_RECORD_INTO]

def dropDatabase(db)
  db.drop(URL)
  expect(db.stat()).to eq 0
end

def createDatabase(db)
  db.create(URL)
  if (db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR)
    dropDatabase(db)
    db.create(URL)
  end
  expect(db.stat()).to eq 0
end

def openDatabase(db)
  return db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
end

def createUsersTable(db)
  openDatabase(db)
  dbdef = db.dbDef()
  expect(dbdef).not_to eq nil
  td = Transactd::Tabledef.new()
  td.schemaCodePage = Transactd::CP_UTF8
  td.setTableName(TABLENAME)
  td.setFileName(TABLENAME)
  td.charsetIndex = Transactd::CHARSET_UTF8
  tableid = 1
  td.id = tableid
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # id
  fieldIndex = 0
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName(FIELDS[fieldIndex])
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # name
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName(FIELDS[fieldIndex])
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(60)
  # age
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName(FIELDS[fieldIndex])
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # group_id
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName(FIELDS[fieldIndex])
  fd.type = Transactd::Ft_integer
  fd.len = 4
  # primary key
  keynum = 0
  kd = dbdef.insertKey(tableid, keynum)
  kd.segment(0).fieldNum = 0 # id
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  td = dbdef.tableDefs(tableid)
  td.primaryKeyNum = keynum
  # update
  dbdef.updateTableDef(tableid)
  expect(dbdef.stat()).to eq 0
end

def insertData(db)
  tb = db.openTable(TABLENAME, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  begin
    db.beginTrn()
    tb.clearBuffer()
    for i in 1..MAX_ID
      tb.setFV(0, i) # id
      tb.setFV(1, i.to_s() + " user") # name
      tb.setFV(2, (i % 50) + 15 + rand(15)) # age
      tb.setFV(3, (i + rand(5)) % 5) # group_id
      tb.insert()
    end
    db.endTrn()
    tb.close()
  rescue => e
    db.abortTrn()
    tb.close()
    expect(true).to eq false
  end
end


class UserBase
  @_alias_map = {}
  def self.setAlias(aliased)
    @_alias_map = (aliased ? { :group => 'group_id' } : {}) # This is reverse map
    self
  end
  def self.alias_map
    return @_alias_map
  end
  
  @_nodefine_original = false
  def self.set_nodefine_original(v)
    @_nodefine_original = v
    self
  end
  def self.nodefine_original
    return @_nodefine_original
  end
end


def openTable(db)
  tb = db.openTable(TABLENAME)
  expect(db.stat()).to eq 0
  return tb
end

def seekId1(tb)
  tb.setKeyNum(0)
  expect(tb.stat()).to eq 0
  tb.clearBuffer()
  expect(tb.stat()).to eq 0
  tb.setFV(0, 1)
  expect(tb.stat()).to eq 0
  tb.seek()
  expect(tb.stat()).to eq 0
  return tb.fields
end

def openActiveTable(db, alias_map = nil)
  at = Transactd::ActiveTable.new(db, TABLENAME)
  at.index(0).keyValue(0, 0)
  if alias_map.is_a?(Hash)
    at.resetAlias()
    alias_map.each {|key, orign| at.alias(orign, key.to_s) }
  else
    at.alias('group_id', 'group')
  end
  return at
end

def getRecordSet(at)
  rs = at.read(Transactd::Query.new())
  expect(rs.size()).to be > 0
  return rs
end

def doTestArray(rec, rec_id = 1)
  expect(rec.is_a?(Array)).to eq true
  expect(rec.length).to eq FIELDS.length
  expect(rec[0]).to eq rec_id
  expect(rec[1]).to eq "#{rec_id} user"
end

def doTestHash(rec, aliased, with_number, rec_id = 1)
  expect(rec.is_a?(Hash)).to eq true
  fs = aliased ? ALIASED_FIELDS : FIELDS
  for name in fs
    expect(rec.has_key?(name)).to eq true
  end
  for i in 0...fs.length
    expect(rec.has_key?(i)).to eq with_number
  end
  expect(rec[FIELDS[0]]).to eq rec_id
  expect(rec[FIELDS[1]]).to eq "#{rec_id} user"
  if with_number
    expect(rec[0]).to eq rec_id
    expect(rec[1]).to eq "#{rec_id} user"
  end
end

def doTestObject(rec, aliased, rec_id = 1)
  expect(rec.is_a?(Object)).to eq true
  expect(rec.is_a?(Hash)).to eq false
  expect(rec.is_a?(Array)).to eq false
  fs = aliased ? ALIASED_FIELDS : FIELDS
  for name in fs
    expect(rec.respond_to?(name)).to eq true
    expect(rec.respond_to?(name + '=')).to eq true
  end
  expect(rec.id).to eq rec_id
  expect(rec.name).to eq "#{rec_id} user"
  rec.id = rec_id + 10
  rec.name = "#{rec_id} user *"
  expect(rec.id).to eq rec_id + 10
  expect(rec.name).to eq "#{rec_id} user *"
end

def doTestClass(rec, klass)
  aliased = klass.alias_map.length != 0
  expect(rec.is_a?(klass)).to eq true
  expect(rec.is_a?(Hash)).to eq false
  expect(rec.is_a?(Array)).to eq false
  expected_fields = aliased ? ALIASED_FIELDS : FIELDS
  expected_fields += FIELDS unless klass.nodefine_original
  expected_fields.uniq!
  not_expected_fields = (ALIASED_FIELDS | FIELDS) - expected_fields
  #p expected_fields
  #p not_expected_fields
  for name in expected_fields
    expect(rec.respond_to?(name)).to eq true
    expect(rec.respond_to?(name + '=')).to eq true
  end
  for name in not_expected_fields
    expect(rec.respond_to?(name)).to eq false
    expect(rec.respond_to?(name + '=')).to eq false
  end
end

def doTestTable(db, klass)
  # table
  tb = openTable(db)
  tb.fetchMode = Transactd::FETCH_USR_CLASS
  tb.fetchClass = klass
  if klass.alias_map.length > 0
    tb.setAlias("group_id", "group")
  end
  rec = seekId1(tb)
  doTestClass(rec, klass)
  for i in 2..CHECK_MAX_ID
    tb.seekNext()
    expect(tb.stat()).to eq 0
    rec = tb.fields()
    doTestClass(rec,  klass)
  end
  tb.close()
end

def doTestRecordset(rs, klass)
  # recordset
  rs.fetchClass = klass
  for i in 0...CHECK_MAX_ID
    rec = rs[i]
    doTestClass(rec, klass)
  end
end

def doTestFieldsBase(rec, aliased, rec_id = 1)
  expect(rec.is_a?(Transactd::Record)).to eq true
  expect(rec.is_a?(Hash)).to eq false
  expect(rec.is_a?(Array)).to eq false
  fs = aliased ? ALIASED_FIELDS : FIELDS
  for i in 0...fs.length
    expect(rec.indexByName(fs[i])).to eq i
  end
  expect(rec[rec.indexByName(fs[0])]).to eq rec_id
  expect(rec[rec.indexByName(fs[1])]).to eq "#{rec_id} user"
end

def findAll(tb)
  tb.setFilter('', 0, 0)
  expect(tb.stat()).to eq 0
  tb.setKeyNum(0)
  expect(tb.stat()).to eq 0
  tb.clearBuffer()
  expect(tb.stat()).to eq 0
  tb.seekFirst(0)
  expect(tb.stat()).to eq 0
  q = Transactd::Query.new()
  q.where('id', '<=', CHECK_MAX_ID).reject(1)
  tb.setQuery(q)
  expect(tb.stat()).to eq 0
  return tb.findAll
end

def getRecordSetForFindAll(at)
  q = Transactd::Query.new()
  q.where('id', '<=', CHECK_MAX_ID).reject(1)
  rs = at.read(q)
  expect(rs.size()).to eq CHECK_MAX_ID
  return rs
end

def doTestTableArray(db, klass)
  # table
  tb = openTable(db)
  tb.fetchMode = Transactd::FETCH_USR_CLASS
  tb.fetchClass = klass
  if klass.alias_map.length > 0
    tb.setAlias("group_id", "group")
  end
  arr = findAll(tb)
  expect(arr.is_a?(Array)).to eq true
  expect(arr.length).to eq CHECK_MAX_ID
  for i in 0...arr.length
    rec = arr[i]
    doTestClass(rec, klass)
  end
  tb.close()
end

def doTestRecordsetArray(rs, klass)
  # recordset
  rs.fetchClass = klass
  arr = rs.to_a
  expect(arr.is_a?(Array)).to eq true
  expect(arr.length).to eq CHECK_MAX_ID
  for i in 0...arr.length
    rec = arr[i]
    doTestClass(rec, klass)
  end
end

describe Transactd, 'FetchMode' do
  it 'create database and table' do
    db = Transactd::Database.new()
    createDatabase(db)
    openDatabase(db)
    createUsersTable(db)
    insertData(db)
    db.close()
  end
  
  it 'fetch with FETCH_VAL_NUM' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_NUM
    rec = seekId1(tb)
    doTestArray(rec)
    for i in 2..CHECK_MAX_ID
      tb.seekNext()
      expect(tb.stat()).to eq 0
      rec = tb.fields()
      doTestArray(rec, i)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_VAL_NUM
    for i in 0...CHECK_MAX_ID
      rec = rs[i]
      doTestArray(rec, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch with FETCH_VAL_ASSOC' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_ASSOC
    rec = seekId1(tb)
    doTestHash(rec, false, false)
    for i in 2..CHECK_MAX_ID
      tb.seekNext()
      expect(tb.stat()).to eq 0
      rec = tb.fields()
      doTestHash(rec, false, false, i)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_VAL_ASSOC
    for i in 0...CHECK_MAX_ID
      rec = rs[i]
      doTestHash(rec, true, false, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch with FETCH_VAL_BOTH' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_BOTH
    rec = seekId1(tb)
    doTestHash(rec, false, true)
    for i in 2..CHECK_MAX_ID
      tb.seekNext()
      expect(tb.stat()).to eq 0
      rec = tb.fields()
      doTestHash(rec, false, true, i)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_VAL_BOTH
    for i in 0...CHECK_MAX_ID
      rec = rs[i]
      doTestHash(rec, true, true, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch with FETCH_OBJ' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_OBJ
    rec = seekId1(tb)
    doTestObject(rec, false)
    for i in 2..CHECK_MAX_ID
      tb.seekNext()
      expect(tb.stat()).to eq 0
      rec = tb.fields()
      doTestObject(rec, false, i)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_OBJ
    for i in 0...CHECK_MAX_ID
      rec = rs[i]
      doTestObject(rec, true, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch with FETCH_USR_CLASS' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # receiver classes
    User_NA_NUO = Class.new(UserBase).setAlias(false).set_nodefine_original(false)
    User_NA__UO = Class.new(UserBase).setAlias(false).set_nodefine_original(true)
    User__A_NUO = Class.new(UserBase).setAlias(true).set_nodefine_original(false)
    User__A__UO = Class.new(UserBase).setAlias(true).set_nodefine_original(true)
    # receiver classes are not initialized by Transactd yet
    expect(User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # open activeTable and not aliased recordset
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    at.release()
    # (1) User_NA_NUO : not aliased, not nodefine_original
    doTestTable(db, User_NA_NUO)
    doTestRecordset(rs, User_NA_NUO)
    expect(User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # (2) User_NA__UO : not aliased, nodefine_original
    doTestTable(db, User_NA__UO)
    doTestRecordset(rs, User_NA__UO)
    expect(User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # open activeTable and aliased recordset
    at = openActiveTable(db, User__A_NUO.alias_map)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    at.release()
    
    # (3) User__A_NUO : aliased, not nodefine_original
    doTestTable(db, User__A_NUO)
    doTestRecordset(rs, User__A_NUO)
    expect(User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # (4) User__A__UO : aliased, nodefine_original
    doTestTable(db, User__A__UO)
    doTestRecordset(rs, User__A__UO)
    expect(User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(User__A__UO.instance_variable_get(:@_accessor_initialized)).to be true
    # close all
    db.close()
  end
  
  it 'fetch with FETCH_RECORD_INTO' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    rec = seekId1(tb)
    doTestFieldsBase(rec, false)
    for i in 2..CHECK_MAX_ID
      tb.seekNext()
      expect(tb.stat()).to eq 0
      rec = tb.fields()
      doTestFieldsBase(rec, false, i)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSet(at)
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    for i in 0...CHECK_MAX_ID
      rec = rs[i]
      doTestFieldsBase(rec, true, i + 1)
    end
    at.release()
    # recordset::getRecord()
    at = openActiveTable(db)
    rs = getRecordSet(at)
    for i in 0...CHECK_MAX_ID
      rs.fetchMode = FETCHMODES[i % FETCHMODES.length]
      rec = rs.getRecord(i)
      doTestFieldsBase(rec, true, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch ALL with FETCH_VAL_NUM' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_NUM
    arr = findAll(tb)
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestArray(rec, i + 1)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_VAL_NUM
    arr = rs.to_a
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestArray(rec, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch ALL with FETCH_VAL_ASSOC' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_ASSOC
    arr = findAll(tb)
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestHash(rec, false, false, i + 1)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_VAL_ASSOC
    arr = rs.to_a
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestHash(rec, true, false, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch ALL with FETCH_VAL_BOTH' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_VAL_BOTH
    arr = findAll(tb)
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestHash(rec, false, true, i + 1)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_VAL_BOTH
    arr = rs.to_a
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestHash(rec, true, true, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch ALL with FETCH_OBJ' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_OBJ
    arr = findAll(tb)
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestObject(rec, false, i + 1)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_OBJ
    arr = rs.to_a
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      doTestObject(rec, true, i + 1)
    end
    at.release()
    db.close()
  end
  
  it 'fetch ALL with FETCH_USR_CLASS' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # receiver classes
    AR_User_NA_NUO = Class.new(UserBase).setAlias(false).set_nodefine_original(false)
    AR_User_NA__UO = Class.new(UserBase).setAlias(false).set_nodefine_original(true)
    AR_User__A_NUO = Class.new(UserBase).setAlias(true).set_nodefine_original(false)
    AR_User__A__UO = Class.new(UserBase).setAlias(true).set_nodefine_original(true)
    # receiver classes are not initialized by Transactd yet
    expect(AR_User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # open table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    # open activeTable and not aliased recordset
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    at.release()
    # (1) AR_User_NA_NUO : not aliased, not nodefine_original
    doTestTable(db, AR_User_NA_NUO)
    doTestRecordset(rs, AR_User_NA_NUO)
    expect(AR_User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # (2) AR_User_NA__UO : not aliased, nodefine_original
    doTestTable(db, AR_User_NA__UO)
    doTestRecordset(rs, AR_User_NA__UO)
    expect(AR_User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be_nil
    expect(AR_User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # open activeTable and aliased recordset
    at = openActiveTable(db, AR_User__A_NUO.alias_map)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    at.release()
    # (3) AR_User__A_NUO : aliased, not nodefine_original
    doTestTable(db, AR_User__A_NUO)
    doTestRecordset(rs, AR_User__A_NUO)
    expect(AR_User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User__A__UO.instance_variable_get(:@_accessor_initialized)).to be_nil
    # (4) AR_User__A__UO : aliased, nodefine_original
    doTestTable(db, AR_User__A__UO)
    doTestRecordset(rs, AR_User__A__UO)
    expect(AR_User_NA_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User_NA__UO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User__A_NUO.instance_variable_get(:@_accessor_initialized)).to be true
    expect(AR_User__A__UO.instance_variable_get(:@_accessor_initialized)).to be true
    # close all
    tb.close()
    db.close()
  end
  
  it 'fetch ALL with FETCH_RECORD_INTO (same as FETCH_VAL_BOTH)' do
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_VALUE)
    db = Transactd::Database.new()
    openDatabase(db)
    # table
    tb = openTable(db)
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    arr = findAll(tb)
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      # same as FETCH_VAL_BOTH, not doTestFieldsBase(rec, false, i + 1)
      doTestHash(rec, false, true, i + 1)
    end
    tb.close()
    # activeTable
    at = openActiveTable(db)
    rs = getRecordSetForFindAll(at)
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    arr = rs.to_a
    expect(arr.is_a?(Array)).to eq true
    expect(arr.length).to eq CHECK_MAX_ID
    for i in 0...arr.length
      rec = arr[i]
      # same as FETCH_VAL_BOTH, not doTestFieldsBase(rec, true, i + 1)
      doTestHash(rec, true, true, i + 1)
    end
    at.release()
    db.close()
  end
end
