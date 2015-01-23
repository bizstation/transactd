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


#define BOOKMARK_SIZE 4
#define DATASIZE_BYTE 2


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

/* Structure description of a filter data buffer

Normal Mode
=============================================================
-----------------------------------
header
-----------------------------------
    --- P.SQL ---
    len         2byte   total of databaffer
    type        2byte   "EG" or "UC"(Include Current record)
    ---  or (Transatcd) ---
    ilen        int:28  total of databaffer    
    itype       int:4   See define FILTER_CURRENT_TYPE_INC
    ---  end or ---
    rejectCount 2byte   reject count
    ---  or (sending supply value to server) ---
    preparedId  2byte   preparedId
    ---  end or ---
    logicalCount 2byte  logical count
-----------------------------------
filter
-----------------------------------
    --- begin repeat ---
    type        1byte   data type
    len         2byte   field length (compare value length)
    pos         2byte   field position (zero origin)
    logType     1byte   compare type (=,>,<,<>,>=,<=)
                        + 16 var type compare whole length
                        + 32 compare by asc
                        + 64 copare with other field
                        + 128 case insensitive
    opr         1byte   next type 
                        0 end
                        1 and 
                        2 or
                        +32 prepare placeholder (Only transactd)
    data        2 or n  field position (if compare type +64) or comapre value
    --- end repeat   ---
-----------------------------------
result
-----------------------------------
    maxRows     2byte   max record count
    fieldCount  2byte   field count of a record
    --- begin repeat ---
    len         2byte   field length
    pos         2byte   field position
    --- end repeat   ---
-----------------------------------

Seeks mode
=============================================================
-----------------------------------
header
-----------------------------------
    same as normal
-----------------------------------
seek values
-----------------------------------
    --- begin repeat (key data of one record)---
    data        2byte   length of key data (multi segments)
    len         n       key data
    --- end repeat   ---
-----------------------------------
result
-----------------------------------
    same as normal

Prepare execute mode
=============================================================
-----------------------------------
header
-----------------------------------
    itype   |= FILTER_TYPE_SUPPLYVALUE
    Onother is same as normal.
-----------------------------------
supply values
-----------------------------------
    --- begin repeat ---
    2byte length of value
    n     value
    --- end repeat   ---
-----------------------------------
result
-----------------------------------
    same as normal
*/

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
    
    unsigned char* writeBuffer(unsigned char* p)
    {
        int n = sizeof(resultField);
        memcpy(p, this, n);
        return p + n;
    }

    int size() const { return sizeof(resultField);}
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
    unsigned char* writeBuffer(unsigned char* p)
    {
        int n = sizeof(resultDef);
        memcpy(p, this, n);
        return p + n;
    }
    
    int size() const { return sizeof(resultDef); }
    
    friend class filter;
};

struct seek
{
    unsigned char* data;
    unsigned short len;

public:
    // setParam from keyValue
    bool setParam(uchar_td* buf, ushort_td keylen)
    {
        len = keylen;
        data = buf;
        return true;
    }

    int size(bool isTransactd) const 
    { 
        if (!isTransactd)
            return len;
        return sizeof(len) + len; 
    }

    unsigned char* writeBuffer(unsigned char* p, bool end,
                               bool isTransactd) const
    {
        int n = sizeof(len);
        if (isTransactd)
            memcpy(p, &len, n);
        else
            n = 0;
        memcpy(p + n, data, len);
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

    int size() const
    {
        return (int)((unsigned char*)&data - &type) + getDatalen();
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
        placeHolder = false;
        if ((logType != 255) && (fieldNum != -1))
        {
            bool ret = true;
            fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
            setFieldParam(fd);

            if (compField)
                ret = setCompFiled(tb, value); // value is field name
            else
            {
                if (_tcscmp(value, _T("?"))==0)
                    placeHolder = true;
                setValue(tb, value);
            }
            return ret;
        }
        return false;
    }

    unsigned char* writeBuffer(unsigned char* p, bool end, bool preparingMode) const
    {
        int n = (int)((unsigned char*)&data - &type);
        memcpy(p, this, n);
        if (end)
            *(p + n - 1) = eCend;
        if (preparingMode && placeHolder)
            *(p + n - 1) |= FILTER_COMBINE_PREPARE;    
        p += n;
        n = getDatalen();
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
            unsigned int ilen : 28;
            unsigned int itype : 4;
        };
    };

public:
    union
    {
        unsigned short rejectCount;
        unsigned short preparedId;
    };
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

