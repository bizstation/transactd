#pragma once
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
#include "resource.h"

#include "tdclatl_i.h"
#include "Field.h"
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include "_IDatabaseEvents_CP.H"

using namespace ATL;
void __stdcall onRecordCount(bzs::db::protocol::tdap::client::table* tb,
                          int count, bool& cancel);

class ATL_NO_VTABLE CTableTd
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CTableTd, &CLSID_Table>,
	  public ISupportErrorInfo,
      public IDispatchImpl<ITable, &IID_ITable, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>,
      public IConnectionPointContainerImpl<CTableTd>,
      public CProxy_ITableEvents<CTableTd>

{

    int m_filterRejectCount;
    int m_filterGetCount;
    bzs::db::protocol::tdap::tabledef* m_tabledef;
    short GetFieldNum(VARIANT* Index);
    CComBSTR m_str;
    CComObject<CField>* m_fieldObj;

public:
    CTableTd() : m_filterRejectCount(1), m_filterGetCount(0), m_fieldObj(NULL)
    {
    }

    bzs::db::protocol::tdap::client::table_ptr m_tb;

    BEGIN_COM_MAP(CTableTd)
    COM_INTERFACE_ENTRY(ITable)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    BEGIN_CONNECTION_POINT_MAP(CTableTd)
    CONNECTION_POINT_ENTRY(__uuidof(_ITableEvents))
    END_CONNECTION_POINT_MAP()


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease()
    {
        if (m_fieldObj)
            m_fieldObj->Release();
    };

public:
    STDMETHOD(get_Text)(VARIANT Index, BSTR* Value);
    STDMETHOD(get_Vlng)(VARIANT Index, int* Value);
    STDMETHOD(put_Text)(VARIANT Index, BSTR Value);
    STDMETHOD(put_Vlng)(VARIANT Index, int Value);
    STDMETHOD(get_TableDef)(ITableDef** Value);
    STDMETHOD(Insert)(eUpdateType ncc);
    STDMETHOD(Delete)(VARIANT_BOOL inkey);
    STDMETHOD(ClearBuffer)(eNullReset resetType);
    STDMETHOD(Close)();
    STDMETHOD(SeekFirst)(eLockType lockBias);
    STDMETHOD(SeekLast)(eLockType lockBias);
    STDMETHOD(SeekPrev)(eLockType lockBias);
    STDMETHOD(SeekNext)(eLockType lockBias);
    STDMETHOD(Seek)(eLockType lockBias);
    STDMETHOD(SeekGreater)(VARIANT_BOOL orEqual, eLockType lockBias);
    STDMETHOD(SeekLessThan)(VARIANT_BOOL orEqual, eLockType lockBias);
    STDMETHOD(get_Bookmark)(IBookmark** Value);
    STDMETHOD(SeekByBookmark)(IBookmark* bm, eLockType lockBias);
    STDMETHOD(get_Percentage)(VARIANT param, long* Value);
    STDMETHOD(get_RecordLength)(long* Value);
    STDMETHOD(RecordCount)(VARIANT_BOOL estimate, VARIANT_BOOL fromCurrent,
                           long* Value);
    STDMETHOD(SeekByPercentage)(long Value);
    STDMETHOD(get_KeyNum)(short* Value);
    STDMETHOD(get_Stat)(eStatus* eStatus);
    STDMETHOD(put_Stat)(eStatus eStatus);
    STDMETHOD(put_KeyNum)(short Value);
    STDMETHOD(get_Vdbl)(VARIANT Index, double* Value);
    STDMETHOD(put_Vdbl)(VARIANT Index, double Value);
    STDMETHOD(get_Vbin)(VARIANT Index, BSTR* Value);
    STDMETHOD(put_Vbin)(VARIANT Index, BSTR Value);
    STDMETHOD(FindFirst)();
    STDMETHOD(FindLast)();
    STDMETHOD(FindNext)(VARIANT_BOOL notIncCurrent);
    STDMETHOD(FindPrev)(VARIANT_BOOL notIncCurrent);
    STDMETHOD(put_Filter)(BSTR Value);
    STDMETHOD(get_V64)(VARIANT Index, __int64* Value);
    STDMETHOD(put_V64)(VARIANT Index, __int64 Value);
    STDMETHOD(UpDate)(eUpdateType ncc);
    STDMETHOD(StepFirst)(eLockType lockBias);
    STDMETHOD(StepLast)(eLockType lockBias);
    STDMETHOD(StepPrev)(eLockType lockBias);
    STDMETHOD(StepNext)(eLockType lockBias);

    STDMETHOD(AbortBulkInsert)();
    STDMETHOD(BeginBulkInsert)(long Value);
    STDMETHOD(CommitBulkInsert)();
    STDMETHOD(get_FilterGetCount)(long* Value);
    STDMETHOD(get_FilterRejectCount)(long* Value);
    STDMETHOD(put_FilterGetCount)(long Value);
    STDMETHOD(put_FilterRejectCount)(long Value);
    STDMETHOD(Field)(VARIANT Index, IField** Value);
    STDMETHOD(get_IsOpen)(VARIANT_BOOL* Value);
    STDMETHOD(get_CanDelete)(VARIANT_BOOL* Value);
    STDMETHOD(get_CanInsert)(VARIANT_BOOL* Value);
    STDMETHOD(get_CanRead)(VARIANT_BOOL* Value);
    STDMETHOD(get_CanWrite)(VARIANT_BOOL* Value);
    STDMETHOD(ClearOwnerName)(void);
    STDMETHOD(CreateIndex)(VARIANT_BOOL specifyKeyNum);
    STDMETHOD(DropIndex)(VARIANT_BOOL norenumber);
    STDMETHOD(get_Datalen)(unsigned int* Value);
    STDMETHOD(get_WriteImageLen)(unsigned int* Value);
    STDMETHOD(SetAccessRights)(unsigned char curd);
    STDMETHOD(SetOwnerName)(BSTR* name, short enctype);
    STDMETHOD(TdapErr)(OLE_HANDLE hWnd, BSTR* Value);
    STDMETHOD(StatMsg)(BSTR* Value);
    STDMETHOD(Unlock_)();
    STDMETHOD(get_BlobFieldUsed)(VARIANT_BOOL* Value);
    STDMETHOD(get_BookmarkFindCurrent)(IBookmark** Value);
    STDMETHOD(get_BookmarksCount)(int* Value);
    STDMETHOD(Find)(eFindType FindType);
    STDMETHOD(get_RecordHash)(unsigned int* Value);
    STDMETHOD(get_LogicalToString)(VARIANT_BOOL* Value);
    STDMETHOD(put_LogicalToString)(VARIANT_BOOL Value);
    STDMETHOD(MoveBookmarks)(long Value);
    STDMETHOD(get_MyDateTimeValueByBtrv)(VARIANT_BOOL* Value);
    STDMETHOD(get_ValiableFormatType)(VARIANT_BOOL* Value);
    STDMETHOD(SmartUpdate)(void);
    STDMETHOD(KeyValueDescription)(BSTR* Value);
    STDMETHOD(SetQuery)(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal);
    STDMETHOD(Prepare)(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal);
    STDMETHOD(SetPrepare)(IPreparedQuery* Value);
    STDMETHOD(FieldNumByName)(BSTR Name, short* Value);
    STDMETHOD(get_StatReasonOfFind)(short* Value);
    STDMETHOD(get_LastFindDirection)(short* Value);
    STDMETHOD(get_BookmarkLen)(unsigned short* Value);
    STDMETHOD(get_Bookmarks)(long index, IBookmark** Value);
    STDMETHOD(GetFVNull)(VARIANT Index, VARIANT_BOOL* Value);
    STDMETHOD(SetFVNull)(VARIANT Index, VARIANT_BOOL Value);
    STDMETHOD(GetFVint)(VARIANT Index, int* Value);
    STDMETHOD(GetFV64)(VARIANT Index, __int64* Value);
    STDMETHOD(GetFVdbl)(VARIANT Index, double* Value);
    STDMETHOD(GetFVstr)(VARIANT Index, BSTR* Value);
    STDMETHOD(SetFV)(VARIANT Index, VARIANT Value);
    STDMETHOD(SetTimestampMode)(eTimeStampMode mode);
    STDMETHOD(GetFVbits)(VARIANT Index, IBitset** Value);
    STDMETHOD(GetCreateSql)( BSTR* retVal);
    STDMETHOD(get_UpdateConflictCheck)(VARIANT_BOOL* retVal);
    STDMETHOD(SetUpdateConflictCheck)(VARIANT_BOOL Value, VARIANT_BOOL* retVal);
    STDMETHOD(SetAlias)(BSTR orign,  BSTR alias);
};
