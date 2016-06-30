/*=================================================================
   Copyright (C) 2012-2016 BizStation Corp All rights reserved.

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
#include <bzs/db/engine/mysql/mysqlProtocol.h>
#include <bzs/db/engine/mysql/errorMessage.h>
#include <bzs/db/engine/mysql/mydebuglog.h>
#include <bzs/db/engine/mysql/ha.h>
#include <bzs/netsvc/server/IAppModule.h> //lookup for result value
#include <bzs/db/transactd/appModule.h>
#include <bzs/rtl/stl_uty.h>
#include <limits.h>
#include <boost/algorithm/string.hpp>
#ifdef WIN32
#include <mbstring.h>
#endif
#include "databaseSchema.h"
#include <bzs/db/transactd/connManager.h>
#include <bzs/rtl/exception.h>
#include <random>

extern int getTransactdIsolation();
extern unsigned int getTransactdLockWaitTimeout();

/* implemnts in transactd.cpp */
extern unsigned int g_tcpServerType;

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
boost::mutex g_mutex_opentable;

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
    const char* p = (const char*)req.keybuf;
    if (p && p[0])
    {
        std::string s(p);
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
        if (name.size() > pos)
        {
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

int errorCode(int ha_error)
{ // see mysqld_error.h or my_base.h or dbManager.h share/errmsg.txt

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
        return STATUS_SQL_PARSE_ERROR;
    else if (ha_error == ER_TABLE_EXISTS_ERROR)
        return STATUS_TABLE_EXISTS_ERROR;
    else if (ha_error == DBM_ERROR_TABLE_USED)
        return STATUS_CANNOT_LOCK_TABLE;
    else if(ha_error == ER_BAD_DB_ERROR)
        return ERROR_NO_DATABASE;
    else if(ha_error == ER_NO_SUCH_TABLE)
        return STATUS_TABLE_NOTOPEN;
    else if(ha_error == ER_SPECIFIC_ACCESS_DENIED_ERROR)
        return STATUS_ACCESS_DENIED; 
    else if(ha_error == ER_TABLEACCESS_DENIED_ERROR)
        return STATUS_ACCESS_DENIED; 
    return MYSQL_ERROR_OFFSET + ha_error;
}

//-------------------------------------------------------------
//  class dbExecuter
//-------------------------------------------------------------

dbExecuter::dbExecuter(netsvc::server::IAppModule* mod)
    : dbManager(mod), m_readHandler(new ReadRecordsHandler()),
      m_blobBuffer(new blobBuffer())
{
    m_scramble[0] = 0x00;
}

dbExecuter::~dbExecuter()
{
    delete m_blobBuffer;
    delete m_readHandler;
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
}

bool isMetaDb(const request& req)
{
    char buf[MAX_PATH];
    const char* p = (const char*)req.keybuf;
    char_m* st = NULL;
    if (p && p[0])
    {
        strncpy(buf, p, MAX_PATH);
        _strlwr(buf);
        st = _mbsstr((char_m*)buf, (char_m*)BdfNameTitle);
        if (st == NULL)
            st = (char_m*)strstr(buf, TRANSACTD_SCHEMANAME);
    }
    return (st != NULL);
}

inline bool getUserPasswd(request& req, char* &user, char* &pwd)
{
    char* p = strchr((char*)req.keybuf, '\t');
    if (p)
    {
        p = strchr(++p, '\t');
        if (p)
        {
            pwd = ++p;
            user = pwd + MYSQL_SCRAMBLE_LENGTH;
            while (p < user)
                if (*p++) return true;
        }
        return false;
    }
    return false;
}

inline bool dbExecuter::doAuthentication(request& req, database* db)
{
    if (m_authChecked) return true;
    bool ret = true;
    if (strcmp(g_auth_type, AUTH_TYPE_MYSQL_STR)==0)
    {
        ret = false;
        char host[MAX_HOSTNAME] = {0x00};
        char *user = 0x00;
        char *pwd = 0x00;
        bool pwdRecieved = getUserPasswd(req, user, pwd);
        if (user && user[0] && m_mod->checkHost(user, host, MAX_HOSTNAME))
        {
            //uint8* p = find_acl_user_(host, user, FALSE);
            unsigned char buf[22];
            unsigned char* p = db->getUserSha1Passwd(host, user, buf);
            if (!pwdRecieved && !p) 
                ret = true;
            else if (p && pwdRecieved)
                ret = (FALSE == check_scramble((const unsigned char*)pwd, (const char*)m_scramble, p));
            if (ret)
            {
                m_mod->setUser(user);
                m_mod->setHost(host);
                if (host[0])
                    db->setGrant(host, user, db->name().c_str());
            }
        }
    }else
        m_mod->skipGrants(true);
    m_authChecked = ret;
    return ret;
}

/*
@param connect Is connect command, The database name is not required
@return 0 auth failed
        1 success but see req.result
*/
bool dbExecuter::getDatabaseWithAuth(request& req, database** db, bool connect)
{
    *db = NULL;
    bool created = false;
    std::string dbname = getDatabaseName(req);
    bool ret = false;
    if (connect && dbname == "")
        dbname = "mysql";
    else
        connect = false;
    if (dbname != "")
    {
        *db = getDatabase(dbname.c_str(), req.cid, created);
        if (*db)
            ret = doAuthentication(req, *db);
        if (connect || (created && !ret))
        {
            dbManager::releaseDatabase(req.cid);
            *db = NULL;
        }
    }else
        req.result = 1;

    return ret;
}

/* When connect failed, Delete database object.

*/
bool dbExecuter::connect(request& req)
{
    req.paramMask = 0;
    if (req.keyNum == LG_SUBOP_DISCONNECT)
    {
        dbManager::releaseDatabase(req.cid);
        return true;
    }
    database* db = NULL;
    bool ret = getDatabaseWithAuth(req, &db, true);
    if (ret &&  (req.result == 0) && db)
    {
        if (!db->existsDatabase())
            req.result = ERROR_NO_DATABASE;
    }
    if (db && (!ret || req.result))
        dbManager::releaseDatabase(req.cid);
    return ret;
}

inline void dbExecuter::doCreateTable(request& req)
{
    database* db;
    if (!getDatabaseWithAuth(req, &db) || req.result)
    {
        req.result = ERROR_TD_INVALID_CLINETHOST;
        return;
    }

    // if table name is mata table and database is nothing
    //  then cretate database too.
    std::string dbSqlname = getDatabaseName(req, FOR_SQL);
    std::string cmd;
    if (isMetaDb(req))
    { // for database operation
        if (((req.keyNum == CR_SUBOP_CREATE_DBONLY) || (req.keyNum == 0)) &&
                (db->existsDatabase() == false))
        {
            req.result = ddl_createDataBase(dbSqlname);
            if (req.result == ER_DB_CREATE_EXISTS + MYSQL_ERROR_OFFSET)
                req.result = 0;
        }
        else if (req.keyNum == CR_SUBOP_DROP)
        {
            if (req.result == 0)
            {
                std::string dbname = db->name();
                dbManager::releaseDatabase(req.cid);
                req.result = ddl_dropDataBase(dbname, dbSqlname, req.cid);
                if (ER_DB_DROP_EXISTS+ MYSQL_ERROR_OFFSET == req.result) req.result = 0;
            }
            return;
        }
    }
    if (req.result == 0)
    { // table operation
        std::string tableSqlName = getTableName(req, FOR_SQL);
        std::string tableName = getTableName(req);
        if (req.keyNum == CR_SUBOP_DROP) // -128 is delete
        {
            req.result = ddl_dropTable(db, tableName, dbSqlname,
                                        tableSqlName);
            if (req.result == ER_BAD_TABLE_ERROR + MYSQL_ERROR_OFFSET)
                req.result = 0;
        }
        else if (req.keyNum == CR_SUBOP_RENAME)
        { // rename new is keybuf
            request reqold;
            reqold.keybuf = req.data;
            reqold.keylen = *req.datalen;
            req.result = ddl_renameTable(
                db, getTableName(reqold), /*oldname*/
                dbSqlname, getTableName(reqold, FOR_SQL), /*oldname*/
                tableSqlName /*newName*/);
        }
        else if (req.keyNum == CR_SUBOP_SWAPNAME)
        { // swap name name2 = keybuf
            request reqold;
            reqold.keybuf = req.data;
            reqold.keylen = *req.datalen;
            req.result = ddl_replaceTable(
                db, getTableName(reqold), /*oldname*/
                tableName, /*newName*/
                dbSqlname, getTableName(reqold, FOR_SQL), /*oldname*/
                tableSqlName /*newName*/);
        }
        else if (req.keyNum != CR_SUBOP_CREATE_DBONLY)
        { // create
            if (req.data == NULL)
                req.result = 1;
            else
            { //-1 is overwrite
                if (req.keyNum == CR_SUB_FLAG_EXISTCHECK && tableName.size())
                {
                    req.result = ddl_dropTable(db, tableName, dbSqlname,
                                                tableSqlName);
                    if (req.result == ER_BAD_TABLE_ERROR + MYSQL_ERROR_OFFSET)
                        req.result = 0;
                }
                if (req.result == 0)
                    req.result = ddl_createTable(db, makeSQLcreateTable(req).c_str());
            }
        }
    }
}

// open table and assign handle
inline bool dbExecuter::doOpenTable(request& req, char* buf, bool reconnect)
{
    database* db;
    bool ret = getDatabaseWithAuth(req, &db);
    if (ret && req.result == 0)
    {
        short mode = req.keyNum;
        bool getschema = IS_MODE_GETSCHEMA(mode);
        if (getschema)
            mode -= TD_OPEN_MASK_GETSHCHEMA; 
        bool getDefaultImage = IS_MODE_GETDEFAULTIMAGE(mode);
        if (getDefaultImage)
            mode -= TD_OPEN_MASK_GETDEFAULTIMAGE; 
        table* tb = NULL;
        {// Lock open table by another thread
            boost::mutex::scoped_lock lck(g_mutex_opentable);
            if (req.keyNum == TD_OPEN_EXCLUSIVE/* && isMetaDb(req)*/)
            {
                if (database::tableRef.count(db->name(), getTableName(req)))
                {
                    req.result = STATUS_CANNOT_LOCK_TABLE;
                    return ret;
                }
            }
             // if error occured that throw no exception
            tb = db->openTable(getTableName(req), mode, getOwnerName(req), getDatabaseName(req, false)); 
            if (db->stat())
                req.result = (short_td)errorCode(db->stat());
        }
        
        if (tb)
        {
            tb->setBlobBuffer(m_blobBuffer);
            try
            {
                uint hdl = addHandle(getDatabaseID(req.cid), tb->id());
                m_tb = getTable(hdl);
                req.pbk->handle = hdl;
                if (!reconnect)
                {   //return bookmark size
                    req.paramMask = P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN;
                    unsigned int len = m_tb->posPtrLen();
                    unsigned char* p = (unsigned char*)req.data;
                    //no primary key
                    if ((m_tb->tableFlags() & HA_PRIMARY_KEY_REQUIRED_FOR_POSITION) && !m_tb->primaryKey())
                        len = 0xFFFF;
                    memcpy(p, &len , sizeof(ushort_td));
                    req.resultLen = sizeof(ushort_td);
                    //copy default record image
                    if (getDefaultImage)
                    {
                        if (!m_tb->isMysqlNull())
                        {
                            req.result = STATUS_INVALID_NULLMODE;
                            return false;
                        }
                        p += sizeof(ushort_td);
                        len = m_tb->writeDefaultImage(p, *req.datalen - req.resultLen);
                        req.resultLen += len;
                        if (!getschema && *req.datalen != req.resultLen)
                        {
                            req.result = STATUS_INVALID_DATASIZE;
                            return false;
                        }
                    }else if(getschema)
                    {
                        if (*req.datalen < 4)
                        {
                            req.result = STATUS_BUFFERTOOSMALL;
                            return false;
                        }
                        p += sizeof(ushort_td);
                        p[0] = 0x00;
                        p[1] = 0x00;
                        len = 2;
                        req.resultLen += len;
                    }
                    if (getschema)
                    {
                        p += len;
                        len = m_tb->writeSchemaImage(p , *req.datalen - req.resultLen);
                        if (len == sizeof(ushort_td))
                        {
                            req.result = STATUS_BUFFERTOOSMALL;
                            return false;
                        }
                        req.resultLen += len;
                    }
                }
            }
            catch (bzs::rtl::exception& e)
            {
                tb->close();
                throw e;
            }
        }
    }
    return ret;
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
    }else if (!tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
    req.result = dbm->errorCodeSht(tb->stat());
}

inline void dbExecuter::doSeekKey(request& req, int op, engine::mysql::rowLockMode* lck)
{
    bool read = true;
    m_tb = getTable(req.pbk->handle, SQLCOM_SELECT, lck);
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
            m_tb->setRowLock(lck);
            m_tb->seekKey(flag, m_tb->keymap());
            if (lck->lock && m_tb->stat())
                m_tb->setRowLockError();
        }
    }
    readAfter(req, m_tb, this);
}

