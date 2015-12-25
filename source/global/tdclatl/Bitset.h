// Bitset.h : CBitset の宣言

#pragma once
#include "resource.h"       // メイン シンボル



#include "tdclatl_i.h"
#include <bzs/db/protocol/tdap/client/table.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM の完全サポートを含んでいない Windows Mobile プラットフォームのような Windows CE プラットフォームでは、単一スレッド COM オブジェクトは正しくサポートされていません。ATL が単一スレッド COM オブジェクトの作成をサポートすること、およびその単一スレッド COM オブジェクトの実装の使用を許可することを強制するには、_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA を定義してください。ご使用の rgs ファイルのスレッド モデルは 'Free' に設定されており、DCOM Windows CE 以外のプラットフォームでサポートされる唯一のスレッド モデルと設定されていました。"
#endif

using namespace ATL;


// CBitset

class ATL_NO_VTABLE CBitset :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBitset, &CLSID_Bitset>,
	public IDispatchImpl<IBitset, &IID_IBitset, &LIBID_transactd, /*wMajor =*/ 1, /*wMinor =*/ 0>
{

public:
    bzs::db::protocol::tdap::bitset m_bitset;

	CBitset()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BITSET)


BEGIN_COM_MAP(CBitset)
	COM_INTERFACE_ENTRY(IBitset)
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

    STDMETHOD(get_Bit)(short Index, VARIANT_BOOL* retVal);
    STDMETHOD(put_Bit)(short Index, VARIANT_BOOL Value);
    STDMETHOD(Equals)(IBitset* bitset, VARIANT_BOOL* retVal);
    STDMETHOD(Contains)(IBitset* bitset, VARIANT_BOOL Value, VARIANT_BOOL* retVal);
};

OBJECT_ENTRY_AUTO(__uuidof(Bitset), CBitset)
