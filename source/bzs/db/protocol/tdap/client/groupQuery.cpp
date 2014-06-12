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
#include "recordsetImple.h"
#include "fieldNames.h"

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


struct recordsetQueryImple
{
	row_ptr row;
	std::vector<unsigned char> compType;
	std::vector<short> indexes;
	std::vector<char> combine;
	short endIndex;
	fielddefs compFields;
	
};

// ---------------------------------------------------------------------------
// class recordsetQuery
// ---------------------------------------------------------------------------
recordsetQuery::recordsetQuery():query(),m_imple(new recordsetQueryImple){}

recordsetQuery::~recordsetQuery()
{
	delete m_imple;
}

void recordsetQuery::init(const fielddefs* fdinfo)
{
	const std::vector<std::_tstring>& tokns = getWheres();
	short pos = 0;
	for (int i=0;i<(int)tokns.size();i+=4)
	{
		int index = fdinfo->indexByName(tokns[i].c_str());
		m_imple->indexes.push_back(index);
		m_imple->compFields.push_back(&((*fdinfo)[index]), true/*rePosition*/);
	}
	memoryRecord* mr = memoryRecord::create(m_imple->compFields);
	mr->setRecordData(0, 0, &m_imple->endIndex, true);
	m_imple->row.reset(mr);


	int index = 0;
	for (int i=0;i<(int)tokns.size();i+=4)
	{
		field fd = (*m_imple->row)[index];
		fd = tokns[i+2].c_str();
		bool part = fd.isCompPartAndMakeValue();
		unsigned char ct = getFilterLogicTypeCode(tokns[i+1].c_str());
		if (!part) ct |= CMPLOGICAL_VAR_COMP_ALL;

		m_imple->compType.push_back(ct);
		if (i+3 < (int)tokns.size())
		{
			std::_tstring s = tokns[i+3];
			if (s == _T("or"))
				m_imple->combine.push_back(eCor);
			else if (s == _T("and"))
				m_imple->combine.push_back(eCand);
		}else
			m_imple->combine.push_back(eCend);
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
	for (int i=0;i<(int)m_imple->indexes.size();++i)
	{
		short index =  m_imple->indexes[i];
		bool ret = isMatch((*row)[index].comp((*m_imple->row)[i]), m_imple->compType[i]);
		if (m_imple->combine[i] == eCend) return ret;
		if (ret && m_imple->combine[i] == eCor) return true;
		if (!ret && m_imple->combine[i] == eCand) return false;

	}
	assert(0);
	return false;
}

inline void setValue(row_ptr& row, int key, double value)
{
	(*row)[key] = value;
}

// ---------------------------------------------------------------------------
// class groupQueryImple
// ---------------------------------------------------------------------------
class groupQueryImple : public fieldNames
{
	std::vector<groupFuncBase* > m_funcs;

	void removeFields(recordsetImple& mdls)
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

public:
	fieldNames& reset()
	{
		m_funcs.clear();
		return fieldNames::reset();
 	}

	void addFunction(groupFuncBase* func)
	{
		m_funcs.push_back(func);
	}

	void grouping(recordsetImple& mdls)
	{
		std::vector<recordsetImple::key_type> keyFields;

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

		grouping_comp<recordsetImple> groupingComp(mdls, keyFields);
		std::vector<int> index;
		recordsetImple::iterator it = begin(mdls), ite = end(mdls);

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
		recordsetImple c(mdls);

		clear(mdls);
		for (int i=0;i<(int)index.size();++i)
		{
			recordsetImple::row_type cur = c.getRow(index[i]);
			for (int j=0;j<(int)m_funcs.size();++j)
				setValue(cur, m_funcs[j]->resultKey(), m_funcs[j]->result(i));
			mdls.push_back(cur);
		}
		removeFields(mdls);
	}
};

// ---------------------------------------------------------------------------
// class groupQuery
// ---------------------------------------------------------------------------
groupQuery::groupQuery():m_imple(new groupQueryImple)
{

}

groupQuery::~groupQuery()
{
	delete m_imple; 
}

groupQuery& groupQuery::reset() 
{
	m_imple->reset();
	return *this;
}

groupQuery& groupQuery::addFunction(groupFuncBase* func)
{
	m_imple->addFunction(func);
	return *this;
}

void groupQuery::grouping(recordsetImple& rs)
{
	m_imple->grouping(rs);
}

groupQuery& groupQuery::keyField(const _TCHAR* name, const _TCHAR* name1, const _TCHAR* name2, const _TCHAR* name3
			,const _TCHAR* name4, const _TCHAR* name5, const _TCHAR* name6, const _TCHAR* name7
			,const _TCHAR* name8, const _TCHAR* name9, const _TCHAR* name10)
{
	m_imple->keyField(name, name1, name2, name3, name4, name5, name6, name7, name8
			, name9, name10);
	return *this;
}

const std::vector<std::_tstring>& groupQuery::getKeyFields()const
{
	return m_imple->getKeyFields();
}



// ---------------------------------------------------------------------------
// class groupFuncBaseImple
// ---------------------------------------------------------------------------
class groupFuncBaseImple
{
public:
	typedef double value_type;

private:
	friend class groupQueryImple;
	const _TCHAR* m_targetName;
	const _TCHAR* m_resultName;
	int m_resultKey;
	int m_targetKey;

public:
	std::vector<value_type> m_values;
	std::vector<__int64> m_counts;
	recordsetQuery* m_query;

	inline void initResultVariable(int index)
	{
		std::vector<value_type>::iterator it = m_values.begin(); 
		if (index)
			it += index;
		m_values.insert(it, 0.0f);
	}

	inline void init(const fielddefs* fdinfo)
	{
		if (m_query)
			m_query->init(fdinfo);
		m_targetKey = m_targetName ? fdinfo->indexByName(m_targetName): -1;
		m_resultKey = fdinfo->indexByName(m_resultName);
		if (m_resultKey == -1) m_resultKey = (int)fdinfo->size();
	}
	
	inline groupFuncBaseImple(const _TCHAR* targetName , const _TCHAR* resultName=NULL
		, recordsetQuery* query=NULL)
		:m_targetName(targetName),m_query(query)
	{
	   m_resultName = ((resultName == NULL) || resultName[0]==0x00)
									 ? targetName:resultName;
	   m_values.reserve(10);
	}

	inline void setQuery(recordsetQuery* query)
	{
		m_query = query;
	}

	inline const _TCHAR* targetName() const {return m_targetName;}

	inline const _TCHAR* resultName() const {return m_resultName;}

	inline int resultKey() const {return m_resultKey;}

	inline void reset(){m_values.clear();};

	inline value_type result(int groupIndex)const{return m_values[groupIndex];};

	inline bool match(const row_ptr row) const 
	{
		if (m_query)
			return m_query->match(row);
		return false;
	}

	inline int targetKey()const {return m_targetKey;}
};

// ---------------------------------------------------------------------------
// class groupFuncBase
// ---------------------------------------------------------------------------
groupFuncBase::groupFuncBase(const _TCHAR* targetName , const _TCHAR* resultName
		, recordsetQuery* query)
		:m_imple(new groupFuncBaseImple(targetName, resultName, query)){}

groupFuncBase::~groupFuncBase()
{
	delete m_imple;
}

void groupFuncBase::initResultVariable(int index)
{
	m_imple->initResultVariable(index);
}

void groupFuncBase::doCalc(const row_ptr& row, int groupIndex){}

void groupFuncBase::init(const fielddefs* fdinfo)
{
	m_imple->init(fdinfo);
}

groupFuncBase& groupFuncBase::setQuery(recordsetQuery* query)
{
	m_imple->setQuery(query);
	return *this;
}

const _TCHAR* groupFuncBase::targetName() const
{
	return m_imple->targetName();
}

const _TCHAR* groupFuncBase::resultName() const
{
	return m_imple->resultName();
}

int groupFuncBase::resultKey() const 
{
	return m_imple->resultKey();
}

void groupFuncBase::reset()
{
	m_imple->reset();
}

void groupFuncBase::operator()(const row_ptr& row, int index, bool insert)
{
	if (insert)
		initResultVariable(index);// setNullValue
	if (m_imple->match(row))
		doCalc(row, index);
}

groupFuncBase::value_type groupFuncBase::result(int groupIndex) const
{
	return m_imple->result(groupIndex);
}


// ---------------------------------------------------------------------------
// class groupFuncBase
// ---------------------------------------------------------------------------

sum::sum(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
		:groupFuncBase(targetName, resultName, query){}

void sum::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	m_imple->m_values[index] += fieldValue((*row)[m_imple->targetKey()], tmp);
}

// ---------------------------------------------------------------------------
// class count
// ---------------------------------------------------------------------------
count::count(const _TCHAR* resultName, recordsetQuery* query)
	:groupFuncBase(NULL, resultName, query){}

void count::doCalc(const row_ptr& row, int index)
{
	m_imple->m_values[index] = m_imple->m_values[index] + 1;
}


// ---------------------------------------------------------------------------
// class avg
// ---------------------------------------------------------------------------
avg::avg(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName){}

void avg::initResultVariable(int index)
{
	sum::initResultVariable(index);
	m_imple->m_counts.insert(m_imple->m_counts.begin() + index, 0);

}

void avg::doCalc(const row_ptr& row, int index)
{
	sum::doCalc(row, index);
	m_imple->m_counts[index] = m_imple->m_counts[index] + 1;
}

avg::value_type avg::result(int index)const
{
	return m_imple->m_values[index]/m_imple->m_counts[index];
}


// ---------------------------------------------------------------------------
// class min
// ---------------------------------------------------------------------------
min::min(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName, query),m_flag(true){}

void min::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	tmp = fieldValue((*row)[m_imple->targetKey()], tmp);
	if (m_flag || m_imple->m_values[index]  > tmp)
	{
		m_flag = false;
		m_imple->m_values[index]  = tmp;
	}
}

// ---------------------------------------------------------------------------
// class max
// ---------------------------------------------------------------------------
max::max(const _TCHAR* targetName , const _TCHAR* resultName, recordsetQuery* query)
	:sum(targetName, resultName, query),m_flag(true){}

void max::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	tmp = fieldValue((*row)[m_imple->targetKey()], tmp);
	if (m_flag || m_imple->m_values[index]  < tmp)
	{
		m_flag = false;
		m_imple->m_values[index]  = tmp;
	}
}



}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs



