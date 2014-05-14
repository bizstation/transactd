#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_TRDORMAPI_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_TRDORMAPI_H
/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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
#include "trdboostapi.h"
#include "filedNameAlias.h"
#include "groupQuery.h"
#include "memRecord.h"
#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <vector>
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

class qlogic
{
	std::_tstring m_name;
	std::_tstring m_value;
	std::_tstring m_type;
	combineType m_next;

public:

	qlogic(const _TCHAR* name, const _TCHAR* type, const _TCHAR* value, combineType next)
		:m_name(name),m_type(type),m_value(value),m_next(next){}
	qlogic(const _TCHAR* name, const _TCHAR* type, int value, combineType next)
		:m_name(name),m_type(type),m_next(next)
		{
			_TCHAR buf[50];
			m_value = _ltot(value, buf, 10);
		}
	qlogic(const _TCHAR* name, const _TCHAR* type, __int64 value, combineType next)
		:m_name(name),m_type(type),m_next(next)
		{
			_TCHAR buf[50];
			m_value = _i64tot(value, buf, 10);
		}
	qlogic(const _TCHAR* name, const _TCHAR* type, double value, combineType next)
		:m_name(name),m_type(type),m_next(next)
		{
			_TCHAR buf[50];
			_stprintf(buf, _T("%.*f"),15, value);
			m_value = buf;
		}
};

class databaseManager : boost::noncopyable
{
	database* m_db;
	database_ptr m_dbPtr;
	std::vector<table_ptr> m_tables;
	int findTable(const _TCHAR* name)
	{
		for (int i=0;i<(int)m_tables.size();++i)
			if (_tcscmp(m_tables[i]->tableDef()->tableName(), name)==0)
				return i;
		return -1;
	}
public:
	databaseManager(database_ptr db):m_dbPtr(db),m_db(db.get()){};
	databaseManager(database* db):m_db(db){};
	table_ptr table(const _TCHAR* name)
	{
		int index =  findTable(name);
		if (index !=-1)
			return  m_tables[index];
		table_ptr t = openTable(m_db, name);
		m_tables.push_back(t);
		return t;
	}
	database* db(){return m_db;}

};

class query : public queryBase
{
public:
	query():queryBase(){}
	query& reset(){queryBase::reset();return *this;}

	query& select(const TCHAR* name, const TCHAR* name1=NULL, const TCHAR* name2=NULL, const TCHAR* name3=NULL
				,const TCHAR* name4=NULL, const TCHAR* name5=NULL, const TCHAR* name6=NULL, const TCHAR* name7=NULL
				,const TCHAR* name8=NULL, const TCHAR* name9=NULL, const TCHAR* name10=NULL)
	{
		if (_tcscmp(name, _T("*"))==0)
		{
			clearSelectFields();
			return *this;
		}
		addField(name);
		if (name1) addField(name1);
		if (name2) addField(name2);
		if (name3) addField(name3);
		if (name4) addField(name4);
		if (name5) addField(name5);
		if (name6) addField(name6);
		if (name7) addField(name7);
		if (name8) addField(name8);
		if (name9) addField(name9);
		if (name10) addField(name10);
		return *this;
	}

	template <class T>
	query& where(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		addLogic(name, qlogic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}

#ifndef SWIG
	template <class T>
	query& and(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		if (whereTokens() == 0)
			throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

		addLogic(_T("and"), name, qlogic, boost::lexical_cast<std::_tstring>(value).c_str());
 		return *this;
	}

	template <class T>
	query& or(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		if (whereTokens() == 0)
			throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

		addLogic(_T("or"), name, qlogic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}
#endif

	template <class T>
	query& in(const _TCHAR* name, const _TCHAR* qlogic, T value)
	{
		if (whereTokens() == 0)
			throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

		addLogic(_T("or"), name, qlogic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}

	template <class T0, class T1 , class T2, class T3
				,class T4, class T5 , class T6 , class T7>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv3).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv4).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv5).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv6).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv7).c_str());
		return *this;
	}
	template <class T0, class T1 , class T2, class T3
				,class T4, class T5 , class T6>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv3).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv4).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv5).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv6).c_str());
		return *this;
	}

	template <class T0, class T1 , class T2, class T3
				,class T4, class T5>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv3).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv4).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv5).c_str());
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv3).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv4).c_str());
		return *this;
	}

	template <class T0, class T1 , class T2, class T3>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv3).c_str());
		return *this;
	}

	template <class T0, class T1 , class T2>
	query& in(const T0 kv0, const T1 kv1, const T2 kv2)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv2).c_str());
		return *this;
	}

	template <class T0, class T1>
	query& in(const T0 kv0, const T1 kv1)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv1).c_str());
		return *this;
	}

	template <class T0>
	query& in(const T0 kv0)
	{
		addSeekKeyValue(boost::lexical_cast<std::_tstring>(kv0).c_str());
		return *this;
	}

};


