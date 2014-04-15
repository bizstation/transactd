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

void CRecord::FinalRelease()
{
	if (m_fieldObj)
		;//m_fieldObj->Release();
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

STDMETHODIMP CRecord::Field(VARIANT Index, IField** retVal)
{
    short index = GetFieldNum(&Index);
    if (index < 0)
        return Error("Invalid index", IID_ITable);

	if (m_fieldObj == NULL)
	{
		CComObject<CField>::CreateInstance(&m_fieldObj);
		if (m_fieldObj)
			;//m_fieldObj->AddRef();
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