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
#include <bzs/db/protocol/tdap/tdapSchema.h>

using namespace ATL;

class ATL_NO_VTABLE CTdVersion
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CTdVersion, &CLSID_TdVersion>,
      public IDispatchImpl<ITdVersion, &IID_ITdVersion, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{

public:
    CTdVersion() { memset(&m_ver, 0, sizeof(m_ver)); }
    bzs::db::protocol::tdap::btrVersion m_ver;
    BEGIN_COM_MAP(CTdVersion) COM_INTERFACE_ENTRY(ITdVersion)
        COM_INTERFACE_ENTRY(IDispatch) END_COM_MAP()

        DECLARE_PROTECT_FINAL_CONSTRUCT()

        HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease() {}

public:
    STDMETHOD(get_MajorVersion)(short* Value);
    STDMETHOD(put_MajorVersion)(short Value);
    STDMETHOD(get_MinorVersion)(short* Value);
    STDMETHOD(put_MinorVersion)(short Value);
    STDMETHOD(get_Type)(short* Value);
    STDMETHOD(put_Type)(short Value);

    STDMETHOD(ModuleTypeString)(BSTR* Value);
    STDMETHOD(ModuleVersionShortString)(BSTR* Value);
};