template <class T>
inline typename std::vector<T>::iterator begin(std::vector<T>& m){return m.begin();}

template <class T>
inline typename std::vector<T>::iterator end(std::vector<T>& m){return m.end();}

template <class T>
inline void push_back(std::vector<T>& m, T c){return m.push_back(c);}



/* Container has readBefore(table_ptr) function*/
template <class Container>
void readBefore(typename Container::header_type* dummy, Container& mdls
				, table_ptr tb, const aliasMap_type* alias)
{
	mdls.readBefore(tb, alias);
}

/* Container has'nt readBefore(table_ptr) function*/
template <class Container>
void readBefore(...){}


/* Container operation handlter

*/
template <class MAP
			, class Container
			, class T=MAP::mdl_typename
			, class FDI=MAP::fdi_typename>
class mdlsHandler
{
	mdlsHandler();

protected:
	Container& m_mdls;
	int m_option;
	FDI* m_fdi;
	MAP* m_map;

	template <class mdls_type>
	void addContainer(T* u, typename mdls_type::item_type* p )
	{
		mdls_type::item_type ptr(u);
		push_back(m_mdls, ptr);
	}

	template <class mdls_type>
	void addContainer(T* u, ...)
	{
		push_back(m_mdls, boost::shared_ptr<T>(u));
	}


public:
	mdlsHandler(Container& mdls):m_mdls(mdls){}

	virtual ~mdlsHandler(){};

	void init(int option, FDI* fdi, MAP& map, table_ptr tb, const aliasMap_type* alias=NULL)
	{
		m_option = option;
		m_fdi = fdi;
		m_map = &map;

		readBefore<Container>(0, m_mdls, tb, alias);
	}

	void operator()(const fields& fds)
	{
		T* u(create(m_mdls, m_option));
		m_map->readMap(*u, fds, m_option);
		addContainer<Container>(u, 0);
	}

};

/* For sort in readEach
*/
template <class MAP, class T>
class compFunc
{
	MAP& m_map;
	int m_keynum;
public:
	compFunc(MAP& map, int keynum):m_map(map),m_keynum(keynum){}
	bool operator()(T* l, T* r) const
	{
		return m_map.compKeyValue(*l, *r, m_keynum);
	}

	bool operator()(boost::shared_ptr<T>& l, boost::shared_ptr<T>& r) const
	{
		return m_map.compKeyValue(*l, *r, m_keynum);
	}
};


#ifndef SWIG
template <class T, class RET>
bool sortFuncBase(T&l, T& r , RET (T::*func1)() const)
{
	RET retl = (l.*func1)();
	RET retr = (r.*func1)();
	return retl < retr;
}
#endif

template <class T, class FUNC1, class FUNC2, class FUNC3>
bool sortFunc(T&l, T& r , FUNC1 func1, FUNC2 func2, FUNC3 func3)
{
	bool v = sortFuncBase(l, r, func1);
	if (func2)
	{
		if (v) return v;
		v = sortFuncBase(r, l, func1);
		if (v) return !v;
		v = sortFuncBase(l, r, func2);
		if (func3)
		{
			if (v) return v;
			v = sortFuncBase(r, l, func2);
			if (v) return !v;
			v = sortFuncBase(l, r, func3);
		}
	}
	return v;
}

template <class FUNC1, class FUNC2, class FUNC3>
class sortFunctor
{
	FUNC1 m_func1;
	FUNC2 m_func2;
	FUNC3 m_func3;
public:
	sortFunctor(FUNC1 func1, FUNC2 func2, FUNC3 func3)
		:m_func1(func1),m_func2(func2), m_func3(func3){}
	template <class T>
	bool operator()(T* l, T* r) const
	{
		return sortFunc(*l, *r, m_func1, m_func2, m_func2);
	}

