
#include <iostream>
#include <locale.h>
#include <boost/iterator/iterator_facade.hpp>
#include "ormRecordset.h"
#include "benchmark.h"

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;



static const char_td keynum_group = 1;
static const char_td primary_key = 0;

void showConsole(recordset& rowset)
{

	 const tdc::fielddefs& fields = *rowset.fieldDefs();
	 for (int j=0;j<(int)fields.size();++j)
		 std::tcout << fields[j].name()  << _T("\t");
	 std::tcout << _T("\n");

	for (int i=0;i<(int)rowset.size();++i)
	{
		row& m = *rowset[i];
		for (int j=0;j<(int)m.size();++j)
		{
			std::tcout << m[(size_t)j].c_str()  << _T("\t");
			if (j == (int)m.size() -1)
			   std::tcout << _T("\n");
		}
	}
}

bool showDbdefError(dbdef* def, _TCHAR* msg)
{
	 std::tcout << msg << _T(" erorr:No.") << def->stat();
	 return false;
}

/*--------------------------------------------------------------------------------*/
bool createUserTable(dbdef* def)
{
	short tableid = 1;
	tabledef t;
	memset(&t, 0, sizeof(t));
	tabledef* td = &t;
	td->id = tableid;
	td->setTableName(_T("user"));
	td->setFileName(_T("user"));
	def->insertTable(td);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("user insertTable"));

	short filedIndex = 0;
	fielddef* fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("id"));
	fd->type = ft_autoinc;
	fd->len = 4;

	++filedIndex;
	fd = def->insertField(tableid, filedIndex);
	fd->setName(_T("–¼‘O"));
	fd->type = ft_myvarchar;
	fd->len = 33;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("group"));
	fd->type = ft_integer;
	fd->len = 4;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("tel"));
	fd->type = ft_myvarchar;
	fd->len = 21;

	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment& seg1 = kd->segments[0];
	seg1.fieldNum = 0;
	seg1.flags.bit8 = true;    //extended key type
	seg1.flags.bit1 = true;//chanageable
	kd->segmentCount = 1;
	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;

	++keyNum;
	kd = def->insertKey(tableid, keyNum);
	seg1 = kd->segments[0];
	seg1.fieldNum = 2;
	seg1.flags.bit0 = true;
	seg1.flags.bit8 = true;
	seg1.flags.bit1 = true;
	kd->segmentCount = 1;

	def->updateTableDef(tableid);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("user updateTableDef"));

	return true;
}

/*--------------------------------------------------------------------------------*/
bool createGroupTable(dbdef* def)
{
	short tableid = 2;
	tabledef t;
	tabledef* td = &t;
	memset(&t, 0, sizeof(t));
	td->id = tableid;
	td->setTableName(_T("groups"));
	td->setFileName(_T("groups"));

	def->insertTable(td);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("groups insertTable"));

	short filedIndex = 0;
	fielddef* fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("code"));
	fd->type = ft_integer;
	fd->len = 4;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("name"));
	fd->type = ft_myvarbinary;
	fd->len = 33;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("name2"));
	fd->type = ft_myvarbinary;
	fd->len = 33;

	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment& seg1 = kd->segments[0];
	seg1.fieldNum = 0;
	seg1.flags.bit8 = true;    //extended key type
	seg1.flags.bit1 = true;//chanageable
	kd->segmentCount = 1;

	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;
	td->flags.bit0 = true;
	def->updateTableDef(tableid);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("groups updateTableDef"));
	return true;
}

/*--------------------------------------------------------------------------------*/
bool createUserExtTable(dbdef* def)
{
	short tableid = 3;
	tabledef t;
	memset(&t, 0, sizeof(t));
	tabledef* td = &t;
	td->id = tableid;
	td->setTableName(_T("extention"));
	td->setFileName(_T("extention"));

	def->insertTable(td);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("extention insertTable"));

	short filedIndex = 0;
	fielddef* fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("id"));
	fd->type = ft_integer;
	fd->len = 4;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("comment"));
	fd->type = ft_myvarchar;
	fd->len = 255;


	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment& seg1 = kd->segments[0];
	seg1.fieldNum = 0;
	seg1.flags.bit8 = true;    //extended key type
	seg1.flags.bit1 = true;//chanageable
	kd->segmentCount = 1;
	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;
	def->updateTableDef(tableid);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("extention updateTableDef"));
	return true;
}

