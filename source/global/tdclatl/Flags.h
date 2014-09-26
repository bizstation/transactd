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

class ATL_NO_VTABLE CFlags
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CFlags, &CLSID_Flags>,
      public IDispatchImpl<IFlags, &IID_IFlags, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
    CFlags() {}
    bzs::db::protocol::tdap::FLAGS* m_flags;

    BEGIN_COM_MAP(CFlags)
    COM_INTERFACE_ENTRY(IFlags)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(get_Bits)(short Index, VARIANT_BOOL* Value);
    STDMETHOD(put_Bits)(short Index, VARIANT_BOOL Value);

    STDMETHOD(All)(unsigned short* Value);

    STDMETHOD(GetBit)(short Index, VARIANT_BOOL* Value);
    STDMETHOD(SetBit)(short Index, VARIANT_BOOL Value);
};
