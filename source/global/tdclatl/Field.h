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
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/field.h>

using namespace ATL;

class ATL_NO_VTABLE CField
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CField, &CLSID_Field>,
      public IDispatchImpl<IField, &IID_IField, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
    CField() : m_tb(NULL) {}
    bzs::db::protocol::tdap::client::field m_fd;
    bzs::db::protocol::tdap::client::table* m_tb;
    short m_index;

    BEGIN_COM_MAP(CField)
    COM_INTERFACE_ENTRY(IField)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(put_Text)(BSTR Value);
    STDMETHOD(get_Text)(BSTR* Value);
    STDMETHOD(put_Vlng)(int Value);
    STDMETHOD(get_Vlng)(int* Value);
    STDMETHOD(put_V64)(__int64 Value);
    STDMETHOD(get_V64)(__int64* Value);
    STDMETHOD(put_Vdbl)(double Value);
    STDMETHOD(get_Vdbl)(double* Value);
    STDMETHOD(put_Vbin)(BSTR Value);
    STDMETHOD(get_Vbin)(BSTR* Value);

    STDMETHOD(IsNull)(VARIANT_BOOL* Value);
    STDMETHOD(SetNull)(VARIANT_BOOL Value);
    STDMETHOD(SetValue)(VARIANT Value);
    STDMETHOD(I)(int* Value);
    STDMETHOD(I64)(__int64* Value);
    STDMETHOD(D)(double* Value);
    STDMETHOD(Bin)(BSTR* Value);
    STDMETHOD(Str)(BSTR* Value);
    STDMETHOD(get_Type)(short* Value);
    STDMETHOD(get_Len)(short* Value);

};
