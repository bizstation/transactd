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
#include "fieldNameAlias.h"
#include "memRecord.h"
#include "groupComp.h"
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

/** @cond INTERNAL */
template <class Container>
typename Container::key_type resolvKeyValue(Container& m,
                                            const std::_tstring& name,
                                            bool noexception = false);

template <class Container> typename Container::iterator begin(Container& m);

template <class Container> typename Container::iterator end(Container& m);

template <class Container> void clear(Container& m);

template <class Container>
void push_back(Container& m, typename Container::row_type c);

template <class ROW_TYPE, class KEY_TYPE, class T>
void setValue(ROW_TYPE& row, KEY_TYPE key, const T& value);
/** @endcond */

/** @cond INTERNAL */

template <class T>
inline typename std::vector<T>::iterator begin(std::vector<T>& m)
{
    return m.begin();
}

template <class T>
inline typename std::vector<T>::iterator end(std::vector<T>& m)
{
    return m.end();
}

template <class T> inline void push_back(std::vector<T>& m, T c)
{
    return m.push_back(c);
}

#if (_MSC_VER || (__BCPLUSPLUS__ && !defined(__clang__)))

/* Container has readBefore(table_ptr, alias) function*/
template <class Container>
inline void readBefore(Container& mdls, table_ptr tb,
                       const aliasMap_type* alias,
                       typename Container::header_type* dummy = 0)
{
    mdls.readBefore(tb, alias);
}

/* Container has'nt readBefore(table_ptr, alias) function*/
template <class Container> inline void readBefore(...){};

#else

template <class Container>
void push_back(Container& m, typename Container::item_type c);

template <class T> class has_header
{
    typedef char yes;
    typedef struct
    {
        char foo[2];
    } no;

    template <class C> static yes test(typename C::header_type*);

    template <class C> static no test(...);

public:
#ifdef SWIG
    static const bool value;
#else
    static const bool value = sizeof(test<T>(0)) == sizeof(char);
#endif
};

/* Container has readBefore(table_ptr, alias) function*/
template <class Container>
inline void
readBefore(Container& mdls, table_ptr tb, const aliasMap_type* alias,
           typename boost::enable_if<has_header<Container> >::type* = 0)

{
    mdls.readBefore(tb, alias);
}

/* Container has'nt readBefore(table_ptr, alias) function*/
template <class Container>
inline void
readBefore(Container& mdls, table_ptr tb, const aliasMap_type* alias,
           typename boost::disable_if<has_header<Container> >::type* = 0)
{
}
#endif

/* Container operation handlter

*/
template <class MAP, class Container, class T = typename MAP::mdl_typename,
          class FDI = typename MAP::fdi_typename>
class mdlsHandler
{
    mdlsHandler();

protected:
    Container& m_mdls;
    int m_option;
    FDI* m_fdi;
    MAP* m_map;

    template <class mdls_type>
    void addContainer(T* u, typename mdls_type::item_type* p)
    {
        typename mdls_type::item_type ptr(u);
        push_back(m_mdls, ptr);
    }

    template <class mdls_type> void addContainer(T* u, ...)
    {
        push_back(m_mdls, u);
    }

public:
    mdlsHandler(Container& mdls) : m_mdls(mdls) {}

    virtual ~mdlsHandler(){};

