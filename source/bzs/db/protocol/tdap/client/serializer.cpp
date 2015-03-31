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
#pragma hdrstop
#include "serializer.h"

#pragma package(smart_init)
#ifdef BCB_32
#pragma option push
#pragma option -Vbr-
#pragma option -vi-
#endif

#include <bzs/db/protocol/tdap/client/groupQuery.h>
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <fstream>

#ifdef __BCPLUSPLUS__
#define BZS_LINK_BOOST_SERIALIZATION
#include <bzs/env/boost_bcb_link.h>
#endif

using namespace boost::archive;
using namespace boost::serialization;

BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::sum, "sum");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::count, "count");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::avg, "avg");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::min, "min");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::max, "max");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::first, "first");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::last, "last");

BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::readStatement,
                        "readStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::readHasMany,
                        "readHasMany");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::orderByStatement,
                        "orderByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::matchByStatement,
                        "matchByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::groupByStatement,
                        "groupByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::reverseOrderStatement,
                        "reverseOrderStatement");

BOOST_CLASS_VERSION(bzs::db::protocol::tdap::client::groupFuncBase, 1)
BOOST_CLASS_VERSION(bzs::db::protocol::tdap::client::queryBase, 1)

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

client::query* replaceQueryParams(client::query* tq, client::query& tmpq,
                                  struct queryStatementsImple* parent);

void toU8(std::_tstring& src, std::string& dst)
{
#ifdef _UNICODE
    char buf[2048];
    WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, buf, 2048, NULL, NULL);
    dst = buf;

#else
    dst = src;
#endif
}

void fromU8(std::string& src, std::_tstring& dst)
{
#ifdef _UNICODE
    wchar_t buf[2048];
    MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buf, 2048);
    dst = buf;
#else
    dst = src;
#endif
}

template <class Archive>
void serialize_string(Archive& ar, const char* name, std::_tstring& v)
{
    std::string s;
    if (!Archive::is_loading::value)
        toU8(v, s);
    ar& boost::serialization::make_nvp(name, s);
    if (Archive::is_loading::value)
        fromU8(s, v);
}

template <class Archive>
void serialize(Archive&, executable&, const unsigned int)
{
}

template <class Archive>
void serialize(Archive& ar, sortField& q, const unsigned int)
{

    serialize_string(ar, "name", q.name);
    ar& boost::serialization::make_nvp("asc", q.asc);
}

template <class Archive>
void serialize(Archive& ar, sortFields& q, const unsigned int)
{
    int count = (int)q.m_params.size();
    ar& boost::serialization::make_nvp("count", count);
    for (int i = 0; i < count; i++)
    {
        if (Archive::is_loading::value)
        {
            sortField f;
            ar& boost::serialization::make_nvp("field", f);
            q.m_params.push_back(f);
        }
        else
            ar& boost::serialization::make_nvp("field", q.m_params[i]);
    }
}

template <class Archive>
void serialize(Archive& ar, groupByStatement& q, const unsigned int /*version*/)
{
    boost::serialization::base_object<executable>(q);
    ar& boost::serialization::make_nvp(
        "keyFields", boost::serialization::base_object<fieldNames>(q));
    ar& boost::serialization::make_nvp("functions", *q.m_statements);
}

template <class Archive>
void serialize(Archive& ar, matchByStatement& q, const unsigned int /*version*/)
{
    boost::serialization::base_object<executable>(q);
    ar& make_nvp("matchByStatement",
                 boost::serialization::base_object<recordsetQuery>(q));
}

template <class Archive>
void serialize(Archive& ar, orderByStatement& q, const unsigned int /*version*/)
{
    boost::serialization::base_object<executable>(q);
    ar& boost::serialization::make_nvp("sortFields", *q.m_sortFields);
}

template <class Archive>
void serialize(Archive& /*ar*/, reverseOrderStatement& q,
               const unsigned int /*version*/)
{
    boost::serialization::base_object<executable>(q);
}

template <class Archive>
void serialize(Archive& ar, readStatement& q, const unsigned int /*version*/)
{
    boost::serialization::base_object<executable>(q);
    ar& boost::serialization::make_nvp(
        "keyFields", boost::serialization::base_object<fieldNames>(q));
    ar& boost::serialization::make_nvp(
        "query", boost::serialization::base_object<query>(q));
    ar& boost::serialization::make_nvp("params", *q.internalPtr());
}

