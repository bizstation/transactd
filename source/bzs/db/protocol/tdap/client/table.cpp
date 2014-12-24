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

#include "table.h"
#include "field.h"
#include "fields.h"
#include "filter.h"
#include "database.h"
#include "bulkInsert.h"
#include <bzs/db/protocol/tdap/tdapRequest.h>
#include <bzs/rtl/strtrim.h>
#include <bzs/db/protocol/tdap/myDateTime.cpp>
#include <bzs/rtl/stringBuffers.h>
#include "stringConverter.h"
#include <boost/timer.hpp>

#pragma package(smart_init)

using namespace bzs::db;
using namespace bzs::rtl;

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

class recordCache;

struct tbimpl
{

    void* bookMarks;
    client::fields fields;
    pq_handle filterPtr;
    recordCache* rc;
    multiRecordAlocator* mraPtr;
    void* dataBak;
    void* smartUpDate;
    void* bfAtcPtr;
    void* optionalData;
    uint_td dataBufferLen;
    int bookMarksMemSize;
    int maxBookMarkedCount;
    char keybuf[MAX_KEYLEN];
    char keyNumIndex[128];

    struct
    {
        unsigned char exBookMarking : 1;
        unsigned char smartUpDateFlag : 1;
        unsigned char dataPacked : 1;
    };

    tbimpl(table& tb)
        : bookMarks(NULL), fields(tb), rc(NULL), mraPtr(NULL),
          dataBak(NULL), smartUpDate(NULL), bfAtcPtr(NULL), optionalData(NULL),
          dataBufferLen(0), bookMarksMemSize(0), maxBookMarkedCount(0),
          smartUpDateFlag(false), dataPacked(false)
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
    }
};

// ---------------------------------------------------------------------------
// class recordCache
// ---------------------------------------------------------------------------

unsigned int hash(const char* s, int len)
{
    unsigned int h = 0;
    for (int i = 0; i < len; i++)
        h = h * 137 + *(s + i);
    return h % 1987;
}

class recordCache
{
    table* m_tb;
    filter* m_filter;
    unsigned int m_row;
    unsigned int m_len;
    unsigned int m_unpackLen;
    unsigned int m_rowCount;
    bookmark_td m_bookmark;
    uchar_td* m_ptr;
    uchar_td* m_tmpPtr;
    blobHeader* m_hd;
    short_td m_seekMultiStat;
    int m_memblockType;

public:
    inline recordCache(table* tb) : m_tb(tb), m_memblockType(mra_first)
    {
        reset();
    }

    inline void reset()
    {
        m_filter = NULL;
        m_row = 0;
        m_rowCount = 0;
        m_ptr = NULL;
        m_len = 0;
        m_tmpPtr = NULL;
        m_memblockType = mra_first;
    }

    inline void setMemblockType(int v) { m_memblockType = v; }

    inline void moveNextRow(int bookmarkSize)
    {
        m_ptr += m_len;
        m_len = m_unpackLen = *((unsigned short*)m_ptr);
        m_ptr += DATASIZE_BYTE;
        if (bookmarkSize)
        {
            m_bookmark = *((bookmark_td*)(m_ptr));
            m_ptr += bookmarkSize;
        }
    }

    inline void moveBlobRow(int row)
    {
        // blob pointer is allready point to next row
        if (m_hd)
        {
            while (row - m_hd->curRow)
            {
                for (int j = 0; j < m_hd->fieldCount; ++j)
                    m_hd->nextField = (blobField*)m_hd->nextField->next();
                ++m_hd->curRow;
            }
        }
    }

    inline void hasManyJoinMra(int rowCount, uchar_td* data)
    {
        int rowOffset = 0;
        int row = 0; // zero start
        int count = 0;
        char* ptr = (char*)data + DATASIZE_BYTE; // rowCount
        unsigned short len = 0;
        for (int i = 0; i < (int)rowCount; ++i)
        {
            ptr += len;
            len = *((unsigned short*)ptr);
            ptr += DATASIZE_BYTE;

            // get sequential number
            int tmp = *((int*)(ptr));
            ptr += sizeof(int);
            // If len == 0 then next first record read error
            if ((len == 0) || (tmp != row))
            {
                if (count)
                {
                    m_tb->m_impl->mraPtr->duplicateRow(row + rowOffset, count);
                    rowOffset += count;
                }
                ++row;
                count = 0;
            }
            else if (i != 0)
                ++count;
        }
        if (count)
            m_tb->m_impl->mraPtr->duplicateRow(row + rowOffset, count);
    }

    inline void resetMra(filter* p, uchar_td* data, unsigned int totalSize,
                      const blobHeader* hd)
    {
        reset(p, data, totalSize, hd);
        if (m_rowCount)
        {
            multiRecordAlocator* mra = m_tb->m_impl->mraPtr;
            unsigned char* bd = NULL; //blob data
            if (m_filter->hasManyJoin())
                hasManyJoinMra(m_rowCount, data);
            size_t recordLen = m_filter->fieldSelected()
                                    ? m_filter->totalFieldLen()
                                    : m_tb->tableDef()->maxRecordLen;
            mra->init(m_rowCount, recordLen, m_memblockType, m_tb);
            if (hd)
                bd = mra->allocBlobBlock(hd->dataSize);       
            
            // copy each row data
            int bookmarkSize = m_filter->bookmarkSize();
            const tabledef* td = m_tb->tableDef();
            ushort_td fieldCount = m_filter->fieldCount();
            m_tmpPtr = mra->ptr(m_row, mra_current_block);
            int resultOffset = 0;

            while (m_row < m_rowCount)
            {
                if ((m_len == 0) && m_filter->isSeeksMode() && fieldCount)
                    mra->setInvalidRecord(m_row, true);
                else
                {
                    if (m_filter->fieldSelected())
                    {
                        uchar_td* fieldPtr = m_ptr;
                        resultOffset = 0;
                        int blobFieldNum = 0;
                        for (int i = 0; i < fieldCount; i++)
                        {
                            const fielddef& fd =
                                td->fieldDefs[m_filter->selectFieldIndexes()[i]];
                            if (fd.isBlob())
                            {
                                bd = fd.setBlobFieldPointer(m_tmpPtr + resultOffset, m_hd, bd, blobFieldNum++);
                                fieldPtr += fd.len;
                            }
                            else
                                fieldPtr += fd.unPackCopy(m_tmpPtr + resultOffset, fieldPtr);
                            resultOffset += fd.len;
                        }
                    }
                    else if (m_tb->valiableFormatType())
                    {
                        memset(m_tmpPtr, 0, td->maxRecordLen);
                        memcpy(m_tmpPtr, m_ptr, m_len);
                        m_unpackLen = m_tb->unPack((char*)m_tmpPtr, m_len);
                        m_tb->setBlobFieldPointer((char*)m_tmpPtr, m_hd);
                        resultOffset = m_unpackLen; 
                    }
                    else
                    {
                        memcpy(m_tmpPtr, m_ptr, m_len);
                        resultOffset = m_len;
                    }
                }
                ++m_row;
                moveNextRow(bookmarkSize);
                m_tmpPtr += resultOffset;
            }
        }
        //prebuilt next ead operation
        setMemblockType(mra_nextrows);
    }

