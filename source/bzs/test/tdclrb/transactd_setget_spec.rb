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

INT_VALUE = 10
INT_VALUE_AS_DBL = 10.0
INT_VALUE_AS_STR = '10'
INT_NULL = 0
INT_NULL_AS_DBL = 0.0
INT_NULL_AS_STR = '0'

STR_VALUE = 'テスト'
STR_NULL = ''
STR_VALUE_AS_BIN = ["e38386e382b9e38388"].pack('H*')

BIN_VALUE = ["e38386e382b900e38388"].pack('H*')
BIN_VALUE_AS_STR = 'テス' # cut off after 0x00
BIN_NULL = ''

SJIS_VALUE = STR_VALUE.encode("Shift_JIS")
SJIS_NULL = STR_NULL.encode("Shift_JIS")

SJIS_BIN_VALUE = ["83658358008367"].pack('H*')
SJIS_BIN_VALUE_AS_STR = BIN_VALUE_AS_STR.encode("Shift_JIS") # cut off after 0x00
SJIS_BIN_NULL = BIN_NULL.encode("Shift_JIS")

FI_numNotNull   = 1
FI_numNullable  = 2
FI_strNotNull   = 3
FI_strNullable  = 4
FI_binNotNull   = 5
FI_binNullable  = 6
FI_binSJISNotNull   = 7
FI_binSJISNullable  = 8

HOSTNAME = getHost()
USERNAME = getEnv('TRANSACTD_RSPEC_USER')
USERPART = USERNAME == '' ? '' : USERNAME + '@'
PASSWORD = getEnv('TRANSACTD_RSPEC_PASS')
PASSPART = PASSWORD == '' ? '' : '&pwd=' + PASSWORD
DBNAME = 'testsetget'
TABLENAME = 'setget'
URL = 'tdap://' + USERPART + HOSTNAME + DBNAME + '?dbfile=transactd_schema' + PASSPART

