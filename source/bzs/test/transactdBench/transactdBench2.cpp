/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

#include <tchar.h>
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <time.h>
#include <bzs/rtl/benchmark.h>
#include <boost/bind.hpp>
#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS


static const char keynum_id = 0;
static const short fn_id = 0;
static const short fn_name = 1;

static const int USE_NORMAL = 0;
static const int USE_TRANS = 1;
static const int USE_BALKINS = 2;
static const int USE_SNAPSHOT = 4;

using namespace bzs::rtl;
using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;


/* -------------------------------------------------------------------------------- */
void write(fields& fds, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		fds.clear();
		fds[fn_id] = i;
		fds[fn_name] = i;
		insertRecord(fds);
	}
}

/* -------------------------------------------------------------------------------- */
bool deleteAll(database_ptr db, table_ptr tb, int start, int end)
{
	dbTransaction trn(db);
	trn.begin();

	for (int i = start; i < end; i++)
	{
		indexIterator it = readIndex_v(tb, eSeekEqual, keynum_id, i);
		if (!it.isEnd())
			deleteRecord(it);
	}
	trn.end();
	return true;
}

/* -------------------------------------------------------------------------------- */
void inserts(database_ptr db, table_ptr tb, int start, int end, int mode, int unit)
{
	int total = end - start;
	int count = total / unit;
	int st = start;
	int en = st;
	fields fds(tb);
	while (en != end)
	{
		en = st + unit;
		if (mode == USE_TRANS)
		{
			dbTransaction trn(db);
			trn.begin();
			write(fds, st, en);
			trn.end();
		}else if (mode == USE_BALKINS)
		{
			autoBulkinsert bi(tb);
			write(fds, st, en);
		}else
			write(fds, st, en);
		st = en;
	}
}

/* -------------------------------------------------------------------------------- */
template <class T>
void checkFldIdValue(T it, int value)
{
	const fields& fds = *it;
	if (it.isEnd() || fds[fn_id] != value)
	{
		_TCHAR buf[256];
		_stprintf_s(buf, 256, _T("read error.Can not found value %d = %d\r\n"), value, fds[fn_id].i());
		THROW_BZS_ERROR_WITH_MSG(buf);
	}
}

/* -------------------------------------------------------------------------------- */
void doRead( database_ptr db, table_ptr tb, int start, int end, int shapshot)
{
	for (int i = start; i < end; i++)
	{
		indexIterator it = readIndex_v(tb, eSeekEqual, keynum_id, i);
		checkFldIdValue(it, i);
	}
}

/* -------------------------------------------------------------------------------- */
void read( database_ptr db, table_ptr tb, int start, int end, int shapshot)
{
	if (shapshot == USE_SNAPSHOT)
	{
		dbSnapshot sn(db);
		doRead(db, tb, start, end, shapshot);
	}else
		doRead(db, tb, start, end, shapshot);
}

/* -------------------------------------------------------------------------------- */
void doRreads(database_ptr db, table_ptr tb, int start, int end, int unit)
{
	int total = end - start;
	int count = total / unit;
	int st = start;
	int en = st;
	//filterParams fp( _T("*"), 1 , 20);
	query q;
	q.all().reject(2).limit(20);
	findIterator itsf = find(tb, keynum_id, q, st);
	while (en != end)
	{
		en = st + unit;
		for (int i = st; i < en; i++)
		{
			checkFldIdValue(itsf, i);
			++itsf;
		}
		st = en;
	}
}
/* -------------------------------------------------------------------------------- */
void reads(database_ptr db, table_ptr tb, int start, int end, int unit, int shapshot)
{
	if (shapshot == USE_SNAPSHOT)
	{
		dbSnapshot sn(db);
		doRreads(db, tb, start, end, unit);
	}else
		doRreads(db, tb, start, end, unit);
}
/* -------------------------------------------------------------------------------- */
void doUupdates(table_ptr tb, int st, int en, int tran)
{
	_TCHAR buf[256];
	for (int i = st; i < en; i++)
	{
		fields fd(tb);
		fd[fn_id] = i;
		_ltot_s(i + 1 + tran, buf, 30, 10);
		fd[fn_name] = buf;
		updateRecord(fd, keynum_id);
	}
}
/* -------------------------------------------------------------------------------- */
void updates(database_ptr db, table_ptr tb, int start, int end, int tran, int unit)
{
	int total = end - start;
	int count = total / unit;
	int st = start;
	int en = st;
	while (en != end)
	{
		en = st + unit;
		if (tran == USE_TRANS)
		{
			dbTransaction trn(db);
			trn.begin();
			doUupdates(tb, st, en, 1);
			trn.end();
		}else
			doUupdates(tb, st, en, 0);
		st = en;
	}
}

