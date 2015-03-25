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

class ATL_NO_VTABLE CFieldDef
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CFieldDef, &CLSID_FieldDef>,
      public IDispatchImpl<IFieldDef, &IID_IFieldDef, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
    bzs::db::protocol::tdap::fielddef* fielddef()
    {
        if (m_fielddef)
            return NULL;
        return &(*m_tabledefPtr)->fieldDefs[m_index];
    }

    const bzs::db::protocol::tdap::fielddef* const_fielddef()
    {
        if (m_fielddef)
            return m_fielddef;
        return &(*m_tabledefPtr)->fieldDefs[m_index];
    }
    bool isWritabale() { return (m_tabledefPtr != NULL); }
    HRESULT write_error()
    {
        return Error("This object is no writable.", IID_IFieldDef);
    }

public:
    CFieldDef() : m_tabledefPtr(NULL), m_fielddef(NULL) {}
    const bzs::db::protocol::tdap::fielddef* m_fielddef;
    bzs::db::protocol::tdap::tabledef** m_tabledefPtr;
    short m_index;

    BEGIN_COM_MAP(CFieldDef)
    COM_INTERFACE_ENTRY(IFieldDef)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(get_Name)(BSTR* Value);
    STDMETHOD(get_Type)(eFieldType* Value);
    STDMETHOD(get_Len)(short* Value);
    STDMETHOD(get_Decimals)(unsigned char* Value);
    STDMETHOD(get_Max)(double* Value);
    STDMETHOD(get_Min)(double* Value);
    STDMETHOD(get_DefValue)(double* Value);
    STDMETHOD(get_LookTable)(unsigned char* Value);
    STDMETHOD(get_LookField)(unsigned char* Value);
    STDMETHOD(get_LookViewField)(short Index, unsigned char* Value);
    STDMETHOD(get_EnableFlags)(IFlags** Value);

    STDMETHOD(put_Decimals)(unsigned char Value);
    STDMETHOD(put_DefValue)(double Value);
    STDMETHOD(put_EnableFlags)(IFlags* Value);
    STDMETHOD(put_Len)(short Value);
    STDMETHOD(put_LookField)(unsigned char Value);
    STDMETHOD(put_LookTable)(unsigned char Value);
    STDMETHOD(put_LookViewField)(short Index, unsigned char Value);
    STDMETHOD(put_Max)(double Value);
    STDMETHOD(put_Min)(double Value);
    STDMETHOD(put_Name)(BSTR Value);
    STDMETHOD(put_Type)(eFieldType Value);
    STDMETHOD(get_Keylen)(unsigned short* Value);
    STDMETHOD(put_Keylen)(unsigned short Value);
    STDMETHOD(get_NullValue)(unsigned short* Value);
    STDMETHOD(put_NullValue)(unsigned short Value);
    STDMETHOD(get_Align)(unsigned int* Value);
    STDMETHOD(get_TypeName)(BSTR* Value);
    STDMETHOD(get_IsStringType)(VARIANT_BOOL* Value);
    STDMETHOD(get_CharsetIndex)(eCharset* Value);
    STDMETHOD(put_CharsetIndex)(eCharset Value);
    STDMETHOD(get_CodePage)(unsigned int* Value);
    STDMETHOD(get_CharNum)(unsigned int* Value);
    STDMETHOD(SetLenByCharnum)(unsigned short Value);
    STDMETHOD(get_Index)(short* Value);
	STDMETHOD(SetPadCharSettings)(VARIANT_BOOL set, VARIANT_BOOL trim);
	STDMETHOD(get_UsePadChar)(VARIANT_BOOL* Value);
	STDMETHOD(get_TrimPadChar)(VARIANT_BOOL* Value);

};
