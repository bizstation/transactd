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
#include <bzs/db/protocol/tdap/client/database.h>
#include "_IDatabaseEvents_CP.H"

using namespace ATL;

class CDbDef;

void __stdcall onCopyData(bzs::db::protocol::tdap::client::database* db,
                          int recordCount, int count, bool& cancel);
bool __stdcall onDeleteRecord(bzs::db::protocol::tdap::client::database* db,
                              bzs::db::protocol::tdap::client::table* tb,
                              bool inkey);

class ATL_NO_VTABLE CDatabase
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CDatabase, &CLSID_Database>,
	  public ISupportErrorInfo,
      public IDispatchImpl<IDatabase, &IID_IDatabase, &LIBID_transactd,
                           /* wMajor = */ 1, /* wMinor = */ 0>,
      public IConnectionPointContainerImpl<CDatabase>,
      public CProxy_IDatabaseEvents<CDatabase>

{
    bzs::db::protocol::tdap::client::database* m_db;
    bool m_IsAtatchOK;
    bool m_needRelese;

public:
    CDatabase() : m_needRelese(true), m_IsAtatchOK(true)
    {
        bzs::db::protocol::tdap::client::nsdatabase::setCheckTablePtr(true);
        m_db = bzs::db::protocol::tdap::client::database::create();
        m_db->setOptionalData(this);
        m_db->setOnCopyData(onCopyData);
        m_db->setOnDeleteRecord(onDeleteRecord);
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DATABASE)

    BEGIN_COM_MAP(CDatabase)
    COM_INTERFACE_ENTRY(IDatabase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    BEGIN_CONNECTION_POINT_MAP(CDatabase)
    CONNECTION_POINT_ENTRY(__uuidof(_IDatabaseEvents))
    END_CONNECTION_POINT_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }

    void FinalRelease()
    {
        if (m_needRelese && m_db)
            m_db->release();
    };

public:
    bzs::db::protocol::tdap::client::database* database() { return m_db; };

    STDMETHOD(Open)(BSTR Uri, eSchemaType SchemaType, eOpenMode Mode, BSTR Dir,
                    BSTR Ownername, VARIANT_BOOL* Param6);
    STDMETHOD(get_DbDef)(IDbDef** Value);
    STDMETHOD(OpenTable)(VARIANT TableID, eOpenMode Mode,
                         VARIANT_BOOL AutoCreate, BSTR OwnerName, BSTR Uri,
                         ITable** ret);
    STDMETHOD(AtatchDatabase)(__int64* nativeDatabase,
                              VARIANT_BOOL noRelease = 0);
    STDMETHOD(get_RootDir)(BSTR* Value);
    STDMETHOD(put_RootDir)(BSTR Value);
    STDMETHOD(get_Stat)(eStatus* Value);
    STDMETHOD(AbortTrn)();
    STDMETHOD(BeginTrn)(eLockType bias);
    STDMETHOD(EndTrn)();
    STDMETHOD(BeginSnapshot)(eStLockType bias);
    STDMETHOD(EndSnapshot)();
    STDMETHOD(get_NativeDatabase)(__int64** Value);

    STDMETHOD(Drop)();
    STDMETHOD(DropTable)(BSTR TableName);
    STDMETHOD(Create)(BSTR URI, int type);
    STDMETHOD(Close)(VARIANT_BOOL withDropDefaultSchema = 0);
    STDMETHOD(Connect)(BSTR URI, VARIANT_BOOL newConnection,
                       VARIANT_BOOL* Value);
    STDMETHOD(Disconnect)(BSTR URI, VARIANT_BOOL* Param2);
    STDMETHOD(DisconnectForReconnectTest)(VARIANT_BOOL* Param2);
    STDMETHOD(Reconnect)(VARIANT_BOOL* Param2);

    STDMETHOD(get_EnableTrn)(VARIANT_BOOL* Value);
    STDMETHOD(GetBtrVersion)(int index, ITdVersion** ver);

    STDMETHOD(IsTransactdUri)(BSTR uri, VARIANT_BOOL* Value);
    STDMETHOD(get_IsUseTransactd)(VARIANT_BOOL* Value);
    STDMETHOD(SetUseTransactd)(void);
    STDMETHOD(get_LocalSharing)(VARIANT_BOOL* Value);
    STDMETHOD(put_LocalSharing)(VARIANT_BOOL Value);
    STDMETHOD(get_LockWaitCount)(VARIANT_BOOL* Value);
    STDMETHOD(put_LockWaitCount)(VARIANT_BOOL Value);
    STDMETHOD(get_LockWaitTime)(VARIANT_BOOL* Value);
    STDMETHOD(put_LockWaitTime)(VARIANT_BOOL Value);
    STDMETHOD(get_UseLongFilename)(VARIANT_BOOL* Value);
    STDMETHOD(put_UseLongFilename)(VARIANT_BOOL Value);
    STDMETHOD(get_OpenTableCount)(short* Value);
    STDMETHOD(ReadDatabaseDirectory)(BSTR* Value);
    STDMETHOD(Rename)(BSTR oldUri, BSTR newUri);
    STDMETHOD(get_Uri)(BSTR* uri);
    STDMETHOD(TdapErr)(OLE_HANDLE hWnd, BSTR* Value);
    STDMETHOD(StatMsg)(BSTR* Value);
    STDMETHOD(Clone)(IDatabase** Value);
    STDMETHOD(AssignSchemaData)(IDbDef* Src, short* Value);
    STDMETHOD(Continuous)(eContinusOpr Op, VARIANT_BOOL inclideRepfile,
                          eContinusStatus* Value);
    STDMETHOD(ConvertTable)(short TableIndex, VARIANT_BOOL Turbo,
                            BSTR OwnerName);
    STDMETHOD(CopyTableData)(ITable* Dest, ITable* Src, VARIANT_BOOL Turbo,
                             short KeyNum, int MaxSkip,
                             short* Value);
    STDMETHOD(CreateTable)(short FileNum, BSTR Uri, VARIANT_BOOL* Value);
    STDMETHOD(ExistsTableFile)(short TableIndex, BSTR OwnerName);
    STDMETHOD(get_IsOpened)(VARIANT_BOOL* Value);
    STDMETHOD(get_TableReadOnly)(VARIANT_BOOL* Value);
    STDMETHOD(put_TableReadOnly)(VARIANT_BOOL Value);
    STDMETHOD(SwapTablename)(BSTR Uri1, BSTR Uri2);
    STDMETHOD(get_ClientID)(short* Value);
    STDMETHOD(get_RefCount)(int* Value);
    STDMETHOD(AclReload)(short* Value);
    STDMETHOD(get_TrnsactionFlushWaitStatus)(VARIANT_BOOL* Value);
    STDMETHOD(put_ExecCodePage)(unsigned int Value);
    STDMETHOD(get_ExecCodePage)(unsigned int* Value);
    STDMETHOD(get_MaxTables)(int* Value);
    STDMETHOD(get_TrxIsolationServer)(eSrvIsorationType* Value);
    STDMETHOD(get_TrxLockWaitTimeoutServer)(int* Value);
    STDMETHOD(put_AutoSchemaUseNullkey)(VARIANT_BOOL Value);
    STDMETHOD(get_AutoSchemaUseNullkey)(VARIANT_BOOL* Value);
    STDMETHOD(put_CompatibleMode)(int Value);
    STDMETHOD(get_CompatibleMode)(int* Value);
};

OBJECT_ENTRY_AUTO(__uuidof(Database), CDatabase)
