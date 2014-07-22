/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
#include <bzs/env/tstring.h>
#pragma hdrstop

//#define BOOST_THREAD_BUILD_LIB
#if (__BCPLUSPLUS__  && _WIN64)
#	define BOOST_ALL_NO_LIB
#endif

#include "nsDatabase.h"
#include "sharedData.h"
#include "nsTable.h"
#include "stringConverter.h"
#include <sys/stat.h>
#include <stdio.h>
#include <boost/thread/mutex.hpp>

#ifdef LINUX
#include <dlfcn.h>
#include <cstddef>
#include <bzs/env/crosscompile.h>
typedef void* HANDLE;
typedef void* HINSTANCE;
#endif

#pragma package(smart_init)


#if (__BCPLUSPLUS__)
#   ifdef _WIN64
#       ifdef _RTLDLL
#	    	pragma comment(lib, "boost_filesystem-bcb64-mt-1_50.a")
#	    	pragma comment(lib, "boost_system-bcb64-mt-1_50.a")
#		else
#	    	pragma comment(lib, "libboost_filesystem-bcb64-mt-s-1_50.a")
#	    	pragma comment(lib, "libboost_system-bcb64-mt-s-1_50.a")
#		endif
#   else
#       ifdef _RTLDLL
#	    	pragma comment(lib, "boost_system-bcb-mt-1_39.lib")
#	    	pragma comment(lib, "boost_filesystem-bcb-mt-1_39.lib")
#	    	pragma comment(lib, "boost_thread-bcb-mt-1_39.lib")
#		else
#	    	pragma comment(lib, "libboost_system-bcb-mt-s-1_39.lib")
#	    	pragma comment(lib, "libboost_filesystem-bcb-mt-s-1_39.lib")
#	    	pragma comment(lib, "libboost_thread-bcb-mt-s-1_39.lib")
#		endif
#   endif
#endif


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
extern EnginsFunc engins;
unsigned int g_lastTrnTime = 0;
unsigned int nsdatabase::m_execCodepage = GetACP();


PACKAGE void registEnginsPtr(EnginsFunc func) {engins = func;}

BTRCALLID_PTR BTRCALLIDX = NULL;
BTRCALLID_PTR MYTICALLID = NULL;

HANDLE hBtrvDLL = NULL;
HANDLE hTrsdDLL = NULL;

#ifdef LINUX
#define LIB_PREFIX "lib"
#else
#define LIB_PREFIX
#endif

BTRCALLID_PTR getTrnsctdEntryPoint()
{
	if (MYTICALLID)
		return MYTICALLID;
	

	if (hTrsdDLL == NULL)
		hTrsdDLL = LoadLibraryA(LIB_PREFIX TDCLC_LIBNAME);

	if (hTrsdDLL)
		MYTICALLID = (BTRCALLID_PTR)GetProcAddress((HINSTANCE)hTrsdDLL, "BTRCALLID");
	return MYTICALLID;
}

BTRCALLID_PTR getBtrvEntryPoint() {return BTRCALLIDX;}

int smartLoadLibrary()
{
	int ret = 0;
	if (hBtrvDLL == NULL)
		hBtrvDLL = LoadLibrary(_T("W3Btrv7"));

	if (hBtrvDLL == NULL)
		hBtrvDLL = LoadLibrary(_T("WBtrv32"));

	if (hBtrvDLL)
		BTRCALLIDX = (BTRCALLID_PTR)GetProcAddress((HINSTANCE)hBtrvDLL, "BTRCALLID");
	if (BTRCALLIDX)
		ret = 1;
	MYTICALLID = getTrnsctdEntryPoint();
	if (MYTICALLID)
		ret = 2;
	return ret;
}

void smartFreeLibrary()
{
	if (hBtrvDLL)
		FreeLibrary((HINSTANCE)hBtrvDLL);

	if (hTrsdDLL)
		FreeLibrary((HINSTANCE)hTrsdDLL);
	hBtrvDLL = NULL;
	hTrsdDLL = NULL;
	BTRCALLIDX = NULL;
	MYTICALLID = NULL;
}

struct nsdbimpl
{
	int refCount;
	int tranCount;
	unsigned short id;
	short snapShotCount;
	nstable* tables[nsdatabase::maxtables];
	uchar_td clientID[16];
	uchar_td* cid() {return clientID;}
	_TCHAR bdfPath[MAX_PATH];
	short tableCount;
	short lockWaitCount;
	short lockWaitTime;
	bool uriMode;
	bool uselongFilename;
	bool localSharing;

