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
#include "activeTable.h"
#include "Database.h"
#include "QueryBase.h"
#include "Recordset.h"
#include "Record.h"
#include "TableDef.h"
#include "PooledDbManager.h"
#include "PreparedQuery.h"
#include "Table.h"

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

void CActiveTable::setResult(IActiveTable** retVal)
{
    this->QueryInterface(IID_IActiveTable, (void**)retVal);
}

void CActiveTable::FinalRelease()
{
    delete m_at;
}

STDMETHODIMP CActiveTable::SetDatabase(VARIANT Value, BSTR tableName)
{
    try
    {
        if (Value.vt != VT_DISPATCH)
        {
            _TCHAR tmp[256];
            wsprintf(tmp, _T("SetDatabase Type error type = %d"), Value.vt);
            return Error(tmp, IID_IActiveTable);
        }
        CPooledDbManager* pm = dynamic_cast<CPooledDbManager*>(Value.pdispVal);
        if (pm)
        {
            m_at = new activeTable(&pm->m_mgr, tableName);
            m_at->table()->setOptionalData((void*)NULL);
            return S_OK;
        }

        CDatabase* p = dynamic_cast<CDatabase*>(Value.pdispVal);
        if (p)
        {
            m_at = new activeTable(p->database(), tableName);
            m_at->table()->setOptionalData((void*)p->database());
            return S_OK;
        }
        return Error(_T("SetDatabase Type error"), IID_IActiveTable);
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IActiveTable);
    }
}