inline void dbExecuter::doMoveFirst(request& req, engine::mysql::rowLockMode* lck)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_SELECT, lck);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        m_tb->setRowLock(lck);
        if (m_tb->isNisKey(m_tb->keyNum()))
        {
            m_tb->clearKeybuf();
            m_tb->seekKey(HA_READ_KEY_OR_NEXT, m_tb->keymap());
        }
        else
            m_tb->getFirst();
        if (lck->lock && m_tb->stat())
            m_tb->setRowLockError();
    }
    readAfter(req, m_tb, this);
}

inline void dbExecuter::doMoveKey(request& req, int op, engine::mysql::rowLockMode* lck)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_SELECT, lck);
    if (m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
    {
        m_tb->setRowLock(lck);
        if (op == TD_KEY_FIRST)
            m_tb->getFirst();
        else if (op == TD_KEY_LAST)
            m_tb->getLast();
        else if (op == TD_KEY_NEXT)
            m_tb->getNext();
        else if (op == TD_KEY_PREV)
            m_tb->getPrev();
        if (lck->lock && m_tb->stat())
            m_tb->setRowLockError();

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
    if (!m_tb->setKeyNum(keynum))
    {
        req.result = m_tb->stat();
        return ret;
    }

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
        if (!m_tb->cursor())
            req.paramMask |= P_MASK_PB_ERASE_BM;
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
    if (op == TD_KEY_SEEK_MULTI && !(ereq->itype & FILTER_TYPE_SEEKS_BOOKMARKS))
    {
        char keynum = m_tb->keyNumByMakeOrder(req.keyNum);
        if (!m_tb->setKeyNum(keynum))
        {
            req.result = m_tb->stat();
            return ret;
        }
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
        {
            if (ereq->itype & FILTER_TYPE_SEEKS_BOOKMARKS)
                req.result =
                    errorCodeSht(seekBookmarkEach((extRequestSeeks*)req.data, noBookmark));
            else
                req.result =
                    errorCodeSht(seekEach((extRequestSeeks*)req.data, noBookmark));
        }
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
        if (!m_tb->cursor())
            req.paramMask |= P_MASK_PB_ERASE_BM;
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

//FILTER_TYPE_SEEKS_BOOKMARKS
inline short dbExecuter::seekBookmarkEach(extRequestSeeks* ereq, bool noBookmark)
{
    short stat = 0;
    seek* fd = &ereq->seekData;

    for (int i = 0; i < ereq->logicalCount; ++i)
    {
        m_tb->movePos((uchar*)fd->ptr, -1);
        if (m_tb->stat() == 0)
        {
            if (noBookmark)
                stat = m_readHandler->write((const unsigned char *)_T("dummy"), 0);
            else
                stat =
                    m_readHandler->write(m_tb->position(), m_tb->posPtrLen());
        }
        else
            stat = m_readHandler->write(NULL, 0);
        if (stat)
            break;
        fd = fd->next();
    }
    if (stat == 0)
        stat = STATUS_REACHED_FILTER_COND;
    return stat;
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
        /* If NULL even any one segment, all be NULL 
           The fd->ptr[0] is null Indicator. 
        */
        if (fd->null == 0)
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
                    stat = m_readHandler->write(m_tb->position(), m_tb->posPtrLen());
            }
            else
                stat = m_readHandler->write(NULL, 0);

            if (stat) break;
            // for hasMany join
            if (seg)
            {
                while (m_tb->stat() == 0)
                {
                    m_tb->getNextSame(keyMap);
                    if (m_tb->stat() == 0)
                        stat = m_readHandler->write(
                            (uchar*)&i, 4); // write seek sequential number
                    if (stat) break;
                }
                if (stat) break;
            }
        }else
            stat = m_readHandler->write(NULL, 0);
        if (stat)  break;
        fd = fd->next();
    }
    if (stat == 0)
        stat = STATUS_REACHED_FILTER_COND;
    return stat;
}

