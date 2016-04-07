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
#include <bzs/db/protocol/tdap/client/connMgr.h>

using namespace ATL;

class ATL_NO_VTABLE CConnMgr
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CConnMgr, &CLSID_ConnMgr>,
      public ISupportErrorInfo,
      public IDispatchImpl<IConnMgr, &IID_IConnMgr, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{
    bzs::db::protocol::tdap::client::connMgr* m_mgr;

public:
    CConnMgr() : m_mgr(NULL) {}

    DECLARE_REGISTRY_RESOURCEID(IDR_CONNMGR)

    BEGIN_COM_MAP(CConnMgr)
    COM_INTERFACE_ENTRY(IConnMgr)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct() { return S_OK; }
    void FinalRelease() { if (m_mgr)m_mgr->release(); }

public:
    STDMETHOD(SetDatabase)(VARIANT db);

    STDMETHOD(Connect)(BSTR uri, VARIANT_BOOL* retVal);
    STDMETHOD(Disconnect)();
    STDMETHOD(Databases)(IConnRecords** retVal);
    STDMETHOD(Tables)(BSTR dbname, IConnRecords** retVal);
    STDMETHOD(Views)(BSTR dbname, IConnRecords** retVal);
    STDMETHOD(SchemaTables)(BSTR dbname, IConnRecords** retVal);
    STDMETHOD(Sysvars)(IConnRecords** retVal);
    STDMETHOD(SlaveStatus)(IConnRecords** retVal);
    STDMETHOD(Connections)(IConnRecords** retVal);
#ifdef _WIN64
    STDMETHOD(InUseDatabases)(__int64 connid, IConnRecords** retVal);
    STDMETHOD(InUseTables)(__int64 connid, int db, IConnRecords** retVal);
    STDMETHOD(PostDisconnectOne)(__int64 connid);
#else
    STDMETHOD(InUseDatabases)(BSTR connid, IConnRecords** retVal);
    STDMETHOD(InUseTables)(BSTR connid, int db, IConnRecords** retVal);
    STDMETHOD(PostDisconnectOne)(BSTR connid);
#endif
    STDMETHOD(PostDisconnectAll)();
    STDMETHOD(get_Stat)(short* retVal);
    STDMETHOD(TdapErr)(OLE_HANDLE hWnd, BSTR* retVal);
    
    STDMETHOD(RemoveSystemDb)(IConnRecords** retVal);
    STDMETHOD(SlaveStatusName)(int index, BSTR* retVal);
    STDMETHOD(SysvarName)(int index, BSTR* retVal);

};

OBJECT_ENTRY_AUTO(__uuidof(ConnMgr), CConnMgr)
