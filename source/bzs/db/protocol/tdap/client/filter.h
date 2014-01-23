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

/** copy data for select comp
 */
inline ushort_td copyForCompare(const fielddef& fd, uchar_td* to, const uchar_td* from, bool part)
{
    ushort_td varlen = varlenForFilter(fd);
    int copylen = compDataLen(fd, from, part);
    if (varlen)
        memcpy(to, from, varlen);
    memcpy(to + varlen, fd.keyData(from), copylen);
    return copylen + varlen;
}


#define NEW_FILTER

#ifdef NEW_FILTER

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
            int n = _tcslen(p);

            if ((ret = (p[n-1] == _T('*')))!=0)
            {
                p[n-1] = 0x00;
                tb->setFV(index, p);
            }
        }
        return ret;
    }

    void allocBuffer(int size)
    {
         if (data)
            delete [] data;
         data = new unsigned char[len + 2];
    }

    void copyToBuffer(table* tb, short index, bool part)
    {
        fielddef* fd = &tb->tableDef()->fieldDefs[index];
        len = copyForCompare(*fd, (uchar_td*)data,
                    (const uchar_td*)tb->fieldPtr(index), part);
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
            allocBuffer(len);
            //backup
            memcpy(tmp, tb->fieldPtr(fieldNum), len);

            if (compField)
                ret = setCompFiled(tb, fieldNum, value);// value is field name
            else
            {
                tb->setFV(fieldNum, value);
                bool part = isPart(tb, fieldNum);
                int varlen = varlenForFilter(*fd);
                if (varlen > len)
                     allocBuffer(varlen);

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
                && !(logType & CMPLOGICAL_FIELD)
                && !isStringType(type));
    }

    bool isNextFiled(logic* src)
    {
        return ((pos + len) ==  src->pos);
    }

    void joinAfter(logic* src)
    {
        assert(src);
        assert(src->data);
        void* tmp = data;
        data = new unsigned char[len + src->len + 2];
        memcpy(data, tmp, len);
        memcpy(data + len, src->data, src->len);
        len += src->len;
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
    //friend class filter;
    table* m_tb;
    header m_hd;
    resultDef m_ret;
    std::vector<resultField*> m_fields;
    std::vector<logic*> m_logics;
    int m_extendBuflen;
    std::_tstring m_str;

    void cleanup()
    {
        for (int i=0;i<(int)m_logics.size();++i)
            delete m_logics[i];
        for (int i=0;i<(int)m_fields.size();++i)
            delete m_fields[i];
        m_logics.erase(m_logics.begin(), m_logics.end());
        m_fields.erase(m_fields.begin(), m_fields.end());
        m_hd.reset();
        m_ret.reset();
    }

    bool setSelect(std::vector<std::_tstring>& selects)
    {
        if (selects.size() == 0)
            return false;
        if (selects[0] == _T("*"))
            addAllFields();
        else
        {
            for (int i=0;i<(int)selects.size();++i)
                addSelect(selects[i].c_str());
        }
        return true;
    }

    bool doSetFilter(const _TCHAR* str, ushort_td rejectCount, ushort_td maxRows)
    {
        cleanup();
        setRejectCount(rejectCount);
        setMaxRows(maxRows);
        std::vector<std::_tstring> tmp;
        std::vector<std::_tstring> selects;

        boost::algorithm::split(tmp, str, boost::is_space());
        std::_tstring s = tmp[0];
        boost::algorithm::to_lower(s);

        if (s == _T("select"))
        {
            if (tmp.size() < 2) return false;// no field specify
            tmp.erase(tmp.begin());
            s = tmp[0];
            if (s == _T("*"))
                addAllFields();
            else
            {
                boost::algorithm::split(selects, tmp[0], boost::is_any_of(_T(",")));
                if (!setSelect(selects))
                    return false;
            }
            tmp.erase(tmp.begin());

        }else
        {
            if (s == _T("*"))
                tmp.erase(tmp.begin());
            addAllFields();
        }

        if (tmp.size() > 2)
        {
            for (int i=1;i<(int)tmp.size();i+=3)
            {
                char combine = eCend;
                bool compField = (tmp[i+2][0] == _T('['));
                if (compField)
                {
                    tmp.erase(tmp.begin());
                    tmp.erase(tmp.end() - 1);
                }
                if (i+3 <(int) tmp.size())
                {
                    s = tmp[i+2];
                    boost::algorithm::to_lower(s);
                    if (s == _T("or")) combine = eCor;
                    else if (s == _T("and"))
                        combine = eCand;
                    else
                        return false;
                }
                if (!addWhere(tmp[i].c_str(), tmp[i+1].c_str()
                        , tmp[i+2].c_str(), combine, compField))
                    return false;

            }
        }
        return true;
    }

    ushort_td resultRowSize()
    {
        ushort_td recordLen = 0;
        for (int i=0;i<(int)m_fields.size();++i)
            recordLen += m_fields[i]->len;
        return recordLen;
    }

    ushort_td calcMaxRows()
    {
        int recordLen = 6; //length(2) + bookmark(4)
        recordLen += resultRowSize();
        return (ushort_td)(57000 / recordLen);
    }

    ushort_td resultBufferNeedSize()
    {
        return m_ret.maxRows * resultRowSize() + 2;
    }

    void joinLogic()
    {
        for (int i=m_logics.size()-2;i>=0;--i)
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

    int doWwriteBuffer(bool estimate)
    {
        unsigned char* p = (unsigned char*)m_tb->dataBak();
        joinLogic();
 	    m_hd.logicalCount = m_logics.size();
        m_ret.fieldCount = m_fields.size();
        if (m_ret.maxRows == 0)
            m_ret.maxRows = calcMaxRows();

        unsigned char* start = p;
        p =  m_hd.writeBuffer(p, estimate);

        for (int i=0;i<(int)m_logics.size();++i)
            p = m_logics[i]->writeBuffer(p, estimate);

        for (int i=0;i<(int)m_fields.size();++i)
            p = m_fields[i]->writeBuffer(p, estimate);

        p =  m_ret.writeBuffer(p, estimate);

        //write total length
        int len = p - start;
        unsigned short* s = (unsigned short*)start;
        *s = len;
        return len;
    }

    void allocDataBuffer()
    {
        int inputlen = doWwriteBuffer(true);
        int resultLen = resultBufferNeedSize();
        m_extendBuflen = std::max(inputlen, resultLen);

        if ((m_fields.size() != 1) || m_tb->valiableFormatType())
            m_extendBuflen += m_tb->buflen();

        if ((int)m_tb->buflen() < m_extendBuflen)
        {
            m_tb->setDataBak((void*) realloc(m_tb->dataBak(), m_extendBuflen));
            m_tb->setData(m_tb->dataBak());
        }
    }

public:
    filter(table* tb):m_tb(tb){}
    ~filter()
    {
        cleanup();
    }

    bool setFilter(const _TCHAR* str, ushort_td rejectCount, ushort_td maxRows)
    {
        bool ret = doSetFilter(str, rejectCount, maxRows);
        if (!ret)
            cleanup();
        return ret;
    }

    inline void setPositionType(bool incCurrent)
    {
        m_hd.setPositionType(incCurrent);
    }

    inline bool positionTypeNext() const
    {
        return m_hd.positionTypeNext();
    }

    void setRejectCount(ushort_td v){m_hd.rejectCount = v;}
    ushort_td rejectCount()const {return m_hd.rejectCount;}

    void setMaxRows(ushort_td v){m_ret.maxRows = v;}
    ushort_td maxRows()const {return m_ret.maxRows;}

    //-----------------------------
    ushort_td recordCount()const {return maxRows();}
    ushort_td* recordCountDirect()
    {
        return &m_ret.maxRows;
    }
    void setRecordCountDirect(ushort_td* v)
    {
        m_ret.maxRows = *v;
    }
    void setPosTypeNext(bool v)
    {
        setPositionType(!v);
    }
    uint_td exDataBufLen() const
    {
        return extendBuflen();
    }
    void init(table* pBao){};

    _TCHAR* filterStr()
    {
        m_str = _T("select ");
        if (!fieldSelected())
            m_str += _T("* ");
        else
        {
            for (int i=0;i<m_fields.size();++i)
                m_str += m_fields.pos;
        }
        return _T("");
    }
    //-----------------------------

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

    ushort_td fieldCount() const
    {
        return m_ret.fieldCount;
    }

    void setFieldCount(ushort_td v)
    {
        m_ret.fieldCount = v;
    }

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
        doWwriteBuffer(false);
    }

    ushort_td extendBuflen() const
    {
        return m_extendBuflen;
    }

    bool fieldSelected() const
    {
        return !((m_fields.size() == 1) && (m_fields[0]->pos == 0));
    }
};

