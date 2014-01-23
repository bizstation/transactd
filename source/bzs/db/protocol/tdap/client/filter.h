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


inline ushort_td varlenForFilter(const fielddef& fd)
{
    if (((fd.type >= ft_myvarchar) && (fd.type <= ft_mywvarbinary)) || fd.type == ft_lstring)
        return fd.len < 256 ? 1 : 2;
    else if ((fd.type == ft_myblob) || (fd.type == ft_mytext))
        return fd.len - 8;
    return 0;
}

/** Length of compare
 * if part of string or zstring then return strlen.
 */
inline uint_td compDataLen(const fielddef& fd, const uchar_td* ptr, bool part)
{
    uint_td length = fd.keyDataLen(ptr);
    if (part)
    {
        if ((fd.type == ft_string) || (fd.type == ft_zstring) || (fd.type == ft_note))
            length = (uint_td)strlen((const char*)ptr);
        else if ((fd.type == ft_wstring) || (fd.type == ft_wzstring))
            length = (uint_td)wcslen((const wchar_t*)ptr);
    }
    return length;
}

bool verType(uchar_td type)
{
    if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || type == ft_lstring)
        return true;
    return false;
}


#pragma option -a-
pragma_pack1

struct resultField
{
	unsigned short len;
    unsigned short pos;

    bool setParam(table* tb, const _TCHAR* name)
    {
        short fieldNum = tb->fieldNumByName(name);
        if (fieldNum != -1)
        {
            fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
            len = fd->len;
            pos = fd->pos;
            return true;
        }
        return false;
    }
    unsigned char* writeBuffer(unsigned char* p, bool estimate)
    {
        int n = sizeof(resultField);
        if (!estimate) memcpy(p, this, n);
        return p+n;
    }
};

struct resultDef
{
    resultDef()
    {
        reset();
    }
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
        if (!estimate) memcpy(p, this, n);
        return p + n;
    }
    friend class filter;
};

struct logic
{
	unsigned char	type;
	unsigned short	len;
	unsigned short	pos;
	unsigned char	logType;
	char			opr;
    unsigned char*  data;

public:
    logic():data(NULL){}

    ~logic()
    {
        delete [] data;
    }

    void setFieldParam(fielddef* fd )
    {
        type = fd->type;
        len = fd->len;
        pos = fd->pos;
    }

    int getDatalen() const
    {
        if (logType & 64)
            return 2;
        return len;
    }

    bool setCompFiled(table* tb, short index, const _TCHAR* name)
    {
         short tmp = tb->fieldNumByName(name);
         if (tmp !=-1)
         {
            memcpy(data, &tmp, 2);
            logType |= CMPLOGICAL_FIELD;
            return true;
         }
         return false;
    }

    bool isPart(table* tb, short index)
    {
        fielddef* fd = &tb->tableDef()->fieldDefs[index];
        bool ret = false;
        if (fd->isStringType())
        {
             _TCHAR* p = (_TCHAR*)tb->getFVstr(index);
            if (p)
            {
                size_t n = _tcslen(p);
                if ((ret = (p[n-1] == _T('*')))!=0)
                {
                    p[n-1] = 0x00;
                    tb->setFV(index, p);
                }
            }else
                tb->setFV(index, _T(""));
        }
        return ret;
    }

    void allocBuffer(int size)
    {
         if (data)
            delete [] data;
         data = new unsigned char[size + 2];
         memset(data, 0, size + 2);
    }

    void copyToBuffer(table* tb, short index, bool part)
    {
        fielddef* fd = &tb->tableDef()->fieldDefs[index];
        const uchar_td* ptr = (const uchar_td*)tb->fieldPtr(index);
        int varlen = varlenForFilter(*fd);
        int copylen = compDataLen(*fd, ptr, part);
        len = varlen + copylen;
        allocBuffer(len);
        uchar_td* to = (uchar_td*)data;
        if (varlen)
            memcpy(to, ptr, varlen);
        memcpy(to + varlen, fd->keyData(ptr), copylen);

        if (!part && (fd->varLenBytes() || fd->blobLenBytes()))
            logType |= CMPLOGICAL_VAR_COMP_ALL; //match complate
    }

    bool setParam(table* tb, const _TCHAR* name
                        , const _TCHAR* type, const _TCHAR* value, char combine, bool compField = false)
    {
        logType = getFilterLogicTypeCode(type);
        opr = combine;
        short fieldNum = tb->fieldNumByName(name);
        if ((logType!=255) && (fieldNum != -1))
        {
            bool ret = true;
            fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
            setFieldParam(fd);
            unsigned char* tmp = new unsigned char[len];

            //backup
            memcpy(tmp, tb->fieldPtr(fieldNum), len);

            if (compField)
                ret = setCompFiled(tb, fieldNum, value);// value is field name
            else
            {
                tb->setFV(fieldNum, value);
                bool part = isPart(tb, fieldNum);
                copyToBuffer(tb, fieldNum, part);
            }
            //restore
            memcpy(tb->fieldPtr(fieldNum),tmp, len);
            delete [] tmp;
            return ret;
        }
        return false;
    }

