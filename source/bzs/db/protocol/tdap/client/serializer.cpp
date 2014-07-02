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

#if (__BCPLUSPLUS__)
#   ifdef _WIN64
#	    pragma comment(lib, "boost_serialization-bcb64-mt-1_50.a")
#   else
#       ifdef _RTLDLL
#			pragma comment(lib, "boost_serialization-bcb-mt-1_39.lib")
#   	else
#			pragma comment(lib, "libboost_serialization-bcb-mt-s-1_39.lib")
#	   	endif
#   endif
#endif

using namespace boost::archive;
using namespace boost::serialization;

BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::sum, "sum");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::count, "count");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::avg, "avg");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::min, "min");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::max, "max");

BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::readStatement, "readStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::orderByStatement, "orderByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::matchByStatement, "matchByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::groupByStatement, "groupByStatement");
BOOST_CLASS_EXPORT_GUID(bzs::db::protocol::tdap::client::reverseOrderStatement, "reverseOrderStatement");

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

void toU8(std::_tstring& src, std::string& dst)
{
#ifdef _UNICODE
	char buf[2048];
	WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, buf, 1024, NULL, NULL);
	dst = buf;
	
#else
	dst = src;
#endif
}

void fromU8(std::string& src, std::_tstring& dst)
{
#ifdef _UNICODE
	wchar_t buf[2048];
	MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buf, 1024);
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
	ar & boost::serialization::make_nvp(name , s);
	if (Archive::is_loading::value)
		fromU8(s, v);
}

template <class Archive>
void serialize(Archive& , executable& , const unsigned int )
{

}

template <class Archive>
void serialize(Archive& ar, sortField& q, const unsigned int )
{
	
	serialize_string(ar, "name" , q.name);
	ar & boost::serialization::make_nvp("asc" , q.asc);

}

template <class Archive>
void serialize(Archive& ar, sortFields& q, const unsigned int )
{
	int count = (int)q.m_params.size();
	ar & boost::serialization::make_nvp("count" , count);
	for (int i=0;i<count;i++)
	{
		if (Archive::is_loading::value)
		{
			sortField f;
			ar & boost::serialization::make_nvp("field" , f);
			q.m_params.push_back(f);
		}
		else
			ar & boost::serialization::make_nvp("field" , q.m_params[i]);
	}

}

template <class Archive>
void serialize(Archive& ar, groupByStatement& q, const unsigned int /*version*/)
{
	boost::serialization::base_object<executable>(q);
	ar & boost::serialization::make_nvp("keyFields"
				, boost::serialization::base_object<fieldNames>(q));
	ar & boost::serialization::make_nvp("functions" , *q.m_statements);
}

template <class Archive>
void serialize(Archive& ar, matchByStatement& q, const unsigned int /*version*/)
{
	boost::serialization::base_object<executable>(q);
	ar & make_nvp("matchByStatement", boost::serialization::base_object<recordsetQuery>(q));
}

template <class Archive>
void serialize(Archive& ar, orderByStatement& q, const unsigned int /*version*/)
{
	boost::serialization::base_object<executable>(q);
	ar & boost::serialization::make_nvp("sortFields", *q.m_sortFields);

}

template <class Archive>
void serialize(Archive& /*ar*/, reverseOrderStatement& q, const unsigned int /*version*/)
{
	boost::serialization::base_object<executable>(q);
}

template <class Archive>
void serialize(Archive& ar, readStatement& q, const unsigned int /*version*/)
{
	boost::serialization::base_object<executable>(q);
	ar & boost::serialization::make_nvp("keyFields", boost::serialization::base_object<fieldNames>(q));
	ar & boost::serialization::make_nvp("query", boost::serialization::base_object<query>(q));
	ar & boost::serialization::make_nvp("params", *q.internalPtr());
}

template <class Archive>
void serialize(Archive& ar, queryBase& q, const unsigned int version)
{
	split_free(ar, q, version);
}


template<class Archive>
void save(Archive& ar,  const queryBase& q,  const unsigned int /*version*/)
{
	std::_tstring s = q.toString();
	
	serialize_string(ar, "queryString", s);
	int v = q.getReject();
	ar & make_nvp("reject", v);
	v = q.getLimit();
	ar & make_nvp("limit", v);
	v = q.getOptimize();
	ar & make_nvp("optimize", v);
	v = q.isBookmarkAlso();
	ar & make_nvp("boolmarkAlso", v);
	v = q.isAll();
	ar & make_nvp("isAll", v);
}

template<class Archive>
void load(Archive& ar, queryBase& q,  const unsigned int /*version*/)
{
	std::_tstring s;
	int v;

	q.reset();
	serialize_string(ar, "queryString", s);
	q.queryString(s.c_str());

	ar & make_nvp("reject", v);
	q.reject(v);

	ar & make_nvp("limit", v);
	q.limit(v);

	ar & make_nvp("optimize", v);
	q.optimize((queryBase::eOptimize)v);

	ar & make_nvp("boolmarkAlso", v);
	q.bookmarkAlso(v != 0);

	ar & make_nvp("isAll", v);
	if (v) q.all();
}

