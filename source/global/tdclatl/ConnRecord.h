#pragma once
/*=================================================================
   Copyright (C) 2016 BizStation Corp All rights reserved.

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
#include <bzs/db/transactd/connectionRecord.h>

using namespace ATL;

class ATL_NO_VTABLE CConnRecord
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CConnRecord, &CLSID_ConnRecord>,
      public ISupportErrorInfo,
      public IDispatchImpl<IConnRecord, &IID_IConnRecord, &LIBID_transactd,
                           /*wMajor =*/1, /*wMinor =*/0>
{
    
    void setResult(IConnRecord** retVal);

public:
    bzs::db::transactd::connection::record m_rec;
    CConnRecord() {}

    BEGIN_COM_MAP(CConnRecord)
    COM_INTERFACE_ENTRY(IConnRecord)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct() { return S_OK; }
    void FinalRelease() {}

public:
#ifdef _WIN64
    STDMETHOD (get_ConId)(__int64* retVal);
#else
    STDMETHOD (get_ConId)(BSTR* retVal);
#endif
    STDMETHOD (get_Id)(int* retVal); //unsigned int value_id;
    STDMETHOD (get_Db)(short* retVal);
    STDMETHOD (get_Type)(short* retVal); //trnType
    STDMETHOD (get_Name)(BSTR* retVal); 
    STDMETHOD (get_Value)(BSTR* retVal); 
    STDMETHOD (get_Status)(short* retVal); 
    STDMETHOD (get_ReadCount)(int* retVal); 
    STDMETHOD (get_UpdCount)(int* retVal); 
    STDMETHOD (get_DelCount)(int* retVal); 
    STDMETHOD (get_InsCount)(int* retVal); 
};

