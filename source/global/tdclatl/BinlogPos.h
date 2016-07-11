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
// BinlogPos.h : CBinlogPos �̐錾

#pragma once
#include "resource.h"       // ���C�� �V���{��



#include "tdclatl_i.h"
#include <bzs/db/protocol/tdap/client/nsdatabase.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM �̊��S�T�|�[�g���܂�ł��Ȃ� Windows Mobile �v���b�g�t�H�[���̂悤�� Windows CE �v���b�g�t�H�[���ł́A�P��X���b�h COM �I�u�W�F�N�g�͐������T�|�[�g����Ă��܂���BATL ���P��X���b�h COM �I�u�W�F�N�g�̍쐬���T�|�[�g���邱�ƁA����т��̒P��X���b�h COM �I�u�W�F�N�g�̎����̎g�p�������邱�Ƃ���������ɂ́A_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ���`���Ă��������B���g�p�� rgs �t�@�C���̃X���b�h ���f���� 'Free' �ɐݒ肳��Ă���ADCOM Windows CE �ȊO�̃v���b�g�t�H�[���ŃT�|�[�g�����B��̃X���b�h ���f���Ɛݒ肳��Ă��܂����B"
#endif

using namespace ATL;


// CBitset

class ATL_NO_VTABLE CBinlogPos :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CBinlogPos, &CLSID_BinlogPos>,
  public IDispatchImpl<IBinlogPos, &IID_IBinlogPos, &LIBID_transactd, /*wMajor =*/ 1, /*wMinor =*/ 0>
{

public:
    bzs::db::protocol::tdap::client::binlogPos m_pos;

  CBinlogPos()
  {
  }

DECLARE_REGISTRY_RESOURCEID(IDR_BITSET)


BEGIN_COM_MAP(CBinlogPos)
  COM_INTERFACE_ENTRY(IBinlogPos)
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
#ifdef _WIN64
    STDMETHOD(get_Pos)(__int64* retVal);
#else
    STDMETHOD(get_Pos)(int* retVal);
#endif
    STDMETHOD(get_Type)(short* retVal);
    STDMETHOD(get_Filename)(BSTR* retVal);
    STDMETHOD(get_Gtid)(BSTR* retVal);
    STDMETHOD(put_Gtid)(BSTR Value);
};

//OBJECT_ENTRY_AUTO(__uuidof(Bitset), CBitset)
