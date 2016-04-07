/*=================================================================
   Copyright (C) 2016 BizStation Corp All rights reserved.

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
// BinlogPos.cpp : CBinlogPos ‚ÌŽÀ‘•

#include "stdafx.h"
#include "BinlogPos.h"


// CBinlogPos
#ifdef _WIN64
STDMETHODIMP CBinlogPos::get_Pos(__int64* retVal)
{
    *retVal = (__int64)m_pos.pos;
    return S_OK;
}
#else
STDMETHODIMP CBinlogPos::get_Pos(int* retVal)
{
    *retVal = (int)m_pos.pos;
    return S_OK;
}
#endif

STDMETHODIMP CBinlogPos::get_Type(short* retVal)
{
    *retVal = (short)m_pos.type;
    return S_OK;
}

STDMETHODIMP CBinlogPos::get_Filename(BSTR* retVal)
{
    CComBSTR ret;
    ret = m_pos.filename;
    *retVal = ret.Copy();
    return S_OK;
}

STDMETHODIMP CBinlogPos::get_Gtid(BSTR* retVal)
{
    CComBSTR ret;
    ret = m_pos.gtid;
    *retVal = ret.Copy();
    return S_OK;
}



    
