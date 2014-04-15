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
#include <bzs/db/protocol/tdap/client/ormRecordset.h>
using namespace ATL;

class CRecord;

class ATL_NO_VTABLE CARecordset : 
	public CComObjectRootEx<CComSingleThreadModel>, 
	public CComCoClass<CARecordset, &CLSID_Recordset>,
    public IDispatchImpl<IRecordset, &IID_IRecordset, &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{
	void setResult(IRecordset** retVal);
	CComObject<CRecord>* m_recObj;
public:
    recordset* m_rs;

	CARecordset();
	~CARecordset();
	void setRecordset(recordset* rs){m_rs = rs;}

    BEGIN_COM_MAP(CARecordset) 
		COM_INTERFACE_ENTRY(IRecordset) 
		COM_INTERFACE_ENTRY(IDispatch) 
	END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() {return S_OK;}

    void FinalRelease(); 
	

public:

  //STDMETHOD(Clear)(void);
  STDMETHOD(Record)(short Index, IRecord** retVal);
  STDMETHOD(First)(IRecord** retVal);
  STDMETHOD(Last)(IRecord** retVal);
  STDMETHOD(Top)(long Num, IRecordset** retVal);
  STDMETHOD(Erase)(long Index);
  STDMETHOD(Count)(long* retVal);
  STDMETHOD(RemoveField)(short Index, IRecordset** retVal);
  STDMETHOD(GroupBy)(IGroupQuery* gq, enum eGroupFunc func, IRecordset** retVal);
  STDMETHOD(OrderBy)( BSTR name0,  BSTR name1,  BSTR name2,  BSTR name3,  BSTR name4, 
					 BSTR name5,  BSTR name6,  BSTR name7,  BSTR name8
					, IRecordset** retVal);
  STDMETHOD(Reverse)(IRecordset** retVal);
};
