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

#include "filter.h"
#pragma option -w-ucp

#pragma package(smart_init)

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

filter::filter(table* pBao)
{
    m_pFilter = NULL;
    m_pEntendBuf = NULL;
    m_pEntendBuflen = 0;

    init(pBao);

}

void filter::init(table* pBao)
{
    m_tb = pBao;
    strcpy(&m_PosType[0], "UC");
    m_RejectCount = 65000;
    m_iRecordCount = 1;
    m_iFieldCount = 1;
    memset(m_iFieldLen, 0, 255 * sizeof(ushort_td));
    memset(m_iFieldOffset, 0, 255 * sizeof(ushort_td));
    m_iFieldLen[0] = (ushort_td) m_tb->tableDef()->maxRecordLen;
    m_iFieldOffset[0] = 0;
}

filter::~filter()
{
    if (m_pFilter != NULL)
        delete[]m_pFilter; ;
    if (m_pEntendBuf != NULL)
        free(m_pEntendBuf);

}

ushort_td filter::GetFieldLen(int index) {return m_iFieldLen[index];

}

ushort_td filter::GetFieldOffset(int index) {return m_iFieldOffset[index];}

_TCHAR* filter::filterStr() {return m_pFilter;}

bool filter::GetPosType()
{

    if (strcmp(m_PosType, "UC") == 0)
        return false;
    else
        return true;
}

void filter::SetPosType(bool getnext)
{

    if (getnext)
        strcpy(&m_PosType[0], "EG");
    else
        strcpy(&m_PosType[0], "UC");
    if (m_pEntendBuf)
        memcpy((void*)((char*)m_pEntendBuf + 2), &m_PosType[0], 2);

}

void filter::WriteBuffer() {memcpy(m_tb->dataBak(), m_pEntendBuf, m_pEntendBuflen);}

