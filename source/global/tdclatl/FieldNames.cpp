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
#include "FieldNames.h"


using namespace bzs::db::protocol::tdap::client;

void CFieldNames::setResult(IFieldNames** retVal)
{
	this->QueryInterface(IID_IFieldNames, (void**)retVal);
}

STDMETHODIMP CFieldNames::KeyField(BSTR Name0, BSTR Name1, BSTR Name2, BSTR Name3, BSTR Name4, BSTR Name5,
				BSTR Name6, BSTR Name7,	BSTR Name8, BSTR Name9, BSTR Name10, IFieldNames** retVal)
{
	if (Name0 && Name0[0])
	{
		if (!Name1 || !Name1[0])
			m_fnsPtr->keyField(Name0);	
		else if (!Name2 || !Name2[0]) 
			m_fnsPtr->keyField(Name0, Name1);	
		else if (!Name3 || !Name3[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2);	
		else if (!Name4 || !Name4[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3);	
		else if (!Name5 || !Name5[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4);	
		else if (!Name6 || !Name6[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5);	
		else if (!Name7 || !Name7[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6);
		else if (!Name8 || !Name8[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7);
		else if (!Name9 || !Name9[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8);
		else if (!Name10 || !Name10[0]) 
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8, Name9);
		else
			m_fnsPtr->keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8, Name9, Name10);
	}
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CFieldNames::get_Count(int* Value)
{
    *Value = m_fnsPtr->count();
	return S_OK;
}

STDMETHODIMP CFieldNames::addValue(BSTR val)
{
    m_fnsPtr->addValue(val);
	return S_OK;
}

STDMETHODIMP CFieldNames::get_Value(int index, BSTR* retVal)
{
    CComBSTR ret;
    ret = m_fnsPtr->getValue(index);
	*retVal = ret.Copy();
	return S_OK;
}

STDMETHODIMP CFieldNames::addValues(BSTR val, BSTR delm)
{
    m_fnsPtr->addValues(val, delm);
	return S_OK;
}

STDMETHODIMP CFieldNames::Reset(IFieldNames** retVal)
{
    m_fnsPtr->reset();
	setResult(retVal);
	return S_OK;
}
