/*=================================================================
   Copyright (C) 2014,2015 BizStation Corp All rights reserved.

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
#include "filter.h"
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
struct valueItem
{
    std::_tstring value;
    bool null;
};

struct fieldNamesImple
{
    std::vector<valueItem> keyFields;
    fieldNamesImple() {}
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

fieldNames::fieldNames() : m_impl(new fieldNamesImple)
{
}

fieldNames::fieldNames(const fieldNames& r)
    : m_impl(new fieldNamesImple(*r.m_impl))
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

void fieldNames::doAddValue(const _TCHAR* v, bool isNull)
{
    valueItem itm;
    itm.value = v;
    itm.null = isNull;
    m_impl->keyFields.push_back(itm);
}

fieldNames& fieldNames::keyField(const _TCHAR* name, const _TCHAR* name1,
                                 const _TCHAR* name2, const _TCHAR* name3,
                                 const _TCHAR* name4, const _TCHAR* name5,
                                 const _TCHAR* name6, const _TCHAR* name7,
                                 const _TCHAR* name8, const _TCHAR* name9,
                                 const _TCHAR* name10)
{
    m_impl->keyFields.clear();
    if (name)
    {
        doAddValue(name, false);
        if (name1)
        {
            doAddValue(name1, false);
            if (name2)
            {
                doAddValue(name2, false);
                if (name3)
                {
                    doAddValue(name3, false);
                    if (name4)
                    {
                        doAddValue(name4, false);
                        {
                            if (name5)
                            {
                                doAddValue(name5, false);
                                if (name6)
                                {
                                    doAddValue(name6, false);
                                    if (name7)
                                    {
                                        doAddValue(name7, false);
                                        if (name8)
                                        {
                                           doAddValue(name8, false);
                                            if (name9)
                                            {
                                                doAddValue(name9, false);
                                                if (name10)
                                                    doAddValue(name10, false);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return *this;
}

int fieldNames::count() const
{
    return (int)m_impl->keyFields.size();
}

const _TCHAR* fieldNames::getValue(int index) const
{
    assert(index >= 0 && index < count());
    return m_impl->keyFields[index].value.c_str();
}

const _TCHAR* fieldNames::operator[](int index) const
{
    assert(index >= 0 && index < count());
    return m_impl->keyFields[index].value.c_str();
}

void fieldNames::addValue(const _TCHAR* v)
{
    doAddValue(v, false);
}

void fieldNames::addValues(const _TCHAR* values, const _TCHAR* delmi)
{
    std::vector<std::_tstring> tmp;
    boost::algorithm::split(tmp, values, boost::is_any_of(delmi));
    valueItem itm;
    itm.null = false;
    for (int i=0;i < (int)tmp.size(); ++i)
    {
        itm.value = tmp[i];
        m_impl->keyFields.push_back(itm);
    }

}

// ---------------------------------------------------------------------------
// class fieldValues
// ---------------------------------------------------------------------------
fieldValues::fieldValues() : fieldNames() {}

fieldValues::fieldValues(const fieldValues& r): fieldNames(r) {}

fieldValues& fieldValues::operator=(const fieldValues& r)
{
    if (this != &r)
    {
        fieldNames::operator=(r);
    }
    return *this;
}

void fieldValues::addValue(const _TCHAR* v, bool isNull)
{
    doAddValue(v, isNull);
}

bool fieldValues::isNull(int index) const
{
    assert(index >= 0 && index < count());
    return m_impl->keyFields[index].null;

}


// ---------------------------------------------------------------------------
// struct recordsetQueryImple
// ---------------------------------------------------------------------------
struct recordsetQueryImple
{
    row_ptr row;
    struct compItem
    {
        compFieldFunc compFunc;
        short index;
        unsigned char compType;
        char combine;
        struct
        {
            bool nullable : 1;
            bool nulllog  : 1;
        };
    };
    std::vector<compItem> compItems;
    fielddefs compFields;
    short endIndex;
    bool mysqlnull;

    recordsetQueryImple() : row(NULL),mysqlnull(false) {}
    recordsetQueryImple(const recordsetQueryImple& r)
        : row(r.row), compItems(r.compItems), compFields(r.compFields), mysqlnull(r.mysqlnull)
    {
        if (row)
            row->addref();
    }

    ~recordsetQueryImple()
    {
        if (row)
            row->release();
    }
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

recordsetQuery::recordsetQuery() : query(), m_imple(new recordsetQueryImple)
{
}

recordsetQuery::recordsetQuery(const recordsetQuery& r)
    : query(r), m_imple(new recordsetQueryImple(*r.m_imple))
{
}

recordsetQuery& recordsetQuery::operator=(const recordsetQuery& r)
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
    m_imple->mysqlnull = fdinfo->mysqlnullEnable();
    const std::vector<std::_tstring>& tokns = getWheres();
    m_imple->compFields.clear();
    m_imple->compItems.clear();
    for (int i = 0; i < (int)tokns.size(); i += 4)
    {
        recordsetQueryImple::compItem itm;
        itm.index = fdinfo->indexByName(tokns[i].c_str());
        if (itm.index >= 0)
            itm.nullable = (*fdinfo)[itm.index].isNullable();
        m_imple->compItems.push_back(itm);
        m_imple->compFields.push_back(&((*fdinfo)[itm.index]));
    }
    m_imple->compFields.calcFieldPos(0 /*startIndex*/, true);
    m_imple->row = memoryRecord::create(m_imple->compFields);
    m_imple->row->addref();
    m_imple->row->setRecordData(autoMemory::create(), 0, 0, &m_imple->endIndex, true);

    int index = 0;
    for (int i = 0; i < (int)tokns.size(); i += 4)
    {
        recordsetQueryImple::compItem& itm = m_imple->compItems[index];
        field fd = (*m_imple->row)[index];
        fd = tokns[i + 2].c_str();
        bool part = fd.isCompPartAndMakeValue();
        itm.compType = getFilterLogicTypeCode(tokns[i + 1].c_str());
        eCompType log = (eCompType)(itm.compType & 0xf);
        itm.nulllog = ((log == eIsNull) || (log == eIsNotNull));
        if (!part)
            itm.compType |= CMPLOGICAL_VAR_COMP_ALL;
        fielddef& fdd = const_cast<fielddef&>(m_imple->compFields[index]);
        fdd.len = m_imple->compFields[index].compDataLen((const uchar_td*)fd.ptr(), part);
        itm.compFunc = fd.getCompFunc(itm.compType);

        // When use wide string functions, len convert to wide char num. 
        if ((itm.compFunc == compiWString) || (itm.compFunc == compWString))
            fdd.len /= sizeof(char16_t);

        if (i + 3 < (int)tokns.size())
        {
            std::_tstring s = tokns[i + 3];
            if (s == _T("or"))
                itm.combine = eCor;
            else if (s == _T("and"))
                itm.combine = eCand;
        }
        else
            itm.combine = eCend;
        ++index;
    }
}

