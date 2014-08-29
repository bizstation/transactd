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
#else
#    ifdef BCB_32
#    	define DLLLIBSTMT AGRPACK
#    else
#		define DLLLIBSTMT //PACKAGE_IMPORT
#	 endif
#endif //LIB_TDCLSTMT

#if (defined(TRDCL_AUTOLINK) && !defined(LIB_TDCLSTMT))
#	include "trdclcppautolink.h"
#	define TD_STMT_LIB_NAME LIB_PREFIX "tdclstmt" CPP_INTERFACE_VERSTR SHARED_LIB_EXTENTION
#	pragma comment(lib, TD_STMT_LIB_NAME)
#endif //TRDCL_AUTOLINK


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
#define ID_READ_STMT				0
#define ID_READHASMANY_STMT			1
#define ID_GROUPBY_STMT				2
#define ID_ORDERBY_STMT				3
#define ID_REVORDER_STMT			4
#define ID_MATCHBY_STMT				5
#define ID_MAX_STMT					6



class  DLLLIBSTMT  executable
{
	friend struct queryStatementsImple;

public:
	virtual ~executable(){};
	virtual void execute(recordset& rs)=0;
	void release();
	virtual const int typeID()const = 0;

};

class DLLLIBSTMT groupByStatement : public fieldNames, public executable
{
	struct queryStatementsImple* m_parent;
	friend class queryStatements;
	friend struct queryStatementsImple;
	std::vector< groupFuncBase* >* m_statements;

	template <class Archive>
	friend void serialize(Archive& ar, groupByStatement& q, const unsigned int version);
	groupByStatement(const groupByStatement& r);           //no implements
	groupByStatement& operator=(const groupByStatement& r);//no implements


public:
	enum eFunc{fsum, fcount, favg, fmin, fmax};
	groupByStatement();
	~groupByStatement();

	groupFuncBase& addFunction(eFunc v, const fieldNames& targetNames , const _TCHAR* resultName=NULL);
	groupFuncBase& function(int index);
	groupByStatement& reset();
	int size() const;
	void execute(recordset& rs);
	const int typeID()const {return ID_GROUPBY_STMT;};
	static groupByStatement* create();

};

#define MAX_FUNCTION_SIZE (int)groupByStatement::fmax + 1


class DLLLIBSTMT  matchByStatement : public recordsetQuery, public executable
{
	struct queryStatementsImple* m_parent;
	friend class queryStatements;
	friend struct queryStatementsImple;
public:
	void execute(recordset& rs);
	static matchByStatement* create();
	const int typeID()const {return ID_MATCHBY_STMT;};
};

class DLLLIBSTMT orderByStatement : public executable
{
	sortFields* m_sortFields;
	template <class Archive>
	friend void serialize(Archive& ar, orderByStatement& q, const unsigned int version);
	orderByStatement(const orderByStatement& r);           //no implements
	orderByStatement& operator=(const orderByStatement& r);//no implements
public:
	~orderByStatement();
	orderByStatement();
	void execute(recordset& rs);
	void add(const _TCHAR* name, bool  asc=true);
	orderByStatement& reset();
	int size() const;
	const sortField& get(int index) const;
	const int typeID()const {return ID_ORDERBY_STMT;};
	static orderByStatement* create();
};

class DLLLIBSTMT reverseOrderStatement :  public executable
{
	//~reverseOrderStatement(){};
	//reverseOrderStatement():executable(){};

public:
	void execute(recordset& rs);
	const int typeID()const {return ID_REVORDER_STMT;};
	static reverseOrderStatement* create();
};

class DLLLIBSTMT readStatement : public fieldNames, public query, public executable
{
	friend struct queryStatementsImple;
	friend class queryStatements;

	struct queryStatementImple* m_impl;
	queryStatementImple* internalPtr() const;
	template <class Archive>
	friend void serialize(Archive& ar, readStatement& q, const unsigned int version);
	readStatement(const readStatement& r);           //no implements
	readStatement& operator=(const readStatement& r);//no implements
public:
	~readStatement();
	readStatement();

	enum eReadType{opRead, opJoin, opOuterJoin};
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
	readStatement& reset();
	int aliasCount() const;
	const _TCHAR* getAliasFirst(int index) const;
	const _TCHAR* getAliasSecond(int index) const;

	void execute(recordset& rs);
	const int typeID()const {return ID_READ_STMT;};
	static readStatement* create();

};

typedef std::vector<boost::shared_ptr<recordset> > recordsets;


class DLLLIBSTMT readHasMany : public readStatement
{
	struct readHasManyImple* m_readHasManyImpl;
	readHasManyImple* internalPtr() const;
	template <class Archive>
	friend void serialize(Archive& ar, readHasMany& q, const unsigned int version);
	readHasMany(const readHasMany& r);           //no implements
	readHasMany& operator=(const readHasMany& r);//no implements
public:
	~readHasMany();
	readHasMany();
	client::recordsets& recordsets();
	void addkeyValueColumn(const _TCHAR* name);
	const _TCHAR* getkeyValueColumn(int index) const;
	int  keyValueColumns() const;
	readHasMany& reset();
	void execute(recordset& rs);
	const int typeID()const {return ID_READHASMANY_STMT;};
	static readHasMany* create();

};


class executeListner
{
public:
	virtual void onExecuted(const executable* e, const recordset& rs) = 0;
};

class DLLLIBSTMT queryStatements
{
	struct queryStatementsImple* m_impl;
	queryStatementsImple* internalPtr() const;
	template <class Archive>
	friend void serialize(Archive& ar, queryStatements& q, const unsigned int version);
	queryStatements(const queryStatements& r);           //no implements
	queryStatements& operator=(const queryStatements& r);//no implements
	void* getPtr(const executable* e) const;
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
	void pop_back();


	readStatement* addRead(readStatement::eReadType type);
	readHasMany* addHasManyRead();
	groupByStatement* addGroupBy();
	orderByStatement* addOrderBy();
	matchByStatement* addMatchBy();
	reverseOrderStatement* addReverseOrder();
	int statementType(int index);

	readStatement* getReadStatement(executable* e);
	readHasMany* getReadHasMany(executable* e);
	groupByStatement* getGroupByStatement(executable* e);
	orderByStatement* getOrderByStatement(executable* e);
	matchByStatement* getMatchByStatement(executable* e);
	reverseOrderStatement* getReverseOrderStatement(executable* e);

	const readStatement* getReadStatement(const executable* e)const ;
	const readHasMany* getReadHasMany(const executable* e)const ;
	const groupByStatement* getGroupByStatement(const executable* e)const ;
	const orderByStatement* getOrderByStatement(const executable* e)const ;
	const matchByStatement* getMatchByStatement(const executable* e)const ;
	const reverseOrderStatement* getReverseOrderStatement(const executable* e)const ;


	void clear();
	int size() const;
	void save(const _TCHAR* filename);
	void load(const _TCHAR* filename);
	void save(std::stringstream& sf);
	void load(std::stringstream& sf);
	void execute(recordset& rs, const std::vector<std::_tstring>* values=NULL, executeListner* listner=NULL);
	static queryStatements* create(idatabaseManager& dbm);
	static queryStatements* create(database* db);

	void release();

};


/** @endcond */

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_SERIALIZEER_H