def createTable(db)
  dbdef = db.dbDef()
  td = Transactd::Tabledef.new()
  td.charsetIndex = Transactd::CHARSET_UTF8
  td.setTableName(TABLENAME)
  td.setFileName(TABLENAME + '.dat')
  table_id = 1
  td.id = table_id
  td.pageSize = 2048
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # 0: id
  fd = dbdef.insertField(table_id, 0)
  fd.setName('id')
  fd.type = Transactd::Ft_autoinc
  fd.len = 4
  fd.setNullable(false)
  # 1: numNotNull
  fd = dbdef.insertField(table_id, FI_numNotNull)
  fd.setName('numNotNull')
  fd.type = Transactd::Ft_integer
  fd.len = 1
  fd.setNullable(false)
  # 2: numNullable
  fd = dbdef.insertField(table_id, FI_numNullable)
  fd.setName('numNullable')
  fd.type = Transactd::Ft_integer
  fd.len = 1
  fd.setNullable(true)
  # 3: strNotNull
  fd = dbdef.insertField(table_id, FI_strNotNull)
  fd.setName('strNotNull')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(40)
  fd.setNullable(false)
  # 4: strNullable
  fd = dbdef.insertField(table_id, FI_strNullable)
  fd.setName('strNullable')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(40)
  fd.setNullable(true)
  # 5: binNotNull
  fd = dbdef.insertField(table_id, FI_binNotNull)
  fd.setName('binNotNull')
  fd.type = Transactd::Ft_myblob
  fd.len = 9
  fd.setNullable(false)
  fd.setCharsetIndex(Transactd::charsetIndex("binary"))
  # 6: binNullable
  fd = dbdef.insertField(table_id, FI_binNullable)
  fd.setName('binNullable')
  fd.type = Transactd::Ft_myblob
  fd.len = 9
  fd.setNullable(true)
  fd.setCharsetIndex(Transactd::charsetIndex("binary"))
  # 7: binSJISNotNull
  fd = dbdef.insertField(table_id, FI_binSJISNotNull)
  fd.setName('binSJISNotNull')
  fd.type = Transactd::Ft_myblob
  fd.len = 9
  fd.setNullable(false)
  fd.setCharsetIndex(Transactd::charsetIndex("cp932"))
  # 8: binSJISNullable
  fd = dbdef.insertField(table_id, FI_binSJISNullable)
  fd.setName('binSJISNullable')
  fd.type = Transactd::Ft_myblob
  fd.len = 9
  fd.setNullable(true)
  fd.setCharsetIndex(Transactd::charsetIndex("cp932"))
  # primary key
  kd = dbdef.insertKey(table_id, 0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  td.primaryKeyNum = 0
  # save schema
  dbdef.updateTableDef(table_id)
  expect(dbdef.stat()).to eq 0
  # open table
  tb = db.openTable(TABLENAME)
  # insert test data
  #   id:1
  tb.clearBuffer()
  tb.setFV(FI_numNotNull,     INT_VALUE)
  tb.setFV(FI_strNotNull,     STR_VALUE)
  tb.setFV(FI_binNotNull,     BIN_VALUE,  BIN_VALUE.length)
  tb.setFV(FI_binSJISNotNull, STR_VALUE)
  tb.insert()
  expect(tb.stat()).to eq 0
  #   id:2
  tb.clearBuffer()
  tb.setFV(FI_numNotNull,       INT_VALUE)
  tb.setFV(FI_numNullable,      INT_VALUE)
  tb.setFV(FI_strNotNull,       STR_VALUE)
  tb.setFV(FI_strNullable,      BIN_VALUE)
  tb.setFV(FI_binNotNull,       STR_VALUE)
  tb.setFV(FI_binNullable,      BIN_VALUE, BIN_VALUE.length)
  tb.setFV(FI_binSJISNotNull,   STR_VALUE)
  tb.setFV(FI_binSJISNullable,  SJIS_BIN_VALUE, SJIS_BIN_VALUE.length)
  tb.insert()
  expect(tb.stat()).to eq 0
  tb.close
end

def createDatabase
  db = Transactd::Database.new()
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  if db.stat() != Transactd::ERROR_NO_DATABASE
    expect(db.stat()).to eq 0
    db.drop()
    expect(db.stat()).to eq 0
  end
  db.create(URL)
  expect(db.stat()).to eq 0
  db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  createTable(db)
  db.close
end

def testTableGetFVX(tb)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  # numNotNull
  expect(tb.getFVNull(FI_numNotNull)).to be false
  expect( tb.getFVint(FI_numNotNull)).to eq INT_VALUE
  expect(  tb.getFV64(FI_numNotNull)).to eq INT_VALUE
  expect( tb.getFVdbl(FI_numNotNull)).to eq INT_VALUE_AS_DBL
  expect( tb.getFVstr(FI_numNotNull)).to eq INT_VALUE_AS_STR
  # numNullable
  expect(tb.getFVNull(FI_numNullable)).to be true
  expect( tb.getFVint(FI_numNullable)).to eq INT_NULL
  expect(  tb.getFV64(FI_numNullable)).to eq INT_NULL
  expect( tb.getFVdbl(FI_numNullable)).to eq INT_NULL_AS_DBL
  expect( tb.getFVstr(FI_numNullable)).to eq INT_NULL_AS_STR
  # strNotNull
  expect(tb.getFVNull(FI_strNotNull)).to be false
  expect( tb.getFVstr(FI_strNotNull)).to eq STR_VALUE
  bin =   tb.getFVbin(FI_strNotNull)
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin.force_encoding("utf-8")).to eq STR_VALUE
  # strNullable
  expect(tb.getFVNull(FI_strNullable)).to be true
  expect( tb.getFVstr(FI_strNullable)).to eq STR_NULL
  bin =   tb.getFVbin(FI_strNullable)
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin.force_encoding("utf-8")).to eq STR_NULL
  # binNotNull
  expect(tb.getFVNull(FI_binNotNull)).to be false
  expect( tb.getFVstr(FI_binNotNull)).to eq BIN_VALUE_AS_STR
  bin =   tb.getFVbin(FI_binNotNull)
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin).to eq BIN_VALUE
  # binNullable
  expect(tb.getFVNull(FI_binNullable)).to be true
  expect( tb.getFVstr(FI_binNullable)).to eq BIN_NULL
  bin =   tb.getFVbin(FI_binNullable)
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin).to eq BIN_NULL
  # binSJISNotNull
  expect(tb.getFVNull(FI_binSJISNotNull)).to be false
  expect( tb.getFVstr(FI_binSJISNotNull)).to eq STR_VALUE # getFVstr converts value to utf_8 from shift_jis.
  bin =   tb.getFVbin(FI_binSJISNotNull)                  # getFVbin does not convert encoding.
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin.force_encoding("Shift_JIS")).to eq SJIS_VALUE
  # binSJISNotNull
  expect(tb.getFVNull(FI_binSJISNullable)).to be true
  expect( tb.getFVstr(FI_binSJISNullable)).to eq STR_NULL
  bin =   tb.getFVbin(FI_binSJISNullable)
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin.force_encoding("Shift_JIS")).to eq STR_NULL
end

