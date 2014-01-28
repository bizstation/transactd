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

#include "table.h"
#include "filter.h"
#include "database.h"
#include "bulkInsert.h"
#include "stringConverter.h"
#include <bzs/rtl/strtrim.h>
#include <bzs/rtl/stringBuffers.h>
#include <bzs/db/protocol/tdap/myDateTime.cpp>
#include <bzs/db/blobStructs.h>
#include <vector>
#include <algorithm>
#include <boost/timer.hpp>
#include <boost/shared_array.hpp>

#pragma package(smart_init)

#ifdef __BCPLUSPLUS__
#define _strupr strupr
#endif

using namespace bzs::db;
using namespace bzs::rtl;
using namespace bzs::db::protocol::tdap;

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

#ifdef _WIN32
#ifdef _INICODE
#define EXEC_CODEPAGE CP_UTF16
#else
#define EXEC_CODEPAGE GetACP()
#endif
#else
#define EXEC_CODEPAGE CP_UTF8
#endif


class CFiledNameIndex
{

public:
    CFiledNameIndex(short i, std::_tstring n);
    bool operator < (const CFiledNameIndex& rt) const ;

    short index;

    std::_tstring name;
};

class recordCache;


struct tbimpl
{
    stringConverter* cv;
    void* bookMarks;
    filter* filterPtr;
    recordCache* rc;
    void* dataBak;
    void* smartUpDate;
    void* bfAtcPtr;
	void* optionalData;
    int bookMarksMemSize;
    int maxBookMarkedCount;
    char keybuf[MAX_KEYLEN];
    char exNext;
    char keyNumIndex[128];
    short exSlideStat;

    struct
    {
    unsigned char exBookMarking : 1;
    unsigned char myDateTimeValueByBtrv: 1;
    unsigned char trimPadChar: 1;
    unsigned char usePadChar: 1;
    unsigned char smartUpDateFlag: 1;
    unsigned char logicalToString: 1;
    unsigned char dataPacked: 1;
    };

    std::vector<boost::shared_array<char> >blobs;
    ::bzs::rtl::stringBuffer strBufs;
    std::vector<CFiledNameIndex>fields;

    tbimpl() : strBufs(4096), bookMarks(NULL),bfAtcPtr(NULL), maxBookMarkedCount(0), bookMarksMemSize(0),
        filterPtr(NULL), rc(NULL), dataBak(NULL), optionalData(NULL),myDateTimeValueByBtrv(true), trimPadChar(true),
        usePadChar(true), smartUpDate(NULL), smartUpDateFlag(false)
        , logicalToString(false),dataPacked(false)
    {
        memset(&keyNumIndex[0], 0, 128);

    }

    ~tbimpl()
    {

        if (dataBak)
            free(dataBak);
        if (smartUpDate)
            free(smartUpDate);
        if (bookMarks)
            free(bookMarks);

        delete filterPtr;
    }
};

// ---------------------------------------------------------------------------
// class recordCache
// ---------------------------------------------------------------------------

unsigned int hash(const char *s, int len)
{
    unsigned int h = 0;
    for (int i = 0; i < len; i++)
        h = h * 137 + *(s + i);
    return h % 1987;
}

class recordCache
{
    table* m_tb;
    filter* m_pFilter;
    unsigned int m_row;
    unsigned int m_len;
    unsigned int m_unpackLen;
    unsigned int m_rowCount;
    bookmark_td m_bookmark;
    char* m_ptr;
    char* m_tmpPtr;
    blobHeader* m_hd;

public:
    inline recordCache(table* tb) : m_tb(tb) {reset();}

    inline void reset()
    {
        m_pFilter = NULL;
        m_row = 0;
        m_rowCount = 0;
        m_ptr = NULL;
        m_len = 0;
        m_tmpPtr = NULL;
    }

    inline void reset(filter* p, char* data, unsigned int totalSize, const blobHeader* hd)
    {
        m_pFilter = p;
        m_row = 0;
        m_rowCount = *((unsigned short*)data);
        m_ptr = data + DATASIZE_BYTE;
        m_len = m_unpackLen = *((unsigned short*)m_ptr); // Not include bookmark and size bytes.
        m_ptr += DATASIZE_BYTE;
        m_bookmark = *((bookmark_td*)(m_ptr));
        m_ptr += BOOKMARK_SIZE;
        m_tmpPtr = data + totalSize;
        m_hd = const_cast<blobHeader*>(hd);
    }

    inline const char* moveRow(int count)
    {
        for (int i = 0; i < count; i++)
        {
            m_ptr += m_len;
            m_len = m_unpackLen = *((unsigned short*)m_ptr);
            m_ptr += DATASIZE_BYTE;
            m_bookmark = *((bookmark_td*)(m_ptr));
            m_ptr += BOOKMARK_SIZE;

            if (m_hd)
            {
                m_hd->nextField = (blobField*)m_hd->nextField->next();
                ++m_hd->curRow;
            }
        }
        m_tb->m_impl->strBufs.clear();
        if (m_pFilter->fieldSelected())
        {
            int offset = 0;
            memset(m_tmpPtr, 0, m_len);
            for (int i = 0; i < m_pFilter->fieldCount(); i++)
            {
                memcpy((char*)m_tmpPtr + m_pFilter->fieldOffset(i), m_ptr + offset,
                    m_pFilter->fieldLen(i));
                offset += m_pFilter->fieldLen(i);
            }
            m_tb->setBlobFieldPointer(m_tmpPtr, m_hd);
            return m_tmpPtr;
        }
        else if (m_tb->valiableFormatType())
        {
            memcpy(m_tmpPtr, m_ptr, m_len);
            m_unpackLen = m_tb->unPack((char*)m_tmpPtr, m_len);
            m_tb->setBlobFieldPointer(m_tmpPtr, m_hd);
            return m_tmpPtr;
        }
        else
        {
            m_tb->setBlobFieldPointer(m_ptr, m_hd);
            return m_ptr;
        }
    }

    inline const char* setRow(unsigned int rowNum)
    {
        if (rowNum < m_rowCount)
        {
            unsigned int moveCount = rowNum - m_row;
            m_row = rowNum;
            return moveRow(moveCount);
        }
        return NULL;
    }

    inline unsigned int len() const {return m_unpackLen;};
    inline bookmark_td bookmarkCurRow() const {return m_bookmark;};
    inline int row() const {return m_row;}

    inline int rowCount() const {return m_rowCount;}
    inline bool isEndOfRow(unsigned int row) const {return  (m_rowCount && (row == m_rowCount));}
	inline bool withinCache(unsigned int row) const {return (row < m_rowCount);}
};

// ---------------------------------------------------------------------------
// class CFiledNameIndex
// ---------------------------------------------------------------------------
CFiledNameIndex::CFiledNameIndex(short i, std::_tstring n) : index(i), name(n)
{

}

bool CFiledNameIndex:: operator < (const CFiledNameIndex& rt) const
{
    return name < rt.name;
};


inline __int64 getValue64(const fielddef& fd, const uchar_td* ptr)
{
    __int64 ret = 0;
    switch (fd.type)
    {
    case ft_integer:
    case ft_autoinc:
        switch (fd.len)
        {
        case 1: ret = *((char*)(ptr + fd.pos));
            break;
        case 2: ret = *((short*)(ptr + fd.pos));
            break;
        case 4: ret = *((int*)(ptr + fd.pos));
            break;
        case 8: ret = *((__int64*)(ptr + fd.pos));
            break;
        }
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_bit:
    case ft_currency:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
        switch (fd.len)
        {
        case 1: ret = *((unsigned char*)(ptr + fd.pos));
            break;
        case 2: ret = *((unsigned short*)(ptr + fd.pos));
            break;
        case 4: ret = *((unsigned int*)(ptr + fd.pos));
            break;
        case 3:
        case 5:
        case 6:
        case 7: memcpy(&ret, ptr + fd.pos, fd.len);
            break;
        case 8: ret = *((__int64*)(ptr + fd.pos));
            break;
        }
    }
    return ret;
}

inline void setValue(const fielddef& fd, uchar_td* ptr, __int64 value)
{
    switch (fd.type)
    {
    case ft_integer:
    case ft_autoinc:
        {
            switch (fd.len)
            {
            case 1: *((char*)(ptr + fd.pos)) = (char)value;
                break;
            case 2: *((short*)(ptr + fd.pos)) = (short)value;
                break;
            case 4: *((int*)(ptr + fd.pos)) = (int)value;
                break;
            case 8: *((__int64*)(ptr + fd.pos)) = value;
                break;
            }
        }
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_bit:
    case ft_currency:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_mytime:
    case ft_mydate:
    case ft_mydatetime:
    case ft_mytimestamp:
        memcpy(ptr + fd.pos, &value, fd.len);
        break;
    }
}





table::table(nsdatabase *pbe) : nstable(pbe)
{
    m_impl = new tbimpl();

    m_impl->cv = new stringConverter(nsdatabase::execCodePage(), nsdatabase::execCodePage());

    m_impl->rc = new recordCache(this);

    m_keybuflen = MAX_KEYLEN;
    m_pdata = NULL;
    m_keybuf = &m_impl->keybuf[0];
    m_keynum = 0;

}

table::~table()
{
    delete m_impl->rc;
    delete m_impl->cv;
    delete m_impl;

}

