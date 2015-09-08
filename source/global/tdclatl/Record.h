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
#include <bzs/db/protocol/tdap/client/memRecord.h>

using namespace ATL;
class CField;
class CFieldDefs;
class ATL_NO_VTABLE CRecord
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CRecord, &CLSID_Record>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IRecord, &IID_IRecord, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{

    CComObject<CField>* m_fieldObj;
    short GetFieldNum(VARIANT* Index);

public:
    bzs::db::protocol::tdap::client::fieldsBase* m_rec;
    CRecord() : m_fieldObj(NULL) {}

    BEGIN_COM_MAP(CRecord)
    COM_INTERFACE_ENTRY(IRecord)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease();

public:
    STDMETHOD(get_Size)(short* retVal);
    STDMETHOD(get_Field)(VARIANT Index, IField** retVal);
    STDMETHOD(get_IsInvalidRecord)(VARIANT_BOOL* retVal);
};

class ATL_NO_VTABLE CWritableRecord
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CWritableRecord, &CLSID_Record>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IWritableRecord, &IID_IWritableRecord,
                           &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{

    CComObject<CField>* m_fieldObj;
    CComObject<CFieldDefs>* m_fieldDefsObj;
    short GetFieldNum(VARIANT* Index);

public:
    bzs::db::protocol::tdap::client::writableRecord* m_rec;
    CWritableRecord() : m_fieldObj(NULL), m_fieldDefsObj(NULL) {}

    BEGIN_COM_MAP(CWritableRecord)
    COM_INTERFACE_ENTRY(IWritableRecord)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease();

public:
    STDMETHOD(Clear)();
    STDMETHOD(get_Size)(short* retVal);
    STDMETHOD(get_Field)(VARIANT Index, IField** retVal);
    STDMETHOD(Save)();
    STDMETHOD(Insert)();
    STDMETHOD(Del)(VARIANT_BOOL KeysetAlrady);
    STDMETHOD(Update)();
    STDMETHOD(Read)(VARIANT KeysetAlradyOrBookmark, VARIANT_BOOL* retVal);
    STDMETHOD(get_FieldDefs)(IFieldDefs** retVal);
    STDMETHOD(get_IsInvalidRecord)(VARIANT_BOOL* retVal);
};
