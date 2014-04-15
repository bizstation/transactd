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
#include "FieldDef.h"
#include "Flags.h"

STDMETHODIMP CFieldDef::get_Name(BSTR* Value)
{
    CComBSTR ret;
    ret = const_fielddef()->name();
    *Value = ret.Copy();
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Name(BSTR Value)
{
    if (!isWritabale())return write_error();
		
	fielddef()->setName(Value);
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Type(eFieldType* Value)
{
    *Value = (eFieldType)const_fielddef()->type;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Type(eFieldType Value)
{
    if (!isWritabale())return write_error();

    fielddef()->type = (uchar_td)Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Len(short* Value)
{
    *Value = const_fielddef()->len;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Len(short Value)
{
    if (!isWritabale())return write_error();
    fielddef()->len = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Decimals(unsigned char* Value)
{
    *Value = const_fielddef()->decimals;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Decimals(unsigned char Value)
{
    if (!isWritabale())return write_error();
    fielddef()->decimals = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Max(double* Value)
{
    *Value = const_fielddef()->max;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Max(double Value)
{
    if (!isWritabale())return write_error();
    fielddef()->max = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Min(double* Value)
{
    *Value = const_fielddef()->min;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Min(double Value)
{
    if (!isWritabale())return write_error();
    fielddef()->min = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_DefValue(double* Value)
{
    *Value = const_fielddef()->defValue;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_DefValue(double Value)
{
    if (!isWritabale())return write_error();
    fielddef()->defValue = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_LookTable(unsigned char* Value)
{
    *Value = const_fielddef()->lookTable;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_LookTable(unsigned char Value)
{
    if (!isWritabale())return write_error();
    fielddef()->lookTable = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_LookField(unsigned char* Value)
{
    *Value = const_fielddef()->lookField;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_LookField(unsigned char Value)
{
    if (!isWritabale())return write_error();
    fielddef()->lookField = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_LookViewField(short Index, unsigned char* Value)
{
    *Value = const_fielddef()->lookFields[Index];
    return S_OK;
}

STDMETHODIMP CFieldDef::put_LookViewField(short Index, unsigned char Value)
{
    if (!isWritabale())return write_error();
    fielddef()->lookFields[Index] = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_EnableFlags(IFlags** Value)
{
    CComObject<CFlags> *piObj;
    CComObject<CFlags>::CreateInstance(&piObj);
	if (piObj)
	{
		piObj->m_flags = const_fielddef()->enableFlags;
		IFlags* fl;
		piObj->QueryInterface(IID_IFlags, (void**)&fl);
		_ASSERTE(fl);
		*Value = piObj;
	}else
		*Value = 0;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_EnableFlags(IFlags* Value)
{
    if (!isWritabale())return write_error();
    Value->All(&fielddef()->enableFlags.all);
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Keylen(unsigned short* Value)
{
	*Value = const_fielddef()->keylen;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_Keylen(unsigned short Value)
{
    if (!isWritabale())return write_error();
	fielddef()->keylen = Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_NullValue(unsigned short* Value)
{
	*Value = const_fielddef()->nullValue;
    return S_OK;
}

STDMETHODIMP CFieldDef::put_NullValue(unsigned short Value)
{
    if (!isWritabale())return write_error();
	fielddef()->nullValue = (uchar_td)Value;
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Align(unsigned int* Value)
{
	*Value = const_fielddef()->align();
    return S_OK;
}

STDMETHODIMP CFieldDef::get_TypeName( BSTR* Value)
{
	*Value = ::SysAllocString(const_fielddef()->typeName());
    return S_OK;
}

STDMETHODIMP CFieldDef::get_IsStringType( VARIANT_BOOL* Value)
{
	*Value = const_fielddef()->isStringType();
    return S_OK;
}

STDMETHODIMP CFieldDef::get_CharsetIndex(eCharset* Value)
{
	*Value = (eCharset)const_fielddef()->charsetIndex();
    return S_OK;
}

STDMETHODIMP CFieldDef::put_CharsetIndex(eCharset Value)
{
    if (!isWritabale())return write_error();
	fielddef()->setCharsetIndex((unsigned char)Value);
    return S_OK;
}

STDMETHODIMP CFieldDef::get_CodePage(unsigned int* Value)
{
	*Value = const_fielddef()->codePage();
    return S_OK;
}

STDMETHODIMP CFieldDef::get_CharNum(unsigned int* Value)
{
	*Value = const_fielddef()->charNum();
    return S_OK;
}

STDMETHODIMP CFieldDef::SetLenByCharnum(unsigned short Value)
{
    if (!isWritabale())return write_error();
	fielddef()->setLenByCharnum(Value);
    return S_OK;
}

STDMETHODIMP CFieldDef::get_Index(short* Value)
{
	*Value = m_index;
    return S_OK;

}

