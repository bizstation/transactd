#pragma once
/*=================================================================
   Copyright (C) 2016 BizStation Corp All rights reserved.

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
#include <bzs/db/transactd/connectionRecord.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>

using namespace ATL;

class ATL_NO_VTABLE CConnRecords
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CConnRecords, &CLSID_ConnRecords>,
      public ISupportErrorInfo,
      public IDispatchImpl<IConnRecords, &IID_IConnRecords, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{
    void setResult(IConnRecords** retVal);

public:
    CConnRecords()  {}
    bzs::db::protocol::tdap::client::connMgr::records m_recs;

    BEGIN_COM_MAP(CConnRecords)
    COM_INTERFACE_ENTRY(IConnRecords)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct() {return S_OK;}
    void FinalRelease() {}

public:
    STDMETHOD(get_Record)(short index, IConnRecord** retVal);
    STDMETHOD(get_Size)(int* retVal);
};

