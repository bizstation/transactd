#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
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
#include "fields.h"
#include <assert.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

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

#define BOOKMARK_ALLOC_SIZE 40960
#define BOOKMARK_SIZE 4
#define DATASIZE_BYTE 2

#define BTRV_MAX_DATA_SIZE 57000
#define TDAP_MAX_DATA_SIZE 6291456 // 3Mbyte

/** Length of compare
 * if part of string or zstring then return strlen.
 */
inline uint_td compDataLen(const fielddef& fd, const uchar_td* ptr, bool part)
{
    uint_td length = fd.keyDataLen(ptr);
    if (part)
    {
        if ((fd.type == ft_string) || (fd.type == ft_zstring) ||
            (fd.type == ft_note))
            length = (uint_td)strlen((const char*)ptr);
        else if ((fd.type == ft_wstring) || (fd.type == ft_wzstring))
            length = (uint_td)wcslen((const wchar_t*)ptr);
    }
    return length;
}

inline bool verType(uchar_td type)
{
    if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) ||
        type == ft_lstring)
        return true;
    return false;
}

#pragma pack(push, 1)
pragma_pack1;

struct resultField
{
    unsigned short len;
    unsigned short pos;

    int setParam(table* tb, const _TCHAR* name)
    {
        short fieldNum = tb->fieldNumByName(name);
        if (fieldNum != -1)
        {
            fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
            len = fd->len;
            pos = fd->pos;
            return fieldNum;
        }
        return -1;
    }
    unsigned char* writeBuffer(unsigned char* p, bool estimate)
    {
        int n = sizeof(resultField);
        if (!estimate)
            memcpy(p, this, n);
        return p + n;
    }
};

struct resultDef
{
    resultDef() { reset(); }
    void reset()
    {
        maxRows = 0;
        fieldCount = 0;
    }
    unsigned short maxRows;
    unsigned short fieldCount;
    unsigned char* writeBuffer(unsigned char* p, bool estimate)
    {
        int n = sizeof(resultDef);
        if (!estimate)
            memcpy(p, this, n);
        return p + n;
    }
    friend class filter;
};

struct seek
{
    unsigned char* data;
    unsigned short len;

public:
    size_t getLength() { return sizeof(len) + len; }

    // setParam from keyValue
    bool setParam(uchar_td* buf, ushort_td keylen)
    {
        len = keylen;
        data = buf;
        return true;
    }

    unsigned char* writeBuffer(unsigned char* p, bool estimate, bool end,
                               bool isTransactd) const
    {
        int n = sizeof(len);
        if (!estimate)
        {
            if (isTransactd)
                memcpy(p, &len, n);
            else
                n = 0;
            memcpy(p + n, data, len);
        }
        else if (!isTransactd)
            n = 0;

        return p + n + len;
    }
};

/* logic decrare DLLLIB for test_tdclcpp Windows*/
struct DLLLIB logic
{
    unsigned char type;
    unsigned short len;
    unsigned short pos;
    unsigned char logType;
    char opr;
    unsigned char* data;
    unsigned int databuflen;
    short fieldNum;
    bool placeHolder;

public:
    logic() : data(NULL), databuflen(0), placeHolder(false) {}

    ~logic() { delete[] data;}

    /* Important for vector erase */
    logic& operator=(const logic& r)
    {
        if (this != &r)
        {
            if (data != r.data)
                delete[] data;
            type = r.type;
            len = r.len;
            pos = r.pos;
            logType = r.logType;
            opr = r.opr;
            data = r.data;
            databuflen = r.databuflen;
            fieldNum = r.fieldNum;
            placeHolder = r.placeHolder;
            const_cast<logic&>(r).data = NULL;  //Important for vector erase
        }
        return *this;
    }

    size_t getLength()
    {
        return (size_t)((unsigned char*)&data - &type) + getDatalen();
    }

    void setFieldParam(fielddef* fd)
    {
        type = fd->type;
        len = fd->len;
        pos = fd->pos;
    }

    int getDatalen() const
    {
        if (logType & CMPLOGICAL_FIELD)
            return 2;
        return len;
    }

