#ifndef trdormapiH
#define trdormapiH
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
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <vector>
#include <tstring.h>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

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

class logic
{
    std::_tstring m_name;
	std::_tstring m_value;
	std::_tstring m_type;
	combineType m_next;

public:

	logic(const _TCHAR* name, const _TCHAR* type, const _TCHAR* value, combineType next)
		:m_name(name),m_type(type),m_value(value),m_next(next){}
	logic(const _TCHAR* name, const _TCHAR* type, int value, combineType next)
		:m_name(name),m_type(type),m_next(next)
		{
			_TCHAR buf[50];
			m_value = _ltot(value, buf, 10);
		}
	logic(const _TCHAR* name, const _TCHAR* type, __int64 value, combineType next)
		:m_name(name),m_type(type),m_next(next)
		{
			_TCHAR buf[50];
			m_value = _i64tot(value, buf, 10);
		}
	logic(const _TCHAR* name, const _TCHAR* type, double value, combineType next)
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
	query& where(const _TCHAR* name, const _TCHAR* logic, T value)
	{
        addLogic(name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}

    template <class T>
	query& and(const _TCHAR* name, const _TCHAR* logic, T value)
	{
        if (whereTokens() == 0)
            throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

        addLogic(_T("and"), name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
 		return *this;
	}

	template <class T>
    query& or(const _TCHAR* name, const _TCHAR* logic, T value)
	{
        if (whereTokens() == 0)
            throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

        addLogic(_T("or"), name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}

    template <class T>
    query& in(const _TCHAR* name, const _TCHAR* logic, T value)
	{
        if (whereTokens() == 0)
            throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

        addLogic(_T("or"), name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
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
inline std::vector<T>::iterator begin(std::vector<T>& m){return m.begin();}

template <class T>
inline std::vector<T>::iterator end(std::vector<T>& m){return m.end();}

template <class T>
inline void push_back(std::vector<T>& m, T c){return m.push_back(c);}



/* Container has readBefore(table_ptr) function*/
template <class Container>
void readBefore(Container::header_type* dummy, Container& mdls, table_ptr tb)
{
    mdls.readBefore(tb);
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

    void init(int option, FDI* fdi, MAP& map, table_ptr tb)
    {
        m_option = option;
        m_fdi = fdi;
        m_map = &map;

        readBefore<Container>(0, m_mdls, tb);
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


template <class T, class RET>
bool sortFuncBase(T&l, T& r , RET (T::*func1)() const)
{
    RET retl = (l.*func1)();
    RET retr = (r.*func1)();
    return retl < retr;
}

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
//#define STD_BINARY_SERCH

//------------------------------------------------------------------------------

template < class MAP, class Contatiner>
class grouping_comp
{
    MAP& m_map;
    const std::vector<typename Contatiner::group_key_type>& m_keys;
    Contatiner& m_mdls;


public:
    grouping_comp(MAP& map, Contatiner& mdls
            , const std::vector<typename Contatiner::group_key_type>& keys)
        :m_map(map),m_mdls(mdls),m_keys(keys) {}

    #ifdef STD_BINARY_SERCH
    bool operator() (int lv, int rv) const
    {
        const typename MAP::mdl_pure_typename& lm = m_mdls[lv] ;
        const typename MAP::mdl_pure_typename& rm = m_mdls[rv] ;
        for (int i=0;i<m_keys.size();++i)
        {
            const Contatiner::group_key_type& s = m_keys[i];
            int ret = m_map.compByGroupKey(*lm, *rm, s);
            if (ret) return (ret < 0);
        }
        return false;
    }
    #else

    int operator() (int lv, int rv) const
    {
        const typename MAP::mdl_pure_typename& lm = m_mdls[lv] ;
        const typename MAP::mdl_pure_typename& rm = m_mdls[rv] ;
        for (int i=0;i<m_keys.size();++i)
        {
            const Contatiner::group_key_type& s = m_keys[i];
            int ret = m_map.compByGroupKey(*lm, *rm, s);
            if (ret) return ret;
        }
        return 0;
    }
    #endif

    bool isEqual(const typename MAP::mdl_pure_typename& lm
                    , const typename MAP::mdl_pure_typename& rm)
	{
		for (int i=0;i< m_keys.size();++i)
		{
			const Contatiner::group_key_type& s = m_keys[i];
            int ret = m_map.compByGroupKey(*lm, *rm, s);
            if (ret) return false;
		}
		return true;
	}
};




class groupQuery
{
    std::vector<std::_tstring> m_keyFields;
    std::_tstring m_targetField;
    const query* m_having;

     /* remove none grouping fields */
    template <class MAP>
	void removeFileds(typename MAP::mdl_pure_typename m)
	{
		/*var tgtName = func.fieldName;
		var obj = new Object();
		obj[tgtName] = data[tgtName];
		for (key in data)
		{
			for (var i=0;i< fields.length;++i)
			{
				if (key === fields[i])
					obj[key] = data[key];
			}
		} */
		//return obj;
	}

    template <class Container, class FUNC>
    int binary_search(int key, const Container& a
            , int left, int right, FUNC func, bool& find)
    {
        find = false;
        if (right == 0) return 0; // no size

        int mid, tmp, end = right;
        while(left <= right)
        {
            mid = (left + right) / 2;
            if (mid >= end)
                return end;
            if ((tmp = func(a[mid], key)) == 0)
            {
                find = true;
                return mid;
            }
            else if (tmp < 0)
                left = mid + 1;  //keyValue is more large
            else
                right = mid - 1; //keyValue is more small
        }
        return left;
    }

public:

    groupQuery& keyField(const TCHAR* name, const TCHAR* name1=NULL, const TCHAR* name2=NULL, const TCHAR* name3=NULL
                ,const TCHAR* name4=NULL, const TCHAR* name5=NULL, const TCHAR* name6=NULL, const TCHAR* name7=NULL
                ,const TCHAR* name8=NULL, const TCHAR* name9=NULL, const TCHAR* name10=NULL)
    {
        m_keyFields.clear();
        m_keyFields.push_back(name);
        if (name1) m_keyFields.push_back(name1);
        if (name2) m_keyFields.push_back(name2);
        if (name3) m_keyFields.push_back(name3);
        if (name4) m_keyFields.push_back(name4);
        if (name5) m_keyFields.push_back(name5);
        if (name6) m_keyFields.push_back(name6);
        if (name7) m_keyFields.push_back(name7);
        if (name8) m_keyFields.push_back(name8);
        if (name9) m_keyFields.push_back(name9);
        if (name10) m_keyFields.push_back(name10);
        return *this;
    }

    groupQuery& targetField(const _TCHAR* name)
    {
        m_targetField = name;
        return *this;
    }

    groupQuery& having(const query& q) {m_having = &q;return *this;}
	const std::vector<std::_tstring>& getKeyFields()const {return m_keyFields;}
	const std::_tstring& getTTeyFields()const {return m_targetField;}
	const query& getHaving() const {return *m_having;}

    template <class MAP, class Container, class FUNC>
    void grouping(MAP& map, Container& mdls,  FUNC func)
    {
        std::vector<typename Container::group_key_type> keyFields;
        /*if (typeid(typename Contatiner::group_key_type) == typeid(std::_tstring))
        	keyFields = m_keyFields;
        else*/
        {
	        for (int i=0;i<m_keyFields.size();++i)
	        	keyFields.push_back(mdls.keyTypeValue(m_keyFields[i]));
		}
        typename Container::group_key_type targetFiled = mdls.keyTypeValue(m_targetField, true);

		grouping_comp<typename MAP, typename Container> groupingComp(map, mdls, keyFields);

        std::vector<int> index;
        typename Container::iterator it = mdls.begin(),ite = mdls.end();

        std::vector<FUNC> funcs;

        int i,n = 0;
        #ifndef STD_BINARY_SERCH
		while(it != ite)
        {
            bool found = false;
            i = binary_search(n, index, 0, index.size(), groupingComp, found);
            if (!found)
            {
                index.insert(index.begin() + i, n);
                funcs.insert(funcs.begin() + i, FUNC());
            }
            funcs[i]((*(*it))[targetFiled]);
            ++n;
            ++it;
		}

        #else
		while(it != ite)
        {
            std::vector<int>::iterator p = lower_bound(index.begin(), index.end(), n, groupingComp);
            i = p - index.begin() ;
            if (!std::binary_search(index.begin(), index.end(), n, groupingComp))
            {
                index.insert(p, n);
                if (i<0) i= 0;
                funcs.insert(funcs.begin() + i, FUNC());
            }
            funcs[i]((*(*it))[targetFiled]);
            ++n;
            ++it;
		}
        #endif
        //real sort by index
        typename Container c(mdls);
        mdls.rows().clear();
        for (int i=0;i<index.size();++i)
        {
            typename MAP::mdl_pure_typename cur = c[index[i]];
            (*cur)[targetFiled] = funcs[i].result();
            mdls.push_back(cur);
        }


    }
};
//------------------------------------------------------------------------------


template <class MAP, class T=MAP::mdl_typename, class FDI=MAP::fdi_typename>
class activeTable : boost::noncopyable
{
    typedef std::vector<boost::shared_ptr<T> > collection_vec_type;
    table_ptr m_tb;
    FDI* m_fdi;
    MAP m_map;
    int m_option;
    bool m_useTransactd;

    inline size_t size(collection_vec_type& mdls){return mdls.size();}
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


    ~activeTable(){destroyFdi(m_fdi);}

    inline void beginBulkInsert(int maxBuflen){m_tb->beginBulkInsert(maxBuflen);}
    inline void abortBulkInsert(){m_tb->abortBulkInsert();}
    inline ushort_td commitBulkInsert() {m_tb->commitBulkInsert();}

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
        m_tb->setQuery(&q);
        if (m_tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*m_tb));

        map.init(m_option, m_fdi, m_map, m_tb);
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
        m_tb->setQuery(&q);
        if (m_tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*m_tb));
        map.init(m_option, m_fdi, m_map, m_tb);
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

    void read(T& mdl, bool setKeyValueFromObj=true)
    {
        fields fds(m_tb);
        if (setKeyValueFromObj)
            m_map.setKeyValues(mdl, fds, m_tb->keyNum());
        indexIterator it = readIndex(m_tb, eSeekEqual);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeTable read"), &(*m_tb));
        m_map.readMap(mdl, fds, m_option);
    }

    void update(T& mdl, bool setKeyValueFromObj=true)
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

    template <class Container>
    void update(Container& mdls)
    {
        typename Container::iterator it = begin(mdls),ite = end(mdls);
        while (it != ite)
            update(*it);

    }

    // No need object
    void del()
    {
        readIndex(m_tb, eSeekEqual);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeTable delete"), &(*m_tb));
        m_tb->del();
    }

    //Recieve delete record by mdl
    void del(T& mdl, bool setKeyValueFromObj=true)
    {
        read(mdl, setKeyValueFromObj);
        m_tb->del();
    }

    template <class Container>
    void del(Container& mdls)
    {
        typename Container::iterator it = begin(mdls),ite = end(mdls);
        while (it != ite)
            del(*it);

    }

    void insert(T& mdl)
    {
        fields fds(m_tb);
        m_map.writeMap(mdl, fds, m_option);
        insertRecord(fds);
        m_map.readAuntoincValue(mdl, fds, m_option);
    }

    template <class Container>
    void insert(Container& mdls)
    {
        typename Container::iterator it = begin(mdls),ite = end(mdls);
        while (it != ite)
            insert(*it);

    }

    void save(T& mdl, bool setKeyValueFromObj=true)
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
        q.clearSeekKeyValues();
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

    /* No use field select */
    template <class Container>
    void readEach(Container& mdls, bool sorted=false, bzs::rtl::exception* e=NULL)
    {
        fields fds(m_tb);
        typename Container::iterator it = mdls.begin(),itb = mdls.begin(),ite = end(mdls);
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

    template <class Container, class FUNC>
    activeTable& groupBy(Container& mdls, groupQuery& q, FUNC func)
    {
        q.grouping<MAP, typename Container, typename FUNC>(m_map, mdls, func);

    }

public:
    template <class Container>
    void join(Container& mdls, queryBase& q, const _TCHAR* name1
                    , const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
                    , const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
                    , const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
                    , const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
                    , const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
    {
        if (!m_useTransactd)
            nstable::throwError(_T("activeTable P.SQL can not use this"), (short_td)0);
        q.clearSeekKeyValues();
        fields fds(m_tb);

        //DWORD t = timeGetTime();

        typename Container::iterator it = mdls.begin(),ite = mdls.end();
        //前回の結果セットから目的のフィールドを取り出す
        //フィールドの名前は解るので名前から番号を引く
        //型はフィールドタイプvalueとする
        typename Container::group_key_type key[11];
        if (name1) key[0] = mdls.keyTypeValue(name1);
        if (name2) key[1] = mdls.keyTypeValue(name2);
        if (name3) key[2] = mdls.keyTypeValue(name3);
        if (name4) key[3] = mdls.keyTypeValue(name4);
        if (name5) key[4] = mdls.keyTypeValue(name5);
        if (name6) key[5] = mdls.keyTypeValue(name6);
        if (name7) key[6] = mdls.keyTypeValue(name7);
        if (name8) key[7] = mdls.keyTypeValue(name8);
        if (name9) key[8] = mdls.keyTypeValue(name9);
        if (name10) key[9] = mdls.keyTypeValue(name10);
        if (name11) key[10] = mdls.keyTypeValue(name11);

        _TCHAR tmp[1024];
        while(it != ite)
        {
            T& mdl = *(*it);
            if (name1) q.addSeekKeyValue(toString(mdl[key[0]], tmp, 1024));
            if (name2) q.addSeekKeyValue(toString(mdl[key[1]], tmp, 1024));
            if (name3) q.addSeekKeyValue(toString(mdl[key[2]], tmp, 1024));
            if (name4) q.addSeekKeyValue(toString(mdl[key[3]], tmp, 1024));
            if (name5) q.addSeekKeyValue(toString(mdl[key[4]], tmp, 1024));
            if (name6) q.addSeekKeyValue(toString(mdl[key[5]], tmp, 1024));
            if (name7) q.addSeekKeyValue(toString(mdl[key[6]], tmp, 1024));
            if (name8) q.addSeekKeyValue(toString(mdl[key[7]], tmp, 1024));
            if (name9) q.addSeekKeyValue(toString(mdl[key[8]], tmp, 1024));
            if (name10) q.addSeekKeyValue(toString(mdl[key[9]], tmp, 1024));
            if (name11) q.addSeekKeyValue(toString(mdl[key[10]], tmp, 1024));
            ++it;
        }
        /*
        while(it != ite)
        {
            T& mdl = *(*it);
            if (name1) q.addSeekKeyValue(toString(mdl[key[0]]).c_str());
            if (name2) q.addSeekKeyValue(toString(mdl[key[1]]).c_str());
            if (name3) q.addSeekKeyValue(toString(mdl[key[2]]).c_str());
            if (name4) q.addSeekKeyValue(toString(mdl[key[3]]).c_str());
            if (name5) q.addSeekKeyValue(toString(mdl[key[4]]).c_str());
            if (name6) q.addSeekKeyValue(toString(mdl[key[5]]).c_str());
            if (name7) q.addSeekKeyValue(toString(mdl[key[6]]).c_str());
            if (name8) q.addSeekKeyValue(toString(mdl[key[7]]).c_str());
            if (name9) q.addSeekKeyValue(toString(mdl[key[8]]).c_str());
            if (name10) q.addSeekKeyValue(toString(mdl[key[9]]).c_str());
            if (name11) q.addSeekKeyValue(toString(mdl[key[10]]).c_str());
            ++it;
        } */
        //t = timeGetTime() - t;
        //std::tcout  << _T("--addSeekValue-- time = ")  << t <<   _T("\n");

        //t = timeGetTime();

        m_tb->setQuery(&q);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeTable readEach Query"), &(*m_tb));

        //t = timeGetTime() - t;
        //std::tcout  << _T("--join setQuery-- time = ")  << t <<   _T("\n");
        //t = timeGetTime();

        int offset = mdls.header().size();
        typename MAP::collection_orm_typename map(mdls);
        map.init(m_option, m_fdi, m_map, m_tb);
        m_tb->find();
        //見つからないレコードがあると、その時点でエラーで返る
        //行ごとにエラーかどうかわかった方がよい。
        it = mdls.begin();

        while(it != ite)
        {
            if ((m_tb->stat() != STATUS_SUCCESS)
                        && (m_tb->stat() != STATUS_NOT_FOUND_TI))
                   nstable::throwError(_T("activeTable join"), &(*m_tb));
            T& mdl = *(*it);
            if (m_tb->stat() != 0)
            {
                _TCHAR buf[8192];
                m_tb->keyValueDescription(buf, 8192);
                //if (e)
                //    *e << bzs::rtl::errnoCode(m_tb->stat()) << bzs::rtl::errMessage(buf);
                //else
                int n = fds.inproc_size();
                for (int i=0;i<n;++i)
                    (*(*it)).push_back(var_type((int)0));
                    //THROW_BZS_ERROR_WITH_CODEMSG(m_tb->stat(), buf);
            }


            if (m_tb->stat() == 0)
                m_map.readMap(*(*it), fds, offset);
            m_tb->findNext();
            ++it;
        }

        //t = timeGetTime() - t;
        //std::tcout  << _T("--join read-- time = ")  << t <<   _T("\n");

    }
};





}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//trdormapiH
