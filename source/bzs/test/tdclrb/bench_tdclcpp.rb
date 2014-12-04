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
require 'date'
require 'transactd'

USE_NORMAL = 0
USE_TRANS = 1
USE_BALKINS = 2
USE_SNAPSHOT = 4

FN_ID = 0
FN_NAME = 1

AUTO_CREATE_TABLE = true
PARALLEL_TRN = 1000
LOCK_SINGLE_NOWAIT = 200
TRANS_BIAS = PARALLEL_TRN + LOCK_SINGLE_NOWAIT

BULKBUFSIZE = 65535 - 1000

## --------------------------------------------------------------------------------
def showTableError(tb, description)
    if (tb.stat() != 0)
        puts("#{description} error #{tb.tableDef().fileName()}:No.#{tb.stat().to_s}")
    end
end

## --------------------------------------------------------------------------------
def showEnginError(db, tableName)
    if (db.stat() != 0)
        puts("#{tableName} error No.#{db.stat().to_s}")
    end
end

## --------------------------------------------------------------------------------
def openTable(db, tableName, mode)
    tb = db.openTable(tableName, mode, AUTO_CREATE_TABLE)
    showEnginError(db, tableName) if (tb == nil)
    return tb
end

## --------------------------------------------------------------------------------
def createDataBase(db, uri)
    db.create(uri)
    return (db.stat() == 0)
end

## --------------------------------------------------------------------------------
def write(tb, start, endid)
    tb.setKeyNum(0)
    for i in start..(endid - 1) do
        tb.clearBuffer()
        tb.setFV(FN_ID, i)
        tb.setFV(FN_NAME, i)
        tb.insert()
        if (tb.stat() != 0)
            showTableError(tb, 'write')
            return false
        end
    end
    return true
end

## --------------------------------------------------------------------------------
def deleteAll(db, tb, start, endid)
    db.beginTrn(TRANS_BIAS)
    tb.clearBuffer()
    tb.stepFirst()
    while tb.stat() == 0
       tb.del();
       if (tb.stat() != 0)
           showTableError(tb, 'deleteAll')
           db.endTrn()
           return false
       end
       tb.stepNext()
    end
    db.endTrn()
    return true
end

## --------------------------------------------------------------------------------
def Inserts(db, tb, start, endid, mode, unit)
    ret = true
    total = endid - start
    count = total / unit
    st = start
    en = st
    while (en != endid) do
        en = st + unit
        db.beginTrn(TRANS_BIAS) if (mode == USE_TRANS)
        tb.beginBulkInsert(BULKBUFSIZE) if (mode == USE_BALKINS)
        ret = write(tb, st, en)
        tb.commitBulkInsert() if (mode == USE_BALKINS)
        db.endTrn() if (mode == USE_TRANS)
        break if (ret == false)
        st = en
    end
    return ret
end

## --------------------------------------------------------------------------------
def Read(db, tb, start, endid, shapshot)
    ret = true
    tb.clearBuffer()
    db.beginSnapshot() if (shapshot == USE_SNAPSHOT)
    for i in start..(endid - 1) do
        tb.setFV(FN_ID, i)
        tb.seek()
        if ((tb.stat() != 0) || (tb.getFVlng(FN_ID) != i))
            puts("GetEqual Error stat() = #{tb.stat().to_s}  Value #{i.to_s} = #{tb.getFVlng(FN_ID).to_s}")
            ret = false
            break
        end
    end
    db.endSnapshot() if (shapshot == USE_SNAPSHOT)
    return ret
end

## --------------------------------------------------------------------------------
def Reads(db, tb, start, endid, unit, shapshot)
    ret = true
    total = endid - start
    count = total / unit
    st = start
    en = st
    db.beginSnapshot() if (shapshot == USE_SNAPSHOT)
    tb.setKeyNum(0)
    tb.setFilter('*', 1, 20)
    tb.clearBuffer()
    tb.setFV(FN_ID, st)
    tb.find(Transactd::Table::FindForword)
    while (en != endid)
        en = st + unit
        for i in st..(en - 1) do
            if (tb.getFVlng(FN_ID) != i)
                puts("findNext Error stat() = #{tb.stat().to_s}  Value #{i.to_s} = #{tb.getFVlng(FN_ID).to_s}")
                ret = false
                break
            end
            tb.findNext()
        end
        break if (ret == false)
        st = en
    end
    db.endSnapshot() if (shapshot == USE_SNAPSHOT)
    return ret