def testTableSetFV(tb)
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  # SET VALUE AS BINARY
  #   setFV does not convert the values if length parameter is passed.
  tb.setFV(FI_binSJISNotNull, SJIS_BIN_VALUE, SJIS_BIN_VALUE.length)  # SJIS_BIN_VALUE will not be converted.
  str = tb.getFVstr(FI_binSJISNotNull)  # getFVstr converts value to utf_8 from shift_jis.
  expect(str.encoding).to eq Encoding::UTF_8
  expect(str).to eq BIN_VALUE_AS_STR    # the string value will end with null (0x00) character.
  bin = tb.getFVbin(FI_binSJISNotNull)  # getFVbin does not convert encoding.
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin).to eq SJIS_BIN_VALUE      # the binary value can contain null (0x00) characters.
  # SET VALUE AS STRING
  #   setFV converts the values if length parameter is not passed.
  tb.setFV(FI_binSJISNotNull, BIN_VALUE)  # BIN_VALUE will be converted to Shift_JIS from utf_8.
  str = tb.getFVstr(FI_binSJISNotNull)    # getFVstr converts value to utf_8 from shift_jis.
  expect(str.encoding).to eq Encoding::UTF_8
  expect(str).to eq BIN_VALUE_AS_STR      # the string value will end with null (0x00) character.
  bin = tb.getFVbin(FI_binSJISNotNull)    # getFVbin does not convert encoding.
  expect(bin.encoding).to eq Encoding::ASCII_8BIT
  expect(bin.force_encoding("Shift_JIS")).to eq SJIS_BIN_VALUE_AS_STR
    # the value end with null (0x00) character on setFV (because it regarded as string).
end

def testTableGetMethods_CheckArrayValue(arr)
  # string values
  expect(arr[FI_strNotNull].encoding).to eq Encoding::UTF_8
  expect(arr[FI_strNotNull]).to eq STR_VALUE
  expect(arr[FI_strNullable].encoding).to eq Encoding::UTF_8
  expect(arr[FI_strNullable]).to eq BIN_VALUE_AS_STR # cut by null
  # binary values
  expect(arr[FI_binNotNull].encoding).to eq Encoding::ASCII_8BIT
  expect(arr[FI_binNotNull]).to eq STR_VALUE_AS_BIN
  expect(arr[FI_binNullable].encoding).to eq Encoding::ASCII_8BIT
  expect(arr[FI_binNullable]).to eq BIN_VALUE
  # shift_jis string in binary field (automatically converted into utf_8)
  expect(arr[FI_binSJISNotNull].encoding).to eq Encoding::UTF_8
  expect(arr[FI_binSJISNotNull]).to eq STR_VALUE
  expect(arr[FI_binSJISNullable].encoding).to eq Encoding::UTF_8
  expect(arr[FI_binSJISNullable]).to eq BIN_VALUE_AS_STR # cut by null
end

def testTableGetMethods_CheckObjValue(obj)
  # string values
  expect(obj.strNotNull.encoding).to eq Encoding::UTF_8
  expect(obj.strNotNull).to eq STR_VALUE
  expect(obj.strNullable.encoding).to eq Encoding::UTF_8
  expect(obj.strNullable).to eq BIN_VALUE_AS_STR # cut by null
  # binary values
  expect(obj.binNotNull.encoding).to eq Encoding::ASCII_8BIT
  expect(obj.binNotNull).to eq STR_VALUE_AS_BIN
  expect(obj.binNullable.encoding).to eq Encoding::ASCII_8BIT
  expect(obj.binNullable).to eq BIN_VALUE
  # shift_jis string in binary field (automatically converted into utf_8)
  expect(obj.binSJISNotNull.encoding).to eq Encoding::UTF_8
  expect(obj.binSJISNotNull).to eq STR_VALUE
  expect(obj.binSJISNullable.encoding).to eq Encoding::UTF_8
  expect(obj.binSJISNullable).to eq BIN_VALUE_AS_STR # cut by null
end

def testTableGetMethods(tb)
  tb.setKeyNum(0)
  expect(tb.stat()).to eq 0
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.stat()).to eq 0
  Transactd::set_field_value_mode(Transactd::FIELD_VALUE_MODE_VALUE)
  # get values as array
  tb.fetchMode = Transactd::FETCH_VAL_NUM
  testTableGetMethods_CheckArrayValue(tb.fields)
  # get values as obj
  tb.fetchMode = Transactd::FETCH_OBJ
  testTableGetMethods_CheckObjValue(tb.fields)
  # findAll
  q = Transactd::Query.new()
  tb.clearBuffer()
  expect(tb.stat()).to eq 0
  tb.setQuery(q)
  expect(tb.stat()).to eq 0
  # get values as array
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.fetchMode = Transactd::FETCH_VAL_NUM
  arr = tb.findAll
  expect(arr.length).to eq 2
  testTableGetMethods_CheckArrayValue(arr[1])
  # get values as obj
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.fetchMode = Transactd::FETCH_OBJ
  arr = tb.findAll
  expect(arr.length).to eq 2
  testTableGetMethods_CheckObjValue(arr[1])