    inline void reset(filter* p, uchar_td* data, unsigned int totalSize,
                      const blobHeader* hd)
    {
        m_filter = p;
        m_row = 0;
        m_rowCount = *((unsigned short*)data);
        if (m_rowCount)
        {
            m_ptr = data + DATASIZE_BYTE;
            m_len = m_unpackLen =
                *((unsigned short*)m_ptr); // Not include bookmark and size bytes.
        
            m_ptr += DATASIZE_BYTE;
            if (m_filter->bookmarkSize())
            {
                m_bookmark = *((bookmark_td*)(m_ptr));
                m_ptr += m_filter->bookmarkSize();
            }
            m_tmpPtr = data + totalSize;
            m_hd = const_cast<blobHeader*>(hd);
        }
    }

    inline const uchar_td* moveRow(int count)
    {
        // move row data address pointer in result buffer
        int bookmarkSize = m_filter->bookmarkSize();
        for (int i = 0; i < count; i++)
            moveNextRow(bookmarkSize);
        moveBlobRow(m_row);
       
        m_tb->m_fddefs->strBufs()->clear();
        multiRecordAlocator* mra = m_tb->m_impl->mraPtr;
        const tabledef* td = m_tb->tableDef();
        ushort_td fieldCount = m_filter->fieldCount();

        if ((m_len == 0) && m_filter->isSeeksMode() && fieldCount)
        {

            m_seekMultiStat = STATUS_NOT_FOUND_TI;
            memset(m_tmpPtr, 0, td->maxRecordLen);
            return m_tmpPtr;
        }
        else
        {
            m_seekMultiStat = 0;
            if (m_filter->fieldSelected())
            {
                int resultOffset = 0;
                uchar_td* fieldPtr = m_ptr;
                memset(m_tmpPtr, 0, td->maxRecordLen);
                for (int i = 0; i < fieldCount; i++)
                {
                    const fielddef& fd =
                        td->fieldDefs[m_filter->selectFieldIndexes()[i]];
                    resultOffset = fd.pos;
                    fieldPtr += fd.unPackCopy(m_tmpPtr + resultOffset, fieldPtr);
                }
            }
            else if (m_tb->valiableFormatType())
            {
                memset(m_tmpPtr, 0, td->maxRecordLen);
                memcpy(m_tmpPtr, m_ptr, m_len);
                m_unpackLen = m_tb->unPack((char*)m_tmpPtr, m_len);
            }
            else
                m_tmpPtr = m_ptr;

            m_tb->setBlobFieldPointer((char*)m_tmpPtr, m_hd);
            return m_tmpPtr;
        }
    }

    inline const uchar_td* setRow(unsigned int rowNum)
    {
        if (rowNum < m_rowCount)
        {
            unsigned int moveCount = rowNum - m_row;
            m_row = rowNum;
            return moveRow(moveCount);
        }
        return NULL;
    }

    inline unsigned int len() const { return m_unpackLen; };
    inline bookmark_td bookmarkCurRow() const { return m_bookmark; };
    inline int row() const { return m_row; }

    inline int rowCount() const { return m_rowCount; }
    inline bool isEndOfRow(unsigned int row) const
    {
        return (m_rowCount && (row == m_rowCount));
    }
    inline bool withinCache(unsigned int row) const
    {
        return (row < m_rowCount);
    }
    inline short_td seekMultiStat() { return m_seekMultiStat; }
};

table::table(nsdatabase* pbe) : nstable(pbe)
{
    m_fddefs = fielddefs::create();
    m_impl = new tbimpl(*this);

    m_impl->rc = new recordCache(this);

    m_keybuflen = MAX_KEYLEN;
    m_pdata = NULL;
    m_keybuf = &m_impl->keybuf[0];
    m_keynum = 0;
    
    if (isUseTransactd())
    {
        tdap::posblk* pbk = (tdap::posblk*)posblk();
        pbk->tb = this;
        pbk->allocFunc = DDBA;
    }
}

table::~table()
{
    delete m_impl->rc;
    m_fddefs->release();
    delete m_impl;
}

void* __STDCALL table::DDBA(client::table* tb, uint_td size)
{
    return tb->doDdba(size);
}

void* table::doDdba(uint_td size)
{
    if (m_impl->filterPtr)
        size += tableDef()->maxRecordLen;;
    return reallocDataBuffer(size);
}

void table::setMra(multiRecordAlocator* p)
{
    m_impl->mraPtr = p;
}

multiRecordAlocator* table::mra() const
{
    return m_impl->mraPtr;
}

uchar_td table::charset() const
{
    return m_tableDef->charsetIndex;
}

bool table::trimPadChar() const
{
    return m_fddefs->trimPadChar;
}

void table::setTrimPadChar(bool v)
{
    m_fddefs->trimPadChar = v;
}

bool table::usePadChar() const
{
    return m_fddefs->usePadChar;
}

void table::setUsePadChar(bool v)
{
    m_fddefs->usePadChar = v;
}

void* table::dataBak() const
{
    return m_impl->dataBak;
}

void* table::reallocDataBuffer(uint_td v)
{
    if ((m_impl->dataBak == NULL) || (m_impl->dataBufferLen < v))
    {
        v = v * 15 / 10;  // 1.5f
        if (m_impl->dataBak == NULL)
            m_impl->dataBak = (void*)malloc(v);
        else
            m_impl->dataBak = (void*)realloc(m_impl->dataBak, v);
        if (!m_impl->dataBak)
        {
            m_impl->dataBufferLen = 0;
            m_stat = STATUS_CANT_ALLOC_MEMORY;
        }else
            m_impl->dataBufferLen = v;
    }
    setData(m_impl->dataBak);
    return m_impl->dataBak;
}

int table::dataBufferLen() const
{
    return m_impl->dataBufferLen;
}

void* table::optionalData() const
{
    return m_impl->optionalData;
}

void table::setOptionalData(void* v)
{
    m_impl->optionalData = v;
}

bool table::myDateTimeValueByBtrv() const
{
    return m_fddefs->myDateTimeValueByBtrv;
}

bool table::logicalToString() const
{
    return m_fddefs->logicalToString;
}

void table::setLogicalToString(bool v)
{
    m_fddefs->logicalToString = v;
}

fields& table::fields()
{
    return m_impl->fields;
}