inline void dbExecuter::doStepRead(request& req, int op, engine::mysql::rowLockMode* lck)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_SELECT, lck);
    m_tb->setRowLock(lck);
    if (op == TD_POS_FIRST)
        m_tb->stepFirst();
    else if (op == TD_POS_LAST)
        m_tb->stepLast();
    else if (op == TD_POS_NEXT)
        m_tb->stepNext();
    else if (op == TD_POS_PREV)
        m_tb->stepPrev();
    if (lck->lock && m_tb->stat())
        m_tb->setRowLockError();
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
    __int64 aincValue = m_tb->insert(ncc);
    req.result = errorCodeSht(m_tb->stat());
    if (m_tb->stat() == 0)
    {
        if (aincValue)
        {
            req.paramMask = P_MASK_INS_AUTOINC;
            req.data = m_tb->record();
        }
        else
            req.paramMask = P_MASK_POSBLK | P_MASK_KEYBUF;
    }
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
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
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
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
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
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
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
}

inline void dbExecuter::doDelete(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_DELETE);
    m_tb->beginDel();
    if (m_tb->stat() == 0)
        m_tb->del();
    req.result = errorCodeSht(m_tb->stat());
    req.paramMask = P_MASK_POSBLK;
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
}

inline void dbExecuter::doInsertBulk(request& req)
{
    m_tb = getTable(req.pbk->handle, SQLCOM_INSERT);
    if ((req.keyNum == -1) || m_tb->setKeyNum(m_tb->keyNumByMakeOrder(req.keyNum)))
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
    if (!m_tb->cursor())
        req.paramMask |= P_MASK_PB_ERASE_BM;
}

