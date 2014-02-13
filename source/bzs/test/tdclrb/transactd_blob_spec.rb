# coding : utf-8
=begin ============================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

   This program is free software you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program if not, write to the Free Software 
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
   02111-1307, USA.
===================================================================
=end
require 'transactd'
require 'base64'

def getHost()
  hostname = '127.0.0.1/'
  if (ENV['TRANSACTD_RSPEC_HOST'] != nil && ENV['TRANSACTD_RSPEC_HOST'] != '')
    hostname = ENV['TRANSACTD_RSPEC_HOST']
  end
  hostname = hostname + '/' unless (hostname =~ /\/$/)
  return hostname
end

HOSTNAME = getHost()
URL = 'tdap://' + HOSTNAME + 'test_blob?dbfile=test.bdf'
TABLENAME = 'comments'
FDI_ID = 0
FDI_USER_ID = 1
FDI_BODY = 2
FDI_IMAGE = 3

TYPE_SCHEMA_BDF = 0

def dropDatabase(db)
  db.open(URL)
  expect(db.stat()).to eq 0
  db.drop()
  expect(db.stat()).to eq 0
end

def createDatabase(db)
  db.create(URL)
  if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR
    dropDatabase(db)
    db.create(URL)
  end
  expect(db.stat()).to eq 0
end

def openDatabase(db)
  db.open(URL, TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL)
  expect(db.stat()).to eq 0
end

def createTable(db)
  openDatabase(db)
  dbdef = db.dbDef()
  expect(dbdef).not_to be nil
  td = Transactd::Tabledef.new()
  # Set table schema codepage to UTF-8
  #   - codepage for field NAME and tableNAME
  td.schemaCodePage = Transactd::CP_UTF8
  td.setTableName(TABLENAME)
  td.setFileName(TABLENAME + '.dat')
  # Set table default charaset index
  #    - default charset for field VALUE
  td.charsetIndex = Transactd::charsetIndex(Transactd::CP_UTF8)
  td.id = 1
  td.pageSize = 2048
  dbdef.insertTable(td)
  expect(dbdef.stat()).to eq 0
  # id
  fd = dbdef.insertField(1, FDI_ID)
  fd.setName('id')
  fd.type = Transactd::Ft_autoinc
  fd.len = 4
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  # user_id
  fd = dbdef.insertField(1, FDI_USER_ID)
  fd.setName('user_id')
  fd.type = Transactd::Ft_integer
  fd.len = 4
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  # body
  fd = dbdef.insertField(1, FDI_BODY)
  fd.setName('body')
  fd.type = Transactd::Ft_mytext
  fd.len = 10 # 9:TYNYTEXT 10:TEXT 11:MIDIUMTEXT 12:LONGTEXT
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  # image
  fd = dbdef.insertField(1, FDI_IMAGE)
  fd.setName('image')
  fd.type = Transactd::Ft_myblob
  fd.len = 10 # 9:TYNYBLOB 10:BLOB 11:MIDIUMBLOB 12:LONGBLOB
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
  # key
  kd = dbdef.insertKey(1,0)
  kd.segment(0).fieldNum = 0
  kd.segment(0).flags.bit8 = 1
  kd.segment(0).flags.bit1 = 1
  kd.segmentCount = 1
  dbdef.updateTableDef(1)
  expect(dbdef.stat()).to eq 0
end

def openTable(db)
  tb = db.openTable(TABLENAME)
  expect(db.stat()).to eq 0
  return tb
end

def getTestBinary()
  image_base64 = 'R0lGODdhEAAQAKEBAGZmZv#/5mZmczMzCwAAAAAEAAQAAACRowzIgA6BxebTMAgG60nW5NM1kAZikGFHAmgYvYgJpW12FfTyLpJjz+IVSSXR4IlQCoUgCCG8ds0D5xZT3TJYS8IZiMJKQAAOw=='
  return Base64.decode64(image_base64)
end

