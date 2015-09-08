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
#include <bzs/db/protocol/tdap/client/pooledDatabaseManager.h>

using namespace ATL;

class ATL_NO_VTABLE CPooledDbManager
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CPooledDbManager, &CLSID_PooledDbManager>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IPooledDbManager, &IID_IPooledDbManager,
                           &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{

public:
    bzs::db::protocol::tdap::client::pooledDbManager m_mgr;

    CPooledDbManager() {}

    DECLARE_REGISTRY_RESOURCEID(IDR_POOLEDDBMGR)

    BEGIN_COM_MAP(CPooledDbManager)
    COM_INTERFACE_ENTRY(IPooledDbManager)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease(){};

public:
    STDMETHOD(put_MaxConnections)(int n);
    STDMETHOD(get_MaxConnections)(int* retVal);
    STDMETHOD(Reserve)(int size, IConnectParams* param);
    STDMETHOD(Reset)(int waitSec);

    STDMETHOD(get_Stat)(eStatus* Value);
    STDMETHOD(BeginTrn)(eLockType Bias);
    STDMETHOD(EndTrn)(void);
    STDMETHOD(AbortTrn)(void);
    STDMETHOD(BeginSnapshot)(eStLockType bias);
    STDMETHOD(EndSnapshot)(void);
    STDMETHOD(Use)(VARIANT Uri);
    STDMETHOD(Unuse)();
    STDMETHOD(get_EnableTrn)(VARIANT_BOOL* Value);
    STDMETHOD(get_Uri)(BSTR* Uri);
    STDMETHOD(get_IsOpened)(VARIANT_BOOL* Value);
    STDMETHOD(get_ClientID)(short* Value);
    STDMETHOD(Table)(BSTR name, ITable** retVal);
    STDMETHOD(Db)(IDatabase** retval);
    STDMETHOD(put_Option)(__int64 Value);
    STDMETHOD(get_Option)(__int64* Value);
    STDMETHOD(get_Mode)(short* Value);
};

OBJECT_ENTRY_AUTO(__uuidof(PooledDbManager), CPooledDbManager)