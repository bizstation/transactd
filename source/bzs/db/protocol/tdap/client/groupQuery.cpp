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
#pragma hdrstop
#include "groupQuery.h"
#include "memRecordset.h"


#pragma package(smart_init)

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

// ---------------------------------------------------------------------------
// class recordsetQuery
// ---------------------------------------------------------------------------

void recordsetQuery::init(const fielddefs* fdinfo)
{
	const std::vector<std::_tstring>& tokns = getWheres();
	short pos = 0;
	for (int i=0;i<(int)tokns.size();i+=4)
	{
		int index = fdinfo->indexByName(tokns[i]);
		m_indexes.push_back(index);
		m_compFields.push_back(&((*fdinfo)[index]), true/*rePosition*/);
	}
	memoryRecord* mr = memoryRecord::create(m_compFields);
	mr->setRecordData(0, 0, &m_endIndex, true);
	m_row.reset(mr);


	int index = 0;
	for (int i=0;i<(int)tokns.size();i+=4)
	{
		field fd = (*m_row)[index];
		fd = tokns[i+2].c_str();
		bool part = fd.isCompPartAndMakeValue();
		unsigned char ct = getFilterLogicTypeCode(tokns[i+1].c_str());
		if (!part) ct |= CMPLOGICAL_VAR_COMP_ALL;

		m_compType.push_back(ct);
		if (i+3 < (int)tokns.size())
		{
			std::_tstring s = tokns[i+3];
			if (s == _T("or"))
				m_combine.push_back(eCor);
			else if (s == _T("and"))
				m_combine.push_back(eCand);
		}else
			m_combine.push_back(eCend);
		++index;
	}

}

bool recordsetQuery::isMatch(int ret, unsigned char compType) const
{
	compType &= 0xf;// lower then 15
	switch ((eCompType)compType)
	{
	case eEqual: return (ret == 0);
	case eGreaterEq: return (ret >= 0);
	case eLessEq: return (ret <= 0);
	case eGreater: return (ret > 0);
	case eLess: return (ret < 0);
	case eNotEq: return (ret != 0);
	}
	return false;
}

bool recordsetQuery::match(const row_ptr row) const
{
	for (int i=0;i<(int)m_indexes.size();++i)
	{
		short index =  m_indexes[i];
		bool ret = isMatch((*row)[index].comp((*m_row)[i]), m_compType[i]);
		if (m_combine[i] == eCend) return ret;
		if (ret && m_combine[i] == eCor) return true;
		if (!ret && m_combine[i] == eCand) return false;

	}
	assert(0);
	return false;
}


// ---------------------------------------------------------------------------
// class groupFuncBase
// ---------------------------------------------------------------------------
void groupFuncBase::initResultVariable(int index)
{
	std::vector<value_type>::iterator it = m_values.begin();
	if (index)
		it += index;
	m_values.insert(it, 0.0f);
}

void groupFuncBase::init(const fielddefs* fdinfo)
{
	if (m_query)
		m_query->init(fdinfo);
	m_targetKey = m_targetName ? fdinfo->indexByName(m_targetName): -1;
	m_resultKey = fdinfo->indexByName(m_resultName);
	if (m_resultKey == -1) m_resultKey = (int)fdinfo->size();
}

groupFuncBase::groupFuncBase(const _TCHAR* targetName , const _TCHAR* resultName
	, recordsetQuery* query)
	:m_targetName(targetName),m_query(query)
{
   m_resultName = ((resultName == NULL) || resultName[0]==0x00)
								 ? targetName:resultName;
   m_values.reserve(10);
}

groupFuncBase::~groupFuncBase(){};

void groupFuncBase::reset(){m_values.clear();};

void groupFuncBase::operator()(const row_ptr& row, int index, bool insert)
{
	if (insert)
		initResultVariable(index);// setNullValue
	if ((!m_query || m_query->match(row)))
		doCalc(row, index);
}

groupFuncBase::value_type groupFuncBase::result(int groupIndex) const
{
	return m_values[groupIndex];
}

inline void setValue(row_ptr& row, int key, double value)
{
	(*row)[key] = value;
}

// ---------------------------------------------------------------------------
// class groupQuery
// ---------------------------------------------------------------------------

/* remove none grouping fields */
//template <class Container>
void groupQuery::removeFields(Container& mdls)
{
	const fielddefs& fds = *mdls.fieldDefs();
	for (int i=(int)fds.size()-1;i>=0;--i)
	{
		bool enabled = false;
		for (int j=0;j<(int)m_keyFields.size();++j)
		{
			if (m_keyFields[j] == fds[i].name())
			{
				enabled = true;
				break;
			}
		}
		if (!enabled)
		{
			for (int j=0;j<(int)m_funcs.size();++j)
			{
				if (!enabled && (m_funcs[j]->resultKey() == i))
				{
					enabled = true;
					break;
				}
			}
		}

		if (!enabled)
			mdls.removeField(i);
	}
}