bool filter::setFilter(const _TCHAR* str, ushort_td RejectCount, ushort_td CashCount)
{

    _TCHAR* NewPointer;

    BFilter* itm;
    BFilter* Olditm = NULL;

    bool joint = false;
    bool cmpfield;
    ushort_td Oldlen;
    short OldfieldNum = -1;
    BFilterHeader* FilterHead;
    ushort_td LogicalCount = 0;
    BFilterDisc* disc;

    m_RejectCount = RejectCount;
    m_iRecordCount = CashCount;

    if (m_pFilter)
        delete[]m_pFilter;
    size_t size = _tcslen(str) + 2;
    m_pFilter = new _TCHAR[size];
    if (m_pFilter == NULL)
        return false;
    memset(m_pFilter, 0, size * sizeof(_TCHAR));
    _tcscpy(m_pFilter, str);

    int sDataBufSize = 8192;
    if (m_pEntendBuf == NULL)
        m_pEntendBuf = malloc(sDataBufSize);
    else
        memset(m_pEntendBuf, 0, sDataBufSize);

    if (m_pEntendBuf == NULL)
        return false;

    itm = (BFilter*)((char*)m_pEntendBuf + 8);

    NewPointer = m_pFilter;

    m_FieldSelected = false;
    if (m_pFilter[0] == '*')
        LogicalCount = 0;
    else
    {

        // -------------------------------------------------------------
        // SELECT
        if (_tcsstr(m_pFilter, _T("SELECT ")) == NewPointer)
        {
            if (!MakeFieldSelect(&NewPointer))
                return false;
        }

        while (1)
        {
            cmpfield = false;

            // -------------------------------------------------------------
            // Get filed number
            // -------------------------------------------------------------
            short fieldNum = GetField(&NewPointer);
            if (fieldNum == -1)
                break;

            LogicalCount++;
            fielddef* fd = &m_tb->tableDef()->fieldDefs[fieldNum];
            itm->Type = fd->type;
            itm->Len = fd->len;
            itm->Pos = fd->pos;

            // -------------------------------------------------------------
            // Get logical type
            // -------------------------------------------------------------
            itm->CompType = GetCompType(&NewPointer);
            if (itm->CompType == 255)
                return false;

            // -------------------------------------------------------------
            // Get compare value
            // if [fieldName] then comapre as field value
            // -------------------------------------------------------------
            _TCHAR* start = NewPointer;
            if (!GetCompStr(&NewPointer))
                return false;
            if (NewPointer == NULL)
                return false;

            int offset = 0;
            if (start[0] == '[')
            {
                start++;
                int num = m_tb->fieldNumByName(start);
                if (num != -1)
                {
                    itm->Data = m_tb->tableDef()->fieldDefs[num].pos;
                    itm->CompType |= CMPLOGICAL_FIELD;
                    cmpfield = true;
                }
            }
            else
            {

                bool isPart = false;
                if (start[0] == '\'')
                {// string
                    start++; // erase front '
                    int len = (ushort_td) _tcslen(start);
                    start[--len] = 0x00; // erase after '
                    isPart = (start[len - 1] == '*');
                    if (isPart)
                    start[--len] = 0x00;

                }
                m_tb->setFV(fieldNum, start);
                itm->Len =
                    copyForCompare(*fd, (uchar_td*)(&itm->Data),
                    (const uchar_td*)m_tb->fieldPtr(fieldNum), isPart);
                if (!isPart && (fd->varLenBytes() || fd->blobLenBytes()))
                    itm->CompType |= CMPLOGICAL_VAR_COMP_ALL; //match complate
                offset = itm->Len - 2;
            }

            // LogicalType
            itm->LogicalType = GetLogical(&NewPointer);
            // ------------------------------------------------------------
            // chain befor field
            // ------------------------------------------------------------
            Oldlen = fd->len;

            if ((joint == true) && (OldfieldNum + 1 == fieldNum) && (itm->CompType == 1) &&
                (cmpfield == false) && (itm->Len == fd->len) && (itm->Type != 11) &&
                (varlenForFilter(*fd) == 0))
            {
                Olditm->LogicalType = itm->LogicalType;
                Oldlen = itm->Len;
                memcpy((void*)((char*)(&Olditm->Data) + Olditm->Len), &itm->Data, itm->Len);
                Olditm->Len += Oldlen;
                Oldlen = Olditm->Len;
                itm = Olditm;
                itm->Type = 0; // comapre as string

                offset = itm->Len - 2;
                LogicalCount--;
            }
            if (itm->LogicalType != 0) // If zero then logical is end.
            {
                // ------------------------------------------------------------
                // Can it connect?   not * and not zstring
                // ------------------------------------------------------------
                if ((itm->CompType == 1) && (itm->LogicalType == 1) && (cmpfield == false) &&
                    (itm->Len == Oldlen) && (itm->Type != 11) && (varlenForFilter(*fd) == 0))
                    joint = true;
                else
                    joint = false;
                Olditm = itm;
                OldfieldNum = fieldNum;
            }
            // ------------------------------------------------------------
            // move pointer
            itm = (BFilter*)((char*)itm +sizeof(BFilter) + offset);
        }
    }
    _tcscpy(m_pFilter, str);
    // header
    FilterHead = (BFilterHeader*) m_pEntendBuf;
    FilterHead->BufLen = (ushort_td)((char*)itm - (char*)m_pEntendBuf + 4 + (m_iFieldCount * 4));
    memcpy(FilterHead->PosType, m_PosType, 2);
    FilterHead->RejectCount = m_RejectCount;
    FilterHead->LogicalCount = LogicalCount;

    // descipter
    disc = (BFilterDisc*)itm;
    disc->iRecordCount = m_iRecordCount;
    disc->iFieldCount = m_iFieldCount;
    int RecordSize = 0;
    for (int i = 0; i < m_iFieldCount; i++)
    {
        disc->iSelectFields[i].iFieldLen = m_iFieldLen[i];
        disc->iSelectFields[i].iFieldOffset = m_iFieldOffset[i];
        RecordSize += m_iFieldLen[i];
    }
    m_iRecordCountDirect = &disc->iRecordCount;
    m_pEntendBuflen = FilterHead->BufLen;
    if (disc->iRecordCount == 0)
    {
        disc->iRecordCount = (ushort_td)(57000 / (RecordSize + 6));
    }
    m_ExDataBufLen = (ushort_td)((disc->iRecordCount * (RecordSize + 6)) + 2);

    if ((ushort_td)m_ExDataBufLen < m_pEntendBuflen)
        m_ExDataBufLen = m_pEntendBuflen;

    uint_td len;
    if ((m_FieldSelected == false) && !m_tb->valiableFormatType())
        len = m_ExDataBufLen;
    else
        len = (uint_td)(m_ExDataBufLen + m_tb->buflen());

    if (m_tb->buflen() < len)
    {
        m_tb->setDataBak((void*) realloc(m_tb->dataBak(), len));
        m_tb->setData(m_tb->dataBak());

        if (m_tb->data() == NULL)
            return false;

    }
    return true;
}

