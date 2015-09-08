#pragma once
/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include <bzs/db/protocol/tdap/client/groupQuery.h>

using namespace ATL;

class ATL_NO_VTABLE CRecordsetQuery
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CRecordsetQuery, &CLSID_RecordsetQuery>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IRecordsetQuery, &IID_IRecordsetQuery,
                           &LIBID_transactd, /*wMajor =*/1, /*wMinor =*/0>
{

    void setResult(IRecordsetQuery** retVal);

public:
    bzs::db::protocol::tdap::client::recordsetQuery m_qb;
    CRecordsetQuery() {}

    DECLARE_REGISTRY_RESOURCEID(IDR_RECORDSETQUERY)

    BEGIN_COM_MAP(CRecordsetQuery)
    COM_INTERFACE_ENTRY(IRecordsetQuery)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    bzs::db::protocol::tdap::client::recordsetQuery& query() { return m_qb; };
    STDMETHOD(Reset)(IRecordsetQuery** retVal);

    STDMETHOD(When)(BSTR Name, BSTR Logic, VARIANT Value,
                    IRecordsetQuery** retVal);
    STDMETHOD(And)(BSTR Name, BSTR Logic, VARIANT Value,
                   IRecordsetQuery** retVal);
    STDMETHOD(Or)(BSTR Name, BSTR Logic, VARIANT Value,
                  IRecordsetQuery** retVal);
};

class ATL_NO_VTABLE CSortField
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CSortField, &CLSID_SortField>,
      public IDispatchImpl<ISortField, &IID_ISortField, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{

public:
    bzs::db::protocol::tdap::client::sortField m_sortField;
    CSortField() {}

    BEGIN_COM_MAP(CSortField)
    COM_INTERFACE_ENTRY(ISortField)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(get_Name)(BSTR* Value);
    STDMETHOD(put_Name)(BSTR Value);
    STDMETHOD(get_Asc)(VARIANT_BOOL* Value);
    STDMETHOD(put_Asc)(VARIANT_BOOL Value);
};

class ATL_NO_VTABLE CSortFields
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CSortFields, &CLSID_SortFields>,
      public IDispatchImpl<ISortFields, &IID_ISortFields, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{

public:
    bzs::db::protocol::tdap::client::sortFields m_sortFields;
    CSortFields() {}

    DECLARE_REGISTRY_RESOURCEID(IDR_SORTFIELDS)

    BEGIN_COM_MAP(CSortFields)
    COM_INTERFACE_ENTRY(ISortFields)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(Add)(BSTR Name, VARIANT_BOOL Asc);
    STDMETHOD(get_Size)(int* Value);
    STDMETHOD(get_Item)(int Index, ISortField** retVal);
    STDMETHOD(Clear)();
};

OBJECT_ENTRY_AUTO(__uuidof(RecordsetQuery), CRecordsetQuery)
OBJECT_ENTRY_AUTO(__uuidof(SortFields), CSortFields)