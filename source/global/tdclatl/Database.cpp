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
#include "stdafx.h"
#include "Database.h"
#include "DbDef.h"
#include "Table.h"
#include "TdVersion.h"

using namespace bzs::db::protocol::tdap::client;
STDMETHODIMP CDatabase::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IDatabase
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CDatabase::Open(BSTR Uri, eSchemaType SchemaType, eOpenMode Mode,
                             BSTR Dir, BSTR Ownername, VARIANT_BOOL* Param6)
{
    *Param6 = m_db->open(Uri, (short)SchemaType, Mode, Dir, Ownername);
    return S_OK;
}

STDMETHODIMP CDatabase::get_DbDef(IDbDef** Value)
{

    if (!m_db->dbDef())
        return Error("database is not opened. ", IID_IDatabase);

    CComObject<CDbDef>* dbDefObj;
    CComObject<CDbDef>::CreateInstance(&dbDefObj);
    if (dbDefObj)
    {
        dbDefObj->m_dbDef = m_db->dbDef();
        m_IsAtatchOK = false;
        IDbDef* dbdef;
        dbDefObj->QueryInterface(IID_IDbDef, (void**)&dbdef);
        _ASSERTE(dbdef);
        *Value = dbdef;
    }
    else
        *Value = 0;

    return S_OK;
}

STDMETHODIMP CDatabase::OpenTable(VARIANT TableID, eOpenMode Mode,
                                  VARIANT_BOOL AutoCreate, BSTR OwnerName,
                                  BSTR Uri, ITable** ret)
{

    //if (!m_db->dbDef())
    //    return Error("database is not opened. ", IID_IDatabase);
    *ret = NULL;
    table* tb = NULL;
    if (TableID.vt == VT_BSTR)
        tb = m_db->openTable(TableID.bstrVal, Mode, (bool)AutoCreate, OwnerName,
                             Uri);
    else if ((TableID.vt == VT_I2) || (TableID.vt == VT_I4))
        tb = m_db->openTable(TableID.iVal, Mode, (bool)AutoCreate, OwnerName,
                             Uri);

    if (m_db->stat() != 0)
        return S_OK;
        //return Error("Invalid tableid", IID_IDatabase);

    CComObject<CTableTd>* ptb;
    CComObject<CTableTd>::CreateInstance(&ptb);

    if (ptb)
    {
        ptb->m_tb.reset(tb, releaseTable);
        tb->setOptionalData((void*)ptb);
        ITable* itb;
        ptb->QueryInterface(IID_ITable, (void**)&itb);
        _ASSERTE(itb);
        *ret = itb;
        m_IsAtatchOK = false;
    }
        

    return S_OK;
}

STDMETHODIMP CDatabase::AtatchDatabase(__int64* nativeDatabase,
                                       VARIANT_BOOL noRelease)
{
    if ((nativeDatabase) && (m_IsAtatchOK))
    {
        bzs::db::protocol::tdap::client::database* nativePtr =
            reinterpret_cast<bzs::db::protocol::tdap::client::database*>(
                nativeDatabase);
        if (nativePtr)
        {
            if (m_needRelese)
            {
                Close();
                m_db->release();
            }
            m_needRelese = (noRelease == 0);
            m_db = nativePtr;
            return S_OK;
        }
        return Error("Can not get native database pointer.", IID_IDatabase);
    }
    return Error("Current database is already used.", IID_IDatabase);
}

STDMETHODIMP CDatabase::get_RootDir(BSTR* Value)
{
    CComBSTR ret;
    ret = m_db->rootDir();
    *Value = ret.Copy();

    return S_OK;
}