end

def testField(tb)
  tb.setKeyNum(0)
  expect(tb.stat()).to eq 0
  tb.seekFirst()
  expect(tb.stat()).to eq 0
  tb.seekNext()
  expect(tb.stat()).to eq 0
  Transactd::field_value_mode = Transactd::FIELD_VALUE_MODE_VALUE
  tb.fetchMode = Transactd::FETCH_RECORD_INTO
  rec = tb.fields
  # get with []
  testTableGetMethods_CheckArrayValue(rec)
  # set with []=
  rec[FI_strNotNull] = STR_VALUE
  expect(rec[FI_strNotNull].encoding).to eq Encoding::UTF_8
  expect(rec[FI_strNotNull]).to eq STR_VALUE
  rec[FI_strNullable] = STR_VALUE_AS_BIN
  expect(rec[FI_strNullable].encoding).to eq Encoding::UTF_8
  expect(rec[FI_strNullable]).to eq STR_VALUE
  rec[FI_binNotNull] = BIN_VALUE
  expect(rec[FI_binNotNull].encoding).to eq Encoding::ASCII_8BIT
  expect(rec[FI_binNotNull]).to eq BIN_VALUE
  rec[FI_binNullable] = STR_VALUE
  expect(rec[FI_binNullable].encoding).to eq Encoding::ASCII_8BIT
  expect(rec[FI_binNullable]).to eq STR_VALUE_AS_BIN
  rec[FI_binSJISNotNull] = STR_VALUE
  expect(rec[FI_binSJISNotNull].encoding).to eq Encoding::UTF_8
  expect(rec[FI_binSJISNotNull]).to eq STR_VALUE
  rec[FI_binSJISNullable] = STR_VALUE
  expect(rec[FI_binSJISNullable].encoding).to eq Encoding::UTF_8
  expect(rec[FI_binSJISNullable]).to eq STR_VALUE
  # set with set_value_by_object(obj)
  tb.fetchMode = Transactd::FETCH_OBJ
  obj = tb.fields
  obj.strNotNull = STR_VALUE
  obj.strNullable = BIN_VALUE
  obj.binNotNull = STR_VALUE
  obj.binNullable = BIN_VALUE
  obj.binSJISNotNull = STR_VALUE
  obj.binSJISNullable = BIN_VALUE
  tb.clearBuffer
  rec.set_value_by_object(obj)
  testTableGetMethods_CheckArrayValue(rec)
end

def testRecordset(rs)
  expect(rs.size).to eq 2
  # get with [] as array
  rs.fetchMode = Transactd::FETCH_VAL_NUM
  testTableGetMethods_CheckArrayValue(rs[1])
  # get with [] as obj
  rs.fetchMode = Transactd::FETCH_OBJ
  testTableGetMethods_CheckObjValue(rs[1])
  # get with to_a as array
  rs.fetchMode = Transactd::FETCH_VAL_NUM
  arr = rs.to_a
  expect(arr.length).to eq 2
  testTableGetMethods_CheckArrayValue(arr[1])
  # get with to_a as obj
  rs.fetchMode = Transactd::FETCH_OBJ
  arr = rs.to_a
  expect(arr.length).to eq 2
  testTableGetMethods_CheckObjValue(arr[1])
end

describe Transactd, 'set get values' do
  it 'prepare database and table' do
    createDatabase
  end
  
  it 'get values from table' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    expect(db.stat()).to eq 0
    tb = db.openTable(TABLENAME)
    expect(db.stat()).to eq 0
    testTableGetFVX(tb)
    testTableSetFV(tb)
    testTableGetMethods(tb)
    tb.close
    db.close
  end
  
  it 'get/set values from/to record' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    expect(db.stat()).to eq 0
    tb = db.openTable(TABLENAME)
    expect(db.stat()).to eq 0
    testField(tb)
    tb.close
    db.close
  end
  
  it 'get values from recordset' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    expect(db.stat()).to eq 0
    at = Transactd::ActiveTable.new(db, TABLENAME)
    q = Transactd::Query.new()
    rs = at.index(0).keyValue(1).read(q)
    at.release
    testRecordset(rs)
    db.close
  end
end