void table::setBookMarks(int StartId, void* Data, ushort_td Count)
{
    long size = (StartId + Count) * 6;

    if (!m_impl->bookMarks)
    {
        m_impl->bookMarks = malloc(BOOKMARK_ALLOC_SIZE);
        if (m_impl->bookMarks)
            m_impl->bookMarksMemSize = BOOKMARK_ALLOC_SIZE;
        else
        {
            m_stat = STATUS_CANT_ALLOC_MEMORY;
            return;
        }
    }

    if (m_impl->bookMarksMemSize < size)
    {

        m_impl->bookMarks =
            realloc(m_impl->bookMarks, size + BOOKMARK_ALLOC_SIZE);
        m_impl->bookMarksMemSize = size + BOOKMARK_ALLOC_SIZE;
    }
    if (m_impl->bookMarks)
    {
        if (StartId + Count - 1 > m_impl->maxBookMarkedCount)
            m_impl->maxBookMarkedCount = StartId + Count - 1;
        memcpy((void*)((char*)m_impl->bookMarks + ((StartId - 1) * 6)), Data,
               Count * 6);
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

    if (ret > 9500)
        return 9500;
    if (ret == 0)
        return 1;
    return ret;
}

uint_td table::doRecordCount(bool estimate, bool fromCurrent)
{
    uint_td result = 0;
    client::filter* filter = m_impl->filterPtr.get();


    if (filter)
    {
        struct smartChangePreparedId
        {
            ushort_td m_id;
            client::filter* m_filter;
            smartChangePreparedId(client::filter* filter)
                :m_filter(filter) 
            { 
                m_id = m_filter->preparedId(); 
                m_filter->setServerPreparedId(0);
            }

            ~smartChangePreparedId(){ m_filter->setServerPreparedId(m_id); }
        }changePreparedId(filter);
        
        short_td op = (filter->direction() == findForword)
                          ? TD_KEY_NEXT_MULTI
                          : TD_KEY_PREV_MULTI;

        if (tableDef()->keyCount == 0)
            op += TD_POS_NEXT_MULTI - TD_KEY_NEXT_MULTI; // KEY to POS
        short curStat = m_stat;
        m_impl->exBookMarking = true;
        ushort_td recCountOnce = 100;

        bookmark_td bm = bookmark();

        ushort_td tmpRejectCount = filter->rejectCount();
        ushort_td tmpRecCount = filter->recordCount();

        filter->setIgnoreFields(true);
        m_impl->maxBookMarkedCount = 0;
        if (fromCurrent)
            m_stat = curStat;
        else if (op == TD_KEY_NEXT_MULTI)
            seekFirst();
        else if (op == TD_KEY_PREV_MULTI)
            seekLast();
        else if (op == TD_POS_NEXT_MULTI)
            stepFirst();
        else if (op == TD_POS_PREV_MULTI)
            stepLast();

        filter->setMaxRows(recCountOnce);
        if (m_stat == 0)
        {
            filter->setPosTypeNext(false);
            boost::timer t;
            btrvGetExtend(op);
            int eTime = (int)(t.elapsed() * 1000);
            while ((m_stat == 0) || (m_stat == STATUS_LIMMIT_OF_REJECT) ||
                   (m_stat == STATUS_EOF) ||
                   (m_stat == STATUS_REACHED_FILTER_COND))
            {
                bool Complete = false;
                if ((m_stat == STATUS_EOF) ||
                    (m_stat == STATUS_REACHED_FILTER_COND))
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
                filter->setMaxRows(recCountOnce);
                result += *((ushort_td*)m_pdata/*m_impl->dataBak*/);
                setBookMarks(m_impl->maxBookMarkedCount + 1,
                             (void*)((char*)m_pdata/*m_impl->dataBak*/ + 2),
                             *((ushort_td*)m_pdata/*m_impl->dataBak*/));
                m_impl->maxBookMarkedCount = result;
                onRecordCounting(result, Complete);
                if (Complete)
                    break;
                t.restart();
                filter->setPosTypeNext(true);
                btrvGetExtend(op);
                eTime = (int)(t.elapsed() * 1000);
            }
        }

        short tmpStat = m_stat;
        filter->setIgnoreFields(false);
        filter->setMaxRows(tmpRecCount);

        if (bm)
            seekByBookmark(bm);
        m_impl->exBookMarking = false;
        m_stat = tmpStat;
        if (m_stat == STATUS_EOF)
            m_stat = 0;
    }
    else
        return nstable::doRecordCount(estimate, fromCurrent);

    return result;
}

void table::btrvGetExtend(ushort_td op)
{
    client::filter* filter = m_impl->filterPtr.get();
    
    // cacheing direction
    if (!filter->setDirectionByOp(op))
    {
        m_stat = 1;
        return ;
    }

    if (op >= TD_KEY_GE_NEXT_MULTI)
        m_keylen = writeKeyData();

    m_pdata = m_impl->dataBak;
    if (!filter->writeBuffer())
    {
        m_stat = STATUS_WARKSPACE_TOO_SMALL;
        return;
    }
    m_datalen = filter->exDataBufLen();

    tdap(op);
    if (m_stat && (m_stat != STATUS_LIMMIT_OF_REJECT) &&
        (m_stat != STATUS_REACHED_FILTER_COND) && (m_stat != STATUS_EOF))
    {
        m_impl->filterPtr->setStat(m_stat);
        return;
    }
    short stat = m_stat;
    if (!filter->isWriteComleted() && (stat == STATUS_REACHED_FILTER_COND))
        stat = STATUS_LIMMIT_OF_REJECT;

    const blobHeader* hd = blobFieldUsed() ? getBlobHeader() : NULL;
    if (m_impl->mraPtr)
    {
        m_impl->rc->resetMra(filter, (uchar_td*)m_pdata, m_datalen, hd);
        m_stat = stat;
        m_impl->filterPtr->setStat(stat);
    }
    else
    {
        m_impl->rc->reset(filter, (uchar_td*)m_pdata, m_datalen, hd);
        m_stat = stat;
        m_impl->filterPtr->setStat(stat);
        // There is the right record.
        if (m_impl->rc->rowCount() && (!m_impl->exBookMarking))
        {
            m_pdata = (void*)m_impl->rc->setRow(0);
            m_datalen = tableDef()->maxRecordLen;

            m_stat = m_impl->rc->seekMultiStat();
        }else if (!filter->isStatContinue())
            m_stat = STATUS_EOF;
    }
   
}

bool table::recordsLoop(ushort_td& op)
{
    client::filter* filter = m_impl->filterPtr.get();
    filter->setPosTypeNext(true);
    bool flag = (m_stat == STATUS_LIMMIT_OF_REJECT && (filter->rejectCount() == 0));
    if (m_impl->mraPtr && !flag)
    {
        flag = m_impl->filterPtr->isStatContinue();
        if (!flag)       
            m_stat = m_impl->filterPtr->translateStat();//finish
    }
    if (flag)
    {
        op = m_impl->filterPtr->isSeeksMode() ? 
                TD_KEY_SEEK_MULTI : (m_impl->filterPtr->direction() == table::findForword) ?
                        TD_KEY_NEXT_MULTI : TD_KEY_PREV_MULTI;
    }
    return flag;
}

void table::getRecords(ushort_td op)
{
    do
    {
        btrvGetExtend(op);
    }while (recordsLoop(op));

    if ((m_stat == STATUS_REACHED_FILTER_COND) ||
        (m_stat == STATUS_LIMMIT_OF_REJECT))
        m_stat = STATUS_EOF;
}

bookmark_td table::bookmarkFindCurrent() const
{

    if (!m_impl->rc->isEndOfRow(m_impl->rc->row()))
        return m_impl->rc->bookmarkCurRow();
    return 0;
}

inline bool checkStatus(short v)
{
    return ((v == STATUS_SUCCESS) || (v == STATUS_NOT_FOUND_TI) ||
            (v == STATUS_EOF));
}

bool table::doSeekMultiAfter(int row)
{
    const std::vector<short>& fields = m_impl->filterPtr->selectFieldIndexes();

    if (m_stat == STATUS_SUCCESS)
        onReadAfter();
    else if (!checkStatus(m_stat))
        return false;
    if (m_stat)
        m_impl->mraPtr->setInvalidRecord(row, true);
    else
    {
        uchar_td* dst = m_impl->mraPtr->ptr(row, mra_current_block);
        if (m_impl->filterPtr->fieldSelected())
        {
            int resultOffset = 0;

            for (int j = 0; j < (int)fields.size(); ++j)
            {
                fielddef* fd = &tableDef()->fieldDefs[fields[j]];
                fd->unPackCopy(dst + resultOffset,
                               ((uchar_td*)m_pdata) + fd->pos);
                dst += fd->len;
            }
        }
        else
            memcpy(dst, (uchar_td*)m_pdata, m_datalen);
    }
    return true;
}

void table::btrvSeekMulti()
{
    const std::vector<client::seek>& seeks = m_impl->filterPtr->seeks();
    const bool transactd = false;
    bool hasManyJoin = m_impl->filterPtr->hasManyJoin();

    m_impl->rc->reset();
    size_t recordLen = m_impl->filterPtr->fieldSelected()
                           ? m_impl->filterPtr->totalSelectFieldLen()
                           : tableDef()->maxRecordLen;
    if (!hasManyJoin)
        m_impl->mraPtr->init(seeks.size(), recordLen, mra_first, this);

    m_keylen = m_keybuflen;
    m_datalen = m_buflen;
    int type = mra_first;
    int rowOffset = 0;
    for (int i = 0; i < (int)seeks.size(); ++i)
    {
        // 100% need allocate each row
        m_impl->mraPtr->init(1, recordLen, type, this);
        type = mra_nextrows;
        seeks[i].writeBuffer((uchar_td*)m_impl->keybuf, true, transactd);
        if (hasManyJoin)
        {
            tdap((ushort_td)(TD_KEY_OR_AFTER));
            if (!checkStatus(m_stat))
                return;

            if (memcmp(seeks[i].data, m_impl->keybuf, seeks[i].len) == 0)
            {
                doSeekMultiAfter(0);
                while (m_stat == 0)
                {
                    tdap((ushort_td)(TD_KEY_NEXT));
                    if (!checkStatus(m_stat))
                        return;
                    if (memcmp(seeks[i].data, m_impl->keybuf, seeks[i].len) !=
                        0)
                        break;
                    m_impl->mraPtr->duplicateRow(i + rowOffset, 1);
                    ++rowOffset;
                    m_impl->mraPtr->removeLastMemBlock(i + rowOffset);
                    m_impl->mraPtr->init(1, recordLen, type, this);
                    doSeekMultiAfter(0);
                }
            }
            else
                m_impl->mraPtr->setInvalidRecord(0, true);
        }
        else
        {
            tdap((ushort_td)(TD_KEY_SEEK));
            if (!doSeekMultiAfter(i))
                return;
        }
    }
    m_stat = STATUS_EOF;
}

void table::doFind(ushort_td op, bool notIncCurrent)
{
    /*
    First, read from cache.
    If whole row readed from cache then select operation by m_impl->filterPtr->stat()

    */
    m_stat = 0;
    int row = m_impl->rc->row() + 1;

    if (m_impl->rc->withinCache(row) && (!m_impl->exBookMarking))
    { /* read from cache */

        /*Is direction same */
        if (!m_impl->filterPtr->checkFindDirection(op))
        {
            m_stat = 1;
            return;
        }
        m_pdata = (void*)m_impl->rc->setRow(row);
        m_stat = m_impl->rc->seekMultiStat();

        /*If seek multi error, set keyvalue for keyValueDescription*/
        if (m_stat != 0)
            setSeekValueField(row);

        // m_datalen = m_impl->rc->len();
        m_datalen = tableDef()->maxRecordLen;
    }
    else if (m_impl->rc->isEndOfRow(row))
    {
        /* whole row readed */
        /*Is direction same */
        if (!m_impl->filterPtr->checkFindDirection(op))
        {
            m_stat = 1;
            return;
        }
        if (m_impl->filterPtr->isStatContinue())
        {
            //continue reading
            m_impl->rc->setMemblockType(mra_nextrows);
            getRecords(op);
        }else
        {
            //finish
            m_stat = m_impl->filterPtr->translateStat();
            m_impl->filterPtr->setStat(0);
        }
    }
    else
    {
        //reading
        m_impl->filterPtr->setPosTypeNext(notIncCurrent);
        getRecords(op);
    }
}

bool table::doPrepare()
{
    m_stat = 0;
    if (!m_impl->filterPtr)
    {
        m_stat = STATUS_FILTERSTRING_ERROR;
        return false;
    }

    m_pdata = m_impl->dataBak;
    m_impl->filterPtr->setPreparingMode(true);
    m_impl->filterPtr->setPosTypeNext(true);
    if (!m_impl->filterPtr->writeBuffer())
    {
        m_stat = STATUS_WARKSPACE_TOO_SMALL;
        m_impl->filterPtr->setPreparingMode(false);
        return false;
    }
    m_datalen = m_impl->filterPtr->exDataBufLen();

    tdap((ushort_td)(TD_FILTER_PREPARE));
    m_impl->filterPtr->setPreparingMode(false);
    if (m_stat != STATUS_SUCCESS)
        return false;
    m_impl->filterPtr->setServerPreparedId(*((ushort_td*)m_pdata));
    return true;
}

void table::find(eFindType type)
{
    if (!m_impl->filterPtr)
    {
        m_stat = STATUS_FILTERSTRING_ERROR;
        return;
    }
    ushort_td op;

    if (m_impl->filterPtr->isSeeksMode())
    {
        m_impl->filterPtr->resetSeeksWrited();
        op = TD_KEY_SEEK_MULTI;
    }
    else
        op =
            (type == findForword) ? TD_KEY_GE_NEXT_MULTI : TD_KEY_LE_PREV_MULTI;

    if (isUseTransactd())
    {
        m_impl->rc->reset();
        doFind(op, true /*notIncCurrent*/);
    }
    else
    {
        if (op == TD_KEY_SEEK_MULTI)
        {

            if (m_impl->mraPtr)
                btrvSeekMulti();
            else
                m_stat = STATUS_FILTERSTRING_ERROR; // P.SQL not support
            // TD_KEY_SEEK_MULTI
            return;
        }
        if (type == findForword)
            seekGreater(true);
        else
            seekLessThan(true);
        if (m_stat == 0)
        {
            if (type == findForword)
                findNext(false);
            else
                findPrev(false);
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
            m_impl->filterPtr->setPosTypeNext(false);
            getRecords(TD_KEY_PREV_MULTI);
        }
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

void table::setPrepare(const pq_handle stmt)
{
    m_stat = 0;
    if (!stmt)
    {
        m_stat = STATUS_FILTERSTRING_ERROR;
        return;
    }
    m_impl->rc->reset();
    m_impl->exBookMarking = false;
    m_impl->maxBookMarkedCount = 0;
    if (m_impl->filterPtr != stmt)
        m_impl->filterPtr = stmt;
}

pq_handle table::setQuery(const queryBase* query, bool serverPrepare)
{

    m_stat = 0;
    m_impl->rc->reset();
    m_impl->exBookMarking = false;
    m_impl->maxBookMarkedCount = 0;
    m_impl->filterPtr.reset();
    if (query == NULL)
        return m_impl->filterPtr;

    if (!m_impl->filterPtr)
        m_impl->filterPtr.reset(filter::create(this), filter::release);
        
    if (!m_impl->filterPtr)
    {
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return m_impl->filterPtr;
    }

    try
    {
        bool ret = m_impl->filterPtr->setQuery(query);
        if (!ret)
            m_stat = STATUS_FILTERSTRING_ERROR;
        else
        {
            if (serverPrepare && isUseTransactd())
                ret = doPrepare();
        }
        if (!ret)
             m_impl->filterPtr.reset();
    }
    catch (...)
    {
        m_stat = STATUS_FILTERSTRING_ERROR;
        m_impl->filterPtr.reset();
    }
    return m_impl->filterPtr;
}

void table::setFilter(const _TCHAR* str, ushort_td RejectCount,
                      ushort_td CashCount, bool autoEscape)
{
    if (!str || (str[0] == 0x00))
        setQuery(NULL);
    else
    {
        queryBase q;
        q.bookmarkAlso(true);
        q.queryString(str, autoEscape).reject(RejectCount).limit(CashCount);
        setQuery(&q);
    }
}

void table::clearBuffer()
{
    m_impl->rc->reset();
    m_pdata = m_impl->dataBak;
    memset(m_pdata, 0x00, m_buflen);
    if ((bulkIns() == NULL) && blobFieldUsed())
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

    keySpec* ks = (keySpec*)malloc(sizeof(keySpec) * segmentCount);
    memset(ks, 0, sizeof(keySpec) * segmentCount);
    getKeySpec(ks, SpecifyKeyNum);
    m_pdata = ks;
    m_datalen = sizeof(keySpec) * segmentCount;
    nstable::doCreateIndex(SpecifyKeyNum);
    m_pdata = m_impl->dataBak;
    free(ks);
}

void table::smartUpdate()
{
    if (!m_impl->smartUpDate)
        m_impl->smartUpDate = malloc(m_tableDef->maxRecordLen);
    if (m_impl->smartUpDate)
    {
        memcpy(m_impl->smartUpDate, data(), m_tableDef->maxRecordLen);
        m_impl->smartUpDateFlag = true;
    }
    else
        m_impl->smartUpDateFlag = false;
}

bool table::isUniqeKey(char_td keynum)
{
    if ((keynum >= 0) && (keynum < m_tableDef->keyCount))
    {
        keydef* kd = &m_tableDef->keyDefs[m_keynum];
        return !(kd->segments[0].flags.bit0);
    }
    return false;
}

bool table::onUpdateCheck(eUpdateType type)
{
    // Check uniqe key
    if (type == changeInKey)
    {
        if (!isUniqeKey(m_keynum))
        {
            m_stat = STATUS_INVALID_KEYNUM;
            return false;
        }
        else
        {
            if (isUseTransactd() == false)
            {
                // backup update data
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
        if (memcmp(m_impl->smartUpDate, data(), m_tableDef->maxRecordLen) == 0)
        {
            m_impl->smartUpDateFlag = false;
            return false;
        }
    }
    return true;
}

void table::onUpdateAfter(int beforeResult)
{
    if (blobFieldUsed())
        addSendBlob(NULL);

    if (valiableFormatType() && m_impl->dataPacked)
        m_datalen = unPack((char*)m_pdata, m_datalen);
}

bool table::onDeleteCheck(bool inkey)
{
    client::database* db = static_cast<client::database*>(nsdb());
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
        addSendBlob(NULL);
    return ret;
}

void table::doAbortBulkInsert()
{
    nstable::doAbortBulkInsert();
    if (blobFieldUsed())
        addSendBlob(NULL);
}

void table::onInsertAfter(int beforeResult)
{
    if (valiableFormatType() && m_impl->dataPacked)
        m_datalen = unPack((char*)m_pdata, m_datalen);
    if ((bulkIns() == NULL) && blobFieldUsed())
        addSendBlob(NULL);
}

void* table::attachBuffer(void* NewPtr, bool unpack, size_t size)
{
    void* oldptr;
    if (!m_impl->bfAtcPtr)
        m_impl->bfAtcPtr = m_pdata;
    oldptr = m_pdata;
    m_pdata = NewPtr;
    ushort_td len = m_tableDef->maxRecordLen; 
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

void table::init(tabledef* Def, short fnum, bool regularDir)
{
    doInit(Def, fnum, regularDir);
}

void table::doInit(tabledef* Def, short fnum, bool /*regularDir*/)
{
    m_tableDef = Def;
    m_fddefs->addAllFileds(m_tableDef);
    m_fddefs->cv()->setCodePage(mysql::codePage(m_tableDef->charsetIndex));
    ushort_td len = m_tableDef->maxRecordLen;
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
    reallocDataBuffer(len);
    m_buflen = len;
    m_datalen = len;
    setTableid(fnum);
}

keylen_td table::writeKeyDataTo(uchar_td* to, int keySize)
{
    if (m_tableDef->keyCount)
    {
        keydef& keydef =
            m_tableDef->keyDefs[(short)m_impl->keyNumIndex[m_keynum]];
        uchar_td* start = to;
        if (keySize == 0)
            keySize = keydef.segmentCount;

        for (int j = 0; j < keySize; j++)
        {
            int fdnum = keydef.segments[j].fieldNum;
            fielddef& fd = m_tableDef->fieldDefs[fdnum];
            uchar_td* from = (uchar_td*)m_pdata + fd.pos;
            to = fd.keyCopy(to, from);
        }
        return (keylen_td)(to - start);
    }
    return 0;
}

keylen_td table::writeKeyData()
{
    return writeKeyDataTo((uchar_td*)m_impl->keybuf, 0);
}

uint_td table::pack(char* ptr, size_t size)
{
    char* pos = ptr;
    char* end = pos + size;
    int movelen;
    for (int i = 0; i < m_tableDef->fieldCount; i++)
    {
        fielddef& fd = m_tableDef->fieldDefs[i];
        if (fd.type == ft_myfixedbinary)
        {
            memmove(pos + 2, pos, fd.len - 2); // move as size pace in the field
            *((unsigned short*)(pos)) = fd.len - 2; // fixed size
            pos += fd.len;
        }
        else
        {
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
    }
    m_impl->dataPacked = true;
    return (uint_td)(pos - ptr);
}

uint_td table::doGetWriteImageLen()
{
    if (!blobFieldUsed() && !valiableFormatType() &&
        (m_tableDef->flags.bit0 == false))
        return m_tableDef->maxRecordLen;
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
        return pack((char*)m_pdata, m_tableDef->maxRecordLen);
    else
    {
        fielddef* fd = &m_tableDef->fieldDefs[m_tableDef->fieldCount - 1];
        size_t len = 0;
        short* pos;

        if (fd->type == ft_note)
            len = strlen((char*)fieldPtr((short)(m_tableDef->fieldCount - 1))) +
                  1;
        else if (fd->type == ft_lvar)
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
            len = fd->len;

        len += fd->pos;

        return (uint_td)len;
    }
}

uint_td table::unPack(char* ptr, size_t size)
{
    char* pos = ptr;
    const char* end = pos + size;
    const char* max = pos + m_tableDef->maxRecordLen;
    int movelen;
    for (int i = 0; i < m_tableDef->fieldCount; i++)
    {
        fielddef& fd = m_tableDef->fieldDefs[i];
        if (fd.type == ft_myfixedbinary)
        {
            int dl = *((unsigned short*)(pos));
            assert(dl == fd.len - 2);
            memmove(pos, pos + 2, dl);
            pos += dl;
            *((unsigned short*)(pos)) = 0x00;
            ;
            pos += 2;
        }
        else
        {
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
                char* src = pos + dl;
                memmove(pos + fd.len, src, end - src);
                memset(src, 0, movelen);
                end += movelen;
            }
            pos += fd.len;
        }
    }
    m_impl->dataPacked = false;
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
    m_fddefs->blobClear();
}

void table::addSendBlob(const blob* blob)
{
    short stat = m_stat;
    /*backup current data buffer*/
    const void* tmp = data();
    setData((void*)blob);
    tdap(TD_ADD_SENDBLOB);
    /*restore data buffer*/
    setData((void*)tmp);
    m_stat = stat;
}

const blobHeader* table::getBlobHeader()
{
    short stat = m_stat;
    const blobHeader* p;
    /*backup current data buffer*/
    const void* tmp = data();
    setData(&p);
    tdap(TD_GET_BLOB_BUF);
    /*restore data buffer*/
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
    m_fddefs->strBufs()->clear();
    if (valiableFormatType())
    {
        m_datalen = unPack((char*)m_pdata, m_datalen);
        if (m_datalen == 0)
            m_stat = STATUS_BUFFERTOOSMALL;
    }
    if (blobFieldUsed())
    {
        const blobHeader* hd = getBlobHeader();
        setBlobFieldPointer((char*)m_pdata, hd);
    }
    if (m_tableDef->maxRecordLen - m_datalen > 0)
        memset((char*)m_pdata + m_datalen, 0, m_tableDef->maxRecordLen - m_datalen);
}

short table::fieldNumByName(const _TCHAR* name)
{
    return m_fddefs->indexByName(name);
}

void* table::fieldPtr(short index) const
{
    if (!checkIndex(index))
        return NULL;
    return m_impl->fields[index].ptr();
}

void table::setFVA(short index, const char* data)
{
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFVA(data);
}

#ifdef _WIN32

void table::setFVW(short index, const wchar_t* data)
{
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFVW(data);
}

void table::setFVW(const _TCHAR* FieldName, const wchar_t* data)
{
    short index = fieldNumByName(FieldName);
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFVW(data);
}

#endif //_WIN32

void table::setFV(short index, unsigned char data)
{
    if (!checkIndex(index))
        return;
    int value = (long)data;
    setFV(index, value);
}

void table::setFV(short index, int data)
{
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFV(data);
}

void table::setFV(short index, double data)
{
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFV(data);
}

void table::setFV(short index, short data)
{
    if (!checkIndex(index))
        return;
    int value = (int)data;
    setFV(index, value);
}

void table::setFV(short index, float data)
{
    if (!checkIndex(index))
        return;
    double value = (double)data;
    setFV(index, value);
}

short table::getFVsht(short index)
{
    return (short)getFVlng(index);
}

int table::getFVlng(short index)
{
    if (!checkIndex(index))
        return 0;
    return m_impl->fields[index].getFVlng();
}

float table::getFVflt(short index)
{
    return (float)getFVdbl(index);
}

double table::getFVdbl(short index)
{
    if (!checkIndex(index))
        return 0;
    return m_impl->fields[index].getFVdbl();
}

unsigned char table::getFVbyt(short index)
{
    return (unsigned char)getFVlng(index);
}

#ifdef _WIN32

const wchar_t* table::getFVWstr(short index)
{
    if (!checkIndex(index))
        return NULL;
    return m_impl->fields[index].getFVWstr();
}

const wchar_t* table::getFVWstr(const _TCHAR* FieldName)
{
    short index = fieldNumByName(FieldName);
    return getFVWstr(index);
}
#endif //_WIN32

const char* table::getFVAstr(short index)
{
    if (index == -1)
        return NULL;
    return m_impl->fields[index].getFVAstr();
}

int table::getFVint(short index)
{
    return (int)getFVlng(index);
}

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
    if (!checkIndex(index))
        return 0;
    return m_impl->fields[index].getFV64();
}

void table::setFV(short index, __int64 data)
{
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFV(data);
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
    if (!checkIndex(index))
        return;
    m_impl->fields[index].setFV(data, size);
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
    if (!checkIndex(index))
        return NULL;
    return m_impl->fields[index].getFVbin(size);
}

bool table::checkIndex(short index) const
{
    if ((index >= m_tableDef->fieldCount) || (index < 0))
    {
        m_stat = STATUS_INVARID_FIELD_IDX;
        return false;
    }
    return true;
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

/* For keyValueDescription */
bool table::setSeekValueField(int row)
{
    const std::vector<client::seek>& keyValues = m_impl->filterPtr->seeks();
    keydef* kd = &tableDef()->keyDefs[keyNum()];
    if (keyValues.size() % kd->segmentCount)
        return false;
    // Check uniqe key
    if (kd->segments[0].flags.bit0)
        return false;

    const uchar_td* ptr = (const uchar_td*)keyValues[row].data;
    const uchar_td* data;
    ushort_td dataSize;
    if (ptr)
    {
        for (int j = 0; j < kd->segmentCount; ++j)
        {
            short filedNum = kd->segments[j].fieldNum;
            fielddef& fd = tableDef()->fieldDefs[filedNum];
            ptr = fd.getKeyValueFromKeybuf(ptr, &data, dataSize);
            if (data)
            {
                if (fd.maxVarDatalen())
                    setFV(filedNum, data, dataSize);
                else
                    memcpy(fieldPtr(filedNum), data, dataSize);
            }
            else
                setFV(filedNum, _T(""));
        }
    }
    else
        return false;
    return true;
}

void table::keyValueDescription(_TCHAR* buf, int bufsize)
{

    std::_tstring s;
    if (stat() == STATUS_NOT_FOUND_TI)
    {

        for (int i = 0; i < tableDef()->keyDefs[keyNum()].segmentCount; i++)
        {
            short fnum = tableDef()->keyDefs[keyNum()].segments[i].fieldNum;
            s += std::_tstring(tableDef()->fieldDefs[fnum].name()) + _T(" = ") +
                 getFVstr(fnum) + _T("\n");
        }
    }
    else if (stat() == STATUS_DUPPLICATE_KEYVALUE)
    {
        _TCHAR tmp[50];
        for (int j = 0; j < tableDef()->keyCount; j++)
        {
            _stprintf_s(tmp, 50, _T("[key%d]\n"), j);
            s += tmp;
            for (int i = 0; i < tableDef()->keyDefs[j].segmentCount; i++)
            {
                short fnum = tableDef()->keyDefs[j].segments[i].fieldNum;
                s += std::_tstring(tableDef()->fieldDefs[fnum].name()) +
                     _T(" = ") + getFVstr(fnum) + _T("\n");
            }
        }
    }

    _stprintf_s(buf, bufsize, _T("table:%s\nstat:%d\n%s"),
                tableDef()->tableName(), stat(), s.c_str());
}

short table::getCurProcFieldCount() const
{
    if (!m_impl->filterPtr || !m_impl->filterPtr->fieldSelected())
        return tableDef()->fieldCount;
    return (short)m_impl->filterPtr->selectFieldIndexes().size();
}

short table::getCurProcFieldIndex(short index) const
{
    if (!m_impl->filterPtr || !m_impl->filterPtr->fieldSelected())
        return index;
    return m_impl->filterPtr->selectFieldIndexes()[index];
}

//-------------------------------------------------------------------
//      class queryBase
//-------------------------------------------------------------------
typedef boost::escaped_list_separator<_TCHAR> esc_sep;
typedef boost::tokenizer<esc_sep, std::_tstring::const_iterator, std::_tstring>
    tokenizer;

void analyzeQuery(const _TCHAR* str, std::vector<std::_tstring>& selects,
                  std::vector<std::_tstring>& where,
                  std::vector<std::_tstring>& keyValues, bool& nofilter)
{
    esc_sep sep(_T('&'), _T(' '), _T('\''));
    std::_tstring s = str;
    std::_tstring tmp;
    tokenizer tokens(s, sep);

    tokenizer::iterator it = tokens.begin();
    if (it == tokens.end())
        return;
    if (*it == _T("*"))
    {
        nofilter = true;
        return;
    }
    tmp = *it;
    boost::algorithm::to_lower(tmp);
    if (tmp == _T("select"))
    {
        tokenizer::iterator itTmp = it;
        tmp = *(++it);
        if (getFilterLogicTypeCode(tmp.c_str()) == 255)
        {
            esc_sep sep(_T('&'), _T(','), _T('\''));
            tokenizer fields(tmp, sep);
            tokenizer::iterator itf = fields.begin();
            while (itf != fields.end())
                selects.push_back(*(itf++));
            ++it;
        }
        else
            it = itTmp; // field name is select
    }
    if (it == tokens.end())
        return;
    tmp = *it;
    boost::algorithm::to_lower(tmp);
    bool enableWhere = true;
    if (tmp == _T("in"))
    {
        tokenizer::iterator itTmp = it;
        tmp = *(++it);
        if (getFilterLogicTypeCode(tmp.c_str()) == 255)
        {
            enableWhere = false;
            esc_sep sep(_T('&'), _T(','), _T('\''));
            tokenizer values(tmp, sep);
            tokenizer::iterator itf = values.begin();
            while (itf != values.end())
                keyValues.push_back(*(itf++));
        }
        else
            it = itTmp; // field name is in
    }
    if (enableWhere)
    {
        while (it != tokens.end())
            where.push_back(*(it++));
    }
}

keyValuePtr::keyValuePtr(const void* p, ushort_td l, short typeStr)
    : len(l), type(typeStr)
{
    if (type & KEYVALUE_NEED_COPY)
    {
        _TCHAR* tmp = new _TCHAR[len + 1];
        _tcsncpy(tmp, (_TCHAR*)p, len);
        tmp[len] = 0x00;
        ptr = tmp;
    }
    else
        ptr = p;
}

keyValuePtr::~keyValuePtr()
{
    if (type & KEYVALUE_NEED_COPY)
        delete[](_TCHAR*)ptr;
}

struct impl
{
    impl()
        : m_reject(1), m_limit(0), m_joinKeySize(0),
          m_optimize(queryBase::none), m_direction(table::findForword),
          m_nofilter(false), m_withBookmark(false)
    {
    }

    mutable std::_tstring m_str;
    std::vector<std::_tstring> m_selects;
    std::vector<std::_tstring> m_wheres;
    std::vector<std::_tstring> m_keyValues;
    std::vector<keyValuePtr> m_keyValuesPtr;
    int m_reject;
    int m_limit;
    int m_joinKeySize;
    queryBase::eOptimize m_optimize;
    table::eFindType m_direction;
    bool m_nofilter;
    bool m_withBookmark;
};

queryBase::queryBase() : m_impl(new impl)
{
}

queryBase::queryBase(const queryBase& r) : m_impl(new impl(*r.m_impl))
{
}

queryBase& queryBase::operator=(const queryBase& r)
{
    if (this != &r)
    {
        *m_impl = *r.m_impl;
    }
    return *this;
}

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

void queryBase::addLogic(const _TCHAR* name, const _TCHAR* logic,
                         const _TCHAR* value)
{
    m_impl->m_keyValuesPtr.clear();
    m_impl->m_keyValues.clear();
    m_impl->m_wheres.clear();
    m_impl->m_wheres.push_back(name);
    m_impl->m_wheres.push_back(logic);
    m_impl->m_wheres.push_back(value);
    m_impl->m_nofilter = false;
}

void queryBase::addLogic(const _TCHAR* combine, const _TCHAR* name,
                         const _TCHAR* logic, const _TCHAR* value)
{
    m_impl->m_wheres.push_back(combine);
    m_impl->m_wheres.push_back(name);
    m_impl->m_wheres.push_back(logic);
    m_impl->m_wheres.push_back(value);
    m_impl->m_nofilter = false;
}

void queryBase::reserveSeekKeyValueSize(size_t v)
{
    m_impl->m_keyValues.reserve(v);
}

void queryBase::reserveSeekKeyValuePtrSize(size_t v)
{
    m_impl->m_keyValuesPtr.reserve(v);
}

void queryBase::addSeekKeyValue(const _TCHAR* value, bool reset)
{
    if (reset)
    {
        m_impl->m_wheres.clear();
        m_impl->m_keyValues.clear();
        m_impl->m_keyValuesPtr.clear();
    }
    m_impl->m_keyValues.push_back(value);
    // m_impl->m_reject = 1;
    m_impl->m_nofilter = false;
}

void queryBase::addSeekKeyValuePtr(const void* value, ushort_td len,
                                   short typeStr, bool reset)
{
    if (reset)
    {
        m_impl->m_wheres.clear();
        m_impl->m_keyValues.clear();
        m_impl->m_keyValuesPtr.clear();
    }
    m_impl->m_keyValuesPtr.push_back(keyValuePtr(value, len, typeStr));
    m_impl->m_nofilter = false;
}

void queryBase::clearSeekKeyValues()
{
    m_impl->m_keyValues.clear();
    m_impl->m_keyValuesPtr.clear();
}

std::_tstring escape_value(std::_tstring s)
{
    for (int i = (int)s.size() - 1; i >= 0; --i)
    {
        if (s[i] == _T('&'))
            s.insert(s.begin() + i, _T('&'));
        else if (s[i] == _T('\''))
            s.insert(s.begin() + i, _T('&'));
    }
    return s;
}

std::_tstring& escape_string(std::_tstring& s)
{
    bool begin = false;
    for (int i = 0; i < (int)s.size(); ++i)
    {
        if (s[i] == _T('&'))
        {
            s.insert(s.begin() + i, _T('&'));
            ++i;
        }
        else if (s[i] == _T('\''))
        {
            if (begin)
            {
                if ((i == (int)s.size() - 1) || (s[i + 1] == _T(' ')))
                    begin = false;
                else
                    s.insert(s.begin() + i, _T('&'));
                ++i;
            }
            else if ((i == 0) || (s[i - 1] == _T(' ')))
                begin = true;
            else
            {
                s.insert(s.begin() + i, _T('&'));
                ++i;
            }
        }
    }
    return s;
}

queryBase& queryBase::queryString(const _TCHAR* str, bool autoEscape)
{
    m_impl->m_selects.clear();
    m_impl->m_wheres.clear();
    m_impl->m_keyValues.clear();
    m_impl->m_nofilter = false;
    if (str && str[0])
    {
        std::_tstring s = str;
        boost::trim(s);
        if (autoEscape)
            escape_string(s);
        analyzeQuery(s.c_str(), m_impl->m_selects, m_impl->m_wheres,
                     m_impl->m_keyValues, m_impl->m_nofilter);
    }
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
    m_impl->m_selects.clear();
    m_impl->m_wheres.clear();
    m_impl->m_keyValues.clear();
    m_impl->m_nofilter = true;
    return *this;
}

queryBase& queryBase::optimize(queryBase::eOptimize v)
{
    m_impl->m_optimize = v;
    return *this;
}

queryBase::eOptimize queryBase::getOptimize() const
{
    return m_impl->m_optimize;
}

queryBase& queryBase::bookmarkAlso(bool v)
{
    m_impl->m_withBookmark = v;
    return *this;
}

bool queryBase::isBookmarkAlso() const
{
    return m_impl->m_withBookmark;
}

queryBase& queryBase::joinKeySize(int v)
{
    m_impl->m_joinKeySize = v;
    return *this;
}

int queryBase::getJoinKeySize() const
{
    return m_impl->m_joinKeySize;
}

table::eFindType queryBase::getDirection() const
{
    return m_impl->m_direction;
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
        for (int i = 0; i < (int)selects.size(); ++i)
            s += selects[i] + _T(",");

        if (s.size())
            s.replace(s.size() - 1, 1, _T(" "));
    }

    for (size_t i = 0; i < wheres.size(); i += 4)
    {
        if (i + 1 < wheres.size())
            s += wheres[i] + _T(" ") + wheres[i + 1];
        if (i + 2 < wheres.size())
            s += _T(" '") + escape_value(wheres[i + 2]) + _T("' ");
        if (i + 3 < wheres.size())
            s += wheres[i + 3] + _T(" ");
    }

    if (keyValues.size())
    {
        s += _T("in ");
        for (size_t i = 0; i < keyValues.size(); ++i)
            s += _T("'") + escape_value(keyValues[i]) + _T("',");
    }
    if (s.size())
        s.erase(s.end() - 1);

    return s.c_str();
}

int queryBase::getReject() const
{
    return m_impl->m_reject;
}

int queryBase::getLimit() const
{
    return m_impl->m_limit;
}

bool queryBase::isAll() const
{
    return m_impl->m_nofilter;
}

const std::vector<std::_tstring>& queryBase::getSelects() const
{
    return m_impl->m_selects;
}
const std::vector<std::_tstring>& queryBase::getWheres() const
{
    return m_impl->m_wheres;
}
const std::vector<std::_tstring>& queryBase::getSeekKeyValues() const
{
    return m_impl->m_keyValues;
}
const std::vector<keyValuePtr>& queryBase::getSeekValuesPtr() const
{
    return m_impl->m_keyValuesPtr;
}
short queryBase::selectCount() const
{
    return (short)m_impl->m_selects.size();
}

const _TCHAR* queryBase::getSelect(short index) const
{
    assert((index >= 0) && (index < (short)m_impl->m_selects.size()));
    return m_impl->m_selects[index].c_str();
}

short queryBase::whereTokens() const
{
    return (short)m_impl->m_wheres.size();
}

const _TCHAR* queryBase::getWhereToken(short index) const
{
    assert((index >= 0) && (index < (short)m_impl->m_wheres.size()));
    return m_impl->m_wheres[index].c_str();
}

void queryBase::setWhereToken(short index, const _TCHAR* v)
{
    assert((index >= 0) && (index < (short)m_impl->m_wheres.size()));
    m_impl->m_wheres[index] = v;
}

/* alias field name change to original field name */
void queryBase::reverseAliasName(const _TCHAR* alias, const _TCHAR* src)
{
    std::vector<std::_tstring>& selects = m_impl->m_selects;
    std::vector<std::_tstring>& wheres = m_impl->m_wheres;
    std::_tstring s;
    for (size_t i = 0; i < wheres.size(); i += 4)
    {
        if (wheres[i] == alias)
            wheres[i] = src;
        if (i + 2 < wheres.size())
        {
            s = src;
            s.insert(0, _T("["));
            s += _T("[");
            if (wheres[i + 2] == s)
                wheres[i + 2] = s;
        }
    }

    for (size_t i = 0; i < selects.size(); ++i)
        if (selects[i] == alias)
            selects[i] = src;
}

void queryBase::release()
{
    delete this;
}

queryBase* queryBase::create()
{
    return new queryBase();
}

int makeSupplyValues(const _TCHAR* values[], int size,
                         const _TCHAR* value, const _TCHAR* value1,
                         const _TCHAR* value2, const _TCHAR* value3,
                         const _TCHAR* value4, const _TCHAR* value5,
                         const _TCHAR* value6, const _TCHAR* value7,
                         const _TCHAR* value8, const _TCHAR* value9,
                         const _TCHAR* value10)
{
    if (size == 0) return 0;
    memset(values, sizeof(_TCHAR*), size);
    values[0] = value;
    if (size < 2 || !value1) return 1;
    values[1] = value1;
    if (size < 3 || !value2) return 2; 
    values[2] = value2;
    if (size < 4 || !value3) return 3; 
    values[3] = value3;
    if (size < 5 || !value4) return 4; 
    values[4] = value4;
    if (size < 6 || !value5) return 5; 
    values[5] = value5;
    if (size < 7 || !value6) return 6; 
    values[6] = value6;
    if (size < 8 || !value7) return 7; 
    values[7] = value7;
    if (size < 9 || !value8) return 8; 
    values[8] = value8;
    if (size < 10 || !value9) return 9; 
    values[9] = value9;
    if (size < 11 || !value10) return 10; 
    values[10] = value10;
    return 11;
}

bool supplyValues(pq_handle& filter, const _TCHAR* values[], int size)
{
    return filter->supplyValues(values, size);
}

bool supplyValue(pq_handle& filter, int index, const _TCHAR* v)
{
    return filter->supplyValue(index, v);
}

bool supplyValue(pq_handle& filter, int index, short v)
{
    return filter->supplyValue(index, v);
}

bool supplyValue(pq_handle& filter, int index, int v)
{
    return filter->supplyValue(index, v);
}

bool supplyValue(pq_handle& filter, int index, __int64 v)
{
    return filter->supplyValue(index, v);
}

bool supplyValue(pq_handle& filter, int index, float v)
{
    return filter->supplyValue(index, v);
}

bool supplyValue(pq_handle& filter, int index, double v)
{
    return filter->supplyValue(index, v);
}
/*
bool supplyInValues(pq_handle& filter, const _TCHAR* values[], size_t size, int segments)
{
    return filter->supplySeekValues(values, size, segments);
}*/

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
