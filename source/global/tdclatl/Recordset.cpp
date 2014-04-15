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
#include "Recordset.h"
#include "Record.h"
#include "Field.h"
#include "GroupQuery.h"

CARecordset::CARecordset():m_rs(new recordset()),m_recObj(NULL)
{


}

CARecordset::~CARecordset() 
{
	
	delete m_rs;

}

void CARecordset::FinalRelease() 
{
	if (m_recObj)
		;//m_recObj->Release();
	//delete m_rs;

}

void CARecordset::setResult(IRecordset** retVal)
{
	this->QueryInterface(IID_IRecordset, (void**)retVal);
}

/*STDMETHODIMP CARecordset::Clear(void)
{
	return S_OK;
}*/

STDMETHODIMP CARecordset::Record(short Index, IRecord** retVal)
{
    if (Index >= 0 && Index < m_rs->size())
	{
		if (m_recObj == NULL)
		{
			CComObject<CRecord>::CreateInstance(&m_recObj);
			if (m_recObj)
				;//m_recObj->AddRef();
		}
		if (m_recObj)
		{
			m_recObj->m_rec = (*m_rs)[Index].get(); 
			IRecord* rec;
			m_recObj->QueryInterface(IID_IRecord, (void**)&rec);
			_ASSERTE(rec);
			*retVal = rec;
			
		}
		return S_OK;
	}
	return Error("Invalid index", IID_IRecordset);	
}

STDMETHODIMP CARecordset::First(IRecord** retVal)
{
	return Record(0, retVal);
}

STDMETHODIMP CARecordset::Last(IRecord** retVal)
{
	return Record((short)m_rs->size()-1, retVal);
}

STDMETHODIMP CARecordset::Top(long Num, IRecordset** retVal)
{
	if (Num > 0 && Num < m_rs->size())
	{
		CComObject<CARecordset>* rsObj;
		CComObject<CARecordset>::CreateInstance(&rsObj);
		if (rsObj)
		{
			m_rs->top(*rsObj->m_rs, Num);
			IRecordset* rs;
			rsObj->QueryInterface(IID_IRecordset, (void**)&rs);
			_ASSERTE(rs);
			*retVal = rs;
		}
		return S_OK;
	}
	return Error("Invalid top number", IID_IRecordset);	
}

STDMETHODIMP CARecordset::Erase(long Index)
{
	if (Index >= 0 && Index < m_rs->size())
	{
		m_rs->erase(Index);
		return S_OK;
	}
	return Error("Invalid index", IID_IRecordset);	
}

STDMETHODIMP CARecordset::Count(long* retVal)
{
	*retVal = (long)m_rs->count();
	return S_OK;
}

STDMETHODIMP CARecordset::RemoveField(short Index, IRecordset** retVal)
{
	if (Index >= 0 && Index < m_rs->fieldInfo()->size())
	{
		m_rs->removeField(Index);
		setResult(retVal);	
		return S_OK;
	}
	return Error("Invalid field index", IID_IRecordset);	
}

STDMETHODIMP CARecordset::GroupBy(IGroupQuery* igq, enum eGroupFunc func, IRecordset** retVal)
{
	if (igq)
	{
		CGroupQuery* gq = dynamic_cast<CGroupQuery*>(igq);
		if (func == fsum)
			m_rs->groupBy(gq->m_gq, sum<row, int, double>());
		else if (func == fmin)
			m_rs->groupBy(gq->m_gq, min<row, int, double>());
		else if (func == fmax)
			m_rs->groupBy(gq->m_gq, max<row, int, double>());
		else if (func == favg)
			m_rs->groupBy(gq->m_gq, avg<row, int, double>());
	}
	setResult(retVal);	
	return S_OK;

}

STDMETHODIMP CARecordset::OrderBy( BSTR Name0,  BSTR Name1,  BSTR Name2,  BSTR Name3,  BSTR Name4, 
					 BSTR Name5,  BSTR Name6,  BSTR Name7,  BSTR Name8
					, IRecordset** retVal)
{
	if (!Name1 || !Name1[0])
		m_rs->orderBy(Name0);	
	else if (!Name2 || !Name2[0]) 
		m_rs->orderBy(Name0, Name1);	
	else if (!Name3 || !Name3[0]) 
		m_rs->orderBy(Name0,  Name1,  Name2);	
	else if (!Name4 || !Name4[0]) 
		m_rs->orderBy(Name0,  Name1,  Name2, Name3);	
	else if (!Name5 || !Name5[0]) 
		m_rs->orderBy(Name0,  Name1,  Name2, Name3, Name4);	
	else if (!Name6 || !Name6[0]) 
		m_rs->orderBy(Name0,  Name1,  Name2, Name3, Name4, Name5);	
	else if (!Name7 || !Name7[0]) 
		m_rs->orderBy(Name0,  Name1,  Name2, Name3, Name4, Name5, Name6);
	else
		m_rs->orderBy(Name0,  Name1,  Name2, Name3, Name4, Name5, Name6, Name7);
	setResult(retVal);
	return S_OK;
}

STDMETHODIMP CARecordset::Reverse(IRecordset** retVal)
{
	m_rs->reverse();
	setResult(retVal);	
	return S_OK;
}
