// HaNameREsolver.cpp : CHaNameREsolver ‚ÌŽÀ‘•

#include "stdafx.h"
#include "HaNameREsolver.h"
#include <comutil.h>
using namespace bzs::db::protocol::tdap::client;

// CHaNameREsolver
STDMETHODIMP CHaNameResolver::Start(BSTR master, BSTR slaves,
                   BSTR slaveHostsWithPort, short slaveNum,
                   BSTR userName,
                   BSTR password,
                   int option, int* retVal)
{
    *retVal = haNameResolver::start(_bstr_t(master), _bstr_t(slaves),
                        _bstr_t(slaveHostsWithPort),slaveNum,
                        _bstr_t(userName),_bstr_t(password), 
                        option);
    return S_OK;
}

STDMETHODIMP CHaNameResolver::AddPortMap(short mysqlPort, short transactdPort)
{
    haNameResolver::addPortMap(mysqlPort, transactdPort);
    return S_OK;
}

STDMETHODIMP CHaNameResolver::ClearPortMap()
{
    haNameResolver::clearPortMap();
    return S_OK;
}

STDMETHODIMP CHaNameResolver::Stop()
{
    haNameResolver::stop();
    return S_OK;
}

STDMETHODIMP CHaNameResolver::get_Master(BSTR* retVal)
{
    CComBSTR ret= haNameResolver::master();
    *retVal = ret.Copy();
    return S_OK;
}

STDMETHODIMP CHaNameResolver::get_Slave(BSTR* retVal) 
{
    CComBSTR ret= haNameResolver::slave();
    *retVal = ret.Copy();
    return S_OK;
}

    
