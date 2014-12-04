/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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
=================================================================*/

#include "tdapCommandExecuter.h"
#include "recordsetReader.h"
#include <bzs/db/blobBuffer.h>
#include <bzs/db/engine/mysql/database.h>
#include <bzs/db/engine/mysql/errorMessage.h>
#include <bzs/db/engine/mysql/mydebuglog.h>
#include <bzs/netsvc/server/IAppModule.h> //lookup for result value
#include <bzs/rtl/stl_uty.h>
#include <limits.h>
#include <boost/algorithm/string.hpp>
#ifdef WIN32
#include <mbstring.h>
#endif
#include "databaseSchema.h"
#include <bzs/db/transactd/connManager.h>
#include <bzs/rtl/exception.h>

namespace bzs
{
namespace db
{

using namespace engine::mysql;

namespace protocol
{
using namespace transactd;
using namespace transactd::connection;
namespace tdap
{
namespace mysql
{

static const char* TableNameTitle = "dbfile=";
static const char* BdfNameTitle = ".bdf";

class smartReadRecordsHandler
{
    ReadRecordsHandler* m_hdr;
    bool m_ended;

public:
    smartReadRecordsHandler(ReadRecordsHandler* hdr)
        : m_hdr(hdr), m_ended(false)
    {
    }
    unsigned int end()
    {
        m_ended = true;
        return m_hdr->end();
    }

