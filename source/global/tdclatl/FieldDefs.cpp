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
#include "FieldDefs.h"
#include "FieldDef.h"

void CFieldDefs::FinalRelease()
{
	if (m_fieldDefObj)
		m_fieldDefObj->AddRef();
}

short CFieldDefs::GetFieldNum(VARIANT* Index)
{
    short index = -1;
    if (Index->vt == VT_BSTR)
        index = m_fds->indexByName(Index->bstrVal);
    else if ((Index->vt == VT_I2) || (Index->vt == VT_I4))
        index = Index->iVal;
    return index;
}

STDMETHODIMP CFieldDefs::IndexByName(BSTR Name, short* retVal)
{
	*retVal = m_fds->indexByName(Name);
	return S_OK;
}

STDMETHODIMP CFieldDefs::get_FieldDef(VARIANT Name, IFieldDef** retVal)
{
	short index = GetFieldNum(&Name);
	if (index >= 0)
	{
		if (m_fieldDefObj == NULL)
		{
			CComObject<CFieldDef>::CreateInstance(&m_fieldDefObj);
			m_fieldDefObj->AddRef();
		}
		if (m_fieldDefObj)
		{
			m_fieldDefObj->m_fielddef = &((*m_fds)[index]);

			IFieldDef* fd;
			m_fieldDefObj->QueryInterface(IID_IFieldDef, (void**)&fd);
			_ASSERTE(fd);
			*retVal = fd;
		}
	}else
		return Error("Invalid index", IID_IFieldDefs);
	return S_OK;
}

STDMETHODIMP CFieldDefs::get_Size(short* retVal)
{
	*retVal = (short)m_fds->size();
	return S_OK;
}