bool insertData(database_ptr db)
{
	_TCHAR tmp[256];
	table* tb = db->openTable(_T("user"), TD_OPEN_NORMAL);
	tb->clearBuffer();
	for (int i= 1;i<= 20000;++i)
	{
		tb->setFV((short)0, i);
		_stprintf(tmp, _T("%d user"), i);
		tb->setFV(1, tmp);
		tb->setFV(_T("group"), ((i -1) % 100) + 1);
		tb->insert();
	}
	tb->release();

	tb = db->openTable(_T("groups"), TD_OPEN_NORMAL);
	tb->clearBuffer();
	for (int i= 1;i<= 100;++i)
	{
		tb->setFV((short)0, i);
		_stprintf(tmp, _T("%d group"), i);
		tb->setFV(1, tmp);
		tb->insert();
	}
	tb->release();
	tb = db->openTable(_T("extention"), TD_OPEN_NORMAL);
	tb->clearBuffer();
	for (int i= 1;i<= 20000;++i)
	{
		tb->setFV((short)0, i);
		_stprintf(tmp, _T("%d comment"), i);
		tb->setFV(1, tmp);
		tb->insert();
	}
	tb->release();
	return true;
}


tdc::query q;
groupQuery gq;

void initQuery()
{
	q.reset();
	gq.reset();
}

bool btest(recordset* rsp, queryTable* atup, queryTable* atgp, queryTable* atep, int kind, int n)
{
	queryTable& atu = *atup;
	queryTable& atg = *atgp;
	queryTable& ate = *atep;
	recordset& rs = *rsp;


	for (int i= 0;i<n;++i)
	{
		rs.clear();
		if (kind &1)
		{
			initQuery();
			atu.alias(_T("–¼‘O"), _T("name"));

			q.select(_T("id"), _T("name"),_T("group"))
					.where(_T("id"), _T("<="), i+15000);

			atu.index(0).keyValue(i+1).read(rs, q);

		//Join extention::comment
			if (kind & 2)
			{
				initQuery();
				ate.index(0).join(rs, q.select(_T("comment")).optimize(queryBase::hasOneJoin), _T("id"));
			}
			if (kind & 4)
			{
				//Join group::name
				initQuery();
				atg.alias(_T("name"), _T("group_name"));
				atg.index(0).join(rs, q.select(_T("group_name")), _T("group"));
			}
		}
		/*
		size_t count = rs.count;
		for (size_t j= 0;j<count;++j)
			 record& s = rs.record(j);*/

	}
	return true;
}


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale("japanese"));
	database_ptr db = createDatadaseObject();
	try
	{
		_TCHAR* host = _T("localhost");
		int kind = 7;
		int n = 100;
		bool makeDatabase = true;

		if (argc >= 5)
			n = _ttol(argv[4]);
		if (argc >= 4)
			kind = _ttol(argv[3]);
		if (argc >= 3)
			host = argv[2];
		if (argc >= 2)
			makeDatabase = _ttol(argv[1]);

		connectParams param(_T("tdap"), host, _T("querytest"), _T("test.bdf"));
		param.setMode(TD_OPEN_NORMAL);
		if (makeDatabase)
		{
	   	    if (db->open(param.uri(), TD_OPEN_NORMAL))
			    dropDatabase(db);
			createDatabase(db, param);
			openDatabase(db, param);
			if (!createUserTable(db->dbDef()))return 1;
		    if (!createGroupTable(db->dbDef()))return 1;
		    if (!createUserExtTable(db->dbDef()))return 1;

			insertData(db);
		}else
			openDatabase(db, param);

		queryTable atu(db, _T("user"));
	    queryTable atg(db, _T("groups"));
		queryTable ate(db, _T("extention"));

		recordset rs;

		bzs::rtl::benchmark bm;
		bm.report(boost::bind(btest, &rs, &atu, &atg, &ate, kind, n), "exec time ");
		//showConsole(rs);
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
	}

	return 1;
}





