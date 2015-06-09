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
#include "DbDef.h"
#include "database.h"
#include "TableDef.h"
#include "FieldDef.h"
#include "KeyDef.h"
#include "Field.h"

using namespace bzs::db::protocol::tdap;

STDMETHODIMP CDbDef::get_TableCount(short* Value)
{
    *Value = m_dbDef->tableCount();
    return S_OK;
}

STDMETHODIMP CDbDef::TableDef(short Index, ITableDef** Value)
{
    if (!m_dbDef->tableDefs(Index))
        return Error("Invalid index.", IID_IDbDef);

    CComObject<CTableDef>* piObj = NULL;
    CComObject<CTableDef>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_dbDef->tableDefPtr(Index);
        ITableDef* tbd;
        piObj->QueryInterface(IID_ITableDef, (void**)&tbd);
        _ASSERTE(tbd);
        *Value = tbd;
    }
    else
        *Value = 0;

    return S_OK;
}

STDMETHODIMP CDbDef::InsertTable(short index, ITableDef** Param1)
{
    tabledef TableDef;
    memset(&TableDef, 0, sizeof(TableDef));
    TableDef.setTableName(L"new_table");
    TableDef.setFileName(L"new_table.dat");
    TableDef.id = index;
    TableDef.keyCount = 0;
    TableDef.fieldCount = 0;
    TableDef.flags.all = 0;
    TableDef.primaryKeyNum = -1;
    TableDef.parentKeyNum = -1;
    TableDef.replicaKeyNum = -1;
    TableDef.pageSize = 2048;

    m_dbDef->insertTable(&TableDef);
    this->TableDef(index, Param1);
    return S_OK;
}

STDMETHODIMP CDbDef::DeleteField(short TableIndex, short FieldIndex)
{
    m_dbDef->deleteField(TableIndex, FieldIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::DeleteKey(short TableIndex, short KeyIndex)
{
    m_dbDef->deleteKey(TableIndex, KeyIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::InsertField(short TableIndex, short InsertIndex,
                                 IFieldDef** Param3)

{
    fielddef* fdPtr = m_dbDef->insertField(TableIndex, InsertIndex);
    if (!fdPtr)
        return Error("Invalid index.", IID_IDbDef);

    CComObject<CFieldDef>* piObj;
    CComObject<CFieldDef>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_dbDef->tableDefPtr(TableIndex);
        piObj->m_index = InsertIndex;
        IFieldDef* fd;
        piObj->QueryInterface(IID_IFieldDef, (void**)&fd);
        _ASSERTE(fd);
        *Param3 = piObj;
    }
    else
        *Param3 = 0;

    return S_OK;
}

STDMETHODIMP CDbDef::InsertKey(short TableIndex, short InsertIndex,
                               IKeyDef** Param3)

{
    keydef* keyPtr = m_dbDef->insertKey(TableIndex, InsertIndex);
    if (!keyPtr)
        return Error("Invalid index.", IID_IDbDef);
    CComObject<CKeyDef>* piObj;
    CComObject<CKeyDef>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_dbDef->tableDefPtr(TableIndex);
        ;
        piObj->m_index = InsertIndex;
        IKeyDef* kb;
        piObj->QueryInterface(IID_IKeyDef, (void**)&kb);
        _ASSERTE(kb);
        *Param3 = piObj;
    }
    else
        *Param3 = 0;
    return S_OK;
}

STDMETHODIMP CDbDef::UpDateTableDef(short TableIndex)
{
    m_dbDef->updateTableDef(TableIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::CompAsBackup(short TableIndex, VARIANT_BOOL* Value)
{
    *Value = m_dbDef->compAsBackup(TableIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::DeleteTable(short TableIndex)
{
    m_dbDef->deleteTable(TableIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::TableNumByName(BSTR Name, short* Value)
{
    *Value = m_dbDef->tableNumByName(Name);
    return S_OK;
}

STDMETHODIMP CDbDef::FieldNumByName(short TableIndex, BSTR Name, short* Value)
{
    *Value = m_dbDef->fieldNumByName(TableIndex, Name);
    return S_OK;
}

STDMETHODIMP CDbDef::FieldValidLength(eFieldQuery Query, short FieldType,
                                      unsigned int* Value)
{
    *Value = m_dbDef->fieldValidLength((client::eFieldQuery)Query,
                                       (uchar_td)FieldType);
    return S_OK;
}

STDMETHODIMP CDbDef::FindKeynumByFieldNum(short TableIndex, short Index,
                                          unsigned short* Value)
{
    *Value = m_dbDef->findKeynumByFieldNum(TableIndex, Index);
    return S_OK;
}

STDMETHODIMP CDbDef::GetRecordLen(short TableIndex, unsigned short* Value)
{
    *Value = m_dbDef->getRecordLen(TableIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::get_OpenMode(eOpenMode* Value)
{
    *Value = (eOpenMode)m_dbDef->openMode();
    return S_OK;
}

STDMETHODIMP CDbDef::PopBackup(short TableIndex)
{
    m_dbDef->popBackup(TableIndex);
    return S_OK;
}
STDMETHODIMP CDbDef::PushBackup(short TableIndex)
{
    m_dbDef->pushBackup(TableIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::RenumberTable(short OldIndex, short NewIndex)
{
    m_dbDef->renumberTable(OldIndex, NewIndex);
    return S_OK;
}

STDMETHODIMP CDbDef::Reopen(eOpenMode Mode)
{
    m_dbDef->reopen((char_td)Mode);
    return S_OK;
}

STDMETHODIMP CDbDef::get_Version(int* Value)
{
    *Value = m_dbDef->version();
    return S_OK;
}

STDMETHODIMP CDbDef::put_Version(int Value)
{
    m_dbDef->setVersion(Value);
    return S_OK;
}

STDMETHODIMP CDbDef::get_Stat(eStatus* Value)
{
    *Value = (eStatus)m_dbDef->stat();
    return S_OK;
}

STDMETHODIMP CDbDef::TdapErr(OLE_HANDLE hWnd, BSTR* Value)
{
    if (Value)
    {
        wchar_t tmp[512] = { NULL };
        m_dbDef->tdapErr((HWND)hWnd, tmp);
        *Value = ::SysAllocString(tmp);
    }
    else
        m_dbDef->tdapErr((HWND)hWnd);
    return S_OK;
}

STDMETHODIMP CDbDef::ValidateTableDef(short TableIndex, short* Value)
{
    *Value = m_dbDef->validateTableDef(TableIndex);
    return S_OK;
}
