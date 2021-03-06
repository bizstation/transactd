/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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
=================================================================*/
#include "stdafx.h"
#include "Table.h"
#include "Database.h"
#include "DbDef.h"
#include "TableDef.h"
#include "QueryBase.h"
#include "PreparedQuery.h"
#include <bzs/db/protocol/tdap/client/fields.h>
#include "Bookmark.h"
#include "Bitset.h"

using namespace bzs::db::protocol::tdap;

STDMETHODIMP CTableTd::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_ITable
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

short CTableTd::GetFieldNum(VARIANT* Index)
{
    short index = -1;
    if (Index->vt == VT_BSTR)
        index = m_tb->fieldNumByName(Index->bstrVal);
    else if (Index->vt == VT_I2)
        index = Index->iVal;
    else if (Index->vt == VT_I4)
        index = (short)Index->lVal;
    return index;
}

STDMETHODIMP CTableTd::get_Text(VARIANT Index, BSTR* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

    *Value = ::SysAllocString(m_tb->getFVstr(index));
    return S_OK;
}

STDMETHODIMP CTableTd::get_Vlng(VARIANT Index, int* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFVint(index);
    return S_OK;
}

STDMETHODIMP CTableTd::put_Text(VARIANT Index, BSTR Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    m_tb->setFVW(index, Value);
    return S_OK;
}

STDMETHODIMP CTableTd::put_Vlng(VARIANT Index, int Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    m_tb->setFV(index, Value);
    return S_OK;
}

STDMETHODIMP CTableTd::SetFVNull(VARIANT Index, VARIANT_BOOL Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    m_tb->setFVNull(index, Value != 0);
    return S_OK;
}

STDMETHODIMP CTableTd::GetFVNull(VARIANT Index, VARIANT_BOOL* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFVNull(index);
    return S_OK;
}


STDMETHODIMP CTableTd::GetFVint(VARIANT Index, int* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFVint(index);
    return S_OK;
}

STDMETHODIMP CTableTd::GetFV64(VARIANT Index, __int64* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFV64(index);
    return S_OK;
}

STDMETHODIMP CTableTd::GetFVdbl(VARIANT Index, double* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFVdbl(index);
    return S_OK;
}

STDMETHODIMP CTableTd::GetFVstr(VARIANT Index, BSTR* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

    *Value = ::SysAllocString(m_tb->getFVstr(index));
    return S_OK;
}

STDMETHODIMP CTableTd::SetFV(VARIANT Index, VARIANT Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

    if (Value.vt == VT_BSTR)
        m_tb->setFV(index, Value.bstrVal);
    else if (Value.vt == VT_R4)
        m_tb->setFV(index, Value.fltVal);
    else if (Value.vt == VT_R8)
        m_tb->setFV(index, Value.dblVal);
    else if (Value.vt == VT_I2)
        m_tb->setFV(index, Value.iVal);
    else if (Value.vt == VT_I4 || Value.vt == VT_INT)
        m_tb->setFV(index, Value.lVal);
    else if (Value.vt == VT_I8)
        m_tb->setFV(index, Value.llVal);
    else if (Value.vt == VT_NULL)
        m_tb->setFV(index, (wchar_t*)NULL);
    else if ((Value.vt == VT_DISPATCH) && Value.pdispVal)
    {
        CBitset* b = dynamic_cast<CBitset*>(Value.pdispVal);
        if (b)
            m_tb->setFV(index, b->m_bitset.internalValue());    
    }
    else
    {
        VariantChangeType( &Value, &Value, 0, VT_BSTR );
        if (Value.bstrVal[0])
            m_tb->setFV(index, Value.bstrVal);
        else
            m_tb->setFV(index, L"");
    }
    return S_OK;
}


STDMETHODIMP CTableTd::get_V64(VARIANT Index, __int64* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFV64(index);
    return S_OK;
}

STDMETHODIMP CTableTd::put_V64(VARIANT Index, __int64 Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    m_tb->setFV(index, Value);
    return S_OK;
}

STDMETHODIMP CTableTd::get_Vdbl(VARIANT Index, double* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    *Value = m_tb->getFVdbl(index);
    return S_OK;
}

STDMETHODIMP CTableTd::put_Vdbl(VARIANT Index, double Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    m_tb->setFV(index, Value);
    return S_OK;
}

STDMETHODIMP CTableTd::get_Vbin(VARIANT Index, BSTR* Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    uint_td size;
    void* p = m_tb->getFVbin(index, size);

    *Value = ::SysAllocStringByteLen((char*)p, size);

    return S_OK;
}

STDMETHODIMP CTableTd::put_Vbin(VARIANT Index, BSTR Value)
{

    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    int len = ::SysStringByteLen(Value);
    if (len > m_tb->tableDef()->fieldDefs[index].len)
        len = m_tb->tableDef()->fieldDefs[index].len;
    m_tb->setFV(index, Value, len);
    return S_OK;
}

STDMETHODIMP CTableTd::get_TableDef(ITableDef** Value)
{
    CComObject<CTableDef>* piObj;
    CComObject<CTableDef>::CreateInstance(&piObj);
    if (piObj)
    {
        m_tabledef = const_cast<tabledef*>(m_tb->tableDef());
        piObj->m_tabledefPtr = &m_tabledef;
        ITableDef* tbd;
        piObj->QueryInterface(IID_ITableDef, (void**)&tbd);
        _ASSERTE(tbd);
        *Value = tbd;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CTableTd::Insert(eUpdateType ncc)
{
    m_tb->insert((client::nstable::eUpdateType)ncc);
    return S_OK;
}

STDMETHODIMP CTableTd::Delete(VARIANT_BOOL inkey)
{
    m_tb->del(inkey);
    return S_OK;
}

STDMETHODIMP CTableTd::ClearBuffer(eNullReset resetType)
{
  
    m_tb->clearBuffer((resetType == defaultNull) ? 
            client::table::defaultNull : client::table::clearNull);
    return S_OK;
}

STDMETHODIMP CTableTd::Close()
{
    m_tb->close();
    return S_OK;
}

STDMETHODIMP CTableTd::SeekFirst(eLockType lockBias)
{
    m_tb->seekFirst((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::SeekLast(eLockType lockBias)
{
    m_tb->seekLast((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::SeekPrev(eLockType lockBias)
{
    m_tb->seekPrev((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::SeekNext(eLockType lockBias)
{
    m_tb->seekNext((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::Seek(eLockType lockBias)
{
    m_tb->seek((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::SeekGreater(VARIANT_BOOL orEqual, eLockType lockBias)
{
    m_tb->seekGreater(orEqual, (ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::SeekLessThan(VARIANT_BOOL orEqual, eLockType lockBias)
{
    m_tb->seekLessThan(orEqual, (ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::get_Bookmark(IBookmark** retVal)
{
    CComObject<CBookmark>* bm;
    CComObject<CBookmark>::CreateInstance(&bm);
    if (!bm)
        return Error("CreateInstance Bookmark", IID_ITable);
    bm->set(m_tb->bookmark(), m_tb->bookmarkLen());
    IBookmark* b;
    bm->QueryInterface(IID_IBookmark, (void**)&b);
    _ASSERTE(b);
    *retVal = b;
    return S_OK;
}

STDMETHODIMP CTableTd::SeekByBookmark(IBookmark* bm, eLockType lockBias)
{
    CBookmark* cb = dynamic_cast<CBookmark*>(bm);
    if (cb)
    {   
        m_tb->seekByBookmark(cb->internalBookmark(), (ushort_td)lockBias);
        return S_OK;
    }
    return Error("Invalid param bookmark", IID_ITable);
}

STDMETHODIMP CTableTd::get_Percentage(VARIANT param, long* Value)
{
    if ((param.vt == VT_DISPATCH) && param.pdispVal)
    {
        CBookmark* bm = dynamic_cast<CBookmark*>(param.pdispVal);
        if (bm)
            *Value = m_tb->getPercentage(bm->internalBookmark());
        else
            return Error("Invalid param 1 not IBookmark", IID_ITable);
    }else
        *Value = m_tb->getPercentage();
    return S_OK;
}

STDMETHODIMP CTableTd::get_RecordLength(long* Value)
{
    *Value = m_tb->recordLength();
    return S_OK;
}

STDMETHODIMP CTableTd::RecordCount(VARIANT_BOOL estimate,
                                   VARIANT_BOOL fromCurrent, long* Value)
{
    m_tb->setOnRecordCount(onRecordCount);
    m_tb->setOptionalData(this);

    *Value = m_tb->recordCount((estimate == -1), (fromCurrent == -1));
    return S_OK;
}

STDMETHODIMP CTableTd::SeekByPercentage(long Value)
{
    m_tb->seekByPercentage(Value);
    return S_OK;
}

STDMETHODIMP CTableTd::get_KeyNum(short* Value)
{
    *Value = m_tb->keyNum();
    return S_OK;
}

STDMETHODIMP CTableTd::get_Stat(eStatus* Value)
{
    *Value = (eStatus)m_tb->stat();
    return S_OK;
}

STDMETHODIMP CTableTd::put_Stat(eStatus Value)
{
    m_tb->setStat(Value);
    return S_OK;
}

STDMETHODIMP CTableTd::put_KeyNum(short Value)
{
    m_tb->setKeyNum((char_td)Value);
    return S_OK;
}

STDMETHODIMP CTableTd::FindFirst()
{
    m_tb->findFirst();
    return S_OK;
}

STDMETHODIMP CTableTd::FindLast()
{
    m_tb->findLast();
    return S_OK;
}

STDMETHODIMP CTableTd::FindNext(VARIANT_BOOL notIncCurrent)
{
    m_tb->findNext((bool)notIncCurrent);
    return S_OK;
}

STDMETHODIMP CTableTd::FindPrev(VARIANT_BOOL notIncCurrent)
{
    m_tb->findPrev((bool)notIncCurrent);
    return S_OK;
}

STDMETHODIMP CTableTd::put_Filter(BSTR Value)
{
    m_tb->setFilter(Value, m_filterRejectCount, m_filterGetCount);
    return S_OK;
}

STDMETHODIMP CTableTd::UpDate(eUpdateType ncc)
{
    m_tb->update((client::nstable::eUpdateType)ncc);
    return S_OK;
}

STDMETHODIMP CTableTd::StepFirst(eLockType lockBias)
{
    m_tb->stepFirst((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::StepLast(eLockType lockBias)
{
    m_tb->stepLast((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::StepPrev(eLockType lockBias)
{
    m_tb->stepPrev((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::StepNext(eLockType lockBias)
{
    m_tb->stepNext((ushort_td)lockBias);
    return S_OK;
}

STDMETHODIMP CTableTd::AbortBulkInsert()
{
    m_tb->abortBulkInsert();
    return S_OK;
}

STDMETHODIMP CTableTd::BeginBulkInsert(long Value)
{
    m_tb->beginBulkInsert(Value);
    return S_OK;
}

STDMETHODIMP CTableTd::CommitBulkInsert()
{
    m_tb->commitBulkInsert(false);
    return S_OK;
}

STDMETHODIMP CTableTd::get_FilterGetCount(long* Value)
{
    *Value = m_filterGetCount;
    return S_OK;
}

STDMETHODIMP CTableTd::get_FilterRejectCount(long* Value)
{
    *Value = m_filterRejectCount;
    return S_OK;
}

STDMETHODIMP CTableTd::put_FilterGetCount(long Value)
{
    m_filterGetCount = Value;
    return S_OK;
}

STDMETHODIMP CTableTd::put_FilterRejectCount(long Value)
{
    m_filterRejectCount = Value;
    return S_OK;
}

STDMETHODIMP CTableTd::Field(VARIANT Index, IField** retVal)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

    if (m_fieldObj == NULL)
    {
        CComObject<CField>::CreateInstance(&m_fieldObj);
        if (!m_fieldObj)
            return Error("CreateInstance Field", IID_ITable);
        m_fieldObj->AddRef();
    }
    try
    {
        client::fields fds(*m_tb);
        m_fieldObj->m_fd = fds[index];
        IField* fd;
        m_fieldObj->QueryInterface(IID_IField, (void**)&fd);
        _ASSERTE(fd);
        *retVal = fd;
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_ITable);
    }
}

STDMETHODIMP CTableTd::get_CanDelete(VARIANT_BOOL* Value)
{
    *Value = m_tb->canDelete();
    return S_OK;
}

STDMETHODIMP CTableTd::get_CanInsert(VARIANT_BOOL* Value)
{
    *Value = m_tb->canInsert();
    return S_OK;
}

STDMETHODIMP CTableTd::get_CanRead(VARIANT_BOOL* Value)
{
    *Value = m_tb->canRead();
    return S_OK;
}

STDMETHODIMP CTableTd::get_CanWrite(VARIANT_BOOL* Value)
{
    *Value = m_tb->canWrite();
    return S_OK;
}

STDMETHODIMP CTableTd::ClearOwnerName(void)
{
    m_tb->clearOwnerName();
    return S_OK;
}

STDMETHODIMP CTableTd::CreateIndex(VARIANT_BOOL specifyKeyNum)
{
    m_tb->createIndex(specifyKeyNum);
    return S_OK;
}

STDMETHODIMP CTableTd::DropIndex(VARIANT_BOOL norenumber)
{
    m_tb->dropIndex(norenumber);
    return S_OK;
}

STDMETHODIMP CTableTd::get_Datalen(unsigned int* Value)
{
    *Value = m_tb->datalen();
    return S_OK;
}

STDMETHODIMP CTableTd::get_WriteImageLen(unsigned int* Value)
{
    *Value = m_tb->getWriteImageLen();
    return S_OK;
}

STDMETHODIMP CTableTd::get_IsOpen(VARIANT_BOOL* Value)
{
    *Value = m_tb->isOpen();
    return S_OK;
}

STDMETHODIMP CTableTd::SetAccessRights(unsigned char curd)
{
    m_tb->setAccessRights(curd);
    return S_OK;
}

STDMETHODIMP CTableTd::SetOwnerName(BSTR* name, short enctype)
{
    m_tb->setOwnerName(*name, (char_td)enctype);
    return S_OK;
}

STDMETHODIMP CTableTd::TdapErr(OLE_HANDLE hWnd, BSTR* Value)
{
    if (Value)
    {
        wchar_t tmp[512] = { NULL };
        m_tb->tdapErr((HWND)hWnd, tmp);
        *Value = ::SysAllocString(tmp);
    }
    else
        m_tb->tdapErr((HWND)hWnd);
    return S_OK;
}

STDMETHODIMP CTableTd::StatMsg(BSTR* Value)
{
    wchar_t tmp[1024] = { NULL };
    m_tb->tdapErr(0, tmp);
    *Value = ::SysAllocString(tmp);
    return S_OK;
}

STDMETHODIMP CTableTd::Unlock_()
{
    m_tb->unlock();
    return S_OK;
}

STDMETHODIMP CTableTd::get_BlobFieldUsed(VARIANT_BOOL* Value)
{
    *Value = m_tb->blobFieldUsed();
    return S_OK;
}

STDMETHODIMP CTableTd::get_BookmarkFindCurrent(IBookmark** retVal)
{
    CComObject<CBookmark>* bm;
    CComObject<CBookmark>::CreateInstance(&bm);
    if (!bm)
        return Error("CreateInstance Bookmark", IID_ITable);
    bm->set(m_tb->bookmarkFindCurrent(), m_tb->bookmarkLen());
    IBookmark* b;
    bm->QueryInterface(IID_IBookmark, (void**)&b);
    _ASSERTE(b);
    *retVal = b;
    return S_OK;
}

STDMETHODIMP CTableTd::get_BookmarksCount(int* Value)
{

    *Value = m_tb->bookmarksCount();
    return S_OK;
}

STDMETHODIMP CTableTd::Find(eFindType FindType)
{

    m_tb->find((client::table::eFindType)FindType);
    return S_OK;
}

STDMETHODIMP CTableTd::get_RecordHash(unsigned int* Value)
{

    *Value = m_tb->getRecordHash();
    return S_OK;
}

STDMETHODIMP CTableTd::get_LogicalToString(VARIANT_BOOL* Value)
{
    *Value = m_tb->logicalToString();
    return S_OK;
}

STDMETHODIMP CTableTd::put_LogicalToString(VARIANT_BOOL Value)
{
    m_tb->setLogicalToString(Value);
    return S_OK;
}

STDMETHODIMP CTableTd::MoveBookmarks(long Value)
{
    m_tb->moveBookmarks(Value);
    return S_OK;
}

STDMETHODIMP CTableTd::get_MyDateTimeValueByBtrv(VARIANT_BOOL* Value)
{
    *Value = m_tb->myDateTimeValueByBtrv();
    return S_OK;
}

STDMETHODIMP CTableTd::get_ValiableFormatType(VARIANT_BOOL* Value)
{
    *Value = m_tb->valiableFormatType();
    return S_OK;
}

STDMETHODIMP CTableTd::SmartUpdate(void)
{
    m_tb->smartUpdate();
    return S_OK;
}

STDMETHODIMP CTableTd::KeyValueDescription(BSTR* Value)
{
    wchar_t tmp[8192];
    m_tb->keyValueDescription(tmp, 8192);
    *Value = tmp;
    return S_OK;
}

STDMETHODIMP CTableTd::SetQuery(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal)
{
    if (Value)
    {
        *retVal = NULL;
        CQueryBase* p = dynamic_cast<CQueryBase*>(Value);
        if (p)
        {
            CComObject<CPreparedQuery>* rsObj;
            CComObject<CPreparedQuery>::CreateInstance(&rsObj);

            if (!rsObj)
                return Error(_T("Can not create preparedQuery"), IID_ITable);
            client::pq_handle pq = m_tb->setQuery(&p->query(), (bool)ServerPrepare);
            if (pq)
            {
                rsObj->setPqHandle(pq);
                IPreparedQuery* pd;
                rsObj->QueryInterface(IID_IPreparedQuery, (void**)&pd);
                _ASSERTE(pd);
                *retVal = pd;
                
            }
            return S_OK;
            /*else
            {
                _TCHAR buf[1024];
                client::table::tdapErr(NULL, m_tb->stat(), m_tb->tableDef()->tableName(), buf);
                return Error(buf, IID_ITable);
            }*/
        }
    }
    return S_FALSE;
}

STDMETHODIMP CTableTd::Prepare(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal)
{
    return SetQuery(Value, ServerPrepare, retVal);
}

STDMETHODIMP CTableTd::SetPrepare(IPreparedQuery* Value)
{
    if (Value)
    {
        CPreparedQuery* p = dynamic_cast<CPreparedQuery*>(Value);
        if (p)
        {
            m_tb->setPrepare(p->getFilter());
            return S_OK;
        }
    }
    return Error(_T("PreparedQuery is NULL"), IID_ITable);
}

STDMETHODIMP CTableTd::FieldNumByName(BSTR Name, short* Value)
{
    *Value = m_tb->fieldNumByName(Name);
    return S_FALSE;
}

STDMETHODIMP CTableTd::get_StatReasonOfFind(short* Value)
{
    *Value = m_tb->statReasonOfFind();
    return S_FALSE;
}

STDMETHODIMP CTableTd::get_LastFindDirection(short* Value)
{
    *Value = (short)m_tb->lastFindDirection();
    return S_FALSE;
}

STDMETHODIMP CTableTd::get_BookmarkLen(unsigned short* Value)
{
    *Value = m_tb->bookmarkLen();
    return S_FALSE;
}

STDMETHODIMP CTableTd::get_Bookmarks(long index, IBookmark** retVal)
{
    CComObject<CBookmark>* bm;
    CComObject<CBookmark>::CreateInstance(&bm);
    if (!bm)
        return Error("CreateInstance Bookmark", IID_ITable);
    bm->set(m_tb->bookmarks((unsigned int)index), m_tb->bookmarkLen());
    IBookmark* b;
    bm->QueryInterface(IID_IBookmark, (void**)&b);
    _ASSERTE(b);
    *retVal = b;
    return S_OK;
}

STDMETHODIMP CTableTd::SetTimestampMode(eTimeStampMode mode)
{
    m_tb->setTimestampMode((int)mode);
    return S_OK;
}

STDMETHODIMP CTableTd::GetFVbits(VARIANT Index, IBitset** Value)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);
    
    CComObject<CBitset>* b;
    CComObject<CBitset>::CreateInstance(&b);
    if (!b)
        return Error("CreateInstance Bitset", IID_ITable);
    b->m_bitset = bzs::db::protocol::tdap::bitset(m_tb->getFV64(index));
    CBitset* bi;
    b->QueryInterface(IID_IBitset, (void**)&bi);
    _ASSERTE(bi);
    *Value = bi;
    return S_OK;
}

STDMETHODIMP CTableTd::GetCreateSql(BSTR* retVal)
{
    uint_td size = 65000;
    char* tmp = new char[size];
    wchar_t* tmpw = new wchar_t[size];
    m_tb->getCreateSql(tmp, &size);
    MultiByteToWideChar(CP_UTF8, 0, tmp, -1, tmpw, size);
    CComBSTR ret = tmpw;
    *retVal = ret.Copy();
    delete [] tmp;
    delete [] tmpw;
    return S_OK;
}

STDMETHODIMP CTableTd::get_UpdateConflictCheck(VARIANT_BOOL* Value)
{
    *Value = m_tb->updateConflictCheck();
    return S_OK;
}

STDMETHODIMP CTableTd::SetUpdateConflictCheck(VARIANT_BOOL Value, VARIANT_BOOL* retVal)
{
    *retVal = m_tb->setUpdateConflictCheck(Value);
    return S_OK;
}
STDMETHODIMP CTableTd::SetAlias(BSTR orign,  BSTR alias)
{
    m_tb->setAlias(orign, alias);
    return S_OK;
}

void __stdcall onRecordCount(bzs::db::protocol::tdap::client::table* tb,
                          int count, bool& cancel)
{
    CTableTd* ctb = reinterpret_cast<CTableTd*>(tb->optionalData());
    ITable* tbPtr = dynamic_cast<ITable*>(ctb);
    _ASSERTE(tbPtr);
    VARIANT_BOOL tmp = 0;
    ctb->Fire_OnRecordCount(tbPtr, count, &tmp);
    if (tmp)
        cancel = true;
}

