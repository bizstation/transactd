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
#include "KeySegment.h"
#include "Flags.h"

STDMETHODIMP CKeySegment::get_FieldNum(unsigned char* Value)
{
    *Value = segment()->fieldNum;
    return S_OK;
}

STDMETHODIMP CKeySegment::get_Flags(IFlags** Value)
{
    CComObject<CFlags>* piObj;
    CComObject<CFlags>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_flags = &(segment()->flags);
        IFlags* fl;
        piObj->QueryInterface(IID_IFlags, (void**)&fl);
        _ASSERTE(fl);
        *Value = piObj;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CKeySegment::put_FieldNum(unsigned char Value)
{
    segment()->fieldNum = Value;
    return S_OK;
}

STDMETHODIMP CKeySegment::put_Flags(IFlags* Value)
{
    Value->All(&segment()->flags.all);
    return S_OK;
}