inline uchar_td table::charset() const {return m_tableDef->charsetIndex;};

bool table::trimPadChar() const {return m_impl->trimPadChar;}

void table::setTrimPadChar(bool v) {m_impl->trimPadChar = v;}

bool table::usePadChar() const {return m_impl->usePadChar;};

void table::setUsePadChar(bool v) {m_impl->usePadChar = v;};

void* table::dataBak() const {return m_impl->dataBak;};

void table::setDataBak(void* v) {m_impl->dataBak = v;};

void* table::optionalData() const {return m_impl->optionalData;}

void table::setOptionalData(void* v) {m_impl->optionalData = v;}

bool table::myDateTimeValueByBtrv() const {return m_impl->myDateTimeValueByBtrv;}

bool table::logicalToString() const {return m_impl->logicalToString;};

void table::setLogicalToString(bool v) {m_impl->logicalToString = v;}

void table::setBookMarks(int StartId, void* Data, ushort_td Count)
{
    long size = (StartId + Count) * 6;
    if (m_impl->bookMarksMemSize < size)
    {

        m_impl->bookMarks = realloc(m_impl->bookMarks, size + BOOKMARK_ALLOC_SIZE);
        m_impl->bookMarksMemSize = size + BOOKMARK_ALLOC_SIZE;
    }
    if (m_impl->bookMarks)
    {
        if (StartId + Count - 1 > m_impl->maxBookMarkedCount)
            m_impl->maxBookMarkedCount = StartId + Count - 1;
        memcpy((void*)((char*)m_impl->bookMarks + ((StartId - 1) * 6)), Data, Count * 6);
    }
    else
        m_stat = STATUS_CANT_ALLOC_MEMORY;
    return;

}

inline short calcNextReadRecordCount(ushort_td curCount, int eTime)
{
    ushort_td ret = curCount;
    if (eTime == 0)
        ret = (ushort_td)(curCount * (short)2);
    else
        ret = (ushort_td)(curCount * ((float)100 / (float)eTime));

    if (ret > 10000)
        return 10000;
    if (ret == 0)
        return 1;
    return ret;
}

uint_td table::doRecordCount(bool estimate, bool fromCurrent)
{
    uint_td result = 0;

    if (m_impl->filterPtr)
    {
        short curStat = m_stat;
        m_impl->exBookMarking = true;
        ushort_td recCountOnce = 50;

        bookmark_td bm = bookmark();

        ushort_td tmpRejectCount = m_impl->filterPtr->rejectCount();
        ushort_td tmpRecCount = m_impl->filterPtr->recordCount();

        m_impl->filterPtr->setIgnoreFields(true);
        m_impl->filterPtr->setMaxRows(10000);
        m_impl->maxBookMarkedCount = 0;
        if (fromCurrent)
            m_stat = curStat;
        else
            seekFirst();

        m_impl->filterPtr->setMaxRows(recCountOnce);
        if (m_stat == 0)
        {
            m_impl->filterPtr->setPosTypeNext(false);
            boost::timer t;
            btrvGetExtend(TD_KEY_NEXT_MULTI);
            int eTime = (int)(t.elapsed() * 1000);
            while ((m_stat == 0) || (m_stat == STATUS_LIMMIT_OF_REJECT) || (m_stat == STATUS_EOF)
                        || (m_stat == STATUS_REACHED_FILTER_COND))
            {
                bool Complete = false;
                if ((m_stat == STATUS_EOF) || (m_stat == STATUS_REACHED_FILTER_COND))
                {
                    Complete = true;
                    m_stat = STATUS_EOF;
                }
                else if (m_stat == STATUS_LIMMIT_OF_REJECT)
                {
                    if (tmpRejectCount != 0)
                    {
                        if (tmpRejectCount == 1)
                            m_stat = STATUS_EOF;
                        Complete = true;
                    }
                }
                recCountOnce = calcNextReadRecordCount(recCountOnce, eTime);
                m_impl->filterPtr->setMaxRows(recCountOnce);
                result += *((ushort_td*)m_impl->dataBak);
                setBookMarks(m_impl->maxBookMarkedCount + 1, (void*)((char*)m_impl->dataBak + 2),
                    *((ushort_td*)m_impl->dataBak));
                m_impl->maxBookMarkedCount = result;

                onRecordCounting(result, Complete);
                if (Complete)
                    break;
                t.restart();
                m_impl->filterPtr->setPosTypeNext(true);
                btrvGetExtend(TD_KEY_NEXT_MULTI);
                eTime = (int)(t.elapsed() * 1000);
            }
        }

        short tmpStat = m_stat;
        m_impl->filterPtr->setIgnoreFields(false);
        m_impl->filterPtr->setMaxRows(tmpRecCount);

        if (bm)
            seekByBookmark(bm);
        m_impl->exBookMarking = false;
        m_stat = tmpStat;
    }
    else
        return nstable::doRecordCount(estimate, fromCurrent);

    return result;
}

void table::btrvGetExtend(ushort_td op)
{

    if (op >= TD_KEY_GE_NEXT_MULTI)
        m_keylen = writeKeyData();
    m_pdata = m_impl->dataBak;
    if (!m_impl->filterPtr->writeBuffer())
    {
        m_stat = STATUS_WARKSPACE_TOO_SMALL;
        return;
    }
    m_datalen = m_impl->filterPtr->exDataBufLen();
    tdap(op);
    short stat = m_stat;
    if (!m_impl->filterPtr->isWriteComleted() && (stat == STATUS_REACHED_FILTER_COND))
        stat = STATUS_LIMMIT_OF_REJECT;

    m_impl->rc->reset(m_impl->filterPtr, (char*)m_impl->dataBak, m_datalen, blobFieldUsed() ?
        getBlobHeader() : NULL);

    m_stat = stat;
    m_impl->exSlideStat = m_stat;
    //There is the right record.
    if (m_impl->rc->rowCount() && (!m_impl->exBookMarking))
    {
        m_pdata = (void*)m_impl->rc->setRow(0);
        m_datalen = m_impl->rc->len();

        m_stat = STATUS_SUCCESS;
    }else if ((m_stat == STATUS_LIMMIT_OF_REJECT) && (m_impl->filterPtr->rejectCount()>=1))
        m_stat = STATUS_EOF;


}

void table::getRecords(ushort_td op)
{
    do
    {
        btrvGetExtend(op);
        m_impl->filterPtr->setPosTypeNext(true);

    }while (m_stat == STATUS_LIMMIT_OF_REJECT && (m_impl->filterPtr->rejectCount()==0));
    if ((m_stat == STATUS_REACHED_FILTER_COND) || (m_stat == STATUS_LIMMIT_OF_REJECT))
        m_stat = STATUS_EOF;
}

bookmark_td table::bookmarkFindCurrent() const
{

    if (!m_impl->rc->isEndOfRow(m_impl->rc->row()))
        return m_impl->rc->bookmarkCurRow();
    return 0;

}

void table::find(eFindType type)
{
    ushort_td op;
    if (m_impl->filterPtr->isSeeksMode())
    {
        m_impl->filterPtr->resetSeeksWrited();
        op = TD_KEY_SEEK_MULTI;
    }
    else
        op = (type == findForword) ? TD_KEY_GE_NEXT_MULTI:TD_KEY_LE_PREV_MULTI;

    if (nsdb()->isUseTransactd())
    {
        m_impl->rc->reset();
        doFind(op, true/*notIncCurrent*/);
    }
    else
    {

        if (op == TD_KEY_SEEK_MULTI)
        {
            //P.SQL not support TD_KEY_SEEK_MULTI
            m_stat = STATUS_FILTERSTRING_ERROR;
            return;
        }
        if (type == findForword)
            seekGreater(true);
        else
            seekLessThan(true);
        if (m_stat == 0)
        {
            if (type == findForword)
                findNext(true);
            else
                findPrev(true);
        }
    }


}

void table::findFirst()
{
    seekFirst();
    if (m_stat == 0)
    {
        if (m_impl->filterPtr)
        {
            m_impl->exNext = 1;
            m_impl->filterPtr->setPosTypeNext(false);
            getRecords(TD_KEY_NEXT_MULTI);
        }
    }
}

void table::findLast()
{
    seekLast();
    if (m_stat == 0)
    {
        if (m_impl->filterPtr)
        {
            m_impl->exNext = -1;
            m_impl->filterPtr->setPosTypeNext(false);
            getRecords(TD_KEY_PREV_MULTI);
        }
    }
}

void table::doFind( ushort_td op, bool notIncCurrent)
{
        /*
        First, read from cache.
        If whole row readed from cache then select operation by m_impl->exSlideStat

        */
        m_stat = 0;
        int row = m_impl->rc->row() + 1;

        if (m_impl->rc->withinCache(row) && (!m_impl->exBookMarking))
        {   /* read from cache */
            m_pdata = (void*)m_impl->rc->setRow(row);
            m_datalen = m_impl->rc->len();
        }
        else if (m_impl->rc->isEndOfRow(row))
        {
            /* whole row readed */
            /* A special situation that if rejectCount() == 0 and status = STATUS_LIMMIT_OF_REJECT
                then it continues . */
            if ((m_impl->exSlideStat == 0)
                || ((m_impl->exSlideStat == STATUS_LIMMIT_OF_REJECT)
                            && (m_impl->filterPtr->rejectCount() == 0)))
            {
                getRecords(op);
                return;
            }
            if ((m_impl->exSlideStat == STATUS_LIMMIT_OF_REJECT) ||
                    (m_impl->exSlideStat == STATUS_REACHED_FILTER_COND))
                    m_stat = STATUS_EOF;
            else
                m_stat = m_impl->exSlideStat;
             m_impl->exSlideStat = 0;
        }
        else
        {
            m_impl->exNext = ((op == TD_KEY_NEXT_MULTI) || (op == TD_KEY_GE_NEXT_MULTI)) ? 1: -1;
            m_impl->filterPtr->setPosTypeNext(notIncCurrent);
            getRecords(op);
        }

}

