#ifndef trdboostapiH
#define trdboostapiH
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
#include <bzs/db/protocol/tdap/client/trdboostapiInternal.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <stdio.h>

#if defined(__GNUC__) 
	#if !__GNUC_PREREQ(4, 3)
		#error "GCC Version is too small. 4.3 or above GCC versions are required."
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

enum eIndexOpType {
    eSeekEqual, eSeekFirst, eSeekLast, eSeekGreaterOrEqual, eSeekGreater, eSeekLessThanOrEqual,
    eSeekLessThan};

enum eStepOpType {
    eStepFirst, eStepLast};

enum eFindCurrntType{
    ePosNeedNext = 1, ePosNeedNone = 0, ePosNeedPrev = -1};

typedef boost::shared_ptr<database>database_ptr;
typedef boost::shared_ptr<table>table_ptr;




class field
{
    short m_index;
    table& m_tb;

public:
    inline field(table& tb, short index) : m_tb(tb), m_index(index) {};

    inline const _TCHAR* c_str() const {return m_tb.getFVstr(m_index);}

    inline const char* a_str() const {return m_tb.getFVAstr(m_index);}

    inline int i() const {return m_tb.getFVint(m_index);}

    inline short i16() const {return m_tb.getFVsht(m_index);}

    inline __int64 i64() const {return m_tb.getFV64(m_index);}

    inline float f() const {return m_tb.getFVflt(m_index);}

    inline double d() const {return m_tb.getFVdbl(m_index);}

    inline field& operator = (const _TCHAR* p)
    {
        m_tb.setFV(m_index, p);
        return *this;
    }

    inline field& operator = (const std::_tstring& p)
    {
        m_tb.setFV(m_index, p.c_str());
        return *this;
    }

#ifdef _UNICODE
    inline field& operator = (const char* p)
    {
        m_tb.setFVA(m_index, p);
        return *this;
    }

    inline field& operator = (const std::string& p)
    {
        m_tb.setFVA(m_index, p.c_str());
        return *this;
    }

#endif

    inline field& operator = (int v)
    {
        m_tb.setFV(m_index, v);
        return *this;
    }

    inline field& operator = (__int64 v)
    {
        m_tb.setFV(m_index, v);
        return *this;
    }

    inline field& operator = (float v)
    {
        m_tb.setFV(m_index, v);
        return *this;
    }

    inline field& operator = (double v)
    {
        m_tb.setFV(m_index, v);
        return *this;
    }

    inline bool operator != (const _TCHAR* p) {return (_tcscmp(p, c_str()) != 0);};
    inline bool operator == (const _TCHAR* p) {return (_tcscmp(p, c_str())==0);};

    inline bool operator != (int v) {return (v != i());};
    inline bool operator == (int v) {return (v == i());};

    inline bool operator != (short v) {return (v != i16());};
    inline bool operator == (short v) {return (v == i16());};

    inline bool operator != (__int64 v) {return (v != i64());};
    inline bool operator == (__int64 v) {return (v == i64());};

    inline bool operator != (float v) {return (v != f());};
    inline bool operator == (float v) {return (v == f());};

    inline bool operator != (double v) {return (v != d());};
    inline bool operator == (double v) {return (v == d());};

    inline void setBin(const void* data, uint_td size){ m_tb.setFV(m_index, data, size);}
    inline void* getBin(uint_td& size){return m_tb.getFVbin(m_index, size);};
};

class fields
{
    table& m_tb;

public:
    inline explicit fields(table& tb) : m_tb(tb) {};
    inline explicit fields(table_ptr tb) : m_tb(*tb) {};
    inline void clearValues(){m_tb.clearBuffer();};
    inline field operator[](size_t index) const {return field(m_tb, (short)index);}

    inline field operator()(const _TCHAR* name) const {
        return field(m_tb, m_tb.fieldNumByName(name));}
    inline table& tb() const {return m_tb;}

    inline size_t size() {return m_tb.tableDef()->fieldCount;}

    inline field fd(size_t index) const {return field(m_tb, (short)index);}

    inline field fd(const _TCHAR* name) const {return field(m_tb, m_tb.fieldNumByName(name));}
};

class filterParams
{
	std::_tstring m_str;
	int m_rejectCount;
	int m_maxRecords; 
public:
	inline filterParams(const _TCHAR* filter, int rejectCount, int maxRecords)
		:m_str(filter),m_rejectCount(rejectCount),m_maxRecords(maxRecords)
	{

	}
	inline const _TCHAR* filter() const {return m_str.c_str();}
	inline int rejectCount() const {return m_rejectCount;}
	inline int maxRecords() const {return m_maxRecords;}


};