STDMETHODIMP CDatabase::put_RootDir(BSTR Value)
{
    m_db->setRootDir(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_Stat(eStatus* Value)
{
    *Value = (eStatus)m_db->stat();
    return S_OK;
}

STDMETHODIMP CDatabase::AbortTrn()
{
    m_db->abortTrn();
    return S_OK;
}

STDMETHODIMP CDatabase::BeginTrn(eLockType bias)
{
    m_db->beginTrn((short)bias);

    return S_OK;
}

STDMETHODIMP CDatabase::EndTrn()
{
    m_db->endTrn();
    return S_OK;
}

STDMETHODIMP CDatabase::get_NativeDatabase(__int64** Value)
{
    *Value = (__int64*)m_db;
    return S_OK;
}

STDMETHODIMP CDatabase::BeginSnapshot(eStLockType bias)
{
    m_db->beginSnapshot(bias);
    return S_OK;
}

STDMETHODIMP CDatabase::EndSnapshot()
{
    m_db->endSnapshot();
    return S_OK;
}

STDMETHODIMP CDatabase::Drop()
{
    m_db->drop();
    return S_OK;
}

STDMETHODIMP CDatabase::DropTable(BSTR TableName)
{
    m_db->dropTable(TableName);

    return S_OK;
}

STDMETHODIMP CDatabase::Create(BSTR URI, int type)
{
    m_db->create(URI, type);
    return S_OK;
}

STDMETHODIMP CDatabase::Close(VARIANT_BOOL withDropDefaultSchema)
{
    m_db->close(withDropDefaultSchema == -1);
    return S_OK;
}

STDMETHODIMP CDatabase::Connect(BSTR URI, VARIANT_BOOL newConnection,
                                VARIANT_BOOL* Value)

{
    *Value = m_db->connect(URI, newConnection);
    return S_OK;
}

STDMETHODIMP CDatabase::Disconnect(BSTR URI, VARIANT_BOOL* Param2)
{
    *Param2 = m_db->disconnect(URI);
    return S_OK;
}

STDMETHODIMP CDatabase::DisconnectForReconnectTest(VARIANT_BOOL* Param2)
{
    *Param2 = m_db->disconnectForReconnectTest();
    return S_OK;
}

STDMETHODIMP CDatabase::Reconnect(VARIANT_BOOL* Param2)
{
    *Param2 = m_db->reconnect();
    return S_OK;
}

STDMETHODIMP CDatabase::get_EnableTrn(VARIANT_BOOL* Value)
{
    *Value = m_db->enableTrn();
    return S_OK;
}

STDMETHODIMP CDatabase::GetBtrVersion(int index, ITdVersion** Value)
{
    if ((index < 0) || (index > 2))
        return Error("Invalid index", IID_IDatabase);
    bzs::db::protocol::tdap::btrVersions vers;
    m_db->getBtrVersion(&vers);

    CComObject<CTdVersion>* ptb;
    CComObject<CTdVersion>::CreateInstance(&ptb);
    ptb->m_ver = vers.versions[index]; // not refarence. no need the this life
    // time control
    ITdVersion* tdVer;
    ptb->QueryInterface(IID_ITdVersion, (void**)&tdVer);
    _ASSERTE(tdVer);
    *Value = tdVer;

    return S_OK;
}

STDMETHODIMP CDatabase::IsTransactdUri(BSTR uri, VARIANT_BOOL* Value)
{
    *Value = m_db->isTransactdUri(uri);
    return S_OK;
}

STDMETHODIMP CDatabase::get_IsUseTransactd(VARIANT_BOOL* Value)
{
    *Value = m_db->isUseTransactd();
    return S_OK;
}

STDMETHODIMP CDatabase::SetUseTransactd()
{
    m_db->setUseTransactd();
    return S_OK;
}

STDMETHODIMP CDatabase::get_LocalSharing(VARIANT_BOOL* Value)
{
    *Value = m_db->localSharing();
    return S_OK;
}

STDMETHODIMP CDatabase::put_LocalSharing(VARIANT_BOOL Value)
{
    m_db->setLocalSharing(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_LockWaitCount(VARIANT_BOOL* Value)
{
    *Value = m_db->lockWaitCount();
    return S_OK;
}

STDMETHODIMP CDatabase::put_LockWaitCount(VARIANT_BOOL Value)
{
    m_db->setLockWaitCount(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_LockWaitTime(VARIANT_BOOL* Value)
{
    *Value = m_db->lockWaitTime();
    return S_OK;
}

STDMETHODIMP CDatabase::put_LockWaitTime(VARIANT_BOOL Value)
{
    m_db->setLockWaitTime(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_UseLongFilename(VARIANT_BOOL* Value)
{
    *Value = m_db->useLongFilename();
    return S_OK;
}

STDMETHODIMP CDatabase::put_UseLongFilename(VARIANT_BOOL Value)
{
    m_db->setUseLongFilename(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_OpenTableCount(short* Value)
{
    *Value = m_db->openTableCount();
    return S_OK;
}

STDMETHODIMP CDatabase::ReadDatabaseDirectory(BSTR* Value)
{
    wchar_t tmp[255] = { NULL };
    m_db->readDatabaseDirectory(tmp, (uchar_td)255);
    *Value = ::SysAllocString(tmp);
    return S_OK;
}

STDMETHODIMP CDatabase::Rename(BSTR oldUri, BSTR newUri)
{
    m_db->rename(oldUri, newUri);
    return S_OK;
}

STDMETHODIMP CDatabase::get_Uri(BSTR* uri)
{
    m_db->uri();
    *uri = ::SysAllocString(m_db->uri());
    return S_OK;
}

STDMETHODIMP CDatabase::TdapErr(OLE_HANDLE hWnd, BSTR* Value)
{
    if (Value)
    {
        wchar_t tmp[512] = { NULL };
        m_db->tdapErr((HWND)hWnd, tmp);
        *Value = ::SysAllocString(tmp);
    }
    else
        m_db->tdapErr((HWND)hWnd);
    return S_OK;
}

STDMETHODIMP CDatabase::StatMsg(BSTR* Value)
{
    wchar_t tmp[1024] = { NULL };
    m_db->statMsg(tmp);
    *Value = ::SysAllocString(tmp);
    return S_OK;
}

STDMETHODIMP CDatabase::Clone(IDatabase** Value)
{
    CComObject<CDatabase>* ptb;
    CComObject<CDatabase>::CreateInstance(&ptb);
    HRESULT ret = ptb->AtatchDatabase((__int64*)m_db->clone());
    if (ret == S_OK)
    {
        IDatabase* dbPtr;
        ptb->QueryInterface(IID_IDatabase, (void**)&dbPtr);
        _ASSERTE(dbPtr);
        *Value = dbPtr;
        return S_OK;
    }
    return ret;
}

STDMETHODIMP CDatabase::AssignSchemaData(IDbDef* Src, short* Value)
{
    CDbDef* dbdef = NULL;
    dbdef = dynamic_cast<CDbDef*>(Src);
    _ASSERTE(dbdef);
    m_db->assignSchemaData(dbdef->m_dbDef);
    return S_OK;
}

STDMETHODIMP CDatabase::Continuous(eContinusOpr Op, VARIANT_BOOL inclideRepfile,
                                   eContinusStatus* Value)
{
    *Value = (eContinusStatus)m_db->continuous((char_td)Op, inclideRepfile);
    return S_OK;
}

STDMETHODIMP CDatabase::ConvertTable(short TableIndex, VARIANT_BOOL Turbo,
                                     BSTR OwnerName)
{
    m_db->convertTable(TableIndex, Turbo, OwnerName);
    return S_OK;
}

STDMETHODIMP CDatabase::CopyTableData(ITable* Dest, ITable* Src,
                                      VARIANT_BOOL Turbo, short KeyNum, int MaxSkip, short* Value)
{
    CTableTd* dest = dynamic_cast<CTableTd*>(Dest);
    CTableTd* src = dynamic_cast<CTableTd*>(Src);

    _ASSERTE(dest);
    _ASSERTE(dest);

    *Value = m_db->copyTableData(dest->m_tb.get(), src->m_tb.get(), Turbo,
                                 KeyNum, MaxSkip);
    return S_OK;
}

STDMETHODIMP CDatabase::CreateTable(VARIANT FileNumOrSql, BSTR Uri,
                                    VARIANT_BOOL* Value)
{
    if (FileNumOrSql.vt == VT_BSTR && Uri[0]==0x00)
    {
        
        int size = WideCharToMultiByte(CP_UTF8, 0, FileNumOrSql.bstrVal, -1, NULL, 0, NULL, NULL);
        char* p = new char[size + 1];
        WideCharToMultiByte(CP_UTF8, 0, FileNumOrSql.bstrVal, -1, p, size + 1, NULL, NULL);
        *Value = m_db->createTable(p);
        delete [] p;
    }
    else
        *Value = m_db->createTable(FileNumOrSql.iVal, Uri);
    return S_OK;
}

STDMETHODIMP CDatabase::ExistsTableFile(short TableIndex, BSTR OwnerName)
{
    m_db->existsTableFile(TableIndex, OwnerName);
    return S_OK;
}

STDMETHODIMP CDatabase::get_IsOpened(VARIANT_BOOL* Value)
{
    *Value = m_db->isOpened();
    return S_OK;
}

STDMETHODIMP CDatabase::get_TableReadOnly(VARIANT_BOOL* Value)
{
    *Value = m_db->tableReadOnly();
    return S_OK;
}

STDMETHODIMP CDatabase::put_TableReadOnly(VARIANT_BOOL Value)
{
    m_db->setTableReadOnly(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::SwapTablename(BSTR Uri1, BSTR Uri2)
{
    m_db->swapTablename(Uri1, Uri2);
    return S_OK;
}

STDMETHODIMP CDatabase::get_ClientID(short* Value)
{
    *Value = (short)m_db->clientID();
    return S_OK;
}

STDMETHODIMP CDatabase::get_RefCount(int* Value)
{
    *Value = m_db->refCount();
    return S_OK;
}

STDMETHODIMP CDatabase::AclReload(short* Value)
{
    *Value = m_db->aclReload();
    return S_OK;
}

STDMETHODIMP CDatabase::get_TrnsactionFlushWaitStatus(VARIANT_BOOL* Value)
{
    *Value = database::trnsactionFlushWaitStatus();
    return S_OK;
}

STDMETHODIMP CDatabase::put_ExecCodePage(unsigned int Value)
{
    database::setExecCodePage(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_ExecCodePage(unsigned int* Value)
{
    *Value = database::execCodePage();
    return S_OK;
}

STDMETHODIMP CDatabase::get_MaxTables(int* Value)
{
    *Value = database::maxtables;
    return S_OK;
}

STDMETHODIMP CDatabase::get_TrxIsolationServer(eSrvIsorationType* Value)
{
    *Value = (eSrvIsorationType)m_db->trxIsolationServer();
    return S_OK;
}

STDMETHODIMP CDatabase::get_TrxLockWaitTimeoutServer(int* Value)
{
    *Value = m_db->trxLockWaitTimeoutServer();
    return S_OK;
}

STDMETHODIMP CDatabase::put_AutoSchemaUseNullkey(VARIANT_BOOL Value)
{
    m_db->setAutoSchemaUseNullkey(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_AutoSchemaUseNullkey(VARIANT_BOOL* Value)
{
    *Value = m_db->autoSchemaUseNullkey();
    return S_OK;
}

STDMETHODIMP CDatabase::put_CompatibleMode(int Value)
{
    database::setCompatibleMode(Value);
    return S_OK;
}

STDMETHODIMP CDatabase::get_CompatibleMode(int* Value)
{
    *Value = database::compatibleMode();
    return S_OK;
}

STDMETHODIMP CDatabase::GetSqlStringForCreateTable(BSTR tableName, BSTR* retVal)
{
    uint_td size = 65000;
    char* tmp = new char[size];
    CComBSTR ret;

    ret = m_db->getSqlStringForCreateTable(tableName, tmp, &size);
    *retVal = ret.Copy();
    delete [] tmp;
    return S_OK;
}

void __stdcall onCopyData(database* db, int recordCount, int count,
                          bool& cancel)
{
    CDatabase* cdb = reinterpret_cast<CDatabase*>(db->optionalData());
    IDatabase* dbPtr = dynamic_cast<IDatabase*>(cdb);
    _ASSERTE(dbPtr);
    VARIANT_BOOL tmp = 0;
    cdb->Fire_OnCopyData(dbPtr, recordCount, count, &tmp);
    if (tmp)
        cancel = true;
}

bool __stdcall onDeleteRecord(database* db, table* tb, bool inkey)
{
    CDatabase* cdb = reinterpret_cast<CDatabase*>(db->optionalData());
    IDatabase* dbPtr = dynamic_cast<IDatabase*>(cdb);
    _ASSERTE(dbPtr);

    CTableTd* ctb = reinterpret_cast<CTableTd*>(tb->optionalData());
    ITable* tbPtr = dynamic_cast<ITable*>(ctb);
    _ASSERTE(tbPtr);

    VARIANT_BOOL tmp = 0;
    cdb->Fire_OnDeleteRecord(dbPtr, tbPtr, &tmp);
    return (tmp != 0);
}