template <class Archive>
void serialize(Archive& ar, readHasMany& q, const unsigned int /*version*/)
{

    ar& boost::serialization::make_nvp(
        "readStatement", boost::serialization::base_object<readStatement>(q));
    ar& boost::serialization::make_nvp("params", *q.internalPtr());
}

template <class Archive>
void serialize(Archive& ar, queryBase& q, const unsigned int version)
{
    split_free(ar, q, version);
}

template <class Archive>
void save(Archive& ar, const queryBase& q, const unsigned int version)
{
    std::_tstring s = q.toString();

    serialize_string(ar, "queryString", s);
    int v = q.getReject();
    ar& make_nvp("reject", v);
    v = q.getLimit();
    ar& make_nvp("limit", v);
    v = q.getOptimize();
    ar& make_nvp("optimize", v);
    v = q.isBookmarkAlso();
    ar& make_nvp("boolmarkAlso", v);

    if (version >= 1)
    {
        v = q.getDirection();
        ar& make_nvp("direction", v);

        v = q.isStopAtLimit();
        ar& make_nvp("stopAtLimit", v);

    }
    v = q.isAll();
    ar& make_nvp("isAll", v);
}

template <class Archive>
void load(Archive& ar, queryBase& q, const unsigned int version)
{
    std::_tstring s;
    int v;

    q.reset();
    serialize_string(ar, "queryString", s);
    q.queryString(s.c_str());

    ar& make_nvp("reject", v);
    q.reject(v);

    ar& make_nvp("limit", v);
    q.limit(v);

    ar& make_nvp("optimize", v);
    q.optimize((queryBase::eOptimize)v);

    ar& make_nvp("boolmarkAlso", v);
    q.bookmarkAlso(v != 0);

    if (version >= 1)
    {
        ar& make_nvp("direction", v);
        q.direction((table::eFindType)v);
        ar& make_nvp("stopAtLimit", v);
        q.stopAtLimit(v == 1);
    }
    ar& make_nvp("isAll", v);
    if (v)
        q.all();
}

template <class Archive>
void serialize(Archive& ar, fieldNames& q, const unsigned int /*version*/)
{
    int count = q.count();
    ar& boost::serialization::make_nvp("count", count);
    std::_tstring s;
    for (int i = 0; i < count; i++)
    {
        if (Archive::is_loading::value)
        {
            serialize_string(ar, "value", s);
            q.addValue(s.c_str());
        }
        else
        {
            s = q.getValue(i);
            serialize_string(ar, "value", s);
        }
    }
}

template <class Archive>
void serialize(Archive& ar, query& q, const unsigned int /*version*/)
{
    ar& make_nvp("readQuery", boost::serialization::base_object<queryBase>(q));
}

template <class Archive>
void serialize(Archive& ar, recordsetQuery& q, const unsigned int /*version*/)
{
    queryBase* qq = q.internalQuery();
    ar& make_nvp("recordsetQuery", *qq);
}

template <class Archive>
void serialize(Archive& ar, groupFuncBase& q, const unsigned int ver)
{
    ar& boost::serialization::make_nvp(
        "query", boost::serialization::base_object<recordsetQuery>(q));

    fieldNames& fns = q.targetNames();
    if (ver >= 1)
        ar& make_nvp("targetNames", fns);

    std::_tstring s;

    if (Archive::is_loading::value)
    {
        // For compatibility
        if (ver < 1)
        {
            serialize_string(ar, "targetName", s);
            q.targetNames().addValue(s.c_str());
        }
        serialize_string(ar, "resultName", s);
        q.setResultName(s.c_str());
    }
    else
    {
        if (q.resultName())
            s = q.resultName();
        else
            s = _T("");
        serialize_string(ar, "resultName", s);
    }
}

template <class Archive>
void serialize(Archive& ar, sum& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, count& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, avg& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, min& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, max& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, first& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, last& q, const unsigned int /*version*/)
{
    ar& make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, groupQuery& q, const unsigned int /*version*/)
{
    fieldNames& v = const_cast<fieldNames&>(q.getKeyFields());
    ar& make_nvp("keyFields", v);
}

struct aliasPair
{
    std::_tstring first;
    std::_tstring second;
    aliasPair() {}
    aliasPair(const _TCHAR* f, const _TCHAR* s) : first(f), second(s) {}
};

typedef aliasPair alias_type;