template <class T>
class tableIterator
{

    table& m_tb;
    fields m_fds;

    inline tableIterator() : m_tb(*((table*)0)), m_fds(*((table*)0)) {};

public:
    static const tableIterator eos;

    inline tableIterator(table& tb) : m_tb(tb), m_fds(tb)
    {
        if ((tb.stat() != 0) && (tb.stat() != STATUS_EOF) && (tb.stat() != STATUS_NOT_FOUND_TI))
            nstable::throwError(std::_tstring(_T("tableIterator")).c_str(), &tb);

    }

    table& tb() const {return m_tb;};

    inline fields& operator*() {return m_fds;}

    inline fields* operator->() {return &m_fds;}

    inline tableIterator& operator++() {
        T::increment(m_tb);
        return *this;
    }

    inline tableIterator& operator--() {
        T::decrement(m_tb);
        return *this;
    }

    inline bool operator == (const tableIterator& v) {return m_tb.stat() != 0;}

    inline bool operator != (const tableIterator& v) {return m_tb.stat() == 0;}

};
typedef tableIterator<indexNavi> indexIterator;
typedef tableIterator<indexFindNavi> findIterator;
typedef tableIterator<stepNavi> stepIterator;


template <> const indexIterator indexIterator::eos;
template <> const findIterator findIterator::eos;
template <> const stepIterator stepIterator::eos;



typedef tableIterator<indexRvNavi> indexRvIterator;
typedef tableIterator<indexRvFindNavi> findRvIterator;
typedef tableIterator<stepRvNavi> stepRvIterator;


template <> const indexRvIterator indexRvIterator::eos;
template <> const findRvIterator findRvIterator::eos;
template <> const stepRvIterator stepRvIterator::eos;



static const int filter_validate_value = 0;
static const int filter_validate_block = 1;
static const int filter_invalidate_value = 2;

typedef boost::function < int(const fields&) > validationFunc;

#define VALIDATION_FUNC(FUNC_PTR, PTR)  boost::bind(FUNC_PTR, PTR, fields_type)

template<class T>
class filterdIterator
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
    static const filterdIterator<T> eos;

    filterdIterator() : m_it(*((T*)0)), m_func(NULL) {}

    filterdIterator(T& it, validationFunc func) : m_it(it), m_func(func)
    {
        int v = callFunc();
        if (v == filter_validate_block)
            operator++();
    }


    inline fields operator*() {return m_it.operator*();}

    inline fields* operator->() {return m_it.operator->();}

    T& operator++()
    {
        int v;
        do {
            ++m_it;
            v = callFunc();
        }
        while (v == filter_validate_block);
        return m_it;
    }


    inline bool operator == (const filterdIterator& v) {return m_it.operator == (v.m_it);}

    inline bool operator != (const filterdIterator& v) {return m_it.operator != (v.m_it);}

};

typedef filterdIterator<indexIterator> filterdIndexIterator;
typedef filterdIterator<stepIterator> filterdStepIterator;
typedef filterdIterator<findIterator> filterdFindIterator;

typedef filterdIterator<indexRvIterator> filterdIndexRvIterator;
typedef filterdIterator<stepRvIterator> filterdStepRvIterator;
typedef filterdIterator<findRvIterator> filterdFindRvIterator;



template<>
const filterdIndexIterator filterdIndexIterator::eos;

template<>
const filterdStepIterator filterdStepIterator::eos;

template<>
const filterdFindIterator filterdFindIterator::eos;

template<>
const filterdIndexRvIterator filterdIndexRvIterator::eos;

template<>
const filterdStepRvIterator filterdStepRvIterator::eos;

template<>
const filterdFindRvIterator filterdFindRvIterator::eos;


inline indexIterator readIndex(table_ptr tb, eIndexOpType op)
{

    switch (op)
    {
    case eSeekEqual: tb->seek();
        break;
    case eSeekFirst: tb->seekFirst();
        break;
    case eSeekGreaterOrEqual: tb->seekGreater(true);
        break;
    case eSeekGreater: tb->seekGreater(false);
        break;
    default:
        assert(0);
    readStatusCheck(*tb, _T("readIndex"));
    }
    return indexIterator(*tb);
}