    bool setCompFiled(table* tb, const _TCHAR* name)
    {
        short tmp = tb->fieldNumByName(name);
        if (tmp != -1)
        {
            allocBuffer(2);
            fielddef& fd = tb->tableDef()->fieldDefs[tmp];
            memcpy(data, &(fd.pos), 2);
            logType |= CMPLOGICAL_FIELD;
            return true;
        }
        return false;
    }

    unsigned char* allocBuffer(unsigned int size)
    {
        if (databuflen < size + 2)
        {
            if (data)
            {
                delete[] data;
                data = NULL;
            }
            databuflen = size + 2; 
        }
        if (!data)
            data = new unsigned char[databuflen];
        memset(data, 0, databuflen);
        return data;
    }

    template <class T>
    inline unsigned int valueLen(const T /*value*/,  unsigned int size)
    {
        return size;
    }

    inline unsigned int valueLen(const _TCHAR* value,  unsigned int /*size*/)
    {
        return (unsigned int)_tcslen(value);
    }

    template <class T>
    void setValue(table* tb, const T value)
    {
        if (logType & CMPLOGICAL_FIELD)
            return;
        fielddef fdd = tb->tableDef()->fieldDefs[fieldNum];
        fdd.pos = 0;
        uchar_td* buf = allocBuffer(fdd.len);
        field fd(buf, fdd, tb->m_fddefs);
        fd = value;
        bool part = fd.isCompPartAndMakeValue();
        int varlen = fdd.varLenByteForKey();
        int copylen = compDataLen(fdd, buf, part);
        len = varlen + copylen;
        if (fdd.blobLenBytes())
        {
            data = new unsigned char[len + 2];
            if (varlen)
                memcpy(data, buf, varlen);
            memcpy(data + varlen, fdd.keyData(buf), copylen);
            delete [] buf;
        }

        if (!part && (fdd.varLenBytes() || fdd.blobLenBytes()))
            logType |= CMPLOGICAL_VAR_COMP_ALL; // match complate
    }

    bool setParam(table* tb, const _TCHAR* name, const _TCHAR* type,
                  const _TCHAR* value, char combine, bool compField = false)
    {
        logType = getFilterLogicTypeCode(type);
        opr = combine;
        fieldNum = tb->fieldNumByName(name);
        if ((logType != 255) && (fieldNum != -1))
        {
            bool ret = true;
            fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
            setFieldParam(fd);

            placeHolder = false;
            if (compField)
                ret = setCompFiled(tb, value); // value is field name
            else
            {
                if (_tcscmp(value, _T("?"))==0)
                    placeHolder = true;
                else
                    setValue(tb, value);
            }
            return ret;
        }
        return false;
    }

    unsigned char* writeBuffer(unsigned char* p, bool estimate, bool end) const
    {
        int n = (int)((unsigned char*)&data - &type);
        if (!estimate)
        {
            memcpy(p, this, n);
            if (end)
                *(p + n - 1) = eCend;
        }
        p += n;

        n = getDatalen();
        if (!estimate)
            memcpy(p, data, n);
        return p + n;
    }

    bool canJoin(bool after)
    {
        bool flag = true;
        if (after)
            flag = (opr == 1);
        return (flag && (logType == 1) && (type != ft_zstring) &&
                !verType(type));
    }

    bool isNextFiled(logic* src) { return ((pos + len) == src->pos); }

    void joinAfter(logic* src)
    {
        assert(src);
        assert(src->data);
        //copy before
        databuflen = len + src->len + 2;
        unsigned char* tmp = new unsigned char[databuflen];
        memcpy(tmp, data, len);
        delete[] data;
        //join next
        memcpy(tmp + len, src->data, src->len);
        len += src->len;
        type = ft_string; // compare by memcmp
        opr = src->opr;
        data = tmp;
        delete [] src->data;
        src->data = NULL;
    }
};

struct header
{
private:
    union
    {
        struct
        {
            unsigned short len;
            char type[2];
        };
        struct
        {
            int ilen : 28;
            int itype : 4;
        };
    };

public:
    unsigned short rejectCount;
    unsigned short logicalCount;
    header() : len(0), rejectCount(1), logicalCount(0)
    {
        type[0] = 0x00;
        type[1] = 0x00;
    }