end

## --------------------------------------------------------------------------------
def Updates(db, tb, start, endid, tran, unit)
    ret = true
    tb.setKeyNum(0)
    total = endid - start
    count = total / unit
    st = start
    en = st
    while (en != endid)
        en = st + unit
        db.beginTrn(TRANS_BIAS) if (tran == USE_TRANS)
        for i in st..(en - 1) do
            tb.setFV(FN_ID, i)
            tb.setFV(FN_NAME, "#{i + 1 + tran}")
            tb.update(Transactd::Table::ChangeInKey)
            if (tb.stat() != 0)
                ret = false
                break
            end
        end
        db.endTrn() if (tran == USE_TRANS)
        break if (ret == false)
        st = en
    end
    return ret
end

## --------------------------------------------------------------------------------
def createTestDataBase(db, uri)
    db.create(uri)
    if (db.stat() != 0)
        puts("createTestDataBase erorr:No.#{db.stat().to_s} #{uri}")
        return false
    end
    if (db.open(uri, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL, '', ''))
        dbdef = db.dbDef()
        td = Transactd::Tabledef.new()
        td.setTableName('user')
        td.setFileName('user.dat')
        td.id = 1
        # td.primaryKeyNum = -1
        # td.parentKeyNum = -1
        # td.replicaKeyNum = -1
        td.pageSize = 2048
        dbdef.insertTable(td)
        
        fd = dbdef.insertField(td.id, 0)
        fd.setName('id')
        fd.type = Transactd::Ft_integer
        fd.len = 4
        dbdef.updateTableDef(1)
        
        fd = dbdef.insertField(td.id, 1)
        fd.setName('name')
        fd.type = Transactd::Ft_myvarchar
        fd.len = 100
        dbdef.updateTableDef(td.id)
        
        kd = dbdef.insertKey(td.id, 0)
        kd.segment(0).fieldNum = 0
        kd.segment(0).flags.bit8 = 1 # extend key type
        kd.segment(0).flags.bit1 = 1 # changeable
        kd.segmentCount = 1
        
        td.primaryKeyNum = 0
        dbdef.updateTableDef(td.id)
        return true
    else
        puts("open daatabse erorr No:#{db.stat().to_s}")
    end
    return false
end

## --------------------------------------------------------------------------------
def printDateTime()
    puts(DateTime.now.strftime('%Y/%m/%d %H:%M:%S'))
end

## --------------------------------------------------------------------------------
def printHeader(uri, count)
    puts("Start Bench mark Insert Items = #{count.to_s}")
    printDateTime()
    puts(uri)
    puts("----------------------------------------")
end

## --------------------------------------------------------------------------------
def printTail()
    puts("----------------------------------------")
end

