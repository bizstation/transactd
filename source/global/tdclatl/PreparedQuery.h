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
#include <bzs/db/protocol/tdap/client/activeTable.h>

using namespace ATL;
class ATL_NO_VTABLE CPreparedQuery
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CPreparedQuery, &CLSID_PreparedQuery>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IPreparedQuery, &IID_IPreparedQuery, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
    bzs::db::protocol::tdap::client::preparedQuery* m_pq;
public:

    CPreparedQuery():m_pq(NULL){}
    void setPqHandle(bzs::db::protocol::tdap::client::pq_handle& stmt)
    {
        if (m_pq)
            delete m_pq;
         m_pq = new bzs::db::protocol::tdap::client::preparedQuery(stmt);
    }
    bzs::db::protocol::tdap::client::pq_handle& getFilter() {return m_pq->getFilter();}

    BEGIN_COM_MAP(CPreparedQuery)
    COM_INTERFACE_ENTRY(IPreparedQuery)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease(){delete m_pq;};

public:
    STDMETHOD(SupplyValue)(int Index, VARIANT Value, VARIANT_BOOL* retVal);
    STDMETHOD(AddValue)(VARIANT Value, VARIANT_BOOL* retVal);
    STDMETHOD(ResetAddIndex)();
    void addValue(BSTR v) {m_pq->addValue(v);}
};

