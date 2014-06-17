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

#define MYSQL_VERSION_ID		50609
#define PIPENAME "Transactd"

#ifdef __BCPLUSPLUS__
#   ifdef _WIN64
#		pragma comment(lib, "libboost_system-bcb64-mt-s-1_50.a")
#		pragma comment(lib, "libboost_thread-bcb64-mt-s-1_50.a")
#   else
#       pragma comment(lib, "libboost_system-bcb-mt-s-1_39.lib")
#       pragma comment(lib, "libboost_thread-bcb-mt-s-1_39.lib")
#   endif
#endif


void writeErrorLog(int err, const char* msg);


#ifdef USETLS
tls_key g_tlsiID1;
#else
__THREAD clientID __THREAD_BCB g_cid;
__THREAD bool __THREAD_BCB g_initCid = false;
#endif




#ifdef _WIN32

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		#ifdef USETLS
		if ((g_tlsiID = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		if ((g_tlsiID1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		#endif

		m_cons = new  connections(PIPENAME);
	}else if(reason == DLL_THREAD_DETACH)
	{
		#ifdef USETLS
		delete (bzs::db::protocol::tdap::client::client*)tls_getspecific(g_tlsiID);
		tls_setspecific(g_tlsiID1, 0);
		#else
		delete g_client;
		g_client = NULL;
		#endif
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		delete m_cons;
		m_cons=NULL;

		#ifdef USETLS
		TlsFree(g_tlsiID);
		TlsFree(g_tlsiID1);

		#endif
	}
	return TRUE;
}
#else

void __attribute__ ((constructor)) onLoadLibrary(void);
void __attribute__ ((destructor)) onUnloadLibrary(void);

void onLoadLibrary(void)
{
	m_cons = new  connections(PIPENAME);
	#ifdef USETLS
	pthread_key_create(&g_tlsiID, NULL);
	pthread_key_create(&g_tlsiID1, NULL);
	#endif

}
void onUnloadLibrary(void)
{
	delete m_cons;
	m_cons=NULL;
	#ifdef USETLS
	pthread_key_delete(g_tlsiID); 
	pthread_key_delete(g_tlsiID1);
	#endif
}
#endif

extern "C" short_td  __STDCALL
	BTRCALLID(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td*   keybuf, keylen_td keylen,
   char_td keyNum, clientID* cid)
{
	bzs::db::protocol::tdap::client::client* client_t = getClientThread();

	short_td ret;
	try
	{

		if ((TD_GET_BLOB_BUF != op) && (TD_ADD_SENDBLOB != op))
			client_t->setParam(op, pbk,data,datalen,keybuf,keylen,keyNum,cid);

		if (client_t->stop_if()) 
		{
			client_t->cleanup();
			return 0;
		}
		if (cid==NULL) return 1;

		switch(op)
		{
		case TD_GET_BLOB_BUF:
			return client_t->getBlobBuffer((const bzs::db::blobHeader**)data);
		case TD_ADD_SENDBLOB:
			return client_t->addBlob((const bzs::db::blob*)data, (keyNum == TD_ASBLOB_ENDROW));
		case TD_OPENTABLE:
		case TD_CREATETABLE:
			client_t->connect();
			if (client_t->result()==0)
			{
				if (op == TD_CREATETABLE)
				{
					if (client_t->readServerCharsetIndex())
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
			client_t->cmdConnect();
			break;
		}
		case TD_STASTISTICS:
			client_t->req().paramMask = P_MASK_DATALEN|P_MASK_KEYBUF|P_MASK_KEYNUM;
			break;
		case TD_RESET_CLIENT: 
			client_t->req().paramMask = P_MASK_KEYONLY;
			break;
		case TD_DROP_INDEX:
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_KEYNUM;
			break;
		case TD_REC_INSERT:
		case TD_INSERT_BULK:
		case TD_REC_UPDATE:
			client_t->req().paramMask = P_MASK_NOKEYBUF|P_MASK_BLOBBODY;
			break;
		case TD_REC_UPDATEATKEY:
			client_t->req().paramMask = P_MASK_ALL|P_MASK_BLOBBODY;
			break;
		case TD_MOVE_BOOKMARK:
		case TD_MOVE_PER:
		case TD_BUILD_INDEX:
			client_t->req().paramMask = P_MASK_NOKEYBUF;
			break;
		case TD_UNLOCK:
		case TD_UPDATE_PART:
			client_t->cleanup();
			return 0;
		case TD_REC_DELETE:
		case TD_CLOSETABLE:
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
			if (op> 50)
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
			if (op> 50)
				client_t->req().paramMask &= ~P_MASK_DATALEN;
			break;
		case TD_KEY_NEXT_MULTI:
		case TD_KEY_PREV_MULTI:
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN|P_MASK_EX_SENDLEN|P_MASK_KEYNUM;
			break;
		case TD_KEY_GE_NEXT_MULTI:
		case TD_KEY_LE_PREV_MULTI:
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN|P_MASK_EX_SENDLEN|P_MASK_KEYNUM|P_MASK_KEYBUF;
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
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_DATALEN;
			break;
		case TD_POS_NEXT_MULTI:
		case TD_POS_PREV_MULTI:
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_DATA
										|P_MASK_DATALEN|P_MASK_EX_SENDLEN;
			break;
		case TD_KEY_SEEK_MULTI:
			client_t->req().paramMask = P_MASK_POSBLK|P_MASK_DATA|P_MASK_KEYNUM
										|P_MASK_DATALEN|P_MASK_EX_SENDLEN;
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
				client_t->req().paramMask =	P_MASK_DATA|P_MASK_DATALEN;
			}else
				client_t->req().result = STATUS_BUFFERTOOSMALL;
			if (datalen < sizeof(btrVersion)*2)
			{
				client_t->cleanup();
				return 0;
			}
			break;
		}


		}
		short_td ret = client_t->execute();
		client_t->cleanup();
		return ret;
	}
	catch(boost::system::system_error &e)
	{
		switch(e.code().value())
		{
		case 11004:
		case 11001:
			ret= ERROR_TD_HOSTNAME_NOT_FOUND;
			break;
		case 10061:
			ret= ERROR_TD_CONNECTION_FAILURE;
			break;
		case 232:
		case 109:
		case 2:
		case 1:
			ret= ERROR_TD_INVALID_CLINETHOST;
			break;
		default:
			ret = e.code().value() + 20000;
		}
		OutputDebugString(e.what());
		writeErrorLog(ret, e.what());
	}
	catch(bzs::netsvc::client::exception &e)
	{
		OutputDebugString(e.what());
		ret = e.error();
		writeErrorLog(e.error(), e.what());
	}
	catch(std::exception &e)
	{
		ret = 1;
		OutputDebugString(e.what());
		writeErrorLog(30000, e.what());
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
#else
	struct tm tmp;
	date = &tmp;
	localtime_x(date, &now);
#endif	
	sprintf_s(buf, bufsize, "%04d/%02d/%02d %02d:%02d:%02d"
			,date->tm_year + 1900, 	date->tm_mon + 1,date->tm_mday
			,date->tm_hour, date->tm_min, date->tm_sec);
	return buf;
}

void writeErrorLog(int err, const char* msg)
{
	mutex::scoped_lock lck(g_mutex);
	char buf[MAX_PATH];
	char buf2[30];
	#ifdef WIN32
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL,SHGFP_TYPE_CURRENT, buf);
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
	if (p==NULL)
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
	return &g_cid;
#endif	
}


void initCid()
{
#ifndef USETLS
	if (!g_initCid)
	{
		g_initCid = true;
		memset(&g_cid, 0, sizeof(clientID));
		g_cid.id = 1;
		g_cid.aid[0] = 'G';
		g_cid.aid[1] = 'X';
	}
#endif
}


extern "C" short_td __STDCALL
	BTRVID(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td* keybuf, char_td keyNum, clientID* cid)
{
	// If keybuf size is less than 255 byte then it will be buffer overrun.
	// It need know size of key buffer before call.  
	keylen_td keylen = 255; 
	return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, cid);
}

extern "C" short_td __STDCALL
	BTRV(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td* keybuf, char_td keyNum)
{
	initCid();
	return BTRVID(op, pbk, data, datalen, keybuf, keyNum, getCid());
}

extern "C" short_td __STDCALL
	BTRCALL(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td*   keybuf, keylen_td keylen,
   char_td keyNum)
{
	initCid();
	return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, getCid());
}

extern "C" short_td __STDCALL
	BTRCALL32(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td*   keybuf, keylen_td keylen,
   char_td keyNum)
{
	initCid();
	return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, getCid());
}

extern "C" short_td __STDCALL
	BTRCALLID32(ushort_td op, posblk* pbk, void_td* data,
   uint_td* datalen, void_td*   keybuf, keylen_td keylen,
   char_td keyNum, clientID* cid)
{
	return BTRCALLID(op, pbk, data, datalen, keybuf, keylen, keyNum, cid);
}