STDMETHODIMP CActiveTable::Index(short Value, IActiveTable** retVal)
{
    m_at->index(Value);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CActiveTable::KeyValue(VARIANT Value0, VARIANT Value1,
                                    VARIANT Value2, VARIANT Value3,
                                    VARIANT Value4, VARIANT Value5,
                                    VARIANT Value6, VARIANT Value7,
                                    IActiveTable** retVal)
{

    if (Value0.vt != VT_BSTR)
        VariantChangeType(&Value0, &Value0, 0, VT_BSTR);

    if (Value1.vt != VT_BSTR)
        VariantChangeType(&Value1, &Value1, 0, VT_BSTR);
    if (!Value1.bstrVal || !Value1.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal);
        setResult(retVal);
        return S_OK;
    }

    if (Value2.vt != VT_BSTR)
        VariantChangeType(&Value2, &Value2, 0, VT_BSTR);
    if (!Value2.bstrVal || !Value2.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal);
        setResult(retVal);
        return S_OK;
    }
    if (Value3.vt != VT_BSTR)
        VariantChangeType(&Value3, &Value3, 0, VT_BSTR);
    if (!Value3.bstrVal || !Value3.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal);
        setResult(retVal);
        return S_OK;
    }
    if (Value4.vt != VT_BSTR)
        VariantChangeType(&Value4, &Value4, 0, VT_BSTR);
    if (!Value4.bstrVal || !Value4.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal,
                       Value3.bstrVal);
        setResult(retVal);
        return S_OK;
    }

    if (Value5.vt != VT_BSTR)
        VariantChangeType(&Value5, &Value5, 0, VT_BSTR);
    if (!Value5.bstrVal || !Value5.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal,
                       Value3.bstrVal, Value4.bstrVal);
        setResult(retVal);
        return S_OK;
    }
    if (Value6.vt != VT_BSTR)
        VariantChangeType(&Value6, &Value6, 0, VT_BSTR);
    if (!Value6.bstrVal || !Value6.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal,
                       Value3.bstrVal, Value4.bstrVal, Value5.bstrVal);
        setResult(retVal);
        return S_OK;
    }

    if (Value7.vt != VT_BSTR)
        VariantChangeType(&Value7, &Value7, 0, VT_BSTR);
    if (!Value7.bstrVal || !Value7.bstrVal[0])
    {
        m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal,
                       Value3.bstrVal, Value4.bstrVal, Value5.bstrVal,
                       Value6.bstrVal);
        setResult(retVal);
        return S_OK;
    }
    m_at->keyValue(Value0.bstrVal, Value1.bstrVal, Value2.bstrVal,
                   Value3.bstrVal, Value4.bstrVal, Value5.bstrVal,
                   Value6.bstrVal, Value7.bstrVal);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CActiveTable::Option(int Value, IActiveTable** retVal)
{
    m_at->option(Value);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CActiveTable::Read(VARIANT query,
                                    VARIANT Value0, VARIANT Value1,
                                    VARIANT Value2, VARIANT Value3,
                                    VARIANT Value4, VARIANT Value5,
                                    VARIANT Value6, VARIANT Value7,
                                IRecordset** retVal)
{

    CComObject<CARecordset>* rsObj;
    CComObject<CARecordset>::CreateInstance(&rsObj);

    if (!rsObj)
        return Error(_T("Can not create recordset"), IID_IActiveTable);
    IRecordset* rs;
    rsObj->QueryInterface(IID_IRecordset, (void**)&rs);
    _ASSERTE(rs);
    *retVal = rs;
        
    try
    {
        if (query.vt == VT_DISPATCH)
        {
            CQueryBase* p = dynamic_cast<CQueryBase*>(query.pdispVal);
            if (p)
            {
                if (Value0.vt != VT_ERROR)
                    return Error(_T("Invalid ActiveTable::Read param 2"), IID_IActiveTable);
                m_at->read(*rsObj->m_rs, p->query());
                return S_OK;
            }
            else
            {
                CPreparedQuery* pq = dynamic_cast<CPreparedQuery*>(query.pdispVal);
                if (pq)
                {
                    pq->ResetAddIndex();
                    if (Value0.vt != VT_ERROR)
                    {
                        if (Value0.vt != VT_BSTR)
                            VariantChangeType(&Value0, &Value0, 0, VT_BSTR);
                        pq->addValue(Value0.bstrVal);
                    }
                    if (Value1.vt != VT_ERROR)
                    {
                        if (Value1.vt != VT_BSTR)
                            VariantChangeType(&Value1, &Value1, 0, VT_BSTR);
                        pq->addValue(Value1.bstrVal);
                    }
                    if (Value2.vt != VT_ERROR)
                    {
                        if (Value2.vt != VT_BSTR)
                            VariantChangeType(&Value2, &Value2, 0, VT_BSTR);
                        pq->addValue(Value2.bstrVal);
                    }
                    if (Value3.vt != VT_ERROR)
                    {
                        if (Value3.vt != VT_BSTR)
                            VariantChangeType(&Value3, &Value3, 0, VT_BSTR);
                        pq->addValue(Value3.bstrVal);
                    }
                    if (Value4.vt != VT_ERROR)
                    {
                        if (Value4.vt != VT_BSTR)
                            VariantChangeType(&Value4, &Value4, 0, VT_BSTR);
                        pq->addValue(Value4.bstrVal);
                    }
                    if (Value5.vt != VT_ERROR)
                    {
                        if (Value5.vt != VT_BSTR)
                            VariantChangeType(&Value5, &Value5, 0, VT_BSTR);
                        pq->addValue(Value5.bstrVal);
                    }
                    if (Value6.vt != VT_ERROR)
                    {
                        if (Value6.vt != VT_BSTR)
                            VariantChangeType(&Value6, &Value6, 0, VT_BSTR);
                        pq->addValue(Value6.bstrVal);
                    }
                    if (Value7.vt != VT_ERROR)
                    {
                        if (Value7.vt != VT_BSTR)
                            VariantChangeType(&Value7, &Value7, 0, VT_BSTR);
                        pq->addValue(Value7.bstrVal);
                    }
                    m_at->read(*rsObj->m_rs, pq->getFilter());
                    return S_OK; 
                }
            }
        }
        else
        {
            queryBase q;
            q.reject(1).limit(1);
            m_at->read(*rsObj->m_rs, q);
            return S_OK;
        }
        return Error(_T("Invalid ActiveTable::Read param 1"), IID_IActiveTable);
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IActiveTable);
    }
}

