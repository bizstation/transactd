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