/*
class filter
{
    class filter_t* m_impl;

public:
    filter(table* tb);
    ~filter();

    bool posTypeNext();
    void setPosTypeNext(bool v);
    uint_td exDataBufLen() const;
    ushort_td recordCount() const;
    void setRecordCount(ushort_td v);
    ushort_td fieldCount() const;
    void setFieldCount(ushort_td v);
    ushort_td* recordCountDirect() const;
    void setRecordCountDirect(ushort_td* v);
    ushort_td rejectCount() const;
    ushort_td fieldLen(int index);
    ushort_td fieldOffset(int index);
    bool fieldSelected() const;
    void init(table*){};
    void WriteBuffer();
    bool setFilter(const _TCHAR* str, ushort_td RejectCount, ushort_td CashCount);
    _TCHAR* filterStr();
};*/


#else
class filter
{

    table* m_tb;

    //buffer
    _TCHAR* m_pFilter;
    void* m_pEntendBuf;
    ushort_td m_pEntendBuflen;
    uint_td m_ExDataBufLen;

    //hedaer
    ushort_td m_RejectCount;
    char m_PosType[3]; // "EG" or "UC"

    //tail
    ushort_td m_iRecordCount;
    ushort_td m_iFieldCount;

    ushort_td* m_iRecordCountDirect;
    //result
    ushort_td m_iFieldLen[255];
    ushort_td m_iFieldOffset[255];

