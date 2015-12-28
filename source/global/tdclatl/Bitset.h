// Bitset.h : CBitset �̐錾

#pragma once
#include "resource.h"       // ���C�� �V���{��



#include "tdclatl_i.h"
#include <bzs/db/protocol/tdap/client/table.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM �̊��S�T�|�[�g���܂�ł��Ȃ� Windows Mobile �v���b�g�t�H�[���̂悤�� Windows CE �v���b�g�t�H�[���ł́A�P��X���b�h COM �I�u�W�F�N�g�͐������T�|�[�g����Ă��܂���BATL ���P��X���b�h COM �I�u�W�F�N�g�̍쐬���T�|�[�g���邱�ƁA����т��̒P��X���b�h COM �I�u�W�F�N�g�̎����̎g�p�������邱�Ƃ���������ɂ́A_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ���`���Ă��������B���g�p�� rgs �t�@�C���̃X���b�h ���f���� 'Free' �ɐݒ肳��Ă���ADCOM Windows CE �ȊO�̃v���b�g�t�H�[���ŃT�|�[�g�����B��̃X���b�h ���f���Ɛݒ肳��Ă��܂����B"
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
