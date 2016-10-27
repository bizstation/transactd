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
DBNAME = 'test_v3'
TABLENAME = 'user'
PROTOCOL = 'tdap://'
BDFNAME = '?dbfile=test.bdf'
URL = PROTOCOL + USERPART + HOSTNAME + DBNAME + BDFNAME + PASSPART
URL_AUTOSCHEMA = PROTOCOL + USERPART + HOSTNAME + DBNAME + PASSPART2

SEB_TABLENAME = "setenumbit"
CREATE_SEB_TABLE_SQL = <<'EOS'
  CREATE TABLE `setenumbit` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `set5` set('A','B','C','D','E') DEFAULT '',
    `set64` set('a0', 'a1', 'a2', 'a3', 'a4', 'a5', 'a6', 'a7', 'a8', 'a9',
      'b0', 'b1', 'b2', 'b3', 'b4', 'b5', 'b6', 'b7', 'b8', 'b9',
      'c0', 'c1', 'c2', 'c3', 'c4', 'c5', 'c6', 'c7', 'c8', 'c9',
      'd0', 'd1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7', 'd8', 'd9',
      'e0', 'e1', 'e2', 'e3', 'e4', 'e5', 'e6', 'e7', 'e8', 'e9',
      'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9',
      'g0', 'g1', 'g2', 'g3') DEFAULT '',
    `enum2` enum('Y','N') DEFAULT 'N',
    `enum260` enum('a0', 'a1', 'a2', 'a3', 'a4', 'a5', 'a6', 'a7', 'a8', 'a9',
      'b0', 'b1', 'b2', 'b3', 'b4', 'b5', 'b6', 'b7', 'b8', 'b9',
      'c0', 'c1', 'c2', 'c3', 'c4', 'c5', 'c6', 'c7', 'c8', 'c9',
      'd0', 'd1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7', 'd8', 'd9',
      'e0', 'e1', 'e2', 'e3', 'e4', 'e5', 'e6', 'e7', 'e8', 'e9',
      'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9',
      'g0', 'g1', 'g2', 'g3', 'g4', 'g5', 'g6', 'g7', 'g8', 'g9',
      'h0', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'h7', 'h8', 'h9',
      'i0', 'i1', 'i2', 'i3', 'i4', 'i5', 'i6', 'i7', 'i8', 'i9',
      'j0', 'j1', 'j2', 'j3', 'j4', 'j5', 'j6', 'j7', 'j8', 'j9',
      'k0', 'k1', 'k2', 'k3', 'k4', 'k5', 'k6', 'k7', 'k8', 'k9',
      'l0', 'l1', 'l2', 'l3', 'l4', 'l5', 'l6', 'l7', 'l8', 'l9',
      'm0', 'm1', 'm2', 'm3', 'm4', 'm5', 'm6', 'm7', 'm8', 'm9',
      'n0', 'n1', 'n2', 'n3', 'n4', 'n5', 'n6', 'n7', 'n8', 'n9',
      'o0', 'o1', 'o2', 'o3', 'o4', 'o5', 'o6', 'o7', 'o8', 'o9',
      'p0', 'p1', 'p2', 'p3', 'p4', 'p5', 'p6', 'p7', 'p8', 'p9',
      'q0', 'q1', 'q2', 'q3', 'q4', 'q5', 'q6', 'q7', 'q8', 'q9',
      'r0', 'r1', 'r2', 'r3', 'r4', 'r5', 'r6', 'r7', 'r8', 'r9',
      's0', 's1', 's2', 's3', 's4', 's5', 's6', 's7', 's8', 's9',
      't0', 't1', 't2', 't3', 't4', 't5', 't6', 't7', 't8', 't9',
      'u0', 'u1', 'u2', 'u3', 'u4', 'u5', 'u6', 'u7', 'u8', 'u9',
      'v0', 'v1', 'v2', 'v3', 'v4', 'v5', 'v6', 'v7', 'v8', 'v9',
      'w0', 'w1', 'w2', 'w3', 'w4', 'w5', 'w6', 'w7', 'w8', 'w9',
      'x0', 'x1', 'x2', 'x3', 'x4', 'x5', 'x6', 'x7', 'x8', 'x9',
      'y0', 'y1', 'y2', 'y3', 'y4', 'y5', 'y6', 'y7', 'y8', 'y9',
      'z0', 'z1', 'z2', 'z3', 'z4', 'z5', 'z6', 'z7', 'z8', 'z9') DEFAULT 'a0',
    `bit1` bit(1) DEFAULT b'0',
    `bit8` bit(8) DEFAULT b'0',
    `bit32` bit(32) DEFAULT b'0',
    `bit64` bit(64) DEFAULT b'0',
    PRIMARY KEY (`id`)
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
EOS
INSERT_SEB_TABLE_SQL = <<'EOS'
  INSERT INTO `setenumbit` (`id`, `set5`, `set64`, `enum2`, `enum260`, `bit1`, `bit8`, `bit32`, `bit64`) VALUES
    (1, 'A', 'a0', 'N', 'a0', b'1', b'1', b'1', b'1'),
    (2, 'A,B,C,D,E', 'a0,g3', 'Y', 'z9', b'1', b'11111111',
      b'11111111111111111111111111111111',
      b'1111111111111111111111111111111111111111111111111111111111111111'),
    (3, '', '', '0', '0', b'0', b'00000000', b'00000000', b'00000000'),
    (4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
    (5, '', 'a0,g1', 'N', 'a0', b'1', b'1', b'1',
      b'0011111111111111111111111111111111111111111111111111111111111111'),
    (6, '', 'a0,g2', 'N', 'a0', b'1', b'1', b'1',
      b'0111111111111111111111111111111111111111111111111111111111111111'),
    (7, '', 'g3', 'N', 'a0', b'1', b'1', b'1',
      b'1000000000000000000000000000000000000000000000000000000000000000');
EOS

def dropDatabase(db)
  #db.open(URL)
  #expect(db.stat()).to eq 0
  #db.drop()
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

def openDatabaseAutoSchema(db)
  return db.open(URL_AUTOSCHEMA, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
end

def isMySQL5_5(db)
  vv = Transactd::BtrVersions.new()
  db.getBtrVersion(vv)
  server_ver = vv.version(1)
  return (db.stat() == 0) &&
    (5 == server_ver.majorVersion) &&
    (5 == server_ver.minorVersion)
end

def isMariaDBWithGtid(db)
  vv = Transactd::BtrVersions.new()
  db.getBtrVersion(vv)
  server_ver = vv.version(1)
  return (db.stat() == 0) &&
    (10 == server_ver.majorVersion) &&
    (server_ver.type == Transactd::MYSQL_TYPE_MARIA)
end


def isLegacyTimeFormat(db)
  vv = Transactd::BtrVersions.new()
  db.getBtrVersion(vv)
  server_ver = vv.version(1)
  return (db.stat() == 0) &&
    (5 == server_ver.majorVersion) &&
    (5 == server_ver.minorVersion) &&
    (server_ver.type == Transactd::MYSQL_TYPE_MYSQL)
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
  fd.setNullable(true, false)
  fd.setDefaultValue(10)
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('tel')
  #fd.type = Transactd::Ft_myvarchar
  #fd.setLenByCharnum(21)
  fd.type = Transactd::Ft_myvarbinary
  fd.len = 64
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
  if (isMySQL5_5(db))
    fd.type = Transactd::Ft_mydatetime
    fd.len = 8
  else
    fd.type = Transactd::Ft_mytimestamp
    fd.len = 4
    fd.setDefaultValue(Transactd::DFV_TIMESTAMP_DEFAULT)
  end
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
  tableid = 2
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
  fieldIndex += 1
  fd = dbdef.insertField(tableid, fieldIndex)
  fd.setName('bits')
  fd.type = Transactd::Ft_integer
  fd.len = 8
  expect(fd.isDefaultNull()).to eq false
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

def createSetEnumBitTable(db)
  tableid = 3
  db.createTable(CREATE_SEB_TABLE_SQL) # RUN SQL
  expect(db.stat()).to eq 0
  db.createTable(INSERT_SEB_TABLE_SQL) # RUN SQL
  expect(db.stat()).to eq 0
end

class User
  attr_accessor :a, :b, :c
  def initialize(_a, _b, _c)
    @a = _a
    @b = _b
    @c = _c
  end
  
  @nodefine_original = false
  def self.nodefine_original
    @nodefine_original ||= false
  end
  def self.nodefine_original=(v)
    @nodefine_original = v
  end
  
  @_alias_map = {}
  def self.alias_map
    @_alias_map = {} if @_alias_map.nil?
    return @_alias_map
  end
  def self.alias_attribute(newname, oldname)
    @_alias_map = {} if @_alias_map.nil?
    @_alias_map[newname.to_sym] = oldname.to_s
  end
  
  def self.reset_class
    remove_instance_variable(:@_alias_map) if self.instance_variables.include?(:@_alias_map)
    remove_instance_variable(:@_accessor_initialized) if self.instance_variables.include?(:@_accessor_initialized)
    ["id", "名前", "名前2", "group", "tel", "update_datetime", "create_datetime"].each { |n|
      remove_method(n.to_sym) if self.instance_methods(false).include?(n.to_sym)
      remove_method("#{n}=".to_sym) if self.instance_methods(false).include?("#{n}=".to_sym)
    }
  end
end

describe Transactd, 'V3Features' do
  it 'test no schema' do
    db = Transactd::Database.new()
    db.open(URL_AUTOSCHEMA)
    if (db.stat == 0)
      db.drop()
      expect(db.stat()).to eq 0
    end
    db.create(URL_AUTOSCHEMA)
    expect(db.stat()).to eq 0
    db.open(URL_AUTOSCHEMA)
    expect(db.stat()).to eq 0
    db.drop()
    expect(db.stat()).to eq 0
  end
  
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
    # default
    expect(Transactd::Database::compatibleMode).to eq Transactd::Database::CMP_MODE_MYSQL_NULL
    # CMP_MODE_OLD_NULL
    Transactd::Database::set_compatible_mode(Transactd::Database::CMP_MODE_OLD_NULL)
    expect(Transactd::Database::compatibleMode).to eq Transactd::Database::CMP_MODE_OLD_NULL
    # CMP_MODE_BINFD_DEFAULT_STR
    Transactd::Database::compatible_mode = Transactd::Database::CMP_MODE_BINFD_DEFAULT_STR
    expect(Transactd::Database::compatibleMode).to eq Transactd::Database::CMP_MODE_BINFD_DEFAULT_STR
    # CMP_MODE_MYSQL_NULL
    Transactd::Database::setCompatibleMode(Transactd::Database::CMP_MODE_MYSQL_NULL)
    expect(Transactd::Database::compatible_mode).to eq Transactd::Database::CMP_MODE_MYSQL_NULL
    db.close()
  end
  
  it 'check' do
    Transactd::Database::setCompatibleMode(Transactd::Database::CMP_MODE_MYSQL_NULL)
    expect(Transactd::Database::compatible_mode).to eq Transactd::Database::CMP_MODE_MYSQL_NULL
    db = Transactd::Database.new()
    openDatabase(db)
    openTableOnce(db)
    dbdef = db.dbDef()
    mysql_5_5 = isMySQL5_5(db)
    td = dbdef.tableDefs(1)
    # isMysqlNullMode
    expect(td.isMysqlNullMode()).to eq true
    # recordlen
    len = 145
    len += 4 if (mysql_5_5)
    len -= 3 if (isLegacyTimeFormat(db))
    expect(td.recordlen()).to eq len
    # size
    expect(td.size()).to eq 1184
    # InUse
    expect(td.inUse()).to eq 0
    # nullfields
    expect(td.nullfields()).to eq 2
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
    expect(fd.defaultValue()).to eq Transactd::DFV_TIMESTAMP_DEFAULT.to_i().to_s()
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
    # getSqlStringForCreateTable
    sql = db.getSqlStringForCreateTable("extention")
    expect(db.stat()).to eq 0
    expect(sql).to eq 'CREATE TABLE `extention` (`id` INT NOT NULL ,`comment` VARCHAR(60) binary NULL DEFAULT NULL,`bits` BIGINT NOT NULL , PRIMARY KEY(`id`)) ENGINE=InnoDB default charset=utf8'
    # setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
    td = dbdef.tableDefs(1)
    td.setValidationTarget(true, 0)
    
    q = Transactd::Query.new()
    atu = Transactd::ActiveTable.new(db, "user")
    # segmentsSizeForInValue
    expect(q.segmentsForInValue(3).getJoinKeySize()).to eq 3
    q.reset()
    expect(q.getJoinKeySize()).to eq 0
    # keyValue null
    tb1 = atu.table()
    tb1.setFV(2, nil)
    expect(tb1.getFVNull(2)).to eq true
    tb1.setFVNull(2, false)
    expect(tb1.getFVNull(2)).to eq false
    atu.index(1).keyValue(nil)
    expect(tb1.getFVNull(2)).to eq true
    # isNull setNull
    atu.alias("名前", "name")
    q.select("id", "name", "group", "tel").where("id", "<=", 10)
    rs = atu.index(0).keyValue(1).read(q)
    expect(rs.count()).to eq 10
    rec = rs.first()
    expect(rec[3].isNull()).to eq true
    rec[3].setNull(false)
    expect(rec[3].isNull()).to eq false
    rec[3].setValue(nil)
    expect(rec[3].isNull()).to eq true
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
    # setValue null
    expect(rec[4].isNull()).to eq true
    rec[4].setNull(false)
    expect(rec[4].isNull()).to eq false
    rec[4].setValue(nil)
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
    hex_str = ['FF00FF02']
    bin = hex_str.pack('H*')
    wr["tel"].setBin(bin)
    ret = wr["tel"].bin()
    expect(bin).to eq ret
    atu.release()
    ate.release()
    db.close()
  end
  
  it 'default null' do
    db = Transactd::Database.new()
    openDatabase(db)
    mysql_5_5 = isMySQL5_5(db)
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
    # setFV null
    tb.setFVNull("tel", true)
    expect(tb.getFVNull("tel")).to eq true
    tb.setFVNull("tel", false)
    expect(tb.getFVNull("tel")).to eq false
    tb.setFV("tel", nil)
    expect(tb.getFVNull("tel")).to eq true
    # timestamp format
    date = Transactd::btrdtoa(Transactd::getNowDate(), true)
    expect(tb.getFVstr("update_datetime")[0...10]).to eq date
    if (! mysql_5_5)
      expect(tb.getFVstr("create_datetime")[0...10]).to eq date
    end
    # setTimestampMode
    tb.setTimestampMode(Transactd::TIMESTAMP_VALUE_CONTROL)
    tb.setTimestampMode(Transactd::TIMESTAMP_ALWAYS)
    # isMysqlNullMode
    expect(tb.tableDef().isMysqlNullMode()).to eq true
    expect(td.inUse()).to eq 1
    tb.close()
    tb.release()
    expect(td.inUse()).to eq 0
    db.close()
  end
  
  it 'bit' do
    db = Transactd::Database.new()
    openDatabase(db)
    tb = db.openTable("extention")
    expect(db.stat()).to eq 0
    tb.setKeyNum(0)
    tb.setFV('id', 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    bits = Transactd::Bitset.new()
    bits[63] = true
    bits[2] =  true
    bits[5] =  true
    tb.setFV('bits', bits)
    tb.update()
    expect(tb.stat()).to eq 0
    q = Transactd::Query.new()
    at = Transactd::ActiveTable.new(db, "extention")
    q.where('id', '=', 1)
    rs = at.index(0).keyValue(1).read(q)
    expect(rs.size()).to eq 1
    bits = rs[0]['bits'].getBits()
    expect(bits.get(63)).to eq true
    expect(bits.get(2)).to eq true
    expect(bits.get(5)).to eq true
    expect(bits.get(62)).to eq false
    expect(bits.get(0)).to eq false
    expect(bits.get(12)).to eq false
    expect(bits[63]).to eq true
    expect(bits[2]).to eq true
    expect(bits[5]).to eq true
    expect(bits[62]).to eq false
    expect(bits[0]).to eq false
    expect(bits[12]).to eq false
    wr = at.getWritableRecord()
    wr['id'] = 1
    bits[63] = false
    bits[12] = true
    bits[0] =  true
    bits[62] =  true
    wr['bits'] = bits
    wr.update()
    tb.setFV('id', 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    bits = tb.getFVbits('bits')
    expect(bits.get(63)).to eq false
    expect(bits.get(2)).to eq true
    expect(bits.get(5)).to eq true
    expect(bits.get(12)).to eq true
    expect(bits.get(0)).to eq true
    expect(bits.get(62)).to eq true
    expect(bits.get(11)).to eq false
    expect(bits.get(13)).to eq false
    expect(bits[63]).to eq false
    expect(bits[2]).to eq true
    expect(bits[5]).to eq true
    expect(bits[12]).to eq true
    expect(bits[0]).to eq true
    expect(bits[62]).to eq true
    expect(bits[11]).to eq false
    expect(bits[13]).to eq false
    tb.close()
    db.close()
  end
  
  it 'bitset' do
    bits1 = Transactd::Bitset.new()
    bits2 = Transactd::Bitset.new()
    bits1[0] = true
    bits1[1] = true
    bits1[63] = true
    bits2[0] = true
    bits2[1] = false
    bits2[63] = true
    expect(bits1.equals(bits2)).to eq false
    expect(bits1.contains(bits2)).to eq true
    expect(bits2.contains(bits1)).to eq false
    all = false
    expect(bits2.contains(bits1, all)).to eq true
  end
  
  it 'set, enum, bit' do
    db = Transactd::Database.new()
    openDatabaseAutoSchema(db)
    expect(db.stat()).to eq 0
    createSetEnumBitTable(db)
    ats = Transactd::ActiveTable.new(db, SEB_TABLENAME)
    q = Transactd::Query.new()
    q.reset().all()
    rs = ats.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 7
    expect(rs[0]['id'].i64()).to eq 1
    expect(rs[0]['set5'].i64()).to eq 1
    expect(rs[0]['set64'].i64()).to eq 1
    expect(rs[0]['enum2'].i64()).to eq 2
    expect(rs[0]['enum260'].i64()).to eq 1
    expect(rs[0]['bit1'].i64()).to eq 1
    expect(rs[0]['bit8'].i64()).to eq 1
    expect(rs[0]['bit32'].i64()).to eq 1
    expect(rs[0]['bit64'].i64()).to eq 1
    expect(rs[1]['id'].i64()).to eq 2
    expect(rs[1]['set5'].i64()).to eq 31
    expect(rs[1]['set64'].i64()).to eq -9223372036854775807
    expect(rs[1]['enum2'].i64()).to eq 1
    expect(rs[1]['enum260'].i64()).to eq 260
    expect(rs[1]['bit1'].i64()).to eq 1
    expect(rs[1]['bit8'].i64()).to eq 0xFF
    expect(rs[1]['bit32'].i64()).to eq 0xFFFFFFFF
    expect(rs[1]['bit64'].i64()).to eq -1
    expect(rs[2]['id'].i64()).to eq 3
    expect(rs[2]['set5'].i64()).to eq 0
    expect(rs[2]['set64'].i64()).to eq 0
    expect(rs[2]['enum2'].i64()).to eq 0
    expect(rs[2]['enum260'].i64()).to eq 0
    expect(rs[2]['bit1'].i64()).to eq 0
    expect(rs[2]['bit8'].i64()).to eq 0
    expect(rs[2]['bit32'].i64()).to eq 0
    expect(rs[2]['bit64'].i64()).to eq 0
    expect(rs[3]['id'].i64()).to eq 4
    expect(rs[3]['set5'].isNull()).to eq true
    expect(rs[3]['set64'].isNull()).to eq true
    expect(rs[3]['enum2'].isNull()).to eq true
    expect(rs[3]['enum260'].isNull()).to eq true
    expect(rs[3]['bit1'].isNull()).to eq true
    expect(rs[3]['bit8'].isNull()).to eq true
    expect(rs[3]['bit32'].isNull()).to eq true
    expect(rs[3]['bit64'].isNull()).to eq true
    ats.release()
    db.dropTable(SEB_TABLENAME);
    expect(db.stat()).to eq 0
    db.close()
  end
  it 'snapshot' do
    db = Transactd::Database.new()
    openDatabase(db)
    bpos = db.beginSnapshot(Transactd::CONSISTENT_READ_WITH_BINLOG_POS)
    if (isMariaDBWithGtid(db))
      expect(bpos.type).to eq Transactd::REPL_POSTYPE_MARIA_GTID
    else
      ret = (bpos.type == Transactd::REPL_POSTYPE_POS) || (bpos.type == Transactd::REPL_POSTYPE_GTID)
      expect(ret).to eq true
    end
    expect(bpos.pos).not_to eq  0
    expect(bpos.filename).not_to eq ""
    print "\nbinlog pos = ", bpos.filename, ":", bpos.pos, "\n"
    print "gtid (set)= ", bpos.gtid, "\n"
    # setGtid
    bpos.gtid = "ABCD"
    expect(bpos.gtid).to eq "ABCD"
    db.endSnapshot();
    db.close()
  end
  it 'get sql' do
    db = Transactd::Database.new()
    openDatabase(db)
    db.execSql("create view idlessthan5 as select * from user where id < 5")
    view = db.getCreateViewSql("idlessthan5")
    expect(view.include?("idlessthan5")).to eq true
    expect(view.include?("名前")).to eq true
    #puts("view" + view)
    tb = db.openTable("user")
    expect(db.stat()).to eq 0
    sql = tb.getCreateSql()
    #puts("sql" + sql)
    expect(sql.include?("CREATE TABLE")).to eq true
    expect(sql.include?("名前")).to eq true
    tb.close()
    db.close()
  end
  it 'create associate' do
    db = Transactd::Database.new()
    openDatabase(db)
    dba = db.createAssociate()
    expect(db.stat()).to eq 0
    expect(dba.isAssociate()).to eq true
    dba.close()
    db.close()
  end
  it 'connMgr' do
    # other database connection
    db_other = Transactd::Database.new()
    openDatabase(db_other)
    tb_other = db_other.openTable("user")
    expect(db_other.stat()).to eq 0
    # connMgr connection
    db = Transactd::Database.new()
    mgr = Transactd::ConnMgr.new(db)
    mgr.connect(db_other.uri())
    expect(mgr.stat()).to eq 0
    # connections
    recs = mgr.connections()
    expect(mgr.stat()).to eq 0
    expect(recs.size()).to eq 1
    # inUseDatabases
    recs = mgr.inUseDatabases(recs[0].conId)
    expect(mgr.stat()).to eq 0
    expect(recs.size()).to eq 1
    # inUseTables
    recs = mgr.inUseTables(recs[0].conId, recs[0].db)
    expect(mgr.stat()).to eq 0
    expect(recs.size()).to eq 2
    # tables, views
    recs = mgr.tables("test_v3")
    expect(mgr.stat()).to eq 0
    recs1 = mgr.views("test_v3")
    expect(mgr.stat()).to eq 0
    expect(recs.size()).to eq 3
    expect(recs1.size()).to eq 1
    expect(recs1[0].name).to eq "idlessthan5"
    # schemaTables
    recs = mgr.schemaTables("test_v3")
    expect(mgr.stat()).to eq 0
    expect(recs.size()).to eq 1
    expect(recs[0].name).to eq "test"
    # databases
    recs = mgr.databases()
    expect(mgr.stat()).to eq 0
    size = recs.size()
    Transactd::ConnMgr::removeSystemDb(recs)
    expect(mgr.stat()).to eq 0
    expect(recs.size()).not_to eq size
    # sysvar
    recs = mgr.sysvars()
    expect(mgr.stat()).to eq 0
    expect(Transactd::ConnMgr::sysvarName(0)).to eq "database_version"
    # statusvar
    recs = mgr.statusvars()
    expect(mgr.stat()).to eq 0
    expect(Transactd::ConnMgr::statusvarName(0)).to eq "tcp_connections"
    # slaveStatus
    recs = mgr.slaveStatus("")
    expect(mgr.stat()).to eq 0
    expect(mgr.slaveStatusName(0)).to eq "Slave_IO_State"
    for i in 0...recs.size() do
      puts (mgr.slaveStatusName(i) + "\t:" + recs[i].value.to_s)
    end
    # extendedvars
    recs = mgr.extendedvars()
    expect(recs.size()).to eq 4
    expect(Transactd::ConnMgr::extendedVarName(0)).to eq "MySQL_Gtid_Mode"
    # record port
    expect(recs[0].port).to eq 0
    # slaveHosts
    recs = mgr.slaveHosts()
    expect(mgr.stat()).to eq 0
    # channels
    recs = mgr.channels()
    expect(mgr.stat()).to eq 0
    # haLock
    ret = mgr.haLock()
    expect(mgr.stat()).to eq 0
    expect(ret).to eq true
    # haUnlock
    mgr.haUnlock()
    expect(mgr.stat()).to eq 0
    # setRole
    ret = mgr.setRole(0)
    expect(mgr.stat()).to eq 0
    expect(ret).to eq true
    ret = mgr.setRole(1)
    expect(mgr.stat()).to eq 0
    expect(ret).to eq true
    # setEnableFailover
    ret = mgr.setEnableFailover(false)
    expect(mgr.stat()).to eq 0
    expect(ret).to eq true
    ret = mgr.setEnableFailover(true)
    expect(mgr.stat()).to eq 0
    expect(ret).to eq true
    expect(mgr.isOpen()).to eq true
    # enableAutoReconnect
    expect(Transactd::Database::enableAutoReconnect()).to eq false
    Transactd::Database::setEnableAutoReconnect(true)
    expect(Transactd::Database::enableAutoReconnect()).to eq true
    Transactd::Database::setEnableAutoReconnect(false)
    mgr.disconnect()
    expect(mgr.stat()).to eq 0
    expect(mgr.isOpen()).to eq false
    # haNameReslover
    host = "localhost"
    user = "root"
    pwd = ""
    ret = Transactd::HaNameResolver::start("master123", "slave1, slave2", host, 0, user, pwd)
    expect(ret).to eq 1
    # portMap
    Transactd::HaNameResolver::addPortMap(3307, 8611)
    Transactd::HaNameResolver::clearPortMap()
    # master slave name
    expect(Transactd::HaNameResolver::master()).to eq host
    expect(Transactd::HaNameResolver::slave()).to eq "-"
    # connect by master roll
    mgr.connect("tdap://" + user + "@master123/?pwd=" + pwd)
    expect(mgr.stat()).to eq 0
    expect(mgr.isOpen()).to eq true
    mgr.disconnect()
    expect(mgr.stat()).to eq 0
    expect(mgr.isOpen()).to eq false
    # stop 
    Transactd::HaNameResolver::stop()
    mgr.connect("tdap://" + user + "@master123/?pwd=" + pwd)
    expect(mgr.stat()).to eq Transactd::ERROR_TD_HOSTNAME_NOT_FOUND
    mgr.disconnect()
    tb_other.close()
    db_other.close()
  end
  it 'fetch_mode' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    tb = db.openTable("user")
    expect(tb.stat()).to eq 0
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    # test fetch field type (table)
    #   FETCH_RECORD_INTO
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    #     FIELD_VALUE_MODE_OBJECT
    Transactd::field_value_mode = Transactd::FIELD_VALUE_MODE_OBJECT
    expect(rec["id"].i()).to eq 1
    #     FIELD_VALUE_MODE_VALUE
    Transactd::set_field_value_mode(Transactd::FIELD_VALUE_MODE_VALUE)
    expect(rec["id"]).to eq 1
    #   FETCH_VAL_NUM
    tb.fetchMode = Transactd::FETCH_VAL_NUM
    expect(tb.fetchMode).to eq Transactd::FETCH_VAL_NUM
    rec = tb.fields()
    expect(rec[0]).to eq 1
    #   FETCH_VAL_ASSOC
    tb.fetchMode = Transactd::FETCH_VAL_ASSOC
    expect(tb.fetchMode).to eq Transactd::FETCH_VAL_ASSOC
    rec = tb.fields()
    expect(rec["id"]).to eq 1
    #   FETCH_VAL_BOTH
    tb.fetchMode = Transactd::FETCH_VAL_BOTH
    expect(tb.fetchMode).to eq Transactd::FETCH_VAL_BOTH
    rec = tb.fields()
    expect(rec[0]).to eq 1
    expect(rec["id"]).to eq 1
    #   FETCH_OBJ
    tb.fetchMode = Transactd::FETCH_OBJ
    expect(tb.fetchMode).to eq Transactd::FETCH_OBJ
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.名前).to eq "1 user"
    #   FETCH_USR_CLASS
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    User.reset_class
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.名前).to eq "1 user"
    expect(usr.a).to eq "1"
    expect(usr.b).to eq "2"
    expect(usr.c).to eq "3"
    tb.close()
    # test fetch field type (activeTable)
    at = Transactd::ActiveTable.new(db, "user")
    q = Transactd::Query.new()
    q.where("id", "<", 10)
    rs = at.index(0).keyValue(0).read(q)
    #   FETCH_RECORD_INTO
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    expect(rs.size()).to eq 9
    #     FIELD_VALUE_MODE_OBJECT
    Transactd::setFieldValueMode(Transactd::FIELD_VALUE_MODE_OBJECT)
    expect(rs[0]["id"].i()).to eq 1
    #     FIELD_VALUE_MODE_VALUE
    Transactd::field_value_mode = Transactd::FIELD_VALUE_MODE_VALUE
    expect(rs[0]["id"]).to eq 1
    #   FETCH_VAL_NUM
    rs.fetchMode = Transactd::FETCH_VAL_NUM
    expect(rs.fetchMode).to eq Transactd::FETCH_VAL_NUM
    expect(rs.size()).to eq 9
    expect(rs[0][0]).to eq 1
    #   FETCH_VAL_ASSOC
    rs.fetchMode = Transactd::FETCH_VAL_ASSOC
    expect(rs.fetchMode).to eq Transactd::FETCH_VAL_ASSOC
    expect(rs.size()).to eq 9
    expect(rs[0]["id"]).to eq 1
    #   FETCH_VAL_BOTH
    rs.fetchMode = Transactd::FETCH_VAL_BOTH
    expect(rs.fetchMode).to eq Transactd::FETCH_VAL_BOTH
    expect(rs.size()).to eq 9
    expect(rs[0][0]).to eq 1
    expect(rs[0]["id"]).to eq 1
    #   FETCH_OBJ
    rs.fetchMode = Transactd::FETCH_OBJ
    expect(rs.fetchMode).to eq Transactd::FETCH_OBJ
    expect(rs.size()).to eq 9
    expect(rs[0].id).to eq 1
    expect(rs[0].名前).to eq "1 user"
    #   FETCH_USR_CLASS
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    rs.fetchClass = User
    rs.ctorArgs = ["1", "2", "3"]
    expect(rs.fetchMode).to eq Transactd::FETCH_USR_CLASS
    expect(rs.size()).to eq 9
    expect(rs[1].id).to eq 2
    expect(rs[1].名前).to eq "2 user"
    expect(rs[0].a).to eq "1"
    expect(rs[0].b).to eq "2"
    expect(rs[0].c).to eq "3"
    at.release
    db.close()
  end
  it 'fetchMode, alias and UTF-8 field name' do
    Transactd::field_value_mode = Transactd::FIELD_VALUE_MODE_VALUE
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    # table
    tb = db.openTable("user")
    expect(tb.stat()).to eq 0
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.setAlias("名前", "名前2")
    expect(tb.stat()).to eq 0
    #   FETCH_OBJ (nodefine_original = true)
    tb.fetchMode = Transactd::FETCH_OBJ
    expect(tb.fetchMode).to eq Transactd::FETCH_OBJ
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.respond_to?(:"名前2")).to be true
    expect(usr.名前2).to eq "1 user"
    #   FETCH_USR_CLASS
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    #     nodefine_original = false
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = false
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.respond_to?(:"名前")).to be true
    expect(usr.respond_to?(:"名前2")).to be true
    expect(usr.名前).to eq "1 user"
    expect(usr.名前2).to eq "1 user"
    expect(usr.a).to eq "1"
    expect(usr.b).to eq "2"
    expect(usr.c).to eq "3"
    #     nodefine_original = true
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = true
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.respond_to?(:"名前2")).to be true
    expect(usr.名前2).to eq "1 user"
    expect(usr.a).to eq "1"
    expect(usr.b).to eq "2"
    expect(usr.c).to eq "3"
    tb.close
    # activeTable
    at = Transactd::ActiveTable.new(db, "user")
    at.alias("名前", "名前2")
    q = Transactd::Query.new()
    q.where("id", "<", 10)
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    #   FETCH_OBJ (nodefine_original = true)
    rs.fetchMode = Transactd::FETCH_OBJ
    expect(rs.fetchMode).to eq Transactd::FETCH_OBJ
    expect(rs[0].id).to eq 1
    expect(rs[0].respond_to?(:"名前")).to be false
    expect(rs[0].respond_to?(:"名前2")).to be true
    expect(rs[0].名前2).to eq "1 user"
    #   FETCH_USR_CLASS
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    expect(rs.fetchMode).to eq Transactd::FETCH_USR_CLASS
    rs.fetchClass = User
    rs.ctorArgs = ["1", "2", "3"]
    #     nodefine_original = false
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = false
    expect(rs[0].id).to eq 1
    expect(rs[0].respond_to?(:"名前")).to be true
    expect(rs[0].respond_to?(:"名前2")).to be true
    expect(rs[0].名前).to eq "1 user"
    expect(rs[0].名前2).to eq "1 user"
    #     nodefine_original = true
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = true
    expect(rs[0].id).to eq 1
    expect(rs[0].respond_to?(:"名前")).to be false
    expect(rs[0].respond_to?(:"名前2")).to be true
    expect(rs[0].名前2).to eq "1 user"
    at.release
    db.close
  end
  it 'set_value_by_object, alias and UTF-8 field name' do
    Transactd::field_value_mode = Transactd::FIELD_VALUE_MODE_VALUE
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    # table
    tb = db.openTable("user")
    expect(tb.stat()).to eq 0
    #   FETCH_OBJ, no alias
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    tb.fetchMode = Transactd::FETCH_OBJ
    expect(tb.fetchMode).to eq Transactd::FETCH_OBJ
    usr = tb.fields()
    expect(usr.名前).to eq "1 user"
    expect(usr.respond_to?(:"名前2")).to be false
    usr.名前 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.名前).to eq "1 ユーザー"
    expect(usr2.respond_to?(:"名前2")).to be false
    #   FETCH_USR_CLASS, no alias
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.nodefine_original = true
    usr = tb.fields()
    expect(usr.名前).to eq "1 user"
    expect(usr.respond_to?(:"名前2")).to be false
    usr.名前 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.名前).to eq "1 ユーザー"
    expect(usr2.respond_to?(:"名前2")).to be false
    #   FETCH_OBJ, with alias
    tb.setAlias("名前", "名前2")
    expect(tb.stat()).to eq 0
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    tb.fetchMode = Transactd::FETCH_OBJ
    expect(tb.fetchMode).to eq Transactd::FETCH_OBJ
    usr = tb.fields()
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.respond_to?(:"名前")).to be false
    expect(usr2.名前2).to eq "1 ユーザー"
    #   FETCH_USR_CLASS, with alias, nodefine_original = true
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = true
    usr = tb.fields()
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.respond_to?(:"名前")).to be false
    expect(usr2.名前2).to eq "1 ユーザー"
    #   FETCH_USR_CLASS, with alias, nodefine_original = false
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(tb.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = tb.fields()
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = false
    usr = tb.fields()
    expect(usr.名前).to eq "1 user"
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー 名前2"
    rec.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.名前).to eq "1 ユーザー 名前2"
    expect(usr2.名前2).to eq "1 ユーザー 名前2"
    usr.名前 = "1 ユーザー 名前"
    rec.set_value_by_object(usr)
    usr3 = tb.fields()
    expect(usr3.名前).to eq "1 ユーザー 名前"
    expect(usr3.名前2).to eq "1 ユーザー 名前"
    tb.close
    # activeTable
    at = Transactd::ActiveTable.new(db, "user")
    q = Transactd::Query.new()
    q.where("id", "<", 10)
    #   FETCH_OBJ, no alias
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = rs[0]
    rs.fetchMode = Transactd::FETCH_OBJ
    expect(rs.fetchMode).to eq Transactd::FETCH_OBJ
    usr = rs[0]
    expect(usr.名前).to eq "1 user"
    expect(usr.respond_to?(:"名前2")).to be false
    usr.名前 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = rs[0]
    expect(usr2.名前).to eq "1 ユーザー"
    expect(usr2.respond_to?(:"名前2")).to be false
    #   FETCH_USR_CLASS, no alias
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = rs[0]
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    expect(rs.fetchMode).to eq Transactd::FETCH_USR_CLASS
    rs.fetchClass = User
    rs.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.nodefine_original = true
    usr = rs[0]
    expect(usr.名前).to eq "1 user"
    expect(usr.respond_to?(:"名前2")).to be false
    usr.名前 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = rs[0]
    expect(usr2.名前).to eq "1 ユーザー"
    expect(usr2.respond_to?(:"名前2")).to be false
    #   FETCH_OBJ, with alias
    at.alias("名前", "名前2")
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = rs[0]
    rs.fetchMode = Transactd::FETCH_OBJ
    expect(rs.fetchMode).to eq Transactd::FETCH_OBJ
    usr = rs[0]
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = rs[0]
    expect(usr2.respond_to?(:"名前")).to be false
    expect(usr2.名前2).to eq "1 ユーザー"
    #   FETCH_USR_CLASS, with alias, nodefine_original = true
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = rs[0]
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    expect(rs.fetchMode).to eq Transactd::FETCH_USR_CLASS
    rs.fetchClass = User
    rs.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = true
    usr = rs[0]
    expect(usr.respond_to?(:"名前")).to be false
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー"
    rec.set_value_by_object(usr)
    usr2 = rs[0]
    expect(usr2.respond_to?(:"名前")).to be false
    expect(usr2.名前2).to eq "1 ユーザー"
    #   FETCH_USR_CLASS, with alias, nodefine_original = false
    rs = at.index(0).keyValue(0).read(q)
    expect(rs.size()).to eq 9
    rs.fetchMode = Transactd::FETCH_RECORD_INTO
    expect(rs.fetchMode).to eq Transactd::FETCH_RECORD_INTO
    rec = rs[0]
    rs.fetchMode = Transactd::FETCH_USR_CLASS
    expect(rs.fetchMode).to eq Transactd::FETCH_USR_CLASS
    rs.fetchClass = User
    rs.ctorArgs = ["1", "2", "3"]
    User.reset_class
    User.alias_attribute("名前2", "名前")
    User.nodefine_original = false
    usr = rs[0]
    expect(usr.名前).to eq "1 user"
    expect(usr.名前2).to eq "1 user"
    usr.名前2 = "1 ユーザー 名前2"
    rec.set_value_by_object(usr)
    usr2 = rs[0]
    expect(usr2.名前).to eq "1 ユーザー 名前2"
    expect(usr2.名前2).to eq "1 ユーザー 名前2"
    usr.名前 = "1 ユーザー 名前"
    rec.set_value_by_object(usr)
    usr3 = rs[0]
    expect(usr3.名前).to eq "1 ユーザー 名前"
    expect(usr3.名前2).to eq "1 ユーザー 名前"
    at.release
    db.close
  end
  it 'set_ailas' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    tb = db.openTable("user")
    expect(tb.stat()).to eq 0
    tb.setAlias("名前", "name")
    tb.setAlias("id", "user_id")
    expect(tb.fieldNumByName("user_id")).to eq 0
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    User.reset_class
    User.alias_attribute("name", "名前")
    User.alias_attribute("user_id", "id")
    User.nodefine_original = false
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    usr = tb.fields()
    expect(usr.id).to eq 1
    expect(usr.user_id).to eq 1
    expect(usr.名前).to eq "1 user"
    expect(usr.name).to eq "1 user"
    q = Transactd::Query.new()
    q.select("name").where("id", "<", 10)
    tb.setQuery(q)
    tb.clearBuffer()
    users = tb.findAll()
    expect(users.length).to eq 9
    expect(users[0].name).to eq "1 user"
    tb.close()
    db.close()
  end
  it 'insert_object' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    tb = db.openTable("user")
    expect(tb.stat()).to eq 0
    tb.setAlias("名前", "name")
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb.fetchMode = Transactd::FETCH_USR_CLASS
    User.reset_class
    tb.fetchClass = User
    tb.ctorArgs = ["1", "2", "3"]
    expect(tb.fetchMode).to eq Transactd::FETCH_USR_CLASS
    usr = tb.fields()
    usr.id = 0
    usr.name = 'test_insertObject'
    tb.insert_by_object(usr)
    tb.seekLast()
    usr = tb.fields()
    expect(usr.name).to eq 'test_insertObject'
    expect(usr.id).to eq 1001
    row = tb.getRecord()
    row.set_value_by_object(usr)
    usr2 = tb.fields()
    expect(usr2.a).to eq usr.a
    expect(usr2.b).to eq usr.b
    expect(usr2.c).to eq usr.c
    expect(usr2.id).to eq usr.id
    expect(usr2.name).to eq usr.name
    expect(usr2.group).to eq usr.group
    expect(usr2.tel).to eq usr.tel
    expect(usr2.update_datetime).to eq usr.update_datetime
    expect(usr2.create_datetime).to eq usr.create_datetime
    tb.close
    db.close
  end
  it 'test UCC' do
    db = Transactd::Database.new()
    db.open(URL, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
    tb = db.openTable("user")
    tb2 = db.openTable("user")
    # test ChangeCurrentCc
    #   updateConflictCheck true
    db.beginTrn()
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb2.seekFirst()
    expect(tb2.stat()).to eq 0
    tb.setFV("名前", 'John')
    tb.update()
    expect(tb.stat()).to eq 0
    tb2.setFV("名前", 'mike')
    tb2.setUpdateConflictCheck(true)
    expect(tb2.updateConflictCheck()).to eq true
    tb2.update(Transactd::Nstable::ChangeCurrentCc)
    expect(tb2.stat()).to eq Transactd::STATUS_CHANGE_CONFLICT
    db.abortTrn()
    #   updateConflictCheck false
    db.beginTrn()
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb2.seekFirst()
    expect(tb2.stat()).to eq 0
    tb.setFV("名前", 'John')
    tb.update()
    expect(tb.stat()).to eq 0
    tb2.setFV("名前", 'mike')
    tb2.setUpdateConflictCheck(false)
    expect(tb2.updateConflictCheck()).to eq false
    tb2.update(Transactd::Nstable::ChangeCurrentCc)
    expect(tb2.stat()).to eq 0
    db.abortTrn()
    # test ChangeInKey
    #   updateConflictCheck true
    db.beginTrn()
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb2.seekFirst()
    expect(tb2.stat()).to eq 0
    tb.setFV("名前", 'John')
    tb.update()
    expect(tb.stat()).to eq 0
    tb2.setFV("名前", 'mike')
    tb2.setUpdateConflictCheck(true)
    expect(tb2.updateConflictCheck()).to eq true
    tb2.update(Transactd::Nstable::ChangeInKey)
    expect(tb2.stat()).to eq Transactd::STATUS_CHANGE_CONFLICT
    db.abortTrn()
    #   updateConflictCheck false
    db.beginTrn()
    tb.seekFirst()
    expect(tb.stat()).to eq 0
    tb2.seekFirst()
    expect(tb2.stat()).to eq 0
    tb.setFV("名前", 'John')
    tb.update()
    expect(tb.stat()).to eq 0
    tb2.setFV("名前", 'mike')
    tb2.setUpdateConflictCheck(false)
    expect(tb2.updateConflictCheck()).to eq false
    tb2.update(Transactd::Nstable::ChangeInKey)
    expect(tb2.stat()).to eq 0
    db.abortTrn()
    tb.close
    tb2.close
    db.close
  end
end
