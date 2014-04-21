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
#include "Flags.h"

STDMETHODIMP CFlags::get_Bits(short Index, VARIANT_BOOL* Value)
{
    switch (Index)
    {
    case 0: *Value = m_flags->bit0;
        break;
    case 1: *Value = m_flags->bit1;
        break;
    case 2: *Value = m_flags->bit2;
        break;
    case 3: *Value = m_flags->bit3;
        break;
    case 4: *Value = m_flags->bit4;
        break;
    case 5: *Value = m_flags->bit5;
        break;
    case 6: *Value = m_flags->bit6;
        break;
    case 7: *Value = m_flags->bit7;
        break;
    case 8: *Value = m_flags->bit8;
        break;
    case 9: *Value = m_flags->bit9;
        break;
    case 10: *Value = m_flags->bitA;
        break;
    case 11: *Value = m_flags->bitB;
        break;
    case 12: *Value = m_flags->bitC;
        break;
    case 13: *Value = m_flags->bitD;
        break;
    case 14: *Value = m_flags->bitE;
        break;
    case 15: *Value = m_flags->bitF;
        break;
    }
    return S_OK;
}

STDMETHODIMP CFlags::put_Bits(short Index, VARIANT_BOOL Value)
{
    switch (Index)
    {
    case 0: m_flags->bit0 = Value;
        break;
    case 1: m_flags->bit1 = Value;
        break;
    case 2: m_flags->bit2 = Value;
        break;
    case 3: m_flags->bit3 = Value;
        break;
    case 4: m_flags->bit4 = Value;
        break;
    case 5: m_flags->bit5 = Value;
        break;
    case 6: m_flags->bit6 = Value;
        break;
    case 7: m_flags->bit7 = Value;
        break;
    case 8: m_flags->bit8 = Value;
        break;
    case 9: m_flags->bit9 = Value;
        break;
    case 10: m_flags->bitA = Value;
        break;
    case 11: m_flags->bitB = Value;
        break;
    case 12: m_flags->bitC = Value;
        break;
    case 13: m_flags->bitD = Value;
        break;
    case 14: m_flags->bitE = Value;
        break;
    case 15: m_flags->bitF = Value;
        break;
    }

    return S_OK;
}

STDMETHODIMP CFlags::All(unsigned short* Value)
{
    *Value = m_flags->all;
    return S_OK;
}

STDMETHODIMP CFlags::SetBit(short Index, VARIANT_BOOL Value) {return put_Bits(Index, Value);}

STDMETHODIMP CFlags::GetBit(short Index, VARIANT_BOOL* Value) {return get_Bits(Index, Value);}
