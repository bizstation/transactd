#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_TRDBOOSTAPI_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_TRDBOOSTAPI_H
/*=================================================================
   Copyright (C) 2013 2014 BizStation Corp All rights reserved.

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
#include "trdboostapiInternal.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <iterator>
#include <stdio.h>
#include <bzs/db/protocol/tdap/uri.h>

#if defined(__GNUC__)
#if (!defined(__clang__) &&                                                    \
     ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 3)))
#error "Your GCC version is too old. 4.3 or later versions are required."
#endif
#endif

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

enum eIndexOpType
{
    eSeekEqual,
    eSeekFirst,
    eSeekLast,
    eSeekGreaterOrEqual,
    eSeekGreater,
    eSeekLessThanOrEqual,
    eSeekLessThan
};

enum eStepOpType
{
    eStepFirst,
    eStepLast
};

enum eFindCurrntType
{
    ePosNeedNext = 1,
    ePosNeedNone = 0,
    ePosNeedPrev = -1
};

class connectParams
{
    _TCHAR m_buf[MAX_PATH];
    short m_type;
    char_td m_mode;

public:
    inline connectParams(const _TCHAR* protocol, const _TCHAR* hostOrIp,
                         const _TCHAR* dbname, const _TCHAR* schemaTable
                         ,const _TCHAR* userName=NULL, const _TCHAR* passwd=NULL)
        : m_type(TYPE_SCHEMA_BDF), m_mode(TD_OPEN_READONLY)
    {
        _TCHAR dbf[MAX_PATH]={0x00};
        if (schemaTable && schemaTable[0])
        {
            const _TCHAR* ext = _T(".bdf");
            if (_tcscmp(schemaTable, TRANSACTD_SCHEMANAME)==0)
                ext = _T("");
            _stprintf_s(dbf, MAX_PATH, _T("dbfile=%s%s"), schemaTable, ext);   
        }
        if (userName == NULL || (userName[0] == 0x00))
            _stprintf_s(m_buf, MAX_PATH, _T("%s://%s/%s?%s"), protocol,
                    hostOrIp, dbname, dbf);
        else
        {
            if (!passwd) passwd = _T("");
            _stprintf_s(m_buf, MAX_PATH, _T("%s://%s@%s/%s?%s&pwd=%s"),
                protocol, userName, hostOrIp, dbname, dbf, passwd);
        }
    }
    inline explicit connectParams(const _TCHAR* uri)
        : m_type(TYPE_SCHEMA_BDF), m_mode(TD_OPEN_READONLY)
    {
        _tcscpy_s(m_buf, MAX_PATH, uri);
        size_t len = _tcslen(uri);
        if (len > 5)
        {
            _TCHAR tmp[10];
            _tcscpy_s(tmp, 10, uri + len - 4);
#ifdef _tcsupr_s
            _tcsupr_s(tmp, 10);
#else
            _tcsupr(tmp);
#endif
            if (_tcscmp(tmp, _T(".DDF"))==0)
                m_type = TYPE_SCHEMA_DDF;
        }
    }
    inline void setMode(char_td v) { m_mode = v; }

    inline void setType(short v)
    {
        if (m_type != v)
        {
            if ((_tcslen(m_buf) > 3) && m_buf[_tcslen(m_buf) - 3] == _T('.'))
            {
                m_buf[_tcslen(m_buf) - 3] = 0x00;
                if (v == TYPE_SCHEMA_BDF)
                    _tcscat_s(m_buf, MAX_PATH, _T("bdf"));
                else
                    _tcscat_s(m_buf, MAX_PATH, _T("ddf"));
            }
        }
        m_type = v;
    }

    inline const _TCHAR* uri(bool noPasswd=false) const 
    { 
        return m_buf; 
    }

    inline char_td mode() const { return m_mode; };

    inline short type() const { return m_type; };
};

/* databaseManager interface
   If use some databases, implemnt a this interface and set the activeTable
   constructor
   Create table by name and option from suitable database.

*/
class idatabaseManager
{

public:
    virtual ~idatabaseManager(){};
    virtual void reset(int) = 0;
    virtual table_ptr table(const _TCHAR* name) = 0;
    virtual database* db() const = 0;
    virtual void use(const connectParams* param = NULL) = 0;
    virtual void unUse() = 0;
    virtual void setOption(__int64 v) = 0;
    virtual __int64 option() = 0;
    virtual void beginTrn(short bias) = 0;
    virtual void endTrn() = 0;
    virtual void abortTrn() = 0;
    virtual int enableTrn() = 0;
    virtual void beginSnapshot(short bias = CONSISTENT_READ) = 0;
    virtual void endSnapshot() = 0;
    virtual const _TCHAR* uri() const = 0;
    virtual char_td mode() const = 0;
    virtual bool isOpened() const = 0;
    virtual short_td stat() const = 0;
    virtual uchar_td* clientID() const = 0;
};