bool filter::MakeFieldSelect(_TCHAR** str)
{
    _TCHAR* p;
    _TCHAR* ret;
    _TCHAR* point[255];
    ushort_td index = 0;
    _TCHAR FiledName[20] = {0x00};

    m_iFieldCount = 0;


    *str += 7; // "select" length = 7
    point[index] = *str;

    ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*)_T(" "));
    if (ret == NULL)
        return false;
    ret[0] = 0x00; // set null
    *str = ret + 1;

    index++;
    p = (_TCHAR*)_tcsmstr((_TUCHAR*)point[index - 1], (_TUCHAR*)_T(",")) + 1;
    point[index] = p;
    p--;
    while (p)
    {
        index++;
        p = (_TCHAR*)_tcsmstr((_TUCHAR*)point[index - 1], (_TUCHAR*)_T(","));
        if (p)
            point[index] = p + 1;
        else
            break;

    }
    point[index] = ret + 1;
    size_t n;
    short FieldNum;
    for (int i = 0; i < index; i++)
    {
        n = point[i + 1] - point[i] - 1;
        _tcsncpy(FiledName, point[i], n);
        FiledName[n] = 0x00;
        FieldNum = m_tb->fieldNumByName(FiledName);
        if (FieldNum < 0)
            return false;
        m_iFieldLen[i] = m_tb->tableDef()->fieldDefs[FieldNum].len;
        m_iFieldOffset[i] = m_tb->tableDef()->fieldDefs[FieldNum].pos;
    }
    m_iFieldCount = index;
    m_FieldSelected = true;

    return true;
}

short filter::GetField(_TCHAR** str)
{
    _TCHAR* ret;
    _TCHAR FiledName[20] = {0x00};
    int offset = 0;
    // Get string that until next space.
    if (*str[0] == '[')
    {
        offset = 1;
        ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*) _T("]"));
    }
    else
        ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*)_T(" "));

    if (ret)
    {
        _tcsncpy(FiledName, *str + offset, ret - *str - offset);
        *str = ret + 1 + offset;
        return m_tb->fieldNumByName(FiledName);
    }
    else
        return -1;
}

bool filter::GetCompStr(_TCHAR** str)
{
    _TCHAR* ret;
    int offset = 0;

    // Get string that until next space.
    if (*str[0] == _T('['))
    {
        offset = 1;
        ret = _tcsstr(*str, _T("]"));
        if (!ret)
            return false;
    }
    else if (*str[0] == _T('\''))
    { // find next '
        ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str + 1, (_TUCHAR*)_T("\'"));

        if (ret)
            ret += 1;
        else
            return false;
    }
    else
        ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*)_T(" "));

    if (ret)
    {
        *ret = 0x00;
        *str = ret + 1 + offset;

    }
    return true;
}

uchar_td filter::GetLogical(_TCHAR** str)
{
    _TCHAR* ret;
    _TCHAR LogicalStr[10] = {0x00};

    // Get string that until next space.
    ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*)_T(" "));
    if (ret)
    {
        _tcsncpy(LogicalStr, *str, ret - *str);
        _tcsmupr((_TUCHAR*)LogicalStr);
        *str = ret + 1;
        if (_tcscmp(LogicalStr, _T("AND")) == 0)
            return (uchar_td)1;
        else if (_tcscmp(LogicalStr, _T("OR")) == 0)
            return (uchar_td)2;
        else
            return 0;
    }
    else
        return 0;
}

uchar_td filter::GetCompType(_TCHAR** str)
{
    _TCHAR* ret;
    _TCHAR cmpstr[10] = {0x00};

    // Get string that until next space.
    ret = (_TCHAR*)_tcsmstr((_TUCHAR*)*str, (_TUCHAR*)_T(" "));
    if (ret)
    {
        _tcsncpy(cmpstr, *str, ret - *str);
        *str = ret + 1;
        if (_tcscmp(cmpstr, _T("=")) == 0)
            return (uchar_td)1;

        if (_tcscmp(cmpstr, _T(">")) == 0)
            return (uchar_td)2;

        if (_tcscmp(cmpstr, _T("<")) == 0)
            return (uchar_td)3;

        if (_tcscmp(cmpstr, _T("<>")) == 0)
            return (uchar_td)4;

        if (_tcscmp(cmpstr, _T("=>")) == 0)
            return (uchar_td)5;
        if (_tcscmp(cmpstr, _T(">=")) == 0)
            return (uchar_td)5;

        if (_tcscmp(cmpstr, _T("=<")) == 0)
            return (uchar_td)6;
        if (_tcscmp(cmpstr, _T("<=")) == 0)
            return (uchar_td)6;
        return 255;
    }
    else
        return 255;
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