template <class Archive>
void serialize(Archive& ar, alias_type& q, const unsigned int /*version*/)
{
    std::_tstring s = q.first;
    std::_tstring s2 = q.second;
    serialize_string(ar, "first", s);
    serialize_string(ar, "second", s2);
    if (Archive::is_loading::value)
    {
        q.first = s;
        q.second = s2;
    }
}

//---------------------------------------------------------------------------
//   class executable
//---------------------------------------------------------------------------
void executable::release()
{
    delete this;
};

//---------------------------------------------------------------------------
//   class groupByStatement
//---------------------------------------------------------------------------
class prepairedValues
{
    const std::vector<std::_tstring>* m_values;
    mutable int m_index;

public:
    inline void setValues(const std::vector<std::_tstring>* values)
    {
        m_values = values;
        m_index = 0;
    }

    const _TCHAR* replace(const _TCHAR* v) const
    {
        if (_tcscmp(v, _T("?")) == 0)
        {
            if (m_index >= (int)m_values->size())
                THROW_BZS_ERROR_WITH_MSG(_T("Too few values for prepair."));
            return (*m_values)[m_index++].c_str();
        }
        return v;
    }
};

//---------------------------------------------------------------------------
//   class groupByStatement
//---------------------------------------------------------------------------
groupByStatement* groupByStatement::create()
{
    return new groupByStatement();
}

groupByStatement::groupByStatement()
    : fieldNames(), m_statements(new std::vector<groupFuncBase*>())
{
}

groupByStatement::~groupByStatement()
{
    reset();
    delete m_statements;
}

groupFuncBase& groupByStatement::addFunction(eFunc v,
                                             const fieldNames& targetNames,
                                             const _TCHAR* resultName)
{
    groupFuncBase* func;
    switch (v)
    {
    case fsum:
        func = new client::sum(targetNames, resultName);
        break;
    case fcount:
        func = new client::count(resultName);
        break;
    case favg:
        func = new client::avg(targetNames, resultName);
        break;
    case fmin:
        func = new client::min(targetNames, resultName);
        break;
    case fmax:
        func = new client::max(targetNames, resultName);
        break;
    case ffirst:
        func = new client::first(targetNames, resultName);
        break;
    case flast:
        func = new client::last(targetNames, resultName);
        break;
    };
    m_statements->push_back(func);
    return *func;
}

groupFuncBase& groupByStatement::function(int index)
{
    assert(index >= 0 && index < (int)m_statements->size());
    return *((*m_statements)[index]);
}

groupByStatement& groupByStatement::reset()
{
    for (int i = 0; i < (int)m_statements->size(); ++i)
        delete ((*m_statements)[i]);
    m_statements->clear();
    fieldNames::reset();
    return *this;
}

int groupByStatement::size() const
{
    return (int)m_statements->size();
}

void groupByStatement::execute(recordset& rs)
{
    const _TCHAR* keys[8] = { NULL };
    for (int i = 0; i < count(); ++i)
        keys[i] = getValue(i);
    groupQuery q;
    q.keyField(keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6],
               keys[7]);
    std::vector<boost::shared_ptr<groupFuncBase> > statements;
    for (int i = 0; i < (int)m_statements->size(); ++i)
    {
        boost::shared_ptr<groupFuncBase> p((*m_statements)[i]->clone());
        statements.push_back(p);
        query* pq = p->internalQuery();
        replaceQueryParams(NULL, *pq, m_parent);
        q.addFunction(p.get());
    }
    rs.groupBy(q);
}

//---------------------------------------------------------------------------
//   class matchByStatement
//---------------------------------------------------------------------------
matchByStatement* matchByStatement::create()
{
    return new matchByStatement();
}

void matchByStatement::execute(recordset& rs)
{
    client::query q;
    client::query* ret = replaceQueryParams(internalQuery(), q, m_parent);
    if (ret != internalQuery())
    {
        recordsetQuery rq;
        *rq.internalQuery() = *ret;
        rs.matchBy(rq);
    }
    else
        rs.matchBy(*this);
}

//---------------------------------------------------------------------------
//   class orderByStatement
//---------------------------------------------------------------------------
orderByStatement* orderByStatement::create()
{
    return new orderByStatement();
}

orderByStatement::orderByStatement() : m_sortFields(new sortFields())
{
}

orderByStatement::~orderByStatement()
{
    delete m_sortFields;
}

void orderByStatement::execute(recordset& rs)
{
    rs.orderBy(*m_sortFields);
}

