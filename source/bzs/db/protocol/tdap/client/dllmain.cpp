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

#include <bzs/db/protocol/tdap/client/client.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/rtl/stl_uty.h>
#include <bzs/env/fileopen.h>
#include <bzs/rtl/datetime.h>
#ifdef WIN32
#include <Shlobj.h>
#include <direct.h>
#endif

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace bzs::netsvc::client;

#define MYSQL_VERSION_ID 50609
#define PIPENAME "Transactd"

#ifdef __BCPLUSPLUS__
#define BZS_LINK_BOOST_SYSTEM
#define BZS_LINK_BOOST_THREAD
#include <bzs/env/boost_bcb_link.h>
#endif

void writeErrorLog(int err, const char* msg);
bool win_thread_pool_shutdown = false;

#ifdef USETLS
tls_key g_tlsiID1;
tls_key g_tlsiID_SC1;

void cleanupClinet(void* p)
{
    delete ((bzs::db::protocol::tdap::client::client*)(p));
}

void cleanupClientID(void* p)
{
    delete ((clientID*)(p));
}

void cleanupWChar(void* p)
{
    delete ((wchar_t*)(p));
}

#else // NOT USETLS
__THREAD clientID __THREAD_BCB g_cid;
__THREAD bool __THREAD_BCB g_initCid = false;
#endif // NOT USETLS

#ifdef _WIN32

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
#ifdef _MSC_VER
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
        //_CrtSetBreakAlloc(155);
#endif

