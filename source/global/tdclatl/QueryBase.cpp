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

STDMETHODIMP CQueryBase::AddInValue(VARIANT Value, VARIANT_BOOL Reset)
{
	VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addSeekKeyValue(Value.bstrVal, (Reset==-1));
	return S_OK;
}

STDMETHODIMP CQueryBase::In(VARIANT Value
			,VARIANT Value1, VARIANT Value2, VARIANT Value3, VARIANT Value4, VARIANT Value5
			,VARIANT Value6, VARIANT Value7, VARIANT Value8, VARIANT Value9, VARIANT Value10
			, IQueryBase** retVal)
{
	setResult(retVal);
	VariantChangeType( &Value, &Value, 0, VT_BSTR );
	m_qb.addSeekKeyValue(Value.bstrVal, false);
	
	VariantChangeType( &Value1, &Value1, 0, VT_BSTR );
	if (Value1.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value1.bstrVal, false);
	VariantChangeType( &Value2, &Value2, 0, VT_BSTR );
	if (Value2.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value2.bstrVal, false);
	VariantChangeType( &Value3, &Value3, 0, VT_BSTR );
	if (Value3.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value3.bstrVal, false);
	VariantChangeType( &Value4, &Value4, 0, VT_BSTR );
	if (Value4.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value4.bstrVal, false);
	VariantChangeType( &Value5, &Value5, 0, VT_BSTR );
	if (Value5.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value5.bstrVal, false);
	VariantChangeType( &Value6, &Value6, 0, VT_BSTR );
	if (Value6.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value6.bstrVal, false);
	VariantChangeType( &Value7, &Value7, 0, VT_BSTR );
	if (Value7.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value7.bstrVal, false);
	VariantChangeType( &Value8, &Value8, 0, VT_BSTR );
	if (Value8.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value8.bstrVal, false);
	VariantChangeType( &Value9, &Value9, 0, VT_BSTR );
	if (Value9.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value9.bstrVal, false);
	VariantChangeType( &Value10, &Value10, 0, VT_BSTR );
	if (Value10.bstrVal == L"") return S_OK;
	m_qb.addSeekKeyValue(Value10.bstrVal, false);
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


STDMETHODIMP CQueryBase::Optimize(VARIANT_BOOL v, IQueryBase** retVal)
{
	m_qb.optimize(v==-1);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CQueryBase::IsOptimize(VARIANT_BOOL* retVal)
{
	*retVal = m_qb.isOptimize();
	return S_OK;
}