void orderByStatement::add(const _TCHAR* name, bool asc)
{
    m_sortFields->add(name, asc);
}

int orderByStatement::size() const
{
    return (int)m_sortFields->size();
}

const sortField& orderByStatement::get(int index) const
{
    return (*m_sortFields)[index];
}

orderByStatement& orderByStatement::reset()
{
    m_sortFields->clear();
    return *this;
}

//---------------------------------------------------------------------------
//   class reverseOrderStatement
//---------------------------------------------------------------------------
reverseOrderStatement* reverseOrderStatement::create()
{
    return new reverseOrderStatement();
}

void reverseOrderStatement::execute(recordset& rs)
{
    rs.reverse();
};

//---------------------------------------------------------------------------
//        struct queryStatementsImple
//---------------------------------------------------------------------------
struct queryStatementsImple
{
    idatabaseManager* dbm;
    database* db;
    int id;
    std::_tstring title;
    std::_tstring description;
    std::vector<executable*> statements;
    prepairedValues pv;

    inline queryStatementsImple() : dbm(NULL), db(NULL){};

    ~queryStatementsImple() { reset(); }

    void reset()
    {
        for (int i = 0; i < (int)statements.size(); ++i)
            statements[i]->release();
        statements.clear();
        title = _T("");
        description = _T("");
        id = 0;
    }

    inline void execute(recordset& rs, executeListner* listner)
    {
        for (size_t i = 0; i < statements.size(); ++i)
        {
            statements[i]->execute(rs);
            if (listner)
                listner->onExecuted(statements[i], rs);
        }
    }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

//---------------------------------------------------------------------------
//   struct queryStatementImple
//---------------------------------------------------------------------------
struct queryStatementImple
{
    struct queryStatementsImple* parent;
    std::_tstring database;
    std::_tstring table;
    int option;
    fieldNames* keyFields;
    client::query* query;
    readStatement::eReadType readType;
    short index;
    // int dbIndex;

    std::vector<alias_type> aliases;

    queryStatementImple(){};

    void alias(const _TCHAR* src, const _TCHAR* dst)
    {
        aliases.push_back(alias_type(src, dst));
    }

    inline void use(idatabaseManager* dbm, const connectParams* p)
    {
        /*dbIndex = dbm->findDbIndex(p);
        if (dbIndex==-1)
                return false;*/
        dbm->use(p);
        // return dbm->db()->isOpened();
    }

    inline void use(client::database* db, const connectParams* p)
    {
        if (db && db->isOpened())
            return;
        if (p)
            connectOpen(db, *p, false);
    }

    template <class Database> inline void execute(recordset& rs, Database db)
    {

        const _TCHAR* keys[8] = { NULL };
        for (int i = 0; i < keyFields->count(); ++i)
            keys[i] = parent->pv.replace(keyFields->getValue(i));

        client::query q;
        client::query* tq = replaceQueryParams(query, q, parent);

        if (database != _T(""))
        {
            connectParams p(database.c_str());
            use(db, &p);
        }
        else
            use(db, NULL);

        /*if (!isOpened(db, p))      //Change current db in dbm
                connect(db, p, false); //Change current db in dbm
        */

        activeTable at(db, table.c_str());
        at.index(index).option(option);
        for (int i = 0; i < (int)aliases.size(); ++i)
            at.alias(aliases[i].first.c_str(), aliases[i].second.c_str());
        if (readType == readStatement::opRead)
        {
            at.keyValue(keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],
                        keys[6], keys[7]);
            at.read(rs, *tq);
        }
        else if (readType == readStatement::opJoin)
            at.join(rs, *tq, keys[0], keys[1], keys[2], keys[3], keys[4],
                    keys[5], keys[6], keys[7]);
        else if (readType == readStatement::opOuterJoin)
            at.outerJoin(rs, *tq, keys[0], keys[1], keys[2], keys[3], keys[4],
                         keys[5], keys[6], keys[7]);
    }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& make_nvp("readType", readType);
        serialize_string(ar, "database", database);
        serialize_string(ar, "table", table);
        ar& make_nvp("index", index);
        int count = (int)aliases.size();
        ar& boost::serialization::make_nvp("alias_count", count);
        alias_type a;
        for (int i = 0; i < count; i++)
        {
            if (!Archive::is_loading::value)
                a = aliases[i];
            ar& make_nvp("alias", a);
            if (Archive::is_loading::value)
                aliases.push_back(a);
        }
    }
};

