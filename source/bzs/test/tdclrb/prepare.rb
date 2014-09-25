# coding : utf-8
=begin =============================================================
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

FN_ID = 0
FN_NAME = 1

PARALLEL_TRN = 1000
LOCK_SINGLE_NOWAIT = 200
TRANS_BIAS = PARALLEL_TRN + LOCK_SINGLE_NOWAIT

def createTable(db)
    dbdef = db.dbDef()
    td = Transactd::Tabledef.new()
    # Set table schema codepage to UTF-8
    #       - codepage for field NAME and tableNAME
    td.schemaCodePage = Transactd::CP_UTF8
    td.setTableName('user')
    td.setFileName('user.dat')
    # Set table default charaset index
    #        - default charset for field VALUE
    td.charsetIndex = Transactd::charsetIndex(Transactd::CP_UTF8)
    td.id = 1
    td.pageSize = 2048
    dbdef.insertTable(td)
    # id
    fd = dbdef.insertField(td.id, FN_ID)
    fd.setName('id')
    fd.type = Transactd::Ft_integer
    fd.len = 4
    dbdef.updateTableDef(1)
    # name
    fd = dbdef.insertField(td.id, FN_NAME)
    fd.setName('name')
    fd.type = Transactd::Ft_myvarchar
    fd.len = 100
    dbdef.updateTableDef(td.id)
    # key
    kd = dbdef.insertKey(td.id, 0)
    kd.segment(0).fieldNum = 0
    kd.segment(0).flags.bit8 = 1 # extend key type
    kd.segment(0).flags.bit1 = 1 # changeable
    kd.segmentCount = 1
    td.primaryKeyNum = 0
    dbdef.updateTableDef(td.id)
end

## --------------------------------------------------------------------------------
def printUsage()
    puts("usage: ruby prepare.rb databaseUri functionNumber rangeStart rangeEnd gap")
    puts("\t --- functionNumber list ---")
    puts("\t 0: only create database and table")
    puts("\t 1: delete records from rangeStart to rangeEnd with gap")
    puts("\t 2: set records from rangeStart to rangeEnd with gap")
    puts("\t 3: check records from rangeStart to rangeEnd with gap")
    puts("example : ruby prepare.rb \"tdap://localhost/test?dbfile=test.bdf\" 1 1 1000 1")
end

## --------------------------------------------------------------------------------
def main(argv)
    if (argv.length < 3)
        printUsage()
        return
    end
    uri = argv[1]
    functionNumber = Integer(argv[2])
    if !([0,1,2,3].include?(functionNumber))
        printUsage()
        return
    end
    rangeStart = 0
    rangeEnd = 0
    gap = 0
    if functionNumber > 0
        if argv.length < 5
            printUsage()
            return
        end
        rangeStart = Integer(argv[3])
        rangeEnd = Integer(argv[4])
        if rangeStart < 0 || rangeEnd < 0
            printUsage()
            return
        end
        if rangeStart > rangeEnd
            tmp = rangeStart
            rangeStart = rangeEnd
            rangeEnd = tmp
        end
        if argv.length >= 6
            gap = Integer(argv[5])
        end
    end
    
    db = Transactd::Database.new()
    
    if !db.open(uri, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL, '', '')
        db.create(uri)
        if db.stat() != 0
            puts("create database error No #{db.stat()}")
            db.close()
            return
        end
    end
    
    if !db.open(uri, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL, '', '')
        puts("open table erorr No #{db.stat().to_s}")
        db.close()
        return
    end
    
    tb = db.openTable('user', Transactd::TD_OPEN_NORMAL, true)
    if db.stat() == Transactd::STATUS_TABLE_EXISTS_ERROR || tb == nil
        createTable(db)
        tb = db.openTable('user', Transactd::TD_OPEN_NORMAL, true)
    end
    
    if functionNumber == 1
        db.beginTrn(TRANS_BIAS)
        tb.clearBuffer()
        for i in rangeStart..rangeEnd do
            tb.setFV(FN_ID, i)
            tb.seek()
            if tb.stat() == 0
                tb.del()
                if tb.stat() != 0
                    puts("delete erorr No #{tb.stat().to_s}")
                    db.endTrn()
                    tb.close()
                    db.close()
                    return false
                end
            elsif tb.stat() != 4
                puts("delete erorr No #{tb.stat().to_s}")
                db.endTrn()
                tb.close()
                db.close()
                return false
            end
        end
        db.endTrn()
    end
    
    if functionNumber == 2
        db.beginTrn(TRANS_BIAS)
        for i in rangeStart..rangeEnd do
            tb.clearBuffer()
            tb.setFV(FN_ID, i)
            tb.seek()
            if (tb.stat() == 0)
                tb.setFV(FN_NAME, "#{i + gap}")
                tb.update()
                if (tb.stat() != 0)
                    puts("set erorr No #{tb.stat().to_s}")
                    db.endTrn()
                    tb.close()
                    db.close()
                    return false
                end
            else
                tb.clearBuffer()
                tb.setFV(FN_ID, i)
                tb.setFV(FN_NAME, i + gap)
                tb.insert()
                if (tb.stat() != 0)
                    puts("set erorr No #{tb.stat().to_s}")
                    db.endTrn()
                    tb.close()
                    db.close()
                    return false
                end
            end
        end
        db.endTrn()
    end
    
    if functionNumber == 3
        tb.clearBuffer()
        tb.setFV(FN_ID, rangeStart)
        tb.seekGreater(true)
        for i in rangeStart..rangeEnd do
            if (tb.stat() != 0)
                puts("check error stat=#{tb.stat()} Expected ID #{i}")
                tb.close()
                db.close()
                return false
            end
            if (tb.getFVlng(FN_ID) != i)
                puts("check error stat=#{tb.stat()} Expected ID #{i} but ID #{tb.getFVlng(FN_ID)}")
                tb.close()
                db.close()
                return false
            end
            if (tb.getFVstr(FN_NAME).to_s != "#{i + gap}")
                puts("check error stat=#{tb.stat()} Expected Name #{i + gap} but Name #{tb.getFVstr(FN_NAME)}")
                tb.close()
                db.close()
                return false
            end
            tb.seekNext()
        end
    end
    
    tb.close()
    db.close()
    return
end

args = ARGV
args.unshift(__FILE__)
main(args)