inline void dbExecuter::doGetSchema(request& req, netsvc::server::netWriter* nw)
{
    nw->setClientBuffferSize(*req.datalen);
    nw->beginExt(false);
    char* p = nw->curPtr() - sizeof(unsigned short);// orver write row space
    if (req.keyNum == SC_SUBOP_VIEW_BY_SQL)
    {// Return SQL statement as show create view. 
        TABLE_LIST tables; char key[256]; 
        const char* keyPtr = key;
        database* db = getDatabaseCid(req.cid);
        THD* thd = db->thd();
        std::string dbname = getDatabaseName(req);;
        std::string name = getTableName(req);
        tables.init_one_table(dbname.c_str(), dbname.size(), name.c_str(),
                name.size(), name.c_str(), TL_READ);
        uint key_length= cp_get_table_def_key(thd, &tables, &keyPtr);
        if (!cp_tdc_open_view(thd, &tables, name.c_str(), key, key_length, OPEN_VIEW_NO_PARSE))
        {
            unsigned int len = (unsigned int)(tables.view_body_utf8.length + 17 + name.size());
            if (len  <= *req.datalen)
            {
                char* p = nw->curPtr() - sizeof(unsigned short);// orver write row space
                sprintf_s(p, *req.datalen, "CREATE VIEW %s as %s", name.c_str(), tables.view_body_utf8.str);
                p[len] = 0x00;
                nw->asyncWrite(NULL, len+1, netsvc::server::netWriter::curSeekOnly);
                nw->writeHeadar(P_MASK_DATA | P_MASK_DATALEN, 0);
            }
            else
                nw->writeHeadar(0, STATUS_BUFFERTOOSMALL);
        }else
            nw->writeHeadar(0, STATUS_TABLE_NOTOPEN);
    }
    else if (req.keyNum == SC_SUBOP_BY_SQL)
    {// Return SQL statement as show create table. 
        String s;
        m_tb = getTable(req.pbk->handle);
        m_tb->getCreateSql(&s);
        unsigned int len = (unsigned int)s.length();
        if (len+1 <= *req.datalen)
        {
            char* p = nw->curPtr() - sizeof(unsigned short);// orver write row space
            memcpy(p, s.ptr(), len);
            p[len] = 0x00;
            nw->asyncWrite(NULL, len+1, netsvc::server::netWriter::curSeekOnly);
            nw->writeHeadar(P_MASK_DATA | P_MASK_DATALEN, (short_td)errorCode(m_tb->stat()));
        }
        else
            nw->writeHeadar(0, STATUS_BUFFERTOOSMALL);
        m_tb->unUse();
    }
    else
    {// Return tabledef image binary.
        database* db = getDatabaseCid(req.cid);
        std::string name = getTableName(req);
        schemaBuilder sb;
        protocol::tdap::tabledef* td = sb.getTabledef(db, name.c_str(), 
                (unsigned char*)p, *req.datalen);
        if (td)
        {
            nw->asyncWrite(NULL, td->varSize + sizeof(unsigned short), netsvc::server::netWriter::curSeekOnly);
            nw->writeHeadar(P_MASK_DATA | P_MASK_DATALEN, (short_td)errorCode(db->stat()));
        }
        else
            nw->writeHeadar(0, errorCodeSht(sb.stat()));
    }
    unsigned int* totalLen = (unsigned int*)nw->ptr();
    nw->datalen = *totalLen = nw->resultLen();
}

/**
@result 
         2byte recordLength
         4byte esitimate recordCount
*/
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
        req.result = errorCodeSht(m_tb->stat());
    }
    else
        req.result = STATUS_BUFFERTOOSMALL;
}

inline enum_tx_isolation getIsolationLevel(int op)
{
    if (op > CONSISTENT_READ)
        return (enum_tx_isolation)0;
    else if (op > TRN_ISO_SERIALIZABLE)
        return ISO_SERIALIZABLE;
    else if(op > TRN_ISO_REPEATABLE_READ)
        return ISO_REPEATABLE_READ;
    return ISO_READ_COMMITTED;
}

inline bool snapshotWithBinlogPos(int op)
{
    return (op == CONSISTENT_READ_WITH_BINLOG_POS+TD_BEGIN_SHAPSHOT);
}

inline short getTrnsactionType(int op)
{
    op = op % 1000;
    if (op > NOWAIT_WRITE) // 500
        op -= NOWAIT_WRITE;
    if (op >= 300)
        return TRN_RECORD_LOCK_MUILTI;
    return TRN_RECORD_LOCK_SINGLE;
}

