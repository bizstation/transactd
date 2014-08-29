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
#include "Record.h"
#include "Field.h"
#include "FieldDefs.h"

void CRecord::FinalRelease()
{
	if (m_fieldObj)
		m_fieldObj->Release();

}
short CRecord::GetFieldNum(VARIANT* Index)
{
    short index = -1;
    if (Index->vt == VT_BSTR)
        index = m_rec->indexByName(Index->bstrVal);
    else if ((Index->vt == VT_I2) || (Index->vt == VT_I4))
        index = Index->iVal;
    return index;
}

STDMETHODIMP CRecord::get_Size(short* retVal)
{
	*retVal = (short)m_rec->size();
	return S_OK;
}

STDMETHODIMP CRecord::get_Field(VARIANT Index, IField** retVal)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

	if (m_fieldObj == NULL)
	{
		CComObject<CField>::CreateInstance(&m_fieldObj);
		m_fieldObj->AddRef();
		
	}
	if (m_fieldObj)
	{				 
		m_fieldObj->m_fd = (*m_rec)[index]; 
		IField* fd;
		m_fieldObj->QueryInterface(IID_IField, (void**)&fd);
		_ASSERTE(fd);
		*retVal = fd;
	}
	return S_OK;

}

STDMETHODIMP CRecord::get_IsInvalidRecord( VARIANT_BOOL* retVal)
{
	*retVal = m_rec->isInvalidRecord();
	return S_OK;
}

void CWritableRecord::FinalRelease()
{
	if (m_fieldObj)
		m_fieldObj->Release();
	if (m_fieldDefsObj)
		m_fieldDefsObj->Release();

}

short CWritableRecord::GetFieldNum(VARIANT* Index)
{
    short index = -1;
    if (Index->vt == VT_BSTR)
        index = m_rec->indexByName(Index->bstrVal);
    else if ((Index->vt == VT_I2) || (Index->vt == VT_I4))
        index = Index->iVal;
    return index;
}

STDMETHODIMP CWritableRecord::Clear()
{
	m_rec->clear();
	return S_OK;
}

STDMETHODIMP CWritableRecord::get_Size(short* retVal)
{
	*retVal = (short)m_rec->size();
	return S_OK;
}

STDMETHODIMP CWritableRecord::get_Field(VARIANT Index, IField** retVal)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_IWritableRecord);

	if (m_fieldObj == NULL)
	{
		CComObject<CField>::CreateInstance(&m_fieldObj);
		m_fieldObj->AddRef();
	}
	if (m_fieldObj)
	{				 
		m_fieldObj->m_fd = (*m_rec)[index]; 
		IField* fd;
		m_fieldObj->QueryInterface(IID_IField, (void**)&fd);
		_ASSERTE(fd);
		*retVal = fd;
	}
	return S_OK;

}

STDMETHODIMP CWritableRecord::Save()
{
	try
	{
		m_rec->save();
		return S_OK;
	}
	catch(bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Insert()
{
	try
	{
		m_rec->insert();
		return S_OK;
	}
	catch(bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Del(VARIANT_BOOL KeysetAlrady)
{
	try
	{
		m_rec->del(KeysetAlrady);
		return S_OK;
	}
	catch(bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Update()
{
	try
	{
		m_rec->update();
		return S_OK;
	}
	catch(bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Read(VARIANT_BOOL KeysetAlrady, VARIANT_BOOL* retVal)
{
	try
	{
		*retVal = m_rec->read(KeysetAlrady);
		return S_OK;
	}
	catch(bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::get_FieldDefs(IFieldDefs** retVal)
{
	if (m_fieldDefsObj == NULL)
	{
		CComObject<CFieldDefs>::CreateInstance(&m_fieldDefsObj);
		m_fieldDefsObj->AddRef();
	}
	if (m_fieldDefsObj)
	{
		m_fieldDefsObj->m_fds = m_rec->fieldDefs();
		
		IFieldDefs* fds;
		m_fieldDefsObj->QueryInterface(IID_IFieldDefs, (void**)&fds);
		_ASSERTE(fds);
		*retVal = fds;
	}
	return S_OK;
}

STDMETHODIMP CWritableRecord::get_IsInvalidRecord(VARIANT_BOOL* retVal)
{
	*retVal = m_rec->isInvalidRecord();
	return S_OK;
}
