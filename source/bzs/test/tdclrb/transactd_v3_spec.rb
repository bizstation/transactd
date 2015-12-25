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
RECORD_KEYVALUE_FIELDOBJECT = 1

require 'rbconfig'
IS_WINDOWS = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/)

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
PASSPART2 = PASSWORD == '' ? '' : '?pwd=' + PASSWORD
DBNAME = 'testv3'
TABLENAME = 'user'
PROTOCOL = 'tdap://'
BDFNAME = '?dbfile=test.bdf'
URL = PROTOCOL + USERPART + HOSTNAME + DBNAME + BDFNAME + PASSPART

def dropDatabase(db)
  db.open(URL)
  expect(db.stat()).to eq 0
  db.drop()
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

def createUserTable(db)
  dbdef = db.dbDef()
  expect(dbdef).not_to eq nil
  td = Transactd::Tabledef.new()
  td.schemaCodePage = Transactd::CP_UTF8
  td.setTableName(TABLENAME)
  td.setFileName(TABLENAME + '.dat')
  td.charsetIndex = Transactd::CHARSET_UTF8
  tableid = 1
  td.id = tableid
  td.pageSize = 2048
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  fieldIndex = 0
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('id')
  fd.type = Transactd::Ft_autoinc
  fd.len = 4
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('名前')
  fd.type = Transactd::Ft_myvarchar
  fd.len = 2
  expect(fd.isValidCharNum()).to eq false
  fd.setLenByCharnum(20)
  expect(fd.isValidCharNum()).to eq true
  fd.setDefaultValue("John")
  expect(fd.isPadCharType()).to eq false
  expect(fd.isDateTimeType()).to eq false
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('group')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  fd.setDefaultValue(10)
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('tel')
  fd.type = Transactd::Ft_myvarchar
  fd.len = 4
  fd.setLenByCharnum(21)
  fd.setNullable(true)
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('update_datetime')
  fd.type = Transactd::Ft_mytimestamp
  fd.len = 7
  fd.setDefaultValue(Transactd::DFV_TIMESTAMP_DEFAULT)
  fd.setTimeStampOnUpdate(true)
  expect(fd.isTimeStampOnUpdate()).to eq true
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('create_datetime')
  fd.type = Transactd::Ft_mytimestamp
  fd.len = 4
  fd.setDefaultValue(Transactd::DFV_TIMESTAMP_DEFAULT)
  fd.setTimeStampOnUpdate(false)
  expect(fd.isTimeStampOnUpdate()).to eq false
  expect(fd.isPadCharType()).to eq false
  expect(fd.isDateTimeType()).to eq true
  keynum = 0
  kd = dbdef.insertKey(tableid, keynum)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  td = dbdef.tableDefs(tableid)
  td.primaryKeyNum = keynum
  keynum += 1
  kd = dbdef.insertKey(tableid, keynum)
  kd.segment(0).fieldNum = 2
  kd.segment(0).flags.bit0 = 1 # key_duplicate
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  dbdef.updateTableDef(tableid)
  expect(dbdef.stat()).to eq 0
end

def createUserExtTable(db)
  openDatabase(db)
  dbdef = db.dbDef()
  expect(dbdef).not_to eq nil
  td = Transactd::Tabledef.new()
  td.schemaCodePage = Transactd::CP_UTF8
  td.setTableName("extention")
  td.setFileName("extention")
  td.charsetIndex = Transactd::CHARSET_UTF8
  tableid = 3
  td.id = tableid
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  fieldIndex = 0
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('comment')
  fd.type = Transactd::Ft_myvarchar
  fd.setLenByCharnum(60)
  fd.setNullable(true)
  expect(fd.isDefaultNull()).to eq true
  keynum = 0
  kd = dbdef.insertKey(tableid, keynum)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  td = dbdef.tableDefs(tableid)
  td.primaryKeyNum = keynum
  dbdef.updateTableDef(tableid)
  expect(dbdef.stat()).to eq 0
end

