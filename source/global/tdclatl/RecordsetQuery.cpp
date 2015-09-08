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
#include "RecordsetQuery.h"
STDMETHODIMP CRecordsetQuery::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IRecordsetQuery
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

void CRecordsetQuery::setResult(IRecordsetQuery** retVal)
{
    this->QueryInterface(IID_IRecordsetQuery, (void**)retVal);
}

STDMETHODIMP CRecordsetQuery::Reset(IRecordsetQuery** retVal)
{
    m_qb.reset();
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CRecordsetQuery::When(BSTR Name, BSTR Logic, VARIANT Value,
                                   IRecordsetQuery** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.when(Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CRecordsetQuery::And(BSTR Name, BSTR Logic, VARIANT Value,
                                  IRecordsetQuery** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.and_(Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CRecordsetQuery::Or(BSTR Name, BSTR Logic, VARIANT Value,
                                 IRecordsetQuery** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.or_(Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CSortField::get_Name(BSTR* Value)
{
    return S_OK;
}

STDMETHODIMP CSortField::put_Name(BSTR Value)
{
    return S_OK;
}

STDMETHODIMP CSortField::get_Asc(VARIANT_BOOL* Value)
{
    return S_OK;
}

STDMETHODIMP CSortField::put_Asc(VARIANT_BOOL Value)
{
    return S_OK;
}

//------------------------------------------------------------------------
STDMETHODIMP CSortFields::Add(BSTR Name, VARIANT_BOOL Asc)
{
    m_sortFields.add(Name, (Asc == -1));
    return S_OK;
}

STDMETHODIMP CSortFields::get_Size(int* Value)
{
    *Value = (int)m_sortFields.size();
    return S_OK;
}

STDMETHODIMP CSortFields::Item(int Index, ISortField** retVal)
{
    CComObject<CSortField>* obj;

    CComObject<CSortField>::CreateInstance(&obj);
    if (obj)
    {
        obj->m_sortField = m_sortFields[Index];

        ISortField* fd;
        obj->QueryInterface(IID_ISortField, (void**)&fd);
        _ASSERTE(fd);
        *retVal = fd;
    }
    return S_OK;
}

STDMETHODIMP CSortFields::Clear()
{
    m_sortFields.clear();
    return S_OK;
}