    void init(int option, FDI* fdi, MAP& map, table_ptr tb,
              const aliasMap_type* alias = NULL)
    {
        m_option = option;
        m_fdi = fdi;
        m_map = &map;
        readBefore<Container>(m_mdls, tb, alias);
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
template <class MAP, class T> class compFunc
{
    MAP& m_map;
    int m_keynum;

public:
    compFunc(MAP& map, int keynum) : m_map(map), m_keynum(keynum) {}
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
bool sortFuncBase(const T& l, const T& r, RET (T::*func1)() const)
{
    RET retl = (l.*func1)();
    RET retr = (r.*func1)();
    return retl < retr;
}

template <class T, class FUNC1, class FUNC2, class FUNC3>
bool sortFunc(const T& l, const T& r, FUNC1 func1, FUNC2 func2, FUNC3 func3)
{
    bool v = sortFuncBase(l, r, func1);
    if (func2)
    {
        if (v)
            return v;
        v = sortFuncBase(r, l, func1);
        if (v)
            return !v;
        v = sortFuncBase(l, r, func2);
        if (func3)
        {
            if (v)
                return v;
            v = sortFuncBase(r, l, func2);
            if (v)
                return !v;
            v = sortFuncBase(l, r, func3);
        }
    }
    return v;
}

template <class FUNC1, class FUNC2, class FUNC3> class sortFunctor
{
    FUNC1 m_func1;
    FUNC2 m_func2;
    FUNC3 m_func3;

public:
    sortFunctor(FUNC1 func1, FUNC2 func2, FUNC3 func3)
        : m_func1(func1), m_func2(func2), m_func3(func3)
    {
    }
    template <class T> bool operator()(const T* l, const T* r) const
    {
        return sortFunc(*l, *r, m_func1, m_func2, m_func2);
    }

    template <class T>
    bool operator()(const boost::shared_ptr<T>& l,
                    const boost::shared_ptr<T>& r) const
    {
        bool v = sortFunc(*l, *r, m_func1, m_func2, m_func2);
        return v;
    }
};

template <class Container, class FUNC1, class FUNC2, class FUNC3>
void sort(Container& mdls, FUNC1 func1, FUNC2 func2, FUNC3 func3)
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

template <class Container, class FUNC1> void sort(Container& mdls, FUNC1 func1)
{
    sortFunctor<FUNC1, FUNC1, FUNC1> functor(func1, NULL, NULL);
    std::sort(begin(mdls), end(mdls), functor);
}

template <class T2, class T, class Container>
inline boost::shared_ptr<std::vector<T> > listup(Container& mdls,
                                                 T (T2::*func)() const)
{
    typename Container::iterator it = begin(mdls), ite = end(mdls);

    boost::shared_ptr<std::vector<T> > mdlst(new std::vector<T>());
    while (it != ite)
    {
        T2& mdl = *(*it);
        T ref = (mdl.*func)();
        mdlst->push_back(ref);
        ++it;
    }
    return mdlst;
}

class mraResetter
{
    table_ptr& m_tb;

public:
    mraResetter(table_ptr& tb) : m_tb(tb) {}

    ~mraResetter()
    {
        if (m_tb->mra())
            m_tb->mra()->setJoinType(mra_nojoin);
        m_tb->setMra(NULL);
    }
};

/** @endcond */

template <class MAP, class T = typename MAP::mdl_typename,
          class FDI = typename MAP::fdi_typename>
class activeObject : boost::noncopyable
{
    void init(idatabaseManager* mgr, const _TCHAR* name)
    {
        m_tb = mgr->table(name);
    }

    void init(database_ptr& db, const _TCHAR* name)
    {
        m_tb = openTable(db, name);
    }

    void init(database* db, const _TCHAR* name) { m_tb = openTable(db, name); }

protected:
    table_ptr m_tb;
    FDI* m_fdi;
    MAP m_map;
    int m_option;
    fdNmaeAlias m_alias;

public:
    typedef std::vector<boost::shared_ptr<T> > collection_vec_type;

    explicit activeObject(idatabaseManager* mgr)
        : m_fdi(createFdi((FDI*)0)), m_map(*m_fdi), m_option(0)
    {
        init(mgr, m_map.getTableName());
        if (table() && m_fdi)
            initFdi(m_fdi, m_tb.get());
    }

    explicit activeObject(database_ptr& db)
        : m_fdi(createFdi((FDI*)0)), m_map(*m_fdi), m_option(0)
    {
        init(db, m_map.getTableName());
        if (table() && m_fdi)
            initFdi(m_fdi, m_tb.get());
    }

    explicit activeObject(idatabaseManager* mgr, const _TCHAR* tableName)
        : m_fdi(createFdi((FDI*)0)), m_map(*m_fdi), m_option(0)
    {
        init(mgr, tableName);
        if (table() && m_fdi)
            initFdi(m_fdi, m_tb.get());
    }

    /*explicit activeObject(dbmanager_ptr& mgr, const _TCHAR* tableName)
            :m_option(0)
            ,m_fdi(createFdi(m_fdi))
            ,m_map(*m_fdi)
            {
                    init(mgr, tableName);
                    if (table() && m_fdi)
                            initFdi(m_fdi, m_tb.get());
            }
     */

    explicit activeObject(database_ptr& db, const _TCHAR* tableName)
        : m_fdi(createFdi((FDI*)0)), m_map(*m_fdi), m_option(0)
    {
        init(db, tableName);
        if (table() && m_fdi)
            initFdi(m_fdi, m_tb.get());
    }

    explicit activeObject(database* db, const _TCHAR* tableName)
        : m_fdi(createFdi((FDI*)0)), m_map(*m_fdi), m_option(0)
    {
        init(db, tableName);
        if (table() && m_fdi)
            initFdi(m_fdi, m_tb.get());
    }

    ~activeObject() { destroyFdi(m_fdi); }

    activeObject& index(int v)
    {
        m_tb->clearBuffer();
        m_tb->setKeyNum(v);
        return *this;
    }

    /** @cond INTERNAL */

    template <class T0> activeObject& keyValue(const T0 kv0)
    {
        keyValueSetter<T0>::set(m_tb, m_tb->keyNum(), kv0);
        return *this;
    }

    template <class T0, class T1>
    activeObject& keyValue(const T0 kv0, const T1 kv1)
    {
        keyValueSetter<T0, T1>::set(m_tb, m_tb->keyNum(), kv0, kv1);
        return *this;
    }

    template <class T0, class T1, class T2>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2)
    {
        keyValueSetter<T0, T1, T2>::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2);
        return *this;
    }

    template <class T0, class T1, class T2, class T3>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                           const T3 kv3)
    {
        keyValueSetter<T0, T1, T2, T3>::set(m_tb, m_tb->keyNum(), kv0, kv1, kv2,
                                            kv3);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                           const T3 kv3, const T4 kv4)
    {
        keyValueSetter<T0, T1, T2, T3, T4>::set(m_tb, m_tb->keyNum(), kv0, kv1,
                                                kv2, kv3, kv4);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4, class T5>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                           const T3 kv3, const T4 kv4, const T5 kv5)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5>::set(m_tb, m_tb->keyNum(), kv0,
                                                    kv1, kv2, kv3, kv4, kv5);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4, class T5,
              class T6>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                           const T3 kv3, const T4 kv4, const T5 kv5,
                           const T6 kv6)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(
            m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6);
        return *this;
    }

    /** @endcond */

    template <class T0, class T1, class T2, class T3, class T4, class T5,
              class T6, class T7>
    activeObject& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                           const T3 kv3, const T4 kv4, const T5 kv5,
                           const T6 kv6, const T7 kv7)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
            m_tb, m_tb->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
        return *this;
    }

    inline table_ptr table() const { return m_tb; };

    activeObject& option(int v)
    {
        m_option = v;
        return *this;
    }

    template <class Any_Map_type>
    activeObject& readMap(Any_Map_type& map, queryBase& q)
    {
        mraResetter mras(m_tb);
        m_alias.reverseAliasNamesQuery(q);
        m_tb->setQuery(&q);
        if (m_tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*m_tb));

        map.init(m_option, m_fdi, m_map, m_tb, &m_alias);
        m_tb->find(q.getDirection());
        if (q.getDirection() == table::findForword)
        {
            findIterator itsf(*m_tb);
            for_each(itsf, map);
        }
        else
        {
            findRvIterator itsf(*m_tb);
            for_each(itsf, map);
        }
        return *this;
    }

    template <class Any_Map_type>
    activeObject& readMap(Any_Map_type& map, queryBase& q, validationFunc func)
    {
        mraResetter mras(m_tb);
        m_alias.reverseAliasNamesQuery(q);

        m_tb->setQuery(&q);
        if (m_tb->stat())
            nstable::throwError(_T("Query is inaccurate"), &(*m_tb));
        map.init(m_option, m_fdi, m_map, m_tb, &m_alias);
        m_tb->find(q.getDirection());
        if (q.getDirection() == table::findForword)
        {
            findIterator itsf(*m_tb);
            filterdFindIterator it(itsf, func);
            for_each(it, map);
        }
        else
        {
            findRvIterator itsf(*m_tb);
            filterdFindRvIterator it(itsf, func);
            for_each(it, map);
        }
        return *this;
    }

    activeObject& read(collection_vec_type& mdls, queryBase& q,
                       validationFunc func)
    {
        mdlsHandler<MAP, collection_vec_type> map(mdls);
        return readMap(map, q, func);
    }

    activeObject& read(collection_vec_type& mdls, queryBase& q)
    {
        mdlsHandler<MAP, collection_vec_type> map(mdls);
        return readMap(map, q);
    }

    template <class Container> activeObject& read(Container& mdls, queryBase& q)
    {
        typename MAP::collection_orm_typename map(mdls);
        return readMap(map, q);
    }

    template <class Container>
    activeObject& read(Container& mdls, queryBase& q, validationFunc func)
    {
        typename MAP::collection_orm_typename map(mdls);
        return readMap(map, q, func);
    }

    template <class T2> void read(T2& mdl, bool setKeyValueFromObj = true)
    {
        fields fds(m_tb);
        if (setKeyValueFromObj)
            m_map.setKeyValues(mdl, fds, m_tb->keyNum());
        indexIterator it = readIndex(m_tb, eSeekEqual);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject read"), &(*m_tb));
        m_map.readMap(mdl, fds, m_option);
    }

    template <class T2> void update(T2& mdl, bool setKeyValueFromObj = true)
    {
        fields fds(m_tb);
        if (setKeyValueFromObj)
            m_map.setKeyValues(mdl, fds, m_tb->keyNum());
        indexIterator it = readIndex(m_tb, eSeekEqual);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject update"), &(*m_tb));

        m_map.writeMap(mdl, fds, m_option);
        updateRecord(it);
    }

    // No need object
    void del()
    {
        readIndex(m_tb, eSeekEqual);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject delete"), &(*m_tb));
        m_tb->del();
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject delete"), &(*m_tb));
    }

    // Recieve delete record by mdl
    template <class T2> void del(T2& mdl, bool setKeyValueFromObj = true)
    {
        read(mdl, setKeyValueFromObj);
        m_tb->del();
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject delete"), &(*m_tb));
    }

    template <class T2> void insert(T2& mdl)
    {
        fields fds(m_tb);
        m_map.writeMap(mdl, fds, m_option);
        insertRecord(fds);
        m_map.readAuntoincValue(mdl, fds, m_option);
    }

    template <class T2> void save(T2& mdl, bool setKeyValueFromObj = true)
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