template <class T>
class tableIterator
    : public std::iterator<std::bidirectional_iterator_tag, fields, void>
{

    table& m_tb;
    fields m_fds;
    ushort_td m_lockBias;

public:
    inline tableIterator(table& tb, ushort_td lockBias = LOCK_BIAS_DEFAULT) : m_tb(tb), m_fds(tb),m_lockBias(lockBias)
    {
        readStatusCheck(tb, _T("tableIterator"));
    }

    table& tb() const { return m_tb; };

    void setLockBias(ushort_td v) { m_lockBias = v; }

    inline fields& operator*() { return m_fds; }

    inline fields* operator->() { return &m_fds; }

    inline tableIterator& operator++()
    {
        T::increment(m_tb, m_lockBias);
        return *this;
    }

    inline tableIterator& operator--()
    {
        T::decrement(m_tb, m_lockBias);
        return *this;
    }

    inline bool operator==(const tableIterator& v) { return m_tb.stat() != 0; }

    inline bool operator!=(const tableIterator& v) { return m_tb.stat() == 0; }

    inline bool isEnd() { return m_tb.stat() != 0; }
};

typedef tableIterator<indexNavi> indexIterator;
typedef tableIterator<indexFindNavi> findIterator;
typedef tableIterator<stepNavi> stepIterator;

typedef tableIterator<indexRvNavi> indexRvIterator;
typedef tableIterator<indexRvFindNavi> findRvIterator;
typedef tableIterator<stepRvNavi> stepRvIterator;

static const int filter_validate_value = 0;
static const int filter_validate_block = 1;
static const int filter_invalidate_value = 2;

typedef boost::function<int(const fields&)> validationFunc;

#define VALIDATION_FUNC(FUNC_PTR, PTR) boost::bind(FUNC_PTR, PTR, fields_type)

template <class T>
class filterdIterator
    : public std::iterator<std::input_iterator_tag, fields, void>
{
    T& m_it;
    validationFunc m_func;
    inline int callFunc()
    {
        int v = m_func(*m_it);
        if (v == filter_invalidate_value)
            m_it.tb().setStat(STATUS_EOF);
        return v;
    }

public:
    filterdIterator(T& it, validationFunc func) : m_it(it), m_func(func)
    {
        int v = callFunc();
        if (v == filter_validate_block)
            operator++();
    }

    inline fields operator*() { return m_it.operator*(); }

    inline fields* operator->() { return m_it.operator->(); }

    T& operator++()
    {
        int v;
        do
        {
            ++m_it;
            v = callFunc();
        } while (v == filter_validate_block);
        return m_it;
    }

    inline bool operator==(const filterdIterator& v)
    {
        return m_it.operator==(v.m_it);
    }

    inline bool operator!=(const filterdIterator& v)
    {
        return m_it.operator!=(v.m_it);
    }

    inline bool isEnd() { return m_it.isEnd(); }
};

typedef filterdIterator<indexIterator> filterdIndexIterator;
typedef filterdIterator<stepIterator> filterdStepIterator;
typedef filterdIterator<findIterator> filterdFindIterator;

typedef filterdIterator<indexRvIterator> filterdIndexRvIterator;
typedef filterdIterator<stepRvIterator> filterdStepRvIterator;
typedef filterdIterator<findRvIterator> filterdFindRvIterator;