	nsdbimpl() : refCount(1), snapShotCount(0), tranCount(0), uselongFilename(false), tableCount(0),
		lockWaitCount(10), lockWaitTime(100), localSharing(false), uriMode(false)
	{

	}

	void setId(unsigned short id_)
	{
		id = id_;

		// make client id
		memset(clientID, 0, 12);
		clientID[12] = 'G';
		clientID[13] = 'X';
		memcpy(&clientID[14], &id, 2);
		bdfPath[0] = 0x00;
	}

	nsdbimpl& operator= (const nsdbimpl & rt)
	{
		if (&rt != this)
		{
			lockWaitCount = rt.lockWaitCount;
			lockWaitTime = rt.lockWaitCount;
			uselongFilename = rt.uselongFilename;
			uriMode = rt.uriMode;
		}
		return *this;
	}
   
};

boost::mutex g_mutex;

nsdatabase::nsdatabase() : m_stat(0)
{
	
	int type = 0;
	if (hBtrvDLL == 0x00)
		type = smartLoadLibrary();

	m_btrcallid = getBtrvEntryPoint();
	if (m_btrcallid == NULL)
		m_btrcallid = getTrnsctdEntryPoint();
	if (!m_btrcallid)
		nstable::throwError(_T("Can't load C Interface library"), ERROR_LOAD_CLIBRARY);

	m_nsimpl = new nsdbimpl();
	if ((type == 2) || MYTICALLID)
		setUseLongFilename(true);
	else
	{
		btrVersions v;
		memset(&v, 0, sizeof(btrVersions));
		uchar_td posblk[POS_BLOCK_SIZE] = {0x00};
		getBtrVersion(&v, posblk);
		if ((v.versions[1].majorVersion >= 9) || (v.versions[0].majorVersion >= 9))
			setUseLongFilename(true);
	}

	for (int i = 0; i < maxtables; i++)
		m_nsimpl->tables[i] = NULL;

	boost::mutex::scoped_lock lck(g_mutex);
	// serach empty
	int i;
	for (i = 0; i < MAX_BTRENGIN; i++)
		if (engins()[i] == NULL)
			break;

	engins()[i] = this;

	// no empty
	if (MAX_BTRENGIN == i)
	{
		m_stat = -1;
		return;
	}
	m_nsimpl->setId((unsigned short)i);

}

int nsdatabase::refCount()const {return m_nsimpl->refCount;}

void nsdatabase::addref()
{
	++m_nsimpl->refCount;
}

void nsdatabase::release()
{
	if (--m_nsimpl->refCount == 0)
		delete this;
}

nsdatabase::~nsdatabase()
{
	reset();

	boost::mutex::scoped_lock lck(g_mutex);
	if (m_nsimpl->id != 0)
		engins()[m_nsimpl->id] = NULL;
	delete m_nsimpl;
#ifdef _WIN32
	OutputDebugString(_T("delete database\n"));
#endif
}

nsdatabase* nsdatabase::clone() const
{
	nsdatabase* p = new nsdatabase();
	*p = *this;
	return p;
}

nsdatabase& nsdatabase:: operator = (const nsdatabase & rt)
{
	if (&rt != this)
	{
		*m_nsimpl = *rt.m_nsimpl;
		setUri(rt.uri());
		m_btrcallid = rt.m_btrcallid;
	}
	return *this;
}

int nsdatabase::enableTrn() const {return m_nsimpl->tranCount;}

short nsdatabase::stat() const {return m_stat;}

uchar_td* nsdatabase::clientID() const {return m_nsimpl->clientID;}

short nsdatabase::openTableCount() const {return m_nsimpl->tableCount;}

_TCHAR* nsdatabase::uri() const {return m_nsimpl->bdfPath;}

bool nsdatabase::uriMode() const {return m_nsimpl->uriMode;}

nstable** nsdatabase::tables() {return m_nsimpl->tables;}

short nsdatabase::lockWaitCount() const {return m_nsimpl->lockWaitCount;}

void nsdatabase::setLockWaitCount(short v) {m_nsimpl->lockWaitCount = v;}

short nsdatabase::lockWaitTime() const {return m_nsimpl->lockWaitTime;}

void nsdatabase::setLockWaitTime(short v) {m_nsimpl->lockWaitTime = v;}

bool nsdatabase::localSharing() const {return m_nsimpl->localSharing;}

void nsdatabase::setLocalSharing(bool v) {m_nsimpl->localSharing = v;}



