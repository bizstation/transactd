/*=================================================================
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
=================================================================*/
#include "hsCommandExecuter.h"
#include <bzs/db/engine/mysql/database.h>
#include <bzs/db/engine/mysql/errorMessage.h>
#include <bzs/db/engine/mysql/mydebuglog.h>
#include <bzs/netsvc/server/IAppModule.h> //lookup for result value
#include <bzs/rtl/stl_uty.h>
#include <limits.h>
#include <bzs/db/transactd/connManager.h>
#include <bzs/rtl/exception.h>

#ifdef USE_HANDLERSOCKET
namespace bzs
{
namespace db
{
using namespace engine::mysql;
namespace protocol
{
using namespace transactd;
using namespace transactd::connection;
namespace hs
{

//-----------------------------------------------------------------------
//    request
//-----------------------------------------------------------------------
ha_rkey_function request::seekFlag()
{
    switch (table.key.logical)
    {
    case HS_LG_EQUAL:
        return HA_READ_KEY_EXACT;
    case HS_LG_GREATER:
        return HA_READ_AFTER_KEY;
    case HS_LG_GREATEROREQUAL:
        return HA_READ_KEY_OR_NEXT;
    case HS_LG_LESS:
        return HA_READ_BEFORE_KEY;
    case HS_LG_LESSOREQUAL:
        return HA_READ_KEY_OR_PREV;
    }
    return HA_READ_KEY_EXACT;
}

bool request::naviSame()
{
    return (table.key.logical == HS_LG_EQUAL);
}

bool request::naviForward()
{
    return (table.key.logical == HS_LG_GREATER) ||
           (table.key.logical == HS_LG_GREATEROREQUAL);
}

void seriarizeRecordsetBegin(engine::mysql::table* tb, resultBuffer& buf,
                             bool sendBforeValue)
{
    if (sendBforeValue)
    {
        buf.append("0\t");
        buf.append((int)tb->useFields().size());
    }
}

void seriarizeRecord(engine::mysql::table* tb, resultBuffer& buf,
                     bool sendBforeValue)
{
    if (sendBforeValue)
    {
        for (int i = 0; i < (int)tb->useFields().size(); i++)
        {
            buf.append("\t");
            int size;
            const char* p = tb->valStr(tb->useFields()[i], size);
            if (size == -1)
                buf.append("\0");
            else
                buf.append(p, size);
        }
    }
}

void seriarizeRecordsetEnd(engine::mysql::table* tb, resultBuffer& buf,
                           int count, bool sendBforeValue)
{
    if (!sendBforeValue)
    {
        buf.append("0\t1\t");
        buf.append(count);
        buf.append("\n");
    }
    else
        buf.append("\n");
}

void writeError(int error, resultBuffer& buf, const char* msg)
{
    buf.append(error);
    buf.append("\t");
    buf.append(msg);
    buf.append("\n");
}

void writeStatus(int stat, resultBuffer& buf, int option)
{
    int v = 0;
    v = std::min(stat, 1);
    buf.append(v);
    buf.append("\t");
    buf.append(option);
    buf.append("\n\0");
}

//-----------------------------------------------------------------------
//    clas  dbExecuter
//-----------------------------------------------------------------------
#define READ_SUCCESS 0
#define FILTER_MATCH READ_SUCCESS
#define FILTER_SKIP 1
#define FILTER_BREAK 2

int update(request& req, engine::mysql::table* tb, int type)
{
    tb->beginUpdate(tb->keyNum());
    tb->setUseValues(req.table.values, type);
    tb->update(true);
    return tb->stat();
}

int del(request& req, engine::mysql::table* tb, int type)
{
    tb->beginDel();
    tb->del();
    return tb->stat();
}

inline bool match(short logical, int v)
{
    switch (logical)
    {
    case HS_LG_EQUAL:
        return (v == 0); //==
    case HS_LG_GREATER:
        return (v > 0); //>
    case HS_LG_LESS:
        return (v < 0); //<
    case HS_LG_NOTEQUAL:
        return (v != 0); //!=
    case HS_LG_GREATEROREQUAL:
        return (v >= 0); //>=
    case HS_LG_LESSOREQUAL:
        return (v <= 0); //<=
    }
    return false;
}

int filter(engine::mysql::table* tb, request::table::filters_type& filters)
{
    for (size_t i = 0; i < filters.size(); ++i)
    {
        request::table::filter& f = filters[i];
        if (f.col >= (int)tb->useFields().size())
            THROW_BZS_ERROR_WITH_MSG("fcol");
        Field* fd = tb->field(tb->useFields()[f.col]);
        uchar tmp[1024] = { 0 };
        if (fd->pack_length() > 1024)
            THROW_BZS_ERROR_WITH_MSG("field_length > 1024");
        memcpy(tmp, fd->ptr, fd->pack_length());
        tb->setValue(fd->field_index, f.value, 0);
        int v = fd->cmp(tmp, fd->ptr);
        memcpy(fd->ptr, tmp, fd->pack_length());
        if (!match(f.logical, v))
            return (f.type == 'F') ? FILTER_SKIP : FILTER_BREAK;
    }
    return FILTER_MATCH;
}

inline int type(request& req)
{
    if (req.op == HS_OP_UPDATE_INC)
        return UPDATE_INC;
    if (req.op == HS_OP_UPDATE_DEC)
        return UPDATE_DEC;
    return UPDATE_REPLACE;
}

inline int dbExecuter::readAfter(request& req, engine::mysql::table* tb,
                                 resultBuffer& buf, changeFunc func)
{
    int ret = filter(tb, req.table.filters);
    if (ret == FILTER_MATCH)
    {
        if (req.result.offset == 0)
        {
            seriarizeRecord(tb, buf, req.result.returnBeforeValue);
            if (func)
                func(req, tb, type(req));
            return READ_SUCCESS;
        }
        --req.result.offset;
        return FILTER_SKIP;
    }
    return ret;
}

inline void setKeyValues(request& req, engine::mysql::table* tb, int index)
{
    if (req.table.in.keypart >= tb->keyDef(tb->keyNum()).user_defined_key_parts)
        THROW_BZS_ERROR_WITH_MSG("icol");
    tb->clearBuffer();
    if (index != -1)
        tb->setKeyValues(req.table.key.values, req.table.in.keypart,
                         &(req.table.in.values[index]));
    else
        tb->setKeyValues(req.table.key.values, -1);
}

void dbExecuter::doRecordOperation(request& req, engine::mysql::table* tb,
                                   resultBuffer& buf, changeFunc func)
{
    int count = 0;
    int curInValueIndex = req.table.in.values.size() ? 0 : -1;
    int offset = req.result.offset;
    int filterResult = 0;
    seriarizeRecordsetBegin(tb, buf, req.result.returnBeforeValue);

    setKeyValues(req, tb, curInValueIndex);
    if (curInValueIndex >= 0)
        ++curInValueIndex;
    tb->seekKey(req.seekFlag(), tb->keymap());
    if (tb->stat() == 0)
    {
        filterResult = readAfter(req, tb, buf, func);
        if (filterResult == READ_SUCCESS)
            ++count;
    }
    if (((tb->stat() == 0) && (filterResult != FILTER_BREAK)) ||
        (curInValueIndex > -1))
    {
        for (int i = 1; i < req.result.limit + offset; i++)
        {
            if (curInValueIndex > -1)
            {
                if (curInValueIndex >= (int)req.table.in.values.size())
                    break;
                setKeyValues(req, tb, curInValueIndex++);
                tb->seekKey(req.seekFlag(), tb->keymap());
            }
            else
            {
                if (req.naviSame())
                    tb->getNextSame(tb->keymap());
                else if (req.naviForward())
                    tb->getNext();
                else
                    tb->getPrev();
            }

            if (tb->stat())
                break;
            filterResult = readAfter(req, tb, buf, func);
            if (filterResult == READ_SUCCESS)
                ++count;
            if (tb->stat() || (filterResult == FILTER_BREAK))
                break;
        }
    }
    seriarizeRecordsetEnd(m_tb, buf, count, req.result.returnBeforeValue);
}

int dbExecuter::commandExec(std::vector<request>& requests,
                            netsvc::server::netWriter* nw)
{
    request& req = requests[0];
    resultBuffer buf(nw->ptr());
    size_t& size = nw->datalen;
    try
    {
        switch (req.op)
        {
        case HS_OP_QUIT:
            return EXECUTE_RESULT_QUIT;
        case HS_OP_AUTH:
            writeStatus(0, buf, 1); // allways success.
            break;
        case HS_OP_OPEN:
        {
            checkNewHandle(req.handle);
            database* db = getDatabase(req.db.name, 0 /*cid*/);
            m_tb = db->openTable(req.table.name, req.table.openMode, NULL);
            addHandle(getDatabaseID(0 /*cid*/), m_tb->id(), req.handle);
            m_tb = getTable(req.handle);
            m_tb->setUseFieldList(req.table.fields);
            m_tb->setKeyNum(req.table.key.name);
            writeStatus(m_tb->stat(), buf, 1);
            break;
        }
        case HS_OP_INSERT:
        {
            m_tb = getTable(req.handle, SQLCOM_INSERT);
            m_tb->clearBuffer();
            m_tb->setUseValues(req.table.values, 0);
            m_tb->insert(true);
            writeStatus(m_tb->stat(), buf, 1);
            break;
        }
        case HS_OP_UPDATE_INC:
        case HS_OP_UPDATE_DEC:
        case HS_OP_UPDATE:
        {
            m_tb = getTable(req.handle, SQLCOM_UPDATE);
            doRecordOperation(req, m_tb, buf, update);
            break;
        }
        case HS_OP_DELETE:
        {
            m_tb = getTable(req.handle, SQLCOM_UPDATE);
            doRecordOperation(req, m_tb, buf, del);
            break;
        }
        case HS_OP_READ:
        {
            m_tb = getTable(req.handle);
            doRecordOperation(req, m_tb, buf, NULL);
            break;
        }
        }
        if (m_tb)
            m_tb->unUse();
        size = buf.size();
        return EXECUTE_RESULT_SUCCESS;
    }

    catch (bzs::rtl::exception& e)
    {
        clenupNoException();
        const std::string* msg = getMsg(e);
        const int* code = getCode(e);
        if (code && msg)
            writeError(*code, buf, msg->c_str());
        else
        {
            if (msg)
                writeError(1, buf, msg->c_str());
            else
                writeError(1, buf, "");
            sql_print_error("%s", boost::diagnostic_information(e).c_str());
        }
        printErrorMessage(code, msg);
    }

    catch (...)
    {
        clenupNoException();
        writeError(1, buf, "Unknown error.");
    }
    size = buf.size();
    return EXECUTE_RESULT_SUCCESS;
}

// ---------------------------------------------------------------------------
//    class commandExecuter
// ---------------------------------------------------------------------------
#define PARSEREAD_START 0
#define PARSEREAD_COMMAND 1
#define PARSEREAD_HANDLEID 2
#define PARSEREAD_SEEKCMD 3
#define PARSEREAD_DBNAME 4
#define PARSEREAD_TABLENAME 5
#define PARSEREAD_INDEXNAME 6
#define PARSEREAD_FIELDS 7
#define PARSEREAD_KEYVAL_COUNT 8
#define PARSEREAD_KEYVAL 9
#define PARSEREAD_LIMIT 10
#define PARSEREAD_OFFSET 11
#define PARSEREAD_DATA_COUNT 12
#define PARSEREAD_DATA_VAL 13
#define PARSEREAD_IN_MODE 15
#define PARSEREAD_IN_KEYPART 16
#define PARSEREAD_IN_COUNT 17
#define PARSEREAD_IN_VAL 18
#define PARSEREAD_FL_TYPE 19
#define PARSEREAD_FL_LOG 20
#define PARSEREAD_FL_COL 21
#define PARSEREAD_FL_VAL 22

inline void setHandleID(const std::string& src, int& parseMode, request* req)

{
    req->handle = (short)atol(src.c_str());
    if (req->op == HS_OP_OPEN)
        parseMode = PARSEREAD_DBNAME;
    else
        parseMode = PARSEREAD_SEEKCMD;
}

inline void setCmd(const std::string& src, int& parseMode, request* req)
{
    if (src[0] >= '0' && src[0] <= '9')
        return setHandleID(src, parseMode, req);
    else
        req->op = src[0];
    parseMode = PARSEREAD_HANDLEID;
}

inline void setChangeCmd(const std::string& src, int& parseMode, request* req)
{
    req->op = src[0];
    if (req->table.key.logical && (req->op == HS_OP_INSERT))
        req->op = HS_OP_UPDATE_INC;
    req->result.returnBeforeValue = (src[1] == '?');
    if (req->op == HS_OP_INSERT)
        parseMode = PARSEREAD_DATA_COUNT;
    else
        parseMode = PARSEREAD_DATA_VAL;
}

inline void setSeekCmd(const std::string& src, int& parseMode, request* req)
{
    if ((src[0] == HS_OP_INSERT) || (src[0] == HS_OP_UPDATE) ||
        (src[0] == HS_OP_DELETE))
        return setChangeCmd(src, parseMode, req);
    req->table.key.logical = src[0];
    if ((src.size() > 1) && (src[1] == '='))
        req->table.key.logical += 0xff;
    req->result.returnBeforeValue = true;
    parseMode = PARSEREAD_KEYVAL_COUNT;
}

inline void setDbname(const std::string& src, int& parseMode, request* req)
{
    strncpy(req->db.name, src.c_str(), DEBNAME_SIZE);

    parseMode = PARSEREAD_TABLENAME;
}

inline void setTablename(const std::string& src, int& parseMode, request* req)
{
    strncpy(req->table.name, src.c_str(), TABELNAME_SIZE);
    req->table.openMode = TD_OPEN_NORMAL;
    parseMode = PARSEREAD_INDEXNAME;
}

inline void setIndexname(const std::string& src, int& parseMode, request* req)
{
    strncpy(req->table.key.name, src.c_str(), INDEXNAME_SIZE);
    parseMode = PARSEREAD_FIELDS;
}

inline void setValueCount(const std::string& src, int& parseMode, int& count)
{
    count = atol(src.c_str());
}

inline void setValue(const std::string& src, int& parseMode,
                     std::vector<std::string>& values, int& count)
{
    if (count)
    {
        values.push_back(src);
        --count;
    }
}

inline bool setLimit(const std::string& src, int& parseMode, request* req)
{
    if ((src[0] >= '0') && (src[0] <= '9'))
    {
        req->result.limit = atol(src.c_str());
        parseMode = PARSEREAD_OFFSET;
        return false;
    }
    parseMode = PARSEREAD_IN_MODE;
    return true;
}

inline void setOffset(const std::string& src, int& parseMode, request* req)
{
    req->result.offset = atol(src.c_str());
    parseMode = PARSEREAD_IN_MODE;
}

inline void setFields(const std::string& src, int& parseMode, request* req)
{
    req->table.fields = src;
    parseMode = PARSEREAD_START;
}

inline bool setInMode(const std::string& src, int& parseMode, request* req)
{
    if (src[0] == '@')
    {
        parseMode = PARSEREAD_IN_KEYPART;
        return false;
    }
    parseMode = PARSEREAD_FL_TYPE;
    return true;
}

inline void setInKeyPart(const std::string& src, int& parseMode, request* req)
{
    req->table.in.keypart = (short)atol(src.c_str());
    parseMode = PARSEREAD_IN_COUNT;
}

inline bool setFilterType(const std::string& src, int& parseMode, request* req)
{
    if ((src[0] == 'F') || (src[0] == 'W'))
    {
        request::table::filter f;
        f.type = src[0];
        req->table.filters.push_back(f);

        parseMode = PARSEREAD_FL_LOG;
        return false;
    }
    parseMode = PARSEREAD_SEEKCMD;
    return true;
}

inline void setFilterLog(const std::string& src, int& parseMode, request* req)
{
    request::table::filter& f =
        req->table.filters[req->table.filters.size() - 1];
    f.logical = src[0];
    if ((src.size() > 1) && (src[1] == '='))
        f.logical += 0xff;
    else if ((src.size() > 1) && (src[1] == '>'))
        f.logical += 0xfe;
    parseMode = PARSEREAD_FL_COL;
}

inline void setFilterCol(const std::string& src, int& parseMode, request* req)
{
    request::table::filter& f =
        req->table.filters[req->table.filters.size() - 1];
    f.col = (short)atol(src.c_str());
    parseMode = PARSEREAD_FL_VAL;
}

inline void setFilterVal(const std::string& src, int& parseMode, request* req)
{
    request::table::filter& f =
        req->table.filters[req->table.filters.size() - 1];
    f.value = src;
    parseMode = PARSEREAD_FL_TYPE;
}

commandExecuter::commandExecuter(__int64 /*parent*/)
    : m_dbExec(new dbExecuter())
{
}

commandExecuter::~commandExecuter()
{
    m_dbExec.reset();
}

size_t commandExecuter::perseRequestEnd(const char* p, size_t transfered,
                                        bool& comp) const
{
    if (transfered)
    {
        const char* tmp = p + transfered - 1;
        comp = (*tmp == '\n');
    }
    if (!comp && transfered < strlen(p) + 10)
        return transfered + 64000;
    return 0;
}

char* readTorkn(std::string& buf, char* src, const char* end)
{
    buf.erase();
    while (!((*src == '\t') || (*src == '\n') || (*src == '\r')))
    {
        if (src == end)
            break;
        if (*src == 0x01)
            *(++src) -= 0x040;
        buf.append(1, *src);
        ++src;
    }
    return ++src;
}

bool commandExecuter::parse(const char* ptr, size_t size)
{
    m_requests.clear();
    char* p = (char*)ptr;
    const char* end = p + size;
    int parseMode = PARSEREAD_START;
    int keyval_count = 0;
    int data_count = INT_MAX;
    int in_count = 0;
    bool skip = false;
    std::string buf;
    buf.reserve(1024);
    request* req = NULL;
    while (p != end)
    {
        if (skip == false)
            p = readTorkn(buf, p, end);
        else
            skip = false;
        if (buf.size() == 0)
            break;
        switch (parseMode)
        {
        case PARSEREAD_START:
            m_requests.push_back(request());
            req = &m_requests[m_requests.size() - 1];
            parseMode = PARSEREAD_COMMAND;
            skip = true;
            break;
        case PARSEREAD_COMMAND:
            setCmd(buf, parseMode, req);
            break;
        case PARSEREAD_HANDLEID:
            setHandleID(buf, parseMode, req);
            break;
        case PARSEREAD_SEEKCMD:
            setSeekCmd(buf, parseMode, req);
            break;
        case PARSEREAD_DATA_COUNT:
            setValueCount(buf, parseMode, data_count);
            parseMode = PARSEREAD_DATA_VAL;
            break;
        case PARSEREAD_DATA_VAL:
            setValue(buf, parseMode, req->table.values, data_count);
            if (data_count == 0)
                parseMode = PARSEREAD_START;
            break;
        case PARSEREAD_KEYVAL_COUNT:
            setValueCount(buf, parseMode, keyval_count);
            parseMode = PARSEREAD_KEYVAL;
            break;
        case PARSEREAD_KEYVAL:
            setValue(buf, parseMode, req->table.key.values, keyval_count);
            if (keyval_count == 0)
                parseMode = PARSEREAD_LIMIT;
            break;
        case PARSEREAD_DBNAME:
            setDbname(buf, parseMode, req);
            break;
        case PARSEREAD_TABLENAME:
            setTablename(buf, parseMode, req);
            break;
        case PARSEREAD_INDEXNAME:
            setIndexname(buf, parseMode, req);
            break;
        case PARSEREAD_FIELDS:
            setFields(buf, parseMode, req);
            break;
        case PARSEREAD_LIMIT:
            skip = setLimit(buf, parseMode, req);
            break;
        case PARSEREAD_OFFSET:
            setOffset(buf, parseMode, req);
            break;
        case PARSEREAD_IN_MODE:
            skip = setInMode(buf, parseMode, req);
            break;
        case PARSEREAD_IN_KEYPART:
            setInKeyPart(buf, parseMode, req);
            break;
        case PARSEREAD_IN_COUNT:
            setValueCount(buf, parseMode, in_count);
            parseMode = PARSEREAD_IN_VAL;
            break;
        case PARSEREAD_IN_VAL:
            setValue(buf, parseMode, req->table.in.values, in_count);
            if (in_count == 0)
                parseMode = PARSEREAD_FL_TYPE;
            break;
        case PARSEREAD_FL_TYPE:
            skip = setFilterType(buf, parseMode, req);
            break;
        case PARSEREAD_FL_LOG:
            setFilterLog(buf, parseMode, req);
            break;
        case PARSEREAD_FL_COL:
            setFilterCol(buf, parseMode, req);
            parseMode = PARSEREAD_FL_VAL;
            break;
        case PARSEREAD_FL_VAL:
            setFilterVal(buf, parseMode, req);
            break;
        }
    }
    return !(m_requests.size() == 0);
}

} // namespace hs
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // USE_HANDLERSOCKET