template <class Archive>
void serialize(Archive& ar, fieldNames& q, const unsigned int /*version*/)
{
	int count = q.count();
	ar & boost::serialization::make_nvp("count", count);
	std::_tstring s;
	for (int i=0;i<count;i++)
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
	ar & make_nvp("readQuery", boost::serialization::base_object<queryBase>(q));
}

template <class Archive>
void serialize(Archive& ar, recordsetQuery& q, const unsigned int /*version*/)
{
	queryBase* qq = q.internalQuery();
	ar & make_nvp("recordsetQuery", *qq);
}

template <class Archive>
void serialize(Archive& ar, groupFuncBase& q, const unsigned int /*version*/)
{
	ar & boost::serialization::make_nvp("query"
				, boost::serialization::base_object<recordsetQuery>(q));
	std::_tstring s;
	if (Archive::is_loading::value)
	{

		serialize_string(ar, "targetName", s);
		q.setTargetName(s.c_str());
		serialize_string(ar, "resultName", s);
		q.setResultName(s.c_str());

	}else
	{
		if (q.targetName())
			s = q.targetName();
		else
			s = _T("");
		serialize_string(ar, "targetName", s);
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
	 ar & make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, count& q, const unsigned int /*version*/)
{
	 ar & make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, avg& q, const unsigned int /*version*/)
{
	 ar & make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, min& q, const unsigned int /*version*/)
{
	 ar & make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, max& q, const unsigned int /*version*/)
{
	 ar & make_nvp("param", boost::serialization::base_object<groupFuncBase>(q));
}

template <class Archive>
void serialize(Archive& ar, groupQuery& q, const unsigned int /*version*/)
{
	fieldNames& v = const_cast<fieldNames&>(q.getKeyFields());
	ar & make_nvp("keyFields", v);
}

struct aliasPair
{
	std::_tstring first;
	std::_tstring second;
	aliasPair(){}
	aliasPair(const _TCHAR* f, const _TCHAR* s):first(f),second(s){}
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
		if (_tcscmp(v, _T("?"))==0)
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
groupByStatement::groupByStatement()
	:fieldNames(),m_statements(new std::vector< groupFuncBase* >()){}

groupByStatement::~groupByStatement()
{
	reset();
	delete m_statements;
}

groupFuncBase& groupByStatement::addFunction(eFunc v, const _TCHAR* targetName , const _TCHAR* resultName)
{
	groupFuncBase* func;
	switch(v)
	{
	case fsum:func =   new client::sum(targetName, resultName);break;
	case fcount:func = new client::count(targetName);break;
	case favg:func =   new client::avg(targetName, resultName);break;
	case fmin:func =   new client::min(targetName, resultName);break;
	case fmax:func =   new client::max(targetName, resultName);break;
	};
	m_statements->push_back(func);
	return *func;
}


groupFuncBase& groupByStatement::function(int index)
{
	assert(index >= 0 && index < (int)m_statements->size());
	return *((*m_statements)[index]);
}

fieldNames& groupByStatement::reset()
{
	for (int i=0;i<(int)m_statements->size();++i)
		delete ((*m_statements)[i]);
	m_statements->clear();
	return *this;
}

int groupByStatement::size() const {return (int)m_statements->size();}

void groupByStatement::execute(recordset& rs)
{
    const _TCHAR* keys[8]={NULL};
	for (int i=0;i<count();++i)
			keys[i] = getValue(i);
	groupQuery q;
	q.keyField(keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],keys[6], keys[7]);

	for (int i=0;i<(int)m_statements->size();++i)
		q.addFunction(((*m_statements)[i]));
	rs.groupBy(q);
}

//---------------------------------------------------------------------------
//   class matchByStatement
//---------------------------------------------------------------------------
void matchByStatement::execute(recordset& rs){rs.matchBy(*this);};

//---------------------------------------------------------------------------
//   class orderByStatement
//---------------------------------------------------------------------------
orderByStatement::orderByStatement():m_sortFields(new sortFields()){}

orderByStatement::~orderByStatement(){delete m_sortFields;}

void orderByStatement::execute(recordset& rs){rs.orderBy(*m_sortFields);};

void orderByStatement::add(const _TCHAR* name, bool  asc)
{
	m_sortFields->add(name, asc);
}

//---------------------------------------------------------------------------
//   class reverseOrderStatement
//---------------------------------------------------------------------------
void reverseOrderStatement::execute(recordset& rs){rs.reverse();};


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

	inline queryStatementsImple():dbm(NULL),db(NULL){};

	~queryStatementsImple()
	{
		reset();
	}

	void reset()
	{
		 for(int i=0;i<(int)statements.size();++i)
			delete statements[i];
		 statements.clear();
	}

	inline void execute(recordset& rs)
	{
		for (size_t i=0;i<statements.size();++i)
				statements[i]->execute(rs);
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
	
	std::vector<alias_type> aliases;

	queryStatementImple(){};

	void alias(const _TCHAR* src, const _TCHAR* dst)
	{
		aliases.push_back(alias_type(src, dst));
	}


	inline const client::database* getDatabase(idatabaseManager* dbm)
	{
		return dbm->db(table.c_str());
	}

	inline const client::database* getDatabase(client::database* db)
	{
		return db;
	}

	template <class Database>
	inline void execute(recordset& rs, Database db)
	{

		const _TCHAR* keys[8]={NULL};
		for (int i=0;i<keyFields->count();++i)
			keys[i] = parent->pv.replace(keyFields->getValue(i));

		client::query* tq = query;
		int n = tq->whereTokens();
		client::query q;
		if (n)
		{
			q = *query;
			for (int i=0;i<n;++i)
			{
				if (_tcscmp(q.getWhereToken(i), _T("?")) == 0)
					q.setWhereToken(i, parent->pv.replace(q.getWhereToken(i)));
			}
			tq = &q;
		}

		if (!getDatabase(db)->isOpened())
		{
			connectParams p(database.c_str());
			connect(db, p, false);
		}
		activeTable at(db, table.c_str());
		at.index(index).option(option);
		for (int i=0;i<(int)aliases.size();++i)
			at.alias(aliases[i].first.c_str(), aliases[i].second.c_str());
		if (readType == readStatement::opRead)
		{
			at.keyValue(keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],keys[6], keys[7]);
			at.read(rs, *tq);
		}
		else if (readType == readStatement::opJoin)
			at.join(rs, *tq, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],keys[6], keys[7]);
		else if(readType == readStatement::opOuterJoin)
			at.outerJoin(rs, *tq, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],keys[6], keys[7]);
	}

