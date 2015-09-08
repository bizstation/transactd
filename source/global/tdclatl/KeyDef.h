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
#include <bzs/db/protocol/tdap/client/dbDef.h>
using namespace ATL;

class ATL_NO_VTABLE CKeyDef
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CKeyDef, &CLSID_KeyDef>,
      public IDispatchImpl<IKeyDef, &IID_IKeyDef, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
    bzs::db::protocol::tdap::keydef* keydef()
    {
        return &(*m_tabledefPtr)->keyDefs[m_index];
    }

public:
    CKeyDef() : m_tabledefPtr(NULL) {}
    bzs::db::protocol::tdap::tabledef** m_tabledefPtr;
    short m_index;

    BEGIN_COM_MAP(CKeyDef)
    COM_INTERFACE_ENTRY(IKeyDef)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(get_SegmentCount)(unsigned char* Value);
    STDMETHOD(get_Segments)(short Index, IKeySegment** Value);
    STDMETHOD(put_SegmentCount)(unsigned char Value);
};