	template <class T>
	bool operator()(boost::shared_ptr<T>& l, boost::shared_ptr<T>& r) const
	{
		bool v =  sortFunc(*l, *r, m_func1, m_func2, m_func2);
		return v;
	}
};

template <class Container, class FUNC1, class FUNC2, class FUNC3>
void sort(Container& mdls, FUNC1 func1, FUNC2 func2, FUNC3 func3 )
{
	sortFunctor<FUNC1, FUNC2, FUNC3> functor(func1, func2, func3);
	std::sort(begin(mdls), end(mdls), functor);
}

template <class Container, class FUNC1, class FUNC2>
void sort(Container& mdls, FUNC1 func1, FUNC2 func2)
{
	sortFunctor<FUNC1, FUNC2, FUNC1> functor(func1, func2, NULL);
	std::sort(begin(mdls), end(mdls), functor);
}

template <class Container, class FUNC1>
void sort(Container& mdls, FUNC1 func1)
{
	sortFunctor<FUNC1, FUNC1, FUNC1> functor(func1, NULL, NULL);
	std::sort(begin(mdls), end(mdls), functor);
}

#ifndef SWIG
template <class T2, class T, class Container>
inline boost::shared_ptr<std::vector<T> > listup(Container& mdls, T (T2::*func)()const)
{
	typename Container::iterator it = begin(mdls), ite = end(mdls);

	boost::shared_ptr<std::vector<T> > mdlst( new std::vector<T>());
	while(it != ite)
	{
		T2& mdl = *(*it);
		T ref = (mdl.*func)();
		mdlst->push_back(ref);
		++it;
	}
	return mdlst;
}
#endif


class mraResetter
{
	table_ptr& m_tb;
public:
	mraResetter(table_ptr& tb):m_tb(tb){}

	~mraResetter()
	{
		if (m_tb->mra())
			m_tb->mra()->setJoinType( mra::mra_nojoin);
	   m_tb->setMra(NULL);
	}
};


template <class MAP, class T=MAP::mdl_typename, class FDI=MAP::fdi_typename>
class activeTable : boost::noncopyable
{
protected:
	typedef std::vector<boost::shared_ptr<T> > collection_vec_type;
	table_ptr m_tb;
	FDI* m_fdi;
	MAP m_map;
	int m_option;
	bool m_useTransactd;
	fdNmaeAlias m_alias;

	inline T& getItem(collection_vec_type& mdls, unsigned int index){return *(mdls[index]);}
	void init(databaseManager& mgr, const _TCHAR* name)
	{
		m_tb = mgr.table(name);
	}

	void init(database_ptr& db, const _TCHAR* name)
	{
		m_tb = openTable(db, name);
	}

	void init(database* db, const _TCHAR* name)
	{
		m_tb = openTable(db, name);
	}

	inline void reverseAliasNamesQuery(queryBase& q)
	{
		aliasMap_type::const_iterator it = m_alias.map().begin();
		while(it != m_alias.map().end())
		{
			q.reverseAliasName((*it).second.c_str(), (*it).first.c_str());
			++it;
		}
	}

	template <class Container>
	void doJoin(bool innner, Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
	{
		if (!m_useTransactd)
			nstable::throwError(_T("activeTable P.SQL can not use this"), (short_td)0);
		reverseAliasNamesQuery(q);
		q.clearSeekKeyValues();
		fields fds(m_tb);
		mraResetter mras(m_tb);
		typename Container::iterator it = mdls.begin(),ite = mdls.end();

		bool optimize = !(q.getOptimize() & queryBase::hasOneJoin);
		std::vector<std::vector<int> > joinRowMap;
		std::vector<typename Container::key_type> fieldIndexes;
		groupQuery gq;
		gq.keyField(name1, name2, name3, name4, name5, name6, name7, name8, name9, name10, name11);
		gq.getFieldIndexes(mdls, fieldIndexes);

		/* optimizing join
			If base recordset is made by unique key and join by uniqe field, that can not opitimize.
		*/
		if (optimize)
		{
			gq.makeJoinMap(mdls, joinRowMap, fieldIndexes);
			q.reserveSeekKeyValueSize(joinRowMap.size());
			std::vector<std::vector<int> >::iterator it1 = joinRowMap.begin(),ite1 = joinRowMap.end();
			while(it1 != ite1)
			{
				T& mdl = *(mdls[(*it1)[0]]);
				for (int i=0;i<fieldIndexes.size();++i)
					q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr());
				++it1;
			}
		}
		else
		{
			while(it != ite)
			{
				T& mdl = *(*it);
				for (int i=0;i<fieldIndexes.size();++i)
					q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr());
				++it;
			}
		}