inline indexIterator readIndex(table_ptr tb, eIndexOpType op, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{

    switch (op)
    {
    case eSeekEqual:
        tb->seek(lockBias);
        break;
    case eSeekFirst:
        tb->seekFirst(lockBias);
        break;
    case eSeekGreaterOrEqual:
        tb->seekGreater(true, lockBias);
        break;
    case eSeekGreater:
        tb->seekGreater(false, lockBias);
        break;
    default:
        assert(0);
        readStatusCheck(*tb, _T("readIndex"));
    }
    return indexIterator(*tb, lockBias);
}

inline indexRvIterator readIndexRv(table_ptr tb, eIndexOpType op, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{

    switch (op)
    {
    case eSeekEqual:
        tb->seek(lockBias);
        break;
    case eSeekLast:
        tb->seekLast(lockBias);
        break;
    case eSeekLessThanOrEqual:
        tb->seekLessThan(true, lockBias);
        break;
    case eSeekLessThan:
        tb->seekLessThan(false, lockBias);
        break;
    default:
        assert(0);
        readStatusCheck(*tb, _T("readIndexRv"));
    }
    return indexRvIterator(*tb, lockBias);
}

template <class T>
inline indexIterator readIndex(table_ptr tb, eIndexOpType op, char_td keynum,
                               T func, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{
    tb->setKeyNum(keynum);
    if (&func)
    {
        fields fds(*tb);
        func(fds);
    }
    return readIndex(tb, op, lockBias);
}

template <class T>
inline indexRvIterator readIndexRv(table_ptr tb, eIndexOpType op,
                                   char_td keynum, T func, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{
    tb->setKeyNum(keynum);
    if (&func)
    {
        fields fds(*tb);
        func(fds);
    }
    return readIndexRv(tb, op, lockBias);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
inline indexIterator
readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum, const T0 kv0,
            const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4,
            const T5 kv5, const T6 kv6, const T7 kv7)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
        tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    return readIndex(tb, op);
}

/** @cond INTERNAL */

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0,
                                 const T1 kv1, const T2 kv2, const T3 kv3,
                                 const T4 kv4, const T5 kv5, const T6 kv6)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2,
                                                    kv3, kv4, kv5, kv6);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0,
                                 const T1 kv1, const T2 kv2, const T3 kv3,
                                 const T4 kv4, const T5 kv5)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                                kv4, kv5);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4>
inline indexIterator
readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum, const T0 kv0,
            const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
{
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                            kv4);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0,
                                 const T1 kv1, const T2 kv2, const T3 kv3)
{
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0,
                                 const T1 kv1, const T2 kv2)
{
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    return readIndex(tb, op);
}

template <class T0, class T1>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0,
                                 const T1 kv1)
{
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    return readIndex(tb, op);
}

template <class T0>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op,
                                 const char_td keynum, const T0 kv0)
{
    keyValueSetter<T0>::set(tb, keynum, kv0);
    return readIndex(tb, op);
}
/** @endcond */

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
inline indexRvIterator
readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum, const T0 kv0,
              const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4,
              const T5 kv5, const T6 kv6, const T7 kv7)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
        tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    return readIndexRv(tb, op);
}

/** @cond INTERNAL */

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0,
                                     const T1 kv1, const T2 kv2, const T3 kv3,
                                     const T4 kv4, const T5 kv5, const T6 kv6)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2,
                                                    kv3, kv4, kv5, kv6);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0,
                                     const T1 kv1, const T2 kv2, const T3 kv3,
                                     const T4 kv4, const T5 kv5)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                                kv4, kv5);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4>
inline indexRvIterator
readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum, const T0 kv0,
              const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
{
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                            kv4);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0,
                                     const T1 kv1, const T2 kv2, const T3 kv3)
{
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0,
                                     const T1 kv1, const T2 kv2)
{
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    return readIndexRv(tb, op);
}

template <class T0, class T1>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0,
                                     const T1 kv1)
{
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    return readIndexRv(tb, op);
}

template <class T0>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op,
                                     const char_td keynum, const T0 kv0)
{
    keyValueSetter<T0>::set(tb, keynum, kv0);
    return readIndexRv(tb, op);
}
/** @endcond */

