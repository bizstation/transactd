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

#define JOIN_NO_MORERECORD 1

class DLLLIB fieldNames
{

protected:
    /** @cond INTERNAL */
    struct fieldNamesImple* m_impl;
    /** @endcond */
    void doAddValue(const _TCHAR* v, bool isNull);
public:
    fieldNames();
    fieldNames(const fieldNames& r);
    fieldNames& operator=(const fieldNames& r);

    virtual ~fieldNames();
    virtual fieldNames& reset();
    fieldNames& keyField(const _TCHAR* name, const _TCHAR* name1 = NULL,
                         const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                         const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                         const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                         const _TCHAR* name8 = NULL, const _TCHAR* name9 = NULL,
                         const _TCHAR* name10 = NULL);

    int count() const;
    const _TCHAR* operator[](int index) const;
    const _TCHAR* getValue(int index) const;
    void addValue(const _TCHAR* v);
    void addValues(const _TCHAR* values,
                   const _TCHAR* delmi); // delmi = boost::is_any_of
    static fieldNames* create();
    void release();
};

class DLLLIB fieldValues : public fieldNames
{
public:
    fieldValues();
    fieldValues(const fieldValues& r);
    fieldValues& operator=(const fieldValues& r);
    void addValue(const _TCHAR* v, bool isNull);
    bool isNull(int index) const;
};

struct sortField
{
    std::_tstring name;
    bool asc;
};

class sortFields
{
    std::vector<sortField> m_params;
    /** @cond INTERNAL */
    template <class Archive>
    friend void serialize(Archive& ar, sortFields& q, const unsigned int);
    /** @endcond */

public:
    inline sortFields& add(const _TCHAR* name, bool asc)
    {
        sortField op = { name, asc };
        m_params.push_back(op);
        return *this;
    }
    inline size_t size() const { return m_params.size(); }
    inline const sortField& operator[](int index) const
    {
        return m_params[index];
    }
    inline void clear() { m_params.clear(); }
};

class DLLLIB recordsetQuery : protected query
{
    friend class groupFuncBase;
    friend class recordsetImple;

    struct recordsetQueryImple* m_imple;

    void createTempRecord();
    void init(const fielddefs* fdinfo);
    void init(const fielddefs* fdinfo, const fielddefs* rfdinfo);
    bool match(const row_ptr row) const;
    bool matchJoin(const row_ptr rrow) const;
    void setJoinRow(const row_ptr row);
    int matchStatus() const;

public:
    recordsetQuery();
    recordsetQuery(const recordsetQuery& r);
    ~recordsetQuery();

    recordsetQuery& operator=(const recordsetQuery& r);

    template <class T>
    inline recordsetQuery& when(const _TCHAR* name, const _TCHAR* qlogic,
                                T value)
    {
        query::where(name, qlogic, value);
        return *this;
    }

    template <class T>
    inline recordsetQuery& and_(const _TCHAR* name, const _TCHAR* qlogic,
                                T value)
    {
        query::and_(name, qlogic, value);
        return *this;
    }

    template <class T>
    inline recordsetQuery& or_(const _TCHAR* name, const _TCHAR* qlogic,
                               T value)
    {
        query::or_(name, qlogic, value);
        return *this;
    }

    inline recordsetQuery& reset()
    {
        query::reset();
        return *this;
    }

    inline recordsetQuery& whenIsNull(const _TCHAR* name)
    {
        query::whereIsNull(name);
        return *this;
    }

    inline recordsetQuery& whenIsNotNull(const _TCHAR* name)
    {
        query::whereIsNotNull(name);
        return *this;
    }

    inline recordsetQuery& andIsNull(const _TCHAR* name)
    {
        query::andIsNull(name);
        return *this;
    }

    inline recordsetQuery& andIsNotNull(const _TCHAR* name)
    {
        query::andIsNotNull(name);
        return *this;
    }

    inline recordsetQuery& orIsNull(const _TCHAR* name)
    {
        query::orIsNull(name);
        return *this;
    }

    inline recordsetQuery& orIsNotNull(const _TCHAR* name)
    {
        query::orIsNotNull(name);
        return *this;
    }