		m_tb->setQuery(&q);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable Join Query"), &(*m_tb));

		typename MAP::collection_orm_typename map(mdls);

		/* ignore list for inner join */
		std::vector<typename Container::iterator> ignores;
		it = mdls.begin();
		map.init(m_option, m_fdi, m_map, m_tb, &m_alias.map());
		if (m_tb->mra())
		{
			m_tb->mra()->setJoinType(innner ? mra::mra_innerjoin : mra::mra_outerjoin);
			if (optimize)
				m_tb->mra()->setJoinRowMap(&joinRowMap);
		}
		m_tb->find();
		while(1)
		{
			if (m_tb->stat())
			{
				if ((m_tb->stat() == STATUS_EOF) ||
					((m_tb->stat() != STATUS_SUCCESS) && (m_tb->stat() != STATUS_NOT_FOUND_TI)))
					break;
				else if (innner)
					ignores.push_back(it);
			}
			++it;
			m_tb->findNext(); //mra copy value to memrecord
		}

		readStatusCheck(*m_tb, _T("join"));
		m_tb->mra()->setJoinRowMap(NULL);
		/* remove record see ignore list for inner join */
		for (int i=(int)ignores.size()-1;i>=0;--i)
			mdls.erase(ignores[i]);
	}

public:

	explicit activeTable(databaseManager& mgr)
			:m_option(0)
			,m_fdi(createFdi(m_fdi))
			,m_map(*m_fdi)
			,m_useTransactd(mgr.db()->isUseTransactd())
			{
				init(mgr, m_map.getTableName());
				if (table() && m_fdi)
					initFdi(m_fdi, m_tb.get());
			}

	explicit activeTable(database_ptr& db)
			:m_option(0)
			,m_fdi(createFdi(m_fdi))
			,m_map(*m_fdi)
			,m_useTransactd(db->isUseTransactd())
			{
				init(db, m_map.getTableName());
				if (table() && m_fdi)
					initFdi(m_fdi, m_tb.get());
			}


	explicit activeTable(databaseManager& mgr, const _TCHAR* tableName)
			:m_option(0)
			,m_fdi(createFdi(m_fdi))
			,m_map(*m_fdi)
			,m_useTransactd(mgr.db()->isUseTransactd())
			{
				init(mgr, tableName);
				if (table() && m_fdi)
					initFdi(m_fdi, m_tb.get());
			}

	explicit activeTable(database_ptr& db, const _TCHAR* tableName)
			:m_option(0)
			,m_fdi(createFdi(m_fdi))
			,m_map(*m_fdi)
			,m_useTransactd(db->isUseTransactd())
			{
				init(db, tableName);
				if (table() && m_fdi)
					initFdi(m_fdi, m_tb.get());
			}

	explicit activeTable(database* db, const _TCHAR* tableName)
			:m_option(0)
			,m_fdi(createFdi(m_fdi))
			,m_map(*m_fdi)
			,m_useTransactd(db->isUseTransactd())
			{
				init(db, tableName);
				if (table() && m_fdi)
					initFdi(m_fdi, m_tb.get());
			}

	~activeTable(){destroyFdi(m_fdi);}

	inline void beginBulkInsert(int maxBuflen){m_tb->beginBulkInsert(maxBuflen);}

	inline void abortBulkInsert(){m_tb->abortBulkInsert();}

	inline ushort_td commitBulkInsert() {return m_tb->commitBulkInsert();}


	activeTable& index(int v)
	{
		m_tb->clearBuffer();
		m_tb->setKeyNum(v);
		return *this;
	}

	template <class T0>
	activeTable& keyValue(const T0 kv0)
	{
		keyValueSetter<T0>::set(m_tb, m_tb->keyNum(), kv0);
		return *this;
	}

	template <class T0, class T1>
	activeTable& keyValue(const T0 kv0, const T1 kv1)
	{
		keyValueSetter<T0, T1>::set(m_tb, m_tb->keyNum(), kv0, kv1);
		return *this;
	}

	template <class T0, class T1 , class T2>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2)
	{
		keyValueSetter<T0, T1, T2>::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
	{
		keyValueSetter<T0, T1, T2, T3>::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4)
	{
		keyValueSetter<T0, T1, T2, T3, T4>
				::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 >
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5>
				::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 , class T6>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5, T6>
				::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3
				,class T4, class T5 , class T6 , class T7>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>
				::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
		return *this;
	}

	inline table_ptr table() const {return m_tb;};


	activeTable& option(int v)
	{
		m_option = v;
		return *this;
	}

	template <class Any_Map_type>
	activeTable& readRange(Any_Map_type& map, queryBase& q)
	{
		mraResetter mras(m_tb);
		reverseAliasNamesQuery(q);
		m_tb->setQuery(&q);
		if (m_tb->stat())
			nstable::throwError(_T("Query is inaccurate"), &(*m_tb));

		map.init(m_option, m_fdi, m_map, m_tb, &m_alias.map());
		m_tb->find(q.getDirection());
		if (q.getDirection() == table::findForword)
		{
			findIterator itsf(*m_tb);
			for_each(itsf, map);
		}else
		{
			findRvIterator itsf(*m_tb);
			for_each(itsf, map);
		}
		return *this;
	}

	template <class Any_Map_type>
	activeTable& readRange(Any_Map_type& map, queryBase& q, validationFunc func)
	{
		mraResetter mras(m_tb);
		reverseAliasNamesQuery(q);
		m_tb->setQuery(&q);
		if (m_tb->stat())
			nstable::throwError(_T("Query is inaccurate"), &(*m_tb));
		map.init(m_option, m_fdi, m_map, m_tb, &m_alias.map());
		m_tb->find(q.getDirection());
		if (q.getDirection() == table::findForword)
		{
			findIterator itsf(*m_tb);
			filterdFindIterator it(itsf, func);
			for_each(it, map);
		}else
		{
			findRvIterator itsf(*m_tb);
			filterdFindRvIterator it(itsf, func);
			for_each(it, map);
		}
		return *this;
	}

	activeTable& read(collection_vec_type& mdls, queryBase& q, validationFunc func)
	{
		mdlsHandler<MAP, collection_vec_type> map(mdls);
		return readRange(map, q, func);
	}

	activeTable& read(collection_vec_type& mdls, queryBase& q)
	{
		mdlsHandler<MAP, collection_vec_type> map(mdls);
		return readRange(map, q);
	}

	template <class Container>
	activeTable& read(Container& mdls, queryBase& q)
	{
		typename MAP::collection_orm_typename map(mdls);
		return readRange(map, q);
	}

	
	template <class Container>
	activeTable& read(Container& mdls, queryBase& q, validationFunc func)
	{
		typename MAP::collection_orm_typename map(mdls);
		return readRange(map, q, func);
	}

	template <class T2>
	void read(T2& mdl, bool setKeyValueFromObj=true)
	{
		fields fds(m_tb);
		if (setKeyValueFromObj)
			m_map.setKeyValues(mdl, fds, m_tb->keyNum());
		indexIterator it = readIndex(m_tb, eSeekEqual);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable read"), &(*m_tb));
		m_map.readMap(mdl, fds, m_option);
	}

	template <class T2>
	void update(T2& mdl, bool setKeyValueFromObj=true)
	{
		fields fds(m_tb);
		if (setKeyValueFromObj)
			m_map.setKeyValues(mdl, fds, m_tb->keyNum());
		indexIterator it = readIndex(m_tb, eSeekEqual);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable update"), &(*m_tb));

		m_map.writeMap(mdl, fds, m_option);
		updateRecord(it);

	}

	/*template <class Container>
	void update(Container& mdls)
	{
		typename Container::iterator it = begin(mdls),ite = end(mdls);
		while (it != ite)
			update(*it);

	}*/

	// No need object
	void del()
	{
		readIndex(m_tb, eSeekEqual);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable delete"), &(*m_tb));
		m_tb->del();
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable delete"), &(*m_tb));
	}

	//Recieve delete record by mdl
	template <class T2>
	void del(T2& mdl, bool setKeyValueFromObj=true)
	{
		read(mdl, setKeyValueFromObj);
		m_tb->del();
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable delete"), &(*m_tb));
	}

	/*template <class Container>
	void del(Container& mdls)
	{
		typename Container::iterator it = begin(mdls),ite = end(mdls);
		while (it != ite)
			del(*it);

	}*/

	template <class T2>
	void insert(T2& mdl)
	{
		fields fds(m_tb);
		m_map.writeMap(mdl, fds, m_option);
		insertRecord(fds);
		m_map.readAuntoincValue(mdl, fds, m_option);
	}

	/*template <class Container>
	void insert(Container& mdls)
	{
		typename Container::iterator it = begin(mdls),ite = end(mdls);
		while (it != ite)
			insert(*it);

	}*/

	template <class T2>
	void save(T2& mdl, bool setKeyValueFromObj=true)
	{
		fields fds(m_tb);
		if (setKeyValueFromObj)
			m_map.setKeyValues(mdl, fds, m_tb->keyNum());
		indexIterator it = readIndex(m_tb, eSeekEqual);
		if (m_tb->stat() == STATUS_NOT_FOUND_TI)
			insert(mdl);
		else
		{
			m_map.writeMap(mdl, fds, m_option);
			updateRecord(it);
		}
	}



	/* mdlsがキーフィールドに対応するメンバによってソート済の時は
	   sortedをtrueにします。検索するレコードと通信量が激減します。
	*/
	template <class Container>
	void readEach(Container& mdls, queryBase& q, bool sorted=false, bzs::rtl::exception* e=NULL)
	{
		mraResetter mras(m_tb);
		reverseAliasNamesQuery(q);
		fields fds(m_tb);
		typename Container::iterator it = begin(mdls),itb = begin(mdls),ite = end(mdls);
		it = itb = begin(mdls);
		T& mdlb = *(*it);
		if (!m_useTransactd)
			nstable::throwError(_T("activeTable P.SQL can not use this"), (short_td)0);
		while(it != ite)
		{
			//if mdl has same key value, to be once read access to server
			T& mdl = *(*it);
			if ((it == itb)
					|| !sorted
					|| (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum())==true)
					|| (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum())==true))
			{
				m_map.setKeyValues(mdl, fds, m_tb->keyNum());
				keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];
				for (int i=0;i<kd->segmentCount;++i)
					q.addSeekKeyValue(fds[kd->segments[i].fieldNum].c_str());
			}
			mdlb = mdl;
			++it;
		}
		m_tb->setQuery(&q);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeTable readEach Query"), &(*m_tb));
		m_tb->find();
		//見つからないレコードがあると、その時点でエラーで返る
		//行ごとにエラーかどうかわかった方がよい。
		it = itb = begin(mdls);
		while(it != ite)
		{
			if ((m_tb->stat() != STATUS_SUCCESS)
						&& (m_tb->stat() != STATUS_NOT_FOUND_TI))
				nstable::throwError(_T("activeTable readEach"), &(*m_tb));
			T& mdl = *(*it);
			if ((it != itb) &&
				(!sorted
					|| (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum())==true)
					|| (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum())==true)))
			{
				m_tb->findNext();
				if (m_tb->stat() != 0)
				{
					_TCHAR buf[8192];
					m_tb->keyValueDescription(buf, 8192);
					if (e)
						*e << bzs::rtl::errnoCode(m_tb->stat()) << bzs::rtl::errMessage(buf);
					else
						THROW_BZS_ERROR_WITH_CODEMSG(m_tb->stat(), buf);
				}
			}
			if (m_tb->stat() == 0)
				m_map.readMap(mdl, fds, m_option);
			mdlb = mdl;
			++it;
		}
	}

	/** Join相当の処理を事前ソートして高速に行います。
	*/
