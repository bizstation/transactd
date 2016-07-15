
#pragma once
#include "resource.h"       // メイン シンボル



#include "tdclatl_i.h"
#include <bzs/db/protocol/tdap/client/haNameResolver.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM の完全サポートを含んでいない Windows Mobile プラットフォームのような Windows CE プラットフォームでは、単一スレッド COM オブジェクトは正しくサポートされていません。ATL が単一スレッド COM オブジェクトの作成をサポートすること、およびその単一スレッド COM オブジェクトの実装の使用を許可することを強制するには、_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA を定義してください。ご使用の rgs ファイルのスレッド モデルは 'Free' に設定されており、DCOM Windows CE 以外のプラットフォームでサポートされる唯一のスレッド モデルと設定されていました。"
#endif

using namespace ATL;


// CHaNameResolver

class ATL_NO_VTABLE CHaNameResolver :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHaNameResolver, &CLSID_HaNameResolver>,
	public IDispatchImpl<IHaNameResolver, &IID_IHaNameResolver, &LIBID_transactd, /*wMajor =*/ 1, /*wMinor =*/ 0>
{

public:


	CHaNameResolver()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_HANAMERESOLVER)


BEGIN_COM_MAP(CHaNameResolver)
	COM_INTERFACE_ENTRY(IHaNameResolver)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
    STDMETHOD(Start)(BSTR master, BSTR slaves,
                   BSTR slaveHostsWithPort, short slaveNum,
                   BSTR userName,
                   BSTR password, 
                   int option, int* retVal);
    STDMETHOD(AddPortMap)(short mysqlPort, short transactdPort);
    STDMETHOD(ClearPortMap)();
    STDMETHOD(Stop)();
    STDMETHOD(get_Master)(BSTR* retVal);
    STDMETHOD(get_Slave)(BSTR* retVal);  
    
};

OBJECT_ENTRY_AUTO(__uuidof(HaNameResolver), CHaNameResolver)