private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & make_nvp("readType", readType);

		if (!Archive::is_loading::value)
		{
			if (parent->dbm && getDatabase(parent->dbm)->isOpened()) database = parent->dbm->db(table.c_str())->uri();
			if (parent->db && getDatabase(parent->db)->isOpened()) database = parent->db->uri();
		}
		serialize_string(ar, "database", database);
		serialize_string(ar, "table", table);
		ar & make_nvp("index", index);
		//ar & make_nvp("aliases", aliases);

		int count = (int)aliases.size();
		ar & boost::serialization::make_nvp("alias_count" , count);
		alias_type a;
		for (int i=0;i<count;i++)
		{
			if (!Archive::is_loading::value)
				 a = aliases[i];
			ar & make_nvp("alias", a);
			if (Archive::is_loading::value)
				aliases.push_back(a);
		}
	}
};


template <class Archive>
void serialize(Archive& ar, readStatement& q, const unsigned int version);


template <class Archive>
void queryStatementsImple::serialize(Archive& ar, const unsigned int version)
{
	ar & make_nvp("id", id);
	serialize_string(ar, "title", title);
	serialize_string(ar, "description", description);
	ar & make_nvp("items", statements);
	if (Archive::is_loading::value)
	{
		for (int i=0;i<(int)statements.size();++i)
		{
			readStatement* p = dynamic_cast<readStatement*>(statements[i]);
			if (p)
				p->m_impl->parent = this;
		}
	}
}

//---------------------------------------------------------------------------
//        class readStatement
//---------------------------------------------------------------------------
readStatement::readStatement():m_impl(new queryStatementImple)
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

queryStatements::queryStatements(idatabaseManager& dbm):m_impl(new queryStatementsImple)
{
	m_impl->dbm = &dbm;
}

queryStatements::queryStatements(database* db):m_impl(new queryStatementsImple)
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
	readStatement* p = new readStatement();
	p->readType(type);
	p->m_impl->parent = this->m_impl;
	m_impl->statements.push_back(p);
	return p;
}

groupByStatement* queryStatements::addGroupBy()
{
	groupByStatement* p = new groupByStatement();
	m_impl->statements.push_back(p);
	return p;
}

orderByStatement* queryStatements::addOrderBy()
{
	orderByStatement* p = new orderByStatement();
	m_impl->statements.push_back(p);
	return p;
}

matchByStatement* queryStatements::addMatchBy()
{
	matchByStatement* p = new matchByStatement();
	m_impl->statements.push_back(p);
	return p;
}

reverseOrderStatement* queryStatements::addReverseOrder()
{
	reverseOrderStatement* p = new reverseOrderStatement();
	m_impl->statements.push_back(p);
	return p;
}

executable* queryStatements::get(int index)
{
	return m_impl->statements[index];
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
	std::ofstream file(filename);
	xml_oarchive oa(file);
	queryStatementsImple& queryStatements = *m_impl;
	oa << BOOST_SERIALIZATION_NVP(queryStatements);
}

void queryStatements::load(const _TCHAR* filename)
{
	m_impl->reset();

	std::ifstream file(filename);
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

void queryStatements::execute(recordset& rs, const std::vector<std::_tstring>* values)
{
	m_impl->pv.setValues(values);
	m_impl->execute(rs);
}




}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#ifdef BCB_32
#pragma option pop
#endif




