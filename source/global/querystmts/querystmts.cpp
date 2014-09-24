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

#include <bzs/db/protocol/tdap/client/serializer.h>
#include <bzs/db/protocol/tdap/client/databaseManager.h>
#include <bzs/rtl/benchmark.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <iostream>
#include <boost/program_options.hpp>

#ifdef __BCPLUSPLUS__
#	define BZS_LINK_BOOST_PROGRAM_OPTIONS
#	include <bzs/env/boost_bcb_link.h>
#endif

using namespace bzs::db::protocol::tdap::client;
using namespace boost::program_options;

//---------------------------------------------------------------------------
struct cmd_parmas
{
	std::string query;
	std::string ivalues;
	bool print;
	cmd_parmas():print(false){}
};


enum eEncoding{eEAuto=0, eEAnsi=1, eEUnicode=2, eEUtf8=3};

void analyzeBom(unsigned char* p, eEncoding& type)
{
	if ((p[0] == 0xFF) &&(p[1] == 0xFE))
		type = eEUnicode;
	else if ((p[0] == 0xEF) &&(p[1] == 0xBB)&&(p[2] == 0xBF))
		type = eEUtf8;
	else
		type = eEAnsi;
}

bool readInputValues(const char* filename, std::vector<std::_tstring>& inputValues)
{
	//Linux            : A input file encording is utf8.
	//Windows multibyte: A input file encording is utf8.
	//Windows unicode  : A input file encording is locale charset.
	FILE* fp = fopen(filename, "rt");
	if(!fp)
	{
		_ftprintf(stderr, _T("Error ! Can not open the input file (%s)\n"), filename);
		return false;
	}
	char tmp[5];
	eEncoding encodeing = eEAnsi;
	fseek(fp, 0L, SEEK_SET);

	if (fgets(tmp, 4, fp))
	{
		analyzeBom((unsigned char*)tmp, encodeing);
		if (encodeing == eEAnsi)
			fseek(fp, 0L, SEEK_SET);
		else if (encodeing == eEUnicode)
			fseek(fp, 2L, SEEK_SET);
		else if (encodeing == eEUtf8)
			fseek(fp, 3L, SEEK_SET);

	}
	if ((encodeing == eEAnsi)|| (encodeing == eEUtf8))
	{
		char buf[512];
		while (fgets(buf, 512, fp))
		{
			buf[strlen(buf)-1]=0x00;
			#ifdef _UNICODE
			wchar_t wbuf[1024];
				UINT codePage = (encodeing==eEUtf8) ? CP_UTF8:CP_ACP;
				MultiByteToWideChar(codePage, (codePage==CP_UTF8)?0:MB_PRECOMPOSED, buf, -1, wbuf, 1024);
				inputValues.push_back(wbuf);
			#else
				inputValues.push_back(buf);
			#endif //_UNICODE
		}
	}else
	{
	#ifndef _WIN32
		_ftprintf(stderr, _T("Error ! Can not read the unicode encording input file(%s).\n"), filename);
	#else
		wchar_t wbuf[512];
		while (fgetws(wbuf, 512, fp))
		{
			wbuf[wcslen(wbuf)-1]=0x00;
			#ifdef _UNICODE
				inputValues.push_back(wbuf);
			#else
				char buf[1024];
				WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, 1024, NULL, NULL);
				inputValues.push_back(buf);
			#endif  //_UNICODE
		}

	#endif
	}
	fclose(fp);
	return true;
}

void printResult(recordset& rs)
{
	const fielddefs& fields = *rs.fieldDefs();
	for (int j=0;j<(int)fields.size();++j)
		_tprintf(_T("%s\t"), fields[j].name());
	_tprintf(_T("\n"));

	for (int i=0;i<(int)rs.size();++i)
	{
		row& m = rs[i];
		for (int j=0;j<(int)m.size();++j)
		{
			_tprintf(_T("%s\t"), m[(short)j].c_str());
			if (j == (int)m.size() -1)
				_tprintf(_T("\n"));
		}
	}
	
}

void execute(queryStatements* qs, recordset* rs, std::vector<std::_tstring>* values)
{
	qs->execute(*rs, values);
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	#ifdef _WIN32
	//set locale to current user locale.
	std::locale::global(std::locale(""));
	#endif

	//setup command line options
	cmd_parmas  pm;
	options_description opt("command line option");
	opt.add_options()
		("query,q",value<std::string>(&pm.query),"prepared query file path")
		("values,v", value<std::string>(&pm.ivalues),"prepared input values file path")
		("print,p", value<bool>(&pm.print),"print out result");
	variables_map values;
	try
	{
		store(parse_command_line(argc, argv, opt), values);
		notify(values);
		if (!values.count("query")
				/*|| !values.count("values")*/)
		{
			std::cout << opt << std::endl;
			return 1;
		}

		disbDbManager dbm;
		//boost::shared_ptr<queryStatements> qs(queryStatements::create(dbm),
		//	boost::bind(&queryStatements::release, qs.get()));
		queryStatements qs(dbm);
		recordset rs;
		std::vector<std::_tstring> inputValues;

		if (values.count("values") && !readInputValues(pm.ivalues.c_str(), inputValues))
			 return 1;

		const _TCHAR* path;
		#ifdef _UNICODE
			wchar_t buf[2048];
			MultiByteToWideChar(CP_UTF8, 0, pm.query.c_str(), -1, buf, 2048);
			path = buf;
		#else
			path = pm.query.c_str();
		#endif
		qs.load(path);
		//bzs::rtl::benchmark::report2(boost::bind(execute, &qs, &rs, &inputValues), "execute");
		bzs::rtl::benchmark::start();
		execute(&qs, &rs, &inputValues);
		int t = bzs::rtl::benchmark::stop();
		if (pm.print)
			printResult(rs);
		char tmp[120];
		sprintf_s(tmp, 120, "OK, %lu record(s) :", rs.size());
		bzs::rtl::benchmark::showTimes(t, tmp);

		fflush(stdout);

		return 0;
	}

	catch(bzs::rtl::exception &e)
	{
		_ftprintf(stderr, _T("Error ! %s\n"), getMsg(e)->c_str());
	}

	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return 1;
}
//---------------------------------------------------------------------------
