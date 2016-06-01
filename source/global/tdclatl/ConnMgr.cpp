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
#include "ConnMgr.h"
#include "ConnRecords.h"
#include "Database.h"

using namespace bzs::db::protocol::tdap::client;

STDMETHODIMP CConnMgr::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IConnMgr
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CConnMgr::SetDatabase(VARIANT Value)
{
    try
    {
        if (Value.vt != VT_DISPATCH)
        {
            _TCHAR tmp[256];
            wsprintf(tmp, _T("SetDatabase Type error type = %d"), Value.vt);
            return Error(tmp, IID_IConnMgr);
        }

        CDatabase* p = dynamic_cast<CDatabase*>(Value.pdispVal);
        if (p)
        {
            m_mgr = connMgr::create(p->database());
            return S_OK;
        }
        return Error(_T("SetDatabase Type error"), IID_IConnMgr);
    }
    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IConnMgr);
    }
}


STDMETHODIMP CConnMgr::Connect(BSTR uri, VARIANT_BOOL* retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    m_mgr->connect(uri);
    return S_OK;
}

STDMETHODIMP CConnMgr::Disconnect()
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    m_mgr->disconnect();
    return S_OK;
}

void setResult(CComObject<CConnRecords>* obj, IConnRecords** retVal)
{
    IConnRecords* recs;
    obj->QueryInterface(IID_IConnRecords, (void**)&recs);
    _ASSERTE(recs);
    *retVal = recs;

}

STDMETHODIMP CConnMgr::Databases(IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->databases();
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::Tables(BSTR dbname, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->tables(dbname);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::Views(BSTR dbname, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->views(dbname);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::SchemaTables(BSTR dbname, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->schemaTables(dbname);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::Sysvars(IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->sysvars();
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::Statusvars(IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->statusvars();
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::SlaveStatus(BSTR channel, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->slaveStatus(channel);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::Connections(IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->connections();
    setResult(obj, retVal);
    return S_OK;
}

#ifdef _WIN64
STDMETHODIMP CConnMgr::InUseDatabases(__int64 connid, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->inUseDatabases(connid);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::InUseTables(__int64 connid, int db, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->inUseTables(connid, db);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::PostDisconnectOne(__int64 connid)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    m_mgr->postDisconnectOne(connid);
    return S_OK;
}

#else

STDMETHODIMP CConnMgr::InUseDatabases(BSTR connid, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->inUseDatabases(_wtoi64(connid));
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::InUseTables(BSTR connid, int db, IConnRecords** retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComObject<CConnRecords>* obj;
    CComObject<CConnRecords>::CreateInstance(&obj);
    if (!obj)
        return Error("CreateInstance ConnRecords", IID_IConnMgr);
    obj->m_recs = m_mgr->inUseTables(_wtoi64(connid), db);
    setResult(obj, retVal);
    return S_OK;
}

STDMETHODIMP CConnMgr::PostDisconnectOne(BSTR connid)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    m_mgr->postDisconnectOne(_wtoi64(connid));
    return S_OK;
}

#endif

STDMETHODIMP CConnMgr::PostDisconnectAll()
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    m_mgr->postDisconnectAll();
    return S_OK;
}

STDMETHODIMP CConnMgr::get_Stat(short* retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    *retVal = m_mgr->stat();
    return S_OK;
}

STDMETHODIMP CConnMgr::TdapErr(OLE_HANDLE hWnd, BSTR* retVal)
{
    return S_OK;
}
    
STDMETHODIMP CConnMgr::RemoveSystemDb(IConnRecords** retVal)
{
    CConnRecords* p = dynamic_cast<CConnRecords*>(*retVal);
    if (p)
    {
        connMgr::removeSystemDb(p->m_recs);
        return S_OK;
    }
    return Error(_T("Invalid object"), IID_IConnMgr);
}

STDMETHODIMP CConnMgr::SlaveStatusName(int index, BSTR* retVal)
{
    if (m_mgr == NULL) Error(_T("No database error"), IID_IConnMgr);
    CComBSTR ret= m_mgr->slaveStatusName(index);
    *retVal = ret.Copy();
    return S_OK;
}

STDMETHODIMP CConnMgr::SysvarName(int index, BSTR* retVal)
{
    CComBSTR ret= connMgr::sysvarName(index);
    *retVal = ret.Copy();
    return S_OK;
}

STDMETHODIMP CConnMgr::StatusvarName(int index, BSTR* retVal)
{
    CComBSTR ret= connMgr::statusvarName(index);
    *retVal = ret.Copy();
    return S_OK;
}