    inline const _TCHAR* toString() const { return queryBase::toString(); }

    inline query* internalQuery() { return this; }
    static recordsetQuery* create();
    void release();
};

class DLLLIB groupFuncBase : public recordsetQuery
{
protected:
    friend class groupQueryImple;
    friend class groupFuncBaseImple;

    typedef double numeric_type;

    class groupFuncBaseImple* m_imple;

    void init(const fielddefs* fdinfo);
    unsigned char* stringResult(int index) const;
    uchar_td resultType() const;
    ushort_td resultLen() const;
    void operator()(const row_ptr& row, int index, bool insert);

    virtual void initResultVariable(int index);
    virtual void doCalcEachkey(const field& fd, int index);
    virtual void doCalc(const row_ptr& row, int index);
    virtual void doReset();
    virtual void doInit(const fielddefs* fdinfo);
    virtual numeric_type numericResult(int index) const;
    bool insertFlag() const ;
    void clearInsertFlag();

public:
    groupFuncBase();
    groupFuncBase(const groupFuncBase& v);
    groupFuncBase(const fieldNames& targetNames,
                  const _TCHAR* resultName = NULL);
    virtual ~groupFuncBase();
    groupFuncBase& operator=(const groupFuncBase& v);
    groupFuncBase& operator=(const recordsetQuery& v);
    fieldNames& targetNames() const;
    const _TCHAR* resultName() const;
    void setResultName(const _TCHAR* v);
    int resultKey() const;
    void reset();
    bool isNull(int index) const;
    uchar_td decimals() const;
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
    groupQuery& reset();
    groupQuery& addFunction(groupFuncBase* func);
    groupQuery& keyField(const _TCHAR* name, const _TCHAR* name1 = NULL,
                         const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                         const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                         const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                         const _TCHAR* name8 = NULL, const _TCHAR* name9 = NULL,
                         const _TCHAR* name10 = NULL);
    const fieldNames& getKeyFields() const;
    const groupFuncBase* getFunction(int index) const;
    int functionCount() const;
    static groupQuery* create();
    void release();
};

class DLLLIB sum : public groupFuncBase
{
protected:
    virtual void doCalcEachkey(const field& fd, int index);
    groupFuncBase* clone();
public:
    sum() : groupFuncBase() {}
    sum(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static sum* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};

class DLLLIB count : public groupFuncBase
{
protected:
    groupFuncBase* clone();
    void doCalcEachkey(const field& fd, int index);
    void doCalc(const row_ptr& row, int index);
    void initResultVariable(int index);
public:
    count(): groupFuncBase(){}
    count(const _TCHAR* resultName);
    count(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static count* create(const _TCHAR* resultName);
    static count* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};

class DLLLIB avg : public groupFuncBase
{
    void initResultVariable(int index);
    void doCalcEachkey(const field& fd, int index);
    numeric_type numericResult(int index) const;
    groupFuncBase* clone();
public:
    avg();
    avg(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static avg* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);

};

#undef min
class DLLLIB min : public groupFuncBase
{
protected:
    void doCalcEachkey(const field& fd, int index);
    groupFuncBase* clone();
    min& operator=(const min& r);

public:
    min() : groupFuncBase() {}
    min(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static min* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};

#undef max
class DLLLIB max : public groupFuncBase
{
    void doCalcEachkey(const field& fd, int index);
    groupFuncBase* clone();
    max& operator=(const max& r);

public:
    max() : groupFuncBase(){}
    max(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static max* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};


class DLLLIB last : public groupFuncBase
{
protected:
    void doCalc(const row_ptr& row, int index);
    void doInit(const fielddefs* fdinfo);
    groupFuncBase* clone();
    void storeValue(const row_ptr& row, int index);
public:
    last() : groupFuncBase(){}
    last(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static last* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};


class DLLLIB first : public last
{
protected:
    void doCalc(const row_ptr& row, int index);
    void doReset();
    groupFuncBase* clone();
    first& operator=(const first& r);
public:
    first() : last() {}
    first(const fieldNames& targetNames, const _TCHAR* resultName = NULL);
    static first* create(const fieldNames& targetNames,
                       const _TCHAR* resultName = NULL);
};



} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H
