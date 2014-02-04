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




class cursor
{



protected:
	table_ptr m_table;

public:
    cursor(databaseManager& mgr, const _TCHAR* name)
	{
        m_table = mgr.table(name);
	}

    cursor(database_ptr& db, const _TCHAR* name)
    {
        m_table = openTable(db, name);
    }

    cursor(database* db, const _TCHAR* name)
    {
        m_table = openTable(db, name);
    }

    inline cursor& index(int v)
    {
        m_table->setKeyNum(v);
        return *this;
    }

    template <class T0>
	inline cursor& position(const T0 kv0)
	{
        keyValue<T0>::set(m_table, m_table->keyNum(), kv0);
        return *this;
	}

    template <class T0, class T1>
	inline cursor& position(const T0 kv0, const T1 kv1)
	{
        keyValue<T0, T1>::set(m_table, m_table->keyNum(), kv0, kv1);
        return *this;
	}

    template <class T0, class T1 , class T2>
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2)
	{
        keyValue<T0, T1, T2>::set(m_table, m_table->keyNum(), kv0, kv1, kv2);
        return *this;
	}

    template <class T0, class T1 , class T2, class T3>
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
	{
        keyValue<T0, T1, T2, T3>::set(m_table, m_table->keyNum(), kv0, kv1, kv2, kv3);
        return *this;
	}

    template <class T0, class T1 , class T2, class T3, class T4>
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
                            ,const T4 kv4)
	{
        keyValue<T0, T1, T2, T3, T4>
                ::set(m_table, m_table->keyNum(), kv0, kv1, kv2, kv3, kv4);
        return *this;
	}

    template <class T0, class T1 , class T2, class T3, class T4, class T5 >
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
                            ,const T4 kv4, const T5 kv5)
	{
        keyValue<T0, T1, T2, T3, T4, T5>
                ::set(m_table, m_table->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5);
        return *this;
	}

    template <class T0, class T1 , class T2, class T3, class T4, class T5 , class T6>
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
                            ,const T4 kv4, const T5 kv5, const T6 kv6)
	{
        keyValue<T0, T1, T2, T3, T4, T5, T6>
                ::set(m_table, m_table->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6);
        return *this;
	}

    template <class T0, class T1 , class T2, class T3
                ,class T4, class T5 , class T6 , class T7>
	inline cursor& position(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
                            ,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
	{
        keyValue<T0, T1, T2, T3, T4, T5, T6, T7>
                ::set(m_table, m_table->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
        return *this;
	}

    inline table_ptr table() const {return m_table;};

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
	query& and(const _TCHAR* name, const _TCHAR* type, T value)
	{
        if (m_wheres.size() == 0)
            throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

        addLogic(_T("and"), name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
 		return *this;
	}

	template <class T>
    query& or(const _TCHAR* name, const _TCHAR* type, T value)
	{
        if (m_wheres.size() == 0)
            throw bzs::rtl::exception(STATUS_FILTERSTRING_ERROR, _T("Invalid function call."));

        addLogic(_T("or"), name, logic, boost::lexical_cast<std::_tstring>(value).c_str());
		return *this;
	}

    template <class T>
    query& in(const _TCHAR* name, const _TCHAR* type, T value)
	{
        if (m_wheres.size() == 0)
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

/*
template <class Containor>
inline typename Containor::iterator begin(Containor& m){return m.begin();}

template <class Containor>
inline typename Containor::iterator end(Containor& m){return m.end();}

template <class Containor, class T>
inline void push_back(Containor& m, T c){return m.push_back(c);}
*/
template <class T>
inline std::vector<T>::iterator begin(std::vector<T>& m){return m.begin();}

template <class T>
inline std::vector<T>::iterator end(std::vector<T>& m){return m.end();}

template <class T>
inline void push_back(std::vector<T>& m, T c){return m.push_back(c);}

/* Collection operation handlter

*/
template <class MAP
            , class MDLSTYPE
            , class T=MAP::mdl_typename
            , class FDI=MAP::fdi_typename>
class mdlsHandler
{
    typedef std::vector<boost::shared_ptr<typename T> > collection_vec_type;

    mdlsHandler();
protected:
    MDLSTYPE& m_mdls;
    int m_option;
    FDI* m_fdi;
    MAP* m_map;



public:
    mdlsHandler(MDLSTYPE& mdls)
        :m_mdls(mdls){}

    virtual ~mdlsHandler(){};
    /* 初期化 */
    void init(int option, FDI* fdi, MAP& map)
    {
        m_option = option;
        m_fdi = fdi;
        m_map = &map;
    }

    template <class mdls_type>
    void addCollection(T* u, typename mdls_type::item_type* p )
    {
        mdls_type::item_type ptr(u);
        //m_mdls.push_back(ptr);
        push_back(m_mdls, ptr);
    }

    template <class mdls_type>
    void addCollection(T* u, ...)
    {
        //m_mdls.push_back(boost::shared_ptr<T>(u));
        push_back(m_mdls, boost::shared_ptr<T>(u));
    }

    void operator()(const fields& fds)
    {
        T* u(create(m_mdls));
        m_map->readMap(*u, fds, m_option);
        addCollection<MDLSTYPE>(u, 0);
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
    //return (l.*func1)() < (r.*func1)();

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

template <class MDLS, class FUNC1, class FUNC2, class FUNC3>
void sort(MDLS& mdls, FUNC1 func1, FUNC2 func2, FUNC3 func3 )
{
    sortFunctor<FUNC1, FUNC2, FUNC3> functor(func1, func2, func3);
    std::sort(begin(mdls), end(mdls), functor);
}

template <class MDLS, class FUNC1, class FUNC2>
void sort(MDLS& mdls, FUNC1 func1, FUNC2 func2)
{
    sortFunctor<FUNC1, FUNC2, FUNC1> functor(func1, func2, NULL);
    std::sort(begin(mdls), end(mdls), functor);
}

template <class MDLS, class FUNC1>
void sort(MDLS& mdls, FUNC1 func1)
{
    sortFunctor<FUNC1, FUNC1, FUNC1> functor(func1, NULL, NULL);
    std::sort(begin(mdls), end(mdls), functor);
}

template <class T2, class T, class MDLS>
inline boost::shared_ptr<std::vector<T> > listup(MDLS& mdls, T (T2::*func)()const)
{
    typename MDLS::iterator it = begin(mdls), ite = end(mdls);

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


template <class MAP, class T=MAP::mdl_typename, class FDI=MAP::fdi_typename>
class activeTable : boost::noncopyable
{
    typedef std::vector<boost::shared_ptr<T> > collection_vec_type;

    FDI* m_fdi;
    MAP m_map;
    int m_option;
    cursor m_cb;
    bool m_useTransactd;

    inline size_t size(collection_vec_type& mdls){return mdls.size();}
    inline T& getItem(collection_vec_type& mdls, unsigned int index){return *(mdls[index]);}

public:
    activeTable(databaseManager& mgr)
            :m_option(0)
            ,m_fdi(createFdi(m_fdi))
            ,m_map(*m_fdi)
            ,m_cb(mgr, m_map.getTableName())
            ,m_useTransactd(mgr.db()->isUseTransactd())
            {
                if (m_cb.table() && m_fdi)
                    initFdi(m_fdi, m_cb.table().get());
            }

    activeTable(database_ptr& db)
            :m_option(0)
            ,m_fdi(createFdi(m_fdi))
            ,m_map(*m_fdi)
            ,m_cb(db, m_map.getTableName())
            ,m_useTransactd(db->isUseTransactd())
            {
                if (m_cb.table() && m_fdi)
                    initFdi(m_fdi, m_cb.table().get());
            }

    ~activeTable(){destroyFdi(m_fdi);}

    cursor& cursor(){return m_cb;}

    activeTable& option(int v)
    {
        m_option = v;
        return *this;
    }

    template <class MDLSMAP>
    void readsBy(MDLSMAP& map, queryBase& q)
    {
        map.init(m_option, m_fdi, m_map);
        table_ptr tb = m_cb.table();

        tb->setQuery(&q);
        if (tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*tb));

        tb->find(q.getDirection());
        if (q.getDirection() == table::findForword)
        {
            findIterator itsf(*tb);
            for_each(itsf, map);
        }else
        {
            findRvIterator itsf(*tb);
            for_each(itsf, map);
        }
    }


    template <class MDLSMAP>
    void readsBy(MDLSMAP& map, queryBase& q, validationFunc func)
    {
        map.init(m_option, m_fdi, m_map);
        table_ptr tb = m_cb.table();
        tb->setQuery(&q);
        if (tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*tb));
        tb->find(q.getDirection());
        if (q.getDirection() == table::findForword)
        {
            findIterator itsf(*tb);
            filterdFindIterator it(itsf, func);
            for_each(it, map);
        }else
        {
            findRvIterator itsf(*tb);
            filterdFindRvIterator it(itsf, func);
            for_each(it, map);
        }
    }


    inline void reads(collection_vec_type& mdls, queryBase& q, validationFunc func)
    {
        mdlsHandler<MAP, collection_vec_type> map(mdls);
        readsBy(map, q, func);
    }

    inline void reads(collection_vec_type& mdls, queryBase& q)
    {
        mdlsHandler<MAP, collection_vec_type> map(mdls);
        readsBy(map, q);
    }

    template <class MDLS>
    inline void reads(MDLS& mdls, queryBase& q)
    {
        typename MAP::collection_orm_typename map(mdls);
        readsBy(map, q);
    }

    template <class MDLS>
    inline void reads(MDLS& mdls, queryBase& q, validationFunc func)
    {
        typename MAP::collection_orm_typename map(mdls);
        readsBy(map, q, func);
    }

    void read(T& mdl)
    {
        table_ptr tb = m_cb.table();
        indexIterator it = readIndex(tb, eSeekEqual);
        if (tb->stat() != 0)
            nstable::throwError(_T("activeTable read"), &(*tb));

        fields fds(tb);
        m_map.readMap(mdl, fds, m_option);
    }

    void update(T& mdl)
    {
        table_ptr tb = m_cb.table();
        indexIterator it = readIndex(tb, eSeekEqual);
        if (tb->stat() != 0)
            nstable::throwError(_T("activeTable update"), &(*tb));

        fields fds(tb);
        m_map.writeMap(mdl, fds, m_option);
        updateRecord(it);

    }

    void del()
    {
        table_ptr tb = m_cb.table();
        readIndex(tb, eSeekEqual);
        if (tb->stat() != 0)
            nstable::throwError(_T("activeTable delete"), &(*tb));
        tb->del();
    }

    //Recieve delete record by mdl
    void del(T& mdl)
    {
        read(mdl);
        m_cb.table()->del();
    }

    void insert(T& mdl)
    {

        fields fds(m_cb.table());
        m_map.writeMap(mdl, fds, m_option);
        insertRecord(fds);
        // set autoincrement value
        m_map.readAuntoincValue(mdl, fds, m_option);
    }

    void save(T& mdl)
    {
        table_ptr tb = m_cb.table();
        indexIterator it = readIndex(tb, eSeekEqual);
        if (it.tb().stat() == STATUS_NOT_FOUND_TI)
            insert(mdl);
        else
        {
            fields fds(tb);
            m_map.writeMap(mdl, fds, m_option);
            updateRecord(it);
        }
    }

    /* mdlsがキーフィールドに対応するメンバによってソート済の時は
       sortedをtrueにします。検索するレコードと通信量が激減します。
    */
    template <class MDLS>
    void readEach(MDLS& mdls, queryBase& q, bool sorted=false)
    {
        q.clearSeekKeyValues();
        table_ptr tb = m_cb.table();
        fields fds(tb);
        typename MDLS::iterator it = begin(mdls),itb = begin(mdls),ite = end(mdls);
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
                    || (m_map.compKeyValue(mdl, mdlb, tb->keyNum())==true)
                    || (m_map.compKeyValue(mdlb, mdl, tb->keyNum())==true))
            {
                m_map.setKeyValues(mdl, fds, tb->keyNum());
                keydef* kd = &tb->tableDef()->keyDefs[tb->keyNum()];
                for (int i=0;i<kd->segmentCount;++i)
                    q.addSeekKeyValue(fds[kd->segments[i].fieldNum].c_str());
            }
            mdlb = mdl;
            ++it;
        }
        tb->setQuery(&q);
        if (tb->stat() != 0)
            nstable::throwError(_T("activeTable readEach Query"), &(*tb));
        tb->find();
        //見つからないレコードがあると、その時点でエラーで返る
        //行ごとにエラーかどうかわかった方がよい。
        it = itb = begin(mdls);
        while(it != ite)
        {
            if (tb->stat() != 0)
                nstable::throwError(_T("activeTable readEach"), &(*tb));
            T& mdl = *(*it);
            if ((it != itb) &&
                (!sorted
                    || (m_map.compKeyValue(mdl, mdlb, tb->keyNum())==true)
                    || (m_map.compKeyValue(mdlb, mdl, tb->keyNum())==true)))
                tb->findNext();
            m_map.readMap(mdl, fds, m_option);
            mdlb = mdl;
            ++it;
        }

    }

    /** Join相当の処理を事前ソートして高速に行います。
    */
    template <class BASEMDLS, class T2>
    void readEach(BASEMDLS& mdls, T* (T2::*func)()const, queryBase& q)
    {
        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        table_ptr tb = m_cb.table();
        compFunc<MAP, T> comp(m_map, tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, q, true);
    }


    /* No use field select */
    template <class MDLS>
    void readEach(MDLS& mdls, bool sorted=false, bzs::rtl::exception* e=NULL)
    {
        table_ptr tb = m_cb.table();
        fields fds(tb);
        typename MDLS::iterator it = mdls.begin(),itb = mdls.begin(),ite = end(mdls);
        it = itb = begin(mdls);
        T& mdlb = *(*it);
        while(it != ite)
        {
            T& mdl = *(*it);
            if ((it == itb)
                    || !sorted
                    || (m_map.compKeyValue(mdl, mdlb, tb->keyNum())==true)
                    || (m_map.compKeyValue(mdlb, mdl, tb->keyNum())==true))
            {
                m_map.setKeyValues(mdl, fds, tb->keyNum());
                readIndex(tb, eSeekEqual);
                if (tb->stat() != 0)
                {
                    _TCHAR buf[4096];
                    table::keyValueDescription(tb.get(), buf, 4096);
                    if (e)
                        *e << bzs::rtl::errnoCode(tb->stat()) << bzs::rtl::errMessage(buf);
                    else
                        THROW_BZS_ERROR_WITH_CODEMSG(tb->stat(), buf);
                }
            }
            if (tb->stat() == 0)
                m_map.readMap(mdl, fds, m_option);
            mdlb = mdl;
            ++it;
        }
    }

    /* No use field select */
    template <class BASEMDLS, class T2>
    void readEach(BASEMDLS& mdls, T* (T2::*func)()const)
    {

        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        table_ptr tb = m_cb.table();
        compFunc<MAP, T> comp(m_map, tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, true, NULL);
    }

    template <class BASEMDLS, class T2>
    void readEach(BASEMDLS& mdls, T* (T2::*func)()const , bzs::rtl::exception& e)
    {

        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        table_ptr tb = m_cb.table();
        compFunc<MAP, T> comp(m_map, tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, true, &e);
    }



};

//template <class T2, class T, class MDLS>
//inline boost::shared_ptr<std::vector<T> > listup(MDLS& mdls, T (T2::*func)()const)


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//trdormapiH
