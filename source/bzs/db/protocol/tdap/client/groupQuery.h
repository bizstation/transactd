#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H
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
#include "fieldNames.h"
#include "memRecord.h"
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include "groupComp.h"

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

typedef fieldsBase row;
typedef boost::shared_ptr<row> row_ptr;

class AGRPACK recordsetQuery : protected query
{
	friend class groupFuncBase;
	friend class recordset;
	row_ptr m_row;
	std::vector<unsigned char> m_compType;
	std::vector<short> m_indexes;
	std::vector<char> m_combine;
	short m_endIndex;
	fielddefs m_compFields;

	void init(const fielddefs* fdinfo);

	bool isMatch(int ret, unsigned char compType) const;

	bool match(const row_ptr row) const;

public:

	template <class T>
	inline recordsetQuery& when(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		query::where(name, qlogic, value);
		return *this;
	}

	template <class T>
	inline recordsetQuery& and_(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		query::and_(name, qlogic, value);
		return *this;
	}

	template <class T>
	inline recordsetQuery& or_(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		query::or_(name, qlogic, value);
		return *this;
	}

	inline recordsetQuery& reset()
	{
		query::reset();
		return *this;
	}


};


class AGRPACK groupFuncBase
{
public:
	typedef double value_type;

private:
	friend class groupQuery;

	const _TCHAR* m_targetName;
	const _TCHAR* m_resultName;
protected:
	int m_resultKey;
	int m_targetKey;
	std::vector<value_type> m_values;
	recordsetQuery* m_query;

	virtual void initResultVariable(int index);
	virtual void doCalc(const row_ptr& row, int groupIndex){};

	void init(const fielddefs* fdinfo);

public:
	groupFuncBase(const _TCHAR* targetName , const _TCHAR* resultName=NULL
		, recordsetQuery* query=NULL);

	virtual ~groupFuncBase();

	inline groupFuncBase& setQuery(recordsetQuery* query)
	{
		m_query = query;
		return *this;
	}

	inline const _TCHAR* targetName() const {return m_targetName;}

	inline const _TCHAR* resultName() const {return m_resultName;}

	inline int resultKey() const {return m_resultKey;}

	void reset();

	void operator()(const row_ptr& row, int index, bool insert);

	virtual value_type result(int groupIndex)const;

};


class AGRPACK groupQuery : public fieldNames
{
	std::vector<groupFuncBase* > m_funcs;
	typedef recordset Container;
	void removeFields(Container& mdls);
public:
	fieldNames& reset() ;
	groupQuery& addFunction(groupFuncBase* func);
	void grouping(Container& mdls);
};


class AGRPACK sum : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);

public:
	sum(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};

class AGRPACK count : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);
	value_type result(int index)const;

public:
	count(const _TCHAR* resultName, recordsetQuery* query=NULL);

};


class AGRPACK avg : public sum
{
	std::vector<__int64> m_count;
	void initResultVariable(int index);
	void doCalc(const row_ptr& row, int index);
	value_type result(int index)const;

public:
	avg(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};


#undef min
class AGRPACK min : public sum
{
	bool m_flag ;
	void doCalc(const row_ptr& row, int index);
public:
	min(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};

#undef max
class AGRPACK max : public sum
{
	bool m_flag ;
	void doCalc(const row_ptr& row, int index);

public:
	max(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};



}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H

