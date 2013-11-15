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


#pragma option -a-
pragma_pack1

struct BFilter
{
    uchar_td Type;
    ushort_td Len;
    ushort_td Pos;
    uchar_td CompType;
    uchar_td LogicalType;
    ushort_td Data;
};

struct BFilterHeader
{
    ushort_td BufLen;
    char PosType[2]; // "EG" or "UC"
    ushort_td RejectCount;
    ushort_td LogicalCount;
};

struct BSelectField
{
    ushort_td iFieldLen;
    ushort_td iFieldOffset;
};

struct BFilterDisc
{
    ushort_td iRecordCount;
    ushort_td iFieldCount;
    BSelectField iSelectFields[255];
};

#pragma option -a
pragma_pop

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
    filter(table* pBao);
    ~filter();

    bool posTypeNext() {return GetPosType();}

    void setPosTypeNext(bool v) {SetPosType(v);}

    uint_td exDataBufLen() const {return m_ExDataBufLen;}

    ushort_td recordCount() const {return m_iRecordCount;}

    void setRecordCount(ushort_td v) {m_iRecordCount = v;}

    ushort_td fieldCount() const {return m_iFieldCount;}

    void setFieldCount(ushort_td v) {m_iFieldCount = v;}

    void* entendBuf() const {return m_pEntendBuf;}

    void setEntendBuf(void* v) {m_pEntendBuf = v;};

    ushort_td* recordCountDirect() const {return m_iRecordCountDirect;}

    void setRecordCountDirect(ushort_td* v) {m_iRecordCountDirect = v;}

    ushort_td rejectCount() const {return m_RejectCount;}

    ushort_td fieldLen(int index) {return GetFieldLen(index);}

    ushort_td fieldOffset(int index) {return GetFieldOffset(index);}

    bool fieldSelected() const {return m_FieldSelected;}

    void init(table* pBao);
    bool GetPosType();
    void SetPosType(bool);
    void WriteBuffer();
    bool setFilter(const _TCHAR* str, ushort_td RejectCount, ushort_td CashCount);
    _TCHAR* filterStr();

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
