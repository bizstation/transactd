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
#include "TableDef.h"
#include "FieldDef.h"
#include "KeyDef.h"
#include "Flags.h"

STDMETHODIMP CTableDef::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_ITableDef
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CTableDef::get_FieldDef(short Index, IFieldDef** Value)
{
    if (Index >= (*m_tabledefPtr)->fieldCount)
        return Error("Invalid index.", IID_ITableDef);

    CComObject<CFieldDef>* piObj;
    CComObject<CFieldDef>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_tabledefPtr;
        piObj->m_index = Index;
        IFieldDef* fd;
        piObj->QueryInterface(IID_IFieldDef, (void**)&fd);
        _ASSERTE(fd);
        *Value = piObj;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CTableDef::get_KeyDef(short Index, IKeyDef** Value)
{
    if (Index >= (*m_tabledefPtr)->keyCount)
        return Error("Invalid index.", IID_ITableDef);

    CComObject<CKeyDef>* piObj;
    CComObject<CKeyDef>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_tabledefPtr;
        piObj->m_index = Index;
        IKeyDef* kb;
        piObj->QueryInterface(IID_IKeyDef, (void**)&kb);
        _ASSERTE(kb);
        *Value = piObj;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CTableDef::get_FieldCount(short* Value)
{
    *Value = (*m_tabledefPtr)->fieldCount;
    return S_OK;
}

STDMETHODIMP CTableDef::get_KeyCount(short* Value)
{
    *Value = (*m_tabledefPtr)->keyCount;
    return S_OK;
}

STDMETHODIMP CTableDef::get_PageSize(short* Value)
{
    *Value = (*m_tabledefPtr)->pageSize;
    return S_OK;
}

STDMETHODIMP CTableDef::put_PageSize(short Value)
{
    (*m_tabledefPtr)->pageSize = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_PreAlloc(short* Value)
{
    *Value = (*m_tabledefPtr)->preAlloc;
    return S_OK;
}

STDMETHODIMP CTableDef::put_PreAlloc(short Value)
{
    (*m_tabledefPtr)->preAlloc = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_FileName(BSTR* Value)
{
    CComBSTR ret;
    ret = (*m_tabledefPtr)->fileName();
    *Value = ret.Copy();

    return S_OK;
}

STDMETHODIMP CTableDef::put_FileName(BSTR Value)
{

    (*m_tabledefPtr)->setFileName(Value);
    return S_OK;
}

STDMETHODIMP CTableDef::get_TableName(BSTR* Value)
{
    CComBSTR ret;
    ret = (*m_tabledefPtr)->tableName();
    *Value = ret.Copy();
    return S_OK;
}

STDMETHODIMP CTableDef::put_TableName(BSTR Value)
{
    (*m_tabledefPtr)->setTableName(Value);
    return S_OK;
}

STDMETHODIMP CTableDef::get_Flags(IFlags** Value)
{
    CComObject<CFlags>* piObj;
    CComObject<CFlags>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_flags = &((*m_tabledefPtr)->flags);
        IFlags* fl;
        piObj->QueryInterface(IID_IFlags, (void**)&fl);
        _ASSERTE(fl);
        *Value = piObj;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CTableDef::put_Flags(IFlags* Value)
{
    Value->All(&(*m_tabledefPtr)->flags.all);
    return S_OK;
}

STDMETHODIMP CTableDef::get_PrimaryKeyNum(unsigned char* Value)
{
    *Value = (*m_tabledefPtr)->primaryKeyNum;
    return S_OK;
}

STDMETHODIMP CTableDef::put_PrimaryKeyNum(unsigned char Value)
{
    (*m_tabledefPtr)->primaryKeyNum = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_ParentKeyNum(unsigned char* Value)
{
    *Value = (*m_tabledefPtr)->parentKeyNum;
    return S_OK;
}

STDMETHODIMP CTableDef::put_ParentKeyNum(unsigned char Value)
{
    (*m_tabledefPtr)->parentKeyNum = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_ReplicaKeyNum(unsigned char* Value)
{
    *Value = (*m_tabledefPtr)->replicaKeyNum;
    return S_OK;
}

STDMETHODIMP CTableDef::put_ReplicaKeyNum(unsigned char Value)
{
    (*m_tabledefPtr)->replicaKeyNum = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_OptionFlags(IFlags** Value)
{
    CComObject<CFlags>* piObj;
    CComObject<CFlags>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_flags = &((*m_tabledefPtr)->optionFlags);
        IFlags* fl;
        piObj->QueryInterface(IID_IFlags, (void**)&fl);
        _ASSERTE(fl);
        *Value = piObj;
    }
    else
        *Value = 0;

    return S_OK;
}

STDMETHODIMP CTableDef::put_OptionFlags(IFlags* Value)
{
    Value->All(&(*m_tabledefPtr)->optionFlags.all);
    return S_OK;
}

STDMETHODIMP CTableDef::get_IconIndex(unsigned char* Value)
{
    *Value = (*m_tabledefPtr)->iconIndex;
    return S_OK;
}

STDMETHODIMP CTableDef::put_IconIndex(unsigned char Value)
{
    (*m_tabledefPtr)->iconIndex = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_Id(short* Value)
{
    *Value = (*m_tabledefPtr)->id;
    return S_OK;
}

STDMETHODIMP CTableDef::put_Id(short Value)
{
    (*m_tabledefPtr)->id = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_Charsetindex(eCharset* Value)
{
    *Value = (eCharset)(*m_tabledefPtr)->charsetIndex;
    return S_OK;
}

STDMETHODIMP CTableDef::put_Charsetindex(eCharset Value)
{
    (*m_tabledefPtr)->charsetIndex = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_FixedRecordLen(unsigned short* Value)
{
    *Value = (*m_tabledefPtr)->fixedRecordLen;
    return S_OK;
}

STDMETHODIMP CTableDef::put_FixedRecordLen(unsigned short Value)
{
    (*m_tabledefPtr)->fixedRecordLen = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_RecordLen(unsigned short* Value)
{
    *Value = (*m_tabledefPtr)->recordlen();
    return S_OK;
}

STDMETHODIMP CTableDef::get_SchemaCodePage(unsigned int* Value)
{
    *Value = (*m_tabledefPtr)->schemaCodePage;
    return S_OK;
}

STDMETHODIMP CTableDef::put_SchemaCodePage(unsigned int Value)
{
    (*m_tabledefPtr)->schemaCodePage = Value;
    return S_OK;
}

STDMETHODIMP CTableDef::get_Version(unsigned short* Value)
{
    *Value = (*m_tabledefPtr)->version;
    return S_OK;
}

STDMETHODIMP CTableDef::get_FieldNumByName(BSTR Name, short* Value)
{
    *Value = (*m_tabledefPtr)->fieldNumByName(Name);
    return S_OK;
}

STDMETHODIMP CTableDef::get_Nullfields(short* Value)
{
    *Value = (*m_tabledefPtr)->nullfields();
    return S_OK;
}

STDMETHODIMP CTableDef::get_InUse(short* Value)
{
    *Value = (*m_tabledefPtr)->inUse();
    return S_OK;
}

STDMETHODIMP CTableDef::get_MysqlNullMode(VARIANT_BOOL* Value)
{
    *Value = (*m_tabledefPtr)->isMysqlNullMode();
    return S_OK;
}

STDMETHODIMP CTableDef::get_Size(int* Value)
{
    *Value = (*m_tabledefPtr)->size();
    return S_OK;
}