fieldNames& groupQuery::reset()
{
	fieldNames::reset();
	m_funcs.clear();
	return *this;
}

groupQuery& groupQuery::addFunction(groupFuncBase* func)
{
	m_funcs.push_back(func);
	return *this;
}

void groupQuery::grouping(Container& mdls)
{
	std::vector<Container::key_type> keyFields;

	/* convert key value from field name */
	for (int i=0;i<(int)m_keyFields.size();++i)
		keyFields.push_back(resolvKeyValue(mdls, m_keyFields[i]));

	for (int i=0;i<(int)m_funcs.size();++i)
	{

		groupFuncBase* f = m_funcs[i];
		f->init(mdls.fieldDefs());

		if (f->resultKey() == (int)mdls.fieldDefs()->size())
		{
			groupFuncBase::value_type dummy=0;
			mdls.appendCol(f->resultName(), getFieldType(dummy), sizeof(dummy));
		}
	}

	grouping_comp<Container> groupingComp(mdls, keyFields);
	std::vector<int> index;
	Container::iterator it = begin(mdls), ite = end(mdls);

	int i,n = 0;
	while(it != ite)
	{
		bool found = false;
		i = binary_search(n, index, 0, (int)index.size(), groupingComp, found);
		if (!found)
			index.insert(index.begin() + i, n);
		for (int j=0;j<(int)m_funcs.size();++j)
			(*m_funcs[j])(*it, i, !found);
		++n;
		++it;
	}

	//real sort by index
	Container c(mdls);

	clear(mdls);
	for (int i=0;i<(int)index.size();++i)
	{
		Container::row_type cur = c.getRow(i);//[index[i]];
		for (int j=0;j<(int)m_funcs.size();++j)
			setValue(cur, m_funcs[j]->resultKey(), m_funcs[j]->result(i));
		mdls.push_back(cur);
	}
	removeFields(mdls);
}


inline __int64 fieldValue(const field& fd, __int64 ) {return fd.i64();}
inline int fieldValue(const field& fd, int ) {return fd.i();}
inline short fieldValue(const field& fd, short ) {return (short)fd.i();}
inline char fieldValue(const field& fd, char ) {return (char)fd.i();}
inline double fieldValue(const field& fd, double ) {return fd.d();}
inline float fieldValue(const field& fd, float ) {return fd.f();}
inline const _TCHAR* fieldValue(const field& fd, const _TCHAR* ) {return fd.c_str();}

// ---------------------------------------------------------------------------
// class sum
// ---------------------------------------------------------------------------
void sum::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	m_values[index] += fieldValue((*row)[m_targetKey], tmp);
}

sum::sum(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:groupFuncBase(targetName, resultName, query){}


// ---------------------------------------------------------------------------
// class count
// ---------------------------------------------------------------------------
void count::doCalc(const row_ptr& row, int index)
{
	m_values[index] = m_values[index] + 1;
}

count::value_type count::result(int index)const{return m_values[index];}

count::count(const _TCHAR* resultName, recordsetQuery* query)
	:groupFuncBase(NULL, resultName, query){}


// ---------------------------------------------------------------------------
// class avg
// ---------------------------------------------------------------------------
void avg::initResultVariable(int index)
{
	sum::initResultVariable(index);
	m_count.insert(m_count.begin() + index, 0);

}

void avg::doCalc(const row_ptr& row, int index)
{
	sum::doCalc(row, index);
	m_count[index] = m_count[index] + 1;
}

avg::value_type avg::result(int index)const
{
	return m_values[index]/m_count[index];
}

avg::avg(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName){}

// ---------------------------------------------------------------------------
// class min
// ---------------------------------------------------------------------------
void min::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	tmp = fieldValue((*row)[m_targetKey], tmp);
	if (m_flag || m_values[index]  > tmp)
	{
		m_flag = false;
		m_values[index]  = tmp;
	}
}

min::min(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName, query),m_flag(true){}

// ---------------------------------------------------------------------------
// class max
// ---------------------------------------------------------------------------
void max::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	tmp = fieldValue((*row)[m_targetKey], tmp);
	if (m_flag || m_values[index]  < tmp)
	{
		m_flag = false;
		m_values[index]  = tmp;
	}
}

max::max(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName, query),m_flag(true){}



}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs



