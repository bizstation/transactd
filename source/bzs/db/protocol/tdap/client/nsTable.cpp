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


#include "nsTable.h"
#include "nsDatabase.h"
#include "bulkInsert.h"

#include <limits.h>
#include <string.h>								// Required for _fstrstr()
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <boost/filesystem.hpp>





#pragma package(smart_init)

/* TODO: To be support internal started transction with original flag.*/
#ifdef __x86_64__ 
#define MEM_FREED_MAGIC_NUMBER (nstimpl*)0x0FEEEFEEEFEEEFEEE
#else
#define MEM_FREED_MAGIC_NUMBER (nstimpl*)0x0FEEEFEEE
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

struct nstimpl
{
    nstimpl():refCount(1), bulkIns(NULL), shared(false), isOpen(false)
    {
        posblk[0] = 0x00;
    }
	int refCount;
    bulkInsert* bulkIns;
    nsdatabase* nsdb;
    percentage_td percentage;
    bookmark_td bookmark;
    _TCHAR uri[MAX_PATH];
    uchar_td posblk[POS_BLOCK_SIZE];
    short tableid;
    bool shared;
    bool isOpen;
};

// -----------------------------------------------------------------
// class nstable
// -----------------------------------------------------------------
extern TCHAR* getErrorMessageLocale(int errorCode, _TCHAR* buf, size_t size);

nstable::nstable(nsdatabase *pbe)
{
    m_impl = new nstimpl();
    m_impl->nsdb = pbe;

	//addref to nsdatabase
	pbe->addref();
    if (pbe->openTableCount() == nsdatabase::maxtables)
    {
        m_stat = STATUS_LMITS_MAX_TABLES;
        m_impl->nsdb = NULL;
        return;
    }
    pbe->registerTable(this);
    m_keylen = 0;
    m_keynum = 0;
    m_pdata = NULL;
    m_keybuf = NULL;
    m_stat = STATUS_SUCCESS;
    m_read = true;
    m_write = true;
    m_insart = true;
    m_delete = true;

}

nstable::~nstable()
{
    if (m_impl->bulkIns)
        delete m_impl->bulkIns;
    close();

	if (!m_impl->shared)
		m_impl->nsdb->unregisterTable(this);
    m_impl->nsdb->internalRelease();
    delete m_impl;
	m_impl = MEM_FREED_MAGIC_NUMBER;
}

TCHAR* nstable::getErrorMessage(int errorCode, _TCHAR* buf, size_t size)
{
    return getErrorMessageLocale(errorCode, buf, size);

}

/*  Key number is set by user.
    KyeNum
    - -1 = Unlock the record that specified by bm in multi records.
    - -2 = Unlock all records
    - { = Unlock single record.
*/
void nstable::unlock(bookmark_td bm)
{
    void* db = m_pdata;
    if (m_keynum == -1)
        m_pdata = &bm;
    m_datalen = 4;
    tdap(TD_UNLOCK);
    m_pdata = db;
}

void nstable::seekByBookmark() {seekByBookmark(m_impl->bookmark);}

const _TCHAR* nstable::uri() const {return m_impl->uri;}

const uchar_td* nstable::posblk() const {return m_impl->posblk;}

nsdatabase* nstable::nsdb() const {return m_impl->nsdb;}

bulkInsert* nstable::bulkIns() const {return m_impl->bulkIns;}

void nstable::setIsOpen(bool v) {m_impl->isOpen = v;}

bool nstable::isOpen() const {return m_impl->isOpen;}

short nstable::tableid() const {return m_impl->tableid;}

void nstable::setTableid(short v) {m_impl->tableid = v;}

void nstable::setShared()
{
    m_impl->shared = true;
    m_impl->nsdb->unregisterTable(this);
}

int nstable::refCount() const
{
	return m_impl->refCount;
}

void nstable::addref(void)
{
	++m_impl->refCount;
}

void nstable::release()
{
	
	/* If before called database::drop() database::close() etc then nstable::destory called.
	   Client cache nstable pointer that is invalid.
	   It test by test method.
	*/
	try
	{
		if (test(this))
		{
			if (--m_impl->refCount == 0)
				delete this;
		}
	}catch(...){}
}

void nstable::destroy()
{
	if (!m_impl->shared)
		delete this;
}

void nstable::doClose()
{
    if (m_impl->isOpen)
    {
        tdap(TD_CLOSETABLE);
        if (m_stat == STATUS_SUCCESS)
            m_impl->isOpen = false;
    }
}

void nstable::stepFirst(ushort_td LockBias)
{
    m_datalen = m_buflen;
    tdap((ushort_td)(TD_POS_FIRST + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::stepLast(ushort_td LockBias)
{
    m_datalen = m_buflen;
    tdap((ushort_td)(TD_POS_LAST + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::stepPrev(ushort_td LockBias)
{
    m_datalen = m_buflen;
    tdap((ushort_td)(TD_POS_PREV + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::stepNext(ushort_td LockBias)
{
    m_datalen = m_buflen;
    tdap((ushort_td)(TD_POS_NEXT + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekFirst(ushort_td LockBias)
{
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap((ushort_td)(TD_KEY_FIRST + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekLast(ushort_td LockBias)
{
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap((ushort_td)(TD_KEY_LAST + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekPrev(ushort_td LockBias)
{
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap((ushort_td)(TD_KEY_PREV + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekNext(ushort_td LockBias)
{
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap((ushort_td)(TD_KEY_NEXT + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seek(ushort_td LockBias)
{
    m_keylen = writeKeyData();
    m_datalen = m_buflen;
    tdap((ushort_td)(TD_KEY_SEEK + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekGreater(bool orEqual, ushort_td LockBias)
{
    m_keylen = writeKeyData();
    m_datalen = m_buflen;
    if (orEqual)
        tdap((ushort_td)(TD_KEY_OR_AFTER + LockBias));
    else
        tdap((ushort_td)(TD_KEY_AFTER + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::seekLessThan(bool orEqual, ushort_td LockBias)
{
    m_keylen = writeKeyData();
    m_datalen = m_buflen;
    if (orEqual)
        tdap((ushort_td)(TD_KEY_OR_BEFORE + LockBias));
    else
        tdap((ushort_td)(TD_KEY_BEFORE + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

void nstable::doOpen(const _TCHAR* name, char_td mode, const _TCHAR* ownerName)
{
    void* svm_keybuf = m_keybuf;
    char_td svm_keynum = m_keynum;
    keylen_td svm_keybuflen = m_keylen;

    if (m_impl->nsdb == NULL)
    {
        m_stat = STATUS_LMITS_MAX_TABLES;
        return;
    }
#ifdef _WIN32
    // if name has space then covert to short path name.
    if ((m_impl->nsdb->useLongFilename() == false) && _tcsstr(name, _T(" ")))
        GetShortPathName(name, m_impl->uri, MAX_PATH);
    else
#endif
        if (m_impl->uri != name)
            _tcscpy_s(m_impl->uri, MAX_PATH, name);

    // for trnasctd
    if (m_impl->nsdb->isTransactdUri(m_impl->uri))
    {
        if (m_impl->nsdb->setUseTransactd() == false)
        {
            m_stat = STATUS_REQUESTER_DEACTIVE;
            return;
        }
    }

    char tmpName[MAX_PATH]={0x00};
    const char* p = nsdatabase::toServerUri(tmpName, MAX_PATH, m_impl->uri, m_impl->nsdb->isUseTransactd());
    m_keybuf = (void*)p;
    m_keylen = (keylen_td)strlen(p) + 1;

    if (m_impl->nsdb->localSharing())
        m_keynum = (char_td)(mode - 64);
    else
        m_keynum = mode;

    char ownerNameBuf[OWNERNAME_SIZE+1] = {0x00};
    if (NULL != ownerName && 0x00 != ownerName[0])
    {
        const char* p = toChar(ownerNameBuf, ownerName, 22);
        m_pdata = (void*)p;
        m_datalen = (uint_td)strlen(p) + 1;
        if (m_datalen > 11)
        {
            m_stat = STATUS_TOO_LONG_OWNERNAME;
            return;
        }

    }
    else
        m_datalen = 0;

    tdap(TD_OPENTABLE);
    if (m_stat == STATUS_SUCCESS)
        m_impl->isOpen = true;
    m_keybuf = svm_keybuf;
    m_keynum = svm_keynum;
    m_keylen = svm_keybuflen;
}

void nstable::doUpdate(eUpdateType type)
{
    int trnCount = nsdb()->enableTrn();
    if (m_write == false)
    {
        m_stat = STATUS_NO_ACR_UPDATE_DELETE;
        return;
    }



    if (onUpdateCheck(type) == false)
        return;

    m_stat = STATUS_SUCCESS;
    int option = onUpdateBefore();
    if (m_stat)
    {
        if (trnCount < nsdb()->enableTrn())
            nsdb()->abortTrn();
        return ;
    }
    char_td keynum = m_keynum;
    m_keylen = m_keybuflen;

    if (type == changeCurrentNcc)
        m_keynum = -1;
    else
        m_keylen = writeKeyData();
    m_datalen = getWriteImageLen();

    if (m_impl->nsdb->isUseTransactd() && (type == changeInKey))
        tdap(TD_REC_UPDATEATKEY);
    else
        tdap(TD_REC_UPDATE);
    m_keynum = keynum;
    onUpdateAfter(option);
}

void nstable::doDel(bool inkey)
{
    if (m_delete == false)
    {
        m_stat = STATUS_NO_ACR_UPDATE_DELETE;
        return;
    }
    if (onDeleteCheck(inkey)==false)
        return;

    m_datalen = m_buflen;
    if (m_impl->nsdb->isUseTransactd() && inkey)
    {
        if (!isUniqeKey(m_keynum))
        {
            m_stat = STATUS_INVALID_KEYNUM;
            return;
        }
        m_keylen = writeKeyData();
        tdap(TD_REC_DELLETEATKEY);
    }
    else
    {
        if (inkey)
        {
            m_keylen = writeKeyData();
            seek();
            if (m_stat)
                return;
        }
        tdap(TD_REC_DELETE);
    }

}

void nstable::onInsertAfter(int beforeResult)
{
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
}

ushort_td nstable::doInsert(bool ncc)
{
    int trnCount = nsdb()->enableTrn();
    if (m_insart == false)
    {
        m_stat = STATUS_NO_ACR_INSERT;
        return 0;
    }
    m_stat = STATUS_SUCCESS;
    int option = onInsertBefore();
    if (m_stat)
    {
        if (trnCount < nsdb()->enableTrn())
            nsdb()->abortTrn();
        return 0;
    }
    ushort_td ins_rows = 0;
    char_td keynum = m_keynum;
    m_keylen = m_keybuflen;
    if (m_datalen > m_buflen)
        m_stat = STATUS_INVALID_VALLEN;
    m_datalen = getWriteImageLen();
    if (m_impl->bulkIns)
        ins_rows = m_impl->bulkIns->insert((const char*)data(), m_datalen, this);
    else
    {
        tdap(TD_REC_INSERT);
        if (ncc)
            m_keynum = -1;
        if (m_stat == STATUS_SUCCESS)
            ins_rows = 1;
    }
    m_keynum = keynum;
    onInsertAfter(option);

    return ins_rows;
}

void nstable::beginBulkInsert(int maxBuflen)
{
    if (m_impl->bulkIns)
        delete m_impl->bulkIns;
    m_impl->bulkIns = new bulkInsert(maxBuflen);
    m_stat = STATUS_SUCCESS;
}

ushort_td nstable::doCommitBulkInsert(bool autoCommit)
{
    ushort_td count = 0;
    if (m_impl->bulkIns && m_impl->bulkIns->count())
    {
        void* p = m_pdata;
        uint_td len = m_datalen;
        m_pdata = m_impl->bulkIns->data();
        m_datalen = m_impl->bulkIns->dataLen();
        m_keylen = m_keybuflen;
        tdap(TD_INSERT_BULK);

        if (m_stat == STATUS_SUCCESS)
            count = (*(ushort_td*)m_pdata);
        m_pdata = p;
        m_datalen = len;
        if (!autoCommit)
        {
            delete m_impl->bulkIns;
            m_impl->bulkIns = NULL;
        }
    }
    return count;
}

void nstable::abortBulkInsert()
{
    delete m_impl->bulkIns;
    m_impl->bulkIns = NULL;
    m_stat = STATUS_SUCCESS;
}

bookmark_td nstable::bookmark()
{
    void* db = m_pdata;

    m_pdata = &m_impl->bookmark;
    m_datalen = sizeof(bookmark_td);
    tdap(TD_BOOKMARK);
    m_pdata = db;

    return m_impl->bookmark;
}

void nstable::seekByBookmark(bookmark_td bm, ushort_td LockBias)
{
    int count = 0;
    memcpy(m_pdata, &bm, sizeof(bookmark_td));
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap((ushort_td)(TD_MOVE_BOOKMARK + LockBias));
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
    // TODO: This code has some problem.
    while (m_stat == STATUS_BUFFERTOOSMALL)
    {
        count++;
        m_buflen = m_buflen * 2;
        m_datalen = m_buflen;
        m_pdata = realloc(m_pdata, m_buflen);
        memcpy(m_pdata, &bm, sizeof(bookmark_td));
        tdap(TD_MOVE_BOOKMARK);
        if (m_stat == STATUS_SUCCESS)
        {
            onReadAfter();
            break;
        }
        if (count > 10)
            break;
    }
}

percentage_td nstable::getPercentage()
{
    void* db = m_pdata;
    m_pdata = &m_impl->percentage;
    m_datalen = sizeof(percentage_td);
    m_keylen = m_keybuflen;
    tdap(TD_GET_PER);
    m_impl->percentage &= 0xffff;
    m_pdata = db;
    return m_impl->percentage;
}

percentage_td nstable::getPercentage(bookmark_td bm)
{
    void* db = m_pdata;
    char_td ky = m_keynum;

    m_pdata = &m_impl->percentage;
    m_datalen = sizeof(percentage_td);
    m_keynum = -1;

    m_impl->percentage = bm;
    m_keylen = m_keybuflen;
    tdap(TD_GET_PER);
    m_impl->percentage &= 0xffff;

    m_pdata = db;
    m_keynum = ky;

    return m_impl->percentage;
}

void nstable::seekByPercentage()
{
    if (m_impl->percentage < 0)
        m_impl->percentage = 0;
    else if (m_impl->percentage > 10000)
        m_impl->percentage = 10000;
    memcpy(m_pdata, &m_impl->percentage, sizeof(TD_GET_PER));
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap(TD_MOVE_PER);
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();

}

void nstable::seekByPercentage(percentage_td pc)
{
    if (pc < 0)
        pc = 0;
    else if (pc > 10000)
        pc = 10000;
    memcpy(m_pdata, &pc, sizeof(percentage_td));
    m_datalen = m_buflen;
    m_keylen = m_keybuflen;
    tdap(TD_MOVE_PER);
    if (m_stat == STATUS_SUCCESS)
        onReadAfter();

}

void nstable::setOwnerName(const _TCHAR* Owner_p, char_td mode)
{

    void* svm_pdata = m_pdata;
    void* svm_keybuf = m_keybuf;
    keylen_td svm_keybuflen = m_keylen;
    char_td svm_keynum = m_keynum;

    char buf[21];
    const char* Owner = toChar(buf, Owner_p, 20);

    char dataBuf[9];
    char keyBuf[9];

    m_datalen = (uint_td)strlen(Owner) + 1;
    m_keylen = (keylen_td)(strlen(Owner) + 1);
    m_keynum = mode;
    strcpy(dataBuf, Owner);
    strcpy(keyBuf, Owner);
    m_pdata = dataBuf;
    m_keybuf = keyBuf;

    tdap(TD_SET_OWNERNAME);

    m_pdata = svm_pdata;
    m_keybuf = svm_keybuf;
    m_keylen = svm_keybuflen;
    m_keynum = svm_keynum;
}

void nstable::clearOwnerName() {tdap(TD_CLEAR_OWNERNAME);}

void nstable::stats(void* dataBuf, uint_td len, bool estimate)
{
    void* svm_pdata = m_pdata;
    void* svm_keybuf = m_keybuf;
    keylen_td svm_keybuflen = m_keylen;
    char_td svm_keynum = m_keynum;
    uchar_td keyBuf[255];

    m_pdata = dataBuf;
    m_datalen = len;
    m_keybuf = keyBuf;
    m_keylen = 255;
    m_keynum = estimate;

    tdap(TD_TABLE_INFO);

    m_pdata = svm_pdata;
    m_keybuf = svm_keybuf;
    m_keylen = svm_keybuflen;
    m_keynum = svm_keynum;
}

uint_td nstable::doRecordCount(bool estimate, bool fromCurrent)
{
    fileSpec *fs;
    uint_td Count;

    fs = (fileSpec*)malloc(1920);
    stats(fs, 1920, estimate);
    Count = fs->recCount;
    free(fs);
    return Count;
}

ushort_td nstable::recordLength()
{
    fileSpec *fs;
    ushort_td len = 0;

    fs = (fileSpec*)malloc(1920);
    stats(fs, 1920, true);
    if (m_stat == STATUS_SUCCESS)
        len = fs->recLen;
    free(fs);
    return len;
}

void nstable::doCreateIndex(bool specifyKeyNum)
{
    if (specifyKeyNum)
        m_keynum += ((uchar_td)0x80);
    tdap(TD_BUILD_INDEX);
    if (specifyKeyNum)
        m_keynum -= ((uchar_td)0x80);

}

void nstable::dropIndex(bool NoRenumber)
{
    if (NoRenumber)
        m_keynum += ((uchar_td)0x80);
    tdap(TD_DROP_INDEX);
    if (NoRenumber)
        m_keynum -= ((uchar_td)0x80);

}

short_td nstable::doBtrvErr(HWND hWnd, _TCHAR* retbuf)
{
    return tdapErr(hWnd, m_stat, m_impl->uri, retbuf);
}

void nstable::tdap(ushort_td op)
{
    short LoopCount = 0;
    m_op = op;
    if ((op > 4) && (!m_read))
    {
        m_stat = STATUS_NO_ACR_READ;
        return;
    }

    do
    {
        if (m_op != TD_OPENTABLE && m_impl->isOpen == false)
        {
            m_stat = STATUS_TABLE_YET_OPEN;
            return;
        }
        else if (m_op == TD_CLOSETABLE && m_impl->nsdb->enableTrn() > 0)
        {
            m_stat = STATUS_DURING_TRANSACTION;
            return;
        }
        BTRCALLID_PTR func = m_impl->nsdb->btrvFunc();
        m_stat = func(m_op, m_impl->posblk, m_pdata, &m_datalen, m_keybuf, m_keylen, m_keynum,
            m_impl->nsdb->clientID());

        // Wait for record lock or file lock.
        //LoopCount++;
        switch (m_stat)
        {
        case STATUS_LOCK_ERROR: Sleep(m_impl->nsdb->lockWaitTime());
            break;
        case STATUS_FILE_LOCKED: Sleep(m_impl->nsdb->lockWaitTime());
            break;

        default: return;
        }
    }
    while ((m_stat != STATUS_SUCCESS)
                    && (m_impl->nsdb->lockWaitCount() != LoopCount++));

}

/* tdap error handling
 */
short_td nstable::tdapErr(HWND hWnd, short_td status, const _TCHAR* TableName, _TCHAR* retbuf)
{
    if (status == STATUS_SUCCESS) return 0;
    else if (status == STATUS_EOF) return 0;
    else if (status == STATUS_NOT_FOUND_TI) return 0;

	_TCHAR buf[512];
    short_td errorCode = status;
	getErrorMessage(errorCode, buf, 512);

#pragma warning(disable:4996)
    if (retbuf)
        _stprintf(retbuf, _T("table_name:%s %s"), TableName, buf);
#pragma warning(default:4996)

    if ((int)hWnd <= 0) return errorCode;
        
#ifdef _WIN32
    if (TableName)
        MessageBox(hWnd, buf, TableName, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
    else
		MessageBox(hWnd, buf, _T("tdapErr"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
#endif
    return errorCode;
}


void nstable::throwError(const _TCHAR* caption, short statusCode)
{
    _TCHAR tmp[1024]={0x00};
    nstable::tdapErr(0x00, statusCode, NULL, tmp);
    _TCHAR tmp2[1024]={0x00};
    _stprintf_s(tmp2, 1024, _T("%s\n%s\n"),caption, tmp);
    THROW_BZS_ERROR_WITH_CODEMSG(statusCode, tmp2);
}

_TCHAR* nstable::getDirURI(const _TCHAR* path, _TCHAR* buf)
{
    bool uri = false;
    if (_tcsstr(path, _T("btrv://")) || _tcsstr(path, _T("tdap://")))
        uri = true;
#ifdef _WIN32
    if (uri == false)
        _tfullpath(buf, path, MAX_PATH);
    else
#endif
        _tcscpy(buf, path);
    _TUCHAR* p = _tcsmrchr((_TUCHAR*)buf, PSEPARATOR_C);
    _TUCHAR* p2 = _tcsmrchr((_TUCHAR*)buf, '=');
    if (p && p2)
    {
        if (p2 > p)
            p = NULL;
        else
            p2 = NULL;
    }
    if (p)
        * p = 0x00;
    if (p2)
        * (p2 + 1) = 0x00;
    return buf;
}

/* Get file name from full path name.
 *
 */
_TCHAR* nstable::getFileName(const _TCHAR* path, _TCHAR* filename)
{

    _TUCHAR* p = _tcsmrchr((_TUCHAR*)path, PSEPARATOR_C);
    _TUCHAR* p2 = _tcsmrchr((_TUCHAR*)path, '=');
    filename[0] = 0x00;
    if (p2 > p)
        p = p2;

    if (p)
    {
        p++;
        if (*p)
            _tcscpy(filename, (_TCHAR*)p);
    }
    else
        _tcscpy(filename, path);

    return filename;
}

bool nstable::existsFile(const _TCHAR* filename)
{
    return boost::filesystem::exists(filename);
}

bool nstable::test(nstable* p)
{
	try
	{

	char* pp = (char*)(p);
#ifdef _WIN32
	_TCHAR buf[256];
	wsprintf(buf, _T("test(%p) = %p\n"), p, *((nstimpl**)(pp + 4)));
	OutputDebugString(buf);
#endif
#ifdef __x86_64__ 
	return (MEM_FREED_MAGIC_NUMBER != *((nstimpl**)(pp + sizeof(char*))));
#else
	return (MEM_FREED_MAGIC_NUMBER != *((nstimpl**)(pp + sizeof(char*))));
#endif
	}
	catch(...){};
	return false;
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
