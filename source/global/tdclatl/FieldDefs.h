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
#include <bzs/db/protocol/tdap/client/field.h>

using namespace ATL;
class CFieldDef;

class ATL_NO_VTABLE CFieldDefs : public CComObjectRootEx<CComSingleThreadModel>, public CComCoClass<CFieldDefs, &CLSID_FieldDefs>,
    public IDispatchImpl<IFieldDefs, &IID_IFieldDefs, &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{
	CComObject<CFieldDef>* m_fieldDefObj;
	short GetFieldNum(VARIANT* Index);

public:
    CFieldDefs():m_fieldDefObj(NULL){}
	const bzs::db::protocol::tdap::client::fielddefs* m_fds;

    BEGIN_COM_MAP(CFieldDefs) 
		COM_INTERFACE_ENTRY(IFieldDefs) 
		COM_INTERFACE_ENTRY(IDispatch) 
	END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() {return S_OK;}

    void FinalRelease();

public:

  STDMETHOD(IndexByName)(BSTR Name, short* retVal);
  STDMETHOD(get_FieldDef)(VARIANT Name, IFieldDef** retVal);
  STDMETHOD(get_Size)(short* retVal);
};