    void reset()
    {
        rejectCount = 1;
        logicalCount = 0;
        len = 0;
        type[0] = 0x00;
        type[1] = 0x00;
    }

    void setPositionType(bool incCurrent, bool withBookmark, bool supplyValue, bool isTransactd)
    {
        if (isTransactd)
        {
            itype = incCurrent ? FILTER_CURRENT_TYPE_INC
                               : FILTER_CURRENT_TYPE_NOTINC;
            if (!withBookmark)
                itype |= FILTER_CURRENT_TYPE_NOBOOKMARK;
            /*if (supplyValue)
                itype |= FILTER_TYPE_SUPPLYVALUE;*/
        }
        else
        {
            if (incCurrent)
            {
                type[0] = 'U';
                type[1] = 'C';
            }
            else
            {
                type[0] = 'E';
                type[1] = 'G';
            }
        }
    }

    int bookmarkSize(bool isTransactd) const
    {
        if (isTransactd)
            return (itype & FILTER_CURRENT_TYPE_NOBOOKMARK) ? 0 : BOOKMARK_SIZE;
        assert(type[0]);
        // if (type[1] == 'N') return 0;
        return BOOKMARK_SIZE;
    }

    bool positionTypeNext(bool isTransactd) const
    {
        if (isTransactd)
            return !(itype & FILTER_CURRENT_TYPE_INC);
        return (type[0] == 'E');
    }

    void setLen(int size, bool isTransactd)
    {
        if (isTransactd)
            ilen = size;
        else
            len = size;
    }

    unsigned char* writeBuffer(unsigned char* p, bool estimate) const
    {
        int n = sizeof(header);
        if (!estimate)
            memcpy(p, this, n);
        return p + n;
    }
};
#pragma pack(pop)
pragma_pop;

class autoBackup
{
    table* m_tb;
    char* m_buf;
    int m_len;
public:
    autoBackup(table* tb, std::vector<char>& b):m_tb(tb),
        m_len(m_tb->tableDef()->maxRecordLen)
    {
        b.resize(m_len);
        m_buf = &b[0]; 
        memcpy(m_buf, m_tb->fieldPtr(0), m_len);
    }

    ~autoBackup()
    {
        memcpy(m_tb->fieldPtr(0), m_buf, m_len);
    }
};


class filter
{
    table* m_tb;
    header m_hd;
    resultDef m_ret;
    std::vector<resultField> m_fields;
    std::vector<short> m_selectFieldIndexes;
    std::vector<logic> m_logics;
    std::vector<seek> m_seeks;
    std::vector<uchar_td> m_seeksDataBuffer;
    std::vector<short> m_placeHolderIndexes;
    size_t m_seeksWritedCount;
    size_t m_logicalLimitCount;
    std::vector<char> m_recordBackup;

    int m_extendBuflen;
    short m_stat;
    ushort_td m_preparedId;
    table::eFindType m_direction;
    struct
    {
        bool m_ignoreFields : 1;
        bool m_seeksMode : 1;
        bool m_useOptimize : 1;
        bool m_withBookmark : 1;
        bool m_isTransactd : 1;
        bool m_hasManyJoin : 1;
        
    };

    inline int maxDataBuffer()
    {
        return m_isTransactd ? TDAP_MAX_DATA_SIZE : BTRV_MAX_DATA_SIZE;
    }

    void addAllFields()
    {
        m_fields.resize(1);
        resultField& r = m_fields[0];
        r.len = (ushort_td)m_tb->tableDef()->maxRecordLen;
        r.pos = 0;
    }

    bool addSelect(resultField& r, const _TCHAR* name)
    {
        int fieldNum = r.setParam(m_tb, name);
        if (fieldNum != -1)
        {
            m_selectFieldIndexes.push_back(fieldNum);
            return true;
        }
        return false;
    }

    bool setSelect(const std::vector<std::_tstring>& selects)
    {
        m_fields.resize(selects.size());
        for (size_t i = 0; i < selects.size(); ++i)
        {
            if (!addSelect(m_fields[i], selects[i].c_str()))
                return false;
        }
        return true;
    }

