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
#include "StdAfx.h"
#include "TdVersion.h"
#include <bzs/db/protocol/tdap/client/database.h>

using namespace bzs::db::protocol::tdap::client;

STDMETHODIMP CTdVersion::get_MajorVersion(short* Value)
{
    *Value = m_ver.majorVersion;
    return S_OK;
}

STDMETHODIMP CTdVersion::put_MajorVersion(short Value)
{
    m_ver.majorVersion = Value;
    return S_OK;
}

STDMETHODIMP CTdVersion::get_MinorVersion(short* Value)
{
    *Value = m_ver.minorVersion;
    return S_OK;
}

STDMETHODIMP CTdVersion::put_MinorVersion(short Value)
{
    m_ver.minorVersion = Value;
    return S_OK;
}

STDMETHODIMP CTdVersion::get_Type(short* Value)
{
    *Value = m_ver.type;
    return S_OK;
}

STDMETHODIMP CTdVersion::put_Type(short Value)
{
    m_ver.type = (char)Value;
    return S_OK;
}

STDMETHODIMP CTdVersion::ModuleTypeString(BSTR* Value)
{
    *Value = ::SysAllocString(m_ver.moduleTypeString());
    return S_OK;
}

STDMETHODIMP CTdVersion::ModuleVersionShortString(BSTR* Value)
{

    wchar_t tmp[512] ={NULL};
    m_ver.moduleVersionShortString(tmp);
    *Value = ::SysAllocString(tmp);
    return S_OK;
}