#ifndef SWIG
	template <class BaseContainer, class T2>
	void readEach(BaseContainer& mdls, T* (T2::*func)()const, queryBase& q)
	{
		boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
		compFunc<MAP, T> comp(m_map, m_tb->keyNum());
		std::sort(refList->begin(), refList->end(), comp);
		readEach(*refList, q, true, NULL);
	}

	template <class BaseContainer, class T2>
	void readEach(BaseContainer& mdls, T* (T2::*func)()const, queryBase& q, bzs::rtl::exception& e)
	{
		boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
		compFunc<MAP, T> comp(m_map, m_tb->keyNum());
		std::sort(refList->begin(), refList->end(), comp);
		readEach(*refList, q, true, &e);
	}
#endif

	/* No use field select */
	template <class Container>
	void readEach(Container& mdls, bool sorted=false, bzs::rtl::exception* e=NULL)
	{
		fields fds(m_tb);
		mraResetter mras(m_tb);
		typename Container::iterator it = begin(mdls),itb = begin(mdls),ite = end(mdls);
		it = itb = begin(mdls);
		T& mdlb = *(*it);
		while(it != ite)
		{
			T& mdl = *(*it);
			if ((it == itb)
					|| !sorted
					|| (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum())==true)
					|| (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum())==true))
			{
				m_map.setKeyValues(mdl, fds, m_tb->keyNum());
				readIndex(m_tb, eSeekEqual);
				if (m_tb->stat() != 0)
				{
					_TCHAR buf[8192];
					m_tb->keyValueDescription(buf, 8192);
					if (e)
						*e << bzs::rtl::errnoCode(m_tb->stat()) << bzs::rtl::errMessage(buf);
					else
						THROW_BZS_ERROR_WITH_CODEMSG(m_tb->stat(), buf);
				}
			}
			if (m_tb->stat() == 0)
				m_map.readMap(mdl, fds, m_option);
			mdlb = mdl;
			++it;
		}
	}

	/* No use field select */