## --------------------------------------------------------------------------------
def main(argv)
    if (argv.length < 4)
        puts("usage: ruby bench_tdclcpp.rb databaseUri processNumber functionNumber noDeleteFlag")
        puts("\t --- functionNumber list ---")
        puts("\t-1: all function")
        puts("\t 0: Insert")
        puts("\t 1: Insert in transaction. 20rec x 1000times")
        puts("\t 2: Insert by bulkmode. 20rec x 1000times")
        puts("\t 3: read each record")
        puts("\t 4: read each record with snapshot")
        puts("\t 5: read range. 20rec x 1000times")
        puts("\t 6: read range with snapshpot. 20rec x 1000times")
        puts("\t 7: update")
        puts("\t 8: update in transaction. 20rec x 1000times")
        puts("example : ruby bench_tdclcpp.rb \"tdap://localhost/test?dbfile=test.bdf\" 0 -1 0")
        return
    end
    uri = argv[1] # "tdap://localhost/test?dbfile=test.bdf"
    procID = Integer(argv[2]) # 0
    count = 20000
    start = procID * count + 1
    endid = start + count
    exeType = Integer(argv[3]) # -1
    insertBeforeNoDelete = ((argv.length > 4) && (Integer(argv[4]) != 0))
    
    db = Transactd::Database.new()
    if (db.open(uri, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL, '', '') == false)
        if (!createTestDataBase(db, uri))
            db.close()
            return
        end
        puts("CreateDataBase success.")
    end
    printHeader(uri, count)
    
    if (!db.open(uri, Transactd::TYPE_SCHEMA_BDF, Transactd::TD_OPEN_NORMAL, '', ''))
        puts("open table erorr No:#{db.stat().to_s}")
    else
        tb = openTable(db, 'users', Transactd::TD_OPEN_NORMAL)
        if tb == nil
          puts "can not open table 'users'"
          db.close()
          return
        end
        
        if ((exeType == -1) || (exeType == 0))
            if (insertBeforeNoDelete || deleteAll(db, tb, start, endid))
                Transactd::Benchmark::start()
                succeed = Inserts(db, tb, start, endid, USE_NORMAL, 1)
                Transactd::Benchmark::showTimeSec(succeed, ': Insert')
            else
                puts("deleteAll erorr No:#{tb.stat().to_s}")
            end
        end
        if ((exeType == -1) || (exeType == 1))
            if (insertBeforeNoDelete || deleteAll(db, tb, start, endid))
                Transactd::Benchmark::start()
                succeed = Inserts(db, tb, start, endid, USE_TRANS, 20)
                Transactd::Benchmark::showTimeSec(succeed, ': Insert in transaction. 20rec x 1000times.')
            else
                puts("deleteAll erorr No:#{tb.stat().to_s}")
            end
        end
        if ((exeType == -1) || (exeType == 2))
            if (insertBeforeNoDelete || deleteAll(db, tb, start, endid))
                Transactd::Benchmark::start()
                succeed = Inserts(db, tb, start, endid, USE_BALKINS, 20)
                Transactd::Benchmark::showTimeSec(succeed, ': Insert by bulkmode. 20rec x 1000times.')
            else
                puts("deleteAll erorr No:#{tb.stat().to_s}")
            end
        end
        if ((exeType == -1) || (exeType == 3))
            Transactd::Benchmark::start()
            succeed = Read(db, tb, start, endid, USE_NORMAL)
            Transactd::Benchmark::showTimeSec(succeed, 'read each record.')
        end
        if ((exeType == -1) || (exeType == 4))
            Transactd::Benchmark::start()
            succeed = Read(db, tb, start, endid, USE_SNAPSHOT)
            Transactd::Benchmark::showTimeSec(succeed, ': read each record with snapshot.')
        end
        if ((exeType == -1) || (exeType == 5))
            Transactd::Benchmark::start()
            succeed = Reads(db, tb, start, endid, 20, USE_NORMAL)
            Transactd::Benchmark::showTimeSec(succeed, ': read range. 20rec x 1000times.')
        end
        if ((exeType == -1) || (exeType == 6))
            Transactd::Benchmark::start()
            succeed = Reads(db, tb, start, endid, 20, USE_SNAPSHOT)
            Transactd::Benchmark::showTimeSec(succeed, ': read range with snapshpot. 20rec x 1000times.')
        end
        if ((exeType == -1) || (exeType == 7))
            Transactd::Benchmark::start()
            succeed = Updates(db, tb, start, endid, USE_NORMAL, 1)
            Transactd::Benchmark::showTimeSec(succeed, ': update.')
        end
        if ((exeType == -1) || (exeType == 8))
            Transactd::Benchmark::start()
            succeed = Updates(db, tb, start, endid, USE_TRANS, 20)
            Transactd::Benchmark::showTimeSec(succeed, ': update in transaction. 20rec x 1000times.')
        end
    end
    tb.close()
    db.close()
    printTail()
    return
end

args = ARGV
args.unshift(__FILE__)
main(args)
