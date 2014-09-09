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

class DLLLIB fieldNames
{

protected:
/** @cond INTERNAL */
	struct fieldNamesImple* m_impl;
/** @endcond */

public:
	fieldNames();
	fieldNames(const fieldNames& r);
	fieldNames& operator=(const fieldNames& r);

	virtual ~fieldNames();
	virtual fieldNames& reset();
	fieldNames& keyField(const _TCHAR* name, const _TCHAR* name1=NULL, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
				,const _TCHAR* name4=NULL, const _TCHAR* name5=NULL, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
				,const _TCHAR* name8=NULL, const _TCHAR* name9=NULL, const _TCHAR* name10=NULL);

	int count() const;
	const _TCHAR* operator[](int index) const;
	const _TCHAR* getValue(int index) const;
	void addValue(const _TCHAR* v);
	void addValues(const _TCHAR* values, const _TCHAR* delmi); //delmi = boost::is_any_of
	static fieldNames* create();
	void release();
};


struct sortField
{
	std::_tstring name;
	bool  asc;
};


class sortFields
{
	std::vector<sortField> m_params;
	/** @cond INTERNAL */
	template <class Archive>
	friend void serialize(Archive& ar, sortFields& q, const unsigned int );
	/** @endcond */

public:
	inline sortFields& add(const _TCHAR* name, bool  asc)
	{
		sortField op ={name, asc};
		m_params.push_back(op);
		return *this;
	}
	inline size_t size() const {return m_params.size();}
	inline const sortField& operator[](int index) const {return m_params[index];}
	inline void clear(){m_params.clear();}
};


class DLLLIB recordsetQuery : protected query
{
	friend class groupFuncBase;
	friend class recordsetImple;
	
	struct recordsetQueryImple* m_imple;
	void init(const fielddefs* fdinfo);
	bool isMatch(int ret, unsigned char compType) const;
	bool match(const row_ptr row) const;

public:

	recordsetQuery();
	recordsetQuery(const recordsetQuery& r);
	~recordsetQuery();

	recordsetQuery& operator=(const recordsetQuery& r);

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

	inline const _TCHAR* toString() const{return queryBase::toString();}

	inline query* internalQuery(){return this;}
	static recordsetQuery* create();
	void release();

};


class DLLLIB groupFuncBase : public recordsetQuery
{
protected:
	friend class groupQueryImple;

	class groupFuncBaseImple* m_imple;
	virtual void initResultVariable(int index);
	virtual void doCalc(const row_ptr& row, int groupIndex);
	void init(const fielddefs* fdinfo);

public:
	typedef double value_type;
	groupFuncBase();
	groupFuncBase(const groupFuncBase& v);
	groupFuncBase& operator=(const groupFuncBase& v);
	groupFuncBase(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	virtual ~groupFuncBase();
	groupFuncBase& operator=(const recordsetQuery& v);
	fieldNames&  targetNames() const ;
	const _TCHAR* resultName() const;
	void setResultName(const _TCHAR* v);
	int resultKey() const ;
	void reset();
	void operator()(const row_ptr& row, int index, bool insert);
	virtual value_type result(int groupIndex)const;
	virtual groupFuncBase* clone() = 0;
};

class recordsetImple;

class DLLLIB groupQuery 
{
	friend class recordsetImple;
	class groupQueryImple* m_imple;
	void grouping(recordsetImple& rs);
public:
	groupQuery();
	groupQuery(const groupQuery& r);
	groupQuery& operator=(const groupQuery& r);

	~groupQuery();
	groupQuery& reset() ;
	groupQuery& addFunction(groupFuncBase* func);
	groupQuery& keyField(const _TCHAR* name, const _TCHAR* name1=NULL, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
				,const _TCHAR* name4=NULL, const _TCHAR* name5=NULL, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
				,const _TCHAR* name8=NULL, const _TCHAR* name9=NULL, const _TCHAR* name10=NULL);
	const fieldNames& getKeyFields()const;
	const groupFuncBase* getFunction(int index) const;
	int functionCount() const;
	static groupQuery* create();
	void release();

};


class DLLLIB sum : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);

public:
	sum(){}
	sum(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	groupFuncBase* clone();
	static sum* create(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
};


class DLLLIB count : public groupFuncBase
{
protected:
	void doCalc(const row_ptr& row, int index);

public:
	count(){}
	count(const _TCHAR* resultName);
	groupFuncBase* clone();
	static count* create(const _TCHAR* resultName);

};


class DLLLIB avg : public sum
{

	void initResultVariable(int index);
	void doCalc(const row_ptr& row, int index);
	value_type result(int index)const;

public:
	avg(){}
	avg(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	groupFuncBase* clone();
	static avg* create(const fieldNames& targetNames , const _TCHAR* resultName=NULL);

};


#undef min
class DLLLIB min : public sum
{
	bool m_flag ;
	void doCalc(const row_ptr& row, int index);

public:
	min(){}
	min(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	groupFuncBase* clone();
	static min* create(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
};


#undef max
class DLLLIB max : public sum
{
	bool m_flag ;
	void doCalc(const row_ptr& row, int index);
public:
	max(){}
	max(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	groupFuncBase* clone();
	static max* create(const fieldNames& targetNames , const _TCHAR* resultName=NULL);
};



}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H