    void setPositionType(bool incCurrent, bool isTransactd, int tp)
    {
        if (isTransactd)
        {
            itype = incCurrent ? FILTER_CURRENT_TYPE_INC
                               : FILTER_CURRENT_TYPE_NOTINC;
            itype |= tp;
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

    unsigned char* writeBuffer(unsigned char* p, unsigned short prepareId) const
    {
        int n = sizeof(header);
        memcpy(p, this, n);
        if (prepareId)
            memcpy(p + 4, &prepareId, sizeof(unsigned short));
        return p + n;
    }

    inline int size() const { return sizeof(header); }
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
    size_t m_seeksLimitIndex;
    std::vector<char> m_recordBackup;
    uchar_td* m_buftmp;

    int m_extendBuflen;
    short m_stat;
    ushort_td m_preparedId;
    table::eFindType m_direction;
    queryBase::eOptimize m_cachedOptimize;
    struct
    {
        bool m_ignoreFields : 1;
        bool m_seeksMode : 1;
        bool m_useOptimize : 1;
        bool m_withBookmark : 1;
        bool m_isTransactd : 1;
        bool m_hasManyJoin : 1;
        bool m_preparingMode : 1;
        bool m_ddba : 1;
        
    };

    struct bufSize
    {
        bufSize():logic(0), seeks(0), select(0),retRowSize(0) {}
        void clear() { logic = 0; seeks = 0; select = 0; retRowSize = 0;}
        int logic;
        int seeks;
        int select;
        int retRowSize;
    }bsize;

    inline int maxDataBuffer()
    {
        //return 2048; //Small buffer test
        return m_isTransactd ? TDAP_MAX_DATA_SIZE : BTRV_MAX_DATA_SIZE;
    }

    void addAllFields()
    {
        m_fields.resize(1);
        resultField& r = m_fields[0];
        r.len = (ushort_td)m_tb->tableDef()->maxRecordLen;
        r.pos = 0;
        bsize.select = r.size();
        bsize.retRowSize = r.len;
    }

    bool addSelect(resultField& r, const _TCHAR* name)
    {
        int fieldNum = r.setParam(m_tb, name);
        if (fieldNum != -1)
        {
            m_selectFieldIndexes.push_back(fieldNum);
            bsize.select += r.size();
            bsize.retRowSize += r.len;
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
        m_hd.logicalCount = (ushort_td)m_logics.size();
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
            bsize.logic += l.size();
            ++index;
        }
        return true;
    }

    uchar_td* reallocSeeksDataBuffer(size_t size)
    {
        if (m_seeksDataBuffer.size() < size)
            m_seeksDataBuffer.resize(size);        
        uchar_td* dataBuf = &m_seeksDataBuffer[0];
        return dataBuf;
    }

    inline const _TCHAR* c_str_v(const _TCHAR* v) const { return v; }

    inline const _TCHAR* c_str_v(const std::_tstring& v) const { return v.c_str(); }

    // Need covert data types
    template <class T>
    bool doSsetSeekValues(keydef* kd, int joinKeySize, const T& keyValues, size_t size, uchar_td* dataBuf )
    {
        autoBackup recb(m_tb, m_recordBackup);
        int index = 0;
        bsize.seeks = 0;
        for (size_t i = 0; i < size; i += joinKeySize)
        {
            for (int j = 0; j < joinKeySize; ++j)
                m_tb->setFV(kd->segments[j].fieldNum, c_str_v(keyValues[i + j]));
            seek& l = m_seeks[index];
            ushort_td len = m_tb->writeKeyDataTo(dataBuf, joinKeySize);
            if (!l.setParam(dataBuf, len))
                return false;
            bsize.seeks += l.size(m_isTransactd);
            dataBuf += len;
            ++index;
        }
        return true;
    }

    // no need covert data types
    bool doSsetSeekValues(keydef* kd, int joinKeySize, const std::vector<keyValuePtr>& keyValues, size_t size, uchar_td* dataBuf )
    {
        int index = 0;
        bsize.seeks = 0;
        fielddef* fds = m_tb->tableDef()->fieldDefs;
        for (size_t i = 0; i < size; i += joinKeySize)
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
            bsize.seeks += l.size(m_isTransactd);
            dataBuf = to;
            ++index;
        }
        return true;
    }
    
    bool prebuiltSeeks( keydef* kd, size_t size, const queryBase* q, int& keySize, uchar_td** dataBuf)
    {
        // Check specify key size is smoller than kd->segmentCount or equal
        if (keySize == 0)
            keySize = kd->segmentCount;
        else if (kd->segmentCount < keySize) 
                return false;
        if (size % keySize)
            return false;
        
        m_hasManyJoin = (kd->segmentCount != keySize) || kd->segments[0].flags.bit0;
        if (m_hasManyJoin)
            m_withBookmark = true;
        if (q && m_hasManyJoin && 
                !(q->getOptimize() & queryBase::joinHasOneOrHasMany))
            return false;
        m_seeks.resize(size / keySize);
        int maxKeylen = 0;
        for (int j = 0; j < keySize; ++j)
            maxKeylen +=
                m_tb->tableDef()->fieldDefs[kd->segments[j].fieldNum].len + 2;

        // alloc databuffer
        *dataBuf = reallocSeeksDataBuffer(maxKeylen * m_seeks.size());
        m_hd.rejectCount = 0;
        m_seeksMode = true;
        m_seeksWritedCount = 0;
        return true;
    }

    template <class vector_type>
    bool setSeeks(const vector_type& keyValues, const queryBase* q)
    {
        int keySize = q->getJoinKeySize();
        uchar_td* dataBuf;
        keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];
        
        if (!prebuiltSeeks(kd, keyValues.size(), q, keySize, &dataBuf))
             return false;
        
        if (!doSsetSeekValues(kd, keySize, keyValues, keyValues.size(), dataBuf))
            return false;

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
        m_cachedOptimize = q->getOptimize();

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
            recordLen += bsize.retRowSize;
        return recordLen;
    }

