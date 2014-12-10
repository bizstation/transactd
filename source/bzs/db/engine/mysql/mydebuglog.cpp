/* =================================================================
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
 ================================================================= */
#include <my_config.h>
#include "mydebuglog.h"
#include <bzs/db/protocol/tdap/mysql/tdapCommandExecuter.h>
#include <bzs/env/crosscompile.h>
#include <bzs/env/fileopen.h>

using namespace bzs::db::protocol::tdap::mysql;

#ifdef DEBUG_LOG
bzs::db::engine::mysql::debugdb dbg;
#endif

void bzs::rtl::debuglog::init()
{
#ifdef DEBUG_LOG
    bzs::rtl::debuglog::regist(&dbg);
#endif
    fn_format(logfilename, "transctd_debug", "", ".log",
              MY_REPLACE_EXT | MY_UNPACK_FILENAME);

    FILE* fp = fileOpen(logfilename, "w");
    if (fp)
    {
        fputs(dateTime(), fp);
        fputs("Transctd debug Start\n", fp);
        fclose(fp);
    }
}

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

char msg[1024];

void debugdb::stop()
{
    bzs::rtl::debuglog::write("Transctd debug End");
}

void debugdb::write(ushort_td op, request& req, bool error)
{
    bool writeflag = false;

    switch (op)
    {
    case TD_RESET_CLIENT:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg, 1024,
                  "TD_RESET_CLIENT result = %d \tdbname = %s\t cid = %d \n",
                  req.result, dbname.c_str(), req.cid);
        writeflag = true;
        break;
    }
    case TD_CREATETABLE:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg, 1024,
                  "TD_CREATETABLE result = %d \tdbname = %s\t table=%s\n",
                  req.result, dbname.c_str(), getTableName(req).c_str());
        writeflag = true;
        break;
    }
    case TD_OPENTABLE:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg, 1024,
                  "TD_OPENTABLE result = %d \tHandle = %d \tdbname = %s\n",
                  req.result, req.pbk->handle, dbname.c_str());
        writeflag = true;
        break;
    }
    case TD_CLOSETABLE:
        sprintf_s(msg, 1024, "TD_CLOSETABLE result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_SEEK:
        sprintf_s(msg, 1024, "TD_KEY_SEEK result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_AFTER:
        sprintf_s(msg, 1024, "TD_KEY_AFTER result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_OR_AFTER:
        sprintf_s(msg, 1024, "TD_KEY_OR_AFTER result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_BEFORE:
        sprintf_s(msg, 1024, "TD_KEY_BEFORE result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_OR_BEFORE:
        sprintf_s(msg, 1024, "TD_KEY_OR_BEFORE result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_FIRST:
        sprintf_s(msg, 1024, "TD_KEY_FIRST result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_PREV:
        sprintf_s(msg, 1024, "TD_KEY_PREV result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_LAST:
        sprintf_s(msg, 1024, "TD_KEY_LAST result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_NEXT:
        sprintf_s(msg, 1024, "TD_KEY_NEXT result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_REC_INSERT:
        sprintf_s(msg, 1024, "TD_REC_INSERT result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_REC_UPDATE:
        sprintf_s(msg, 1024, "TD_REC_UPDATE result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_REC_DELETE:
        sprintf_s(msg, 1024, "TD_REC_DELETE result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_BEGIN_TRANSACTION:
        sprintf_s(msg, 1024, "TD_BEGIN_TRANSACTION cid = %d \n", req.cid);
        writeflag = true;
        break;
    case TD_END_TRANSACTION:
        sprintf_s(msg, 1024, "TD_END_TRANSACTION cid = %d \n", req.cid);
        writeflag = true;
        break;
    case TD_ABORT_TRANSACTION:
        sprintf_s(msg, 1024, "TD_ABORT_TRANSACTION cid = %d \n", req.cid);
        writeflag = true;
        break;
    case TD_BEGIN_SHAPSHOT:
        sprintf_s(msg, 1024, "TD_BEGIN_SHAPSHOT cid = %d \n", req.cid);
        writeflag = true;
        break;
    case TD_END_SNAPSHOT:
        sprintf_s(msg, 1024, "TD_END_SNAPSHOT cid = %d \n", req.cid);
        writeflag = true;
        break;
    case TD_TABLE_INFO:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg, 1024, "TD_TABLE_INFO result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    }

    case TD_POS_FIRST:
        sprintf_s(msg, 1024, "TD_POS_FIRST result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_POS_LAST:
        sprintf_s(msg, 1024, "TD_POS_LAST result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_POS_NEXT:
        sprintf_s(msg, 1024, "TD_POS_NEXT result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_POS_PREV:
        sprintf_s(msg, 1024, "TD_POS_PREV result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_BOOKMARK:
        if (req.result == 0)
            sprintf_s(
                msg, 1024,
                "TD_BOOKMARK result = %d \tHandle = %d key = %d pos = %d\n",
                req.result, req.pbk->handle, req.keyNum, *((uint*)req.data));
        else
            sprintf_s(msg, 1024,
                      "TD_BOOKMARK result = %d \tHandle = %d key = %d \n",
                      req.result, req.pbk->handle, req.keyNum);
        writeflag = true;
        break;

    case TD_MOVE_BOOKMARK:
        sprintf_s(
            msg, 1024,
            "TD_MOVE_BOOKMARK result = %d \tHandle = %d key = %d pos = %d\n",
            req.result, req.pbk->handle, req.keyNum, *((uchar*)req.data));
        writeflag = true;
        break;

    case TD_GETDIRECTORY:
        sprintf_s(msg, 1024, "TD_GETDIRECTORY cid = %d \n", req.cid);
        writeflag = true;
        break;

    case TD_SET_OWNERNAME:
        sprintf_s(msg, 1024, "TD_SET_OWNERNAME result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_DROP_INDEX:
        sprintf_s(msg, 1024, "TD_DROP_INDEX result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_NEXT_MULTI:
        sprintf_s(msg, 1024, "TD_KEY_NEXT_MULTI result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_KEY_PREV_MULTI:
        sprintf_s(msg, 1024, "TD_KEY_PREV_MULTI result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_POS_NEXT_MULTI:
        sprintf_s(msg, 1024, "TD_POS_NEXT_MULTI result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_POS_PREV_MULTI:
        sprintf_s(msg, 1024, "TD_POS_PREV_MULTI result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_MOVE_PER:
        sprintf_s(msg, 1024, "TD_MOVE_PER result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_GET_PER:
        sprintf_s(msg, 1024, "TD_GET_PER result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_INSERT_BULK:
        sprintf_s(msg, 1024, "TD_INSERT_BULK result = %d \tHandle = %d\n",
                  req.result, req.pbk->handle);
        writeflag = true;
        break;
    case TD_VERSION:
        sprintf_s(msg, 1024, "TD_VERSION result = %d \n", req.result);
        writeflag = true;
        break;
    }
    if (error)
    {
        sprintf_s(msg, 1024, "ERROR result = %d op=%d\t\tHandle = %d\n",
                  req.result, op, req.pbk ? req.pbk->handle : 0);
        writeflag = true;
    }
    if (writeflag)
        bzs::rtl::debuglog::write(msg);
}

void debugdb::fieldDump(table* tb)
{
    boost::mutex::scoped_lock lck(m_mutex);
    FILE* fp = fileOpen(logfilename, "a+");
    if (fp)
    {
        int size;
        for (int j = 0; j < (int)tb->internalTable()->s->fields; j++)
        {
            fputs(tb->internalTable()->s->field[j]->field_name, fp);
            fputs("=", fp);
            fputs(tb->valStr(j, size), fp);
            fputs("\n", fp);
        }
        fclose(fp);
    }
}

void debugdb::writeInsert(table* tb, const char* packPtr, int packlen, int row,
                          int errorCount)
{
    char tmp[64];
    sprintf(tmp, "INSERT PACKED   Row=%d len=%d ErrorRows=%d ", packlen, row,
            errorCount);
    writeDump(tmp, packPtr + sizeof(ushort_td), packlen);
    uint len = tb->recordLen() - tb->nullBytes();
    sprintf(tmp, "INSERT UNPACKED Row=%d len=%d stat=%d", row, len, tb->stat());
    writeDump(tmp, (const char*)tb->record(), len);
    fieldDump(tb);
}

#ifdef DEBUG_READRECORDS

void debugdb::writeReadRecordsBegin(extResultDef* resultDef, extRequest* req)
{
    char tmp[2048];
    char tmp2[1024] = { 0x00 };
    sprintf(tmp, "READS RESULTDEF maxRows=%d fieldCount=%d\n",
            resultDef->maxRows, resultDef->fieldCount);
    for (int i = 0; i < resultDef->fieldCount; i++)
    {
        resultField* fd = &resultDef->field[i];
        sprintf(tmp2, "RESULTDEF FIELD%d pos =%d len=%d \n", i + 1, fd->pos,
                fd->len);
        strcat(tmp, tmp2);
    }
    sprintf(tmp2, "READS REQ len=%d reject=%d logicalCount=%d\n", req->len,
            req->rejectCount, req->logicalCount);
    strcat(tmp, tmp2);
    logicalField* field = &req->field;
    while (1)
    {
        sprintf(
            tmp2, "REQUEST FIELD1 type =%d len=%d pos=%d logType=%d opr=%d ",
            field->type, field->len, field->pos, field->logType, field->opr);
        strcat(tmp, tmp2);
        if (field->logType & CMPLOGICAL_FIELD)
            sprintf(tmp2, "offset=%d\n", field->offset);
        else
        {
            strcat(tmp, "Value=");
            for (int j = 0; j < field->len; j++)
            {
                sprintf(tmp2, "%02X ", *(field->ptr + j));
                strcat(tmp, tmp2);
            }
            strcat(tmp, "\n");
        }
        if (field->opr == 0)
            break; // this is last
        field = field->next();
    }
    write(tmp);
}

void debugdb::writeReadRecordsEnd(resultWriter* rw)
{
    dump("READS RESULT DUMP", rw->resultBuffer() + RETBUF_EXT_RESERVE_SIZE,
         rw->resultLen() - RETBUF_EXT_RESERVE_SIZE);
}
#endif // DEBUG_READRECORDS

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs
