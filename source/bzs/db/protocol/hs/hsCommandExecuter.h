#ifndef BZS_DB_PROTOCOL_HS_HSCOMMANDEXECUTER_H
#define BZS_DB_PROTOCOL_HS_HSCOMMANDEXECUTER_H
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
#include <bzs/db/engine/mysql/dbManager.h>
#include <bzs/db/protocol/ICommandExecuter.h>
#include <bzs/env/crosscompile.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace hs
{

#define HS_OP_READ 'R'
#define HS_OP_OPEN 'P'
#define HS_OP_AUTH 'A'
#define HS_OP_INSERT '+'
#define HS_OP_DELETE 'D'
#define HS_OP_UPDATE 'U'
#define HS_OP_UPDATE_INC '+' + 0xff
#define HS_OP_UPDATE_DEC '-'
#define HS_OP_QUIT 'Q'

#define HS_LG_EQUAL '='
#define HS_LG_GREATER '>'
#define HS_LG_LESS '<'
#define HS_LG_NOTEQUAL '<' + 0xfe //<>
#define HS_LG_GREATEROREQUAL '>' + 0xff //>=
#define HS_LG_LESSOREQUAL '<' + 0xff //<=

#define DEBNAME_SIZE 64
#define TABELNAME_SIZE 64
#define INDEXNAME_SIZE 64

#define HS_OP_RESULTBUFSIZE 64000

//-----------------------------------------------------------------------

//    result buffer
//-----------------------------------------------------------------------

class resultBuffer
{
    char* m_ptr;
    char* m_cur;

public:
    resultBuffer(char* ptr) : m_ptr(ptr), m_cur(m_ptr) {}

    void append(const char* ptr, size_t size)
    {
        const char* p = ptr;
        const char* end = ptr + size;
        for (; p < end; ++p)
        {
            if ((*p >= 0x00) && (*p <= 0x10))
            {
                *m_cur = 0x01;
                *(++m_cur) = *p + 0x40;
            }
            else
                *m_cur = *p;
            ++m_cur;
        }
    }

    void append(const char* ptr)
    {
        const char* p = ptr;
        while (*p)
        {
            *m_cur = *p;
            ++p;
            ++m_cur;
        }
    }

    void append(int v)
    {
        char tmp[50];
        sprintf_s(tmp, 50, "%d", v);
        append(tmp);
    }

    size_t size()
    {
        *m_ptr = 0x00;
        return m_cur - m_ptr;
    }
    const char* c_str() { return m_ptr; }
};
//-----------------------------------------------------------------------
//    request
//-----------------------------------------------------------------------

struct request
{
    short op;
    int handle;

    struct result
    {
        result() : limit(1), offset(0), returnBeforeValue(0){};
        int limit;
        int offset;
        bool returnBeforeValue;
    } result;

    struct db
    {
        char name[DEBNAME_SIZE];
    } db;

    struct table
    {
        table() : openMode(0) {}
        char name[TABELNAME_SIZE];
        short openMode;

        struct key
        {
            key() : logical(0){};
            char name[INDEXNAME_SIZE];
            std::vector<std::string> values;
            short logical;
        } key;

        std::string fields;
        std::vector<std::string> values;
        struct in
        {
            in() : keypart(0) {}
            uint keypart;
            std::vector<std::string> values;
        } in;

        struct filter
        {
            filter() : type(0), logical(0), col(0) {}
            char type;
            short logical;
            short col;
            std::string value;
        };

        typedef std::vector<filter> filters_type;
        filters_type filters;

    } table;

    request() : op(HS_OP_READ), handle(0) {}
    ha_rkey_function seekFlag();
    bool naviForward();
    bool naviSame();
};

//-----------------------------------------------------------------------
//    class dbExecuter
//-----------------------------------------------------------------------
/** Current, no support auth command .
 */
typedef int (*changeFunc)(request& req, engine::mysql::table* tb, int type);
class dbExecuter : public engine::mysql::dbManager
{
    void doRecordOperation(request& req, engine::mysql::table* tb,
                           resultBuffer& buf, changeFunc func);
    inline int readAfter(request& req, engine::mysql::table* tb,
                         resultBuffer& buf, changeFunc func);

public:
    dbExecuter(netsvc::server::IAppModule* mod);
    int commandExec(std::vector<request>& requests,
                    netsvc::server::netWriter* nw);
    int errorCode(int ha_error) { return 0; };
};

//-----------------------------------------------------------------------
//    class commandExecuter
//-----------------------------------------------------------------------
class commandExecuter : public ICommandExecuter,
                        public engine::mysql::igetDatabases
{
    boost::shared_ptr<dbExecuter> m_dbExec;
    mutable std::vector<request> m_requests;

public:
    commandExecuter(netsvc::server::IAppModule* mod);
    ~commandExecuter();
    size_t perseRequestEnd(const char* p, size_t size, bool& comp) const;
    size_t getAcceptMessage(char* message, size_t size) { return 0; };
    bool parse(const char* p, size_t size);
    int execute(netsvc::server::netWriter* nw)
    {
        return m_dbExec->commandExec(m_requests, nw);
    }
    void cleanup(){};

    bool isShutDown() { return m_dbExec->isShutDown(); }
    
    const engine::mysql::databases& dbs() const { return m_dbExec->dbs(); }
    
    boost::mutex& mutex() { return m_dbExec->mutex(); }
};

} // namespace hs
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_HS_HSCOMMANDEXECUTER_H
