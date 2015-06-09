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
#include <bzs/db/protocol/tdap/client/activeTable.h>

using namespace ATL;

class CWritableRecord;
class CARecordset;
class ATL_NO_VTABLE CActiveTable
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CActiveTable, &CLSID_ActiveTable>,
      public IDispatchImpl<IActiveTable, &IID_IActiveTable, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>
{
    void setResult(IActiveTable** retVal);

    bzs::db::protocol::tdap::client::activeTable* m_at;
    CComObject<CWritableRecord>* m_recObj;

public:
    CActiveTable() : m_at(NULL), m_recObj(NULL) {}

    DECLARE_REGISTRY_RESOURCEID(IDR_ACTIVETABLE)

    BEGIN_COM_MAP(CActiveTable)
    COM_INTERFACE_ENTRY(IActiveTable)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease();

public:
    STDMETHOD(SetDatabase)(VARIANT Value, BSTR tableName, short mode);

    STDMETHOD(Index)(short Value, IActiveTable** retVal);
    STDMETHOD(KeyValue)(VARIANT Value0, VARIANT Value1, VARIANT Value2,
                        VARIANT Value3, VARIANT Value4, VARIANT Value5,
                        VARIANT Value6, VARIANT Value7, IActiveTable** retVal);

    STDMETHOD(Option)(int Value, IActiveTable** retVal);
    STDMETHOD(Read)(VARIANT /*IQueryBase**/ query, 
                                    VARIANT Value0, VARIANT Value1,
                                    VARIANT Value2, VARIANT Value3,
                                    VARIANT Value4, VARIANT Value5,
                                    VARIANT Value6, VARIANT Value7,
                                    IRecordset** retVal);
    STDMETHOD(Alias)(BSTR Src, BSTR Dst, IActiveTable** retVal);
    STDMETHOD(ResetAlias)(IActiveTable** retVal);
    STDMETHOD(Join)(IRecordset* rs, VARIANT query, BSTR Name0, BSTR Name1,
                    BSTR Name2, BSTR Name3, BSTR Name4, BSTR Name5, BSTR Name6,
                    BSTR Name7, IRecordset** retVal);
    STDMETHOD(OuterJoin)(IRecordset* rs, VARIANT query, BSTR Name0,
                         BSTR Name1, BSTR Name2, BSTR Name3, BSTR Name4,
                         BSTR Name5, BSTR Name6, BSTR Name7,
                         IRecordset** retVal);
    STDMETHOD(Prepare)(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal);

    STDMETHOD(GetWritableRecord)(IWritableRecord** retVal);
    STDMETHOD(get_TableDef)(ITableDef** Value);
    STDMETHOD(Table)(ITable** retVal);
    STDMETHOD(ReadMore)(IRecordset** retVal);
};

OBJECT_ENTRY_AUTO(__uuidof(ActiveTable), CActiveTable)
