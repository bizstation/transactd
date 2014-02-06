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
#include "QueryBase.h"
#include "Table.h"



STDMETHODIMP CQueryBase::Reset(void)
{
    m_qb.reset();
	return S_OK;
}

STDMETHODIMP CQueryBase::ClearSeekKeyValues(void)
{
    m_qb.clearSeekKeyValues();
	return S_OK;
}

STDMETHODIMP CQueryBase::ClearSelectFields(void)
{
    m_qb.clearSelectFields();
	return S_OK;
}

STDMETHODIMP CQueryBase::Select(BSTR Value
						,BSTR Value1, BSTR Value2, BSTR Value3, BSTR Value4, BSTR Value5
						,BSTR Value6, BSTR Value7, BSTR Value8, BSTR Value9, BSTR Value10
						,IQueryBase** retVal)
{
    m_qb.addField(Value);
    if (Value1 != L"")
		m_qb.addField(Value1);
    if (Value2 != L"")
		m_qb.addField(Value2);
    if (Value3 != L"")
		m_qb.addField(Value3);
    if (Value4 != L"")
		m_qb.addField(Value4);
    if (Value5 != L"")
		m_qb.addField(Value5);
    if (Value6 != L"")
		m_qb.addField(Value6);
    if (Value7 != L"")
		m_qb.addField(Value7);
    if (Value8 != L"")
		m_qb.addField(Value8);
    if (Value9 != L"")
		m_qb.addField(Value9);
    if (Value10 != L"")
		m_qb.addField(Value10);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::Where(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal)
{
    VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addLogic(Name, Logic, Value.bstrVal);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::And(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal)
{
    VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addLogic(L"and", Name, Logic, Value.bstrVal);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::Or(BSTR Name, BSTR Logic, VARIANT Value, IQueryBase** retVal)
{
    VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addLogic(L"or", Name, Logic, Value.bstrVal);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::AddSeekKeyValue(VARIANT Value, VARIANT_BOOL Reset)
{
    VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addSeekKeyValue(Value.bstrVal, (Reset==-1));
	return S_OK;
}

void CQueryBase::setResult(IQueryBase** retVal)
{
	this->QueryInterface(IID_IQueryBase, (void**)retVal);
}

STDMETHODIMP CQueryBase::QueryString(BSTR Value, IQueryBase** retVal)
{
    m_qb.queryString(Value);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::Reject(long Value, IQueryBase** retVal)
{
    m_qb.reject(Value);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::Limit(long Value, IQueryBase** retVal)
{
    m_qb.limit(Value);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::Direction( eFindType FindType, IQueryBase** retVal)
{
    m_qb.direction((bzs::db::protocol::tdap::client::table::eFindType)FindType);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::All(IQueryBase** retVal)
{
    m_qb.all();
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::ToString(BSTR* retVal)
{
  
	CComBSTR ret;
    ret = m_qb.toString();
    *retVal = ret.Copy();
	return S_OK;
}

STDMETHODIMP CQueryBase::GetDirection(eFindType* retVal)
{
    *retVal = (eFindType)m_qb.getDirection();
	return S_OK;
}

STDMETHODIMP CQueryBase::GetReject(long* retVal)
{
    *retVal = m_qb.getReject();
    return S_OK;
}

STDMETHODIMP CQueryBase::GetLimit(long* retVal)
{
    *retVal = m_qb.getLimit();
	return S_OK;
}

STDMETHODIMP CQueryBase::IsAll(VARIANT_BOOL* retVal)
{
    *retVal = m_qb.isAll();
	return S_OK;
}