void table::findNext(bool notIncCurrent)
{

    if (m_impl->filterPtr)
    {
        short op = m_impl->filterPtr->isSeeksMode() ? TD_KEY_SEEK_MULTI
                    : TD_KEY_NEXT_MULTI;
        doFind(op, notIncCurrent);
    }
    else if (notIncCurrent == true)
        seekNext();
}

void table::findPrev(bool notIncCurrent)
{
    if (m_impl->filterPtr)
        doFind(TD_KEY_PREV_MULTI, notIncCurrent);
    else if (notIncCurrent == true)
        seekPrev();

}

void table::setQuery(const queryBase* query)
{

    m_stat = 0;
    m_pdata = m_impl->dataBak;
    m_impl->rc->reset();
    m_impl->exBookMarking = false;
    m_impl->exSlideStat = 0;
    m_impl->exNext = 0;
    if (query == NULL)
    {
        m_impl->maxBookMarkedCount = 0;
        delete m_impl->filterPtr;
        m_impl->filterPtr = NULL;
        return;
    }
    if (m_impl->filterPtr)
        m_impl->filterPtr->init(this);
    else
        m_impl->filterPtr = new filter(this);
    if (m_impl->filterPtr == NULL)
    {
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return;

    }

    if (!m_impl->filterPtr->setQuery(query))
    {
        m_stat = STATUS_FILTERSTRING_ERROR;
        delete m_impl->filterPtr;
        m_impl->filterPtr = NULL;
        return;
    }
    if (!m_impl->bookMarks)
    {
        m_impl->bookMarks = malloc(BOOKMARK_ALLOC_SIZE);
        if (m_impl->bookMarks)
            m_impl->bookMarksMemSize = BOOKMARK_ALLOC_SIZE;
        else
        {
            m_stat = STATUS_FILTERSTRING_ERROR;
            delete m_impl->filterPtr;
            m_impl->filterPtr = NULL;
        }
    }
}

void table::setFilter(const _TCHAR* str, ushort_td RejectCount, ushort_td CashCount)
{
    if (!str || (str[0] == 0x00))
        setQuery(NULL);
    else
    {
        queryBase q;
        q.queryString(str).reject(RejectCount).limit(CashCount);
        setQuery(&q);
    }

}

const _TCHAR* table::filterStr()
{
    if (m_impl->filterPtr)
        return m_impl->filterPtr->filterStr();
    else
        return NULL;
}

void table::clearBuffer()
{
    m_pdata = m_impl->dataBak;
    memset(m_pdata, 0x00, m_buflen);
    if (blobFieldUsed())
        resetSendBlob();

}

void table::getKeySpec(keySpec* ks, bool SpecifyKeyNum)
{
    keydef* KeyDef;
    short FieldNum;
    int j;

    KeyDef = &m_tableDef->keyDefs[m_keynum];
    for (j = 0; j < KeyDef->segmentCount; j++)
    {
        FieldNum = KeyDef->segments[j].fieldNum;
        ks[j].keyPos = (ushort_td)(m_tableDef->fieldDefs[FieldNum].pos + 1);
        ks[j].keyLen = m_tableDef->fieldDefs[FieldNum].len;
        ks[j].keyFlag.all = KeyDef->segments[j].flags.all;
        ks[j].keyCount = 0;
        ks[j].keyType = m_tableDef->fieldDefs[FieldNum].type;

        if (ks[j].keyFlag.bit3 == true)
            ks[j].nullValue = m_tableDef->fieldDefs[FieldNum].nullValue;
        else
            ks[j].nullValue = 0;
        ks[j].reserve2[0] = 0;
        ks[j].reserve2[1] = 0;

        if (SpecifyKeyNum == true)
            ks[j].keyNo = m_keynum;
        else
            ks[j].keyNo = 0;

        ks[j].acsNo = 0;
    }
}

void table::doCreateIndex(bool SpecifyKeyNum)
{
    int segmentCount = m_tableDef->keyDefs[m_keynum].segmentCount;
    keySpec* ks = new keySpec[segmentCount];
    getKeySpec(ks, SpecifyKeyNum);
    m_pdata = ks;
    m_datalen = sizeof(keySpec) * segmentCount;
    nstable::doCreateIndex(SpecifyKeyNum);
    m_pdata = m_impl->dataBak;
    delete[]ks;
}

void table::smartUpdate()
{ 
    if (!m_impl->smartUpDate)
        m_impl->smartUpDate = malloc(m_buflen);
    if (m_impl->smartUpDate)
    {
        memcpy(m_impl->smartUpDate, data(), m_buflen);
        m_impl->smartUpDateFlag = true;
    }
    else
        m_impl->smartUpDateFlag = false;
}

bool table::isUniqeKey(char_td keynum)
{
    if ((keynum>=0) && (keynum < m_tableDef->keyCount))
    {
        keydef* kd = &m_tableDef->keyDefs[m_keynum];
        return !(kd->segments[0].flags.bit0);
    }
    return false;
}

bool table::onUpdateCheck(eUpdateType type)
{
    //Check uniqe key
    if (type == changeInKey)
    {
        if (!isUniqeKey(m_keynum))
        {
            m_stat = STATUS_INVALID_KEYNUM;
            return false;
        }else
        {
            if (nsdb()->isUseTransactd()==false)
            {
                //backup update data
                smartUpdate();
                seek();
                m_impl->smartUpDateFlag = false;
                if (m_stat)
                    return false;
                memcpy(m_pdata, m_impl->smartUpDate, m_datalen);
            }
        }
    }
    else if (m_impl->smartUpDateFlag)
    {
        m_stat = 0;
        if (memcmp(m_impl->smartUpDate, data(), m_buflen) == 0)
        {
            m_impl->smartUpDateFlag = false;
            return false;
        }
    }
    return true;
}

void table::onUpdateAfter(int beforeResult)
{
    //if (blobFieldUsed())
    //    resetSendBlob();
    if (valiableFormatType() && m_impl->dataPacked)
        m_datalen = unPack((char*)m_pdata, m_datalen);

}

bool table::onDeleteCheck(bool inkey)
{
    client::database *db = static_cast<client::database*>(nsdb());
    deleteRecordFn func = db->onDeleteRecord();
	if (func)
	{
		if (func(db, this, inkey))
		{
			m_stat = STATUS_CANT_DEL_FOR_REL;
			return false;
		}
	}
    return true;
}

ushort_td table::doCommitBulkInsert(bool autoCommit)
{
    ushort_td ret = nstable::doCommitBulkInsert(autoCommit);
    if (blobFieldUsed())
        resetSendBlob();
    return ret;
}

void table::onInsertAfter(int beforeResult)
{
    if (valiableFormatType() && m_impl->dataPacked)
        m_datalen = unPack((char*)m_pdata, m_datalen);
    if (blobFieldUsed())
        resetSendBlob();
}

void* table::attachBuffer(void* NewPtr, bool unpack, size_t size)
{
    void* oldptr;
    if (!m_impl->bfAtcPtr)
        m_impl->bfAtcPtr = m_pdata;
    oldptr = m_pdata;
    m_pdata = NewPtr;
    ushort_td len = recordLength();
    if (len < m_tableDef->maxRecordLen)
        len = m_tableDef->maxRecordLen;
    if (unpack)
		len = unPack((char*)m_pdata, size);
	m_datalen = len;
    return oldptr;
}

void table::dettachBuffer()
{
    if (m_impl->bfAtcPtr)
        m_pdata = m_impl->bfAtcPtr;
    m_impl->bfAtcPtr = NULL;

}

void table::init(tabledef* Def, short fnum, bool regularDir) {doInit(Def, fnum, regularDir);}

void table::doInit(tabledef* Def, short fnum, bool /*regularDir*/)
{
    m_tableDef = Def;
    ushort_td len;

    m_impl->cv->setCodePage(mysql::codePage(m_tableDef->charsetIndex));

    if ((len = recordLength()) < m_tableDef->maxRecordLen)
        len = m_tableDef->maxRecordLen;

    if (len == 0)
    {
        m_stat = STATUS_INVALID_RECLEN;
        return;
    }

    for (short i = 0; i < m_tableDef->keyCount; i++)
    {
        if (m_tableDef->flags.bitA == true)
            m_impl->keyNumIndex[m_tableDef->keyDefs[i].keyNumber] = (char)i;
        else
            m_impl->keyNumIndex[i] = (char)i;
    }
    if (m_impl->dataBak)
        free(m_impl->dataBak);
    m_impl->dataBak = (void*) malloc(len);

    if (m_impl->dataBak == NULL)
    {
        if (m_impl->dataBak)
            free(m_impl->dataBak);
        m_impl->dataBak = NULL;
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return;
    }
    m_pdata = m_impl->dataBak;
    m_buflen = len;
    m_datalen = len;
    setTableid(fnum);

}

