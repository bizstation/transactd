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
#include <bzs/db/protocol/tdap/client/trdboostapi.h>

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

class DLLLIB recordsetQuery : protected query
{
	friend class groupFuncBaseImple;
	friend class recordsetImple;
	
	struct recordsetQueryImple* m_imple;
	void init(const fielddefs* fdinfo);
	bool isMatch(int ret, unsigned char compType) const;
	bool match(const row_ptr row) const;

public:
	recordsetQuery();
	~recordsetQuery();

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


class DLLLIB groupFuncBase
{
protected:
	friend class groupQueryImple;

	class groupFuncBaseImple* m_imple;
	virtual void initResultVariable(int index);
	virtual void doCalc(const row_ptr& row, int groupIndex);
	void init(const fielddefs* fdinfo);

public:
	typedef double value_type;
	groupFuncBase(const _TCHAR* targetName , const _TCHAR* resultName=NULL
		, recordsetQuery* query=NULL);

	virtual ~groupFuncBase();

	groupFuncBase& setQuery(recordsetQuery* query);
	const _TCHAR* targetName() const;
	const _TCHAR* resultName() const;
	int resultKey() const ;
	void reset();
	void operator()(const row_ptr& row, int index, bool insert);
	virtual value_type result(int groupIndex)const;
};

class recordsetImple;

class DLLLIB groupQuery 
{
	friend class recordsetImple;
	class groupQueryImple* m_imple;
	const std::vector<std::_tstring>& getKeyFields()const;
	void grouping(recordsetImple& rs);
public:
	groupQuery();
	~groupQuery();
	groupQuery& reset() ;
	groupQuery& addFunction(groupFuncBase* func);
	groupQuery& keyField(const TCHAR* name, const TCHAR* name1=NULL, const TCHAR* name2=NULL, const TCHAR* name3=NULL
				,const TCHAR* name4=NULL, const TCHAR* name5=NULL, const TCHAR* name6=NULL, const TCHAR* name7=NULL
				,const TCHAR* name8=NULL, const TCHAR* name9=NULL, const TCHAR* name10=NULL);

};


class DLLLIB sum : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);

public:
	sum(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};


class DLLLIB count : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);

public:
	count(const _TCHAR* resultName, recordsetQuery* query=NULL);

};


class DLLLIB avg : public sum
{

	void initResultVariable(int index);
	void doCalc(const row_ptr& row, int index);
	value_type result(int index)const;

public:
	avg(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);

};


#undef min
class DLLLIB min : public sum
{
	bool m_flag ;
	void doCalc(const row_ptr& row, int index);

public:
	min(const _TCHAR* targetName , const _TCHAR* resultName=NULL, recordsetQuery* query=NULL);
};


#undef max
class DLLLIB max : public sum
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