#ifndef SWIG
	template <class BaseContainer, class T2>
	void readEach(BaseContainer& mdls, T* (T2::*func)()const)
	{
		boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
		compFunc<MAP, T> comp(m_map, m_tb->keyNum());
		std::sort(refList->begin(), refList->end(), comp);
		readEach(*refList, true, NULL);
	}

	template <class BaseContainer, class T2>
	void readEach(BaseContainer& mdls, T* (T2::*func)()const , bzs::rtl::exception& e)
	{
		boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
		compFunc<MAP, T> comp(m_map, m_tb->keyNum());
		std::sort(refList->begin(), refList->end(), comp);
		readEach(*refList, true, &e);
	}
#endif

	/*template <class Container, class FUNC>
	activeTable& groupBy(Container& mdls, groupQuery& gq, FUNC func)
	{
		gq.grouping(mdls, func);
		return *this;
	}*/

	inline activeTable& alias(const _TCHAR* src, const _TCHAR* dst)
	{
		m_alias.set(src, dst);
		return *this;
	}

	inline activeTable& resetAlias()
	{
		m_alias.clear();
		return *this;
	}

	typedef boost::shared_ptr<writableRecord> record;
	record m_record;

	inline writableRecord& createWritableRecord()
	{
		m_record.reset(writableRecord::create(m_tb, &m_alias.map()), &writableRecord::release);
		return *m_record.get();
	}
	/*
	bool read(memoryRecord& rec)
	{
		rec.copyToBuffer(m_tb.get());
		m_tb->seek();
		if (m_tb->stat())
			return false;
		rec.copyFromBuffer(m_tb.get());
		return true;
	}

	void insert(memoryRecord& rec)
	{
		rec.copyToBuffer(m_tb.get());
		insertRecord(m_tb);
	}

	void del(memoryRecord& rec)
	{
		rec.copyToBuffer(m_tb.get());
		m_tb->seek();
		if (m_tb->stat())
			nstable::throwError(_T("activeTable delete "), m_tb->stat());
		deleteRecord(m_tb.get());
	}

	void update(memoryRecord& rec)
	{
		rec.copyToBuffer(m_tb.get());
		m_tb->seek();
		if (m_tb->stat())
			nstable::throwError(_T("activeTable update "), m_tb->stat());
		rec.copyToBuffer(m_tb.get(), true);
		updateRecord(m_tb.get());
	}

	void save(memoryRecord& rec)
	{
		rec.copyToBuffer(m_tb.get());
		m_tb->seek();
		if (m_tb->stat() == STATUS_NOT_FOUND_TI)
			insertRecord(m_tb);
		else
		{
			rec.copyToBuffer(m_tb.get());
			updateRecord(m_tb.get());
		}
	}*/

	template <class Container>
	activeTable& join(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
	{
		doJoin<typename Container>(true, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8, name9, name10, name11);
		return *this;
	}

	template <class Container>
	activeTable& outerJoin(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
	{
		doJoin<typename Container>(false, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8, name9, name10, name11);
		return *this;

	}


};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_TRDORMAPI_H
