
#pragma once
#include "resource.h"       // ���C�� �V���{��



#include "tdclatl_i.h"
#include <bzs/db/protocol/tdap/client/haNameResolver.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM �̊��S�T�|�[�g���܂�ł��Ȃ� Windows Mobile �v���b�g�t�H�[���̂悤�� Windows CE �v���b�g�t�H�[���ł́A�P��X���b�h COM �I�u�W�F�N�g�͐������T�|�[�g����Ă��܂���BATL ���P��X���b�h COM �I�u�W�F�N�g�̍쐬���T�|�[�g���邱�ƁA����т��̒P��X���b�h COM �I�u�W�F�N�g�̎����̎g�p�������邱�Ƃ���������ɂ́A_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ���`���Ă��������B���g�p�� rgs �t�@�C���̃X���b�h ���f���� 'Free' �ɐݒ肳��Ă���ADCOM Windows CE �ȊO�̃v���b�g�t�H�[���ŃT�|�[�g�����B��̃X���b�h ���f���Ɛݒ肳��Ă��܂����B"
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