describe Transactd, 'blob' do
  before :each do
    @db = Transactd::Database.createObject()
  end
  after :each do
    @db.close()
    @db = nil
  end
  
  it 'create' do
    createDatabase(@db)
    openDatabase(@db)
    createTable(@db)
    tb = openTable(@db)
    tb.close()
  end
  
  it 'insert' do
    image = getTestBinary()
    openDatabase(@db)
    tb = openTable(@db)
    expect(tb).not_to be nil
    # 1
    tb.clearBuffer()
    tb.setFV(FDI_USER_ID, 1)
    tb.setFV(FDI_BODY, "1\ntest\nテスト\n\nあいうえおあいうえお")
    tb.setFV(FDI_IMAGE, image, image.bytesize)
    tb.insert()
    expect(tb.stat()).to eq 0
    # 2
    tb.clearBuffer()
    tb.setFV('user_id', 1)
    tb.setFV('body', "2\ntest\nテスト\n\nあいうえおあいうえお")
    str = "2\ntest\nテスト\n\nあいうえおあいうえお"
    tb.setFV('image', str, str.bytesize)
    tb.insert()
    expect(tb.stat()).to eq 0
    # 3
    tb.clearBuffer()
    tb.setFV(FDI_USER_ID, 2)
    tb.setFV(FDI_BODY, "3\ntest\nテスト\n\nあいうえおあいうえお")
    str = "3\ntest\nテスト\n\nあいうえおあいうえお"
    tb.setFV(FDI_IMAGE, str, str.bytesize)
    tb.insert()
    expect(tb.stat()).to eq 0
    tb.close()
  end
  
  it 'seek' do
    image = getTestBinary()
    openDatabase(@db)
    tb = openTable(@db)
    expect(tb).not_to be nil
    # 1
    tb.clearBuffer()
    tb.setFV(FDI_ID, 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 1
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "1\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE)).to eq image
    # 2
    tb.seekNext()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 2
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    # 3
    tb.seekNext()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 3
    expect(tb.getFVint(FDI_USER_ID)).to eq 2
    expect(tb.getFVstr(FDI_BODY)).to eq "3\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "3\ntest\nテスト\n\nあいうえおあいうえお"
    # 2
    tb.seekPrev()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 2
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    tb.close()
  end
  
  it 'find' do
    image = getTestBinary()
    openDatabase(@db)
    tb = openTable(@db)
    expect(tb).not_to be nil
    # 1
    tb.setKeyNum(0)
    tb.clearBuffer()
    tb.setFilter('id >= 1 and id < 3', 1, 0)
    expect(tb.stat()).to eq 0
    tb.setFV(FDI_ID, 1)
    tb.find(Transactd::Table::FindForword)
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 1
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "1\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE)).to eq image
    # 2
    tb.findNext(true)
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 2
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    # 3... but not found because filtered
    tb.findNext(true)
    expect(tb.stat()).to eq Transactd::STATUS_EOF
    # 2... but changing seek-direction is not allowed
    tb.findPrev(true)
    expect(tb.stat()).to eq Transactd::STATUS_PROGRAM_ERROR
    tb.close()
  end
  
  it 'update' do
    image = getTestBinary()
    openDatabase(@db)
    tb = openTable(@db)
    expect(tb).not_to be nil
    # select 1
    tb.clearBuffer()
    tb.setFV(FDI_ID, 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 1
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "1\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE)).to eq image
    # update
    tb.setFV(FDI_BODY, "1\nテスト\ntest\n\nABCDEFG")
    tb.update()
    expect(tb.stat()).to eq 0
    # select 2
    tb.seekNext()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 2
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    # update
    tb.setFV(FDI_BODY, "2\nテスト\ntest\n\nABCDEFG")
    tb.update()
    expect(tb.stat()).to eq 0
    # check 1
    tb.seekPrev()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 1
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "1\nテスト\ntest\n\nABCDEFG"
    expect(tb.getFVbin(FDI_IMAGE)).to eq image
    # check 2
    tb.seekNext()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 2
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "2\nテスト\ntest\n\nABCDEFG"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "2\ntest\nテスト\n\nあいうえおあいうえお"
    tb.close()
  end
  
  it 'delete' do
    image = getTestBinary()
    openDatabase(@db)
    tb = openTable(@db)
    expect(tb).not_to be nil
    # delete 2
    tb.clearBuffer()
    tb.setFV(FDI_ID, 2)
    tb.seek()
    expect(tb.stat()).to eq 0
    tb.del()
    expect(tb.stat()).to eq 0
    # select 1
    tb.clearBuffer()
    tb.setFV(FDI_ID, 1)
    tb.seek()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 1
    expect(tb.getFVint(FDI_USER_ID)).to eq 1
    expect(tb.getFVstr(FDI_BODY)).to eq "1\nテスト\ntest\n\nABCDEFG"
    expect(tb.getFVbin(FDI_IMAGE)).to eq image
    # next is 3
    tb.seekNext()
    expect(tb.stat()).to eq 0
    expect(tb.getFVint(FDI_ID)).to eq 3
    expect(tb.getFVint(FDI_USER_ID)).to eq 2
    expect(tb.getFVstr(FDI_BODY)).to eq "3\ntest\nテスト\n\nあいうえおあいうえお"
    expect(tb.getFVbin(FDI_IMAGE).force_encoding('UTF-8')).to eq "3\ntest\nテスト\n\nあいうえおあいうえお"
    # eof
    tb.seekNext()
    expect(tb.stat()).to eq Transactd::STATUS_EOF
    tb.close()
  end
  
  it 'drop' do
    dropDatabase(@db)
  end
end