inline stepIterator readStep(table_ptr tb, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{
    tb->stepFirst(lockBias);
    return stepIterator(*tb, lockBias);
}

inline stepRvIterator readStepRv(table_ptr tb, ushort_td lockBias = LOCK_BIAS_DEFAULT)
{
    tb->stepLast(lockBias);
    return stepRvIterator(*tb, lockBias);
}


inline pq_handle setQuery(table_ptr& tb, const queryBase& q, 
                          bool serverPrepare = false) 
{ 
    pq_handle stmt =  tb->setQuery(&q, serverPrepare);
    readStatusCheck(*tb, _T("setQuery"));
    return stmt;
}

/** @cond INTERNAL */
inline pq_handle setQuery(table_ptr& tb, const pq_handle& q) 
{ 
    tb->setPrepare(q); 
    return q;
}
/** @endcond */

inline pq_handle prepare(table_ptr& tb, const queryBase& q, bool serverPrepare=false)
{
    pq_handle stmt = tb->prepare(&q, serverPrepare);
    readStatusCheck(*tb, _T("prepare"));
    return stmt;
}

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,
                         const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
        tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    tb->find(table::findForword);
    return findIterator(*tb);
}


/** @cond INTERNAL */

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,
                         const T4 kv4, const T5 kv5, const T6 kv6)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2,
                                                    kv3, kv4, kv5, kv6);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,
                         const T4 kv4, const T5 kv5)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                                kv4, kv5);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3, class T4>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,
                         const T4 kv4)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                            kv4);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0, class T1, class T2>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1, const T2 kv2)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0, class T1>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0, const T1 kv1)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    tb->find(table::findForword);
    return findIterator(*tb);
}

template <class Q, class T0>
inline findIterator find(table_ptr tb, const char_td keynum, const Q& q,
                         const T0 kv0)
{
    setQuery(tb, q); 
    keyValueSetter<T0>::set(tb, keynum, kv0);
    tb->find(table::findForword);
    return findIterator(*tb);
}
/** @endcond */

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1,
                             const T2 kv2, const T3 kv3, const T4 kv4,
                             const T5 kv5, const T6 kv6, const T7 kv7)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
        tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

/** @cond INTERNAL */

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1,
                             const T2 kv2, const T3 kv3, const T4 kv4,
                             const T5 kv5, const T6 kv6)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2,
                                                    kv3, kv4, kv5, kv6);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3, class T4, class T5>
inline findRvIterator
findRv(table_ptr tb, const char_td keynum, const Q& q, const T0 kv0,
       const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4, const T5 kv5)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                                kv4, kv5);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3, class T4>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1,
                             const T2 kv2, const T3 kv3, const T4 kv4)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                            kv4);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0, class T1, class T2, class T3>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1,
                             const T2 kv2, const T3 kv3)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0, class T1, class T2>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1,
                             const T2 kv2)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0, class T1>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0, const T1 kv1)
{
    setQuery(tb, q); 
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

template <class Q, class T0>
inline findRvIterator findRv(table_ptr tb, const char_td keynum,
                             const Q& q, const T0 kv0)
{
    setQuery(tb, q); 
    keyValueSetter<T0>::set(tb, keynum, kv0);
    tb->find(table::findBackForword);
    return findRvIterator(*tb);
}

/** @endcond */

inline findIterator getFindIterator(indexIterator it, const queryBase& q,
                                    bool isCurrentValid)

{
    if (!it.isEnd())
    {
        it.tb().setQuery(&q);
        if (!isCurrentValid)
            it.tb().findNext(false);
    }
    return findIterator(it.tb());
}

inline findRvIterator getFindIterator(indexRvIterator it, const queryBase& q,
                                      bool isCurrentValid)
{
    if (!it.isEnd())
        it.tb().setQuery(&q);
    if (!isCurrentValid)
        it.tb().findPrev(false);
    return findRvIterator(it.tb());
}

template <class Database_Ptr>
bool isSameUri(const connectParams* param, const Database_Ptr& db)
{
    return db && db->isOpened() && (_tcsicmp(param->uri(), db->uri()) == 0) &&
           (param->mode() == db->mode());
}

inline void releaseDatabase(database* db)
{
    database::destroy(db);
}

inline void releaseTable(table* p)
{
    if (p)
    {
        if (nsdatabase::testTablePtr(p))
            p->release();
    }
}

inline database_ptr createDatabaseObject()
{
    database_ptr p(database::create(), releaseDatabase);
    return p;
}

template <class Database_Ptr>
inline void disconnect(Database_Ptr db, const connectParams& connPrams)
{
    db->disconnect(connPrams.uri());
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Disconnect database ")) +
                             connPrams.uri()).c_str(),
                            db->stat());
}