    unsigned char* writeBuffer(unsigned char* p, bool estimate)
    {
        int n = sizeof(logic) - sizeof(unsigned char*);
        if (!estimate) memcpy(p, this, n);
        p += n;
        n = getDatalen();
        if (!estimate) memcpy(p, data, n);
        return p + n;
    }

    bool canJoin(bool after)
    {
        bool flag = true;
        if (after)
            flag = (opr == 1);
        return (flag
                && (logType == 1)
                && (type != ft_zstring)
                && !verType(type));
    }

    bool isNextFiled(logic* src)
    {
        return ((pos + len) ==  src->pos);
    }

    void joinAfter(logic* src)
    {
        assert(src);
        assert(src->data);
        unsigned char* tmp = data;
        data = new unsigned char[len + src->len + 2];
        memcpy(data, tmp, len);
        memcpy(data + len, src->data, src->len);
        len += src->len;
        type = ft_string; //compare by memcmp
        if (src->opr == eCend)
            opr = eCend;
        delete [] tmp;
    }
};

struct header
{
	unsigned short	len;
	char			type[2];
	unsigned short	rejectCount;
	unsigned short	logicalCount;
    header(){reset();};
    void reset()
    {
        rejectCount = 1;
        logicalCount = 0;
        len = 0;
        setPositionType(true);
    }

    void setPositionType(bool incCurrent)
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

    bool positionTypeNext() const
    {
        return (type[0] == 'E');
    }

    unsigned char* writeBuffer(unsigned char* p, bool estimate)
    {
        int n = sizeof(header);
        if (!estimate) memcpy(p, this, n);
        return p + n;
    }
};
#pragma option -a
pragma_pop



class filter
{

    table* m_tb;
    header m_hd;
    resultDef m_ret;
    std::vector<resultField*> m_fields;
    std::vector<logic*> m_logics;
    int m_extendBuflen;
    std::_tstring m_str;
    bool m_ignoreFields;


    bool addWhere(const _TCHAR* name, const _TCHAR* type, const _TCHAR*  value, char combine, bool compField = false)
    {
        logic* l = new logic();
        if (l->setParam(m_tb, name, type, value, combine, compField))
        {
            m_logics.push_back(l);
            return true;
        }
        delete l;
        return false;
    }

    void addAllFields()
    {
        resultField* r = new resultField();
        r->len = (ushort_td) m_tb->tableDef()->maxRecordLen;
        r->pos = 0;
        m_fields.push_back(r);
    }

    bool addSelect(const _TCHAR* name)
    {
        resultField* r = new resultField();
        if (r->setParam(m_tb, name))
        {
            m_fields.push_back(r);
            return true;
        }
        delete r;
        return false;
    }

    bool setSelect(const std::vector<std::_tstring>& selects)
    {
        for (size_t i=0;i < selects.size();++i)
        {
            if (!addSelect(selects[i].c_str()))
                return false;
        }
        return true;
    }

    bool setWhere(const std::vector<std::_tstring>& where)
    {
        if (where.size() == 0) return true;
        if (where.size() < 3) return false;

        for (size_t i=0;i<where.size();i+=4)
        {
            char combine = eCend;
            std::_tstring value = where[i+2];
            bool compField = (value[0] == _T('['));
            if (compField)
            {
                value.erase(value.begin());
                value.erase(value.end() - 1);
            }
            if (i+3 < where.size())
            {
                std::_tstring s = where[i+3];
                boost::algorithm::to_lower(s);
                if (s == _T("or")) combine = eCor;
                else if (s == _T("and"))
                    combine = eCand;
                else
                    return false;
            }
            if (!addWhere(where[i].c_str(), where[i+1].c_str()
                    , value.c_str(), combine, compField))
                return false;
        }
        return true;

    }

    bool doSetFilter(const queryBase* q)
    {
        cleanup();
        setRejectCount(q->getReject());
        setMaxRows(q->getLimit());

        if (q->isNofilter())
            addAllFields();
        else
        {
            if (q->getSelects().size() == 0)
                addAllFields();
            else if (!setSelect(q->getSelects()))
                return false;
            return setWhere(q->getWheres());
        }
        return true;
    }

    ushort_td resultRowSize(bool ignoreFields)
    {

        ushort_td recordLen = BOOKMARK_SIZE + DATASIZE_BYTE;
        if (!ignoreFields)
        {
            for (size_t i=0;i< m_fields.size();++i)
                recordLen += m_fields[i]->len;
        }
        return recordLen;
    }

