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

class ATL_NO_VTABLE CTableDef
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CTableDef, &CLSID_TableDef>,
	  public ISupportErrorInfo,
      public IDispatchImpl<ITableDef, &IID_ITableDef, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
    CTableDef() : m_tabledefPtr(NULL) {}
    bzs::db::protocol::tdap::tabledef** m_tabledefPtr;

    BEGIN_COM_MAP(CTableDef)
    COM_INTERFACE_ENTRY(ITableDef)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease() {}

public:
    STDMETHOD(get_FieldDef)(short Index, IFieldDef** Value);
    STDMETHOD(get_KeyDef)(short Index, IKeyDef** Value);
    STDMETHOD(get_FieldCount)(short* Value);
    STDMETHOD(get_KeyCount)(short* Value);
    STDMETHOD(get_PageSize)(short* Value);
    STDMETHOD(get_PreAlloc)(short* Value);
    STDMETHOD(get_FileName)(BSTR* Value);
    STDMETHOD(get_TableName)(BSTR* Value);
    STDMETHOD(get_Flags)(IFlags** Value);
    STDMETHOD(get_ParentKeyNum)(unsigned char* Value);
    STDMETHOD(get_ReplicaKeyNum)(unsigned char* Value);
    STDMETHOD(get_OptionFlags)(IFlags** Value);
    STDMETHOD(get_IconIndex)(unsigned char* Value);
    STDMETHOD(get_Id)(short* Value);
    STDMETHOD(put_FileName)(BSTR Value);
    STDMETHOD(put_Flags)(IFlags* Value);
    STDMETHOD(put_IconIndex)(unsigned char Value);
    STDMETHOD(put_Id)(short Value);
    STDMETHOD(put_OptionFlags)(IFlags* Value);
    STDMETHOD(put_PageSize)(short Value);
    STDMETHOD(put_ParentKeyNum)(unsigned char Value);
    STDMETHOD(put_PreAlloc)(short Value);
    STDMETHOD(put_ReplicaKeyNum)(unsigned char Value);
    STDMETHOD(put_TableName)(BSTR Value);
    STDMETHOD(get_Charsetindex)(eCharset* Value);
    STDMETHOD(put_Charsetindex)(eCharset Value);
    STDMETHOD(get_PrimaryKeyNum)(unsigned char* Value);
    STDMETHOD(put_PrimaryKeyNum)(unsigned char Value);
    STDMETHOD(get_FixedRecordLen)(unsigned short* Value);
    STDMETHOD(put_FixedRecordLen)(unsigned short Value);
    STDMETHOD(get_MaxRecordLen)(unsigned short* Value);
    STDMETHOD(put_MaxRecordLen)(unsigned short Value);
    STDMETHOD(get_SchemaCodePage)(unsigned int* Value);
    STDMETHOD(put_SchemaCodePage)(unsigned int Value);
    STDMETHOD(get_Version)(unsigned short* Value);
};
