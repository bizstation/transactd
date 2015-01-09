/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include "ConnectParams.h"

using namespace bzs::db::protocol::tdap::client;

STDMETHODIMP CConnectParams::Init(BSTR protocol, BSTR hostOrIp, BSTR dbname,
                                  BSTR schemaTable, BSTR username, BSTR passwd)
{
    if (m_param)
        delete m_param;
    m_param = new connectParams(protocol, hostOrIp, dbname, schemaTable, username, passwd);
    return S_OK;
}
STDMETHODIMP CConnectParams::put_Uri(BSTR val)
{
    if (m_param)
        delete m_param;
    m_param = new connectParams(val);
    return S_OK;
}

STDMETHODIMP CConnectParams::get_Uri(BSTR* retVal)
{
    if (m_param)
    {
        CComBSTR ret;
        ret = m_param->uri();
        *retVal = ret.Copy();
    }
    return S_OK;
}

STDMETHODIMP CConnectParams::put_Mode(short val)
{
    if (m_param)
        m_param->setMode((char_td)val);
    return S_OK;
}

STDMETHODIMP CConnectParams::get_Mode(short* retVal)
{
    if (m_param)
        *retVal = m_param->mode();
    return S_OK;
}

STDMETHODIMP CConnectParams::put_Type(short val)
{
    if (m_param)
        m_param->setType(val);
    return S_OK;
}

STDMETHODIMP CConnectParams::get_Type(short* retVal)
{
    if (m_param)
        *retVal = m_param->type();
    return S_OK;
}