    ushort_td calcMaxRows()
    {
        return (ushort_td)(57000 / resultRowSize(m_ignoreFields));
    }

    ushort_td resultBufferNeedSize()
    {
        return (m_ret.maxRows * resultRowSize(m_ignoreFields)) + DATASIZE_BYTE;
    }

    void joinLogic()
    {
        for (int i= (int)m_logics.size()-2;i>=0;--i)
        {
            logic* la = m_logics[i+1];
            logic* lb = m_logics[i];
            if (la->canJoin(false) && lb->canJoin(true) && lb->isNextFiled(la))
            {
                lb->joinAfter(la);
                delete la;
                m_logics.erase(m_logics.end() -1);
            }
        }
    }

    int doWriteBuffer(bool estimate)
    {
        unsigned char* p = (unsigned char*)m_tb->dataBak();
        joinLogic();
 	    m_hd.logicalCount = (ushort_td)m_logics.size();
        if (m_ignoreFields)
            m_ret.fieldCount = 0;
        else
            m_ret.fieldCount = (ushort_td)m_fields.size();
        if (m_ret.maxRows == 0)
            m_ret.maxRows = calcMaxRows();

        unsigned char* start = p;
        p =  m_hd.writeBuffer(p, estimate);

        for (size_t i=0;i< m_logics.size();++i)
            p = m_logics[i]->writeBuffer(p, estimate);

        p =  m_ret.writeBuffer(p, estimate);

        if (!m_ignoreFields)
        {
            for (size_t i=0;i< m_fields.size();++i)
                p = m_fields[i]->writeBuffer(p, estimate);
        }

        //write total length
        int len = (int)(p - start);
        unsigned short* s = (unsigned short*)start;
        *s = len;
        return len;
    }

    void allocDataBuffer()
    {
        m_hd.len = doWriteBuffer(true);
        int resultLen = resultBufferNeedSize();
        m_extendBuflen = std::max<int>((int)m_hd.len, resultLen);

        if ((m_fields.size() != 1) || m_tb->valiableFormatType())
            m_extendBuflen += m_tb->buflen();

        if ((int)m_tb->buflen() < m_extendBuflen)
        {
            m_tb->setDataBak((void*) realloc(m_tb->dataBak(), m_extendBuflen));
            m_tb->setData(m_tb->dataBak());
        }
    }

public:
    filter(table* tb):m_tb(tb),m_ignoreFields(false){}
    ~filter()
    {
        cleanup();
    }

    void cleanup()
    {
        for (size_t i=0;i < m_logics.size();++i)
            delete m_logics[i];
        for (size_t i=0;i < m_fields.size();++i)
            delete m_fields[i];
        m_logics.erase(m_logics.begin(), m_logics.end());
        m_fields.erase(m_fields.begin(), m_fields.end());
        m_hd.reset();
        m_ret.reset();
        m_ignoreFields = false;
    }

    bool setQuery(const queryBase* q)
    {
        m_str = q->toString();
        bool ret = doSetFilter(q);
        if (!ret)
            cleanup();
        return ret;
    }

    void setPositionType(bool incCurrent){m_hd.setPositionType(incCurrent);}

    bool positionTypeNext() const{return m_hd.positionTypeNext();}

    void setRejectCount(ushort_td v){m_hd.rejectCount = v;}
    ushort_td rejectCount()const {return m_hd.rejectCount;}

    void setMaxRows(ushort_td v){m_ret.maxRows = v;}
    ushort_td maxRows()const {return m_ret.maxRows;}

    ushort_td recordCount()const {return maxRows();}

    void setPosTypeNext(bool v){setPositionType(!v);}

    uint_td exDataBufLen() const{return extendBuflen();}

    void init(table* pBao){};

    const _TCHAR* filterStr(){return m_str.c_str();}

    ushort_td fieldCount() const {return m_ret.fieldCount;}

    void setFieldCount(ushort_td v){m_ret.fieldCount = v;}

    ushort_td fieldLen(int index) const
    {
        assert(index < (int)m_fields.size());
        return m_fields[index]->len;
    }

    ushort_td fieldOffset(int index) const
    {
        assert(index < (int)m_fields.size());
        return m_fields[index]->pos;
    }

    void writeBuffer()
    {
        allocDataBuffer();
        doWriteBuffer(false);
    }

    ushort_td extendBuflen() const{return m_extendBuflen;}

    bool fieldSelected() const
    {
        return !((m_fields.size() == 1) && (m_fields[0]->pos == 0));
    }

    bool ignoreFields() const {return m_ignoreFields;}

    void setIgnoreFields(bool v){m_ignoreFields = v;};
};





}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
