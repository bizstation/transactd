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
#include "PooledDbManager.h"
#include "ConnectParams.h"
#include "Table.h"
#include "Database.h"

using namespace bzs::db::protocol::tdap::client;

STDMETHODIMP CPooledDbManager::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IPooledDbManager
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CPooledDbManager::put_MaxConnections(int n)
{
    pooledDbManager::setMaxConnections(n);
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_MaxConnections(int* retVal)
{
    *retVal = pooledDbManager::maxConnections();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::Reserve(int size, IConnectParams* Uri)
{
    connectParams* p = NULL;
    CConnectParams* cp = dynamic_cast<CConnectParams*>(Uri);
    if (cp)
        p = cp->internalConnectParams();
    if (p)
    {
        pooledDbManager::reserve(size, *p);
        return S_OK;
    }
    return Error(_T("Invalid ConnectParams."), IID_IPooledDbManager);
}

STDMETHODIMP CPooledDbManager::Reset(int waitSec)
{
    try
    {
        m_mgr.reset(waitSec);
        return S_OK;
    }

    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IPooledDbManager);
    }
}

STDMETHODIMP CPooledDbManager::get_Stat(eStatus* Value)
{
    *Value = (eStatus)m_mgr.stat();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::BeginTrn(eLockType Bias)
{
    m_mgr.beginTrn(Bias);
    return S_OK;
}

STDMETHODIMP CPooledDbManager::EndTrn(void)
{
    m_mgr.endTrn();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::AbortTrn(void)
{
    m_mgr.abortTrn();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::BeginSnapshot(eStLockType bias)
{
    m_mgr.beginSnapshot(bias);
    return S_OK;
}

STDMETHODIMP CPooledDbManager::EndSnapshot(void)
{
    m_mgr.endSnapshot();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::Use(VARIANT Uri)
{
    try
    {
        connectParams* p = NULL;
        if (Uri.vt == VT_DISPATCH)
        {
            CConnectParams* cp = dynamic_cast<CConnectParams*>(Uri.pdispVal);
            if (cp)
                p = cp->internalConnectParams();
        }
        m_mgr.use(p);
        return S_OK;
    }

    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IPooledDbManager);
    }
}

STDMETHODIMP CPooledDbManager::Unuse()
{
    try
    {
        m_mgr.unUse();
        return S_OK;
    }

    catch (bzs::rtl::exception& e)
    {
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IPooledDbManager);
    }
}

STDMETHODIMP CPooledDbManager::get_EnableTrn(VARIANT_BOOL* Value)
{
    *Value = m_mgr.enableTrn();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_Uri(BSTR* Uri)
{
    CComBSTR ret;
    ret = m_mgr.uri();
    *Uri = ret.Copy();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_IsOpened(VARIANT_BOOL* Value)
{
    *Value = m_mgr.isOpened();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_ClientID(short* Value)
{
    *Value = *m_mgr.clientID();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::Table(BSTR name, ITable** retVal)
{
    try
    {
        CComObject<CTableTd>* ptb;
        CComObject<CTableTd>::CreateInstance(&ptb);

        if (ptb)
        {
            ptb->m_tb = m_mgr.table(name);
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
        return Error((*bzs::rtl::getMsg(e)).c_str(), IID_IPooledDbManager);
    }
}

STDMETHODIMP CPooledDbManager::Db(IDatabase** retval)
{
    CComObject<CDatabase>* ptb;
    CComObject<CDatabase>::CreateInstance(&ptb);
    HRESULT ret = ptb->AtatchDatabase((__int64*)m_mgr.db(), -1);
    if (ret == S_OK)
    {
        IDatabase* dbPtr;
        ptb->QueryInterface(IID_IDatabase, (void**)&dbPtr);
        _ASSERTE(dbPtr);
        *retval = dbPtr;
        return S_OK;
    }
    return ret;
}

STDMETHODIMP CPooledDbManager::put_Option(__int64 Value)
{
    m_mgr.setOption(Value);
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_Option(__int64* Value)
{
    *Value = m_mgr.option();
    return S_OK;
}

STDMETHODIMP CPooledDbManager::get_Mode(short* Value)
{
    *Value = m_mgr.mode();
    return S_OK;
}