bool recordsetQuery::isMatch(int ret, unsigned char compType) const
{
    compType &= 0xf; // lower than 15
    switch ((eCompType)compType)
    {
    case eEqual:
    case eBitAnd:
        return (ret == 0);
    case eGreaterEq:
        return (ret >= 0);
    case eLessEq:
        return (ret <= 0);
    case eGreater:
        return (ret > 0);
    case eLess:
        return (ret < 0);
    case eNotEq:
    case eNotBitAnd:
        return (ret != 0);
    default:
        break;
    }
    return false;
}

int nullComp(const field& l, char log)
{
    bool rnull = (log == eIsNull) || (log == eIsNotNull);
    bool lnull = l.isNull();
            
    if (lnull || rnull)
    {
        if (lnull && (log == eIsNull))
            return 1;
        else if (lnull && (log == eIsNotNull))
            return -1;
        else if (log == eIsNull)
            return -1;
        return 1; //(log == (char)eIsNotNull)
    }
    return 0;
}

bool recordsetQuery::match(const row_ptr row) const
{
    for (int i = 0; i < (int)m_imple->compItems.size(); ++i)
    {
        recordsetQueryImple::compItem& itm = m_imple->compItems[i];
        bool ret;
        int nullJudge = 2;
        const field& f = (*row)[itm.index];
        if (m_imple->mysqlnull && (itm.nullable || itm.nulllog))
            nullJudge = f.nullComp((eCompType)(itm.compType & 0xf));
        if (nullJudge < 2)
            ret = (nullJudge == 0) ? true : false;
        else
            ret = isMatch(itm.compFunc(f, (*m_imple->row)[i], itm.compType), itm.compType);

        if (itm.combine == eCend)
            return ret;
        if (ret && itm.combine == eCor)
            return true;
        if (!ret && itm.combine == eCand)
            return false;
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
//#define USE_CLONE_FUNCTION
class groupQueryImple : public fieldNames
{
    std::vector<groupFuncBase*> m_funcs;

    void removeFields(recordsetImple& mdls)
    {
        const fielddefs& fds = *mdls.fieldDefs();
        for (int i = (int)fds.size() - 1; i >= 0; --i)
        {
            bool enabled = false;
            for (int j = 0; j < (int)m_impl->keyFields.size(); ++j)
            {
                if (m_impl->keyFields[j].value == fds[i].name())
                {
                    enabled = true;
                    break;
                }
            }
            if (!enabled)
            {
                for (int j = 0; j < (int)m_funcs.size(); ++j)
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

    void cleanup()
    {
#ifdef USE_CLONE_FUNCTION
        for (int i=0; i< (int)m_funcs.size() ; ++i)
            delete m_funcs[i];
#endif
        m_funcs.clear();
    }

public:
    groupQueryImple() : fieldNames() {}

    ~groupQueryImple() { cleanup(); }

    fieldNames& reset()
    {
        cleanup();
        return fieldNames::reset();
    }

    void addFunction(groupFuncBase* func) 
    { 
#ifdef USE_CLONE_FUNCTION
        m_funcs.push_back(func->clone());
#else
        m_funcs.push_back(func);
#endif
    }

    void grouping(recordsetImple& mdls)
    {
        std::vector<recordsetImple::key_type> keyFields;

        for (int i = 0; i < (int)m_impl->keyFields.size(); ++i)
            keyFields.push_back(resolvKeyValue(mdls, m_impl->keyFields[i].value));

        for (int i = 0; i < (int)m_funcs.size(); ++i)
        {
            groupFuncBase* f = m_funcs[i];
            f->init(mdls.fieldDefs());

            if (f->resultKey() == (int)mdls.fieldDefs()->size())
                mdls.appendField(f->resultName(), f->resultType(),
                                     f->resultLen(), f->decimals());
        }

        grouping_comp<recordsetImple> groupingComp(mdls, keyFields);
        std::vector<int> index;
        recordsetImple::iterator it = begin(mdls), ite = end(mdls);

        int i, n = 0;
        while (it != ite)
        {
            bool found = false;
            i = binary_search(n, index, 0, (int)index.size(), groupingComp, found);
            if (!found)
                index.insert(index.begin() + i, n);
            for (int j = 0; j < (int)m_funcs.size(); ++j)
                (*m_funcs[j])(*it, i, !found);
            ++n;
            ++it;
        }

        // real sort by index
        recordsetImple c(mdls);

        clear(mdls);

        for (int i = 0; i < (int)index.size(); ++i)
        {
            recordsetImple::row_type cur = c.getRow(index[i]);

            for (int j = 0; j < (int)m_funcs.size(); ++j)
            {
                if (m_funcs[j]->isNull(i))
                    (*cur).setInvalidMemblock(m_funcs[j]->resultKey());
                else if (m_funcs[j]->resultType() == ft_float)
                    setValue(cur, m_funcs[j]->resultKey(), m_funcs[j]->numericResult(i));
                else
                    memcpy((*cur)[m_funcs[j]->resultKey()].ptr() ,
                            m_funcs[j]->stringResult(i), m_funcs[j]->resultLen());
            }
            mdls.push_back(cur);
        }
        removeFields(mdls);
    }

    const std::vector<groupFuncBase*>& getFunctions() const { return m_funcs; };
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

groupQuery::groupQuery() : m_imple(new groupQueryImple)
{
}

groupQuery::groupQuery(const groupQuery& r)
    : m_imple(new groupQueryImple(*r.m_imple))
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

groupQuery& groupQuery::keyField(const _TCHAR* name, const _TCHAR* name1,
                                 const _TCHAR* name2, const _TCHAR* name3,
                                 const _TCHAR* name4, const _TCHAR* name5,
                                 const _TCHAR* name6, const _TCHAR* name7,
                                 const _TCHAR* name8, const _TCHAR* name9,
                                 const _TCHAR* name10)
{
    m_imple->keyField(name, name1, name2, name3, name4, name5, name6, name7,
                      name8, name9, name10);
    return *this;
}

const fieldNames& groupQuery::getKeyFields() const
{
    return *m_imple;
}

const groupFuncBase* groupQuery::getFunction(int index) const
{
    assert(index >= 0 && index < functionCount());
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
private:
    friend class groupQueryImple;
    fieldNames m_targetNames;
    std::_tstring m_resultName;
    std::vector<int> m_targetKeys;
    int m_resultKey;
    ushort_td m_resultLen;
    uchar_td m_resultType;
    uchar_td m_decimals;
    bool m_insertFlag;
    void clearStrings()
    {
        std::vector<unsigned char*>::iterator it = m_strings.begin();
        while (it != m_strings.end())
            delete [] *(it++);
        m_strings.clear();
    }

    void copyStrings(const std::vector<unsigned char*>& r)
    {
        std::vector<unsigned char*>::const_iterator it = r.begin();
        while (it != r.end())
        {
            unsigned char* p = new unsigned char[m_resultLen];
            memcpy(p, *(it++), m_resultLen);
            m_strings.push_back(p);
        }
    }

public:
    std::vector<groupFuncBase::numeric_type> m_values;
    std::vector<__int64> m_counts;
    std::vector<unsigned char*> m_strings;
    std::vector<bool> m_nulls;

    inline groupFuncBaseImple() : m_resultKey(-1),
                               m_resultLen(sizeof(double)),  m_resultType(ft_float),
                               m_decimals(0)

    {
    }

    inline groupFuncBaseImple(const fieldNames& targetNames, const _TCHAR* resultName = NULL)
        : m_resultKey(-1), m_resultLen(sizeof(double)), m_resultType(ft_float), m_decimals(0)
                               
    {
        m_targetNames = targetNames;
        m_resultName = (m_targetNames.count() &&
                        ((resultName == NULL) || resultName[0] == 0x00))
                           ? targetNames[0] : resultName;
    }

    inline groupFuncBaseImple(const groupFuncBaseImple& r)
        : m_targetNames(r.m_targetNames), m_resultName(r.m_resultName),
          m_resultKey(r.m_resultKey), 
          m_resultLen(r.m_resultLen), m_resultType(r.m_resultType), m_decimals(r.m_decimals),
          m_values(r.m_values), m_counts(r.m_counts),m_nulls(r.m_nulls)
    {
        copyStrings(r.m_strings);
    }

    groupFuncBaseImple& operator=(const groupFuncBaseImple& r)
    {
        if (this != &r)
        {
            m_targetNames = r.m_targetNames;
            m_resultName = r.m_resultName;
            m_resultKey = r.m_resultKey;
            m_resultType = r.m_resultType; 
            m_resultLen = r.m_resultLen;
            m_decimals = r.m_decimals;
            m_values = r.m_values;
            m_counts = r.m_counts;
            m_nulls = r.m_nulls;
            m_strings.clear();
            copyStrings(r.m_strings); 
        }
        return *this;
    }

    ~groupFuncBaseImple()
    {
        clearStrings();
    }

    inline void setStringResultType(ushort_td len, uchar_td type)
    {
        m_resultType = type;
        m_resultLen = len;
    }

    inline uchar_td resultType() const { return m_resultType; }

    inline ushort_td resultLen() const { return m_resultLen; }

    inline void appendStringBuffer(int index)
    {
        unsigned char* p = new unsigned char[m_resultLen];
        memset(p, 0, m_resultLen);
        std::vector<unsigned char*>::iterator it = m_strings.begin();
        if (index)
            it += index;
        m_strings.insert(it, p);
    }

    inline void initResultVariable(int index)
    {
        if (m_resultType == ft_float)
        {
            std::vector<groupFuncBase::numeric_type>::iterator it = m_values.begin();
            if (index)
                it += index;
            m_values.insert(it, 0.0f);
        }
        else
            appendStringBuffer(index);
        std::vector<bool>::iterator it = m_nulls.begin();
        if (index)
            it += index;
        m_nulls.insert(it, true);
        m_insertFlag = true;
    }

    inline void init(const fielddefs* fdinfo)
    {
        m_targetKeys.clear();
        for (int i = 0; i < m_targetNames.count(); ++i)
        {
            int index = (m_targetNames[i][0] != 0x00)
                                       ? fdinfo->indexByName(m_targetNames[i]) : -1;
            m_targetKeys.push_back(index);

            //Auto set decimals
            if (index != -1)
                m_decimals = std::max<uchar_td>(m_decimals, (*fdinfo)[index].decimals);
        }
        m_resultKey = fdinfo->indexByName(m_resultName);
        if (m_resultKey == -1)
            m_resultKey = (int)fdinfo->size();
    }

    inline fieldNames& targetNames() const
    {
        return (fieldNames&)m_targetNames;
    }

    inline const _TCHAR* resultName() const { return m_resultName.c_str(); }

    inline void setResultName(const _TCHAR* v)
    {
        m_resultName = _T("");
        if (v && v[0])
            m_resultName = v;
    }

    inline int resultKey() const { return m_resultKey; }

    inline void reset()
    {
        m_values.clear();
        m_counts.clear();
        clearStrings();
    }

    inline int targetKey(size_t index) const
    {
        assert(index < m_targetKeys.size());
        return m_targetKeys[index];
    }

    inline int targetKeys() const { return (int)m_targetKeys.size(); }

    inline bool insertFlag() const { return m_insertFlag; }

    inline void clearFlag() { m_insertFlag = false;}

    inline void setDecimals(uchar_td v) { m_decimals = v; }

    inline uchar_td decimals() const { return m_decimals; }

};

// ---------------------------------------------------------------------------
// class groupFuncBase
// ---------------------------------------------------------------------------
groupFuncBase::groupFuncBase() : recordsetQuery(), m_imple(new groupFuncBaseImple())
{
}

groupFuncBase::groupFuncBase(const fieldNames& targetNames,
                             const _TCHAR* resultName)
    : recordsetQuery(), m_imple(new groupFuncBaseImple(targetNames, resultName))
{
}

groupFuncBase::groupFuncBase(const groupFuncBase& r)
    : recordsetQuery(r), m_imple(new groupFuncBaseImple(*r.m_imple))
{
}

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

void groupFuncBase::doInit(const fielddefs* fdinfo)
{
    if (whereTokens() != 0)
        recordsetQuery::init(fdinfo);

    m_imple->init(fdinfo);
}

void groupFuncBase::init(const fielddefs* fdinfo)
{
    doInit(fdinfo);
}

bool groupFuncBase::insertFlag() const
{
    return m_imple->insertFlag();
}

void groupFuncBase::clearInsertFlag()
{
    m_imple->clearFlag();
}

groupFuncBase& groupFuncBase::operator=(const recordsetQuery& v)
{
    recordsetQuery::operator=(v);
    return *this;
}

fieldNames& groupFuncBase::targetNames() const
{
    return m_imple->targetNames();
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

bool groupFuncBase::isNull(int groupIndex) const
{
    return m_imple->m_nulls[groupIndex];
}

void groupFuncBase::reset()
{
    recordsetQuery::reset();
    doReset();
}

void groupFuncBase::doReset()
{
    m_imple->reset();
}

void groupFuncBase::doCalcEachkey(const field& fd, int index) {}

void groupFuncBase::doCalc(const row_ptr& row, int index)
{
    for (int i = 0; i < m_imple->targetKeys(); ++i)
    {
        const field& fd = (*row)[m_imple->targetKey(i)];
        if (!fd.isNull())
        {
            doCalcEachkey(fd, index);
            m_imple->m_nulls[index] = false;
        }
    }
}

void groupFuncBase::operator()(const row_ptr& row, int index, bool insert)
{
    if (insert)
        initResultVariable(index);
    bool flag = (whereTokens() == 0);

    if (!flag)
        flag = match(row);
    if (flag)
        doCalc(row, index);
}

groupFuncBase::numeric_type groupFuncBase::numericResult(int groupIndex) const
{
    return m_imple->m_values[groupIndex];
}

unsigned char* groupFuncBase::stringResult(int groupIndex) const
{
    return m_imple->m_strings[groupIndex];
}

ushort_td groupFuncBase::resultLen() const
{
    return m_imple->resultLen();
}

uchar_td groupFuncBase::resultType() const
{
    return m_imple->resultType();
}

uchar_td groupFuncBase::decimals() const { return m_imple->decimals();}

// ---------------------------------------------------------------------------
// class sum
// ---------------------------------------------------------------------------
sum* sum::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new sum(targetNames, resultName);
}

sum::sum(const fieldNames& targetNames, const _TCHAR* resultName)
    : groupFuncBase(targetNames, resultName)
{
}

void sum::doCalcEachkey(const field& fd, int index)
{
    numeric_type tmp = 0;
    m_imple->m_values[index] += fieldValue(fd, tmp);
}

groupFuncBase* sum::clone()
{
    sum* p = new sum();
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

count* count::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new count(targetNames, resultName);
}

count::count(const _TCHAR* resultName) : groupFuncBase()
{
    setResultName(resultName);
}

count::count(const fieldNames& targetNames, const _TCHAR* resultName)
     : groupFuncBase(targetNames, resultName) {}

void count::initResultVariable(int index)
{
    groupFuncBase::initResultVariable(index);
    m_imple->m_nulls[index] = false;
}

void count::doCalcEachkey(const field& fd, int index)
{
    m_imple->m_values[index] = m_imple->m_values[index] + 1;
}

void count::doCalc(const row_ptr& row, int index)
{
    if (m_imple->targetKeys())
        groupFuncBase::doCalc(row, index);
    else
        m_imple->m_values[index] = m_imple->m_values[index] + 1;
}

groupFuncBase* count::clone()
{
    count* p = new count();
    *p = *this;
    return p;
}

// ---------------------------------------------------------------------------
// class avg
// ---------------------------------------------------------------------------
avg* avg::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new avg(targetNames, resultName);
}

avg::avg() : groupFuncBase()
{
     m_imple->setDecimals(4);
}

avg::avg(const fieldNames& targetNames, const _TCHAR* resultName)
    : groupFuncBase(targetNames, resultName)
{
    m_imple->setDecimals(4);
}

void avg::initResultVariable(int index)
{
    groupFuncBase::initResultVariable(index);
    m_imple->m_counts.insert(m_imple->m_counts.begin() + index, 0);
}

void avg::doCalcEachkey(const field& fd, int index)
{
    numeric_type tmp = 0;
    m_imple->m_values[index] += fieldValue(fd, tmp);
    m_imple->m_counts[index] = m_imple->m_counts[index] + 1;
}

avg::numeric_type avg::numericResult(int index) const
{
    if (m_imple->m_counts[index])
        return m_imple->m_values[index] / m_imple->m_counts[index];
    return 0;
}

groupFuncBase* avg::clone()
{
    avg* p = new avg();
    *p = *this;
    return p;
}

// ---------------------------------------------------------------------------
// class min
// ---------------------------------------------------------------------------
min* min::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new min(targetNames, resultName);
}

min::min(const fieldNames& targetNames, const _TCHAR* resultName)
    : groupFuncBase(targetNames, resultName)
{
}

void min::doCalcEachkey(const field& fd, int index)
{
    numeric_type tmp = 0;
    tmp = fieldValue(fd, tmp);
    if (insertFlag() || (tmp < m_imple->m_values[index]))
        m_imple->m_values[index] = tmp;
    clearInsertFlag();
}

groupFuncBase* min::clone()
{
    min* p = new min();
    *p = *this;
    return p;
}

min& min::operator=(const min& r)
{
    if (this != &r)
        groupFuncBase::operator=(r);
    return *this;

}

// ---------------------------------------------------------------------------
// class max
// ---------------------------------------------------------------------------
max* max::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new max(targetNames, resultName);
}

max::max(const fieldNames& targetNames, const _TCHAR* resultName)
    : groupFuncBase(targetNames, resultName)
{
}

void max::doCalcEachkey(const field& fd, int index)
{
    numeric_type tmp = 0;
    tmp = fieldValue(fd, tmp);
    if (insertFlag() || (tmp > m_imple->m_values[index]))
        m_imple->m_values[index] = tmp;
    clearInsertFlag();
}

groupFuncBase* max::clone()
{
    max* p = new max();
    *p = *this;
    return p;
}

max& max::operator=(const max& r)
{
    if (this != &r)
        groupFuncBase::operator=(r);
    return *this;
}

// ---------------------------------------------------------------------------
// class last
// ---------------------------------------------------------------------------
last* last::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new last(targetNames, resultName);
}

last::last(const fieldNames& targetNames, const _TCHAR* resultName)
    : groupFuncBase(targetNames, resultName)
{

}

void last::doInit(const fielddefs* fdinfo)
{
    groupFuncBase::doInit(fdinfo);
    int size = m_imple->targetKeys();
    if (size)
    {
        const fielddef& fd = (*fdinfo)[m_imple->targetKey(0)];
        if (fd.isStringType() && !fd.isBlob())
            m_imple->setStringResultType(fd.len, fd.type);
    }
}

void last::storeValue(const row_ptr& row, int index)
{
    const field& fd = (*row)[m_imple->targetKey(0)];
    if (!fd.isNull())
    {
        numeric_type tmp = 0;
        if (m_imple->resultType() == ft_float)
            m_imple->m_values[index] = fieldValue(fd, tmp);
        else
        {
            ushort_td len = m_imple->resultLen();
            unsigned char* p = m_imple->m_strings[index];
            memcpy(p, fd.ptr(), len);
        }
        m_imple->m_nulls[index] = false;
        clearInsertFlag();
    }
}

void last::doCalc(const row_ptr& row, int index)
{
    if (m_imple->targetKeys())
        storeValue(row, index);
}

groupFuncBase* last::clone()
{
    last* p = new last();
    *p = *this;
    return p;
}

// ---------------------------------------------------------------------------
// class first
// ---------------------------------------------------------------------------
first* first::create(const fieldNames& targetNames, const _TCHAR* resultName)
{
    return new first(targetNames, resultName);
}

first::first(const fieldNames& targetNames, const _TCHAR* resultName)
    : last(targetNames, resultName)
{

}

void first::doCalc(const row_ptr& row, int index)
{
    if (insertFlag() && m_imple->targetKeys())
        storeValue(row, index);
}

groupFuncBase* first::clone()
{
    first* p = new first();
    *p = *this;
    return p;
}

void  first::doReset()
{
    groupFuncBase::reset();
}

first& first::operator=(const first& r)
{
    if (this != &r)
        last::operator=(r);
    return *this;
}


} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
