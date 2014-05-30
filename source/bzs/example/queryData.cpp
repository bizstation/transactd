﻿/*=================================================================
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

#include "queryData.h"
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <iostream>

#pragma package(smart_init)

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

#ifndef USE_PSQL_DATABASE

#define USER_STRING_TYPE ft_myvarchar
#define GROUP_STRING_TYPE ft_myvarbinary

#else

#define USER_STRING_TYPE ft_zstring
#define GROUP_STRING_TYPE ft_zstring

#endif

bool showDbdefError(dbdef* def, const _TCHAR* msg)
{
	 std::tcout << msg << _T(" erorr:No.") << def->stat();
	 return false;
}

bool showTableError(table* tb, const _TCHAR* msg)
{
	 std::tcout << msg << _T(" erorr:No.") << tb->stat();
	 return false;
}

bool showDbError(database* db, const _TCHAR* msg)
{
	 std::tcout << msg << _T(" erorr:No.") << db->stat();
	 return false;
}

bool createUserTable(dbdef* def)
{
	short tableid = 1;
	tabledef t;
	tabledef* td = &t;
	td->charsetIndex = mysql::charsetIndex(GetACP());
	td->schemaCodePage = CP_UTF8;
	td->id = tableid;
	td->setTableName(_T("user"));
	td->setFileName(_T("user.dat"));

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
#ifdef LINUX
	const char* fd_name = "名前";
#else
	#ifdef _UNICODE
		const wchar_t* fd_name = L"名前";
	#else
		char fd_name[30];
		WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
	#endif
#endif

	fd->setName(fd_name);
	fd->type = USER_STRING_TYPE;
	fd->setLenByCharnum(20);

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("group"));
	fd->type = ft_integer;
	fd->len = 4;

	++filedIndex;
	fd =  def->insertField(tableid, filedIndex);
	fd->setName(_T("tel"));
	fd->type = USER_STRING_TYPE;
	fd->setLenByCharnum(21);

	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment* seg1 = &kd->segments[0];
	seg1->fieldNum = 0;
	seg1->flags.bit8 = true;//extended key type
	seg1->flags.bit1 = true;//chanageable
	kd->segmentCount = 1;
	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;

	++keyNum;
	kd = def->insertKey(tableid, keyNum);
	seg1 = &kd->segments[0];
	seg1->fieldNum = 2;
	seg1->flags.bit0 = true;
	seg1->flags.bit8 = true;
	seg1->flags.bit1 = true;
	kd->segmentCount = 1;

	def->updateTableDef(tableid);
	if (def->stat()!=0)
		return showDbdefError(def, _T("user updateTableDef"));

	return true;
}

bool createGroupTable(dbdef* def)
{
	short tableid = 2;
	tabledef t;
	tabledef* td = &t;
	td->charsetIndex = mysql::charsetIndex(GetACP());
	td->schemaCodePage = CP_UTF8;
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
	fd->type = GROUP_STRING_TYPE;
	fd->len = 33;

	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment* seg1 = &kd->segments[0];
	seg1->fieldNum = 0;
	seg1->flags.bit8 = true;//extended key type
	seg1->flags.bit1 = true;//chanageable
	kd->segmentCount = 1;

	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;
	def->updateTableDef(tableid);
	if (def->stat()!=0)
		return showDbdefError(def, _T("groups updateTableDef"));
	return true;
}

bool createUserExtTable(dbdef* def)
{
	short tableid = 3;
	tabledef t;
	tabledef* td = &t;
	td->charsetIndex = mysql::charsetIndex(GetACP());
	td->schemaCodePage = CP_UTF8;
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
	fd->type = USER_STRING_TYPE;

	fd->setLenByCharnum(60);


	char keyNum = 0;
	keydef* kd = def->insertKey(tableid, keyNum);
	keySegment* seg1 = &kd->segments[0];
	seg1->fieldNum = 0;
	seg1->flags.bit8 = true;//extended key type
	seg1->flags.bit1 = true;//chanageable
	kd->segmentCount = 1;
	td = def->tableDefs(tableid);
	td->primaryKeyNum = keyNum;
	def->updateTableDef(tableid);
	if (def->stat()!=0)
	    return showDbdefError(def, _T("extention updateTableDef"));
	return true;
}

bool insertData(database_ptr db, int maxId)
{
	_TCHAR tmp[256];
	dbTransaction trn(db);
	trn.begin();

	table* tb = db->openTable(_T("user"), TD_OPEN_NORMAL);
	if (db->stat())
	{
		showDbError(db.get(), _T("openTable user"));
		return false;
	}
	tb->clearBuffer();
	for (int i= 1;i<= maxId;++i)
	{
		tb->setFV((short)0, i);
		_stprintf_s(tmp, 256, _T("%d user"), i);
		tb->setFV(1, tmp);
		tb->setFV(_T("group"), ((i -1) % 100) + 1);
		tb->insert();
		if (tb->stat()!=0)
			return showTableError(tb, _T("user insert"));
	}
	tb->release();

	tb = db->openTable(_T("groups"), TD_OPEN_NORMAL);
	if (db->stat())
	{
		showDbError(db.get(), _T("openTable groups"));
		return false;
	}
	tb->clearBuffer();
	for (int i= 1;i<= 100;++i)
	{
		tb->setFV((short)0, i);
		_stprintf_s(tmp, 256, _T("%d group"), i);
		tb->setFV(1, tmp);
		tb->insert();
		if (tb->stat()!=0)
			return showTableError(tb, _T("groups insert"));
	}
	tb->release();
	tb = db->openTable(_T("extention"), TD_OPEN_NORMAL);
	if (db->stat())
	{
		showDbError(db.get(), _T("openTable extention"));
		return false;
	}
	tb->clearBuffer();
	for (int i= 1;i<= maxId;++i)
	{
		tb->setFV((short)0, i);
		_stprintf_s(tmp, 256, _T("%d comment"), i);
		tb->setFV(1, tmp);
		tb->insert();
		if (tb->stat()!=0)
			return showTableError(tb, _T("extention insert"));
	}
	tb->release();
	trn.end();
	return true;
}

int prebuiltData(database_ptr db, const connectParams& param, bool foceCreate, int maxId)
{
	try
	{
		if (db->open(param.uri(), TD_OPEN_NORMAL))
		{
			if(foceCreate)
				dropDatabase(db);
			else
				return 0;
		}
		std::tcout <<  _T("\nInserting query test data. Please wait ... ") << std::flush;
		createDatabase(db, param);
		openDatabase(db, param);
		if (!createUserTable(db->dbDef()))return 1;
		if (!createGroupTable(db->dbDef()))return 1;
		if (!createUserExtTable(db->dbDef()))return 1;
		if (!insertData(db, maxId)) return 1;
		std::tcout <<  _T("done!") << std::endl;
		return 0;
	}
	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
		return 1;
	}

}