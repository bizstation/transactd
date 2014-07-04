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
#include <iostream>
#include <locale.h>
#include <boost/iterator/iterator_facade.hpp>
#include "memRecordset.h"
#include "benchmark.h"

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;



static const char_td keynum_group = 1;
static const char_td primary_key = 0;


bool readUsers(databaseManager* mgr, recordset*  rows)
{
	const int key_date = 1;
	const unsigned short disableReject = 0xFFFF;
	const _TCHAR* startDate =   L"2012/09/01";
	const _TCHAR* endDate =   L"2014/02/28";
	const _TCHAR* prod =  L"商品ｺｰﾄﾞ";
	const _TCHAR* amount =  L"金額";
	const _TCHAR* wkid =  L"extID";
	const _TCHAR* date =  L"日付";
	const _TCHAR* factryEmp =   L"factryEmpID";
	const _TCHAR* factryEmpid =   _T("社員ID");
	const _TCHAR* empId =   L"id";
	const _TCHAR* empName =   L"名前";
	activeTable<map_orm> atwm(*mgr, _T("作業明細"));
	activeTable<map_orm> atwh(*mgr, _T("作業ヘッダー"));
	activeTable<map_orm> atemp(*mgr, _T("社員"));

	recordset& rowset = *rows;
	tdc:query q;
	DWORD t = timeGetTime();
	for (int j=0;j<1;++j)
	{
	rowset.clear();
	//rowset.reserve(4000);

	//startDateからendDateまででwkidが有効な作業明細読み取り


	q.reset().select(prod, amount, wkid)
				.where(date, L"<=", endDate).and_(wkid, L">", 0).reject(disableReject);
	atwm.index(key_date).keyValue(startDate).read(rowset, q);

	//t = timeGetTime() - t;
	//std::tcout  << _T("work record reading time = ")  << t <<   _T("\n");

	//wkidから作業ヘッダーをよみ社員番号を読み取る
   	atwh.index(0).alias(factryEmp, factryEmpid).join(rowset, q.reset().select(factryEmpid), wkid);

	//結果セットをprod と factryEmpでグルーピング
	tdc::groupQuery gq;
	gq.keyField(prod, factryEmpid).resultField(amount);
	rowset.groupBy(gq, sum<>());

	//rowset.groupBy(gq, avg<>());
	//factryEmpより社員テーブルをよみempNameをJoinする
	atemp.alias(empName, _T("社員名"));
	atemp.index(0).join(rowset, q.reset().select(_T("社員名")/*empName*/), factryEmpid);
	rowset.orderBy(amount, factryEmpid);

	}
	std::tcout  << _T("data count = ")  << rowset.size() <<   _T("\n");

	return true;

}
void wirteRecord(databaseManager* mgr)
{
	activeTable<map_orm> atemp(*mgr, _T("社員"));

	tdc::writableRecord& rec = atemp.index(0).getWritableRecord();

	rec[_T("id")] = 1200;
	rec[_T("名前")] = _T("矢口");
	if (!rec.read())
		rec.insert();

	rec.clear();
	rec[_T("id")] = 1201;
	rec[_T("名前")] = _T("小坂");
	if (!rec.read())
		rec.insert();

	//update changed filed only
	rec.clear();
	rec[_T("id")] = 1201;
	rec[_T("parent")] = 1240;
	rec.update();

	rec.del();
	rec[_T("id")] = 1200;
	rec.del();

}

void delUser(databaseManager* mgr)
{
	activeTable<map_orm> atemp(*mgr, _T("社員"));

	//updateでは変更したフィールドだけにしたい
	//変更フィールドがわかるfieldsが必要
	//変更フィールドだけを
	tdc::memoryRecord& rec = atemp.getWritableRecord();
	rec[_T("id")] = 1200;
	atemp.del(rec);
}

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

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale("japanese"));
	database_ptr db = createDatabaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("mri"), _T("dj32.bdf"));
		openDatabase(db, param);
		databaseManager mgr(db);

		recordset rowset;

		bzs::rtl::benchmark bm;
		bm.report(boost::bind(readUsers, &mgr, &rowset), "exec time ");
		showConsole(rowset);
		wirteRecord(&mgr);
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
	}

	return 1;
}