inline indexRvIterator readIndexRv(table_ptr tb, eIndexOpType op)
{

    switch (op)
    {
    case eSeekEqual: tb->seek();
        break;
    case eSeekLast: tb->seekLast();
        break;
    case eSeekLessThanOrEqual: tb->seekLessThan(true);
        break;
    case eSeekLessThan: tb->seekLessThan(false);
        break;
    default:
        assert(0);
    readStatusCheck(*tb, _T("readIndexRv"));
    }
    return indexRvIterator(*tb);
}

template<class T>
inline indexIterator readIndex(table_ptr tb, eIndexOpType op, char_td keynum, T func)
{
    tb->setKeyNum(keynum);
    if (&func)
    {
        fields fds(*tb);
        func(fds);
    }
    return readIndex(tb, op);

}

template<class T>
inline indexRvIterator readIndexRv(table_ptr tb, eIndexOpType op, char_td keynum, T func)
{
    tb->setKeyNum(keynum);
    if (&func)
    {
        fields fds(*tb);
        func(fds);
    }
    return readIndexRv(tb, op);

}

template <class T0, class T1, class T2, class T3
        , class T4, class T5, class T6, class T7>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3
        , class T4, class T5, class T6>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5, const T6 kv6)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3
        , class T4, class T5>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
{
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2, class T3>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1 ,const T2 kv2, const T3 kv3)
{
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    return readIndex(tb, op);
}

template <class T0, class T1, class T2>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1 ,const T2 kv2)
{
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    return readIndex(tb, op);
}

template <class T0, class T1>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1)
{
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    return readIndex(tb, op);
}

template <class T0>
inline indexIterator readIndex_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0)
{
    keyValueSetter<T0>::set(tb, keynum, kv0);
    return readIndex(tb, op);
}


template <class T0, class T1, class T2, class T3
        , class T4, class T5, class T6, class T7>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3
        , class T4, class T5, class T6>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5, const T6 kv6)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3
        , class T4, class T5>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
    ,const T4 kv4, const T5 kv5)
{
    keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3, class T4>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3, const T4 kv4)
{
    keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2, class T3>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1 ,const T2 kv2, const T3 kv3)
{
    keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    return readIndexRv(tb, op);
}

template <class T0, class T1, class T2>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1 ,const T2 kv2)
{
    keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    return readIndexRv(tb, op);
}

template <class T0, class T1>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0, const T1 kv1)
{
    keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    return readIndexRv(tb, op);
}

template <class T0>
inline indexRvIterator readIndexRv_v(table_ptr tb, eIndexOpType op, const char_td keynum
    ,const T0 kv0)
{
    keyValueSetter<T0>::set(tb, keynum, kv0);
    return readIndexRv(tb, op);
}


inline stepIterator readStep(table_ptr tb)
{
    tb->stepFirst();
    return stepIterator(*tb);
}

inline stepRvIterator readStepRv(table_ptr tb)
{
    tb->stepLast();
    return stepRvIterator(*tb);
}


template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2, class T3>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0>
inline findIterator find(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0>::set(tb, keynum, kv0);
    tb->find(table::findForword);
	return findIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::
            set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1, class T2, class T3, class T4>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1, class T2, class T3>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1, class T2>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1, const T2 kv2)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0, class T1>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0, const T1 kv1)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

template <class T0>
inline findRvIterator findRv(table_ptr tb, const char_td keynum, const filterParams& fp
    ,const T0 kv0)
{
    tb->setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
	keyValueSetter<T0>::set(tb, keynum, kv0);
    tb->find(table::findBackForword);
	return findRvIterator(*tb);
}

inline findIterator getFindIterator(indexIterator it, const filterParams& fp
                    ,bool isCurrentValid)

{
    if (it != indexIterator::eos)
	{
        it.tb().setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
		if (!isCurrentValid)
			it.tb().findNext(false);
	}
    return findIterator(it.tb());
}

inline findRvIterator getFindIterator(indexRvIterator it, const filterParams& fp
                    ,bool isCurrentValid)
{
    if (it != indexRvIterator::eos)
        it.tb().setFilter(fp.filter(), fp.rejectCount(), fp.maxRecords());
    if (!isCurrentValid)
        it.tb().findPrev(false);
    return findRvIterator(it.tb());
}