#ifdef USE_CONTAINER_CUD // default not support

    template <class Container> void update(Container& mdls)
    {
        typename Container::iterator it = begin(mdls), ite = end(mdls);
        while (it != ite)
            update(*it);
    }

    template <class Container> void del(Container& mdls)
    {
        typename Container::iterator it = begin(mdls), ite = end(mdls);
        while (it != ite)
            del(*it);
    }

    template <class Container> void insert(Container& mdls)
    {
        typename Container::iterator it = begin(mdls), ite = end(mdls);
        while (it != ite)
            insert(*it);
    }

#endif

    template <class Container>
    void readEach(Container& mdls, queryBase& q, bool sorted = false,
                  bzs::rtl::exception * e = NULL)
    {
        mraResetter mras(m_tb);
        m_alias.reverseAliasNamesQuery(q);
        fields fds(m_tb);
        typename Container::iterator it = begin(mdls), itb = begin(mdls),
                                     ite = end(mdls);
        it = itb = begin(mdls);
        T& mdlb = *(*it);
        if (!m_tb->isUseTransactd())
            nstable::throwError(_T("activeObject P.SQL can not use this"),
                                (short_td)0);
        while (it != ite)
        {
            // if mdl has same key value, to be once read access to server
            T& mdl = *(*it);
            if ((it == itb) || !sorted ||
                (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum()) == true) ||
                (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum()) == true))
            {
                m_map.setKeyValues(mdl, fds, m_tb->keyNum());
                keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];
                for (int i = 0; i < kd->segmentCount; ++i)
                    q.addSeekKeyValue(fds[kd->segments[i].fieldNum].c_str());
            }
            mdlb = mdl;
            ++it;
        }
        m_tb->setQuery(&q);
        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject readEach Query"), &(*m_tb));
        m_tb->find();

        it = itb = begin(mdls);
        while (it != ite)
        {
            if ((m_tb->stat() != STATUS_SUCCESS) &&
                (m_tb->stat() != STATUS_NOT_FOUND_TI))
                nstable::throwError(_T("activeObject readEach"), &(*m_tb));
            T& mdl = *(*it);
            if ((it != itb) &&
                (!sorted ||
                 (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum()) == true) ||
                 (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum()) == true)))
            {
                m_tb->findNext();
                if (m_tb->stat() != 0)
                {
                    _TCHAR buf[8192];
                    m_tb->keyValueDescription(buf, 8192);
                    if (e)
                        *e << bzs::rtl::errnoCode(m_tb->stat())
                           << bzs::rtl::errMessage(buf);
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

    template <class BaseContainer, class T2>
    void readEach(BaseContainer& mdls, T* (T2::*func)() const, queryBase& q)
    {
        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        compFunc<MAP, T> comp(m_map, m_tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, q, true, NULL);
    }

    template <class BaseContainer, class T2>
    void readEach(BaseContainer& mdls, T* (T2::*func)() const, queryBase& q,
                  bzs::rtl::exception& e)
    {
        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        compFunc<MAP, T> comp(m_map, m_tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, q, true, &e);
    }

    /* No use field select */
    template <class Container>
    void readEach(Container& mdls, bool sorted = false,
                  bzs::rtl::exception * e = NULL)
    {
        fields fds(m_tb);
        mraResetter mras(m_tb);
        typename Container::iterator it = begin(mdls), itb = begin(mdls),
                                     ite = end(mdls);
        it = itb = begin(mdls);
        T& mdlb = *(*it);
        while (it != ite)
        {
            T& mdl = *(*it);
            if ((it == itb) || !sorted ||
                (m_map.compKeyValue(mdl, mdlb, m_tb->keyNum()) == true) ||
                (m_map.compKeyValue(mdlb, mdl, m_tb->keyNum()) == true))
            {
                m_map.setKeyValues(mdl, fds, m_tb->keyNum());
                readIndex(m_tb, eSeekEqual);
                if (m_tb->stat() != 0)
                {
                    _TCHAR buf[8192];
                    m_tb->keyValueDescription(buf, 8192);
                    if (e)
                        *e << bzs::rtl::errnoCode(m_tb->stat())
                           << bzs::rtl::errMessage(buf);
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
    void readEach(BaseContainer& mdls, T* (T2::*func)() const)
    {
        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        compFunc<MAP, T> comp(m_map, m_tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, true, NULL);
    }

    template <class BaseContainer, class T2>
    void readEach(BaseContainer& mdls, T* (T2::*func)() const,
                  bzs::rtl::exception& e)
    {
        boost::shared_ptr<std::vector<T*> > refList(listup(mdls, func));
        compFunc<MAP, T> comp(m_map, m_tb->keyNum());
        std::sort(refList->begin(), refList->end(), comp);
        readEach(*refList, true, &e);
    }

    inline activeObject& alias(const _TCHAR* src, const _TCHAR* dst)
    {
        m_alias.set(src, dst);
        return *this;
    }

    inline activeObject& resetAlias()
    {
        m_alias.clear();
        return *this;
    }
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_TRDORMAPI_H