    ~smartReadRecordsHandler()
    {
        if (!m_ended)
            m_hdr->end();
    }
};

std::string& toLowerCaseName(std::string& name, bool forSql)
{
    if (forSql)
    {
        char tmp[MAX_PATH];
        strcpy_s(tmp, MAX_PATH, name.c_str());
        my_casedn_str(global_system_variables.collation_server, tmp);
        name = tmp;
    }
    else
        boost::algorithm::to_lower(name);
    return name;
}

std::string getDatabaseName(const request& req, bool forSql)
{
    std::vector<std::string> ssc;
    std::vector<std::string> ss;
    if (req.keybuf)
    {
        std::string s((const char*)req.keybuf);
        split(ssc, s, "\t");
        if (ssc.size())
        {
            if (forSql && ssc.size() > 1)
                ssc.erase(ssc.begin(), ssc.begin() + 1);
            std::string path = ssc[0];
            std::string derim = "/";
            split(ss, path, derim); /* tdap://serverName/dbName?dbfile=xxx */
            if (ss.size() > 3)
            {
                std::vector<std::string> db;
                split(db, ss[3], "?");
                if (db.size() > 0)
                {
                    if (g_tableNmaeLower)
                        toLowerCaseName(db[0], forSql);
                    return db[0];
                }
            }
        }
    }
    return "";
}

std::string getTableName(const std::string& src, bool forSql)
{
    std::vector<std::string> ssc;
    split(ssc, src, "\t");
    if (forSql && ssc.size() > 1)
        ssc.erase(ssc.begin(), ssc.begin() + 1);

    std::string name = ssc[0];
    size_t pos = name.find(TableNameTitle);
    if (pos != std::string::npos)
    {
        pos += strlen(TableNameTitle);
        while ((name[pos] == '/') || (name[pos] == '\\'))
        {
            if (++pos == name.size())
                return "";
        }
        size_t pos2 = name.find(".", pos);
        if (pos2 == std::string::npos)
            pos2 = name.size();
        size_t pos3 = name.find("&", pos);
        if (pos3 != std::string::npos && (pos3 < pos2))
            pos2 = pos3;

        name = name.substr(pos, pos2 - pos);
        if (g_tableNmaeLower)
            toLowerCaseName(name, forSql);
        return name;
    }

    return "";
}

std::string getTableName(const request& req, bool forSql)
{

    if (req.keybuf)
    {
        std::string s((const char*)req.keybuf, req.keylen);
        return getTableName(s, forSql);
    }
    return "";
}

const char* getOwnerName(const request& req)
{
    const char* p = (const char*)req.data;
    if (*req.datalen && (p[*req.datalen - 1] == 0x00))
        return p;
    return "";
}

void dumpStdErr(int op, request& req, table* tb)
{
    boost::scoped_array<char> msg(new char[1024]);

    sprintf_s(msg.get(), 1024,
              "[Transactd] Exception:op=%d Handle=%d tablename=%s datalen=%d"
              " keynum=%d keylen=%d \n",
              op, req.pbk->handle, tb ? tb->name().c_str() : "", *req.datalen,
              req.keyNum, req.keylen);
    sql_print_error("%s", msg.get());
    // dump Keybuf
    if (req.keylen && req.keybuf)
    {
        sprintf_s(msg.get(), 1024, "[Transactd] Dump key buffer\n");
        sql_print_error("%s", msg.get());
        bzs::rtl::debuglog::dump(stderr, (char*)req.keybuf, req.keylen, 256);
    }
    // dump databuffer
    if (*req.datalen && req.data)
    {
        sprintf_s(msg.get(), 1024, "[Transactd] Dump data buffer\n");
        sql_print_error("%s", msg.get());
        bzs::rtl::debuglog::dump(stderr, (char*)req.data, (int)*req.datalen,
                                 256);
    }
}

//-------------------------------------------------------------
//  class dbExecuter
//-------------------------------------------------------------

dbExecuter::dbExecuter()
    : dbManager(), m_readHandler(new ReadRecordsHandler()),
      m_blobBuffer(new blobBuffer())
{
}

dbExecuter::~dbExecuter()
{
    delete m_blobBuffer;
    delete m_readHandler;
}

void dbExecuter::connect(request& req)
{
    req.paramMask = 0;
    if (req.keyNum == LG_SUBOP_DISCONNECT)
        dbManager::releaseDatabase(req.cid);
    else
    {
        std::string dbname = getDatabaseName(req);
        std::string dbSqlname = getDatabaseName(req, FOR_SQL);
        // exec SQL use database
        if (dbname != "")
        {
            database* db = getDatabase(dbname.c_str(), req.cid);
            dbSqlname.insert(0, "use ");
            req.result = ddl_execSql(db->thd(), dbSqlname);
        }
    }
}

void dbExecuter::releaseDatabase(request& req, int op)
{
    req.paramMask = 0;
    if ((op == TD_RESET_CLIENT) && (req.keyNum != 0))
        req.result = 1;
    else
        dbManager::releaseDatabase(req.cid);
}

std::string dbExecuter::makeSQLcreateTable(const request& req)
{
    return (const char*)req.data;
    // return sql;
}

int dbExecuter::errorCode(int ha_error)
{ // see mysqld_error.h or my_base.h or dbManager.h

    if (ha_error < HA_ERR_FIRST)
        return ha_error;
    else if (ha_error == HA_ERR_KEY_NOT_FOUND)
        return STATUS_NOT_FOUND_TI;
    else if (ha_error == HA_ERR_END_OF_FILE)
        return STATUS_EOF;
    else if (ha_error == HA_ERR_FOUND_DUPP_KEY)
        return STATUS_DUPPLICATE_KEYVALUE;
    else if (ha_error == HA_ERR_CRASHED_ON_USAGE)
        return STATUS_IO_ERROR;
    else if (ha_error == HA_ERR_NO_ACTIVE_RECORD)
        return STATUS_NO_CURRENT;
    else if ((ha_error == HA_ERR_LOCK_WAIT_TIMEOUT) ||
             (ha_error == HA_ERR_LOCK_DEADLOCK) ||
             (ha_error == ER_LOCK_WAIT_TIMEOUT))
        return STATUS_LOCK_ERROR;
    else if (ha_error == STATUS_INVALID_LOCKTYPE)
        return STATUS_INVALID_LOCKTYPE;
    else if (ha_error == HA_ERR_AUTOINC_ERANGE)
        return STATUS_DUPPLICATE_KEYVALUE;
    else if (ha_error == ER_PARSE_ERROR)
        return STATUS_CANT_CREATE;
    else if (ha_error == ER_TABLE_EXISTS_ERROR)
        return STATUS_TABLE_EXISTS_ERROR;
    else if (ha_error == DBM_ERROR_TABLE_USED)
        return STATUS_CANNOT_LOCK_TABLE;
    return 25000 + ha_error;
}

bool isMetaDb(const request& req)
{

    char buf[MAX_PATH];
    strncpy(buf, (char*)req.keybuf, MAX_PATH);
    _strlwr(buf);
    char_m* st = _mbsstr((char_m*)buf, (char_m*)BdfNameTitle);
    if (st == NULL)
        st = (char_m*)strstr(buf, TRANSACTD_SCHEMANAME);
    return (st != NULL);
}

inline void dbExecuter::doCreateTable(request& req)
{
    // if table name is mata table and database is nothing
    //	then cretate database too.
    std::string dbname = getDatabaseName(req);
    std::string dbSqlname = getDatabaseName(req, FOR_SQL);
    if (dbname != "")
    {
        std::string cmd;
        database* db = getDatabase(dbname.c_str(), req.cid);

        if (isMetaDb(req))
        { // for database operation
            if ((req.keyNum == 0) && (db->existsDatabase() == false))
                req.result = ddl_createDataBase(db->thd(), dbSqlname);
            else if (req.keyNum == CR_SUBOP_DROP)
            {
                std::string tableName = getTableName(req);
                if (db->existsTable(tableName))
                    req.result = ddl_dropTable(db, tableName, dbSqlname,
                                               getTableName(req, FOR_SQL));
                if (req.result == 0)
                    req.result = ddl_dropDataBase(db->thd(), dbname, dbSqlname);
                req.result = errorCodeSht(req.result);
                return;
            }
        }
        if (req.result == 0)
        { // table operation
            if ((req.result = ddl_useDataBase(db->thd(), dbSqlname)) == 0)
            {
                std::string tableSqlName = getTableName(req, FOR_SQL);
                std::string tableName = getTableName(req);
                if (req.keyNum == CR_SUBOP_DROP) // -128 is delete
                {

                    if (db->existsTable(tableName))
                        req.result = ddl_dropTable(db, tableName, dbSqlname,
                                                   tableSqlName);
                }
                else if (req.keyNum == CR_SUBOP_RENAME)
                { // rename new is keybuf
                    request reqold;
                    reqold.keybuf = req.data;
                    reqold.keylen = *req.datalen;
                    req.result = ddl_renameTable(
                        db, getTableName(reqold) /*oldname*/
                        ,
                        dbSqlname, getTableName(reqold, FOR_SQL) /*oldname*/
                        ,
                        tableSqlName /*newName*/);
                }
                else if (req.keyNum == CR_SUBOP_SWAPNAME)
                { // swap name name2 = keybuf
                    request reqold;
                    reqold.keybuf = req.data;
                    reqold.keylen = *req.datalen;
                    req.result = ddl_replaceTable(
                        db, getTableName(reqold) /*oldname*/
                        ,
                        tableName /*newName*/
                        ,
                        dbSqlname, getTableName(reqold, FOR_SQL) /*oldname*/
                        ,
                        tableSqlName /*newName*/);
                }
                else
                { // create
                    if (req.data == NULL)
                        req.result = 1;
                    else
                    { //-1 is overwrite
                        if ((req.keyNum == CR_SUB_FLAG_EXISTCHECK) &&
                            (db->existsTable(tableName)))
                            req.result = ddl_dropTable(db, tableName, dbSqlname,
                                                       tableSqlName);
                        if (req.result == 0)
                            req.result =
                                ddl_execSql(db->thd(), makeSQLcreateTable(req));
                    }
                }
            }
        }
    }
    else
        req.result = 1;
    req.result = errorCodeSht(req.result);
}

// open table and assign handle
inline void dbExecuter::doOpenTable(request& req)
{
    std::string dbname = getDatabaseName(req);
    if (dbname != "")
    {
        database* db = getDatabase(dbname.c_str(), req.cid);
        m_tb = db->openTable(
            getTableName(req), req.keyNum,
            getOwnerName(req)); // if error occured that throw exception
        req.result = db->stat();
        if (m_tb)
        {
            req.pbk->handle = addHandle(getDatabaseID(req.cid), m_tb->id());
            m_tb = getTable(req.pbk->handle);
            m_tb->setBlobBuffer(m_blobBuffer);
            req.paramMask = P_MASK_POSBLK;
        }
    }
    else
        req.result = 1;
}

inline void readAfter(request& req, table* tb, dbExecuter* dbm)
{
    if (tb->stat() == 0)
    {
        if ((req.op >= TD_KEY_EQUAL_KO) && (req.op <= TD_KEY_LAST_KO))
            req.paramMask = P_MASK_POSBLK;
        else
        {
            req.paramMask = P_MASK_READRESULT;
            if (tb->blobFields())
                req.paramMask |= P_MASK_BLOBBODY;
            req.data = tb->record();
        }
    }
    req.result = dbm->errorCodeSht(tb->stat());
}

inline void dbExecuter::doSeekKey(request& req, int op)
{
    bool read = true;
    m_tb = getTable(req.pbk->handle);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        m_tb->setKeyValuesPacked((const uchar*)req.keybuf, req.keylen);
        if (read)
        {
            ha_rkey_function flag = HA_READ_KEY_EXACT;
            if (op == TD_KEY_SEEK)
                flag = HA_READ_KEY_EXACT;
            else if (op == TD_KEY_AFTER)
                flag = HA_READ_AFTER_KEY;
            else if (op == TD_KEY_OR_AFTER)
                flag = HA_READ_KEY_OR_NEXT;
            else if (op == TD_KEY_BEFORE)
                flag = HA_READ_BEFORE_KEY;
            else if (op == TD_KEY_OR_BEFORE)
                flag = HA_READ_KEY_OR_PREV;
            m_tb->seekKey(flag, m_tb->keymap());
        }
    }
    readAfter(req, m_tb, this);
}

inline void dbExecuter::doMoveFirst(request& req)
{
    m_tb = getTable(req.pbk->handle);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        if (m_tb->isNisKey(m_tb->keyNum()))
        {
            m_tb->clearKeybuf();
            m_tb->seekKey(HA_READ_KEY_OR_NEXT, m_tb->keymap());
        }
        else
            m_tb->getFirst();
    }
    readAfter(req, m_tb, this);
}

inline void dbExecuter::doMoveKey(request& req, int op)
{
    m_tb = getTable(req.pbk->handle);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        if (op == TD_KEY_FIRST)
            m_tb->getFirst();
        else if (op == TD_KEY_LAST)
            m_tb->getLast();
        else if (op == TD_KEY_NEXT)
            m_tb->getNext();
        else if (op == TD_KEY_PREV)
            m_tb->getPrev();
    }
    readAfter(req, m_tb, this);
}

inline int dbExecuter::doReadMultiWithSeek(request& req, int op,
                                           netsvc::server::netWriter* nw)
{
    DEBUG_MEMDUMP("doReadMultiWithSeek ", req.data, (*((short*)req.data)))
    int ret = 1;
    m_tb = getTable(req.pbk->handle);
    char keynum = m_tb->keyNumByMakeOrder(req.keyNum);
    if (m_tb->setKeyNum(keynum))
    {
        m_tb->setKeyValuesPacked((const uchar*)req.keybuf, req.keylen);
        m_tb->seekKey((op == TD_KEY_GE_NEXT_MULTI) ? HA_READ_KEY_OR_NEXT
                                                   : HA_READ_KEY_OR_PREV,
                      m_tb->keymap());

        extRequest* ereq = (extRequest*)req.data;
        bool noBookmark = (ereq->itype & FILTER_CURRENT_TYPE_NOBOOKMARK) != 0;
        bool execPrepared = (ereq->itype & FILTER_TYPE_SUPPLYVALUE) != 0;
        // smartReadRecordsHandler scope
        {
            smartReadRecordsHandler srrh(m_readHandler);

            if (execPrepared)
            {
                if (m_tb->preparedStatements.size() < ereq->preparedId)
                    req.result = STATUS_INVALID_PREPAREID;
                else
                    req.result = m_readHandler->beginPreparExecute(m_tb, ereq, true, nw,
                                               noBookmark, 
                                              (prepared*)m_tb->preparedStatements[ereq->preparedId - 1]);
            }
            else
                req.result = m_readHandler->begin(m_tb, ereq, true, nw,
                                              (op == TD_KEY_GE_NEXT_MULTI),
                                              noBookmark);
            if (req.result != 0)
                return ret;
            if (m_tb->stat() == 0)
            {
                if (op == TD_KEY_GE_NEXT_MULTI)
                    m_tb->getNextExt(m_readHandler, true, noBookmark);
                else if (op == TD_KEY_LE_PREV_MULTI)
                    m_tb->getPrevExt(m_readHandler, true, noBookmark);
            }
            req.result = errorCodeSht(m_tb->stat());
            DEBUG_WRITELOG2(op, req);
        }
        short dummy = 0;
        size_t& size = nw->datalen;
        size = req.serializeForExt(m_tb, nw);
        char* resultBuffer = nw->ptr();
        netsvc::server::buffers* optionalData = nw->optionalData();
        if ((req.paramMask & P_MASK_BLOBBODY) && m_blobBuffer->fieldCount())
            size = req.serializeBlobBody(m_blobBuffer, resultBuffer, size,
                                         FILE_MAP_SIZE, optionalData, dummy);

        DEBUG_PROFILE_END_OP(1, op)
        ret = EXECUTE_RESULT_SUCCESS;
    }
    if (m_tb)
        m_tb->unUse();
    return ret;
}

inline int dbExecuter::doReadMulti(request& req, int op,
                                   netsvc::server::netWriter* nw)
{
    DEBUG_MEMDUMP("doReadMulti ", req.data, (*((short*)req.data)))
    int ret = 1;
    m_tb = getTable(req.pbk->handle);
    extRequest* ereq = (extRequest*)req.data;
    bool incCurrent = (ereq->itype & FILTER_CURRENT_TYPE_INC) != 0;
    bool noBookmark = (ereq->itype & FILTER_CURRENT_TYPE_NOBOOKMARK) != 0;
    bool execPrepared = (ereq->itype & FILTER_TYPE_SUPPLYVALUE) != 0;

    bool forword = (op == TD_KEY_NEXT_MULTI) || (op == TD_POS_NEXT_MULTI);
    if (op == TD_KEY_SEEK_MULTI)
    {
        char keynum = m_tb->keyNumByMakeOrder(req.keyNum);
        if (!m_tb->setKeyNum(keynum))
            return ret;
    }
    // smartReadRecordsHandler scope
    {
        smartReadRecordsHandler srrh(m_readHandler);
        if (execPrepared)
        {
            if (m_tb->preparedStatements.size() < ereq->preparedId)
                req.result = STATUS_INVALID_PREPAREID;
            else
                req.result = m_readHandler->beginPreparExecute(m_tb, ereq, (op != TD_KEY_SEEK_MULTI),
                                          nw, noBookmark, 
                                          (prepared*)m_tb->preparedStatements[ereq->preparedId - 1]);
        }
        else
            req.result = m_readHandler->begin(m_tb, ereq, (op != TD_KEY_SEEK_MULTI),
                                          nw, forword, noBookmark);
        if (req.result != 0)
            return ret;
        if (op == TD_KEY_SEEK_MULTI)
            req.result =
                errorCodeSht(seekEach((extRequestSeeks*)req.data, noBookmark));
        else
        {
            if (op == TD_KEY_NEXT_MULTI)
                m_tb->getNextExt(m_readHandler, incCurrent, noBookmark);
            else if (op == TD_KEY_PREV_MULTI)
                m_tb->getPrevExt(m_readHandler, incCurrent, noBookmark);
            else if (op == TD_POS_NEXT_MULTI)
                m_tb->stepNextExt(m_readHandler, incCurrent, noBookmark);
            else if (op == TD_POS_PREV_MULTI)
                m_tb->stepPrevExt(m_readHandler, incCurrent, noBookmark);
            req.result = errorCodeSht(m_tb->stat());
        }
        DEBUG_WRITELOG2(op, req);
    }

    short dummy = 0;
    size_t& size = nw->datalen;
    size = req.serializeForExt(m_tb, nw);

    char* resultBuffer = nw->ptr();
    netsvc::server::buffers* optionalData = nw->optionalData();
    if ((req.paramMask & P_MASK_BLOBBODY) && m_blobBuffer->fieldCount())
        size = req.serializeBlobBody(m_blobBuffer, resultBuffer, size,
                                     FILE_MAP_SIZE, optionalData, dummy);

    DEBUG_PROFILE_END_OP(1, op)

    ret = EXECUTE_RESULT_SUCCESS;

    if (m_tb)
        m_tb->unUse();
    return ret;
}

inline __int64 intValue(const unsigned char* p, int len)
{
    switch (len)
    {
    case 1:
        return *((char*)p);
    case 2:
        return *((short*)p);
    case 3:
        return *((int*)p) & 0xFFFFFF;
    case 4:
        return *((int*)p);
    case 8:
        return *((__int64*)p);
    }
    return 0;
}

inline short dbExecuter::seekEach(extRequestSeeks* ereq, bool noBookmark)
{

    short stat = 0;
    seek* fd = &ereq->seekData;
    short seg = m_tb->setKeyValuesPacked(fd->ptr, fd->len);
    key_part_map keyMap = m_tb->keymap();
    if (!noBookmark)
    {
        if (seg != -1)
        {
            keyMap = (1U << seg) - 1;
            seg = 1;
        }
        else
            seg = !m_tb->isUniqueKey();
    }
    else
        seg = 0;

    // Duplicate records need a bookmark.
    if (seg && noBookmark)
        return STATUS_INVALID_BOOKMARK;

    for (int i = 0; i < ereq->logicalCount; ++i)
    {
        m_tb->setKeyValuesPacked(fd->ptr, fd->len);
        m_tb->seekKey(HA_READ_KEY_EXACT, keyMap);
        if (m_tb->stat() == 0)
        {
            if (seg)
            {
                // If duplicate records , bookmark space is request row number.
                stat = m_readHandler->write((uchar*)&i, 4);
            }
            else if (noBookmark)
                stat = m_readHandler->write((const unsigned char *)_T("dummy"), 0);
            else
                stat =
                    m_readHandler->write(m_tb->position(), m_tb->posPtrLen());
        }
        else
            stat = m_readHandler->write(NULL, 0);
        if (stat)
            break;

        // for hasMany join
        if (seg)
        {
            while (m_tb->stat() == 0)
            {
                m_tb->getNextSame(keyMap);
                if (m_tb->stat() == 0)
                    stat = m_readHandler->write(
                        (uchar*)&i, 4); // write seek sequential number
                if (stat)
                    break;
            }
            if (stat)
                break;
        }

        fd = fd->next();
    }
    if (stat == 0)
        stat = STATUS_REACHED_FILTER_COND;
    return stat;
}

inline void dbExecuter::doStepRead(request& req, int op)
{
    m_tb = getTable(req.pbk->handle);
    if (op == TD_POS_FIRST)
        m_tb->stepFirst();
    else if (op == TD_POS_LAST)
        m_tb->stepLast();
    else if (op == TD_POS_NEXT)
        m_tb->stepNext();
    else if (op == TD_POS_PREV)
        m_tb->stepPrev();
    readAfter(req, m_tb, this);
}

inline void dbExecuter::doInsert(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_INSERT);
    bool ncc = (req.keyNum == -1);
    if (!ncc)
    {
        if (!m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
        {
            req.result = errorCodeSht(m_tb->stat());
            return;
        }
    }
    m_tb->clearBuffer();
    m_tb->setRecordFromPacked((const uchar*)req.data, *(req.datalen),
                              req.blobHeader);
    smartBulkInsert sbi(m_tb, 1);
    __int64 aincValue = m_tb->insert(ncc);
    req.result = errorCodeSht(m_tb->stat());
    if (aincValue)
    {
        req.paramMask = P_MASK_INS_AUTOINC;
        req.data = m_tb->record();
    }
    else
        req.paramMask = P_MASK_POSBLK | P_MASK_KEYBUF;
}

inline void dbExecuter::doUpdateKey(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_UPDATE);
    char keynum = m_tb->keyNumByMakeOrder(req.keyNum);
    if (m_tb->setKeyNum(keynum))
    {
        m_tb->setKeyValuesPacked((const uchar*)req.keybuf, req.keylen);
        m_tb->seekKey(HA_READ_KEY_EXACT, m_tb->keymap());
        if (m_tb->stat() == 0)
        {
            m_tb->beginUpdate(keynum);
            if (m_tb->stat() == 0)
            {
                m_tb->setRecordFromPacked((const uchar*)req.data,
                                          *(req.datalen), req.blobHeader);
                m_tb->update(true);
            }
        }
    }
    req.result = errorCodeSht(m_tb->stat());
    req.paramMask = P_MASK_POSBLK | P_MASK_KEYBUF;
}

inline void dbExecuter::doUpdate(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_UPDATE);
    bool ncc = (req.keyNum == -1);
    m_tb->beginUpdate(req.keyNum);
    if (m_tb->stat() == 0)
    {
        m_tb->setRecordFromPacked((const uchar*)req.data, *(req.datalen),
                                  req.blobHeader);
        m_tb->update(ncc);
    }
    req.result = errorCodeSht(m_tb->stat());
    req.paramMask = P_MASK_POSBLK | P_MASK_KEYBUF;
}

