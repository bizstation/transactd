#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_SERIALIZEER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_SERIALIZEER_H
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
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <sstream>

#ifdef LIB_TDCLSTMT
#    define DLLLIBSTMT AGRPACK
#    define DLLTEMPLATE
#else
#    ifdef BCB_32
#    	define DLLLIBSTMT AGRPACK
#    else
#		define DLLLIBSTMT PACKAGE_IMPORT
#	 endif
#    define DLLTEMPLATE extern
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


class executable
{

public:
	virtual ~executable(){};
	virtual void execute(recordset& rs)=0;
};

class DLLLIBSTMT groupByStatement : public fieldNames, public executable
{
	std::vector< groupFuncBase* > m_statements;

	template <class Archive>
	friend void serialize(Archive& ar, groupByStatement& q, const unsigned int version);


public:
	enum eFunc{fsum, fcount, favg, fmin, fmax};
	~groupByStatement();
	groupFuncBase& addFunction(eFunc v, const _TCHAR* targetName , const _TCHAR* resultName=NULL);
	groupFuncBase& function(int index);
	fieldNames& reset();
	int size() const;
	void execute(recordset& rs);
};


class DLLLIBSTMT matchByStatement : public recordsetQuery, public executable
{
public:
	void execute(recordset& rs);
};

class DLLLIBSTMT orderByStatement : public fieldNames, public executable
{

public:
	void execute(recordset& rs);
};

class DLLLIBSTMT reverseOrderStatement :  public executable
{

public:
	void execute(recordset& rs);
};

class DLLLIBSTMT readStatement : public fieldNames, public query, public executable
{
	friend class queryStatementsImple;
	friend class queryStatements;

	struct queryStatementImple* m_impl;
	queryStatementImple* internalPtr() const;
	template <class Archive>
	friend void serialize(Archive& ar, readStatement& q, const unsigned int version);

public:
	enum eReadType{opRead, opJoin, opOuterJoin};
	readStatement();
	~readStatement();
	int getIndex() const ;
	readStatement& index(int v);
	int getOption() const;
	readStatement& option(int v);
	const _TCHAR* getDatabaseUri() const;
	readStatement& databaseUri(const _TCHAR* v);
	const _TCHAR* getTableName() const;
	readStatement& tableName(const _TCHAR* v);
	eReadType getReadType() const;
	readStatement& readType(eReadType v);
	readStatement& alias(const _TCHAR* src, const _TCHAR* dst);
	void execute(recordset& rs);

};

class DLLLIBSTMT queryStatements
{
	struct queryStatementsImple* m_impl;
	queryStatementsImple* internalPtr() const;
	template <class Archive>
	friend void serialize(Archive& ar, queryStatements& q, const unsigned int version);

public:
	queryStatements(idatabaseManager& dbm);
	queryStatements(database* db);
	~queryStatements();
	int getId() const ;
	queryStatements& id(int v);
	const _TCHAR* getTitle() const ;
	queryStatements& title(const _TCHAR* v);
	const _TCHAR* getDescription() const ;
	queryStatements& description(const _TCHAR* v);
	executable* get(int index);

	readStatement* addRead(readStatement::eReadType type);
	groupByStatement* addGroupBy();
	orderByStatement* addOrderBy();
	matchByStatement* addMatchBy();
	reverseOrderStatement* addReverseOrder();

	int size() const;
	void save(const _TCHAR* filename);
	void load(const _TCHAR* filename);
	void save(std::stringstream& sf);
	void load(std::stringstream& sf);
	void execute(recordset& rs, const std::vector<std::_tstring>* values=NULL);
};




}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_SERIALIZEER_H