bool nsdatabase::setUri(const _TCHAR* Path)
{
	_TCHAR buf[MAX_PATH];

#ifdef _WIN32
	_TCHAR* lpFilePart;
	if (useLongFilename() == false)
	{
		GetFullPathName(Path, MAX_PATH, buf, &lpFilePart);
		GetShortPathName(buf, m_nsimpl->bdfPath, MAX_PATH);
		_tcsmupr((_TUCHAR*)m_nsimpl->bdfPath);
	}
	else
#endif
		_tcscpy(m_nsimpl->bdfPath, Path);

	m_nsimpl->uriMode = false;
	if (_tcsstr(m_nsimpl->bdfPath, _T("btrv://")) || _tcsstr(m_nsimpl->bdfPath, _T("tdap://")))
		m_nsimpl->uriMode = true;
#ifdef _WIN32
	else
	{
		struct _stat statbuf;
		if (_tstat(m_nsimpl->bdfPath, &statbuf) == -1)
			return false;
	}
#endif
	return true;
}

void nsdatabase::createTable(fileSpec *pfs, uint_td len, const _TCHAR *pFullPath, short_td mode)
{
	_TCHAR buf[MAX_PATH];
	_TCHAR posblk[128] = {0x00};
#ifdef _WIN32
	if ((useLongFilename() == false) && _tcsstr(pFullPath, _T(" ")))
	{
		GetShortPathName(pFullPath, buf, MAX_PATH);
	}
	else
#endif
	{
		_tcscpy(buf, pFullPath);
	}
	// tdap
	if (isTransactdUri(buf))
	{
		if (setUseTransactd() == false)
			return;
	}

	char buf2[MAX_PATH]={0x00};;
	const char* p = toServerUri(buf2, MAX_PATH, buf, isUseTransactd());

	m_stat = m_btrcallid(TD_CREATETABLE, posblk, pfs, &len, (void*)p, (uchar_td)strlen(p),
			(char_td)mode, m_nsimpl->clientID);
}

void nsdatabase::dropTable(const _TCHAR* pFullPath)
{
	_TCHAR buf[MAX_PATH];
	_TCHAR posblk[128] = {0x00};
#ifdef _WIN32
	if ((useLongFilename() == false) && _tcsstr(pFullPath, _T(" ")))
	{
		GetShortPathName(pFullPath, buf, MAX_PATH);
	}
	else
#endif
	{
		_tcscpy(buf, pFullPath);
	}
	// tdap
	if (isTransactdUri(buf))
	{
		if (setUseTransactd() == false)
			return;
	}

	char buf2[MAX_PATH]={0x00};
	const char* p = toServerUri(buf2, MAX_PATH, buf, isUseTransactd());


	m_stat = m_btrcallid(TD_CREATETABLE, posblk, NULL, NULL, (void*)p, (uchar_td)strlen(p)+1, CR_SUBOP_DROP,
		m_nsimpl->clientID);
}

void nsdatabase::swapTablename(const _TCHAR* Name1, const _TCHAR* Name2)
{
	_TCHAR posblk[128] = {0x00};

	char buf1[MAX_PATH]={0x00};
	char buf2[MAX_PATH]={0x00};
	const char* p = toServerUri(buf1, MAX_PATH, Name1, isUseTransactd());
	const char* p2 = toServerUri(buf2, MAX_PATH, Name2, isUseTransactd());
	uint_td len = (uint_td)strlen(p);

	m_stat = m_btrcallid(TD_CREATETABLE, posblk, (void*)p, &len, (void*)p2, (uchar_td)strlen(p2),
		CR_SUBOP_SWAPNAME, m_nsimpl->clientID);
}

void nsdatabase::rename(const _TCHAR* pFullPath, const _TCHAR* newName)
{
	_TCHAR buf[MAX_PATH];
	_TCHAR posblk[128] = {0x00};
#ifdef _WIN32
	if ((useLongFilename() == false) && _tcsstr(pFullPath, _T(" ")))
	{
		GetShortPathName(pFullPath, buf, MAX_PATH);
	}
	else
#endif
	{
		_tcscpy(buf, pFullPath);
	}
	if (isTransactdUri(buf))
	{
		if (setUseTransactd() == false)
			return;
	}

	char buf2[MAX_PATH]={0x00};
	const char* p = toServerUri(buf2, MAX_PATH, buf, isUseTransactd());
	uint_td len = (uint_td)strlen(p);



	char bufNew[MAX_PATH]={0x00};
#ifdef _WIN32
	if ((useLongFilename() == false) && _tcsstr(newName, _T(" ")))
		GetShortPathName(newName, buf, MAX_PATH);
	else
#endif
		_tcscpy(buf, newName);
	toServerUri(bufNew, MAX_PATH, newName, isUseTransactd());



	m_stat = m_btrcallid(TD_CREATETABLE, posblk, (void*)p, &len, (void*)bufNew, (uchar_td)strlen(bufNew),
		CR_SUBOP_RENAME, m_nsimpl->clientID);
}

