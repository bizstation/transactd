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
#include "KeyDef.h"
#include "KeySegment.h"

STDMETHODIMP CKeyDef::get_SegmentCount(unsigned char* Value)
{
    *Value = keydef()->segmentCount;
    return S_OK;
};

STDMETHODIMP CKeyDef::put_SegmentCount(unsigned char Value)
{
    keydef()->segmentCount = Value;
    return S_OK;
}

STDMETHODIMP CKeyDef::Segments(short Index, IKeySegment** Value)
{
    CComObject<CKeySegment>* piObj;
    CComObject<CKeySegment>::CreateInstance(&piObj);
    if (piObj)
    {
        piObj->m_tabledefPtr = m_tabledefPtr;
        piObj->m_keyIndex = m_index;
        piObj->m_index = Index;
        IKeySegment* sg;
        piObj->QueryInterface(IID_IKeySegment, (void**)&sg);
        _ASSERTE(sg);
        *Value = piObj;
    }
    else
        *Value = 0;
    return S_OK;
}
