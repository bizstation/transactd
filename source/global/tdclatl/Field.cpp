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
#include "Field.h"
#include "Bitset.h"

STDMETHODIMP CField::get_Text(BSTR* Value)
{

    *Value = ::SysAllocString(m_fd.getFVstr());
    return S_OK;
}

STDMETHODIMP CField::get_Vlng(int* Value)
{
    *Value = m_fd.getFVint();
    return S_OK;
}

STDMETHODIMP CField::put_Text(BSTR Value)
{
    m_fd.operator=(Value);
    return S_OK;
}

STDMETHODIMP CField::put_Vlng(int Value)
{
    m_fd.operator=(Value);
    return S_OK;
}

STDMETHODIMP CField::get_V64(__int64* Value)
{
    *Value = m_fd.getFV64();
    return S_OK;
}

STDMETHODIMP CField::put_V64(__int64 Value)
{
    m_fd.operator=(Value);
    return S_OK;
}

STDMETHODIMP CField::get_Vbin(BSTR* Value)
{
    uint_td size;
    void* p = m_fd.getFVbin(size);

    *Value = ::SysAllocStringByteLen((char*)p, size);

    return S_OK;
}

STDMETHODIMP CField::get_Vdbl(double* Value)
{
    *Value = m_fd.getFVdbl();
    return S_OK;
}

STDMETHODIMP CField::put_Vbin(BSTR Value)
{

    int len = ::SysStringByteLen(Value);
    m_fd.setBin(Value, len);
    return S_OK;
}

STDMETHODIMP CField::put_Vdbl(double Value)
{
    m_fd.operator=(Value);
    return S_OK;
}

STDMETHODIMP CField::IsNull(VARIANT_BOOL* Value)
{
    *Value = m_fd.isNull();
    return S_OK;
}

STDMETHODIMP CField::SetNull(VARIANT_BOOL Value)
{
    m_fd.setNull(Value != 0);
    return S_OK;
}

STDMETHODIMP CField::SetValue(VARIANT Value)
{
    if (Value.vt == VT_BSTR)
        m_fd = Value.bstrVal;
    else if (Value.vt == VT_R8 || Value.vt == VT_R4)
        m_fd = Value.dblVal;
    else if (Value.vt == VT_I4 || Value.vt == VT_I2 || Value.vt == VT_INT || Value.vt == VT_I8)
        m_fd = Value.llVal;
    else if(Value.vt == VT_NULL)
        m_fd = (wchar_t*)NULL;
    else if ((Value.vt == VT_DISPATCH) && Value.pdispVal)
    {
        CBitset* b = dynamic_cast<CBitset*>(Value.pdispVal);
        if (b)
            m_fd = b->m_bitset.i64();    
    }
    else
    {
        VariantChangeType( &Value, &Value, 0, VT_BSTR );
        m_fd = Value.bstrVal;
    }
    return S_OK;
}

STDMETHODIMP CField::I(int* Value)
{
    *Value = m_fd.i();
    return S_OK;
}

STDMETHODIMP CField::I64(__int64* Value)
{
    *Value = m_fd.i64();
    return S_OK;
}

STDMETHODIMP CField::D(double* Value)
{
    *Value = m_fd.d();
    return S_OK;
}

STDMETHODIMP CField::Bin(BSTR* Value)
{
    uint_td size;
    void* p = m_fd.getFVbin(size);
    *Value = ::SysAllocStringByteLen((char*)p, size);
    return S_OK;
}

STDMETHODIMP CField::SetBin(BSTR Value)
{
    int len = ::SysStringByteLen(Value);
    m_fd.setBin(Value, len);
    return S_OK;
}

STDMETHODIMP CField::Str(BSTR* Value)
{
    *Value = ::SysAllocString(m_fd.getFVstr());
    return S_OK;
}

STDMETHODIMP CField::get_Type(short* Value)
{
    *Value = m_fd.type();
    return S_OK;
}

STDMETHODIMP CField::get_Len(short* Value)
{
    *Value = m_fd.len();
    return S_OK;
}

STDMETHODIMP CField::GetBits(IBitset** Value)
{
    CComObject<CBitset>* b;
    CComObject<CBitset>::CreateInstance(&b);
    if (!b)
        return Error("CreateInstance Bitset", IID_ITable);
    b->m_bitset = bzs::db::protocol::tdap::client::bitset(m_fd.i64());
    CBitset* bi;
    b->QueryInterface(IID_IBitset, (void**)&bi);
    _ASSERTE(bi);
    *Value = bi;
    return S_OK;
}


