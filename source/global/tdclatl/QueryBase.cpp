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

STDMETHODIMP CQueryBase::Select(BSTR Value, BSTR Value1, BSTR Value2,
                                BSTR Value3, BSTR Value4, BSTR Value5,
                                BSTR Value6, BSTR Value7, BSTR Value8,
                                BSTR Value9, BSTR Value10, IQueryBase** retVal)
{

    m_qb.addField(Value);
    if (Value1 && Value1[0])
        m_qb.addField(Value1);
    if (Value2 && Value2[0])
        m_qb.addField(Value2);
    if (Value3 && Value3[0])
        m_qb.addField(Value3);
    if (Value4 && Value4[0])
        m_qb.addField(Value4);
    if (Value5 && Value5[0])
        m_qb.addField(Value5);
    if (Value6 && Value6[0])
        m_qb.addField(Value6);
    if (Value7 && Value7[0])
        m_qb.addField(Value7);
    if (Value8 && Value8[0])
        m_qb.addField(Value8);
    if (Value9 && Value9[0])
        m_qb.addField(Value9);
    if (Value10 && Value10[0])
        m_qb.addField(Value10);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::Where(BSTR Name, BSTR Logic, VARIANT Value,
                               IQueryBase** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.addLogic(Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::And(BSTR Name, BSTR Logic, VARIANT Value,
                             IQueryBase** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.addLogic(L"and", Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::Or(BSTR Name, BSTR Logic, VARIANT Value,
                            IQueryBase** retVal)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.addLogic(L"or", Name, Logic, Value.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::AddInValue(VARIANT Value, VARIANT_BOOL Reset)
{
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.addSeekKeyValue(Value.bstrVal, (Reset == -1));
    return S_OK;
}

STDMETHODIMP CQueryBase::In(VARIANT Value, VARIANT Value1, VARIANT Value2,
                            VARIANT Value3, VARIANT Value4, VARIANT Value5,
                            VARIANT Value6, VARIANT Value7, VARIANT Value8,
                            VARIANT Value9, VARIANT Value10,
                            IQueryBase** retVal)
{
    setResult(retVal);
    if (Value.vt != VT_BSTR)
        VariantChangeType(&Value, &Value, 0, VT_BSTR);
    m_qb.addSeekKeyValue(Value.bstrVal, false);

    if (Value1.vt != VT_BSTR)
        VariantChangeType(&Value1, &Value1, 0, VT_BSTR);
    if (Value1.bstrVal && Value1.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value1.bstrVal, false);

    if (Value2.vt != VT_BSTR)
        VariantChangeType(&Value2, &Value2, 0, VT_BSTR);
    if (Value2.bstrVal && Value2.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value2.bstrVal, false);

    if (Value3.vt != VT_BSTR)
        VariantChangeType(&Value3, &Value3, 0, VT_BSTR);
    if (Value3.bstrVal && Value3.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value3.bstrVal, false);

    if (Value4.vt != VT_BSTR)
        VariantChangeType(&Value4, &Value4, 0, VT_BSTR);
    if (Value4.bstrVal && Value4.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value4.bstrVal, false);

    if (Value5.vt != VT_BSTR)
        VariantChangeType(&Value5, &Value5, 0, VT_BSTR);
    if (Value5.bstrVal && Value5.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value5.bstrVal, false);

    if (Value6.vt != VT_BSTR)
        VariantChangeType(&Value6, &Value6, 0, VT_BSTR);
    if (Value6.bstrVal && Value6.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value6.bstrVal, false);

    if (Value7.vt != VT_BSTR)
        VariantChangeType(&Value7, &Value7, 0, VT_BSTR);
    if (Value7.bstrVal && Value7.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value7.bstrVal, false);

    if (Value8.vt != VT_BSTR)
        VariantChangeType(&Value8, &Value8, 0, VT_BSTR);
    if (Value8.bstrVal && Value8.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value8.bstrVal, false);

    if (Value9.vt != VT_BSTR)
        VariantChangeType(&Value9, &Value9, 0, VT_BSTR);
    if (Value9.bstrVal && Value9.bstrVal[0])
        return S_OK;
    m_qb.addSeekKeyValue(Value9.bstrVal, false);

    if (Value10.vt != VT_BSTR)
        VariantChangeType(&Value10, &Value10, 0, VT_BSTR);
    if (Value10.bstrVal && Value10.bstrVal[0])
        return S_OK;
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

STDMETHODIMP CQueryBase::Direction(eFindType FindType, IQueryBase** retVal)
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

STDMETHODIMP CQueryBase::Optimize(eOptimize v, IQueryBase** retVal)
{
    m_qb.optimize((bzs::db::protocol::tdap::client::queryBase::eOptimize)v);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::GetOptimize(eOptimize* retVal)
{
    *retVal = (eOptimize)m_qb.getOptimize();
    return S_OK;
}

STDMETHODIMP CQueryBase::SelectCount(short* retVal)
{
    *retVal = m_qb.selectCount();
    return S_OK;
}

STDMETHODIMP CQueryBase::GetSelect(short index, BSTR* retVal)
{
    if (index >= 0 && index < m_qb.selectCount())
        *retVal = ::SysAllocString(m_qb.getSelect(index));
    else
        return Error("Invalid index", IID_IQueryBase);
    return S_OK;
}

STDMETHODIMP CQueryBase::WhereTokenCount(short* retVal)
{
    *retVal = m_qb.whereTokens();
    return S_OK;
}

STDMETHODIMP CQueryBase::GetWhereToken(short index, BSTR* retVal)
{
    if (index >= 0 && index < m_qb.whereTokens())
        *retVal = ::SysAllocString(m_qb.getWhereToken(index));
    else
        return Error("Invalid index", IID_IQueryBase);
    return S_OK;
}

STDMETHODIMP CQueryBase::BookmarkAlso(VARIANT_BOOL v, IQueryBase** retVal)
{
    m_qb.bookmarkAlso(v == -1);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CQueryBase::IsBookmarkAlso(VARIANT_BOOL* retVal)
{
    *retVal = m_qb.isBookmarkAlso();
    return S_OK;
}
