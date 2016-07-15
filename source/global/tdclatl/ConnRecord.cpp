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
#include "stdafx.h"
#include "ConnRecord.h"
STDMETHODIMP CConnRecord::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID* const arr[] = 
  {
    &IID_IConnRecord
  };

  for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
  {
    if (InlineIsEqualGUID(*arr[i],riid))
      return S_OK;
  }
  return S_FALSE;
}


#ifdef _WIN64
STDMETHODIMP CConnRecord::get_ConId(__int64* retVal)
{
    *retVal = m_rec.conId;
    return S_OK;
}
#else
STDMETHODIMP CConnRecord::get_ConId(BSTR* retVal)
{
    wchar_t tmp[256];
    _i64tot_s(m_rec.conId, tmp, 256, 10);
    CComBSTR ret = tmp;
    *retVal = ret.Copy();

    return S_OK;
}
#endif

STDMETHODIMP CConnRecord::get_Id(int* retVal)
{
    *retVal = m_rec.id;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_Db(int* retVal)
{
    *retVal = m_rec.db;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_Type(int* retVal)
{
    *retVal = m_rec.type;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_Name(BSTR* retVal)
{
    CComBSTR ret;
    wchar_t tmp[256];
    ret = m_rec.t_name(tmp, 256);
    *retVal = ret.Copy();
    return S_OK;
} 

STDMETHODIMP CConnRecord::get_Value(BSTR* retVal)
{
    CComBSTR ret;
    wchar_t tmp[256];
    m_rec.value(tmp, 256);
    ret = tmp;
    *retVal = ret.Copy();
    return S_OK;
}

STDMETHODIMP CConnRecord::get_LongValue(BSTR* retVal)
{
    CComBSTR ret;
    wchar_t tmp[256];
    _i64tow_s(m_rec.longValue, tmp, 256, 10);
    ret = tmp;
    *retVal = ret.Copy();
    return S_OK;
}


STDMETHODIMP CConnRecord::get_Status(short* retVal)
{
    *retVal = m_rec.status;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_ReadCount(int* retVal)
{
    *retVal = m_rec.readCount;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_UpdCount(int* retVal)
{
    *retVal = m_rec.updCount;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_DelCount(int* retVal)
{
    *retVal = m_rec.delCount;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_InsCount(int* retVal)
{
    *retVal = m_rec.insCount;
    return S_OK;
}

STDMETHODIMP CConnRecord::get_Port(int* retVal)
{
    *retVal = m_rec.port;
    return S_OK;
}