#ifdef USETLS
        if ((g_tlsiID = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        if ((g_tlsiID1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        if ((g_tlsiID_SC1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
#endif

        m_cons = new connections(PIPENAME);
    }
    else if (reason == DLL_THREAD_DETACH)
    {
#ifdef USETLS
        cleanupClinet(tls_getspecific(g_tlsiID));
        cleanupClientID(tls_getspecific(g_tlsiID1));
        cleanupWChar(tls_getspecific(g_tlsiID_SC1));
#else
        delete g_client;
        g_client = NULL;
#endif
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        try
        {
        	delete m_cons;
        }
        catch(...){}
        m_cons = NULL;

#ifdef USETLS
        cleanupClinet(tls_getspecific(g_tlsiID));
        cleanupClientID(tls_getspecific(g_tlsiID1));
        cleanupWChar(tls_getspecific(g_tlsiID_SC1));
        TlsFree(g_tlsiID);
        TlsFree(g_tlsiID1);
        TlsFree(g_tlsiID_SC1);
#endif
#ifdef _MSC_VER
    OutputDebugString(_T("After tdclc DLL_PROCESS_DETACH \n"));
    _CrtDumpMemoryLeaks();
#endif
    }
    return TRUE;
}
#else // NOT _WIN32

void __attribute__((constructor)) onLoadLibrary(void);
void __attribute__((destructor)) onUnloadLibrary(void);

#if (__BCPLUSPLUS__ && defined(__APPLE__))
#pragma exit onUnloadLibrary
#pragma startup onLoadLibrary
#endif

void onLoadLibrary(void)
{
    if (m_cons == NULL)
    {
        m_cons = new connections(PIPENAME);
#ifdef USETLS
        pthread_key_create(&g_tlsiID, cleanupClinet);
        pthread_key_create(&g_tlsiID1, cleanupClientID);
        pthread_key_create(&g_tlsiID_SC1, cleanupWChar);
#endif
    }
}

void onUnloadLibrary(void)
{
    if (m_cons)
    {
        delete m_cons;
        m_cons = NULL;
#ifdef USETLS
        pthread_key_delete(g_tlsiID);
        pthread_key_delete(g_tlsiID1);
        pthread_key_delete(g_tlsiID_SC1);
#endif
    }
}
#endif // NOT _WIN32

extern "C" PACKAGE_OSX short_td __STDCALL
    BTRCALLID(ushort_td op, posblk* pbk, void_td* data, uint_td* datalen,
              void_td* keybuf, keylen_td keylen, char_td keyNum, clientID* cid)
{
    bzs::db::protocol::tdap::client::client* client_t = getClientThread();

    short_td ret;
    try
    {
        if ((TD_GET_BLOB_BUF != op) && (TD_ADD_SENDBLOB != op))
        {
            if (cid == NULL) return 1;
            
            client_t->setParam(op, pbk, data, datalen, keybuf, keylen, keyNum,
                               cid);

            if (client_t->stop_if())
            {
                client_t->cleanup();
                return 0;
            }
        }

        op = op % 100;
        switch (op)
        {
        case TD_GET_BLOB_BUF:
            return client_t->getBlobBuffer((const bzs::db::blobHeader**)data);
        case TD_ADD_SENDBLOB:
            return client_t->addBlob((const bzs::db::blob*)data,
                                     (keyNum == TD_ASBLOB_ENDROW));
        case TD_REC_INSERT:
        case TD_INSERT_BULK:
        case TD_REC_UPDATE:
            client_t->req().paramMask = P_MASK_NOKEYBUF | P_MASK_BLOBBODY;
            break;
        case TD_REC_UPDATEATKEY:
            client_t->req().paramMask = P_MASK_ALL | P_MASK_BLOBBODY;
            break;
        case TD_MOVE_BOOKMARK:
        case TD_MOVE_PER:
			client_t->req().paramMask = P_MASK_NOKEYBUF;
            break;
        case TD_UNLOCK:
        case TD_CLOSETABLE:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_KEYNUM;
            break;
        case TD_UPDATE_PART:
            client_t->cleanup();
            return 0;
        case TD_REC_DELETE:
        case TD_CLEAR_OWNERNAME:
        case TD_AUTOMEKE_SCHEMA:
            client_t->req().paramMask = P_MASK_POSBLK;
            break;
        case TD_END_TRANSACTION:
        case TD_BEGIN_TRANSACTION:
        case TD_ABORT_TRANSACTION:
        case TD_BEGIN_SHAPSHOT:
        case TD_END_SNAPSHOT:
            client_t->req().paramMask = 0;
            break;
        case TD_KEY_FIRST:
        case TD_KEY_LAST:
        case TD_KEY_FIRST_KO:
        case TD_KEY_LAST_KO:
            client_t->req().paramMask = P_MASK_POS_LEN_KEY;
            if (op > 50)
                client_t->req().paramMask &= ~P_MASK_DATALEN;
            break;
        case TD_KEY_SEEK:
        case TD_KEY_NEXT:
        case TD_KEY_PREV:
        case TD_KEY_AFTER:
        case TD_KEY_OR_AFTER:
        case TD_KEY_BEFORE:
        case TD_KEY_OR_BEFORE:
        case TD_KEY_NEXT_KO:
        case TD_KEY_PREV_KO:
        case TD_KEY_GT_KO:
        case TD_KEY_GE_KO:
        case TD_KEY_LT_KO:
        case TD_KEY_LE_KO:
        case TD_REC_DELLETEATKEY:
            client_t->req().paramMask = P_MASK_KEYNAVI;
            if (op > 50)
                client_t->req().paramMask &= ~P_MASK_DATALEN;
            break;
        case TD_KEY_NEXT_MULTI:
        case TD_KEY_PREV_MULTI:
        case TD_KEY_SEEK_MULTI:
        case TD_FILTER_PREPARE:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_DATA |
                                        P_MASK_DATALEN | P_MASK_EX_SENDLEN |
                                        P_MASK_KEYNUM;
            break;
        case TD_KEY_GE_NEXT_MULTI:
        case TD_KEY_LE_PREV_MULTI:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_DATA |
                                        P_MASK_DATALEN | P_MASK_EX_SENDLEN |
                                        P_MASK_KEYNUM | P_MASK_KEYBUF;
            break;
        case TD_GET_PER:
        case TD_SET_OWNERNAME:
        case TD_TABLE_INFO:
            client_t->req().paramMask = P_MASK_ALL;
            break;
        case TD_BOOKMARK:
        case TD_POS_FIRST:
        case TD_POS_LAST:
        case TD_POS_NEXT:
        case TD_POS_PREV:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_DATALEN;
            break;
        case TD_POS_NEXT_MULTI:
        case TD_POS_PREV_MULTI:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_DATA |
                                        P_MASK_DATALEN | P_MASK_EX_SENDLEN;
            break;
        case TD_GETDIRECTORY:
        case TD_SETDIRECTORY:
            break;
        case TD_VERSION:
        {
            ushort_td datalen = *client_t->req().datalen;
            if (datalen >= sizeof(btrVersion))
            {
                btrVersion& v = (btrVersion&)*((char*)client_t->req().data);
                v.majorVersion = atoi(CPP_INTERFACE_VER_MAJOR);
                v.minorVersion = atoi(CPP_INTERFACE_VER_MINOR);
                v.type = 'N';
                client_t->req().paramMask = P_MASK_DATA | P_MASK_DATALEN;
            }
            else
                client_t->req().result = STATUS_BUFFERTOOSMALL;
            if (datalen < sizeof(btrVersion) * 2)
            {
                client_t->cleanup();
                return 0;
            }
            break;
        }
        case TD_OPENTABLE:
        case TD_CREATETABLE:
            client_t->connect();
            if (client_t->result() == 0)
            {
                if (op == TD_CREATETABLE)
                {
                    if (client_t->getServerCharsetIndex() != -1)
                        client_t->create();
                    else
                    {
                        client_t->cleanup();
                        return 1;
                    }
                }
                else if (op == TD_OPENTABLE)
                    client_t->req().paramMask = P_MASK_ALL;
                if (!client_t->buildDualChasetKeybuf())
                {
                    client_t->cleanup();
                    return SERVER_CLIENT_NOT_COMPATIBLE;
                }
            }
            break;
        case TD_CONNECT:
        {
            if (client_t->req().keyNum == LG_SUBOP_RECONNECT)
                client_t->reconnect();
            else
                client_t->cmdConnect();
            if (client_t->req().keyNum == LG_SUBOP_DISCONNECT_EX)
                return 0;
            break;
        }
        case TD_RECONNECT:
        {
            client_t->req().paramMask = P_MASK_ALL;
            break;
        }
        case TD_STASTISTICS:
            client_t->req().paramMask =
                P_MASK_DATALEN | P_MASK_KEYBUF | P_MASK_KEYNUM;
            break;
        case TD_RESET_CLIENT:
            client_t->req().paramMask = P_MASK_KEYONLY;
            break;
        case TD_BUILD_INDEX:
            client_t->createIndex();
            break;
        case TD_DROP_INDEX:
            client_t->req().paramMask = P_MASK_POSBLK | P_MASK_KEYNUM;
            break;
        case TD_ACL_RELOAD:
            client_t->req().paramMask = 0;
            break;
        }
        short_td ret = client_t->execute();
        client_t->cleanup();
        return ret;
    }
    catch (boost::system::system_error& e)
    {
        ret = errorCode(e.code());
        OutputDebugString(e.what());
        char tmp[512];
        sprintf_s(tmp, 512, "%d %s", e.code().value(), e.what());
        writeErrorLog(ret, tmp);
    }
    catch (bzs::netsvc::client::exception& e)
    {
        OutputDebugString(e.what());
        ret = e.error();
        writeErrorLog(e.error(), e.what());
    }
    catch (std::exception& e)
    {
        ret = ERROR_TD_C_CLIENT_UNKNOWN;
        OutputDebugString(e.what());
        writeErrorLog(ret, e.what());
    }
    catch (...)
    {
        ret = ERROR_TD_C_CLIENT_UNKNOWN;
        writeErrorLog(ret, "Unknown error");
    }
    return ret;
}

mutex g_mutex;

const char* dateTimeStr(char* buf, unsigned int bufsize)
{
    struct tm* date;
    time_t now;
    time(&now);
#ifdef __MINGW32__
    date = localtime(&now);
#else // NOT __MINGW32__
    struct tm tmp;
    date = &tmp;
    localtime_x(date, &now);
#endif // NOT __MINGW32__
    sprintf_s(buf, bufsize, "%04d/%02d/%02d %02d:%02d:%02d",
              date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
              date->tm_hour, date->tm_min, date->tm_sec);
    return buf;
}

void writeErrorLog(int err, const char* msg)
{
    mutex::scoped_lock lck(g_mutex);
    char buf[MAX_PATH];
    char buf2[30];
#ifdef WIN32
    SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
    strcat_s(buf, MAX_PATH, PSEPARATOR "BizStation");
    _mkdir(buf);
    strcat_s(buf, MAX_PATH, PSEPARATOR "Transactd");
    _mkdir(buf);
#else
    strcpy_s(buf, MAX_PATH, "/var/log");
#endif

    strcat_s(buf, MAX_PATH, PSEPARATOR "trnsctcl_error.log");
    FILE* fp = fileOpen(buf, "a+");
    if (fp)
    {
        fprintf(fp, "%s Error No.%d %s\n", dateTimeStr(buf2, 30), err, msg);
        fclose(fp);
    }
}

inline clientID* getCid()
{
#ifdef USETLS
    clientID* p = (clientID*)tls_getspecific(g_tlsiID1);
    if (p == NULL)
    {
        clientID* p = new clientID();
        memset(p, 0, sizeof(clientID));
        p->id = 1;
        p->aid[0] = 'G';
        p->aid[1] = 'X';
        tls_setspecific(g_tlsiID1, p);
    }
    return p;
#else
    if (!g_initCid)
    {
        g_initCid = true;
        memset(&g_cid, 0, sizeof(clientID));
        g_cid.id = 1;
        g_cid.aid[0] = 'G';
        g_cid.aid[1] = 'X';
    }
    return &g_cid;
#endif
}

extern "C" PACKAGE_OSX short_td __STDCALL BTRVID(ushort_td op, posblk* pbk, void_td* data,
                                       uint_td* datalen, void_td* keybuf,
                                       char_td keyNum, clientID* cid)
{
    // If keybuf size is less than 255 byte then it will be buffer overrun.
    // It need know size of key buffer before call.
    keylen_td keylen = 255;
    return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, cid);
}

extern "C" PACKAGE_OSX short_td __STDCALL BTRV(ushort_td op, posblk* pbk,
                                               void_td* data, uint_td* datalen,
                                               void_td* keybuf, char_td keyNum)
{
    return BTRVID(op, pbk, data, datalen, keybuf, keyNum, getCid());
}

extern "C" PACKAGE_OSX short_td __STDCALL
    BTRCALL(ushort_td op, posblk* pbk, void_td* data, uint_td* datalen,
            void_td* keybuf, keylen_td keylen, char_td keyNum)
{
    return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, getCid());
}

extern "C" PACKAGE_OSX short_td __STDCALL
    BTRCALL32(ushort_td op, posblk* pbk, void_td* data, uint_td* datalen,
              void_td* keybuf, keylen_td keylen, char_td keyNum)
{
    return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, getCid());
}

extern "C" PACKAGE_OSX short_td __STDCALL
    BTRCALLID32(ushort_td op, posblk* pbk, void_td* data, uint_td* datalen,
                void_td* keybuf, keylen_td keylen, char_td keyNum,
                clientID* cid)
{
    return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, cid);
}

extern "C" PACKAGE_OSX void __STDCALL BeginWinThreadPoolShutdown()
{
    win_thread_pool_shutdown = true;
}
