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
#include <boost/algorithm/string.hpp>

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
// struct fieldNamesImple
// ---------------------------------------------------------------------------
struct fieldNamesImple
{
	std::vector<std::_tstring> keyFields;
	fieldNamesImple(){}
};

// ---------------------------------------------------------------------------
// class fieldNames
// ---------------------------------------------------------------------------

fieldNames* fieldNames::create()
{
	return new fieldNames();
}

void fieldNames::release()
{
	delete this;
}

fieldNames::fieldNames():m_impl(new fieldNamesImple)
{

}

fieldNames::fieldNames(const fieldNames& r):m_impl(new fieldNamesImple(*r.m_impl))
{

}

fieldNames& fieldNames::operator=(const fieldNames& r)
{
	if (this != &r)
	{
		*m_impl = *r.m_impl;
	}
	return *this;
}


fieldNames::~fieldNames()
{
	delete m_impl;
}

fieldNames& fieldNames::reset()
{
	m_impl->keyFields.clear();
	return *this;
}

fieldNames& fieldNames::keyField(const _TCHAR* name, const _TCHAR* name1, const _TCHAR* name2, const _TCHAR* name3
			,const _TCHAR* name4, const _TCHAR* name5, const _TCHAR* name6, const _TCHAR* name7
			,const _TCHAR* name8, const _TCHAR* name9, const _TCHAR* name10)
{
	m_impl->keyFields.clear();
	if (name) m_impl->keyFields.push_back(name);
	if (name1) m_impl->keyFields.push_back(name1);
	if (name2) m_impl->keyFields.push_back(name2);
	if (name3) m_impl->keyFields.push_back(name3);
	if (name4) m_impl->keyFields.push_back(name4);
	if (name5) m_impl->keyFields.push_back(name5);
	if (name6) m_impl->keyFields.push_back(name6);
	if (name7) m_impl->keyFields.push_back(name7);
	if (name8) m_impl->keyFields.push_back(name8);
	if (name9) m_impl->keyFields.push_back(name9);
	if (name10) m_impl->keyFields.push_back(name10);
	return *this;
}

int fieldNames::count() const
{
	return  (int)m_impl->keyFields.size();
}

const _TCHAR* fieldNames::getValue(int index) const
{
	assert(index>=0 && index < count());
	return m_impl->keyFields[index].c_str();
}

const _TCHAR* fieldNames::operator[](int index) const
{
	assert(index>=0 && index < count());
	return m_impl->keyFields[index].c_str();
}

void fieldNames::addValue(const _TCHAR* v)
{
	m_impl->keyFields.push_back(v);
}

void fieldNames::addValues(const _TCHAR* values, const _TCHAR* delmi)
{
	boost::algorithm::split(m_impl->keyFields, values, boost::is_any_of(delmi));
}

// ---------------------------------------------------------------------------
// struct recordsetQueryImple
// ---------------------------------------------------------------------------
struct recordsetQueryImple
{
	row_ptr row;
	std::vector<unsigned char> compType;
	std::vector<short> indexes;
	std::vector<char> combine;
	short endIndex;
	fielddefs compFields;
	recordsetQueryImple(){}
	recordsetQueryImple(const recordsetQueryImple& r)
		:row(r.row),compType(r.compType),indexes(r.indexes),combine(r.combine)
		,endIndex(r.endIndex),compFields(r.compFields){}

};

// ---------------------------------------------------------------------------
// class recordsetQuery
// ---------------------------------------------------------------------------


recordsetQuery* recordsetQuery::create()
{
	return new recordsetQuery();
}

void recordsetQuery::release()
{
 	delete this;
}

recordsetQuery::recordsetQuery():query(),m_imple(new recordsetQueryImple){}

recordsetQuery::recordsetQuery(const recordsetQuery& r)
	:query(r),m_imple(new recordsetQueryImple(*r.m_imple))
{

}

recordsetQuery& recordsetQuery:: operator=(const recordsetQuery& r)
{
	if (this != &r)
	{
		query::operator=(r);
		*m_imple = *r.m_imple;
	}
	return *this;
}

recordsetQuery::~recordsetQuery()
{
	delete m_imple;
}