keylen_td table::writeKeyData()
{
    if (m_tableDef->keyCount)
    {
        keydef& keydef = m_tableDef->keyDefs[(short)m_impl->keyNumIndex[m_keynum]];
        uchar_td* to = (uchar_td*)m_impl->keybuf;

        for (int j = 0; j < keydef.segmentCount; j++)
        {
            int fdnum = keydef.segments[j].fieldNum;
            fielddef& fd = m_tableDef->fieldDefs[fdnum];
            uchar_td* from = (uchar_td*)m_pdata + fd.pos;
            to = fd.keyCopy(to, from);
        }
        return (keylen_td)(to - (uchar_td*)m_impl->keybuf);
    }
    return 0;
}

uint_td table::pack(char*ptr, size_t size)
{
    char* pos = ptr;
    char* end = pos + size;
    int movelen;
    for (int i = 0; i < m_tableDef->fieldCount; i++)
    {
        fielddef& fd = m_tableDef->fieldDefs[i];
        int blen = fd.varLenBytes();
        int dl = fd.len; // length
        if (blen == 1)
            dl = *((unsigned char*)(pos)) + blen;
        else if (blen == 2)
            dl = *((unsigned short*)(pos)) + blen;
        pos += dl;
        if ((movelen = fd.len - dl) != 0)
        {
            end -= movelen;
            memmove(pos, pos + movelen, end - pos);
        }
    }
    m_impl->dataPacked  = true;
    return (uint_td)(pos - ptr);
}

uint_td table::doGetWriteImageLen()
{
    if (!blobFieldUsed() && !valiableFormatType() && (m_tableDef->flags.bit0 == false))
        return m_buflen;
    // Make blob pointer list
    if (blobFieldUsed())
    {
        for (ushort_td i = 0; i < m_tableDef->fieldCount; i++)
        {
            fielddef& fd = m_tableDef->fieldDefs[i];
            uint_td bytes = fd.blobLenBytes();
            if (bytes)
            {
                uchar_td* fdptr = (uchar_td*)m_pdata + fd.pos;
                blob b(fd.blobDataLen(fdptr), i, fd.blobDataPtr(fdptr));
                addSendBlob(&b);
            }
        }
        addBlobEndRow(); // end row
    }
    else
        addSendBlob(NULL);

    if (valiableFormatType())
        return pack((char*)m_pdata, m_buflen);
    else
    {
        fielddef* FieldDef = &m_tableDef->fieldDefs[m_tableDef->fieldCount - 1];
        size_t len = 0;
        short* pos;

        if (FieldDef->type == ft_note)
            len = strlen((char*)fieldPtr((short)(m_tableDef->fieldCount - 1))) + 1;
        else if (FieldDef->type == ft_lvar)
        {
            // xx................xx.............00
            // ln--data----------ln-----data----00
            pos = (short*)fieldPtr((short)(m_tableDef->fieldCount - 1));
            while (*pos)
            {
                len += 2; // size
                len += *pos;
                pos = (short*)((char*)pos + (*pos + 2)); // next position

            }
            len += 2;
        }
        else
            len = FieldDef->len;

        len += FieldDef->pos;

        return (uint_td)len;
    }
}

uint_td table::unPack(char* ptr, size_t size)
{
    char* pos = ptr;
    const char* end = pos + size;
    const char* max = pos + m_buflen;
    int movelen;
    for (int i = 0; i < m_tableDef->fieldCount; i++)
    {
        fielddef& fd = m_tableDef->fieldDefs[i];
        int blen = fd.varLenBytes();
        int dl = fd.len; // length
        if (blen == 1)
            dl = *((unsigned char*)(pos)) + blen;
        else if (blen == 2)
            dl = *((unsigned short*)(pos)) + blen;
        if ((movelen = fd.len - dl) != 0)
        {
            if (max < end + movelen)
                return 0;
            const char* src = pos + dl;
            memmove(pos + fd.len, src, end - src);
            end += movelen;
        }
        pos += fd.len;
    }
    m_impl->dataPacked  = false;
    return (uint_td)(pos - ptr);
}

void table::addBlobEndRow()
{
    char_td knum = m_keynum;
    m_keynum = TD_ASBLOB_ENDROW;
    addSendBlob(NULL);
    m_keynum = knum;
}

void table::resetSendBlob()
{
    addSendBlob(NULL);
    m_impl->blobs.clear();

}

void table::addSendBlob(const blob* blob)
{
    short stat = m_stat;
    const void *tmp = data();
    setData((void*)blob);
    tdap(TD_ADD_SENDBLOB);
    setData((void*)tmp);
    m_stat = stat;
}

const blobHeader* table::getBlobHeader()
{
    short stat = m_stat;
    const blobHeader* p;
    const void *tmp = data();
    setData(&p);
    tdap(TD_GET_BLOB_BUF);
    setData((void*)tmp);
    std::swap(stat, m_stat);

    if (stat)
        return NULL;
    return p;

}

void table::setBlobFieldPointer(char* ptr, const blobHeader* hd)
{
    if (hd)
    {
        assert(hd->curRow < hd->rows);
        const blobField* f = hd->nextField;
        for (int i = 0; i < hd->fieldCount; i++)
        {
            fielddef& fd = m_tableDef->fieldDefs[f->fieldNum];
            char* fdptr = ptr + fd.pos;
            int sizeByte = fd.blobLenBytes();
            memcpy(fdptr, &f->size, sizeByte);
            const char* data = f->data();
            memcpy(fdptr + sizeByte, &data, sizeof(char*));
            f = f->next();
        }
        ++hd->curRow;
        hd->nextField = (blobField*)f;
    }

}

void table::onReadAfter()
{
    m_impl->strBufs.clear();
    if (valiableFormatType())
    {
        m_datalen = unPack((char*)m_pdata, m_datalen);
        if (m_datalen == 0)
            m_stat = 22;
    }
    if (blobFieldUsed())
    {
        const blobHeader* hd = getBlobHeader();
        setBlobFieldPointer((char*)m_pdata, hd);
    }
    if (m_buflen - m_datalen > 0)
        memset((char*)m_pdata + m_datalen, 0, m_buflen - m_datalen);

}

short table::fieldNumByName(const _TCHAR* name)
{
    short i=0;
    if (name == 0) return i;
    if (m_impl->fields.size() == 0)
    {
#ifdef _UNICODE
        wchar_t buf[74];
        for (i = 0; i < m_tableDef->fieldCount; i++)
            m_impl->fields.push_back(CFiledNameIndex(i, m_tableDef->fieldDefs[i].name(buf)));
#else
        for (i = 0; i < m_tableDef->fieldCount; i++)
            m_impl->fields.push_back(CFiledNameIndex(i, m_tableDef->fieldDefs[i].name()));
#endif

        sort(m_impl->fields.begin(), m_impl->fields.end());
    }
    if (binary_search(m_impl->fields.begin(), m_impl->fields.end(), CFiledNameIndex(0, name)))
    {
        std::vector<CFiledNameIndex>::iterator p;
        p = lower_bound(m_impl->fields.begin(), m_impl->fields.end(), CFiledNameIndex(0, name));
        return m_impl->fields[p - m_impl->fields.begin()].index;
    }

    return -1;
}

void* table::fieldPtr(short index)
{
    if (checkIndex(index) == false)
        return NULL;
    return (void*)((char*) m_pdata + m_tableDef->fieldDefs[index].pos);
}

void table::setFVA(short index, const char* data)
{

    __int64 value;
    double fltValue;
    if (checkIndex(index) == false)
        return;
    fielddef& fd = m_tableDef->fieldDefs[index];

    char* p = (char*)m_pdata + fd.pos;
    if (data == NULL)
    {
        memset(p, 0, fd.len);
        return;
    }

    switch (fd.type)
    {
    case ft_string:
        return store<stringStore, char, char>(p, data, fd, m_impl->cv, m_impl->usePadChar);
    case ft_note:
    case ft_zstring: return store<zstringStore, char, char>(p, data, fd, m_impl->cv);
    case ft_wzstring: return store<wzstringStore, WCHAR, char>(p, data, fd, m_impl->cv);
    case ft_wstring:
        return store<wstringStore, WCHAR, char>(p, data, fd, m_impl->cv, m_impl->usePadChar);
    case ft_mychar: return store<myCharStore, char, char>(p, data, fd, m_impl->cv);
    case ft_myvarchar: return store<myVarCharStore, char, char>(p, data, fd, m_impl->cv);
    case ft_lstring:
    case ft_myvarbinary: return store<myVarBinaryStore, char, char>(p, data, fd, m_impl->cv);
    case ft_mywchar: return store<myWcharStore, WCHAR, char>(p, data, fd, m_impl->cv);
    case ft_mywvarchar: return store<myWvarCharStore, WCHAR, char>(p, data, fd, m_impl->cv);
    case ft_mywvarbinary: return store<myWvarBinaryStore, WCHAR, char>(p, data, fd, m_impl->cv);
    case ft_myblob:
    case ft_mytext:
        {
            char* tmp = blobStore<char>(p, data, fd, m_impl->cv);
            m_impl->blobs.push_back(boost::shared_array<char>(tmp));
            return;
        }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:
    case ft_currency: // currecy
    case ft_float:    // float double
        fltValue = atof(data);
        setFV(index, fltValue);
        return;
    case ft_lvar: // Lvar
        return;

    case ft_date: // date mm/dd/yy
        value = /*StrToBtrDate*/atobtrd((const char*) data).i;
        break;
    case ft_time: // time hh:nn:ss
        value = /*StrToBtrTime*/atobtrt((const char*)data).i;
        break;
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_integer:
    case ft_autoinc:
    case ft_bit: value = _atoi64(data);
        break;
    case ft_logical:
        if (m_impl->logicalToString)
        {
            char tmp[5];
            strncpy(tmp, data, 5);
            if (strcmp(_strupr(tmp), "YES") == 0)
                value = 1;
            else
                value = 0;
        }
        else
            value = atol(data);
        break;
    case ft_timestamp:
    case ft_mytimestamp: value = 0;
        break;
    case ft_mydate:
        {
            myDate d;
            d = data;
            value = d.getValue();
            break;
        }
    case ft_mytime:
        {
            myTime t(fd.len);
            t = data;
            value = t.getValue();
            break;
        }

    case ft_mydatetime:
        {
            myDateTime t(fd.len);
            t = data;
            value = t.getValue();
            break;
        }
    default: return;
    }
    setValue(fd, (uchar_td*)m_pdata, value);
}

#ifdef _WIN32

void table::setFVW(short index, const wchar_t* data)
{

    int value;
    double fltValue;
    if (checkIndex(index) == false)
        return;
    fielddef& fd = m_tableDef->fieldDefs[index];

    char* p = (char*)m_pdata + fd.pos;
    if (data == NULL)
    {
        memset(p, 0, fd.len);
        return;
    }

    switch (fd.type)
    {
    case ft_string:
        return store<stringStore, char, WCHAR>(p, data, fd, m_impl->cv, m_impl->usePadChar);
    case ft_note:
    case ft_zstring: return store<zstringStore, char, WCHAR>(p, data, fd, m_impl->cv);
    case ft_wzstring: return store<wzstringStore, WCHAR, WCHAR>(p, data, fd, m_impl->cv);
    case ft_wstring:
        return store<wstringStore, WCHAR, WCHAR>(p, data, fd, m_impl->cv, m_impl->usePadChar);
    case ft_mychar: return store<myCharStore, char, WCHAR>(p, data, fd, m_impl->cv);
    case ft_myvarchar: return store<myVarCharStore, char, WCHAR>(p, data, fd, m_impl->cv);
    case ft_lstring:
    case ft_myvarbinary: return store<myVarBinaryStore, char, WCHAR>(p, data, fd, m_impl->cv);
    case ft_mywchar: return store<myWcharStore, WCHAR, WCHAR>(p, data, fd, m_impl->cv);
    case ft_mywvarchar: return store<myWvarCharStore, WCHAR, WCHAR>(p, data, fd, m_impl->cv);
    case ft_mywvarbinary: return store<myWvarBinaryStore, WCHAR, WCHAR>(p, data, fd, m_impl->cv);
    case ft_myblob:
    case ft_mytext:
        {
            char* tmp = blobStore<WCHAR>(p, data, fd, m_impl->cv);
            m_impl->blobs.push_back(boost::shared_array<char>(tmp));
            return;
        }

    case ft_date: // date mm/dd/yy
        value = /*StrToBtrDate*/atobtrd(data).i;
        setFV(index, value);
        break;
    case ft_time: // time hh:nn:ss
        value = /*StrToBtrTime*/atobtrt(data).i;
        setFV(index, value);
        return;

    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_integer:
    case ft_autoinc:
    case ft_bit:
        {
            __int64 v = _wtoi64(data);
            setFV(index, v);
            break;
        }
    case ft_logical:
        if (m_impl->logicalToString)
        {
            wchar_t tmp[5];
            wcsncpy(tmp, data, 5);

            if (wcscmp(_wcsupr(tmp), L"YES") == 0)
                value = 1;
            else
                value = 0;
        }
        else
            value = _wtol(data);
        setFV(index, value);
        break;

    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:
    case ft_currency:
    case ft_float: fltValue = _wtof(data);
        setFV(index, fltValue);
        break;
    case ft_timestamp:
        {
            __int64 v = 0;
            setFV(index, v);
            return;
        }
    case ft_mydate:
        {
            myDate d;
            d = data;
            setValue(fd, (uchar_td *)m_pdata, d.getValue());
            return;
        }
    case ft_mytime:
        {
            myTime t(fd.len);
            t = data;
            setValue(fd, (uchar_td*)m_pdata, t.getValue());
            return;
        }
    case ft_mydatetime:
        {
            myDateTime t(fd.len);
            t = data;
            setFV(index, t.getValue());
            return;
        }
    case ft_mytimestamp:
    case ft_lvar: break;
    }

}

void table::setFVW(const _TCHAR* FieldName, const wchar_t* data)
{
    short index = fieldNumByName(FieldName);
    setFVW(index, data);
}

#endif //_WIN32

void table::setFV(short index, unsigned char data)
{
    int value = (long)data;
    setFV(index, value);
}

void table::setFV(short index, int data)
{
    char buf[20];
    double d;
    if (checkIndex(index) == false)
        return;
    int v = data;
    fielddef& fd = m_tableDef->fieldDefs[index];
    switch (fd.type)
    {

    case ft_mydate:
        {
            myDate myd;
            myd.setValue(data, m_impl->myDateTimeValueByBtrv);
            setValue(fd, (uchar_td*)m_pdata, myd.getValue());
            break;
        }
    case ft_mytime:
        {
            myTime myt(fd.len);
            myt.setValue(data, m_impl->myDateTimeValueByBtrv);
            setValue(fd, (uchar_td*)m_pdata, myt.getValue());
            break;
        }
    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_bit:
    case ft_mydatetime:
        switch (m_tableDef->fieldDefs[index].len)
        {
        case 1: *((char*)((char*)m_pdata + fd.pos)) = (char) v;
            break;
        case 2: *((short*)((char*)m_pdata + fd.pos)) = (short)v;
            break;
        case 3: memcpy((char*)m_pdata + fd.pos, &v, 3);
            break;
        case 4: *((int*)((char*)m_pdata + fd.pos)) = v;
            break;
        case 8: *((__int64*)((char*)m_pdata + fd.pos)) = v;
            break;
        }
        break;

    case ft_timestamp:
        {
            __int64 v = 0;
            setFV(index, v);
            return;
        }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:

    case ft_currency:
    case ft_float: d = (double)data;
        setFV(index, d);
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        if (data == 0)
            setFVA(index, "");
        else
        {
            _ltoa_s(data, buf, 20, 10);
            setFVA(index, buf);
        }
        break;
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
        {
            if (data == 0)
                setFV(index, _T(""));
            else
            {
                _TCHAR buf[30];
                _ltot_s(data, buf, 30, 10);
                setFV(index, buf);
            }
            break;
        }
    case ft_lvar: break;

    }
}

void table::setFV(short index, double data)
{
    char buf[20];
    __int64 i64;
    if (checkIndex(index) == false)
        return;

    switch (m_tableDef->fieldDefs[index].type)
    {
    case ft_currency: // currency
        i64 = (__int64)(data * 10000 + 0.5);
        setFV(index, i64);
        break;
    case ft_bfloat: // bfloat
    case ft_float:
        switch (m_tableDef->fieldDefs[index].len)
        {
        case 4: *((float*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos)) = (float)data;
            break;
        case 8: *((double*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos)) = data;
            break;
        default:
            break;
        }
        break;
    case ft_decimal:
    case ft_money: setFVDecimal(index, data);
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa: setFVNumeric(index, data);
        break;

    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_timestamp:
    case ft_bit:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp: i64 = (__int64)data;
        setFV(index, i64);
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        if (data == 0)
            setFVA(index, "");
        else
        {
            sprintf(buf, "%f", data);
            setFVA(index, buf);
            break;
        }
    case ft_lvar: break;
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
        {
            if (data == 0)
                setFV(index, _T(""));
            else
            {
                _TCHAR buf[40];
                _stprintf_s(buf, 40, _T("%f"), data);
                setFV(index, buf);
            }
            break;
        }
    }
}

void table::setFV(short index, bool data)
{
    int value = (int)data;
    setFV(index, value);
}

void table::setFV(short index, short data)
{
    int value = (int)data;
    setFV(index, value);
}

void table::setFV(short index, float data)
{
    double value = (double)data;
    setFV(index, value);

}

short table::getFVsht(short index) {return (short)getFVlng(index);}

