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
#include "Bookmark.h"

void CRecord::FinalRelease()
{
    if (m_fieldObj != NULL)
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

STDMETHODIMP CRecord::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IRecord
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
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
        if (!m_fieldObj)
            return Error("CreateInstance Field", IID_IRecord);
        m_fieldObj->AddRef();
    }
    try
    {
        m_fieldObj->m_fd = (*m_rec)[index];
        IField* fd;
        m_fieldObj->QueryInterface(IID_IField, (void**)&fd);
        _ASSERTE(fd);
        *retVal = fd;
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IRecord);
    }
}

STDMETHODIMP CRecord::get_IsInvalidRecord(VARIANT_BOOL* retVal)
{
    *retVal = m_rec->isInvalidRecord();
    return S_OK;
}


//---------------------------------------------------------------------
STDMETHODIMP CWritableRecord::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IWritableRecord
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

void CWritableRecord::FinalRelease()
{
    if (m_fieldDefsObj != NULL)
        m_fieldDefsObj->Release();
    if (m_fieldObj)
        m_fieldObj->Release();
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
        if (!m_fieldObj)
            return Error("CreateInstance Field", IID_IWritableRecord);
        m_fieldObj->AddRef();
    }
    try
    {
        m_fieldObj->m_fd = (*m_rec)[index];
        IField* fd;
        m_fieldObj->QueryInterface(IID_IField, (void**)&fd);
        _ASSERTE(fd);
        *retVal = fd;
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Save()
{
    try
    {
        m_rec->save();
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
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
    catch (bzs::rtl::exception& e)
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
    catch (bzs::rtl::exception& e)
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
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::Read(VARIANT param,
                                   VARIANT_BOOL* retVal)
{
    try
    {
        if ((param.vt == VT_DISPATCH) && param.pdispVal)
        {
            CBookmark* bm = dynamic_cast<CBookmark*>(param.pdispVal);
            if (bm)
                m_rec->read(bm->internalBookmark());
            else
                return Error("Invalid param 1 not IBookmark", IID_IWritableRecord);
        }
        else if (param.vt == VT_BOOL) 
            *retVal = m_rec->read(param.boolVal);    
        else
            *retVal = m_rec->read();
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::get_FieldDefs(IFieldDefs** retVal)
{
    if (m_fieldDefsObj == NULL)
    {
        CComObject<CFieldDefs>::CreateInstance(&m_fieldDefsObj);
        if (!m_fieldDefsObj)
            return Error("CreateInstance FieldDefs", IID_IWritableRecord);
        m_fieldDefsObj->AddRef();
    }
    try
    {
        m_fieldDefsObj->m_fds = m_rec->fieldDefs();
        IFieldDefs* fds;
        m_fieldDefsObj->QueryInterface(IID_IFieldDefs, (void**)&fds);
        _ASSERTE(fds);
        *retVal = fds;
        return S_OK;
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IWritableRecord);
    }
}

STDMETHODIMP CWritableRecord::get_IsInvalidRecord(VARIANT_BOOL* retVal)
{
    *retVal = m_rec->isInvalidRecord();
    return S_OK;
}
