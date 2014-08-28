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
#include <bzs/db/protocol/tdap/client/trdboostapi.h>



using namespace ATL;

class ATL_NO_VTABLE CConnectParams : 
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CConnectParams, &CLSID_ConnectParams>,
	public IDispatchImpl<IConnectParams, &IID_IConnectParams, &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{

   bzs::db::protocol::tdap::client::connectParams* m_param;

public:
    CConnectParams():m_param(NULL)
    { 
	}

	bzs::db::protocol::tdap::client::connectParams* internalConnectParams(){return m_param;}
	DECLARE_REGISTRY_RESOURCEID(IDR_CONNECTPARAM)

	BEGIN_COM_MAP(CConnectParams) 
		COM_INTERFACE_ENTRY(IConnectParams) 
		COM_INTERFACE_ENTRY(IDispatch) 
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() 
	{
		delete m_param;
		return S_OK;
	}

	void FinalRelease(){};
public:
 
	STDMETHOD(Init)(BSTR protocol, BSTR hostOrIp, BSTR dbname, BSTR schemaTable);
	STDMETHOD(put_Uri)(BSTR val);
	STDMETHOD(get_Uri)(BSTR* retVal);
	STDMETHOD(put_Mode)(short val);
	STDMETHOD(get_Mode)(short* retVal);
	STDMETHOD(put_Type)(short val);
	STDMETHOD(get_Type)(short* retVal);

	

};

OBJECT_ENTRY_AUTO(__uuidof(ConnectParams), CConnectParams)
