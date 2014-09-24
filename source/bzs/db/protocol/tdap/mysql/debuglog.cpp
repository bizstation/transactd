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

#include "debuglog.h"
#include "btrvProtocol.h"
#include "recordsetReader.h"
#include "stl_uty.h"

namespace bzs
{
namespace msqlp
{

static char logfilename[FN_REFLEN];
static boost::mutex g_logMutex;

char msg[1024];
void writeDebuglog(ushort_td op, request& req, bool error)
{
    bool write = false;

    switch (op)
    {
    /*case TD_CONNECT:
    case TD_STOP_ENGINE:

            break;*/
    case TD_RESET_CLIENT:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg,
                  "%s TD_RESET_CLIENT result = %d \tdbname = %s\t cid = %d \n",
                  dateTime(), req.result, dbname.c_str(), req.cid);
        write = true;
        break;
    }
    case TD_CREATETABLE:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(
            msg, "%s TD_CREATETABLE result = %d \tdbname = %s\t table=%s\n",
            dateTime(), req.result, dbname.c_str(), getTableName(req).c_str());
        write = true;
        break;
    }
    case TD_OPENTABLE:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg,
                  "%s TD_OPENTABLE result = %d \tHandle = %d \tdbname = %s\n",
                  dateTime(), req.result, req.pbk->handle, dbname.c_str());
        write = true;
        break;
    }
    case TD_CLOSETABLE:
        sprintf_s(msg, "%s TD_CLOSETABLE result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_SEEK:
        sprintf_s(msg, "%s TD_KEY_SEEK result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_AFTER:
        sprintf_s(msg, "%s TD_KEY_AFTER result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_OR_AFTER:
        sprintf_s(msg, "%s TD_KEY_OR_AFTER result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_BEFORE:
        sprintf_s(msg, "%s TD_KEY_BEFORE result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_OR_BEFORE:
        sprintf_s(msg, "%s TD_KEY_OR_BEFORE result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_FIRST:
        sprintf_s(msg, "%s TD_KEY_FIRST result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_PREV:
        sprintf_s(msg, "%s TD_KEY_PREV result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_LAST:
        sprintf_s(msg, "%s TD_KEY_LAST result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_NEXT:
        sprintf_s(msg, "%s TD_KEY_NEXT result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_REC_INSERT:
        sprintf_s(msg, "%s TD_REC_INSERT result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_REC_UPDATE:
        sprintf_s(msg, "%s TD_REC_UPDATE result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_REC_DELETE:
        sprintf_s(msg, "%s TD_REC_DELETE result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_BEGIN_TRANSACTION:
        sprintf_s(msg, "%s TD_BEGIN_TRANSACTION cid = %d \n", dateTime(),
                  req.cid);
        write = true;
        break;
    case TD_END_TRANSACTION:
        sprintf_s(msg, "%s TD_END_TRANSACTION cid = %d \n", dateTime(),
                  req.cid);
        write = true;
        break;
    case TD_ABORT_TRANSACTION:
        sprintf_s(msg, "%s TD_ABORT_TRANSACTION cid = %d \n", dateTime(),
                  req.cid);
        write = true;
        break;
    case TD_BEGIN_SHAPSHOT:
        sprintf_s(msg, "%s TD_BEGIN_SHAPSHOT cid = %d \n", dateTime(), req.cid);
        write = true;
        break;
    case TD_END_SNAPSHOT:
        sprintf_s(msg, "%s TD_END_SNAPSHOT cid = %d \n", dateTime(), req.cid);
        write = true;
        break;
    case TD_TABLE_INFO:
    {
        std::string dbname = getDatabaseName(req);
        sprintf_s(msg, "%s TD_TABLE_INFO result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    }

    case TD_POS_FIRST:
        sprintf_s(msg, "%s TD_POS_FIRST result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_POS_LAST:
        sprintf_s(msg, "%s TD_POS_LAST result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_POS_NEXT:
        sprintf_s(msg, "%s TD_POS_NEXT result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_POS_PREV:
        sprintf_s(msg, "%s TD_POS_PREV result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_BOOKMARK:
        if (req.result == 0)
            sprintf_s(
                msg,
                "%s TD_BOOKMARK result = %d \tHandle = %d key = %d pos = %d\n",
                dateTime(), req.result, req.pbk->handle, req.keyNum,
                *((uint*)req.data));
        else
            sprintf_s(msg,
                      "%s TD_BOOKMARK result = %d \tHandle = %d key = %d \n",
                      dateTime(), req.result, req.pbk->handle, req.keyNum);
        write = true;
        break;

    case TD_MOVE_BOOKMARK:
        sprintf_s(
            msg,
            "%s TD_MOVE_BOOKMARK result = %d \tHandle = %d key = %d pos = %d\n",
            dateTime(), req.result, req.pbk->handle, req.keyNum,
            *((uchar*)req.data));
        write = true;
        break;

    case TD_GETDIRECTORY:
        sprintf_s(msg, "%s TD_GETDIRECTORY cid = %d \n", dateTime(), req.cid);
        write = true;
        break;

    /*case TD_VERSION:
            break;
    case TD_CLEAR_OWNERNAME:
            */
    case TD_SET_OWNERNAME:
        sprintf_s(msg, "%s TD_SET_OWNERNAME result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_DROP_INDEX:
        sprintf_s(msg, "%s TD_DROP_INDEX result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_NEXT_MULTI:
        sprintf_s(msg, "%s TD_KEY_NEXT_MULTI result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_KEY_PREV_MULTI:
        sprintf_s(msg, "%s TD_KEY_PREV_MULTI result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_POS_NEXT_MULTI:
        sprintf_s(msg, "%s TD_POS_NEXT_MULTI result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_POS_PREV_MULTI:
        sprintf_s(msg, "%s TD_POS_PREV_MULTI result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    case TD_MOVE_PER:
        sprintf_s(msg, "%s TD_MOVE_PER result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_GET_PER:
        sprintf_s(msg, "%s TD_GET_PER result = %d \tHandle = %d\n", dateTime(),
                  req.result, req.pbk->handle);
        write = true;
        break;
    case TD_INSERT_BULK:
        sprintf_s(msg, "%s TD_INSERT_BULK result = %d \tHandle = %d\n",
                  dateTime(), req.result, req.pbk->handle);
        write = true;
        break;
    }
    if (error)
    {
        sprintf_s(msg, "%s ERROR result = %d op=%d\t\tHandle = %d\n",
                  dateTime(), req.result, op, req.pbk ? req.pbk->handle : 0);
        write = true;
    }
    if (write)
    {
        boost::mutex::scoped_lock lck(g_logMutex);
        FILE* fp = fileOpne(logfilename, "a+");
        if (fp)
        {
            fputs(msg, fp);
            fclose(fp);
        }
    }
}
void writeDebuglog(const char* msg)
{
    boost::mutex::scoped_lock lck(g_logMutex);
    FILE* fp = fileOpne(logfilename, "a+");
    if (fp)
    {
        fputs(dateTime(), fp);
        fputs(" ", fp);
        fputs(msg, fp);
        fclose(fp);
    }
}
void writeDebuglogDump(const char* msg, const char* p, int size)
{
    boost::mutex::scoped_lock lck(g_logMutex);
    FILE* fp = fileOpne(logfilename, "a+");
    if (fp)
    {
        fputs(msg, fp);
        fputs("\n", fp);
        dump(fp, p, size, INT_MAX);

        fclose(fp);
    }
}
void fieldDump(table* tb)
{
    boost::mutex::scoped_lock lck(g_logMutex);
    FILE* fp = fileOpne(logfilename, "a+");
    if (fp)
    {
        for (int j = 0; j < (int)tb->m_table->s->fields; j++)
        {
            fputs(tb->m_table->s->field[j]->field_name, fp);
            fputs("=", fp);
            fputs(tb->valStr(j), fp);
            fputs("\n", fp);
        }
        fclose(fp);
    }
}

void initLog()
{
    fn_format(logfilename, "transctd_debug", "", ".log",
              MY_REPLACE_EXT | MY_UNPACK_FILENAME);

    FILE* fp = fileOpne(logfilename, "w");
    if (fp)
    {
        fputs(dateTime(), fp);
        fputs("Transctd debug Start", fp);
        fputs(msg, fp);
        fclose(fp);
    }
}
void endLog()
{
    writeDebuglog("Transctd debug End");
}
void dump(FILE* fp, const char* p, int size, int limit)
{
    size = std::min(size, limit);
    for (int i = 0; i < size; i += 16)
    {

        for (int j = 0; j < 16; j++)
            fprintf(fp, "%02X ", *((unsigned char*)(p + i + j)));
        fprintf(fp, " ");
        for (int j = 0; j < 16; j++)
            fprintf(fp, "%c", *((unsigned char*)(p + i + j)));
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}
void debugReadRecordsBegin(extResultDef* resultDef, extRequest* req)
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
    writeDebuglog(tmp);
}
void debugReadRecordsEnd(resultWriter* rw)
{
    writeDebuglogDump("READS RESULT DUMP",
                      rw->resultBuffer() + RETBUF_EXT_RESERVE_SIZE,
                      rw->resultLen() - RETBUF_EXT_RESERVE_SIZE);
}
void debugInsert(table* tb, const char* packPtr, int packlen, int row,
                 int errorCount)
{
    char tmp[64];
    sprintf(tmp, "INSERT PACKED   Row=%d len=%d ErrorRows=%d ", packlen, row,
            errorCount);
    writeDebuglogDump(tmp, packPtr + sizeof(ushort_td), packlen);
    sprintf(tmp, "INSERT UNPACKED Row=%d len=%ld stat=%d", row, tb->recordLen(),
            tb->stat());
    writeDebuglogDump(tmp, (const char*)tb->record(), tb->recordLen());
    fieldDump(tb);
}

} // namespace msqlp
} // namespace bzs

#ifdef DEBUG_PROFILE
unsigned int g_v;
#endif