class connectParams
{
    _TCHAR m_buf[MAX_PATH];
    short m_mode;
    short m_type;

public:
    inline connectParams(const _TCHAR* protocol, const _TCHAR* hostOrIp, const _TCHAR* dbname,
        const _TCHAR* schemaTable) : m_mode(TD_OPEN_READONLY), m_type(TYPE_SCHEMA_BDF)
    {
        _stprintf_s(m_buf, MAX_PATH, _T("%s://%s/%s?dbfile=%s.bdf")
                    , protocol, hostOrIp, dbname, schemaTable);

    }
    inline explicit connectParams(const _TCHAR* uri) : m_mode(TD_OPEN_READONLY), m_type(TYPE_SCHEMA_BDF)
    {
        _tcscpy_s(m_buf, MAX_PATH, uri);

    }
    inline void setMode(short v){m_mode = v;}

    inline void setType(short v){m_type = v;}

    inline const _TCHAR* uri() const {return m_buf;}

    inline short mode() const {return m_mode;};

    inline short type() const {return m_type;};

};

inline void releaseDatabase(database* db) {database::destroy(db);}

inline void releaseTable(table* p) {if (p) p->release();}

inline database_ptr createDatadaseObject()
{
    database_ptr p(database::create(), releaseDatabase);
    return p;
}

template <class Database_Ptr, class ConnectParam_type>
inline void connect(database_ptr db, const ConnectParam_type& connPrams, bool newConnection)
{
    db->connect(connPrams.uri(), newConnection);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Connect database ")) + connPrams.uri()).c_str(), db->stat());

}

template <class Database_Ptr>
inline void disconnect(database_ptr db, const connectParams& connPrams)
{
    db->disconnect(connPrams.uri());
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Disconnect database ")) + connPrams.uri()).c_str(), db->stat());

}

template <class Database_Ptr>
inline void disconnect(database_ptr db)
{
    db->disconnect();
    if (db->stat())
        nstable::throwError(_T("Disconnect database "), db->stat());

}

template <class Database_Ptr>
inline void createDatabase(Database_Ptr db, const connectParams& connPrams)
{
    db->create(connPrams.uri());
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Create database ")) + connPrams.uri()).c_str(), db->stat());
}

template <class Database_Ptr>
inline void createDatabase(Database_Ptr db, const _TCHAR* uri)
{
    db->create(uri);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Create database "))+ uri).c_str() , db->stat());
}


template <class Database_Ptr, class ConnectParam_type>
inline void openDatabase(Database_Ptr db, const ConnectParam_type& connPrams)
{
    db->open(connPrams.uri(), connPrams.type(), connPrams.mode());
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Open database ")) + connPrams.uri()).c_str(), db->stat());

}

template <class Database_Ptr>
inline void openDatabase(Database_Ptr db, const _TCHAR* uri, short schemaType = 0, short mode = -2, const _TCHAR* dir = NULL,
        const _TCHAR* ownername = NULL)
{
    db->open(uri, schemaType, mode, dir, ownername);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Open database ")) + std::_tstring(uri)).c_str(), db->stat());

}

template <class Database_Ptr>
inline void dropDatabase(Database_Ptr db)
{
    db->drop();
    if (db->stat())
        nstable::throwError(std::_tstring(_T("Drop database ")).c_str(), db->stat());

}

template <class Database_Ptr>
inline table_ptr openTable(Database_Ptr db, const _TCHAR* name)
{
    table_ptr p(db->openTable(name), releaseTable);
    if (db->stat())
        nstable::throwError((std::_tstring(_T("Open table ")) + name).c_str(), db->stat());
    return p;

}

template <class Database_Ptr>
inline void convertTable(Database_Ptr db, short tableid, copyDataFn func=NULL)
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
            nstable::throwError(std::_tstring(_T("Convert table ")).c_str(), db->stat());
        }
    }
}

template <class Database_Ptr>
inline void convertTable(Database_Ptr db, const _TCHAR* name, copyDataFn func=NULL)
{
    assert(db->dbDef());
    short tablenum = db->dbDef()->tableNumByName(name);
    convertTable(db, tablenum, func);
}

inline void insertTable(dbdef* def, short id, const _TCHAR* name, unsigned short charsetIndex)
{
    tabledef td;
    td.setTableName(name);
    td.setFileName(name);
    td.id =id;
    td.charsetIndex = (uchar_td)charsetIndex;
    def->insertTable(&td);
    if (def->stat() != 0)
        nstable::throwError((std::_tstring(_T("Insert tabledef ")) + name).c_str(), def->stat());
}

inline fielddef* insertField(dbdef* def, short tableid, short fieldNum
    , const _TCHAR* name, uchar_td type, ushort_td len )
{

    fielddef* fd =  def->insertField(tableid, fieldNum);
    if (def->stat() != 0)
        nstable::throwError((std::_tstring(_T("Insert fielddef ")) + name).c_str(), def->stat());

    fd->setName(name);
    fd->type = type;
    fd->len = len;
    return fd;
}