template <class Database_Ptr> inline void disconnect(Database_Ptr db)
{
    db->disconnect();
    if (db->stat())
        nstable::throwError(_T("Disconnect database "), db->stat());
}

template <class Database_Ptr>
inline void throwDbError(Database_Ptr db,  const _TCHAR* caption, const _TCHAR* uri)
{
    TCHAR tmp[MAX_PATH];
    tdap::stripPasswd(uri, tmp, MAX_PATH);
    nstable::throwError((std::_tstring(caption) + tmp).c_str(), db->stat());
}

template <class Database_Ptr, class ConnectParam_type>
inline void connect(Database_Ptr db, const ConnectParam_type& connPrams,
                    bool newConnection)
{
    db->connect(connPrams.uri(), newConnection);
    if (db->stat())
        throwDbError(db, _T("Connect database : "), connPrams.uri());
}

template <class Database_Ptr>
inline void createDatabase(Database_Ptr db, const connectParams& connPrams)
{
    db->create(connPrams.uri());
    if (db->stat())
        throwDbError(db, _T("Create database : "), connPrams.uri());
}

template <class Database_Ptr>
inline void createDatabase(Database_Ptr db, const _TCHAR* uri)
{
    db->create(uri);
    if (db->stat())
        throwDbError(db, _T("Create database : "), uri);
}

template <class Database_Ptr, class ConnectParam_type>
inline void openDatabase(Database_Ptr db, const ConnectParam_type& connPrams)
{
    db->open(connPrams.uri(), connPrams.type(), connPrams.mode());
    if (db->stat())
        throwDbError(db, _T("Open database : "), connPrams.uri());
}

template <class Database_Ptr>
inline void openDatabase(Database_Ptr db, const _TCHAR* uri,
                         short schemaType = 0, short mode = -2,
                         const _TCHAR* dir = NULL,
                         const _TCHAR* ownername = NULL)
{
    db->open(uri, schemaType, mode, dir, ownername);
    if (db->stat())
        throwDbError(db, _T("Open database : "), uri);
}

template <class Database_Ptr>
inline void connectOpen(Database_Ptr db, const connectParams& connPrams,
                        bool newConnection)
{
    if (newConnection)
        connect(db, connPrams, newConnection);
    openDatabase(db, connPrams);
}

template <class Database_Ptr> inline void dropDatabase(Database_Ptr db)
{
    db->drop();
    if (db->stat())
        nstable::throwError(std::_tstring(_T("Drop database ")).c_str(),
                            db->stat());
}

template <class Database_Ptr>
inline table_ptr openTable(Database_Ptr db, const _TCHAR* name,
        short mode = TD_OPEN_NORMAL,
        bool autoCreate = true, const _TCHAR* ownerName = NULL,
                     const _TCHAR* uri = NULL)
{
    table_ptr p(db->openTable(name, mode, autoCreate, ownerName, uri),
                releaseTable);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Open table ")) + name).c_str(),
                            db->stat());
    return p;
}

template <class Database_Ptr>
inline table_ptr openTable(Database_Ptr db, short tableid,
        short mode = TD_OPEN_NORMAL,
        bool autoCreate = true, const _TCHAR* ownerName = NULL,
                     const _TCHAR* uri = NULL)
{
    table_ptr p(db->openTable(tableid, mode, autoCreate, ownerName, uri),
                releaseTable);

    if (db->stat())
    {
        _TCHAR buf[50];
        _stprintf_s(buf, 50, _T("Open table id = %d"), tableid);
        nstable::throwError(buf, db->stat());
    }
    return p;
}

template <class Database_Ptr>
inline void dropTable(Database_Ptr db, const _TCHAR* name)
{
    db->dropTable(name);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Drop table ")) + name).c_str(),
                            db->stat());
}

