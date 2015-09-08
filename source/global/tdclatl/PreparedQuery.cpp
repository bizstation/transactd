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
#include "PreparedQuery.h"

STDMETHODIMP CPreparedQuery::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IPreparedQuery
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CPreparedQuery::SupplyValue(int Index, VARIANT Value, VARIANT_BOOL* retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);

    if (Value.bstrVal && Value.bstrVal[0])
    {
        *retVal = m_pq->supplyValue(Index, Value.bstrVal);
        return S_OK;
    }
    return Error(_T("Invalid PreparedQuery::SupplyValue param 2."), IID_IPreparedQuery);
}

STDMETHODIMP CPreparedQuery::AddValue(VARIANT Value, VARIANT_BOOL* retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);

    if (Value.bstrVal && Value.bstrVal[0])
    {
        *retVal = m_pq->addValue(Value.bstrVal);
        return S_OK;
    }
    return Error(_T("Invalid PreparedQuery::AddValue param 1."), IID_IPreparedQuery);
}

STDMETHODIMP CPreparedQuery::ResetAddIndex()
{
    m_pq->resetAddIndex();
    return S_OK;
}

