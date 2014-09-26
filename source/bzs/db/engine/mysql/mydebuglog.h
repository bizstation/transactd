#ifndef BZS_DB_ENGINE_MYSQL_MYDEBUGLOG_H
#define BZS_DB_ENGINE_MYSQL_MYDEBUGLOG_H
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
#include <bzs/db/protocol/tdap/mysql/request.h>
#include <boost/noncopyable.hpp>
#include <stdio.h>
#include <bzs/rtl/debuglog.h>

using namespace bzs::rtl;

namespace bzs
{

namespace db
{
using namespace protocol::tdap::mysql;

namespace engine
{
namespace mysql
{

class table;

class debugdb : public rtl::debuglog
{
    void fieldDump(table* tb);

public:
    virtual ~debugdb(){};

    virtual void stop();
    void write(ushort_td op, protocol::tdap::mysql::request& req,
               bool error = false);
    void writeInsert(table* tb, const char* packPtr, int packlen, int row,
                     int errorCount);
};

extern char msg[1024];

#ifdef DEBUG_LOG

#define DEBUG_WRITELOG2(OP, REQ) ((debugdb*)debuglog::get())->write(OP, REQ);
#define DEBUG_WRITELOG3(OP, REQ, ERR)                                          \
    ((debugdb*)debuglog::get())->write(OP, REQ, ERR);
#define DEBUG_WRITELOG_SP1(FORMAT, PARAM)                                      \
    sprintf_s(msg, FORMAT, PARAM);                                             \
    debuglog::get()->write(msg);
#define DEBUG_MEMDUMP(MSG, PTR, SIZE)                                          \
    ((debugdb*)debuglog::get())->writeDump(MSG, (const char*)PTR, SIZE);

#else // DEBUG_LOG
#define DEBUG_WRITELOG2(OP, REQ)
#define DEBUG_WRITELOG3(OP, REQ, ERR)
#define DEBUG_WRITELOG_SP1(FORMAT, PARAM)
#define DEBUG_MEMDUMP(MSG, PTR, SIZE)
#endif // DEBUG_LOG

#ifdef DEBUG_LOG_BINSERT
#define DEBUG_INSERT(TB, PTR, LEN, ROW, ERRORS)                                \
    ((debugdb*)debuglog::get())->writeInsert(TB, PTR, LEN, ROW, ERRORS);
#else
#define DEBUG_INSERT(TB, PTR, LEN, ROW, ERRORS)
#endif

#ifdef DEBUG_READRECORDS
#define DEBUG_RECORDS_BEGIN(RDEF, REQ)                                         \
    debuglog::get()->writeReadRecordsBegin(RDEF, REQ);
#define DEBUG_RECORDS_END(WRITER) debuglog::get()->writeReadRecordsEnd(WRITER);
#else
#define DEBUG_RECORDS_BEGIN(RDEF, REQ)
#define DEBUG_RECORDS_END(WRITER)
#endif

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_MYDEBUGLOG_H