int table::getFVlng(short index)
{
    int ret = 0;

    if (checkIndex(index) == false)
        return 0;
    fielddef& fd = m_tableDef->fieldDefs[index];
    switch (fd.type)
    {
    case ft_integer:
    case ft_autoinc:
        switch (fd.len)
        {
        case 1: ret = *(((char*)m_pdata + fd.pos));
            break;
        case 2: ret = *((short*)((char*)m_pdata + fd.pos));
            break;
        case 3: memcpy(&ret, (char*)m_pdata + fd.pos, 3);
            ret = ((ret & 0xFFFFFF) << 8) / 0x100;
            break;
        case 8:
        case 4: ret = *((int*)((char*)m_pdata + fd.pos));
            break;
        }
        break;
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_bit:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_mydatetime:
    case ft_mytimestamp:
        switch (fd.len)
        {
        case 1: ret = *((unsigned char*)((char*)m_pdata + fd.pos));
            break;
        case 2: ret = *((unsigned short*)((char*)m_pdata + fd.pos));
            break;
        case 3: memcpy(&ret, (char*)m_pdata + fd.pos, 3);
            break;
        case 8:
        case 4:
            ret = *((unsigned int*)((char*)m_pdata + fd.pos));
            break;
        }
        break;
    case ft_mydate:
        {
            myDate myd;
            myd.setValue((int)getValue64(fd, (const uchar_td*)m_pdata));
            ret = myd.getValue(m_impl->myDateTimeValueByBtrv);
            break;
        }
    case ft_mytime:
        {
            myTime myt(fd.len);
            myt.setValue((int)getValue64(fd, (const uchar_td*)m_pdata));
            ret = (int)myt.getValue(m_impl->myDateTimeValueByBtrv);
            break; ;
        }
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar: ret = atol(getFVAstr(index));
        break;
    case ft_wstring:
    case ft_wzstring:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar: ret = _ttol(getFVstr(index));
        break;
    case ft_currency: ret = (long)(*((__int64*)((char*)m_pdata + fd.pos)) / 10000);
        break;
    case ft_bfloat:
    case ft_float: ret = (long)getFVdbl(index);
        break;
    case ft_decimal:
    case ft_money: ret = (long)getFVDecimal(index);
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa: ret = (long)getFVnumeric(index);
        break;

    case ft_lvar: break;
    default: return 0;
    }
    return ret;
}

float table::getFVflt(short index) {return (float) getFVdbl(index);}

double table::getFVdbl(short index)
{
    double ret = 0;
    if (checkIndex(index) == false)
        return 0;
    switch (m_tableDef->fieldDefs[index].type)
    {
    case ft_currency:
        ret = (double) * ((__int64*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos));
        ret = ret / 10000;
        break;

    case ft_bfloat:
    case ft_timestamp:
    case ft_float:
        switch (m_tableDef->fieldDefs[index].len)
        {
        case 4: ret = (double) * ((float*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos));
            break;
        case 10: // long double
        case 8: ret = (double) * ((double*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos));
            break;
        }
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar: ret = atof(getFVAstr(index));
        break;
    case ft_wstring:
    case ft_wzstring:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar: ret = _ttof(getFVstr(index));
        break;
    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_bit:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp: ret = (double)getFV64(index);
        break;

    case ft_decimal:
    case ft_money: ret = getFVDecimal(index);
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa: ret = getFVnumeric(index);
        break;
    case ft_lvar: break;
    default: return 0;
    }
    return ret;
}

unsigned char table::getFVbyt(short index) {return (unsigned char)getFVlng(index);}

#ifdef _WIN32

const wchar_t* table::getFVWstr(short index)
{

    if (checkIndex(index) == false)
        return NULL;

    fielddef& fd = m_tableDef->fieldDefs[index];
    char* data = (char*)m_pdata + fd.pos;
    switch (fd.type)
    {
    case ft_string:
        return read<stringStore, char, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_note:
    case ft_zstring: return read<zstringStore, char, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_wzstring:
        return read<wzstringStore, WCHAR, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_wstring:
        return read<wstringStore, WCHAR, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_mychar:
        return read<myCharStore, char, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_myvarchar:
        return read<myVarCharStore, char, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_mywchar:
        return read<myWcharStore, WCHAR, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_myblob:
    case ft_mytext: return readBlob<WCHAR>(data, &m_impl->strBufs, fd, m_impl->cv);
    }
    wchar_t* p = (wchar_t*) m_impl->strBufs.getPtrW(max(fd.len * 2, 50));

	wchar_t buf[10] = L"%0.";

    switch (fd.type)
    {

    case ft_integer:
    case ft_bit:
    case ft_autoinc: _i64tow_s(getFV64(index), p, 50, 10);
        return p;
    case ft_logical:
        if (m_impl->logicalToString)
        {
            if (getFVlng(index))
                return L"Yes";
            else
                return L"No";
        }
        else
            _i64tow_s(getFV64(index), p, 50, 10);

    case ft_bfloat:
    case ft_float:
    case ft_currency:
        {

            swprintf_s(p, 50, L"%lf", getFVdbl(index));
			int k = (int)wcslen(p) - 1;
			while (k >= 0)
            {
                if (p[k] == L'0')
                    p[k] = 0x00;
                else if (p[k] == L'.')
                {
                    p[k] = 0x00;
                    break;
                }
				else
                    break;
                k--;
            }
            break;
        }
    case ft_autoIncUnsigned:
    case ft_uinteger:
        swprintf_s(p, 50, L"%lu", getFV64(index));
        break;
    case ft_date: return btrdtoa(getFVlng(index), p);
    case ft_time: return btrttoa(getFVlng(index), p);
    case ft_mydate:
        {
            myDate d;
            d.setValue((int)getValue64(fd, (const uchar_td*)m_pdata));
            return d.toStr(p, m_impl->myDateTimeValueByBtrv);
        }
    case ft_mytime:
        {

            myTime t(fd.len);
            t.setValue(getValue64(fd, (const uchar_td*)m_pdata));
            return t.toStr(p);

        }
    case ft_mydatetime:
        {
            myDateTime t(fd.len);
            t.setValue(getFV64(index));
            return t.toStr(p);
        }
    case ft_mytimestamp:
        {
            myTimeStamp ts(fd.len);
            ts.setValue(getFV64(index));
            return ts.toStr(p);
        }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa: _ltow_s(fd.decimals, p, 50, 10);
        wcscat(buf, p);
        wcscat(buf, L"lf");
        swprintf(p, 50, buf, getFVdbl(index));
        break;
    case ft_lvar: return NULL;
    case ft_timestamp: return btrTimeStamp(getFV64(index)).toString(p);
    default: p[0] = 0x00;
    }
    return p;
}

const wchar_t* table::getFVWstr(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVWstr(index);

}
#endif //_WIN32

const char* table::getFVAstr(short index)
{
    char buf[10] = "%0.";

    if (checkIndex(index) == false)
        return NULL;
    fielddef& fd = m_tableDef->fieldDefs[index];

    char* data = (char*)m_pdata + fd.pos;
    switch (fd.type)
    {

    case ft_string:
        return read<stringStore, char, char>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_note:
    case ft_zstring: return read<zstringStore, char, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_wzstring:
        return read<wzstringStore, WCHAR, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_wstring:
        return read<wstringStore, WCHAR, char>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_mychar:
        return read<myCharStore, char, char>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_myvarchar:
        return read<myVarCharStore, char, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_mywchar:
        return read<myWcharStore, WCHAR, char>(data, &m_impl->strBufs, fd, m_impl->cv,
            m_impl->trimPadChar);
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, char>(data, &m_impl->strBufs, fd, m_impl->cv);
    case ft_myblob:
    case ft_mytext: return readBlob<char>(data, &m_impl->strBufs, fd, m_impl->cv);
    }
    char* p = m_impl->strBufs.getPtrA(max(fd.len * 2, 50));
    switch (fd.type)
    {
    case ft_integer:
    case ft_bit:
    case ft_autoinc: _i64toa_s(getFV64(index), p, 50, 10);
        return p;
    case ft_logical:
        if (m_impl->logicalToString)
        {
            if (getFVlng(index))
                return "Yes";
            else
                return "No";
        }
        else
            _i64toa_s(getFV64(index), p, 50, 10);
        break;
    case ft_bfloat:
    case ft_float:
    case ft_currency:
        {
            sprintf(p, "%lf", getFVdbl(index));
            size_t k = strlen(p) - 1;
            while (1)
            {
                if (p[k] == '0')
                    p[k] = 0x00;
                else if (p[k] == '.')
                {
                    p[k] = 0x00;
                    break;
                }
                else
                    break;
                k--;
            }
            break;
        }
    case ft_date: return btrdtoa(getFVlng(index), p);
    case ft_time: return btrttoa(getFVlng(index), p);
    case ft_autoIncUnsigned:
    case ft_uinteger:
        sprintf_s(p, 50, "%lu", getFV64(index));
        break;

    case ft_mydate:
        {
            myDate d;
            d.setValue((int)getValue64(fd, (const uchar_td*)m_pdata));
            return d.toStr(p, m_impl->myDateTimeValueByBtrv);
        }
    case ft_mytime:
        {
            myTime t(fd.len);
            t.setValue(getValue64(fd, (const uchar_td*)m_pdata));
            return t.toStr(p);
        }
    case ft_mytimestamp:
        {
            myTimeStamp ts(fd.len);
            ts.setValue(getFV64(index));
            return ts.toStr(p);
        }
    case ft_mydatetime:
        {
            myDateTime t(fd.len);
            t.setValue(getFV64(index));
            return t.toStr(p);
        }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa: _ltoa_s(fd.decimals, p, 50, 10);
        strcat(buf, p);
        strcat(buf, "lf");
        sprintf_s(p, 50, buf, getFVdbl(index));
        break;
    case ft_lvar: return NULL;
    case ft_timestamp: return btrTimeStamp(getFV64(index)).toString(p);
    default: p[0] = 0x00;
    }
    return p;
}

