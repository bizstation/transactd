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
#include "GroupQuery.h"
#include "QueryBase.h"
#include "RecordsetQuery.h"

using namespace bzs::db::protocol::tdap::client;

void CGroupQuery::setResult(IGroupQuery** retVal)
{
	this->QueryInterface(IID_IGroupQuery, (void**)retVal);
}

STDMETHODIMP CGroupQuery::KeyField(BSTR Name0, BSTR Name1, BSTR Name2, BSTR Name3, BSTR Name4, BSTR Name5,
				BSTR Name6, BSTR Name7,	BSTR Name8, BSTR Name9,	BSTR Name10,
				IGroupQuery** retVal)
{
	if (Name0 && Name0[0])
	{
		if (!Name1 || !Name1[0])
			m_gq.keyField(Name0);	
		else if (!Name2 || !Name2[0]) 
			m_gq.keyField(Name0, Name1);	
		else if (!Name3 || !Name3[0]) 
			m_gq.keyField(Name0,  Name1, Name2);	
		else if (!Name4 || !Name4[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3);	
		else if (!Name5 || !Name5[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4);	
		else if (!Name6 || !Name6[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5);	
		else if (!Name7 || !Name7[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6);
		else if (!Name8 || !Name8[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7);
		else if (!Name9 || !Name9[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8);
		else if (!Name10 || !Name10[0]) 
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8, Name9);
		else
			m_gq.keyField(Name0,  Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8, Name9, Name10);
	}
	setResult(retVal);
	return S_OK;
}
 
STDMETHODIMP CGroupQuery::AddFunction(eGroupFunc func, BSTR targetName , BSTR resultName, IRecordsetQuery* iq, IGroupQuery** retVal)
{
	
	boost::shared_ptr<groupFuncBase> f;

	if (func == fsum)
		f.reset(new sum(targetName, resultName));  
	else if (func == fmin)
		f.reset(new min(targetName, resultName));  
	else if (func == fmax)
		f.reset(new max(targetName, resultName));  
	else if (func == favg)
		f.reset(new avg(targetName, resultName));  
	else if(func == fcount)
		f.reset(new count(resultName));
	if (iq)
	{
		CRecordsetQuery* q = dynamic_cast<CRecordsetQuery*>(iq);
		if (q)
			*f = (q->m_qb);
	}
	else
		f.reset();
	m_funcs.push_back(f);
	m_gq.addFunction(f.get());

	setResult(retVal);
	return S_OK;

}


STDMETHODIMP CGroupQuery::Reset(IGroupQuery** retVal)
{
	m_gq.reset();
	setResult(retVal);
	return S_OK;
}