inline void dbExecuter::doDeleteKey(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_DELETE);
    char keynum = m_tb->keyNumByMakeOrder(req.keyNum);
    if (m_tb->setKeyNum(keynum))
    {
        m_tb->setKeyValuesPacked((const uchar*)req.keybuf, req.keylen);
        m_tb->seekKey(HA_READ_KEY_EXACT, m_tb->keymap());
        if (m_tb->stat() == 0)
        {
            m_tb->beginDel();
            if (m_tb->stat() == 0)
                m_tb->del();
        }
    }
    req.result = errorCodeSht(m_tb->stat());
    req.paramMask = P_MASK_POSBLK;
}

inline void dbExecuter::doDelete(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_DELETE);
    m_tb->beginDel();
    if (m_tb->stat() == 0)
        m_tb->del();
    req.result = errorCodeSht(m_tb->stat());
    req.paramMask = P_MASK_POSBLK;
}

inline void dbExecuter::doInsertBulk(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_INSERT);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        ushort_td* n = (ushort_td*)req.data;
        ushort_td ret = 0;
        const uchar* pos = (const uchar*)req.data + sizeof(ushort_td);
        int ins_rows = 0;
        BUILINSERT_SCOPE
        {
            smartBulkInsert sbi(m_tb, *n);
            for (ushort_td i = 0; i < *n; i++)
            {
                ushort_td len = *((ushort_td*)pos);
                if (pos + len > (const uchar*)req.data + *req.datalen)
                {
                    ret = STATUS_BUFFERTOOSMALL;
                    break;
                }
                else
                {
                    m_tb->clearBuffer();
                    m_tb->setRecordFromPacked(pos + sizeof(ushort_td), len,
                                              req.blobHeader);

                    if (i == *n - 1)
                        m_tb->insert((req.keyNum != -1));
                    else
                        m_tb->insert(true);
                    ret = m_tb->stat();
                }
                if (ret == 0)
                    ins_rows++;
                DEBUG_INSERT(m_tb, pos + sizeof(ushort_td), len, i,
                             i + 1 - ins_rows)
                pos += len + sizeof(ushort_td);
            }
            *n = ins_rows;
        }
        req.result = errorCodeSht(ret);
        req.resultLen = 4;
        req.paramMask = P_MASK_READ_EXT;
    }
    else
        req.result = errorCodeSht(m_tb->stat());
}

