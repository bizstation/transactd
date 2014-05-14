
#include <iostream>
#include <locale.h>
#include <vector>
#include <boost/iterator/iterator_facade.hpp>
#include "ormMapData.h"
#include "benchmark.h"

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;



static const char_td keynum_group = 1;
static const char_td primary_key = 0;


bool readUsers(databaseManager* mgr, dataset*  rows)
{
	const int key_date = 1;
	const unsigned short disableReject = 0xFFFF;
	const _TCHAR* startDate =   L"2012/09/01";
	const _TCHAR* endDate =   L"2014/02/28";
	const _TCHAR* prod =  L"���i����";
	const _TCHAR* amount =  L"���z";
	const _TCHAR* wkid =  L"extID";
	const _TCHAR* date =  L"���t";
	const _TCHAR* factryEmp =   L"factryEmpID";
	const _TCHAR* empId =   L"id";
	const _TCHAR* empName =   L"���O";
	activeTable<map_orm> atwm(*mgr, _T("��Ɩ���"));
	activeTable<map_orm> atwh(*mgr, _T("��ƃw�b�_�["));
	activeTable<map_orm> atemp(*mgr, _T("�Ј�"));

	dataset& rowset = *rows;
	DWORD t = timeGetTime();
	for (int j=0;j<1000;++j)
	{
	dataset rowset;
	rowset.reserve(4000);

	//startDate����endDate�܂ł�wkid���L���ȍ�Ɩ��דǂݎ��
	tdc:query q;

	q.select(prod, amount, wkid)
				.where(date, L"<=", endDate).and(wkid, L">", 0).reject(disableReject);
	atwm.index(key_date).keyValue(startDate).read(rowset, q);

	}
	/*
	//t = timeGetTime() - t;
	//std::tcout  << _T("work record reading time = ")  << t <<   _T("\n");
	//t = timeGetTime();

	//wkid�����ƃw�b�_�[����ݎЈ��ԍ���ǂݎ��


   	atwh.index(0).join(rowset, q.reset().select(factryEmp), wkid);
	//t = timeGetTime() - t;
	//std::tcout  << _T("join header time = ")  << t <<   _T("\n");
	//t = timeGetTime();

	//���ʃZ�b�g��prod �� factryEmp�ŃO���[�s���O
	tdc::groupQuery gq;
	gq.keyField(prod, factryEmp).resultField(amount);
	atwh.groupBy(rowset, gq, sum<>());
	//t = timeGetTime() - t;
	//std::tcout  << _T("groupBy header time = ")  << t <<   _T("\n");
	//t = timeGetTime();

	//factryEmp���Ј��e�[�u������� empId��empName��Join����
	atemp.index(0).join(rowset, q.reset().select(empId, empName), factryEmp);
	//t = timeGetTime() - t;
	//std::tcout  << _T("join empr time = ")  << t <<   _T("\n");


	std::tcout  << _T("data count = ")  << rowset.size() <<   _T("\n");
	*/
	return true;

}

bool addUser(databaseManager* mgr)
{
	const _TCHAR* empName =   L"���O";
	const _TCHAR* id =   L"id";
	activeTable<map_orm> atemp(*mgr, _T("�Ј�"));

	row_n_ptr user(new row_n());
	row_n& u =  *user;

	//�����ȃt�B�[���h���������Ă���������܂��B
	u[id] = 2000;
	u[empName] = std::_tstring(_T("Takahashi team"));
	atemp.index(0).save(*user);


	u[id] = 2000;
	u[empName] = std::_tstring(_T("Takahashi team2"));
	atemp.index(0).update(*user);

	u[id] = 2000;
	atemp.index(0).del(*user);

}

void showConsole(dataset& rowset)
{
	for (int i=0;i<(int)rowset.size();++i)
	{
		row_ptr m = rowset[i];
	   #ifdef USE_MAP

		std::tcout << boost::any_cast<std::_tstring>((*m)[prod]) << _T("\t")
				   << boost::any_cast<__int64>((*m)[wkid])
					<< _T("\n");
	   #else
	   //OrderBy�͂���Ă��Ȃ�

	   std::tcout << boost::any_cast<std::_tstring>((*m)[0]) << _T("\t")
				   << boost::any_cast<__int64>((*m)[1]) << _T("\t")
				   << toString((*m)[2]) << _T("\t")
				   << toString((*m)[3])  << _T("\t")
				   << toString((*m)[4])   << _T("\t")
				   << toString((*m)[5])   << _T("\t")
					<< _T("\n");

	   #endif
	}
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale("japanese"));
	database_ptr db = createDatadaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("mri"), _T("dj32.bdf"));
		openDatabase(db, param);
		databaseManager mgr(db);

		//addUser(&mgr);
		dataset rowset;

		bzs::rtl::benchmark bm;
		bm.report(boost::bind(readUsers, &mgr, &rowset), "exec time ");
		showConsole(rowset);
		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
	}

	return 1;
}





