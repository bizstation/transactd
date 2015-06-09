#pragma once
/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

template <class T>
class CProxy_IDatabaseEvents
    : public ATL::IConnectionPointImpl<T, &__uuidof(_IDatabaseEvents)>
{
public:
    HRESULT Fire_OnCopyData(IDatabase* db, int recordCount, int count,
                            VARIANT_BOOL* cancel)
    {
        HRESULT hr = S_OK;
        T* pThis = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pThis->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pThis->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant avarParams[4];
                avarParams[3] = db;
                avarParams[2] = recordCount;
                avarParams[1] = count;
                avarParams[0].byref = cancel;
                avarParams[0].vt = VT_BOOL | VT_BYREF;
                CComVariant varResult;

                DISPPARAMS params = { avarParams, NULL, 4, 0 };
                hr = pConnection->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT,
                                         DISPATCH_METHOD, &params, &varResult,
                                         NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnDeleteRecord(IDatabase* db, ITable* tb, VARIANT_BOOL* Value)
    {
        HRESULT hr = S_OK;
        T* pThis = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pThis->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pThis->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant avarParams[3];
                avarParams[2] = db;
                avarParams[1] = tb;
                avarParams[0].byref = Value;
                avarParams[0].vt = VT_BOOL | VT_BYREF;
                CComVariant varResult;

                DISPPARAMS params = { avarParams, NULL, 3, 0 };
                hr = pConnection->Invoke(2, IID_NULL, LOCALE_USER_DEFAULT,
                                         DISPATCH_METHOD, &params, &varResult,
                                         NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnSchemaMgrFn(IDatabase* db, short* Value)
    {
        HRESULT hr = S_OK;
        T* pThis = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pThis->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pThis->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant avarParams[2];
                avarParams[1] = db;
                avarParams[0].byref = Value;
                avarParams[0].vt = VT_I2 | VT_BYREF;
                CComVariant varResult;

                DISPPARAMS params = { avarParams, NULL, 2, 0 };
                hr = pConnection->Invoke(3, IID_NULL, LOCALE_USER_DEFAULT,
                                         DISPATCH_METHOD, &params, &varResult,
                                         NULL, NULL);
            }
        }
        return hr;
    }
};


template <class T>
class CProxy_ITableEvents
    : public ATL::IConnectionPointImpl<T, &__uuidof(_ITableEvents)>
{
public:
    HRESULT Fire_OnRecordCount(ITable* tb, int count, VARIANT_BOOL* cancel)
    {
        HRESULT hr = S_OK;
        T* pThis = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pThis->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pThis->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant avarParams[3];
                avarParams[2] = tb;
                avarParams[1] = count;
                avarParams[0].byref = cancel;
                avarParams[0].vt = VT_BOOL | VT_BYREF;
                CComVariant varResult;

                DISPPARAMS params = { avarParams, NULL, 3, 0 };
                hr = pConnection->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT,
                                         DISPATCH_METHOD, &params, &varResult,
                                         NULL, NULL);
            }
        }
        return hr;
    }
};
