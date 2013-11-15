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
#include <bzs/db/protocol/tdap/client/dbDef.h>


using namespace ATL;



class ATL_NO_VTABLE CDbDef : public CComObjectRootEx<CComSingleThreadModel>, public CComCoClass<CDbDef, &CLSID_DbDef>,
    public IDispatchImpl<IDbDef, &IID_IDbDef, &LIBID_transactd, /* wMajor = */ 1, /* wMinor = */ 0>
{
    

public:
    CDbDef():m_dbDef(NULL) {}

 	bzs::db::protocol::tdap::client::dbdef* m_dbDef;
    
    BEGIN_COM_MAP(CDbDef) 
		COM_INTERFACE_ENTRY(IDbDef) 
		COM_INTERFACE_ENTRY(IDispatch) 
	END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() {return S_OK;}

	void FinalRelease(){}

public:

    STDMETHOD(get_TableCount)(short* Value);
    STDMETHOD(TableDef)(short Index, ITableDef** Value);

    STDMETHOD(InsertTable)(short index, ITableDef** Param1);
    STDMETHOD(DeleteField)(short TableIndex, short FieldIndex);
    STDMETHOD(DeleteKey)(short TableIndex, short KeyIndex);
    STDMETHOD(InsertField)(short TableIndex, short InsertIndex, IFieldDef** Param3);

    STDMETHOD(InsertKey)(short TableIndex, short InsertIndex, IKeyDef** Param3);
    STDMETHOD(UpDateTableDef)(short TableIndex);
	STDMETHOD(CompAsBackup)(short TableIndex, VARIANT_BOOL* Value);
	STDMETHOD(DeleteTable)(short TableIndex);
	STDMETHOD(TableNumByName)(BSTR Name, short* Value);
	STDMETHOD(FieldNumByName)( short TableIndex, BSTR Name, short* Value);
	STDMETHOD(FieldValidLength)( eFieldQuery Query, short FieldType, unsigned int* Value);
	STDMETHOD(FindKeynumByFieldNum)( short TableIndex, short Index, unsigned short* Value);
	STDMETHOD(GetRecordLen)( short TableIndex, unsigned short* Value);
	STDMETHOD(get_OpenMode)( eOpenMode* Value);
	STDMETHOD(PopBackup)( short TableIndex);
	STDMETHOD(PushBackup)( short TableIndex);
	STDMETHOD(RenumberTable)( short OldIndex, short NewIndex);
	STDMETHOD(Reopen)( eOpenMode Mode);
	STDMETHOD(get_Version)( int* Value);
	STDMETHOD(put_Version)( int Value);
	STDMETHOD(get_Stat)( eStatus* Value);
	STDMETHOD(TdapErr)(OLE_HANDLE hWnd, BSTR* Value);

};