/* -------------------------------------------------------------------------------- */
void createUserTableSchema(dbdef* def)
{
	static const short tableid = 1;
	insertTable(def, tableid, _T("user"), CHARSET_LATIN1);

	short fieldNum = 0;
	insertField(def, tableid, fieldNum, _T("id"), ft_integer, 4);
	insertField(def, tableid, ++fieldNum, _T("name"), ft_myvarchar, 100);
	updateTableDef(def, tableid);

	short keyNum = 0;
	keydef* kd = insertKey(def, tableid, keyNum);
	kd->segments[0].fieldNum = 0;
	kd->segments[0].flags.bit8 = 1; // extend key type
	kd->segments[0].flags.bit1 = 1; // changeable
	kd->segmentCount = 1;
	updateTableDef(def, tableid);

}

/* -------------------------------------------------------------------------------- */
void createTestDataBase(database_ptr db, connectParams& params)
{
	params.setMode(TD_OPEN_EXCLUSIVE);
	createDatabase(db, params);
	openDatabase(db, params);
	createUserTableSchema(db->dbDef());
}

/* -------------------------------------------------------------------------------- */
void printHeader(const _TCHAR* uri, int count)
{
	printf("Start Bench mark Insert Items = %d\r\n", count);
	time_t timer;
#ifdef LINUX
	time(&timer);
#else
	timer = time(NULL);
#endif
#pragma warning( disable : 4996 )
	printf("%s", ctime(&timer));
#pragma warning( default : 4996 )

	_tprintf(_T("%s\r\n"), uri);
	printf("BOOST_VERSION = %s\r\n", BOOST_LIB_VERSION );
	printf("----------------------------------------\r\n");
}

/* -------------------------------------------------------------------------------- */
void printTail()
{
	printf("----------------------------------------\r\n");
}

/* -------------------------------------------------------------------------------- */
void showUsage()
{
	printf("usage: transactdBench databaseUri processNumber functionNumber\n "
		"\t --- Below is list of functionNumber  ---\n"
		"\t-1: all function\n"
		"\t 0: Insert\n"
		"\t 1: Insert in transaction. 20rec x 1000times\n"
		"\t 2: Insert by bulkmode. 20rec x 1000times\n"
		"\t 3: read each record\n"
		"\t 4: read each record with snapshpot\n"
		"\t 5: read range. 20rec x 1000times\n"
		"\t 6: read range with snapshpot. 20rec x 1000times\n"
		"\t 7: update\n"
		"\t 8: update in transaction. 20rec x 1000times\n"
		"exsample : transactdBench \"tdap://localhost/test?dbfile=test.bdf\" 0 -1\n");
}

/* -------------------------------------------------------------------------------- */
#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 4)
	{
		showUsage();
		return 0;
	}
	int procID = _ttol(argv[2]); // 0
	int count = 20000;
	int start = procID * count + 1;
	int end = start + count;
	int exeType =  _ttol(argv[3]);// -1
	bool insertBeforeNoDelete = 0;
	if (argc > 4)
		insertBeforeNoDelete = (_ttol(argv[4])!=0);

	try
	{
		database_ptr db = createDatabaseObject();
		connectParams param(argv[1]);

		if (!db->open(param.uri(), param.type(), param.mode()))
			createTestDataBase(db, param);
		printHeader(param.uri(), count);
		openDatabase(db, param);
		table_ptr tb = openTable(db, _T("user"));


		if ((exeType == -1) || (exeType == 0))
		{
			if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
				benchmark::report2(boost::bind(inserts, db, tb, start, end, USE_NORMAL, 1), ": Insert");
		}
		if ((exeType == -1) || (exeType == 1))
		{
			if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
				benchmark::report2(boost::bind(inserts, db, tb, start, end, USE_TRANS, 20)
						, ": Insert in transaction. 20rec x 1000times.");
		}
		if ((exeType == -1) || (exeType == 2))
		{
			if (insertBeforeNoDelete || deleteAll(db, tb, start, end))
				benchmark::report2(boost::bind(inserts, db, tb, start, end, USE_BALKINS, 20)
						, ": Insert by bulkmode. 20rec x 1000times.");
		}
		if ((exeType == -1) || (exeType == 3))
			benchmark::report2(boost::bind( read, db, tb, start, end, USE_NORMAL), ": read each record.");
		if ((exeType == -1) || (exeType == 4))
			benchmark::report2(boost::bind( read, db, tb, start, end, USE_SNAPSHOT), ": read each record with snapshpot.");
		if ((exeType == -1) || (exeType == 5))
			benchmark::report2(boost::bind( reads, db, tb, start, end, 20, USE_NORMAL), ": read range. 20rec x 1000times.");
		if ((exeType == -1) || (exeType == 6))
			benchmark::report2(boost::bind( reads, db, tb, start, end, 20, USE_SNAPSHOT), ": read range with snapshot. 20rec x 1000times.");
		if ((exeType == -1) || (exeType == 7))
			benchmark::report2(boost::bind( updates, db, tb, start, end, USE_NORMAL, 1), ": update.");
		if ((exeType == -1) || (exeType == 8))
			benchmark::report2(boost::bind( updates, db, tb, start, end, USE_TRANS, 20), ": update in transaction. 20rec x 1000times.");

		printTail();
		return 0;
	}
	catch(bzs::rtl::exception &e)
	{
		_tprintf(_T("Error ! %s\n"), getMsg(e)->c_str());

	}
	return 1;
}