// template <class Archive>
// void serialize(Archive& ar, readStatement& q, const unsigned int version);

template <class Archive>
void queryStatementsImple::serialize(Archive& ar, const unsigned int version)
{
    ar& make_nvp("id", id);
    serialize_string(ar, "title", title);
    serialize_string(ar, "description", description);
    ar& make_nvp("items", statements);
    if (Archive::is_loading::value)
    {
        for (int i = 0; i < (int)statements.size(); ++i)
        {
            readStatement* p = dynamic_cast<readStatement*>(statements[i]);
            if (p)
                p->m_impl->parent = this;
            else
            {
                matchByStatement* pm =
                    dynamic_cast<matchByStatement*>(statements[i]);
                if (pm)
                    pm->m_parent = this;
                else
                {
                    groupByStatement* pg =
                        dynamic_cast<groupByStatement*>(statements[i]);
                    if (pg)
                        pg->m_parent = this;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//        class readStatement
//---------------------------------------------------------------------------
readStatement* readStatement::create()
{
    return new readStatement();
}

readStatement::readStatement() : m_impl(new queryStatementImple)
{
    m_impl->keyFields = this;
    m_impl->query = this;
}

readStatement::~readStatement()
{
    delete m_impl;
}

const _TCHAR* readStatement::getDatabaseUri() const
{
    return m_impl->database.c_str();
}

readStatement& readStatement::databaseUri(const _TCHAR* v)
{
    m_impl->database = v;
    return *this;
}

const _TCHAR* readStatement::getTableName() const
{
    return m_impl->table.c_str();
}

readStatement& readStatement::tableName(const _TCHAR* v)
{
    m_impl->table = v;
    return *this;
}

int readStatement::getIndex() const
{
    return m_impl->index;
}

readStatement& readStatement::index(int v)
{
    m_impl->index = v;
    return *this;
}

int readStatement::getOption() const
{
    return m_impl->option;
}

readStatement& readStatement::option(int v)
{
    m_impl->option = v;
    return *this;
}

readStatement::eReadType readStatement::getReadType() const
{
    return m_impl->readType;
}

readStatement& readStatement::readType(readStatement::eReadType v)
{
    m_impl->readType = v;
    return *this;
}

queryStatementImple* readStatement::internalPtr() const
{
    return m_impl;
}

readStatement& readStatement::alias(const _TCHAR* src, const _TCHAR* dst)
{
    m_impl->alias(src, dst);
    return *this;
}

readStatement& readStatement::reset()
{
    query::reset();
    fieldNames::reset();
    m_impl->aliases.clear();
    return *this;
}

int readStatement::aliasCount() const
{
    return (int)m_impl->aliases.size();
}

const _TCHAR* readStatement::getAliasFirst(int index) const
{
    return m_impl->aliases[index].first.c_str();
}

const _TCHAR* readStatement::getAliasSecond(int index) const
{
    return m_impl->aliases[index].second.c_str();
}

void readStatement::execute(recordset& rs)
{
    if (m_impl->parent->dbm)
        m_impl->execute(rs, m_impl->parent->dbm);
    else if (m_impl->parent->db)
        m_impl->execute(rs, m_impl->parent->db);
}

//---------------------------------------------------------------------------
//        class queryExecuter
//---------------------------------------------------------------------------
queryStatements* queryStatements::create(idatabaseManager& dbm)
{
    return new queryStatements(dbm);
}

queryStatements* queryStatements::create(database* db)
{
    return new queryStatements(db);
}

void queryStatements::release()
{
    delete this;
}

queryStatements::queryStatements(idatabaseManager& dbm)
    : m_impl(new queryStatementsImple)
{
    m_impl->dbm = &dbm;
}

queryStatements::queryStatements(database* db)
    : m_impl(new queryStatementsImple)
{
    m_impl->db = db;
}

queryStatements::~queryStatements()
{
    delete m_impl;
}

int queryStatements::getId() const
{
    return m_impl->id;
}

queryStatements& queryStatements::id(int v)
{
    m_impl->id = v;
    return *this;
}

const _TCHAR* queryStatements::getTitle() const
{
    return m_impl->title.c_str();
}

queryStatements& queryStatements::title(const _TCHAR* v)
{
    m_impl->title = v;
    return *this;
}

const _TCHAR* queryStatements::getDescription() const
{
    return m_impl->description.c_str();
}

queryStatements& queryStatements::description(const _TCHAR* v)
{
    m_impl->description = v;
    return *this;
}

readStatement* queryStatements::addRead(readStatement::eReadType type)
{
    readStatement* p = readStatement::create();
    p->readType(type);
    p->m_impl->parent = this->m_impl;
    m_impl->statements.push_back(p);
    return p;
}

readHasMany* queryStatements::addHasManyRead()
{
    readHasMany* p = readHasMany::create();
    m_impl->statements.push_back(p);
    return p;
}

groupByStatement* queryStatements::addGroupBy()
{
    groupByStatement* p = groupByStatement::create();
    p->m_parent = this->m_impl;
    m_impl->statements.push_back(p);
    return p;
}

orderByStatement* queryStatements::addOrderBy()
{
    orderByStatement* p = orderByStatement::create();
    m_impl->statements.push_back(p);
    return p;
}

matchByStatement* queryStatements::addMatchBy()
{
    matchByStatement* p = matchByStatement::create();
    p->m_parent = this->m_impl;
    m_impl->statements.push_back(p);
    return p;
}

reverseOrderStatement* queryStatements::addReverseOrder()
{
    reverseOrderStatement* p = reverseOrderStatement::create();
    m_impl->statements.push_back(p);
    return p;
}

executable* queryStatements::get(int index)
{
    return m_impl->statements[index];
}

void queryStatements::pop_back()
{
    m_impl->statements.pop_back();
}

int queryStatements::size() const
{
    return (int)m_impl->statements.size();
}

queryStatementsImple* queryStatements::internalPtr() const
{
    return m_impl;
}

void queryStatements::save(const _TCHAR* filename)
{
#ifdef _UNICODE
    char p[MAX_PATH];
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, filename, -1, p, MAX_PATH,
                        NULL, NULL);
#else
    const char* p = filename;

#endif
    std::ofstream file(p);
    xml_oarchive oa(file);
    queryStatementsImple& queryStatements = *m_impl;
    oa << BOOST_SERIALIZATION_NVP(queryStatements);
}

void queryStatements::load(const _TCHAR* filename)
{

#ifdef _UNICODE
    char p[MAX_PATH];
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, filename, -1, p, MAX_PATH,
                        NULL, NULL);

#else
    const char* p = filename;

#endif
    m_impl->reset();
    std::ifstream file(p);
    xml_iarchive ia(file);
    queryStatementsImple& queryStatements = *m_impl;
    ia >> BOOST_SERIALIZATION_NVP(queryStatements);
}

void queryStatements::save(std::stringstream& sf)
{
    xml_oarchive oa(sf);
    queryStatementsImple& queryStatements = *m_impl;
    oa << BOOST_SERIALIZATION_NVP(queryStatements);
}

void queryStatements::load(std::stringstream& sf)
{
    m_impl->reset();
    xml_iarchive ia(sf);
    queryStatementsImple& queryStatements = *m_impl;
    ia >> BOOST_SERIALIZATION_NVP(queryStatements);
}

void queryStatements::execute(recordset& rs,
                              const std::vector<std::_tstring>* values,
                              executeListner* listner)
{
    m_impl->pv.setValues(values);
    m_impl->execute(rs, listner);
}

void queryStatements::clear()
{
    m_impl->reset();
}

int queryStatements::statementType(int index)
{
    if (dynamic_cast<readHasMany*>(m_impl->statements[index]))
        return 2;
    if (dynamic_cast<readStatement*>(m_impl->statements[index]))
        return 1;
    if (dynamic_cast<groupByStatement*>(m_impl->statements[index]))
        return 3;
    if (dynamic_cast<orderByStatement*>(m_impl->statements[index]))
        return 4;
    if (dynamic_cast<matchByStatement*>(m_impl->statements[index]))
        return 5;
    if (dynamic_cast<reverseOrderStatement*>(m_impl->statements[index]))
        return 6;

    return 0;
}

readStatement* queryStatements::getReadStatement(executable* e)
{
    return dynamic_cast<readStatement*>(e);
}

readHasMany* queryStatements::getReadHasMany(executable* e)
{
    return dynamic_cast<readHasMany*>(e);
}

groupByStatement* queryStatements::getGroupByStatement(executable* e)
{
    return dynamic_cast<groupByStatement*>(e);
}

orderByStatement* queryStatements::getOrderByStatement(executable* e)
{
    return dynamic_cast<orderByStatement*>(e);
}

matchByStatement* queryStatements::getMatchByStatement(executable* e)
{
    return dynamic_cast<matchByStatement*>(e);
}

reverseOrderStatement* queryStatements::getReverseOrderStatement(executable* e)
{
    return dynamic_cast<reverseOrderStatement*>(e);
}

const readStatement*
queryStatements::getReadStatement(const executable* e) const
{
    return dynamic_cast<const readStatement*>(e);
}

const readHasMany* queryStatements::getReadHasMany(const executable* e) const
{
    return dynamic_cast<const readHasMany*>(e);
}

const groupByStatement*
queryStatements::getGroupByStatement(const executable* e) const
{
    return dynamic_cast<const groupByStatement*>(e);
}

const orderByStatement*
queryStatements::getOrderByStatement(const executable* e) const
{
    return dynamic_cast<const orderByStatement*>(e);
}

const matchByStatement*
queryStatements::getMatchByStatement(const executable* e) const
{
    return dynamic_cast<const matchByStatement*>(e);
}

const reverseOrderStatement*
queryStatements::getReverseOrderStatement(const executable* e) const
{
    return dynamic_cast<const reverseOrderStatement*>(e);
}

//---------------------------------------------------------------------------
//        struct readHasManyImple
//---------------------------------------------------------------------------

struct readHasManyImple
{
    fieldNames columns;
    recordsets rss;

    void reset()
    {
        rss.clear();
        columns.reset();
    }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& make_nvp("columns", columns);
    }
};

//---------------------------------------------------------------------------
//        class readHasMany
//---------------------------------------------------------------------------

readHasMany::readHasMany()
    : readStatement(), m_readHasManyImpl(new readHasManyImple)
{
}

readHasMany::~readHasMany()
{
    delete m_readHasManyImpl;
}

readHasManyImple* readHasMany::internalPtr() const
{
    return m_readHasManyImpl;
}

recordsets& readHasMany::recordsets()
{
    return m_readHasManyImpl->rss;
}

void readHasMany::addkeyValueColumn(const _TCHAR* name)
{
    m_readHasManyImpl->columns.addValue(name);
}

const _TCHAR* readHasMany::getkeyValueColumn(int index) const
{
    return m_readHasManyImpl->columns.getValue(index);
}

int readHasMany::keyValueColumns() const
{
    return m_readHasManyImpl->columns.count();
}

readHasMany& readHasMany::reset()
{
    readStatement::reset();
    m_readHasManyImpl->reset();
    return *this;
}

void readHasMany::execute(recordset& rs)
{
    m_readHasManyImpl->rss.clear();

    std::vector<int> indexes;
    const fielddefs* fds = rs.fieldDefs();
    for (int i = 0; i < m_readHasManyImpl->columns.count(); ++i)
        indexes.push_back(fds->indexByName(getkeyValueColumn(i)));

    for (int i = 0; i < (int)rs.size(); ++i)
    {
        fieldNames::reset();
        // setkey values
        for (int j = 0; j < (int)indexes.size(); ++j)
        {
            const _TCHAR* p = rs[i][indexes[j]].c_str();
            addValue(p);
            if (j == 0)
                addLogic(getkeyValueColumn(j), _T("="), p);
            else
                addLogic(_T("and"), getkeyValueColumn(j), _T("="), p);
        }
        recordset* tmp = new recordset();
        boost::shared_ptr<recordset> r(tmp,
                                       boost::bind(&recordset::release, tmp));
        m_readHasManyImpl->rss.push_back(r);
        readStatement::execute(*r);
    }
}

client::query* replaceQueryParams(client::query* tq, client::query& tmpq,
                                  queryStatementsImple* parent)
{

    int n = 0;
    if (tq)
        n = tq->whereTokens();
    else
        n = tmpq.whereTokens();
    if (n)
    {
        if (tq)
            tmpq = *tq;
        for (int i = 0; i < n; ++i)
        {
            if (_tcscmp(tmpq.getWhereToken(i), _T("?")) == 0)
                tmpq.setWhereToken(i,
                                   parent->pv.replace(tmpq.getWhereToken(i)));
        }
        return &tmpq;
    }
    return tq;
}

readHasMany* readHasMany::create()
{
    return new readHasMany();
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#ifdef BCB_32
#pragma option pop
#endif
