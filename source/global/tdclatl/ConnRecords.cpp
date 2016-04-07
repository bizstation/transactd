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
#include "ConnRecords.h"
#include "ConnRecord.h"
STDMETHODIMP CConnRecords::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IConnRecords
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CConnRecords::get_Record(short index, IConnRecord** retVal)
{
    if (index >= (short)m_recs.size())
        return Error("Invalid index", IID_IConnRecords);
    CComObject<CConnRecord>* obj;
    CComObject<CConnRecord>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecord", IID_IConnRecords);
    obj->m_rec = m_recs[index];
    IConnRecord* rec;
    obj->QueryInterface(IID_IConnRecord, (void**)&rec);
    _ASSERTE(rec);
    *retVal = rec;
    return S_OK;    
}

STDMETHODIMP CConnRecords::get_Size(int* retVal)
{
    *retVal = (int)m_recs.size();
    return S_OK;    
}