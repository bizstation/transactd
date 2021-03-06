#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
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
#include "sqlBuilder.h"
#include "request.h"
#include <bzs/db/blobBuffer.h>
#include <boost/thread/mutex.hpp>
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/netsvc/client/tcpClient.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/uri.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/env/compiler.h>
#include <bzs/rtl/stl_uty.h>
#include <vector>
#ifdef _WIN32
#include <mbstring.h>
#else
#include <pthread.h>
#endif

namespace bnet = bzs::netsvc::client;

extern bnet::connections* m_cons;

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{
#define TABELNAME_PREFIX "dbfile="

class client;
void setClientThread(client* v);

#define PORT_BUFSIZE 16

struct endpoint_t
{
    char host[MAX_PATH];
    char port[PORT_BUFSIZE];
};

/* client class 
This instance is created for each thread.
*/
class client
{
    mutex m_mutex;
    request m_req;
    posblk m_tmpPbk;
    ushort_td m_op;
    ushort_td m_preResult;
    std::string m_sql;
    std::string m_serverCharData;
    char* m_cryptPwd;
    blobBuffer m_blobBuffer;
    uint_td m_tmplen;
    bool m_logout;
    bool m_disconnected;
    bool m_connecting;

    std::vector<char> m_sendbuf;

    bool checkVersion(int major, int ninor)
    {
        const clsrv_ver* v = ver();
        if (!v) return false;
        return (v->srvMajor > major) || ((v->srvMajor == major) && (v->srvMinor >= ninor));
    }

    bool checkVersion(clsrv_ver& ver)
    {
        if ((ver.srvMajor < 2) || ((ver.srvMajor == 2) && (ver.srvMinor < 3)))
            return false;
        return true;
    }

    inline bnet::connection* con() { return m_req.cid->con; };

    inline void setCon(bnet::connection* con)
    {
        m_req.cid->con = con;
    }

    inline void disconnect()
    {
        bnet::connection* c = con();
        if (!c)
            m_req.result = 1;
        else
        {
            setCon(NULL);
            //Release connection refCount
            m_disconnected = m_cons->disconnect(c);
        }
    }

    std::string getHostName(const char* uri)
    {
        _TCHAR tmp[MAX_PATH];
        return hostName(uri, tmp, MAX_PATH);
    }

    endpoint_t* endPoint(const char* uri, endpoint_t* ep)
    {
        tdap::endPoint(uri, ep->host, MAX_PATH, ep->port, PORT_BUFSIZE);
        return ep;
    }

    std::string getTableName(const char* uri)
    {
        if (uri)
        {
            mbchar* en = (mbchar*)uri;
            mbchar* st = _mbsstr(en, (mbchar*)TABELNAME_PREFIX);
            if (st)
            {
                st += strlen(TABELNAME_PREFIX);
                mbchar* tmp = _mbsstr(st, (mbchar*)PSEPARATOR_A);
                if (tmp)
                    st = tmp + 1;
                en = _mbsstr(st, (mbchar*)".");
                if (en == NULL)
                    en = st + strlen((char*)st);
                return std::string((const char*)st, en - st);
            }
        }
        return "";
    }
    static void addSecondCharsetData(unsigned int destCodePage,
                                     std::string& src);

    bool handshake(bnet::connection* c)
    {
        //Implements handshake here
        handshale_t* hst  = (handshale_t*)c->read();
        if (c->error()) return false;
        bool auth = (hst->size == sizeof(handshale_t));
        bool min = (hst->size == (sizeof(handshale_t) -  sizeof(hst->scramble)));
        memcpy(c->versions(), &hst->ver.desc, sizeof(clsrv_ver));

        if (min || auth)
        {
            if (!checkVersion(hst->ver.desc))
            {
                m_preResult = SERVER_CLIENT_NOT_COMPATIBLE; 
                return false;
            }
            c->setCharsetServer(mysql::charsetIndex(hst->ver.cherserServer));
            m_req.cid->lock_wait_timeout = hst->lock_wait_timeout;
            m_req.cid->transaction_isolation = hst->transaction_isolation;
            if (c->userOptions() & CL_OPTION_CHECK_ROLE)
            {
                bool clRoleMaster = (c->userOptions() & HST_OPTION_ROLE_MASTER) != 0; 
                bool srvRoleMaster = (hst->options & HST_OPTION_ROLE_MASTER) != 0;
  
                if (clRoleMaster != srvRoleMaster)
                {
                    m_preResult = ERROR_TD_INVALID_SERVER_ROLE; 
                    return false;
                }
            }
        }
        if (auth)
        {
            char user[50];
            char pwd[MAX_PATH];
            char* p = (char*)m_req.keybuf;
            userName(p, user, 50);
            if (m_cryptPwd == NULL)
                m_cryptPwd = new char[70];
            passwd((const char*)m_req.keybuf, pwd, MAX_PATH);
            if (pwd[0])
                mysqlCryptPwd(m_cryptPwd, pwd, hst->scramble);
            else
                memset(m_cryptPwd, 0, MYSQL_SCRAMBLE_LENGTH);
            strcpy_s(m_cryptPwd + MYSQL_SCRAMBLE_LENGTH, 50, user);
        }else
        {   // No auth
            if (m_cryptPwd)
                delete [] m_cryptPwd;
            m_cryptPwd = NULL;
        }
        return true;
    }

    static bool handshakeCallback(bnet::connection* c, void* data)
    {
        return ((client*)data)->handshake(c);
    }

public:
    client() : m_cryptPwd(NULL), m_disconnected(false), m_connecting(false)
    {
    }

    ~client()
    {
        if (m_cryptPwd)
            delete [] m_cryptPwd;
    }

    void cleanup()
    {
        m_connecting = false;
/* When in win32, delete this object auto maticaly by dllmain function 
   at reason = DLL_THREAD_DETACH.
   But in LINUX does not have that mechanism.
   Each dissconnect delete this object.

*/

#ifndef _WIN32
        // delete this. Do not change member variables after this line.
        if (m_disconnected)
            setClientThread(NULL);
#endif
    }

    inline request& req() { return m_req; }

    inline const clsrv_ver* ver()
    { 
        if (m_req.cid->con)
            return (clsrv_ver*)m_req.cid->con->versions();
        return NULL; 
    }
    inline bool isSupportFunction(short op)
    {
        if (op == TD_GET_SCHEMA)
            return checkVersion(2, 6);
        return false;
    }

    inline void setParam(ushort_td op, posblk* pbk, void_td* data,
                         uint_td* datalen, void_td* keybuf, keylen_td keylen,
                         char_td keyNum, clientID* cid)
    {
        m_op = op;
        m_req.op = op;
        m_req.pbk = pbk;
        m_req.data = data;
        m_req.datalen = datalen;
        m_req.keybuf = keybuf;
        m_req.keylen = keylen;
        m_req.keyNum = keyNum;
        m_req.cid = cid;
        m_req.paramMask = 0;
        m_preResult = 0;
        m_logout = false;
        if (m_req.pbk == NULL)
            m_req.pbk = &m_tmpPbk;
    }

    inline short_td result() { return m_preResult; }

    inline bool stop_if()
    {
        if (m_op == TD_STOP_ENGINE)
        {
            m_req.result = 0;
            disconnect();
            return !m_req.result;
        }
        return false;
    }

    inline bnet::connection* doConnect(endpoint_t& ep, bool newConnection, bool clearNRCache)
    {
        bnet::connection* c = 
            m_cons->connect(ep.host, ep.port,
                client::handshakeCallback, this, newConnection, clearNRCache);
        if (c)
        {
            setCon(c);
            m_connecting = true;
        }
        return c;
    }

    bnet::connection* connect(bool newConnection = false)
    {
        bnet::connection* c = NULL;
        short errorOffset = 0;
        if (!m_req.cid->con)
        {
            m_preResult = 0;
            m_connecting = false;
            endpoint_t ep;
            endPoint((const char*)m_req.keybuf, &ep);
            if (ep.host[0] == 0x00)
            {
                m_preResult = ERROR_TD_HOSTNAME_NOT_FOUND;
                return c;
            }
            c = doConnect(ep, newConnection, false);
            if (!c)
            {
                if (m_cons->haNameResolver() && 
                    (m_preResult == 0 || m_preResult == ERROR_TD_INVALID_SERVER_ROLE))
                {
                    m_preResult = 0;
                    c = doConnect(ep, newConnection, true);
                    errorOffset = ERROR_TD_RECONNECTED_OFFSET;
                }
            }
            if (!c && (m_preResult == 0))
                m_preResult = errorCode(m_cons->connectError());
            if (m_preResult) m_preResult += errorOffset; 
        }
        m_disconnected = !m_req.cid->con;
        return c;
    }

    inline void createIndex()
    {
        if (!ver())
        {
            m_preResult = ERROR_TD_NOT_CONNECTED;
            return; 
        }
        m_req.paramMask = P_MASK_NOKEYBUF;
        int charsetIndexServer =  getServerCharsetIndex();
        if (charsetIndexServer != -1)
        {
            unsigned char keynum = m_req.keyNum;
            bool specifyKeyNum = (keynum >= 0x80);
            if (keynum >= 0x80)
                keynum -= 0x80;
            m_sql = sqlBuilder::sqlCreateIndex((tabledef*)m_req.data, keynum, 
                                specifyKeyNum, charsetIndexServer, ver());
            m_req.data = (ushort_td*)m_sql.c_str();
            m_tmplen = (uint_td)(m_sql.size() + 1);
            m_req.datalen = &m_tmplen;
        }
    }

    inline void getSqlCreate(int charsetIndex = -1)
    {
        if (!ver())
        {
            m_req.result = ERROR_TD_NOT_CONNECTED;
            return; 
        }
        _TCHAR tmp[MAX_PATH*2]={0};
        stripAuth((const char*)m_req.keybuf, tmp, MAX_PATH);
        std::string name = getTableName(tmp);
        charsetIndex =  getServerCharsetIndex();
        if (charsetIndex != -1)
        {
            std::string sql = sqlBuilder::sqlCreateTable(name.c_str(), (tabledef*)m_req.data,
                                           charsetIndex, ver());
            uint_td  datalen = *m_req.datalen;
            *m_req.datalen = (uint_td)(sql.size() + 1);
            if (datalen <= sql.size())
            {
                m_req.result = STATUS_BUFFERTOOSMALL;
                return;
            }
            strcpy_s((char*)m_req.data, *m_req.datalen, sql.c_str());
        }
    }

    inline void create()
    {
        m_req.paramMask = P_MASK_ALL;
        if ((m_req.datalen == NULL) || *m_req.datalen == 0)
            m_req.paramMask = P_MASK_KEYONLY;
        else if (m_req.keyNum >
                 CR_SUBOP_SWAPNAME) // -126 swap -127 is rename. -128 is drop.
        {
             _TCHAR tmp[MAX_PATH*2]={0};
            stripAuth((const char*)m_req.keybuf, tmp, MAX_PATH);
            m_req.paramMask &= ~P_MASK_POSBLK;
            std::string name = getTableName(tmp);
            int charsetIndexServer = getServerCharsetIndex();
            if (charsetIndexServer != -1)
            {
                if ((m_req.keyNum == CR_SUBOP_BY_TABLEDEF) || 
                        (m_req.keyNum == CR_SUBOP_BY_TABLEDEF_NOCKECK)) // make by tabledef
                {
                    m_sql = sqlBuilder::sqlCreateTable(name.c_str(), (tabledef*)m_req.data,
                                           charsetIndexServer, ver());
                    m_req.keyNum -= 2; // -1= exists check 0 = no exists check
                }
                else if ((m_req.keyNum == CR_SUBOP_BY_SQL) || 
                        (m_req.keyNum == CR_SUBOP_BY_SQL_NOCKECK)) // make by sql
                {
                    m_req.keyNum -= 4; // -1= exists check 0 = no exists check
                    if (charsetIndexServer != CHARSET_UTF8 && charsetIndexServer != CHARSET_UTF8B4)
                        m_sql = sqlBuilder::convertString(mysql::codePage(charsetIndexServer), 65001,
                              (const char*)m_req.data);
                    else
                        m_sql = (const char*)m_req.data;
                }
                else
                    m_sql = sqlBuilder::sqlCreateTable(name.c_str(), (fileSpec*)m_req.data,
                                           charsetIndexServer, ver());
                m_req.data = (ushort_td*)m_sql.c_str();
                m_tmplen = (uint_td)(m_sql.size() + 1);
                m_req.datalen = &m_tmplen;
            }
        }
        else if ((m_req.keyNum == CR_SUBOP_SWAPNAME) ||
                 (m_req.keyNum == CR_SUBOP_RENAME))
        {
            int charsetIndexServer =  getServerCharsetIndex();
            if (charsetIndexServer != -1)
            {
                m_sql = (char*)m_req.data;
                addSecondCharsetData(mysql::codePage(charsetIndexServer), m_sql);
                m_req.data = (ushort_td*)m_sql.c_str();
                m_tmplen = (uint_td)(m_sql.size() + 1);
                m_req.datalen = &m_tmplen;
            }
        }
    }

    inline void reconnect()
    {
        bnet::connection* c = con();
        if (!c)
        {
            m_preResult = ERROR_TD_NOT_CONNECTED;
            return;
        }
        endpoint_t ep;
        endPoint((const char*)m_req.keybuf, &ep);
     
        if (ep.host[0] == 0x00)
        {
            m_preResult = ERROR_TD_HOSTNAME_NOT_FOUND;
            return;
        }
        //request req = m_req;
        if (!m_cons->reconnect(c, ep.host, ep.port, handshakeCallback, this))
        {
            if (m_preResult == 0)
                m_preResult = errorCode(m_cons->connectError());
            return;
        }
        //m_req = req;
        m_connecting = true;
        if (getServerCharsetIndex() != -1)
        {
            m_preResult = 0;
            buildDualChasetKeybuf();
            m_disconnected = false;
        }
        m_req.paramMask = P_MASK_KEYONLY;
    }

    inline void cmdConnect()
    {
        if(m_req.keyNum == LG_SUBOP_ASSOCIATE)
        {
            clientID* cid = (clientID*)m_req.keybuf;
            cid->con->addref();
            setCon(cid->con);
        }
        else if ((m_req.keyNum == LG_SUBOP_CONNECT) ||
            (m_req.keyNum == LG_SUBOP_NEWCONNECT))
        {
            if (con()) disconnect();
            bnet::connection* c = 
                    connect((m_req.keyNum == LG_SUBOP_NEWCONNECT));
            if (c)
            {
                if (getServerCharsetIndex() != -1)
                {
                    buildDualChasetKeybuf();
                    m_disconnected = false;
                }
            }
        }
        else if (m_req.keyNum == LG_SUBOP_DISCONNECT)
        {
            if (con())
                m_logout = true;
            else if (m_op == TD_CONNECT)
                m_preResult = 1;
        }else if (m_req.keyNum == LG_SUBOP_DISCONNECT_EX)
        {
            if (con())
                con()->cleanup();
        }else
        	 m_preResult = STATUS_NOSUPPORT_OP;
            
        m_req.paramMask = P_MASK_KEYONLY;
    }

    inline ushort_td execute()
    {
        if (result() == 0)
        {
            bnet::connection* c = con();
            if (!c)
                m_preResult = ERROR_TD_NOT_CONNECTED;
            else
            {
                char* p = c->sendBuffer(m_req.sendLenEstimate());
                unsigned int size = m_req.serialize(p);
                short stat = 0;
                if ((m_req.paramMask & P_MASK_BLOBBODY) && m_blobBuffer.blobs())
                    size = m_req.serializeBlobBody(
                        &m_blobBuffer, p, size, c->sendBufferSize(),
                        c->optionalBuffers(), stat);
                if (stat == 0)
                {
                    bool ex =  (m_req.paramMask & P_MASK_EX_SENDLEN) != 0;
                    if (c->queryFunction(CONNECTION_FUNCTION_DIRECT_READ) && ex)
                    {
                        c->setDirectReadHandler(&m_req);
                        p = c->asyncWriteRead(size);
                        c->setDirectReadHandler(NULL);
                        if (c->error()) return errorCode(c->error());
                    }else
                    {
                        if (m_req.paramMask & P_MASK_DATALEN)
                            c->setReadBufferSizeIf(*m_req.datalen);
                        p = c->asyncWriteRead(size);
                        if (c->error()) return errorCode(c->error());
                        m_req.parse(p, ex);
                    }
                }
                else
                    m_req.result = stat;
                if (m_logout || (m_connecting && m_req.result))
                    disconnect();
                m_preResult = m_req.result;
                m_connecting = false;
            }
        }
        return result();
    }

    inline ushort_td getBlobBuffer(const blobHeader** data)
    {
        *data = m_req.blobHeader;
        return *data ? 0 : 1;
    }

    inline ushort_td addBlob(const blob* data, bool endRow)
    {
        if (endRow)
        {
            if (m_blobBuffer.fieldCount() == 0)
                m_blobBuffer.setFieldCount((unsigned int)m_blobBuffer.blobs());
            return 0;
        }
        if (data == NULL)
            m_blobBuffer.clear();
        else
            m_blobBuffer.addBlob(*data);
        return 0;
    }
    int getServerCharsetIndex();

    bool buildDualChasetKeybuf();
};

#define USETLS // USE TLS ALL

#ifdef USETLS
extern tls_key g_tlsiID;
#else
extern __THREAD client* __THREAD_BCB g_client;
#endif

inline client* getClientThread()
{
#ifdef USETLS
    client* p = (client*)tls_getspecific(g_tlsiID);
    if (p == NULL)
    {
        p = new client();
        tls_setspecific(g_tlsiID, p);
    }
    return p;
#else
    if (g_client == NULL)
        g_client = new client();
    return g_client;
#endif
}

inline void setClientThread(client* v)
{
#ifdef USETLS
    if (v == NULL)
    {
        client* p = (client*)tls_getspecific(g_tlsiID);
        delete p;
    }
    tls_setspecific(g_tlsiID, v);
#else
    if (v == NULL)
        delete g_client;
    g_client = v;
#endif
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