int table::getFVint(short index) {return (int)getFVlng(index);}

int table::getFVint(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return (int)getFVlng(index);

}

int table::getFVlng(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVlng(index);
}

const char* table::getFVAstr(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVAstr(index);
}

double table::getFVdbl(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVdbl(index);
}

unsigned char table::getFVbyt(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVbyt(index);
}

short table::getFVsht(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVsht(index);
}

float table::getFVflt(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVflt(index);
}

void table::setFV(const _TCHAR* FieldName, int data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

void table::setFVA(const _TCHAR* FieldName, const char* data)
{
    short index = fieldNumByName(FieldName);
    setFVA(index, data);
}

void table::setFV(const _TCHAR* FieldName, double data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

void table::setFV(const _TCHAR* FieldName, float data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

void table::setFV(const _TCHAR* FieldName, unsigned char data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

void table::setFV(const _TCHAR* FieldName, short data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

__int64 table::getFV64(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFV64(index);
}

void table::setFV(const _TCHAR* FieldName, __int64 data)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data);
}

__int64 table::getFV64(short index)
{
    if (checkIndex(index) == false)
        return 0;
    fielddef& fd = m_tableDef->fieldDefs[index];

    switch (fd.len)
    {
    case 8:
        switch (fd.type)
        {
        case ft_autoIncUnsigned:
        case ft_uinteger:
        case ft_integer:
        case ft_logical:
        case ft_autoinc:
        case ft_bit:
        case ft_currency:
        case ft_timestamp:
        case ft_mydatetime:
        case ft_mytimestamp: return (__int64) *((__int64*)((char*)m_pdata + fd.pos));
        }
        return 0;
    case 7:
    case 6:
    case 5:
        switch (fd.type)
        {
        case ft_mytime:
        case ft_mydatetime:
        case ft_mytimestamp:
            {
                __int64 v = 0;
                memcpy(&v, (char*)m_pdata + fd.pos, fd.len);
                return v;
            }
        }
        return 0;
    default:

        return (__int64) getFVlng(index);
    }

}

void table::setFV(short index, __int64 data)
{
    if (checkIndex(index) == false)
        return;

    fielddef& fd = m_tableDef->fieldDefs[index];
    switch (fd.len)
    {
    case 8:
        switch (fd.type)
        {
        case ft_autoIncUnsigned:
        case ft_uinteger:
        case ft_integer:
        case ft_logical:
        case ft_autoinc:
        case ft_bit:
        case ft_currency:
        case ft_mydatetime:
        case ft_mytimestamp: *((__int64*)((char*)m_pdata + fd.pos)) = data;
            break;
        case ft_timestamp:
            {
                btrDate d;
                d.i = getNowDate();
                btrTime t;
                t.i = getNowTime();
                *((__int64*)((char*)m_pdata + fd.pos)) = btrTimeStamp(d, t).i64;
                break;
            }
        case ft_float:
            {
                double d = (double)data;
                setFV(index, d);
                break;
            }
        }
        break;
    case 7:
    case 6:
    case 5:
        switch (fd.type)
        {
        case ft_mytime:
        case ft_mydatetime:
        case ft_mytimestamp: memcpy((char*)m_pdata + fd.pos, &data, fd.len);
            break;
        default: break;
        }
        break;
    default:
        {
            int value = (int)data;
            setFV(index, value);
            break;
        }
    }
}

void table::setFV(const _TCHAR* FieldName, const void* data, uint_td size)
{
    short index = fieldNumByName(FieldName);
    setFV(index, data, size);
}

/* if blob and text ,set binary data that is only set pointer. it is not copied.
 *  Caller must hold data until it sends to the server.
 */
void table::setFV(short index, const void* data, uint_td size)
{
    if (checkIndex(index) == false)
        return;

    fielddef& fd = m_tableDef->fieldDefs[index];

    switch (fd.type)
    {
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_lstring:
        {
            int sizeByte = fd.varLenBytes();
            size = std::min<uint_td>((uint_td)(fd.len - sizeByte), size);
            memset((char*)m_pdata + fd.pos, 0, fd.len);
            memcpy((char*)m_pdata + fd.pos, &size, sizeByte);
            memcpy((char*)m_pdata + fd.pos + sizeByte, data, size);
            break;
        }
    case ft_myblob:
    case ft_mytext:
        {
            int sizeByte = fd.len - 8;
            memset((char*)m_pdata + fd.pos, 0, fd.len);
            memcpy((char*)m_pdata + fd.pos, &size, sizeByte);
            memcpy((char*)m_pdata + fd.pos + sizeByte, &data, sizeof(char*));
            break;

        }
    default: m_stat = STATUS_FIELDTYPE_NOTSUPPORT; // this field type is not supported.
    }
}

void* table::getFVbin(const _TCHAR* FieldName, uint_td& size)
{
    short index = fieldNumByName(FieldName);
    return getFVbin(index, size);
}

/* offset is writen at data that is first address of data
 *  text is not converted to unicode form stored charset.
 */
void* table::getFVbin(short index, uint_td& size)
{
    if (checkIndex(index) == false)
        return NULL;

    fielddef& fd = m_tableDef->fieldDefs[index];

    switch (fd.type)
    {
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_lstring:
        {
            int sizeByte = fd.varLenBytes();
            size = 0;
            memcpy(&size, (char*)m_pdata + fd.pos, sizeByte);
            return (void*)((char*)m_pdata + fd.pos + sizeByte);
        }
    case ft_myblob:
    case ft_mytext:
        {
            int sizeByte = fd.len - 8;
            size = 0;
            memcpy(&size, (char*)m_pdata + fd.pos, sizeByte);
            char** ptr = (char**)((char*)m_pdata + fd.pos + sizeByte);
            return (void*)*ptr;
        }
    }
    return NULL;
}

bool table::checkIndex(short index)
{
    if ((index >= m_tableDef->fieldCount) || (index < 0))
    {
        m_stat = STATUS_INVARID_FIELD_IDX;
        return false;
    }
    return true;
}

double table::getFVDecimal(short index)
{
    unsigned char buf[20] = {0x00};
    char result[30] = {0x00};
    char n[10];
    int i;
    char* t;
    unsigned char sign;
    int len = m_tableDef->fieldDefs[index].len;
    result[0] = '+';
    memcpy(buf, (void*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos), len);
    sign = (unsigned char)(buf[len - 1] & 0x0F);
    buf[len - 1] = (unsigned char)(buf[len - 1] & 0xF0);
    for (i = 0; i < len; i++)
    {
        sprintf_s(n, 50, "%02x", buf[i]);
        strcat(result, n);
    }
    i = (int)strlen(result);

    if (sign == 13)
        result[0] = '-';
    result[i - 1] = 0x00;


    t = result + (m_tableDef->fieldDefs[index].len * 2) - m_tableDef->fieldDefs[index].decimals;
    strcpy((char*)buf, t);
    *t = '.';
    strcpy(t + 1, (char*)buf);
    return atof(result);
}

double table::getFVnumeric(short index)
{
    char* t;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    char* pdp = NULL;
    char i;
    char buf[20] = {0x00};
    char dummy[20];

    buf[0] = '+';
    strncpy(buf + 1, (char*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos),
        m_tableDef->fieldDefs[index].len);

    t = &(buf[m_tableDef->fieldDefs[index].len]);

    switch (m_tableDef->fieldDefs[index].type)
    {
    case ft_numeric: pdp = dp;
        break;
    case ft_numericsa: pdp = dpsa;
        break;
    case ft_numericsts: buf[0] = *t;
        *t = 0x00;
        break;
    }

    if (pdp)
    {
        for (i = 0; i < 21; i++)
        {
            if (*t == pdp[i])
            {
                if (i > 10)
                {
                    buf[0] = '-';
                    *t = (char)(i + 38);
                }
                else
                    *t = (char)(i + 48);
                break;
            }
        }
    }

    t = buf + strlen(buf) - m_tableDef->fieldDefs[index].decimals;
    strcpy(dummy, t);
    *t = '.';
    strcpy(t + 1, dummy);
    return atof(buf);
}

void table::setFVDecimal(short index, double data)
{ // Double  -> Decimal
    char buf[30] = "%+0";
    char dummy[30];
    int point;
    bool sign = false;
    unsigned char n;
    int i, k;
    int strl;
    bool offset = false; ;
    point = (m_tableDef->fieldDefs[index].len) * 2;
    _ltoa_s(point, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_tableDef->fieldDefs[index].decimals, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf(dummy, buf, data);
    if (dummy[0] == '-')
        sign = true;

    strl = (int)strlen(dummy + 1) - 1;
    if (strl % 2 == 1)
        strl = strl / 2;
    else
    {
        strl = strl / 2 + 1;
        offset = true;
    }
    memset(buf, 0, 30);
    k = 0;
    n = 0;
    point = (int)strlen(dummy + 1);
    if (strl <= m_tableDef->fieldDefs[index].len)
    {
        for (i = 1; i <= point; i++)
        {
            if ((dummy[i] == '-') || (dummy[i] == '.'));
            else
            {
                if (offset)
                {
                    n = (unsigned char)(n + dummy[i] - 48);
                    buf[k] = n;
                    offset = false;
                    k++;
                }
                else
                {
                    n = (unsigned char)(dummy[i] - 48);
                    n = (unsigned char)(n << 4);
                    offset = true; ;
                }
            }
        }
        if (sign)
            buf[k] += ((unsigned char)(n + 13));
        else
            buf[k] += ((unsigned char)(n + 12));
    }
    memcpy((void*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos), buf,
        m_tableDef->fieldDefs[index].len);

}

void table::setFVNumeric(short index, double data)
{ // Double  -> Numeric
    char buf[30] = "%+0";
    char dummy[30];
    int point;
    int n;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    bool sign = false;
    char* t;

    point = m_tableDef->fieldDefs[index].len + 1;

    _ltoa_s(point, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_tableDef->fieldDefs[index].decimals, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf(dummy, buf, data);
    if (dummy[0] == '-')
        sign = true;


    strcpy(buf, &dummy[point - m_tableDef->fieldDefs[index].decimals] + 1);
    dummy[point - m_tableDef->fieldDefs[index].decimals] = 0x00;
    strcat(dummy, buf);

    n = atol(&dummy[m_tableDef->fieldDefs[index].len]);
    if (sign)
        n += 10;
    t = dummy + 1;
    switch (m_tableDef->fieldDefs[index].type)
    {
    case ft_numeric: dummy[m_tableDef->fieldDefs[index].len] = dp[n];
        break;
    case ft_numericsa: dummy[m_tableDef->fieldDefs[index].len] = dpsa[n];
        break;
    case ft_numericsts:
        if (sign)
            strcat(dummy, "-");
        else
            strcat(dummy, "+");
        t += 1;
        break;
    }

    memcpy((void*)((char*)m_pdata + m_tableDef->fieldDefs[index].pos), t,
        m_tableDef->fieldDefs[index].len);
}

unsigned int table::getRecordHash()
{
    return hash((const char*)fieldPtr(0), datalen());
}

int table::bookMarksCount() const
{
    int ret;
    ret = m_impl->maxBookMarkedCount;
    return ret;
}

void table::moveBookmarksId(long id)
{
    long Point = (id - 1) * 6 + 2;

    if ((id <= m_impl->maxBookMarkedCount) && (m_impl->bookMarks))
        seekByBookmark(*((bookmark_td*)((char*)m_impl->bookMarks + Point)));
    else
        seekByBookmark(0);
}

short_td table::doBtrvErr(HWND hWnd, _TCHAR* retbuf)
{
    return nstable::tdapErr(hWnd, m_stat, m_tableDef->tableName(), retbuf);
}




//-------------------------------------------------------------------
//      class queryBase
//-------------------------------------------------------------------
typedef boost::escaped_list_separator<_TCHAR> esc_sep;
typedef boost::tokenizer<esc_sep
        ,std::_tstring::const_iterator
        ,std::_tstring> tokenizer;

void analyzeQuery(const _TCHAR* str
        , std::vector<std::_tstring>& selects
        , std::vector<std::_tstring>& where
        , std::vector<std::_tstring>& keyValues
        ,bool& nofilter)
{
    selects.clear();
    where.clear();
    keyValues.clear();
    esc_sep sep(_T('&'), _T(' '), _T('\''));
    std::_tstring s = str;
    tokenizer tokens(s, sep);
    nofilter = false;
    tokenizer::iterator it = tokens.begin();
    if (*it == _T("*"))
    {
        nofilter = true;
        return;
    }
    s = *it;
    boost::algorithm::to_lower(s);
    if (s == _T("select"))
    {
        tokenizer::iterator itTmp = it;
        s = *(++it);
        if (getFilterLogicTypeCode(s.c_str())==255)
        {
            esc_sep sep(_T('&'), _T(','), _T('\''));
            tokenizer fields(s, sep);
            tokenizer::iterator itf = fields.begin();
            while (itf != fields.end())
                selects.push_back(*(itf++));
            ++it;
        }else
            it = itTmp; // field name is select
    }
    if (it == tokens.end())
        return;
    s = *it;
    boost::algorithm::to_lower(s);
    bool enableWhere = true;
    if (s == _T("in"))
    {
        tokenizer::iterator itTmp = it;
        s = *(++it);
        if (getFilterLogicTypeCode(s.c_str())==255)
        {
            enableWhere = false;
            esc_sep sep(_T('&'), _T(','), _T('\''));
            tokenizer values(s, sep);
            tokenizer::iterator itf = values.begin();
            while (itf != values.end())
                keyValues.push_back(*(itf++));
        }else
            it = itTmp; // field name is in
    }
    if (enableWhere)
    {
        while (it != tokens.end())
            where.push_back(*(it++));
    }
}


struct impl
{
    impl():
    m_reject(1),m_limit(0),m_direction(table::findForword)
        ,m_nofilter(false){}

	int m_reject;
	int m_limit;
    table::eFindType m_direction;
    bool m_nofilter;
    mutable std::_tstring m_str;
    std::vector<std::_tstring> m_selects;
    std::vector<std::_tstring> m_wheres;
    std::vector<std::_tstring> m_keyValues;
};

queryBase::queryBase():m_impl(new impl){}

queryBase::~queryBase()
{
    delete m_impl;
}

void queryBase::reset()
{
    delete m_impl;
    m_impl = new impl;
}

void queryBase::clearSelectFields()
{
    m_impl->m_selects.clear();
}

void queryBase::addField(const _TCHAR* name)
{
    m_impl->m_selects.push_back(name);
    m_impl->m_nofilter = false;
}

void queryBase::addLogic(const _TCHAR* name, const _TCHAR* logic,  const _TCHAR* value)
{
    m_impl->m_keyValues.clear();
    m_impl->m_wheres.clear();
    m_impl->m_wheres.push_back(name);
    m_impl->m_wheres.push_back(logic);
    m_impl->m_wheres.push_back(value);
    m_impl->m_nofilter = false;

}

void queryBase::addLogic(const _TCHAR* combine, const _TCHAR* name
    , const _TCHAR* logic,  const _TCHAR* value)
{
    m_impl->m_wheres.push_back(combine);
    addLogic(name, logic, value);
}

void queryBase::addSeekKeyValue(const _TCHAR* value, bool reset)
{
    if (reset)
    {
        m_impl->m_wheres.clear();
        m_impl->m_keyValues.clear();
    }
    m_impl->m_keyValues.push_back(value);
    //m_impl->m_reject = 1;
    m_impl->m_nofilter = false;

}

void queryBase::clearSeekKeyValues()
{
    m_impl->m_keyValues.clear();
}

queryBase& queryBase::queryString(const TCHAR* str)
{
    analyzeQuery(str, m_impl->m_selects, m_impl->m_wheres, m_impl->m_keyValues
                                                ,m_impl->m_nofilter);
    return *this;
}

queryBase& queryBase::reject(int v)
{
    m_impl->m_reject = v;
    return *this;
}

queryBase& queryBase::limit(int v)
{
    m_impl->m_limit = v;
    return *this;
}

queryBase& queryBase::direction(table::eFindType v)
{
    m_impl->m_direction = v;
    return *this;
}

queryBase& queryBase::all()
{
    reset();
    m_impl->m_nofilter = true;
    return *this;
}

table::eFindType queryBase::getDirection() const {return m_impl->m_direction;}

std::_tstring escape_value(std::_tstring s)
{
    std::_tstring::iterator it = s.begin();
    for (int i=(int)s.size()-1;i>=0;--i)
    {
        if (s[i] == _T('&'))
            s.insert(s.begin()+i, _T('&'));
        else if (s[i] == _T('\''))
            s.insert(s.begin()+i, _T('&'));
    }
    return s;
}

const _TCHAR* queryBase::toString() const
{
    m_impl->m_str.clear();
    if (m_impl->m_nofilter)
        return _T("*");

    std::_tstring& s = m_impl->m_str;
    std::vector<std::_tstring>& selects = m_impl->m_selects;
    std::vector<std::_tstring>& wheres = m_impl->m_wheres;
    std::vector<std::_tstring>& keyValues = m_impl->m_keyValues;
    if (selects.size())
    {
        s = _T("select ");
        for (int i= 0;i < (int)selects.size();++i)
            s += selects[i] + _T(",");

        if (s.size())
            s.replace(s.size()-1,1, _T(" "));
    }

    for (size_t i= 0;i < wheres.size();i+=4)
    {
        s += wheres[i] + _T(" ") + wheres[i+1] + _T(" '")
             + escape_value(wheres[i+2]) + _T("' ");
        if (i+3 < wheres.size())
            s += wheres[i+3] + _T(" ");
    }

    if (keyValues.size())
    {
        s += _T("in ");
        for (size_t i= 0;i < keyValues.size();++i)
            s += _T("'") + escape_value(keyValues[i]) + _T("',");
    }
    if (s.size())
        s.erase(s.end() -1);


    return s.c_str();
}

int queryBase::getReject()const{return m_impl->m_reject;}

int queryBase::getLimit()const{return m_impl->m_limit;}

bool queryBase::isAll()const{return m_impl->m_nofilter;};

const std::vector<std::_tstring>& queryBase::getSelects() const {return m_impl->m_selects;}
const std::vector<std::_tstring>& queryBase::getWheres() const {return m_impl->m_wheres;}
const std::vector<std::_tstring>& queryBase::getSeekKeyValues() const{return m_impl->m_keyValues;}
}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