    bool setWhere(const std::vector<std::_tstring>& where)
    {
        if (where.size() == 0)
            return true;
        if (where.size() < 3)
            return false;
        m_logics.resize(m_logics.size() + (where.size() + 1) / 4);

        int index = 0;
        for (size_t i = 0; i < where.size(); i += 4)
        {
            if (i + 2 >= where.size())
                return false;
            char combine = eCend;
            std::_tstring value = where[i + 2];
            bool compField = (value.size() && (value[0] == _T('[')));
            if (compField)
            {
                value.erase(value.begin());
                value.erase(value.end() - 1);
            }
            if (i + 3 < where.size())
            {
                std::_tstring s = where[i + 3];
                boost::algorithm::to_lower(s);
                if (s == _T("or"))
                    combine = eCor;
                else if (s == _T("and"))
                    combine = eCand;
                else
                    return false;
            }
            logic& l = m_logics[index];
            if (!l.setParam(m_tb, where[i].c_str(),
                            where[i + 1].c_str(), value.c_str(), combine, compField))
                return false;
            if (l.placeHolder)
                m_placeHolderIndexes.push_back(index);
            ++index;
        }
        return true;
    }

    uchar_td* reallocSeeksDataBuffer(size_t size)
    {
        if (m_seeksDataBuffer.size() < size)
            m_seeksDataBuffer.resize(size);        
        uchar_td* dataBuf = &m_seeksDataBuffer[0];
        memset(dataBuf, 0, size);
        return dataBuf;
    }

    bool doSsetSeekValues(keydef* kd, int joinKeySize, const std::vector<std::_tstring>& keyValues, uchar_td* dataBuf )
    {
        autoBackup recb(m_tb, m_recordBackup);
        int index = 0;
        for (size_t i = 0; i < keyValues.size(); i += joinKeySize)
        {
            for (int j = 0; j < joinKeySize; ++j)
                m_tb->setFV(kd->segments[j].fieldNum, keyValues[i + j].c_str());
            seek& l = m_seeks[index];
            ushort_td len = m_tb->writeKeyDataTo(dataBuf, joinKeySize);
            if (!l.setParam(dataBuf, len))
                return false;
            dataBuf += len;
            ++index;
        }
        return true;
    }

    bool doSsetSeekValues(keydef* kd, int joinKeySize, const std::vector<keyValuePtr>& keyValues, uchar_td* dataBuf )
    {
        int index = 0;
        fielddef* fds = m_tb->tableDef()->fieldDefs;
        for (size_t i = 0; i < keyValues.size(); i += joinKeySize)
        {
            seek& l = m_seeks[index];
            uchar_td* to = dataBuf;
            for (int j = 0; j < joinKeySize; ++j)
            {
                const keyValuePtr& v = keyValues[i + j];
                fielddef& fd = fds[kd->segments[j].fieldNum];
                to = fd.keyCopy(to, (uchar_td*)v.ptr, v.len);
            }
            if (!l.setParam(dataBuf, (ushort_td)(to - dataBuf)))
                return false;
            dataBuf = to;
            ++index;
        }
        return true;
    }

    template <class vector_type>
    bool setSeeks(const vector_type& keyValues, const queryBase* q)
    {
        int keySize = q->getJoinKeySize();
        // Check key values
        keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];
        int joinKeySize = kd->segmentCount;
        if (keySize != 0)
        {
            // Check specify key size is smoller than kd->segmentCount or equal
            if (kd->segmentCount < keySize)
                return false;
            joinKeySize = keySize;
            m_withBookmark = m_hasManyJoin =
                (kd->segmentCount != joinKeySize) || kd->segments[0].flags.bit0;

            // Check when m_hasManyJoin need set joinHasOneOrHasMany
            if (m_hasManyJoin &&
                !(q->getOptimize() & queryBase::joinHasOneOrHasMany))
                return false;
        }

        if (keyValues.size() % joinKeySize)
            return false;
        // Check uniqe key
        if (kd->segments[0].flags.bit0 && !queryBase::joinHasOneOrHasMany)
            return false;
        m_seeks.resize(keyValues.size() / joinKeySize);
        int maxKeylen = 0;
        for (int j = 0; j < joinKeySize; ++j)
            maxKeylen +=
                m_tb->tableDef()->fieldDefs[kd->segments[j].fieldNum].len + 2;

        // alloc databuffer
        uchar_td* dataBuf = reallocSeeksDataBuffer(maxKeylen * m_seeks.size());
        