inline keydef* insertKey(dbdef* def, short tableid, short insertIndex)
{
    keydef* kd =  def->insertKey(tableid, insertIndex);
    if (def->stat() != 0)
        nstable::throwError(std::_tstring(_T("Insert keydef ")).c_str(), def->stat());
    return kd;
}

inline void updateTableDef(dbdef* def, short tableid)
{
    def->updateTableDef(tableid);
    if (def->stat() != 0)
    {
        std::_tstring s;
        if (def->tableDefs(tableid))
            s = def->tableDefs(tableid)->tableName();
        nstable::throwError((std::_tstring(_T("Update tabledef ")) + s).c_str(), def->stat());
    }
}

template <class T>
inline void insertRecord(const T& it, bool ncc = true)
{
    it.tb().insert(ncc);
    if (it.tb().stat() != 0)
        nstable::throwError(std::_tstring(_T("Insert record")).c_str(), &(it.tb()));

}

template <class T>
inline void updateRecord(const T& it, bool ncc = true)
{
    it.tb().update((nstable::eUpdateType)ncc);
    if (it.tb().stat() != 0)
        nstable::throwError(std::_tstring(_T("Update record")).c_str(), &(it.tb()));

}

inline void updateRecord(fields& fd, char_td keynum)
{
    fd.tb().setKeyNum(keynum);
	fd.tb().update(nstable::changeInKey);
    if (fd.tb().stat() != 0)
        nstable::throwError(std::_tstring(_T("Update record")).c_str(), &(fd.tb()));
}

inline void deleteRecord(fields& fd, const char_td keynum)
{
    fd.tb().setKeyNum(keynum);
	fd.tb().del(true/*inKey*/);
    if (fd.tb().stat() != 0)
		nstable::throwError(std::_tstring(_T("Update record")).c_str(), &(fd.tb()));
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void deleteRecord(table_ptr tb, const char_td keynum
    ,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
{
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5, const T6 kv6)
{
	keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5, kv6);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4, const T5 kv5)
{
	keyValueSetter<T0, T1, T2, T3, T4, T5>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4, kv5);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3, class T4>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3,const T4 kv4)
{
	keyValueSetter<T0, T1, T2, T3, T4>::set(tb, keynum, kv0, kv1, kv2, kv3, kv4);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2, class T3>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
{
	keyValueSetter<T0, T1, T2, T3>::set(tb, keynum, kv0, kv1, kv2, kv3);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1, class T2>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1, const T2 kv2)
{
	keyValueSetter<T0, T1, T2>::set(tb, keynum, kv0, kv1, kv2);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0, class T1>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0, const T1 kv1)
{
	keyValueSetter<T0, T1>::set(tb, keynum, kv0, kv1);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T0>
void deleteRecord(table_ptr tb, const char_td keynum
	,const T0 kv0)
{
	keyValueSetter<T0>::set(tb, keynum, kv0);
	fields fd(tb);
	deleteRecord(fd, keynum);
}

template <class T>
inline void deleteRecord(const T& it)
{
	it.tb().del(false/*inKey*/);
	if (it.tb().stat() != 0)
		nstable::throwError(std::_tstring(_T("Delete record")).c_str(), &(it.tb()));
}


template<class T, class F>
void for_each(T iterator, F func) {std::for_each(iterator, T::eos, func);}

class transaction
{
    database_ptr m_db;
    short m_bias;
public:
    transaction(database_ptr db, short bias=LOCK_SINGLE_NOWAIT + PARALLEL_TRN + NOWAIT_WRITE)
        :m_db(db),m_bias(bias){};
    ~transaction(){if (m_db->enableTrn()) m_db->abortTrn();};
    void begin(){m_db->beginTrn(m_bias);}
    void end(){m_db->endTrn();}
    void abort(){m_db->abortTrn();}

};

class autoSnapshot
{
    database_ptr m_db;
public:
    autoSnapshot(database_ptr db):m_db(db)
    {
        m_db->beginSnapshot();
    }

    ~autoSnapshot()
    {
        m_db->endSnapshot();
    }
};


class autoBulkinsert
{
    table_ptr m_tb;
public:
    autoBulkinsert(table_ptr tb, int bufsize = BULKBUFSIZE):m_tb(tb)
    {
        m_tb->beginBulkInsert(bufsize);
    }
    ~autoBulkinsert(){m_tb->commitBulkInsert();}

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs



#endif