inline void dbExecuter::doStat(request& req)
{
    m_tb = getTable(req.pbk->handle);
    req.paramMask = P_MASK_STAT;
    req.resultLen = *req.datalen;
    if (req.resultLen >= 6 + sizeof(uint))
    {
        ushort_td len = (ushort_td)(m_tb->recordLenCl());
#ifdef USE_BTRV_VARIABLE_LEN
        if (m_tb->recordFormatType() & RF_FIXED_PLUS_VALIABLE_LEN)
            len -=
                m_tb->lastVarFiled()->pack_length() - m_tb->lastVarLenBytes();
#endif
        memcpy((char*)req.data, &len, sizeof(ushort_td));
        uint rows = (uint)m_tb->recordCount((req.keyNum != 0));
        memcpy((char*)req.data + 6, &rows, sizeof(uint));
    }
    else
        req.result = STATUS_BUFFERTOOSMALL;
}

inline short getTrnsactionType(int op)
{
    if (op > PARALLEL_TRN) // 1000
        op -= PARALLEL_TRN;
    if (op > NOWAIT_WRITE) // 500
        op -= NOWAIT_WRITE;
    if (op >= 300)
        return TRN_RECORD_LOCK_MUILTI;
    return TRN_RECORD_LOCK_SINGLE;
}

