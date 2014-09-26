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
#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_DEBUGLOG_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_DEBUGLOG_H

#include "request.h"
namespace bzs
{
namespace msqlp
{
struct extResultDef;
struct extRequest;
class resultWriter;

extern char msg[1024];

void writeDebuglog(const char* msg);
void writeDebuglogDump(const char* msg, const char* p, int size);
void initLog();
void endLog();
void dump(FILE* fp, const char* p, int size, int limit);

void writeDebuglog(ushort_td op, request& req, bool error = false);
void fieldDump(table* tb);
void debugInsert(table* tb, const char* packPtr, int packlen, int row,
                 int errorCount);
void debugReadRecordsBegin(extResultDef* resultDef, extRequest* req);
void debugReadRecordsEnd(resultWriter* rw);
} // namespace msqlp
} // namespace bzs

#ifdef DEBUG_LOG
#define DEBUG_WRITELOG(MSG) writeDebuglog(MSG);
#define DEBUG_WRITELOG2(OP, REQ) writeDebuglog(OP, REQ);
#define DEBUG_WRITELOG3(OP, REQ, ERR) writeDebuglog(OP, REQ, ERR);
#define DEBUG_WRITELOG_SP1(FORMAT, PARAM)                                      \
    sprintf_s(msg, FORMAT, PARAM);                                             \
    writeDebuglog(msg);

#else // DEBUG_LOG
#define DEBUG_WRITELOG(MSG)
#define DEBUG_WRITELOG2(OP, REQ)
#define DEBUG_WRITELOG3(OP, REQ, ERR)
#define DEBUG_WRITELOG_SP1(FORMAT, PARAM)
#endif // DEBUG_LOG

#ifdef DEBUG_LOG_BINSERT
#define DEBUG_INSERT(TB, PTR, LEN, ROW, ERRORS)                                \
    debugInsert(TB, PTR, LEN, ROW, ERRORS);
#else
#define DEBUG_INSERT(TB, PTR, LEN, ROW, ERRORS)
#endif

#ifdef DEBUG_RECORDS
#define DEBUG_RECORDS_BEGIN(RDEF, REQ) debugReadRecordsBegin(RDEF, REQ);
#define DEBUG_RECORDS_END(WRITER) debugReadRecordsEnd(WRITER);
#else
#define DEBUG_RECORDS_BEGIN(RDEF, REQ)
#define DEBUG_RECORDS_END(WRITER)
#endif

#if defined(DEBUG_PROFILE) || defined(DEBUG_LOG)
#define DEBUG_PROFILE_INIT() bzs::msqlp::initLog();
#define DEBUG_PROFILE_DEINIT() bzs::msqlp::endLog();
#else
#define DEBUG_PROFILE_INIT()
#define DEBUG_PROFILE_DEINIT()
#endif

#ifdef DEBUG_PROFILE
extern unsigned int g_v;

#define DEBUG_PROFILE_START(V)                                                 \
    if (V)                                                                     \
        g_v = GetTickCount();
#define DEBUG_PROFILE_END(V, NAME)                                             \
    if (V)                                                                     \
    {                                                                          \
        char buf[256];                                                         \
        sprintf(buf, "%-20s\t %ld\r\n", NAME, GetTickCount() - g_v);           \
        bzs::msqlp::writeDebuglog(buf);                                        \
    };

#define DEBUG_PROFILE_END_OP(V, IV)                                            \
    if (V)                                                                     \
    {                                                                          \
        char buf[256];                                                         \
        sprintf(buf, "op = %-15ld\t %ld\r\n", IV, GetTickCount() - g_v);       \
        bzs::msqlp::writeDebuglog(buf);                                        \
    };

#else // DEBUG_PROFILE

#define DEBUG_PROFILE_START(V)
#define DEBUG_PROFILE_END(V, NAME)
#define DEBUG_PROFILE_END_OP(V, IV)

#endif // DEBUG_PROFILE

#endif // BZS_DB_PROTOCOL_TDAP_MYSQL_DEBUGLOG_H