    bool GetCompStr(_TCHAR** str);
    uchar_td GetLogical(_TCHAR** str);
    uchar_td GetCompType(_TCHAR** str);
    short GetField(_TCHAR** str);
    ushort_td GetFieldLen(int index);
    ushort_td GetFieldOffset(int index);
    bool MakeFieldSelect(_TCHAR** str);

    bool m_FieldSelected;

public:
    filter(table* tb);
    ~filter();

    bool posTypeNext() {return GetPosType();}

    void setPosTypeNext(bool v) {SetPosType(v);}

    uint_td exDataBufLen() const {return m_ExDataBufLen;}

    ushort_td recordCount() const {return m_iRecordCount;}

    void setRecordCount(ushort_td v) {m_iRecordCount = v;}

    ushort_td fieldCount() const {return m_iFieldCount;}

    void setFieldCount(ushort_td v) {m_iFieldCount = v;}

    //void* entendBuf() const {return m_pEntendBuf;}

    //void setEntendBuf(void* v) {m_pEntendBuf = v;};

    ushort_td* recordCountDirect() const {return m_iRecordCountDirect;}

    void setRecordCountDirect(ushort_td* v) {m_iRecordCountDirect = v;}

    ushort_td rejectCount() const {return m_RejectCount;}

    ushort_td fieldLen(int index) {return GetFieldLen(index);}

    ushort_td fieldOffset(int index) {return GetFieldOffset(index);}

    bool fieldSelected() const {return m_FieldSelected;}

    void init(table* pBao);
    //bool GetPosType();
    //void SetPosType(bool);
    void WriteBuffer();
    bool setFilter(const _TCHAR* str, ushort_td RejectCount, ushort_td CashCount);
    _TCHAR* filterStr();

};
#endif//NEW_FILTER

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