        if (!doSsetSeekValues(kd, joinKeySize, keyValues, dataBuf))
            return false;

        m_seeksMode = true;
        m_seeksWritedCount = 0;
        return true;
    }

    bool doSetFilter(const queryBase* q)
    {
        cleanup();
        setRejectCount(q->getReject());
        setMaxRows(q->getLimit());
        m_direction = q->getDirection();
        m_useOptimize = ((q->getOptimize() & queryBase::combineCondition) ==
                         queryBase::combineCondition);
        m_withBookmark = q->isBookmarkAlso();
        

        if (q->isAll())
            addAllFields();
        else
        {
            if (q->getSelects().size() == 0)
                addAllFields();
            else if (!setSelect(q->getSelects()))
                return false;

            // seeks or where
            if (q->getSeekKeyValues().size() && q->getWheres().size())
                return false;

            if (q->getSeekKeyValues().size())
                return setSeeks(q->getSeekKeyValues(), q);
            else if (q->getSeekValuesPtr().size())
                return setSeeks(q->getSeekValuesPtr(), q);
            else if (q->getWheres().size())
                return setWhere(q->getWheres());
        }
        return true;
    }

    int resultRowSize(bool ignoreFields) const
    {

        int recordLen = m_hd.bookmarkSize(m_isTransactd) + DATASIZE_BYTE;
        if (!ignoreFields)
        {
            for (size_t i = 0; i < m_fields.size(); ++i)
                recordLen += m_fields[i].len;
        }
        return recordLen;
    }

    int calcMaxRows()
    {
        return maxDataBuffer() / resultRowSize(m_ignoreFields);
    }

    int resultBufferNeedSize()
    {
        return (m_ret.maxRows * resultRowSize(m_ignoreFields)) + DATASIZE_BYTE;
    }

    void joinLogic()
    {
        if (m_seeksMode || !m_useOptimize)
            return;

        for (int i = (int)m_logics.size() - 2; i >= 0; --i)
        {
            logic& la = m_logics[i + 1];
            logic& lb = m_logics[i];
            if (la.canJoin(false) && lb.canJoin(true) && lb.isNextFiled(&la))
            {
                lb.joinAfter(&la);
                m_logics.erase(m_logics.begin() + i + 1);
            }
        }
    }

    int doWriteBuffer(bool estimate)
    {
        unsigned char* p = (unsigned char*)m_tb->dataBak();
        unsigned char* start = p;

        m_hd.logicalCount = (ushort_td)m_logicalLimitCount;
        if (m_ignoreFields)
            m_ret.fieldCount = 0;
        else
            m_ret.fieldCount = (ushort_td)m_fields.size();

        size_t first = 0, last = m_logicalLimitCount;
        if (m_seeksMode)
        {
            first = m_seeksWritedCount;
            last = std::min<size_t>(calcMaxRows() + m_seeksWritedCount,
                                    m_logicalLimitCount);
            m_hd.rejectCount = 0;
            if (m_hasManyJoin)
                m_ret.maxRows = 0;
            else
                m_ret.maxRows = m_hd.logicalCount = (ushort_td)(last - first);
        }
        if (m_ret.maxRows == 0)
            m_ret.maxRows =
                (unsigned short)std::min<int>(calcMaxRows(), USHRT_MAX);

        p = m_hd.writeBuffer(p, estimate);
        if (m_seeksMode)
        {
            for (size_t i = first; i < last; ++i)
                p = m_seeks[i].writeBuffer(p, estimate, (i == (last - 1)),
                                           true);
            if (!estimate)
                m_seeksWritedCount += m_hd.logicalCount;
        }
        else
        {
            for (size_t i = first; i < last; ++i)
                p = m_logics[i].writeBuffer(p, estimate, (i == (last - 1)));
        }

        p = m_ret.writeBuffer(p, estimate);

        if (!m_ignoreFields)
        {
            for (size_t i = 0; i < m_fields.size(); ++i)
                p = m_fields[i].writeBuffer(p, estimate);
        }

        // write total length
        int len = (int)(p - start);
        if (!estimate)
        {
            m_hd.setLen(len, m_isTransactd);
            m_hd.writeBuffer(start, false);
        }
        return len;
    }

    // use seeksMode only
    int calcLogicalCutsize(int oversize)
    {
        int cutsize = 0;
        for (size_t i = m_hd.logicalCount - 1; i != 0; --i)
        {
            cutsize += (int)m_seeks[i + m_seeksWritedCount].getLength();
            if (oversize - cutsize < 0)
            {
                m_logicalLimitCount = i;
                return cutsize;
            }
        }
        return 0;
    }

    bool allocDataBuffer()
    {
        joinLogic();
        m_logicalLimitCount = m_seeksMode ? m_seeks.size() : m_logics.size();
        int len = doWriteBuffer(true);
        if (len > maxDataBuffer())
        {
            if (m_seeksMode)
                len -= calcLogicalCutsize(len - maxDataBuffer() + 1);
            else
                return false;
        }
        // m_hd.len = len;//lost 2byte data at transactd
        int resultLen = (int)resultBufferNeedSize();
        if (resultLen > maxDataBuffer())
        {
            /* change the max rows fit to a max buffer size */
            m_ret.maxRows = calcMaxRows();
            resultLen = resultBufferNeedSize();
        }

        m_extendBuflen = std::max<int>((int)len, resultLen);
        m_extendBuflen =
            std::max<int>(m_extendBuflen, m_tb->tableDef()->maxRecordLen);
        if (fieldSelected() || m_tb->valiableFormatType())
            m_extendBuflen += m_tb->tableDef()->maxRecordLen;

        if ((int)m_tb->dataBufferLen() < m_extendBuflen)
            m_tb->reallocDataBuffer(m_extendBuflen);
        return true;
    }

    filter(table* tb)
        : m_tb(tb), m_extendBuflen(0), m_stat(0), m_preparedId(0),
          m_ignoreFields(false), m_seeksMode(false), m_useOptimize(true),
          m_withBookmark(true), m_seeksWritedCount(0), m_hasManyJoin(false)
    {
        m_isTransactd = m_tb->isUseTransactd();
    }

    ~filter() {}