STDMETHODIMP CActiveTable::Alias(BSTR Src, BSTR Dst, IActiveTable** retVal)
{
    m_at->alias(Src, Dst);
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CActiveTable::ResetAlias(IActiveTable** retVal)
{
    m_at->resetAlias();
    setResult(retVal);
    return S_OK;
}

STDMETHODIMP CActiveTable::Join(IRecordset* rs, VARIANT query, BSTR Name0,
                                BSTR Name1, BSTR Name2, BSTR Name3, BSTR Name4,
                                BSTR Name5, BSTR Name6, BSTR Name7,
                                IRecordset** retVal)
{
    try
    {
        if ((query.vt == VT_DISPATCH) && query.pdispVal)
        {
            CARecordset* p = dynamic_cast<CARecordset*>(rs);
            CQueryBase* qb = dynamic_cast<CQueryBase*>(query.pdispVal);
            CPreparedQuery* pq = dynamic_cast<CPreparedQuery*>(query.pdispVal);
            if (p)
            {
                if (qb)
                {
                    queryBase* q = &qb->query();
                    if (q)
                    {
                        if (!Name1 || !Name1[0])
                            m_at->join(*p->m_rs, *q, Name0);
                        else if (!Name2 || !Name2[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1);
                        else if (!Name3 || !Name3[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2);
                        else if (!Name4 || !Name4[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2, Name3);
                        else if (!Name5 || !Name5[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4);
                        else if (!Name6 || !Name6[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5);
                        else if (!Name7 || !Name7[0])
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6);
                        else
                            m_at->join(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6, Name7);
                        
                        p->QueryInterface(IID_IRecordset, (void**)retVal);
                        return S_OK;
                    }
                }
                else if (pq)
                {
                    bzs::db::protocol::tdap::client::pq_handle& stmt = pq->getFilter();
                    if (stmt)
                    {
                        if (!Name1 || !Name1[0])
                            m_at->join(*p->m_rs, stmt, Name0);
                        else if (!Name2 || !Name2[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1);
                        else if (!Name3 || !Name3[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2);
                        else if (!Name4 || !Name4[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2, Name3);
                        else if (!Name5 || !Name5[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4);
                        else if (!Name6 || !Name6[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5);
                        else if (!Name7 || !Name7[0])
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6);
                        else
                            m_at->join(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6, Name7);
                        p->QueryInterface(IID_IRecordset, (void**)retVal);
                        return S_OK;
                    }
                }
            }
        }
        return Error(_T("Invalid ActiveTable::Join param 2"), IID_IActiveTable);
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IActiveTable);
    }
}

STDMETHODIMP CActiveTable::OuterJoin(IRecordset* rs, VARIANT query,
                                     BSTR Name0, BSTR Name1, BSTR Name2,
                                     BSTR Name3, BSTR Name4, BSTR Name5,
                                     BSTR Name6, BSTR Name7,
                                     IRecordset** retVal)
{
    try
    {
        if ((query.vt == VT_DISPATCH) && query.pdispVal)
        {
            CARecordset* p = dynamic_cast<CARecordset*>(rs);
            CQueryBase* qb = dynamic_cast<CQueryBase*>(query.pdispVal);
            CPreparedQuery* pq = dynamic_cast<CPreparedQuery*>(query.pdispVal);
            if (p)
            {
                if (qb)
                {
                    queryBase* q = &qb->query();
                    if (q)
                    {
                        if (!Name1 || !Name1[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0);
                        else if (!Name2 || !Name2[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1);
                        else if (!Name3 || !Name3[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2);
                        else if (!Name4 || !Name4[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2, Name3);
                        else if (!Name5 || !Name5[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4);
                        else if (!Name6 || !Name6[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5);
                        else if (!Name7 || !Name7[0])
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6);
                        else
                            m_at->outerJoin(*p->m_rs, *q, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6, Name7);
                        
                        p->QueryInterface(IID_IRecordset, (void**)retVal);
                        return S_OK;
                    }
                }
                else if (pq)
                {
                    bzs::db::protocol::tdap::client::pq_handle& stmt = pq->getFilter();
                    if (stmt)
                    {
                        if (!Name1 || !Name1[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0);
                        else if (!Name2 || !Name2[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1);
                        else if (!Name3 || !Name3[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2);
                        else if (!Name4 || !Name4[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2, Name3);
                        else if (!Name5 || !Name5[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4);
                        else if (!Name6 || !Name6[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5);
                        else if (!Name7 || !Name7[0])
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6);
                        else
                            m_at->outerJoin(*p->m_rs, stmt, Name0, Name1, Name2, Name3, Name4,
                                       Name5, Name6, Name7);
                        p->QueryInterface(IID_IRecordset, (void**)retVal);
                        return S_OK;
                    }
                }
            }
        }
        return Error(_T("Invalid ActiveTable::OuterJoin param 2"), IID_IActiveTable);
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IActiveTable);
    }

}

STDMETHODIMP CActiveTable::Prepare(IQueryBase* Value, VARIANT_BOOL ServerPrepare, IPreparedQuery** retVal)
{
    if (Value)
    {
        CQueryBase* p = dynamic_cast<CQueryBase*>(Value);
        if (p)
        {
            CComObject<CPreparedQuery>* rsObj;
            CComObject<CPreparedQuery>::CreateInstance(&rsObj);

            if (!rsObj)
                return Error(_T("Can not create preparedQuery"), IID_ITable);

            rsObj->setPqHandle(m_at->prepare(p->query(), (bool)ServerPrepare));
            IPreparedQuery* pd;
            rsObj->QueryInterface(IID_IPreparedQuery, (void**)&pd);
            _ASSERTE(pd);
            *retVal = pd;
            return S_OK;
        }
    }
    return Error(_T("Invalid ActiveTable::Prepare param 1"), IID_IActiveTable);
}


STDMETHODIMP CActiveTable::GetWritableRecord(IWritableRecord** retVal)
{

    if (m_recObj == NULL)
    {
        CComObject<CWritableRecord>::CreateInstance(&m_recObj);
        m_recObj->m_rec = &m_at->getWritableRecord();
    }
    if (m_recObj)
    {

        IWritableRecord* wrec;
        m_recObj->QueryInterface(IID_IWritableRecord, (void**)&wrec);
        _ASSERTE(wrec);
        *retVal = wrec;
    }

    return S_OK;
}

STDMETHODIMP CActiveTable::get_TableDef(ITableDef** Value)
{
    CComObject<CTableDef>* piObj;
    CComObject<CTableDef>::CreateInstance(&piObj);
    if (piObj)
    {

        piObj->m_tabledefPtr =
            const_cast<tabledef**>(m_at->table()->tableDefPtr());

        ITableDef* tbd;
        piObj->QueryInterface(IID_ITableDef, (void**)&tbd);
        _ASSERTE(tbd);
        *Value = tbd;
    }
    else
        *Value = 0;
    return S_OK;
}

STDMETHODIMP CActiveTable::Table(ITable** retVal)
{
    try
    {
        CComObject<CTableTd>* ptb;
        CComObject<CTableTd>::CreateInstance(&ptb);

        if (ptb)
        {
            ptb->m_tb = m_at->table();
            ptb->m_tb->setOptionalData((void*)NULL);
            ITable* itb;
            ptb->QueryInterface(IID_ITable, (void**)&itb);
            _ASSERTE(itb);
            *retVal = itb;
        }
        else
            *retVal = NULL;
        return S_OK;
    }

    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IActiveTable);
    }
}
