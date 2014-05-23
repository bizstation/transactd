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
#include <bzs/db/protocol/tdap/client/memRecordset.h>
#include <bzs/example/queryData.h>
#include <iostream>
#include <locale.h>

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
			std::tcout << m[(short)j].c_str()  << _T("\t");
			if (j == (int)m.size() -1)
			   std::tcout << _T("\n");
		}
	}
}

void execute(recordset& rs, queryTable& atu, queryTable& atg, queryTable& ate)
{
	tdc::query query;

	rs.clear();
	atu.alias(_T("名前"), _T("name"));

	query.select(_T("id"), _T("name"),_T("group")).where(_T("id"), _T("<="), 15);
	atu.index(0).keyValue(1).read(rs, query);
	
	/* Join extention::comment
	   Use "joinKeyValuesUnique" optimaize option. 
	   Because this join is has one and atu.index(0) is unique key,
	   then join key values(id) are all unique.
	*/ 
	query.reset().select(_T("comment")).optimize(queryBase::joinKeyValuesUnique);
	ate.index(0).join(rs, query, _T("id"));

	//Join group::name
	atg.alias(_T("name"), _T("group_name"));
	query.reset().select(_T("group_name"));
	atg.index(0).join(rs, query, _T("group"));

}

#pragma warning(disable:4101)
#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatadaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("querytest"), _T("test.bdf"));
		param.setMode(TD_OPEN_NORMAL);
		if (prebuiltData(db, param, false, 20))
		{
			std::tcout << "The query data build error." << std::endl;
			return 1;
		}
		queryTable atu(db, _T("user"));
		queryTable atg(db, _T("groups"));
		queryTable ate(db, _T("extention"));

		recordset rs;
		execute(rs, atu, atg, ate);
		showConsole(rs);
		std::cout << "Execute query success. rs.size = " << rs.size() << std::endl;
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
#pragma warning(default:4101)