int dbExecuter::commandExec(request& req, netsvc::server::netWriter* nw)
{
    DEBUG_PROFILE_START(1)
    m_tb = NULL;
    char* resultBuffer = nw->ptr();
    size_t& size = nw->datalen;
    netsvc::server::buffers* optionalData = nw->optionalData();

    int op = req.op % 100;
    int opTrn = req.op;
    if (op == 99)
        return 0;

    if ((op >= TD_KEY_EQUAL_KO) && (op <= TD_KEY_LAST_KO))
        op -= 50;

    try
    {
        posblk* pbk = req.pbk;
        posblk tmp;
        bool transactionResult = false;
        if (pbk == NULL)
            pbk = &tmp;
        req.reset();
        switch (op)
        {
        case TD_GETSERVER_CHARSET:
        {
            if (*req.datalen == sizeof(trdVersiton))
            {
                trdVersiton* ver = (trdVersiton*)req.data;
                strcpy_s(ver->cherserServer, sizeof(ver->cherserServer),
                         global_system_variables.collation_server->csname);
                ver->srvMajor = TRANSACTD_VER_MAJOR;
                ver->srvMinor = TRANSACTD_VER_MINOR;
                ver->srvRelease = TRANSACTD_VER_RELEASE;
                req.resultLen = sizeof(trdVersiton);
                // req.data = (void*)
                // global_system_variables.collation_server->csname;
                // req.resultLen = (uint_td)strlen(
                // global_system_variables.collation_server->csname);
                req.paramMask |= P_MASK_DATA | P_MASK_DATALEN;
            }
            else
                req.result = SERVER_CLIENT_NOT_COMPATIBLE;
            break;
        }
        case TD_CONNECT:
            connect(req);
            break;
        case TD_RESET_CLIENT:
        case TD_STOP_ENGINE: // close all table
            releaseDatabase(req, op);
            break;
        case TD_AUTOMEKE_SCHEMA:
            m_tb = getTable(req.pbk->handle, SQLCOM_INSERT);
            req.result = schemaBuilder().execute(getDatabaseCid(req.cid), m_tb);
            break;
        case TD_CREATETABLE:
            doCreateTable(req);
            break;
        case TD_OPENTABLE:
            doOpenTable(req);
            break;
        case TD_CLOSETABLE:
            m_tb = getTable(req.pbk->handle);
            if (m_tb)
            {
                m_tb->close();
                m_tb = NULL;
            }
            break;
        case TD_KEY_SEEK:
        case TD_KEY_AFTER:
        case TD_KEY_OR_AFTER:
        case TD_KEY_BEFORE:
        case TD_KEY_OR_BEFORE:
            doSeekKey(req, op);
            break;
        case TD_KEY_FIRST:
            doMoveFirst(req);
            break;
        case TD_KEY_PREV:
        case TD_KEY_LAST:
        case TD_KEY_NEXT:
            doMoveKey(req, op);
            break;
        case TD_REC_INSERT:
            doInsert(req);
            break;
        case TD_REC_UPDATEATKEY:
            doUpdateKey(req);
            break;
        case TD_REC_UPDATE:
            doUpdate(req);
            break;
        case TD_REC_DELLETEATKEY:
            doDeleteKey(req);
            break;
        case TD_REC_DELETE:
            doDelete(req);
            break;
        case TD_BEGIN_TRANSACTION:
            transactionResult =
                getDatabaseCid(req.cid)->beginTrn(getTrnsactionType(opTrn));
            break;
        case TD_END_TRANSACTION:
            transactionResult = getDatabaseCid(req.cid)->commitTrn();
            break;
        case TD_ABORT_TRANSACTION:
            transactionResult = getDatabaseCid(req.cid)->abortTrn();
            break;
        case TD_BEGIN_SHAPSHOT:
            transactionResult = getDatabaseCid(req.cid)->beginSnapshot();
            break;
        case TD_END_SNAPSHOT:
            transactionResult = getDatabaseCid(req.cid)->endSnapshot();
            break;
        case TD_TABLE_INFO: // support recordlen and recordCount only.
            doStat(req);
            break;
        case TD_POS_FIRST:
        case TD_POS_LAST:
        case TD_POS_NEXT:
        case TD_POS_PREV:
            doStepRead(req, op);
            break;
        case TD_BOOKMARK:
            m_tb = getTable(req.pbk->handle);
            req.paramMask = P_MASK_MOVPOS;
            req.data = (void*)m_tb->position();
            req.resultLen = m_tb->posPtrLen();
            break;
        case TD_MOVE_BOOKMARK:
            m_tb = getTable(req.pbk->handle);
            m_tb->movePos((uchar*)req.data,
                          m_tb->keyNumByMakeOrder(req.keyNum));
            readAfter(req, m_tb, this);
            break;
        case TD_GETDIRECTORY:
        {
            database* db = getDatabaseCid(req.cid);
            if (db->name().size() < 64)
            {
                req.keylen = (uchar_td)db->name().size() + 1;
                req.keybuf = (void*)db->name().c_str();
                req.paramMask = P_MASK_KEYBUF;
            }
            else
                req.result = STATUS_BUFFERTOOSMALL;
            break;
        }
        case TD_VERSION:
            if (*req.datalen >= sizeof(version) * 3)
            {
                version* v = (version*)req.data;
                ++v;
                v->majorVersion = MYSQL_VERSION_ID / 10000;
                v->minorVersion = (MYSQL_VERSION_ID / 100) % 100;
                v->Type = 'M';
                ++v;
                v->majorVersion = TRANSACTD_VER_MAJOR;
                v->minorVersion = TRANSACTD_VER_MINOR;
                v->Type = 'T';
                req.paramMask = P_MASK_DATA | P_MASK_DATALEN;
                req.resultLen = sizeof(version) * 3;
            }
            else
                req.result = STATUS_BUFFERTOOSMALL;
            break;
        case TD_CLEAR_OWNERNAME:
            req.keybuf = (void_td*)"";
        case TD_SET_OWNERNAME:
        {
            database* db = getDatabaseCid(req.cid);
            m_tb = getTable(req.pbk->handle);
            int num = (req.keyNum > 1) ? req.keyNum - 2 : req.keyNum;
            num += '0';
            std::string s("%@%");
            s += (const char*)&num;
            s += (const char*)req.keybuf;
            req.result = ddl_execSql(
                db->thd(),
                makeSQLChangeTableComment(db->name(), m_tb->name(), s.c_str()));
            break;
        }
        case TD_DROP_INDEX:
        { // Key name of multi byte charctord is not supported. Use only ascii.
            database* db = getDatabaseCid(req.cid);
            m_tb = getTable(req.pbk->handle);
            req.result = ddl_execSql(
                db->thd(),
                makeSQLDropIndex(
                    db->name(), m_tb->name(),
                    m_tb->keyName(m_tb->keyNumByMakeOrder(req.keyNum))));
            break;
        }
        case TD_KEY_GE_NEXT_MULTI:
        case TD_KEY_LE_PREV_MULTI:
            nw->setClientBuffferSize(*(req.datalen));
            if (doReadMultiWithSeek(req, op, nw) == EXECUTE_RESULT_SUCCESS)
            {
                m_tb = NULL;
                return EXECUTE_RESULT_SUCCESS;
            }
            break;
        case TD_KEY_SEEK_MULTI:
        case TD_KEY_NEXT_MULTI:
        case TD_KEY_PREV_MULTI:
        case TD_POS_NEXT_MULTI:
        case TD_POS_PREV_MULTI:
            nw->setClientBuffferSize(*(req.datalen));
            if (doReadMulti(req, op, nw) == EXECUTE_RESULT_SUCCESS)
            {
                m_tb = NULL;
                return EXECUTE_RESULT_SUCCESS;
            }
            break;
        case TD_FILTER_PREPARE:
            m_tb = getTable(req.pbk->handle);
            if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
            {
                prepared* prp = new prepared();
                m_tb->preparedStatements.push_back(prp);
                extRequest* ereq = (extRequest*)req.data;
                req.result = m_readHandler->prepare(
                                            m_tb,
                                            ereq, 
                                            ((ereq->itype & FILTER_TYPE_SEEKS) == 0),
                                            nw,
                                            ((ereq->itype & FILTER_TYPE_FORWORD) != 0),
                                            ((ereq->itype & FILTER_CURRENT_TYPE_NOBOOKMARK) != 0),
                                            prp);
                if (req.result != 0)
                    m_tb->preparedStatements.pop_back();
                else
                {
                    req.paramMask = P_MASK_DATA | P_MASK_DATALEN;
                    *((unsigned short*)req.data) = (unsigned short)m_tb->preparedStatements.size();
                    req.resultLen = 2;
                }
            }
            break;
        case TD_MOVE_PER:
            m_tb = getTable(req.pbk->handle);
            if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
                m_tb->getByPercentage(*((ushort_td*)req.data));
            readAfter(req, m_tb, this);
            break;
        case TD_GET_PER:
            m_tb = getTable(req.pbk->handle);
            m_tb->calcPercentage();
            req.result = errorCodeSht(m_tb->stat());
            if (m_tb->stat() == 0)
            {
                req.paramMask = P_MASK_DATA | P_MASK_DATALEN;
                req.data = m_tb->percentResult();
                req.resultLen = sizeof(int);
            }
            break;
        case TD_INSERT_BULK:
            doInsertBulk(req);
            break;
        }
        if (m_tb)
            m_tb->unUse();
        DEBUG_WRITELOG2(op, req)
        size = req.serialize(m_tb, resultBuffer);
        short dymmy = 0;
        if ((req.result == 0) && (req.paramMask & P_MASK_BLOBBODY) &&
            m_blobBuffer->fieldCount())
            size = req.serializeBlobBody(m_blobBuffer, resultBuffer, size,
                                         FILE_MAP_SIZE, optionalData, dymmy);

        m_tb = NULL;
        if (transactionResult)
        {
            if ((op == TD_BEGIN_TRANSACTION) || (op == TD_BEGIN_SHAPSHOT))
                return EXECUTE_RESULT_FORCSE_SYNC;
            return EXECUTE_RESULT_FORCSE_ASYNC;
        }
        DEBUG_PROFILE_END_OP(1, op)
        return EXECUTE_RESULT_SUCCESS;
    }

    catch (bzs::rtl::exception& e)
    {
        clenupNoException();
        req.reset();
        const int* code = getCode(e);
        if (code)
            req.result = *code;
        else
        {
            req.result = 20000;
            sql_print_error("%s", boost::diagnostic_information(e).c_str());
        }
        printErrorMessage(code, getMsg(e));
    }

    catch (...)
    {
        clenupNoException();
        req.reset();
        req.result = 20001;
        dumpStdErr(op, req, m_tb);
        try
        {
            if (m_tb)
                m_tb->close();
        }
        catch (...)
        {
        }
    }

    DEBUG_WRITELOG3(op, req, true);
    req.paramMask = 0;
    size = req.serialize(NULL, resultBuffer);
    m_tb = NULL;
    return EXECUTE_RESULT_SUCCESS;
}