template <class Database_Ptr>
inline void convertTable(Database_Ptr db, short tableid, copyDataFn func = NULL)
{

    if (db->existsTableFile(tableid, NULL))
    {
        db->setOnCopyData(func);
        db->convertTable(tableid, false, NULL);
        db->setOnCopyData(NULL);
        if (db->stat())
        {
            assert(db->dbDef());
            db->dbDef()->popBackup(tableid);
            nstable::throwError(std::_tstring(_T("Convert table ")).c_str(),
                                db->stat());
        }
    }
}

template <class Database_Ptr>
inline void convertTable(Database_Ptr db, const _TCHAR* name,
                         copyDataFn func = NULL)
{
    assert(db->dbDef());
    short tablenum = db->dbDef()->tableNumByName(name);
    convertTable(db, tablenum, func);
}

inline void insertTable(dbdef* def, short id, const _TCHAR* name,
                        unsigned short charsetIndex)
{
    tabledef td;
    td.setTableName(name);
    td.setFileName(name);
    td.id = id;
    td.charsetIndex = (uchar_td)charsetIndex;
    def->insertTable(&td);
    if (def->stat() != 0)
        nstable::throwError(
            (std::_tstring(_T("Insert tabledef ")) + name).c_str(),
            def->stat());
}

inline void deleteTable(dbdef* def, short id)
{
    def->deleteTable(id);
    if (def->stat() != 0)
        nstable::throwError(_T("Delete tabledef "), def->stat());

}


inline void renumberTable(dbdef* def, short id, short newid)
{
    def->renumberTable(id, newid);
    if (def->stat() != 0)
        nstable::throwError(_T("Renumber table id "), def->stat());

}

inline fielddef* insertField(dbdef* def, short tableid, short fieldNum,
                             const _TCHAR* name, uchar_td type, ushort_td len)
{
    fielddef* fd = def->insertField(tableid, fieldNum);
    if (def->stat() != 0)
        nstable::throwError(
            (std::_tstring(_T("Insert fielddef ")) + name).c_str(),
            def->stat());

    fd->setName(name);
    fd->type = type;
    fd->len = len;
    return fd;
}

inline void deleteField(dbdef* def, short tableid, short fieldNum)
{
    def->deleteField(tableid, fieldNum);
    if (def->stat() != 0)
        nstable::throwError(_T("Delete fielddef "), def->stat());
}

inline keydef* insertKey(dbdef* def, short tableid, short insertIndex)
{
    keydef* kd = def->insertKey(tableid, insertIndex);
    if (def->stat() != 0)
        nstable::throwError(std::_tstring(_T("Insert keydef ")).c_str(),
                            def->stat());
    return kd;
}

inline void deleteKey(dbdef* def, short tableid, short keynum)
{
    def->deleteKey(tableid, keynum);
    if (def->stat() != 0)
        nstable::throwError(_T("Delete keydef "), def->stat());
}

inline void validateTableDef(dbdef* def, short tableid)
{
    def->validateTableDef(tableid);
    if (def->stat() != 0)
    {
        std::_tstring s;
        if (def->tableDefs(tableid))
            s = def->tableDefs(tableid)->tableName();
        nstable::throwError((std::_tstring(_T("Validate tabledef ")) + s).c_str(),
                            def->stat());
    }
}

inline void updateTableDef(dbdef* def, short tableid)
{
    def->updateTableDef(tableid);
    if (def->stat() != 0)
    {
        std::_tstring s;
        if (def->tableDefs(tableid))
            s = def->tableDefs(tableid)->tableName();
        nstable::throwError((std::_tstring(_T("Update tabledef ")) + s).c_str(),
                            def->stat());
    }
}

/** @cond INTERNAL */

template <class T> inline table* getTable(T& it)
{
    return &(it.tb());
}

template <> inline table* getTable(table_ptr& tb)
{
    return tb.get();
}

template <> inline table* getTable(table*& tb)
{
    return tb;
}

/** @endcond */

template <class T> inline void insertRecord(T& some, bool ncc = true)
{
    table* tb = getTable(some);
    tb->insert(ncc);
    if (tb->stat() != 0)
        nstable::throwError(std::_tstring(_T("Insert record")).c_str(), tb);
}