inline rowLockMode* getRowLockMode(int op, rowLockMode* lck)
{
    lck->lock = false;
    lck->read = (op > ROW_LOCK_S);
    if (op > 1000)
        op = op % 1000;
    if (op < NOWAIT_WRITE)
    {
        if (op >= LOCK_SINGLE_NOWAIT && op < LOCK_MULTI_NOWAIT)
            lck->lock = true;
    }
    return lck;
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
        if (!m_authChecked)
        {
          if (op != TD_CONNECT && op != TD_CREATETABLE &&
              op != TD_OPENTABLE && op != TD_GETSERVER_CHARSET)
          {
              req.result = STATUS_ACCESS_DENIED;
              return EXECUTE_RESULT_ACCESS_DNIED;
          }
        }
        switch (op)
        {
        case TD_KEY_SEEK:
        case TD_KEY_AFTER:
        case TD_KEY_OR_AFTER:
        case TD_KEY_BEFORE:
        case TD_KEY_OR_BEFORE:
        {
            rowLockMode lck;
            doSeekKey(req, op, getRowLockMode(opTrn, &lck));
            break;
        }
        case TD_KEY_FIRST:
        {
            rowLockMode lck;
            doMoveFirst(req, getRowLockMode(opTrn, &lck));
            break;
        }
        case TD_KEY_PREV:
        case TD_KEY_LAST:
        case TD_KEY_NEXT:
        {
            rowLockMode lck;
            doMoveKey(req, op, getRowLockMode(opTrn, &lck));
            break;
        }
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
        {
            database* db = getDatabaseCid(req.cid);
            transactionResult = db->beginTrn(getTrnsactionType(opTrn)
                                                    ,getIsolationLevel(opTrn));
            req.result = errorCodeSht(db->stat());
            break;
        }
        case TD_END_TRANSACTION:
        {
            database* db = getDatabaseCid(req.cid);
            transactionResult = db->commitTrn();
            req.result = errorCodeSht(db->stat());
            break;
        }
        case TD_ABORT_TRANSACTION:
        {
            database* db = getDatabaseCid(req.cid);
            transactionResult = db->abortTrn();
            req.result = errorCodeSht(db->stat());
            break;
        }
        case TD_BEGIN_SHAPSHOT:
        {
            bool withSnapshot = snapshotWithBinlogPos(opTrn);
            if (withSnapshot && *req.datalen < sizeof(binlogPos))
                req.result = STATUS_BUFFERTOOSMALL;
            else
            {
                binlogPos bpos;
                memset(&bpos, 0, sizeof(binlogPos));
                database* db = getDatabaseCid(req.cid);
                THD* thd = NULL;
                #ifdef NOTUSE_BINLOG_VAR 
                if (withSnapshot)
                {
                    thd = getThd();
                    db->use();
                }
                #endif
                transactionResult = db->beginSnapshot(
                        getIsolationLevel(opTrn), withSnapshot ? &bpos : NULL, thd, m_blobBuffer);
                req.result = errorCodeSht(db->stat());
                if (transactionResult && withSnapshot)
                {   
                    // return binlog position for replication.
                    req.paramMask = P_MASK_STAT;
                    if (bpos.type == REPL_POSTYPE_GTID) req.paramMask |= P_MASK_BLOBBODY;
                    memcpy(req.data, &bpos, sizeof(binlogPos));
                    req.resultLen = sizeof(binlogPos);
                }
            }
            break;
        }
        case TD_END_SNAPSHOT:
        {
            database* db = getDatabaseCid(req.cid);
            transactionResult = db->endSnapshot();
            req.result = errorCodeSht(db->stat());
            break;
        }
        case TD_TABLE_INFO: // support recordlen and recordCount only.
            doStat(req);
            break;
        case TD_POS_FIRST:
        case TD_POS_LAST:
        case TD_POS_NEXT:
        case TD_POS_PREV:
        {
            rowLockMode lck;
            doStepRead(req, op, getRowLockMode(opTrn, &lck));
            break;
        }
        case TD_BOOKMARK:
            m_tb = getTable(req.pbk->handle);
            req.paramMask = P_MASK_MOVPOS;
            req.data = (void*)m_tb->position();
            req.resultLen = m_tb->posPtrLen();
            break;
        case TD_RECONNECT:
            nw->resize(*req.datalen);
            resultBuffer = nw->ptr();
            
            if (!doOpenTable(req, resultBuffer, true))
            {
                if (req.result == 0)
                    req.result = ERROR_TD_INVALID_CLINETHOST;
                break;
            }
            {
                char* p = (char*)req.data;
                req.keyNum = *p;
                if (*(++p) == 0)
                {
                    req.paramMask = P_MASK_POSBLK;
                    break; // No bookmark
                }
                req.data = ((char*)req.data) + 2;
                
                if (m_tb) m_tb->unUse();
            }
            //fall through  restore position 
        case TD_MOVE_BOOKMARK:
        {
            rowLockMode lck;
            getRowLockMode(opTrn, &lck);
            m_tb = getTable(req.pbk->handle, lck.lock ? SQLCOM_UPDATE : SQLCOM_SELECT);
            m_tb->setRowLock(&lck);
            char keynum = req.keyNum;
            if (keynum != -1)
            {
                keynum = m_tb->keyNumByMakeOrder(req.keyNum);
                if (!m_tb->keynumCheck(keynum))
                {
                    req.result = STATUS_INVALID_KEYNUM;
                    break;
                }
            }
            m_tb->movePos((uchar*)req.data, keynum);
            if (lck.lock && m_tb->stat())
                m_tb->setRowLockError();
            readAfter(req, m_tb, this);
            break;
        }
        case TD_KEY_GE_NEXT_MULTI:
        case TD_KEY_LE_PREV_MULTI:
            nw->setClientBuffferSize(*(req.datalen));
            if (doReadMultiWithSeek(req, op, nw) == EXECUTE_RESULT_SUCCESS)
            {
                m_tb = NULL;
                return EXECUTE_RESULT_SUCCESS;
            }else
                resultBuffer = nw->ptr();
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
            }else
                resultBuffer = nw->ptr();
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
            }else
                req.result = m_tb->stat();
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
                req.paramMask = P_MASK_DATA | P_MASK_DATALEN ;
                req.data = m_tb->percentResult();
                req.resultLen = sizeof(int);
            }
            break;
        case TD_INSERT_BULK:
            doInsertBulk(req);
            break;
        case TD_UNLOCK:
            m_tb = getTable(req.pbk->handle);
            req.result = m_tb->unlock() ? STATUS_NO_CURRENT : 0;
            break;
        /* ddl or once at connection
        */
        case TD_GETSERVER_CHARSET:
        {
            if (*req.datalen == sizeof(trdVersiton))
            {
                trdVersiton* ver = (trdVersiton*)req.data;
                strcpy_s(ver->cherserServer, sizeof(ver->cherserServer),
                         global_system_variables.collation_server->csname);

                ver->desc.srvMysqlMajor = MYSQL_VERSION_ID / 10000;
                ver->desc.srvMysqlMinor = (MYSQL_VERSION_ID / 100) % 100;
                ver->desc.srvMysqlRelease = MYSQL_VERSION_ID % 100;
                ver->desc.srvMajor = TRANSACTD_VER_MAJOR;
                ver->desc.srvMinor = TRANSACTD_VER_MINOR;
                ver->desc.srvRelease = TRANSACTD_VER_RELEASE;
                req.resultLen = sizeof(trdVersiton);
                req.paramMask |= P_MASK_DATA | P_MASK_DATALEN;
            }else
                req.result = SERVER_CLIENT_NOT_COMPATIBLE;
            break;
        }
        
        case TD_CONNECT:
            if (!connect(req))
                req.result = ERROR_TD_INVALID_CLINETHOST;
            break;
        case TD_RESET_CLIENT:
        case TD_STOP_ENGINE: // close all table
            releaseDatabase(req, op);
            break;
        case TD_AUTOMEKE_SCHEMA:
            m_tb = getTable(req.pbk->handle, SQLCOM_INSERT);
            req.result = schemaBuilder().execute(getDatabaseCid(req.cid), m_tb, (req.keyNum==1));
            break;
        case TD_CREATETABLE:
            doCreateTable(req);
            break;
        case TD_OPENTABLE:
            nw->resize(*req.datalen);
            resultBuffer = nw->ptr();
            if (!doOpenTable(req, nw->ptr(), false))
            {
                if (req.result == 0)
                    req.result = ERROR_TD_INVALID_CLINETHOST;
            }
            break;
        case TD_CLOSETABLE:
            m_tb = getTable(req.pbk->handle);
            if (m_tb)
            {
                std::string tbname = m_tb->name();
                m_tb->close();
                m_tb = NULL;
                if (req.keyNum == CR_SUBOP_DROP)
                {
                    database* db = getDatabaseCid(req.cid);
                    req.result = ddl_dropTable(db, tbname, db->name(), tbname);
                }
            }
            break;
        case TD_BUILD_INDEX:
        { 
            database* db = getDatabaseCid(req.cid);
            m_tb = getTable(req.pbk->handle);
            std::string cmd((const char*)req.data);
            std::string tbname = m_tb->name();
            req.result = ddl_addIndex(db, tbname, cmd);
            break;
        }
        case TD_DROP_INDEX:
        { // Key name of multi byte charctor is not supported. Use only ascii.
            database* db = getDatabaseCid(req.cid);
            m_tb = getTable(req.pbk->handle);
            req.result = ddl_dropIndex(db, m_tb->name(), 
                    m_tb->keyName(m_tb->keyNumByMakeOrder(req.keyNum)));
            break;
        }
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
            if (*req.datalen >= sizeof(btrVersion)* 3)
            {
                btrVersion* v = (btrVersion*)req.data;
                ++v;
                v->majorVersion = MYSQL_VERSION_ID / 10000;
                v->minorVersion = (MYSQL_VERSION_ID / 100) % 100;
                #if defined(MARIADB_BASE_VERSION)
                    v->type = MYSQL_TYPE_MARIA;
                #else
                    v->type = MYSQL_TYPE_MYSQL;
                #endif
                ++v;
                v->majorVersion = TRANSACTD_VER_MAJOR;
                v->minorVersion = TRANSACTD_VER_MINOR;
                v->type = 'T';
                req.paramMask = P_MASK_DATA | P_MASK_DATALEN;
                req.resultLen = sizeof(btrVersion)* 3;
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
            req.result = ddl_tableComment(db, m_tb->name(), s.c_str());
            break;
        }
        case TD_ACL_RELOAD:
            req.result = getDatabaseCid(req.cid)->aclReload();
            break;
        case TD_GET_SCHEMA:
            {
                doGetSchema(req, nw);
                return EXECUTE_RESULT_SUCCESS;
            }
        case TD_STORE_TEST:
        {   
            m_tb = getTable(req.pbk->handle, SQLCOM_UPDATE);
            bool ncc = (req.keyNum == -1);
            m_tb->beginUpdate(req.keyNum);
            if (m_tb->stat() == 0)
            {
                std::vector<std::string> ss;
                std::string s((const char*)req.data);
                split(ss, s, "\t");
                if ( ss.size() >= 2)
                {
                    for (int i = 0; i < (int)ss.size() ; i+=2)
                        m_tb->setValue((short)atol(ss[i].c_str()), ss[i + 1], 0);
                    m_tb->update(ncc);
                    req.result = errorCodeSht(m_tb->stat());
                    req.paramMask = P_MASK_POSBLK | P_MASK_KEYBUF;
                    if (!m_tb->cursor())
                        req.paramMask |= P_MASK_PB_ERASE_BM;
                }else
                    req.result = STATUS_INVALID_FIELDVALUE;
            }else
                req.result = errorCodeSht(m_tb->stat());
           
            break;
        }
        case TD_SET_TIMESTAMP_MODE:
        {
            m_tb = getTable(req.pbk->handle);
            bool always = req.keyNum == TIMESTAMP_ALWAYS;
            m_tb->setTimestampAlways(always);
            req.result = STATUS_SUCCESS;
            break;
        }
        default:
            req.result = STATUS_NOSUPPORT_OP;
            break;
        }
        DEBUG_WRITELOG2(op, req)
        DEBUG_ERROR_MEMDUMP(req.result, "error", req.m_readBuffer, *((unsigned int*)req.m_readBuffer))
        size = req.serialize(m_tb, resultBuffer);
        if (m_tb)
            m_tb->unUse();
        short dymmy = 0;
        if ((req.result == 0) && (req.paramMask & P_MASK_BLOBBODY) &&
            m_blobBuffer->fieldCount())
            size = req.serializeBlobBody(m_blobBuffer, resultBuffer, size,
                                         FILE_MAP_SIZE, optionalData, dymmy);

        m_tb = NULL;
        if (transactionResult)
        {
            if ((op == TD_BEGIN_TRANSACTION) || (op == TD_BEGIN_SHAPSHOT))
                return EXECUTE_RESULT_FORCSE_SYNC; // this is begin
            return EXECUTE_RESULT_FORCSE_ASYNC;    // this is end 
        }
        if (req.result == ERROR_TD_INVALID_CLINETHOST)
            return EXECUTE_RESULT_ACCESS_DNIED;
        DEBUG_PROFILE_END_OP(1, op)
        return EXECUTE_RESULT_SUCCESS;
    }

    catch (bzs::rtl::exception& e)
    {
        const int* code = getCode(e);
        clenupNoException();
        DEBUG_ERROR_MEMDUMP(*code, "error", req.m_readBuffer, *((unsigned int*)req.m_readBuffer))

        req.reset();
        
        if (code)
            req.result = *code;
        else
        {
            req.result = 20000;
            sql_print_error("%s", boost::diagnostic_information(e).c_str());
        }
        printWarningMessage(code, getMsg(e));
    }

    catch (...)
    {
        try
        {
            try
            {
                clenupNoException();
                DEBUG_ERROR_MEMDUMP(20001, "error", req.m_readBuffer, *((unsigned int*)req.m_readBuffer))
                req.reset();
                req.result = 20001;
                dumpStdErr(op, req, m_tb);
            }
            catch(...){}
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

void makeRandomKey(unsigned char *buf, unsigned int size)
{
	unsigned char *end= buf + size;
	std::random_device rnd;
	for (; buf < end; buf++)
		*buf = (unsigned char)(rnd() * 94 + 33);
	*buf= 0x00;
}

size_t dbExecuter::getAcceptMessage(char* message, size_t size)
{
    // make handshake packet
    m_authChecked = false;
    assert(size >= sizeof(trdVersiton));

    handshale_t* hst = (handshale_t*)message;
    hst->options = 0;
    trdVersiton* ver = &hst->ver;
    hst->size = sizeof(handshale_t);

    strcpy_s(ver->cherserServer, sizeof(ver->cherserServer),
                global_system_variables.collation_server->csname);
    ver->desc.srvMysqlMajor = MYSQL_VERSION_ID / 10000;
    ver->desc.srvMysqlMinor = (MYSQL_VERSION_ID / 100) % 100;
    ver->desc.srvMysqlRelease = MYSQL_VERSION_ID % 100;
#if defined(MARIADB_BASE_VERSION)
    ver->desc.srvMysqlType = MYSQL_TYPE_MARIA;
#else
    ver->desc.srvMysqlType = MYSQL_TYPE_MYSQL;
#endif
    ver->desc.srvMajor = TRANSACTD_VER_MAJOR;
    ver->desc.srvMinor = TRANSACTD_VER_MINOR;
    ver->desc.srvRelease = TRANSACTD_VER_RELEASE;

    hst->transaction_isolation = (unsigned short)getTransactdIsolation();
    hst->lock_wait_timeout = getTransactdLockWaitTimeout();
    int role = getRole();
    if (role == HA_ROLE_MASTER)
        hst->options |= HST_OPTION_ROLE_MASTER;
    else if(role == HA_ROLE_SLAVE)
        hst->options |= HST_OPTION_ROLE_SLAVE;
    if (strcmp(g_auth_type, AUTH_TYPE_MYSQL_STR) == 0)
    {
        makeRandomKey(m_scramble, MYSQL_SCRAMBLE_LENGTH);
        memcpy(hst->scramble, m_scramble, sizeof(hst->scramble));
        
    }else
    {
        hst->scramble[0] = 0x00;
        hst->options |= HST_OPTION_NO_SCRAMBLE;
        hst->size -= sizeof(m_scramble);
    }
    return hst->size;
}
// ---------------------------------------------------------------------------
//      class connMgrExecuter
// ---------------------------------------------------------------------------
class safeLockReadChannels
{
    connMgrExecuter* m_exec;
    bool m_locked;
public:
    safeLockReadChannels(connMgrExecuter* exec) :
        m_exec(exec), m_locked(false){}
    bool lock()
    {
        int n = 0;
        while (!haLock())
        {
            Sleep(500);
            if (++n >= 120) return false; 
        }
        m_locked = true;
        return m_locked;
    }
    int execute(char* buf, size_t& size)
    {
        return m_exec->channels(buf, size);
    }
    ~safeLockReadChannels() { if (m_locked) haUnlock();}
};

connMgrExecuter::connMgrExecuter(request& req, unsigned __int64 parent, blobBuffer* bb)
    : m_req(req), m_modHandle(parent), m_blobBuffer(bb) 
{
    m_blobBuffer->clear();
}

int serialize(request& req, char* buf, size_t& size, const connection::records& records, short stat
        , blobBuffer* bb = NULL)
{
    req.reset();
    req.paramMask = P_MASK_DATA | P_MASK_DATALEN | P_MASK_KEYBUF;
    if (bb && bb->fieldCount())
        req.paramMask |= P_MASK_BLOBBODY;
    if (records.size())
        req.data = (void*)&records[0];
    else
        req.paramMask = P_MASK_DATALEN;
    int len = sizeof(record);
    req.resultLen = (uint_td)(len * records.size());
    req.result = stat;
    req.keylen = 4;
    req.keybuf = &len;
    size = req.serialize(NULL, buf);
    return EXECUTE_RESULT_SUCCESS;
}

int connMgrExecuter::read(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    unsigned __int64* mod = (unsigned __int64*)m_req.keybuf;
    const connection::records& records = st.getRecords(mod[0], (int)mod[1]);
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::definedDatabases(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.definedDatabases();
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::systemVariables(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.systemVariables();
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::extendedVariables(netsvc::server::netWriter* nw)
{
    connManager st(m_modHandle);
    const connection::records& records = st.extendedVariables(m_blobBuffer);
    int v = serialize(m_req, nw->ptr(), nw->datalen, records, st.stat(), m_blobBuffer);
    short dymmy = 0;
    if ((m_req.result == 0) && m_blobBuffer->fieldCount())
        nw->datalen = m_req.serializeBlobBody(m_blobBuffer, nw->ptr(), nw->datalen,
        FILE_MAP_SIZE, nw->optionalData(), dymmy);
    return v;
}

int connMgrExecuter::statusVariables(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.statusVariables();
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::schemaTables(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.schemaTables((const char*)m_req.keybuf); 
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::definedTables(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.definedTables((const char*)m_req.keybuf, TABLE_TYPE_NORMAL_TABLE);
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::definedViews(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = 
            st.definedTables((const char*)m_req.keybuf, TABLE_TYPE_VIEW);
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::slaveStatus(netsvc::server::netWriter* nw)
{
    connManager st(m_modHandle);
    const connection::records& records = 
            st.readSlaveStatus((const char*)m_req.keybuf, m_blobBuffer);
    int v =  serialize(m_req, nw->ptr(), nw->datalen, records, st.stat(), m_blobBuffer);
    short dymmy = 0;
    if ((m_req.result == 0) && m_blobBuffer->fieldCount())
        nw->datalen = m_req.serializeBlobBody(m_blobBuffer, nw->ptr(), nw->datalen,
                                         FILE_MAP_SIZE, nw->optionalData(), dymmy);
    return v;
}

int connMgrExecuter::channels(char* buf, size_t& size)
{
    connManager st(m_modHandle);
    const connection::records& records = st.channels();
    return serialize(m_req, buf, size, records, st.stat());
}

int connMgrExecuter::slaveHosts(netsvc::server::netWriter* nw)
{
    connManager st(m_modHandle);
    const connection::records& records = st.slaveHosts(m_blobBuffer);
    int v = serialize(m_req, nw->ptr(), nw->datalen, records, st.stat(), m_blobBuffer);
    short dymmy = 0;
    if ((m_req.result == 0) && m_blobBuffer->fieldCount())
        nw->datalen = m_req.serializeBlobBody(m_blobBuffer, nw->ptr(), nw->datalen,
                                        FILE_MAP_SIZE, nw->optionalData(), dymmy);
    return v;
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

/* redefined. First defined at transactd.cpp */
#define TCP_TPOOL_SERVER     2
void haPrintMessage(module* mod, int op, bool retVal)
{
    const char* p="";
    switch (op)
    {
    case TD_STSTCS_HA_LOCK: p = "HA_LOCK"; break;
    case TD_STSTCS_HA_UNLOCK: p = "HA_UNLOCK"; break;
    case TD_STSTCS_HA_SET_ROLEMASTER: p = "HA_SET_ROLEMASTER"; break;
    case TD_STSTCS_HA_SET_ROLENONE: p = "HA_SET_ROLENONE"; break;
    case TD_STSTCS_HA_SET_ROLESLAVE: p = "HA_SET_ROLESLAVE"; break;
    case TD_STSTCS_HA_SET_TRXBLOCK: p = "HA_SET_TRXBLOCK"; break;
    case TD_STSTCS_HA_SET_TRXNOBLOCK: p = "HA_SET_TRXNOBLOCK"; break;
    case TD_STSTCS_HA_ENABLE_FO: p = "HA_ENABLE_FO"; break;
    case TD_STSTCS_HA_DISBLE_FO: p = "HA_DISBLE_FO"; break;
    }
    if (retVal)
        sql_print_information("Transactd: %s by %s@%s", p, mod->user(), mod->host());
    else
        sql_print_error("Transactd: %s by %s@%s", p, mod->user(), mod->host());
}

void connMgrExecuter::execHaCommand()
{
    if (g_tcpServerType == TCP_TPOOL_SERVER)
    {
        m_req.reset();
        m_req.result = STATUS_NOSUPPORT_OP;
        return;
    }
    bool ret = true;
    switch ((int)m_req.keyNum)
    {
    case TD_STSTCS_HA_LOCK:
        ret = haLock();
        break;
    case TD_STSTCS_HA_UNLOCK:
        ret = haUnlock();
        break;
    case TD_STSTCS_HA_SET_ROLEMASTER:
        ret = setRole(HA_ROLE_MASTER);
        break;
    case TD_STSTCS_HA_SET_ROLENONE:
        ret = setRole(HA_ROLE_NONE);
        break;
    case TD_STSTCS_HA_SET_ROLESLAVE:
        ret = setRole(HA_ROLE_SLAVE);
        break;
    case TD_STSTCS_HA_SET_TRXBLOCK:
    case TD_STSTCS_HA_SET_TRXNOBLOCK:
        ret = setTrxBlock(m_req.keyNum == TD_STSTCS_HA_SET_TRXBLOCK);
        break;
    case TD_STSTCS_HA_ENABLE_FO:
    case TD_STSTCS_HA_DISBLE_FO:
        ret = setEnableFailover(m_req.keyNum == TD_STSTCS_HA_ENABLE_FO);
        break;
    }
    module* mod = dynamic_cast<module*>((module*)m_modHandle);
    assert(mod);
    haPrintMessage(mod, m_req.keyNum, ret);
    m_req.reset();
    if (ret == false)
        m_req.result = STATUS_LOCK_ERROR;
}

int connMgrExecuter::commandExec(netsvc::server::netWriter* nw)
{
    char_td op = m_req.keyNum;
    try
    {
        if (op == TD_STSTCS_DISCONNECT_ONE)
            return disconnectOne(nw->ptr(), nw->datalen);
        else if (op == TD_STSTCS_DISCONNECT_ALL)
            return disconnectAll(nw->ptr(), nw->datalen);
        else if (op >= TD_STSTCS_HA_LOCK && op <= TD_STSTCS_HA_DISBLE_FO)
            execHaCommand();
        else if (*m_req.datalen == (uint_td)63976)
        {
            switch (op)
            {
            case TD_STSTCS_READ:
                return read(nw->ptr(), nw->datalen);
            case TD_STSTCS_DATABASE_LIST:
                return definedDatabases(nw->ptr(), nw->datalen);
            case TD_STSTCS_SYSTEM_VARIABLES:
                return systemVariables(nw->ptr(), nw->datalen);
            case TD_STSTCS_STATUS_VARIABLES:
                return statusVariables(nw->ptr(), nw->datalen);
            case TD_STSTCS_SCHEMA_TABLE_LIST:
                return schemaTables(nw->ptr(), nw->datalen);
            case TD_STSTCS_TABLE_LIST:
                return definedTables(nw->ptr(), nw->datalen);
            case TD_STSTCS_VIEW_LIST:
                return definedViews(nw->ptr(), nw->datalen);
            case TD_STSTCS_SLAVE_STATUS:
                return slaveStatus(nw);
            case TD_STSTCS_SLAVE_HOSTS:
                return slaveHosts(nw);
            case TD_STSTCS_SLAVE_CHANNELS:
                return channels(nw->ptr(), nw->datalen);
            case TD_STSTCS_SLAVE_CHANNELS_LOCK:
            {
                safeLockReadChannels readChannels(this);
                if (readChannels.lock())
                    return readChannels.execute(nw->ptr(), nw->datalen);
                m_req.reset();
                m_req.result = STATUS_LOCK_ERROR;
                break;
            }
            case TD_STSTCS_EXTENDED_VARIABLES:
                return extendedVariables(nw);
            default:
                m_req.reset();
                m_req.result = STATUS_NOSUPPORT_OP;
                break;
            }
        }else
        {
            m_req.reset();
            m_req.result = SERVER_CLIENT_NOT_COMPATIBLE;
        }
        nw->datalen = m_req.serialize(NULL, nw->ptr());
    }
    catch (bzs::rtl::exception& e)
    {
        std::string s = *getMsg(e);
        const int* code = getCode(e);
        if (code)
            m_req.result = *code;
        else
        {
            m_req.result = 20000;
            s = boost::diagnostic_information(e);
        }
        char buf[256];
        sprintf_s(buf, 256, "Stastics operation %d : ", (int)op);
        s.insert(0, buf);
        printWarningMessage(code, &s);
    }
    catch (...)
    {
        try
        {
            DEBUG_ERROR_MEMDUMP(20001, "error", m_req.m_readBuffer, 
                            *((unsigned int*)m_req.m_readBuffer))
            m_req.reset();
            m_req.result = 20001;
            dumpStdErr(op, m_req, NULL);
        }
        catch(...){}
    }
    return EXECUTE_RESULT_SUCCESS;
}

// ---------------------------------------------------------------------------
//      class commandExecuter
// ---------------------------------------------------------------------------
commandExecuter::commandExecuter(netsvc::server::IAppModule* mod) 
{
    m_dbExec.reset(new dbExecuter(mod));
}

commandExecuter::~commandExecuter()
{
    m_dbExec.reset();
}

int commandExecuter::execute(netsvc::server::netWriter* nw)
{
    if (m_req.op == TD_STASTISTICS)
        return connMgrExecuter(m_req, (unsigned __int64)m_dbExec->mod(), 
                    m_dbExec->m_blobBuffer).commandExec(nw);
    int ret = m_dbExec->commandExec(m_req, nw);
    
    // Start blocking mode need lock slave servers first.
    // therefore 
    if (isTrxBlocking() && m_dbExec->trxProcessing() == 0)
        return EXECUTE_RESULT_SEND_QUIT;
    return ret;
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