void recordsetQuery::init(const fielddefs* fdinfo)
{
	const std::vector<std::_tstring>& tokns = getWheres();
	m_imple->indexes.clear();
	m_imple->compFields.clear();
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
		bool ret = isMatch((*row)[index].comp((*m_imple->row)[i], m_imple->compType[i]), m_imple->compType[i]);
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
	//not delete by destructor. simple copy is ok;
	std::vector<groupFuncBase* > m_funcs;

	void removeFields(recordsetImple& mdls)
	{
		const fielddefs& fds = *mdls.fieldDefs();
		for (int i=(int)fds.size()-1;i>=0;--i)
		{
			bool enabled = false;
			for (int j=0;j<(int)m_impl->keyFields.size();++j)
			{
				if (m_impl->keyFields[j] == fds[i].name())
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
	groupQueryImple():fieldNames(){}

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

		for (int i=0;i<(int)m_impl->keyFields.size();++i)
			keyFields.push_back(resolvKeyValue(mdls, m_impl->keyFields[i]));

		for (int i=0;i<(int)m_funcs.size();++i)
		{

			groupFuncBase* f = m_funcs[i];
			f->init(mdls.fieldDefs());

			if (f->resultKey() == (int)mdls.fieldDefs()->size())
			{
				groupFuncBase::value_type dummy=0;
				mdls.appendField(f->resultName(), getFieldType(dummy), sizeof(dummy));
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

	const std::vector<groupFuncBase* >& getFunctions() const {return m_funcs;};
};

// ---------------------------------------------------------------------------
// class groupQuery
// ---------------------------------------------------------------------------
groupQuery* groupQuery::create()
{
	return new groupQuery();
}

void groupQuery::release()
{
	delete this;
}

groupQuery::groupQuery():m_imple(new groupQueryImple)
{

}

groupQuery::groupQuery(const groupQuery& r):m_imple(new groupQueryImple(*r.m_imple))
{

}

groupQuery& groupQuery::operator=(const groupQuery& r)
{
	if (this != &r)
	{
		*m_imple = *r.m_imple;
	}
	return *this;
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

const fieldNames& groupQuery::getKeyFields()const
{
	return *m_imple;
}

const groupFuncBase* groupQuery::getFunction(int index) const
{
	assert(index>=0 && index < functionCount());
	return m_imple->getFunctions()[index];
}

int groupQuery::functionCount() const
{
	return (int)m_imple->getFunctions().size();
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
	fieldNames m_targetNames;
	std::_tstring m_resultName;
	int m_resultKey;
	std::vector<int> m_targetKeys;


public:
	std::vector<value_type> m_values;
	std::vector<__int64> m_counts;

	inline groupFuncBaseImple(const fieldNames& targetNames , const _TCHAR* resultName=NULL)
	{
	   m_targetNames = targetNames;
	   m_resultName = (m_targetNames.count() &&
							((resultName == NULL) || resultName[0]==0x00))
									 ? targetNames[0]:resultName;
	   m_values.reserve(10);
	}

	inline void initResultVariable(int index)
	{
		std::vector<value_type>::iterator it = m_values.begin(); 
		if (index)
			it += index;
		m_values.insert(it, 0.0f);
	}

	inline void init(const fielddefs* fdinfo)
	{
		m_targetKeys.clear();
		for (int i=0;i<m_targetNames.count();++i)
			m_targetKeys.push_back((m_targetNames[i][0] != 0x00) ? fdinfo->indexByName(m_targetNames[i]): -1);
		m_resultKey = fdinfo->indexByName(m_resultName);
		if (m_resultKey == -1) m_resultKey = (int)fdinfo->size();
	}

	inline groupFuncBaseImple()
	{
		m_values.reserve(10);
	}

	inline fieldNames& targetNames() const {return (fieldNames&)m_targetNames;}

	inline const _TCHAR* resultName() const {return m_resultName.c_str();}

	inline void setResultName(const _TCHAR* v)
	{
		m_resultName = _T("");
		if (v && v[0])
			m_resultName = v;
	}

	inline int resultKey() const {return m_resultKey;}

	inline void reset(){m_values.clear();};

	inline value_type result(int groupIndex)const{return m_values[groupIndex];};

	inline int targetKey(size_t index)const
	{
		assert(index < m_targetKeys.size());
		return m_targetKeys[index];
	}

	inline int targetKeys()const{return (int)m_targetKeys.size();}
};

// ---------------------------------------------------------------------------
// class groupFuncBase
// ---------------------------------------------------------------------------
groupFuncBase::groupFuncBase():m_imple(new groupFuncBaseImple()){}

groupFuncBase::groupFuncBase(const fieldNames& targetNames , const _TCHAR* resultName)
		:recordsetQuery(),m_imple(new groupFuncBaseImple(targetNames, resultName)){}

groupFuncBase::groupFuncBase(const groupFuncBase& r):recordsetQuery(r)
		,m_imple(new groupFuncBaseImple(*r.m_imple)){}

groupFuncBase& groupFuncBase::operator=(const groupFuncBase& r)
{
	if (this != &r)
	{
		*m_imple = *r.m_imple;
		recordsetQuery::operator=(r);
	}
	return *this;
}

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
	if (whereTokens() != 0)
		recordsetQuery::init(fdinfo);

	m_imple->init(fdinfo);
}

groupFuncBase& groupFuncBase::operator=(const recordsetQuery& v)
{
	recordsetQuery::operator=(v);
	return *this;
}


fieldNames& groupFuncBase::targetNames() const
{
	return 	m_imple->targetNames();
}

const _TCHAR* groupFuncBase::resultName() const
{
	return m_imple->resultName();
}

void groupFuncBase::setResultName(const _TCHAR* v)
{
	m_imple->setResultName(v);
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
	bool flag = (whereTokens() == 0);

	if (!flag)
		flag = match(row);
	if (flag)
	   doCalc(row, index);

}

groupFuncBase::value_type groupFuncBase::result(int groupIndex) const
{
	return m_imple->result(groupIndex);
}


// ---------------------------------------------------------------------------
// class sum
// ---------------------------------------------------------------------------
sum* sum::create(const fieldNames& targetNames , const _TCHAR* resultName)
{
	return new sum(targetNames , resultName);
}

sum::sum(const fieldNames& targetNames , const _TCHAR* resultName)
		:groupFuncBase(targetNames, resultName){}

void sum::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	for (int i=0;i<m_imple->targetKeys();++i)
		m_imple->m_values[index] += fieldValue((*row)[m_imple->targetKey(i)], tmp);
}

groupFuncBase* sum::clone()
{
	groupFuncBase* p = new sum();
	*p = *this;
	return p;
}

// ---------------------------------------------------------------------------
// class count
// ---------------------------------------------------------------------------
count* count::create(const _TCHAR* resultName)
{
	return new count(resultName);
}

count::count(const _TCHAR* resultName):groupFuncBase()
{
	setResultName(resultName);
}

void count::doCalc(const row_ptr& row, int index)
{
	m_imple->m_values[index] = m_imple->m_values[index] + 1;
}

groupFuncBase* count::clone()
{
	groupFuncBase* p = new count();
	*p = *this;
	return p;
}


// ---------------------------------------------------------------------------
// class avg
// ---------------------------------------------------------------------------
avg* avg::create(const fieldNames& targetNames , const _TCHAR* resultName)
{
	return new avg(targetNames , resultName);
}

avg::avg(const fieldNames& targetNames , const _TCHAR* resultName)
	:sum(targetNames, resultName){}

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

groupFuncBase* avg::clone()
{
	groupFuncBase* p = new avg();
	*p = *this;
	return p;
}


// ---------------------------------------------------------------------------
// class min
// ---------------------------------------------------------------------------
min* min::create(const fieldNames& targetNames , const _TCHAR* resultName)
{
	return new min(targetNames , resultName);
}

min::min(const fieldNames& targetNames , const _TCHAR* resultName)
	:sum(targetNames, resultName),m_flag(true){}

void min::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	for (int i=0;i<m_imple->targetKeys();++i)
	{
		tmp = fieldValue((*row)[m_imple->targetKey(i)], tmp);
		if (m_flag || m_imple->m_values[index]  > tmp)
			m_flag = false;
		m_imple->m_values[index]  = tmp;
	}
}

groupFuncBase* min::clone()
{
	groupFuncBase* p = new min();
	*p = *this;
	return p;
}

// ---------------------------------------------------------------------------
// class max
// ---------------------------------------------------------------------------
max* max::create(const fieldNames& targetNames , const _TCHAR* resultName)
{
	return new max(targetNames , resultName);
}

max::max(const fieldNames& targetNames , const _TCHAR* resultName)
	:sum(targetNames, resultName),m_flag(true){}

void max::doCalc(const row_ptr& row, int index)
{
	value_type tmp=0;
	for (int i=0;i<m_imple->targetKeys();++i)
	{
		tmp = fieldValue((*row)[m_imple->targetKey(i)], tmp);
		if (m_flag || m_imple->m_values[index]  < tmp)
			m_flag = false;
		m_imple->m_values[index]  = tmp;
	}
}

groupFuncBase* max::clone()
{
	groupFuncBase* p = new max();
	*p = *this;
	return p;
}


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs



