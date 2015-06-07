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
#include <bzs/db/protocol/tdap/client/table.h>

using namespace ATL;

class ATL_NO_VTABLE CQueryBase
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CQueryBase, &CLSID_QueryBase>,
      public IDispatchImpl<IQueryBase, &IID_IQueryBase, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{
    bzs::db::protocol::tdap::client::queryBase& m_qb;
    void setResult(IQueryBase** retVal);

public:
    CQueryBase() : m_qb(*bzs::db::protocol::tdap::client::queryBase::create())
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_QUERYBASE)

    BEGIN_COM_MAP(CQueryBase)
    COM_INTERFACE_ENTRY(IQueryBase)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() { m_qb.release(); }

public:
    bzs::db::protocol::tdap::client::queryBase& query() { return m_qb; };
    STDMETHOD(Reset)(IQueryBase** retVal);
    STDMETHOD(ClearSeekKeyValues)(void);
    STDMETHOD(ClearSelectFields)(void);
    STDMETHOD(Select)(BSTR Value, BSTR Value1, BSTR Value2, BSTR Value3,
                      BSTR Value4, BSTR Value5, BSTR Value6, BSTR Value7,
                      BSTR Value8, BSTR Value9, BSTR Value10,
                      IQueryBase** retVal);

    STDMETHOD(Where)(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal);
    STDMETHOD(And)(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal);
    STDMETHOD(Or)(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal);
    STDMETHOD(AddInValue)(VARIANT Value, VARIANT_BOOL Reset);
    STDMETHOD(In)(VARIANT Value, VARIANT Value1, VARIANT Value2, VARIANT Value3,
                  VARIANT Value4, VARIANT Value5, VARIANT Value6,
                  VARIANT Value7, VARIANT Value8, VARIANT Value9,
                  VARIANT Value10, IQueryBase** retVal);
    STDMETHOD(QueryString)(BSTR Value, IQueryBase** retVal);
    STDMETHOD(Reject)(long Value, IQueryBase** retVal);
    STDMETHOD(Limit)(long Value, IQueryBase** retVal);
    STDMETHOD(Direction)(eFindType FindType, IQueryBase** retVal);
    STDMETHOD(All)(IQueryBase** retVal);
    STDMETHOD(ToString)(BSTR* retVal);
    STDMETHOD(GetDirection)(eFindType* retVal);
    STDMETHOD(GetReject)(long* retVal);
    STDMETHOD(GetLimit)(long* retVal);
    STDMETHOD(IsAll)(VARIANT_BOOL* retVal);
    STDMETHOD(Optimize)(eOptimize Value, IQueryBase** retVal);
    STDMETHOD(GetOptimize)(eOptimize* retVal);
    STDMETHOD(SelectCount)(short* retVal);
    STDMETHOD(GetSelect)(short index, BSTR* retVal);
    STDMETHOD(WhereTokenCount)(short* retVal);
    STDMETHOD(GetWhereToken)(short index, BSTR* retVal);
    STDMETHOD(BookmarkAlso)(VARIANT_BOOL Value, IQueryBase** retVal);
    STDMETHOD(IsBookmarkAlso)(VARIANT_BOOL* retVal);
    STDMETHOD(StopAtLimit)(VARIANT_BOOL Value, IQueryBase** retVal);
    STDMETHOD(IsStopAtLimit)(VARIANT_BOOL* retVal);
    STDMETHOD(IsSeekByBookmarks)(VARIANT_BOOL* retVal);
    STDMETHOD(AddSeekKeyValue)(VARIANT Value, VARIANT_BOOL Reset);
    STDMETHOD(AddSeekBookmark)(VARIANT Value, VARIANT_BOOL Reset);

};

OBJECT_ENTRY_AUTO(__uuidof(QueryBase), CQueryBase)