public:
    void cleanup()
    {
        m_selectFieldIndexes.clear();
        m_fields.clear();
        m_logics.clear();
        m_seeks.clear();
        m_hd.reset();
        m_ret.reset();
        m_placeHolderIndexes.clear();
        m_ignoreFields = false;
        m_seeksMode = false;
        m_seeksWritedCount = 0;
        m_useOptimize = true;
        m_hasManyJoin = false;
        m_preparedId = 0;
        m_stat = 0;
    }

    void setServerPrepared(ushort_td v) { m_preparedId = v; }

    bool setQuery(const queryBase* q)
    {
        m_stat = 0;
        bool ret = doSetFilter(q);
        if (!ret)
            cleanup();
        return ret;
    }

    bool supplyValues(const std::vector<std::_tstring>& values)
    {
        for (int i = 0;i< (int)values.size();++i)
            supplyValue(i, values[i].c_str());
        return true;

    }

    bool supplyValues(const _TCHAR* values[], int size)
    {
        for (int i = 0;i< size;++i)
            supplyValue(i, values[i]);
        return true;
    }

    template <class T>
    bool supplyValue(int placeHolderIndex, const T value)
    {
        if (placeHolderIndex < (int)m_placeHolderIndexes.size())
        {
            logic& l = m_logics[m_placeHolderIndexes[placeHolderIndex]];
            if (l.placeHolder)
            {
                l.setValue(m_tb, value);
                return true;
            }
        }
        return false;
    }

    bool isWriteComleted() const
    {
        if (!m_seeksMode)
            return true;
        return (m_seeksWritedCount == m_seeks.size());
    }

    inline bool hasManyJoin() const { return m_hasManyJoin; }

    inline void resetSeeksWrited() { m_seeksWritedCount = 0; }

    inline void setPositionType(bool incCurrent)
    {
        m_hd.setPositionType(incCurrent, m_withBookmark, (m_preparedId!=0), m_isTransactd);
    }

    inline bool positionTypeNext() const
    {
        return m_hd.positionTypeNext(m_isTransactd);
    }

    inline void setRejectCount(ushort_td v) { m_hd.rejectCount = v; }
    inline ushort_td rejectCount() const { return m_hd.rejectCount; }

    inline void setMaxRows(ushort_td v) { m_ret.maxRows = v; }
    inline ushort_td maxRows() const { return m_ret.maxRows; }

    inline ushort_td recordCount() const { return maxRows(); }

    inline void setPosTypeNext(bool v) { setPositionType(!v); }

    uint_td exDataBufLen() const
    {
        if (fieldSelected() || m_tb->valiableFormatType())
            return m_extendBuflen - m_tb->tableDef()->maxRecordLen;
        return m_extendBuflen;
    }

    inline ushort_td fieldCount() const { return m_ret.fieldCount; }

    inline void setFieldCount(ushort_td v) { m_ret.fieldCount = v; }

    inline ushort_td fieldLen(int index) const
    {
        assert(index < (int)m_fields.size());
        return m_fields[index].len;
    }

    ushort_td totalFieldLen() const
    {
        return resultRowSize(false) - m_hd.bookmarkSize(m_isTransactd) -
               DATASIZE_BYTE;
    }

    ushort_td totalSelectFieldLen() const
    {
        ushort_td recordLen = 0;
        for (size_t i = 0; i < m_fields.size(); ++i)
            recordLen += m_fields[i].len;
        return recordLen;
    }

    inline ushort_td fieldOffset(int index) const
    {
        assert(index < (int)m_fields.size());
        return m_fields[index].pos;
    }

    bool writeBuffer(bool prepare = false)
    {
        // Preapare need not assigned seeks. 
        if (prepare && m_seeksMode)
            return false;
        if (allocDataBuffer())
            return (doWriteBuffer(false) > 0);
        return false;
    }

    inline ushort_td extendBuflen() const { return m_extendBuflen; }

    bool fieldSelected() const
    {
        return !((m_fields.size() == 1) && (m_fields[0].pos == 0) &&
                 (m_fields[0].len ==
                  (ushort_td)m_tb->tableDef()->maxRecordLen));
    }

    inline bool ignoreFields() const { return m_ignoreFields; }

    inline int bookmarkSize() const { return m_hd.bookmarkSize(m_isTransactd); }

    /* The Ignore fields option don't use with multi seek operation.
       because if a server are not found a record then a server return
       error code in a bookmark field.
    */
    inline void setIgnoreFields(bool v) { m_ignoreFields = v; }
    inline bool isSeeksMode() const { return m_seeksMode; }
    inline table::eFindType direction() const { return m_direction; }
    inline void setDirection(table::eFindType v) { m_direction = v; }
    inline const std::vector<short>& selectFieldIndexes()
    {
        return m_selectFieldIndexes;
    }
    inline const std::vector<seek>& seeks() const { return m_seeks; }
    inline short stat() const { return m_stat; };
    inline void setStat(short v) { m_stat = v; }
    
    // convert for table stat
    inline short translateStat() const 
    {
        if ((m_stat == STATUS_LIMMIT_OF_REJECT) ||
            (m_stat == STATUS_REACHED_FILTER_COND))
            return STATUS_EOF;
        else
            return m_stat;
    }

    /* A special situation that if rejectCount() == 0 and status =
           STATUS_LIMMIT_OF_REJECT
                then it continues . */
    inline bool isStatContinue() const
    {
        if ((m_stat == 0) ||
           ((m_stat == STATUS_LIMMIT_OF_REJECT) && (rejectCount() == 0)))
           return true;
        return false;
    }

    bool checkFindDirection(ushort_td op)
    {
        bool ret=true;
        if ((op == TD_KEY_LE_PREV_MULTI) || (op == TD_KEY_PREV_MULTI))
            ret = (direction() == table::findBackForword);
        else
            ret = (direction() == table::findForword);
        if (!ret)
            assert(0);
        return ret;
    }

    void setDirectionByOp(short op)
    {
        bool v =  ((op == TD_KEY_LE_PREV_MULTI) || (op == TD_KEY_PREV_MULTI) ||
                    (op == TD_POS_PREV_MULTI));
        setDirection(v ? table::findBackForword : table::findForword);
    }

    static filter* create(table* tb)
    {
        return new filter(tb);
    }

    static void release(filter* p){ delete p; }
                
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