void nsdatabase::registerTable(nstable* tb)
{
	int i;
	for (i = 0; i < maxtables; i++)
	{
		if (m_nsimpl->tables[i] == NULL)
		{
			m_nsimpl->tables[i] = tb;
			m_nsimpl->tableCount++;
			break;
		}
	}
}

void nsdatabase::unregisterTable(nstable* table)
{
	int i;
	for (i = 0; i < maxtables; i++)
	{
		if (m_nsimpl->tables[i] == table)
		{
			m_nsimpl->tables[i] = NULL;
			m_nsimpl->tableCount--;
			break;
		}
	}
}


void nsdatabase::reset()
{
	int i;

	if (m_nsimpl->tranCount)
	{
#ifdef _WIN32
	#ifdef LIB_TDCLCPP
			int ret = MessageBox(NULL, _T("Is an uncompleted transaction aborted?"), NULL, 33);
	#else
			int ret = 2;
	#endif
#else
		int ret = 2;
#endif

		m_nsimpl->tranCount = 1;
		if (ret == 1)
			abortTrn();
		else
			endTrn();
	}

	for (i = 0; i < maxtables; i++)
	{
		if (m_nsimpl->tables[i] != NULL)
		{
			m_nsimpl->tables[i]->destroy();
			m_nsimpl->tables[i] = NULL;
		}
	}
	m_nsimpl->lockWaitCount = 10;
	m_nsimpl->lockWaitTime = 200;
	m_nsimpl->tableCount = 0;
	m_nsimpl->bdfPath[0] = 0x00;
	if (m_btrcallid)
	{
		m_stat = m_btrcallid(TD_RESET_CLIENT, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);
		m_stat = m_btrcallid(TD_STOP_ENGINE, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);
		if (m_stat == ERROR_TD_NOT_CONNECTED)
			m_stat = STATUS_SUCCESS;
	}
	if (getBtrvEntryPoint())
		m_btrcallid = getBtrvEntryPoint();

}

void nsdatabase::beginSnapshot()
{
	if (m_nsimpl->snapShotCount == 0)
		m_stat = m_btrcallid(TD_BEGIN_SHAPSHOT, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);
	m_nsimpl->snapShotCount++;
}

void nsdatabase::endSnapshot()
{
	m_nsimpl->snapShotCount--;
	if (m_nsimpl->snapShotCount == 0)
		m_stat = m_btrcallid(TD_END_SNAPSHOT, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);

}

void nsdatabase::beginTrn(short BIAS)
{
	if (m_nsimpl->tranCount == 0)
	{
		m_stat = m_btrcallid((ushort_td)(BIAS + TD_BEGIN_TRANSACTION), NULL, NULL, NULL, NULL, 0, 0,
			m_nsimpl->clientID);
		if (m_stat == 0)
			m_nsimpl->tranCount++;
	}else
		m_nsimpl->tranCount++;
}

void nsdatabase::endTrn()
{

	m_nsimpl->tranCount--;
	if (m_nsimpl->tranCount == 0)
	{
		m_stat = m_btrcallid(TD_END_TRANSACTION, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);

#ifdef _WIN32
		g_lastTrnTime = GetTickCount();
#endif
	}
	if (m_nsimpl->tranCount < 0)
		m_nsimpl->tranCount = 0;
}

void nsdatabase::abortTrn()
{
	m_stat = m_btrcallid(TD_ABORT_TRANSACTION, NULL, NULL, NULL, NULL, 0, 0, m_nsimpl->clientID);

	m_nsimpl->tranCount = 0;
#ifdef _WIN32
	g_lastTrnTime = GetTickCount();
#endif
}

short_td nsdatabase::tdapErr(HWND hWnd, _TCHAR* retbuf) {
	return nstable::tdapErr(hWnd, m_stat, _T("Engin"), retbuf);}