def insertData(db)
  tb = db.openTable("user", Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  tb3 = db.openTable("extention", Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
  begin
    db.beginTrn()
    tb.clearBuffer()
    for i in 1..1000
      tb.setFV(0,  i)
      tb.setFV(1, i.to_s() + " user")
      tb.setFV(2, ((i-1) % 5)+1)
      tb.insert()
    end
    tb3.clearBuffer()
    for i in 1..1000
      tb3.setFV(0,  i)
      tb3.setFV(1, i.to_s() + " comment")
      tb3.insert()
    end
    db.endTrn()
    tb.close()
    tb3.close()
  rescue => e
    db.abortTrn()
    tb.close()
    tb3.close()
    expect(true).to eq false
  end
end

def openTableOnce(db)
  tb = db.openTable("user") # open table to get table schema information.
  tb.close()                # schema information is cached until db.close,
  tb.release()              # if table is closed and table object is released.
end

describe Transactd, 'V3Features' do
  it 'create tables' do
    db = Transactd::Database.new()
    createDatabase(db)
    openDatabase(db)
    createUserTable(db)
    createUserExtTable(db)
    db.close()
  end
  
  it 'insert data' do
    db = Transactd::Database.new()
    openDatabase(db)
    insertData(db)
    db.close()
  end
  
  it 'set mode' do
    db = Transactd::Database.new()
    openDatabase(db)
    db.setAutoSchemaUseNullkey(true)
    expect(db.autoSchemaUseNullkey()).to eq true
    db.setAutoSchemaUseNullkey(false)
    expect(db.autoSchemaUseNullkey()).to eq false
    expect(Transactd::Database::comaptibleMode()).to eq Transactd::Database::CMP_MODE_MYSQL_NULL
    Transactd::Database::setCompatibleMode(Transactd::Database::CMP_MODE_OLD_NULL)
    expect(Transactd::Database::comaptibleMode()).to eq Transactd::Database::CMP_MODE_OLD_NULL
    Transactd::Database::setCompatibleMode(Transactd::Database::CMP_MODE_MYSQL_NULL)
    expect(Transactd::Database::comaptibleMode()).to eq Transactd::Database::CMP_MODE_MYSQL_NULL
    db.close()
  end
  
  it 'check' do
    Transactd::Database::setCompatibleMode(Transactd::Database::CMP_MODE_MYSQL_NULL)
    db = Transactd::Database.new()
    openDatabase(db)
    openTableOnce(db)
    dbdef = db.dbDef()
    td = dbdef.tableDefs(1)
    # isMysqlNullMode
    expect(td.isMysqlNullMode()).to eq true
    expect(td.recordlen()).to eq 145
    # InUse
    expect(td.inUse()).to eq 0
    # nullfields
    expect(td.nullfields()).to eq 1
    # size
    expect(td.size()).to eq 1184
    # fieldNumByName
    expect(td.fieldNumByName("tel")).to eq 3
    # default value
    fd = td.fieldDef(1)
    expect(fd.defaultValue()).to eq "John"
    fd = td.fieldDef(2)
    expect(fd.defaultValue()).to eq "10"
    fd = td.fieldDef(3)
    expect(fd.isDefaultNull()).to eq true
    fd = td.fieldDef(4)
    expect(fd.isTimeStampOnUpdate()).to eq true
    fd = td.fieldDef(5)
    expect(fd.isTimeStampOnUpdate()).to eq false
    # synchronizeSeverSchema
    fd = td.fieldDef(1)
    len = fd.len
    fd.setLenByCharnum(19)
    expect(len).not_to eq fd.len
    dbdef.synchronizeSeverSchema(1) 
    td = dbdef.tableDefs(1)
    fd = td.fieldDef(1)
    expect(len).to eq fd.len
    # syncronize default value
    fd = td.fieldDef(1)
    expect(fd.defaultValue()).to eq "John"
    fd = td.fieldDef(2)
    expect(fd.defaultValue()).to eq "10"
    fd = td.fieldDef(3)
    expect(fd.isDefaultNull()).to eq true
    fd = td.fieldDef(4)
    expect(fd.isTimeStampOnUpdate()).to eq true
    fd = td.fieldDef(5)
    expect(fd.isTimeStampOnUpdate()).to eq false
    
    db.close()
  end
  
  it 'null' do
    db = Transactd::Database.new()
    openDatabase(db)
    dbdef = db.dbDef()
    td = dbdef.tableDefs(1)
    # nullable
    fd = td.fieldDef(3)
    expect(fd.isNullable()).to eq true
    q = Transactd::Query.new()
    atu = Transactd::ActiveTable.new(db, "user")
    Transactd::setRecordValueMode(Transactd::RECORD_KEYVALUE_FIELDOBJECT)
    # isNull setNull
    atu.alias("名前", "name")
    q.select("id", "name", "group", "tel").where("id", "<=", 10)
    rs = atu.index(0).keyValue(1).read(q)
    expect(rs.count()).to eq 10
    rec = rs.first()
    expect(rec[3].isNull()).to eq true
    rec[3].setNull(false)
    expect(rec[3].isNull()).to eq false
    # Join null
    q.reset()
    q.select("comment").optimize(Transactd::QueryBase::JoinHasOneOrHasMany)
    ate = Transactd::ActiveTable.new(db, "extention")
    ate.index(0).join(rs, q, "id")
    last = rs.reverse().first()
    expect(rs.count()).to eq 10
    expect(last["id"].i()).to eq 10
    expect(last["id"].i64()).to eq 10
    expect(last["id"].d()).to eq 10
    expect(rec[4].isNull()).to eq false
    rec[4].setNull(true)
    expect(rec[4].isNull()).to eq true
    # WritableRecord.clear()
    wr = atu.getWritableRecord()
    wr.clear()
    wr["id"].setValue(5)
    wr["tel"].setValue("0236-99-9999")
    wr.update()
    wr.clear()
    wr["id"].setValue(5)
    expect(wr.read()).to eq true
    expect(wr["tel"].str()).to eq "0236-99-9999"
    # whereIsNull
    q.reset()
    q.select("id", "tel").whereIsNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 999
    # whereIsNotNull
    q.reset()
    q.select("id", "tel").whereIsNotNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 1
    # AndIsNull
    q.reset()
    q.select("id", "tel").where("id", "<=", 10).andIsNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 9
    # AndIsNotNull
    q.reset()
    q.select("id", "tel").where("id", "<", 10).andIsNotNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 1
    # OrIsNull
    q.reset()
    q.select("id", "tel").where("id", "<=", 10).orIsNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 1000
    # OrIsNotNull
    q.reset()
    q.select("id", "tel").where("id", "<=", 10).orIsNotNull("tel").reject(0xFFFF)
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 10
    # test recordset query
    q.reset()
    q.select("id", "name", "group", "tel")
    rs = atu.index(0).keyValue(0).read(q)
    expect(rs.count()).to eq 1000
    # recordset whenIsNull
    rq = Transactd::RecordsetQuery.new()
    rq.whenIsNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 999
    # recordset whenIsNotNull
    rq.reset()
    rq.whenIsNotNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 1
    #recordset andIsNull
    rq.reset()
    rq.when("id", "<=", 10).andIsNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 9
    # recordset andIsNotNull
    rq.reset()
    rq.when("id", "<", 10).andIsNotNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 1
    # recordset orIsNull
    rq.reset()
    rq.when("id", "<=", 10).orIsNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 1000
    # recordset orIsNotNull
    rq.reset()
    rq.when("id", "<=", 10).orIsNotNull("tel")
    rs2 = rs.clone()
    rs2 = rs2.matchBy(rq)
    expect(rs2.count()).to eq 10
    # setBin bin
    hex_str = ['FF01FF02']
    bin = hex_str.pack('H*')
    wr["tel"].setBin(bin)
    ret = wr["tel"].bin()
    expect(ret.unpack('H*')[0].upcase()).to eq hex_str[0]
    atu.release()
    ate.release()
    db.close()
  end
  
  it 'default null' do
    db = Transactd::Database.new()
    openDatabase(db)
    dbdef = db.dbDef()
    td = dbdef.tableDefs(1)
    # table::default NULL
    tb = db.openTable("user")
    expect(db.stat()).to eq 0
    tb.setKeyNum(0)
    tb.clearBuffer()
    expect(tb.getFVNull(3)).to eq true
    tb.clearBuffer(Transactd::Table::ClearNull)
    expect(tb.getFVNull(3)).to eq false
    # table NULL
    tb.setFV("id", 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    expect(tb.getFVNull(3)).to eq true
    expect(tb.getFVNull("tel")).to eq true
    tb.setFVNull(3, false)
    expect(tb.getFVNull(3)).to eq false
    tb.setFVNull("tel", true)
    expect(tb.getFVNull("tel")).to eq true
    expect(tb.tableDef().isMysqlNullMode()).to eq true
    expect(td.inUse()).to eq 1
    tb.close()
    db.close()
  end
end