    int calcMaxResultRows()
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
                bsize.logic -= lb.size();
                bsize.logic -= la.size();
                lb.joinAfter(&la);
                bsize.logic += lb.size();
                m_logics.erase(m_logics.begin() + i + 1);
                m_hd.logicalCount--;
            }
        }
    }

    // use seeksMode only
    int calcLogicalCutsize(int oversize)
    {
        int cutsize = 0;
        for (size_t i = m_seeksLimitIndex - 1; i >= m_seeksWritedCount; --i)
        {
            cutsize += (int)m_seeks[i].size(m_isTransactd);
            if (oversize - cutsize < 0)
            {
                m_seeksLimitIndex = i;
                return cutsize;
            }
        }
        return 0;
    }

    // Calc send buffer size and set last index of seeksMode can sent (m_seeksLimitIndex).  
    int calcSendBuflen()
    {
        int len = m_hd.size();
        
        if (!m_preparedId)
        {
            len += m_ret.size();
            if (!m_ignoreFields)
            {
                len += bsize.select;
                m_ret.fieldCount = (ushort_td)m_fields.size();
            }else
                m_ret.fieldCount = 0;
        }
        if (m_seeksMode)
        {
            int maxRows = calcMaxResultRows();
            m_seeksLimitIndex = std::min<size_t>(maxRows, m_seeks.size() - m_seeksWritedCount) + m_seeksWritedCount;
            if (m_seeksWritedCount == 0 && m_seeksLimitIndex == m_seeks.size())
                len += bsize.seeks;
            else
                for (size_t i = m_seeksWritedCount; i < m_seeksLimitIndex; ++i)
                    len += m_seeks[i].size(m_isTransactd);
            
            if (len > maxDataBuffer())
                len -= calcLogicalCutsize(len - maxDataBuffer() + 1);

            m_ret.maxRows = m_hd.logicalCount = (ushort_td)(m_seeksLimitIndex - m_seeksWritedCount);    
            if (m_hasManyJoin)
                m_ret.maxRows = (unsigned short)std::min<int>(maxRows, USHRT_MAX);
        }
        else
        {
            len += bsize.logic;
            // change the max rows fit to a max buffer size 
            if (m_ret.maxRows == 0)
                m_ret.maxRows =
                    (unsigned short)std::min<int>(calcMaxResultRows(), USHRT_MAX);
            else if (resultBufferNeedSize() > maxDataBuffer())
                m_ret.maxRows = calcMaxResultRows(); 
        }
    
        return len;
    }

    bool allocDataBuffer(int len)
    {
        // m_hd.len = len;//lost 2byte data at transactd
        m_extendBuflen = (int)resultBufferNeedSize();
        if (fieldSelected() || m_tb->valiableFormatType())
            m_extendBuflen += m_tb->tableDef()->maxRecordLen;
        m_tb->reallocDataBuffer(m_ddba ? len : m_extendBuflen);
        return true;
    }


    int doWriteBuffer()
    {
#ifdef _DEBUG
        int tmpLen = calcSendBuflen();
#endif
        unsigned char* p = (unsigned char*)m_tb->dataBak();
        unsigned char* start = p;
        p = m_hd.writeBuffer(p, m_preparedId);
        if (m_seeksMode)
        {
            for (size_t i = m_seeksWritedCount; i < m_seeksLimitIndex; ++i)
                p = m_seeks[i].writeBuffer(p, (i == (m_seeksLimitIndex - 1)), true);
            m_seeksWritedCount += m_hd.logicalCount;
        }
        else
        {
            for (size_t i = 0; i < m_logics.size(); ++i)
                p = m_logics[i].writeBuffer(p, (i == (m_logics.size() - 1)), m_preparingMode);
        }
        if (!m_preparedId)
        {
            p = m_ret.writeBuffer(p);
            if (!m_ignoreFields)
            {
                for (size_t i = 0; i < m_fields.size(); ++i)
                    p = m_fields[i].writeBuffer(p);
            }
        }
        // write total length
        int len = (int)(p - start);
        m_hd.setLen(len, m_isTransactd);
        m_hd.writeBuffer(start, m_preparedId);

#ifdef _DEBUG
        assert(len == tmpLen);
#endif
        return len;
    }
    

    filter(table* tb)
        : m_tb(tb), m_seeksWritedCount(0), m_extendBuflen(0), m_stat(0),
          m_preparedId(0),m_ignoreFields(false), m_seeksMode(false),
          m_useOptimize(true),m_withBookmark(true), m_hasManyJoin(false),
          m_preparingMode(false),m_ddba(false)
    {
        m_isTransactd = m_tb->isUseTransactd();
        m_ddba = m_isTransactd;
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
        m_preparingMode = false;
        m_preparedId = 0;
        m_stat = 0;
        bsize.clear();
    }

    void clearSeeks() { m_seeks.clear(); m_seeksWritedCount = 0; }

    queryBase::eOptimize cachedOptimaize() const { return m_cachedOptimize; }

    void setPreparingMode(bool v){ m_preparingMode = v;}

    void setServerPreparedId(ushort_td v) { m_preparedId = v; }

    ushort_td preparedId() const { return m_preparedId; }

    bool setQuery(const queryBase* q)
    {
        m_stat = 0;
        bool ret = doSetFilter(q);
        if (m_placeHolderIndexes.size() && m_useOptimize)
            ret = false;
        if (!ret)
            cleanup();
        return ret;
    }

    bool supplyValues(const std::vector<std::_tstring>& values)
    {
        if (m_placeHolderIndexes.size() != values.size())
            return false;
        for (int i = 0;i< (int)values.size();++i)
            supplyValue(i, values[i].c_str());
        return true;
    }

    bool supplyValues(const _TCHAR* values[], int size)
    {
        if ((int)m_placeHolderIndexes.size() != size)
            return false;
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
                bsize.logic -= l.size();
                l.setValue(m_tb, value);
                bsize.logic += l.size();
                return true;
            }
        }
        return false;
    }

    template <class T>
    bool supplySeekValues(const T& values, size_t size, int keySize)
    {
        uchar_td* dataBuf;
        keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];

        if (!prebuiltSeeks(kd, size, NULL, keySize, &dataBuf))
             return false;

        if (!doSsetSeekValues(kd, keySize, values, size, dataBuf))
            return false;

        m_seeksMode = true;
        m_seeksWritedCount = 0;
        return true;
    }

    bool beginSupplySeekValues(size_t size, int keySize)
    {
        keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];

        if (!prebuiltSeeks(kd, size, NULL, keySize, &m_buftmp))
             return false;
        m_seeksMode = true;
        m_seeksWritedCount = 0;
        bsize.seeks = 0;
        return true;
    }
    
    bool supplySeekValue(const uchar_td* ptr[] , int len[], int keySize, int& index)
    {
        const tabledef* td = m_tb->tableDef();
        keydef* kd = &td->keyDefs[m_tb->keyNum()];
        fielddef* fds = td->fieldDefs;

        seek& l = m_seeks[index];
        uchar_td* to = m_buftmp;
        for (int j = 0; j < keySize; ++j)
        {
            fielddef& fd = fds[kd->segments[j].fieldNum];
            to = fd.keyCopy(to, (uchar_td*)ptr[j], len[j]);
        }
        if (!l.setParam(m_buftmp, (ushort_td)(to - m_buftmp)))
            return false;
        bsize.seeks += l.size(m_isTransactd);
        m_buftmp = to;
        ++index;
        return true;
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
        int type = 0;
        if (m_isTransactd)
        {
            if (m_preparedId)
                type |= FILTER_TYPE_SUPPLYVALUE;
            else if (m_preparingMode && (m_direction == table::findForword))
                type |= FILTER_TYPE_FORWORD;

            if (!m_withBookmark)
                type |= FILTER_CURRENT_TYPE_NOBOOKMARK;
            if (m_seeksMode)
                type |= FILTER_TYPE_SEEKS;
            
        }
        m_hd.setPositionType(incCurrent, m_isTransactd, type);
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

    bool writeBuffer()
    {
        // Preapare need not assigned seeks. 
        if (m_preparingMode && m_seeksMode)
            return false;
        if (!m_isTransactd) 
            m_preparingMode = false; 
        if (!m_preparedId)
            joinLogic();
    
        int len = calcSendBuflen();
        if (len > maxDataBuffer())
            return false; //Too many logics

        if (allocDataBuffer(len))
            return (doWriteBuffer(/*false*/) > 0);
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
    inline bool setDirection(table::eFindType v) 
    { 
        if (m_preparedId == 0)
            m_direction = v;
        return (m_direction == v); 
    }
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
        bool ret;
        if ((op == TD_KEY_LE_PREV_MULTI) || (op == TD_KEY_PREV_MULTI))
            ret = (direction() == table::findBackForword);
        else
            ret = (direction() == table::findForword);
        assert(ret == true);
        return ret;
    }

    bool setDirectionByOp(short op)
    {
        bool v =  ((op == TD_KEY_LE_PREV_MULTI) || (op == TD_KEY_PREV_MULTI) ||
                    (op == TD_POS_PREV_MULTI));
        return setDirection(v ? table::findBackForword : table::findForword);
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