void nsdatabase::getBtrVersion(btrVersions* Vers, uchar_td* posblk)
{

	uchar_td posblkTmp[128] = {0x00};
	if (posblk == NULL)
		posblk = posblkTmp;

	uint_td datalen = sizeof(btrVersions);

	m_stat = m_btrcallid(TD_VERSION, posblk, Vers, &datalen, NULL, 0, 0, m_nsimpl->clientID);
	{
		bool remote = false;
		if (uriMode())
			remote = true;
#ifdef _WIN32
		else if (_tcsstr(m_nsimpl->bdfPath, PSEPARATOR PSEPARATOR) == m_nsimpl->bdfPath)
			remote = true;
		else
		{
			_TCHAR drive[3] = {0x00};
			_tcsncpy(drive, m_nsimpl->bdfPath, 2);
			if (DRIVE_REMOTE == GetDriveType(drive))
				remote = true;
		}
#endif
		if (remote)
		{
			//faile shareing
			if (datalen / 5 == 2)
			{
				Vers->versions[2] = Vers->versions[1];
				Vers->versions[2].type = 'F';
			}
		}
		else
			memset(&Vers->versions[2], 0, sizeof(btrVersion));
	}

}

bool nsdatabase::useLongFilename() {return m_nsimpl->uselongFilename;}

void nsdatabase::setUseLongFilename(bool value) {m_nsimpl->uselongFilename = value;}

bool nsdatabase::setUseTransactd()
{
	m_btrcallid = getTrnsctdEntryPoint();
	if (m_btrcallid == NULL)
		m_stat = STATUS_REQUESTER_DEACTIVE; // can not load db engin;
	else
	{
		m_nsimpl->uriMode = true;
		setLockWaitCount(0);
		setLockWaitTime(0);

	}
	return (m_btrcallid != NULL);
}

bool nsdatabase::isTransactdUri(const _TCHAR* uri) {return (_tcsstr(uri, _T("tdap://")) != NULL);

}

bool nsdatabase::isUseTransactd() {return (m_btrcallid == getTrnsctdEntryPoint());}

void nsdatabase::readDatabaseDirectory(_TCHAR* retBuf, uchar_td buflen)
{
	// keynum is drive name A=1 B=2 C=3 0=default
	char tmp[128];
	m_stat = m_btrcallid(TD_GETDIRECTORY, NULL, NULL, NULL, tmp, 128, 0, m_nsimpl->clientID);
	toTChar(retBuf, tmp, buflen);
}

bool nsdatabase::connect(const _TCHAR* URI, bool newConnection)
{
	if (isTransactdUri(URI))
		if (!setUseTransactd())
			return false;

	uint_td datalen = 0;

	char uri_a[MAX_PATH]={0x00};
	const char* p = toServerUri(uri_a, MAX_PATH, URI, isUseTransactd());
	char_td keyNum = newConnection ? 3 : 0;
	m_stat = m_btrcallid(TD_CONNECT, NULL, NULL, &datalen, (void*)p, (keylen_td)(strlen(p) + 1),
		keyNum, clientID());
	if (m_stat)
		return false;
	return true;
}

bool nsdatabase::disconnect(const _TCHAR* URI)
{
	if (isTransactdUri(URI))
		if (!setUseTransactd())
			return false;
	uint_td datalen = 0;
	char uri_a[MAX_PATH];
	const char* p = toServerUri(uri_a, MAX_PATH, URI, isUseTransactd());
	m_stat = m_btrcallid(TD_CONNECT, NULL, NULL, &datalen, (void*)p, (keylen_td)(strlen(p) + 1), 1,
		clientID());
	if (m_stat)
		return false;
	return true;

}

bool nsdatabase::trnsactionFlushWaitStatus()
{
	bool ret = false;
#ifdef  _WIN32
	if (g_lastTrnTime)
		ret = ((GetTickCount() - g_lastTrnTime) < 8000);
	else
#endif
		g_lastTrnTime = 0;
	return ret;
}

void nsdatabase::setExecCodePage(unsigned int codepage)
{
	m_execCodepage = codepage;
}

unsigned int nsdatabase::execCodePage()
{
	return m_execCodepage;
}

const char* nsdatabase::toServerUri(char* buf, int buflen, const _TCHAR* src, bool trd)
{
	#ifdef _UNICODE
	if (trd)
	{
		stringConverter cv(CP_UTF8, GetACP());
		cv.convert(buf, buflen, src, strlen_t(src));
		return buf;
	}
	#endif
	return toChar(buf, src, buflen);

}

DLLUNLOADCALLBACK_PTR nsdatabase::getDllUnloadCallbackFunc()
{
	if (hTrsdDLL == NULL)
		hTrsdDLL = LoadLibraryA(LIB_PREFIX TDCLC_LIBNAME);
	if (hTrsdDLL)
		return (DLLUNLOADCALLBACK_PTR)GetProcAddress((HINSTANCE)hTrsdDLL, "CallbackRegist");
	return NULL;
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
