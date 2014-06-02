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


class ATL_NO_VTABLE CGroupQuery : 
	public CComObjectRootEx<CComSingleThreadModel>, 
	public CComCoClass<CGroupQuery, &CLSID_GroupQuery>,
    public IDispatchImpl<IGroupQuery, &IID_IGroupQuery, &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{
	void setResult(IGroupQuery** retVal);

public:
	bzs::db::protocol::tdap::client::groupQuery m_gq;
    CGroupQuery(){}

	DECLARE_REGISTRY_RESOURCEID(IDR_GROUPQUERY)

    BEGIN_COM_MAP(CGroupQuery) 
		COM_INTERFACE_ENTRY(IGroupQuery) 
		COM_INTERFACE_ENTRY(IDispatch) 
	END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() {return S_OK;}

    void FinalRelease(){};

public:
  STDMETHOD(KeyField)(BSTR Name, BSTR Name1, BSTR Name2, BSTR Name3, BSTR Name4, BSTR Name5,
				BSTR Name6, BSTR Name7,	BSTR Name8, BSTR Name9,	BSTR Name10,
				IGroupQuery** retVal);
  STDMETHOD(ResultField)(BSTR Name, IGroupQuery** retVal);
  //STDMETHOD(Having)(IQueryBase* query , IGroupQuery** retVal);
  STDMETHOD(Reset)(IGroupQuery** retVal);

};

OBJECT_ENTRY_AUTO(__uuidof(GroupQuery), CGroupQuery)