// ---------------------------------------------------------------------------
//		class connMgrExecuter
// ---------------------------------------------------------------------------
connMgrExecuter::connMgrExecuter(request& req, unsigned __int64 parent)
    : m_req(req), m_modHandle(parent)
{
}

int connMgrExecuter::read(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    unsigned __int64* mod = (unsigned __int64*)m_req.keybuf;

    const connManager::records& records = st.getRecords(mod[0], (int)mod[1]);
    m_req.reset();
    m_req.paramMask = P_MASK_DATA | P_MASK_DATALEN;
    if (records.size())
        m_req.data = (void*)&records[0];
    else
        m_req.paramMask = P_MASK_DATALEN;
    m_req.resultLen = (uint_td)(sizeof(record) * records.size());
    size = m_req.serialize(NULL, buf);
    return EXECUTE_RESULT_SUCCESS;
}

int connMgrExecuter::disconnectOne(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    unsigned __int64* mod = (unsigned __int64*)m_req.keybuf;
    st.disconnect(*mod);
    m_req.reset();
    size = m_req.serialize(NULL, buf);
    return EXECUTE_RESULT_SUCCESS;
}

int connMgrExecuter::disconnectAll(char* buf, size_t& size)
{
    connManager st(m_modHandle);

    st.disconnectAll();
    m_req.reset();
    size = m_req.serialize(NULL, buf);
    return EXECUTE_RESULT_SUCCESS;
}

int connMgrExecuter::commandExec(netsvc::server::netWriter* nw)
{
    if (m_req.keyNum == TD_STSTCS_READ)
        return read(nw->ptr(), nw->datalen);
    else if (m_req.keyNum == TD_STSTCS_DISCONNECT_ONE)
        return disconnectOne(nw->ptr(), nw->datalen);
    else if (m_req.keyNum == TD_STSTCS_DISCONNECT_ALL)
        return disconnectAll(nw->ptr(), nw->datalen);
    return 0;
}

// ---------------------------------------------------------------------------
//		class commandExecuter
// ---------------------------------------------------------------------------
commandExecuter::commandExecuter(__int64 parent) : m_modHandle(parent)
{
    m_dbExec.reset(new dbExecuter());
}

commandExecuter::~commandExecuter()
{
    m_dbExec.reset();
}

size_t commandExecuter::perseRequestEnd(const char* p, size_t transfered,
                                        bool& comp) const
{
    if (transfered < sizeof(unsigned int))
        return 0;
    size_t size = *((unsigned int*)p);
    if (size == transfered)
        comp = true;
    return size;
}

bool commandExecuter::parse(const char* p, size_t size)
{
    m_req.parse(p);
    return 0;
}

} // namespace mysql
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