inline void updateRecord(fields& fd, const char_td keynum)
{
    fd.tb().setKeyNum(keynum);
    fd.tb().update(nstable::changeInKey);
    if (fd.tb().stat() != 0)
        nstable::throwError(std::_tstring(_T("Update record")).c_str(),
                            &(fd.tb()));
}

template <class T> inline void updateRecord(T& some, bool ncc = true)
{
    table* tb = getTable(some);
    tb->update((nstable::eUpdateType)ncc);
    if (tb->stat() != 0)
        nstable::throwError(std::_tstring(_T("Update record")).c_str(), tb);
}

template <class T> inline void deleteRecord(T& some)
{
    table* tb = getTable(some);
    tb->del(false /*inKey*/);
    if (tb->stat() != 0)
        nstable::throwError(std::_tstring(_T("Delete record")).c_str(), tb);
}

template <class T> inline void deleteRecord(T& some, const char_td keynum)
{
    table* tb = getTable(some);

    tb->setKeyNum(keynum);
    tb->del(true /*inKey*/);
    if (tb->stat() != 0)
        nstable::throwError(std::_tstring(_T("Delete record")).c_str(), tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4,
                         const T5 kv5, const T6 kv6, const T7 kv7)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
        tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    fields fd(tb);
    deleteRecord(fd, keynum);
}
/** @cond INTERNAL */

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4,
                         const T5 kv5, const T6 kv6)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2,
                                                    kv3, kv4, kv5, kv6);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4,
                         const T5 kv5)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                                kv4, kv5);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3, class T4>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
{
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3,
                                            kv4);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2, const T3 kv3)
{
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1, const T2 kv2)
{
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0, class T1>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0,
                         const T1 kv1)
{
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    fields fd(tb);
    deleteRecord(fd, keynum);
}

template <class T0>
inline void deleteRecord(table_ptr tb, const char_td keynum, const T0 kv0)
{
    keyValueSetter<T0>::set(tb, keynum, kv0);
    fields fd(tb);
    deleteRecord(fd, keynum);
}
/** @endcond */

template <class T, class F> void for_each(T iterator, F func)
{
    std::for_each(iterator, iterator, func);
}

/** Shared pointer of idatabaseManager.  */
typedef boost::shared_ptr<idatabaseManager> dbmanager_ptr;

template <class T> inline T createDatabaseForConnectionPool(T& p);

/** @cond INTERNAL */
template <> inline database_ptr createDatabaseForConnectionPool(database_ptr& p)
{
    return createDatabaseObject();
}
/** @endcond */

/* Exception safe trnasction
        It can use for database  and idatabaseManager.
*/
template <class DB> class transaction
{
    DB m_db;
    short m_bias;

public:
    inline transaction(DB db, short bias = SINGLELOCK_READ_COMMITED +
                                           NOWAIT_WRITE)
        : m_db(db), m_bias(bias){};
    inline ~transaction()
    {
        if (m_db->enableTrn())
            m_db->abortTrn();
    };
    inline void begin() { m_db->beginTrn(m_bias); }
    inline void end() { m_db->endTrn(); }
    inline void abort() { m_db->abortTrn(); }
};

/** transaction for database */
typedef transaction<database_ptr> dbTransaction;

/** transaction for idatabaseManager */
typedef transaction<dbmanager_ptr> dbmTransaction;

template <class DB> class snapshot
{
    DB m_db;

public:
    snapshot(DB db, short bias = CONSISTENT_READ) : m_db(db) { m_db->beginSnapshot(bias); }

    ~snapshot() { m_db->endSnapshot(); }
};

/** snapshot for database */
typedef snapshot<database_ptr> dbSnapshot;

/** snapshot for idatabaseManager */
typedef snapshot<idatabaseManager*> dbmSnapshot;

class autoBulkinsert
{
    table_ptr m_tb;

public:
    autoBulkinsert(table_ptr tb, int bufsize = BULKBUFSIZE) : m_tb(tb)
    {
        m_tb->beginBulkInsert(bufsize);
    }

    /* For activeObject */
    template <class T>
    autoBulkinsert(T& tba, int bufsize = BULKBUFSIZE)
        : m_tb(tba.table())
    {
        m_tb->beginBulkInsert(bufsize);
    }

    ~autoBulkinsert() { m_tb->commitBulkInsert(); }
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_TRDBOOSTAPI_H
