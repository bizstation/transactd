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
var TYPE_DDF = 1;
var TYPE_BDF = 0;
var READ_ONLY = -2;
var OPEN_NORMAL = 0;
var AUTO_CREATE_TABLE = true;
var OWNER_NAME = "";
var DIR="";

var fn_id = 0;
var fn_user = 1;
var S_NOWAIT_LOCK = 200;
var M_NOWAIT_LOCK = 400;
var CCURR_T_BIAS = 1000;
var trans_bias = S_NOWAIT_LOCK + CCURR_T_BIAS;


// field type
var ft_string       = 0;
var ft_integer      = 1;
var ft_float        = 2;
var ft_date         = 3;
var ft_time         = 4;
var ft_decimal      = 5;
var ft_money        = 6;
var ft_logical      = 7;
var ft_numeric      = 8;
var ft_bfloat       = 9;
var ft_lstring      = 10;
var ft_zstring      = 11;
var ft_note         = 12;
var ft_lvar         = 13;
var ft_uinteger     = 14;
var ft_autoinc      = 15;
var ft_bit          = 16;
var ft_numericsts   = 17;
var ft_numericsa    = 18;
var ft_currency     = 19;
var ft_timestamp    = 20;
var ft_blob         = 21;
var ft_reserve22    = 22;
var ft_reserve23    = 23;
var ft_reserve24    = 24;
var ft_wstring      = 25;
var ft_wzstring     = 26;
var ft_guid         = 27;
var ft_datatime     = 30;
var ft_varchar      = 40;
var ft_varbinary    = 41;
var ft_wvarchar     = 42;
var ft_wvarbinary   = 43;
var ft_char         = 44;
var ft_wchar        = 45;
var ft_mydate       = 46;
var ft_mytime       = 47;
var ft_mydatetime   = 48;
var ft_mytimestamp  = 49;
var ft_mytext       = 50;
var ft_myblob       = 51;
		
//key flag
var key_duplicate   = 0;
var key_changeable  = 1;
var key_allnullkey  = 3;
var key_desc        = 6;
var key_extend      = 8;
var key_anynullkey  = 9;
var key_incase      = 10;

var changeCurrentNcc = 1;

var USE_NONE = 0;
var USE_TRAN = 1;
var USE_BULKINSERT = 2;
var USE_SNAPSHOT = 4;


/*--------------------------------------------------------------------------------*/
  
WScript.quit(main());

/*--------------------------------------------------------------------------------*/
function showTableError(tb, TableName)
{
	if (tb.stat != 0)
		WScript.Echo(TableName + " error No." + tb.stat);
}
/*--------------------------------------------------------------------------------*/
function openTable(db, tableName)
{
	var ret = db.OpenTable(tableName, OPEN_NORMAL, AUTO_CREATE_TABLE,OWNER_NAME,DIR);
	if (ret == null)
		WScript.Echo(tableName + " open erorr:No" + db.stat);
	return ret;
}
/*--------------------------------------------------------------------------------*/
function write(tb, start, end)
{
	tb.KeyNum = 0;
	for (var i=start;i< end;i++)
	{
		tb.ClearBuffer();
		tb.Vlng(fn_id) = i;
		tb.Text("name") = i;
		tb.Insert(changeCurrentNcc);
		if (tb.Stat != 0)
		{
			WScript.Echo(tb.Stat);
			return false;
		}
	}
	return true;
}
/*--------------------------------------------------------------------------------*/
function deleteAll(db,tb, start, end)
{
	db.BeginTrn(trans_bias);
	for (var i=start;i<end;i++)
	{
		tb.Vlng(fn_id) = i;
		tb.Seek();
		if (tb.Stat==0)
		{
			tb.Delete();
			if (tb.Stat!=0)
			{
				showTableError(tb, "deleteAll");
				db.EndTrn();
				return false;
			}   
		}
	}
	db.EndTrn();
	return true;
}
/*--------------------------------------------------------------------------------*/
function Inserts(db, tb,  start, end, name, mode, unit)
{
	var ret = deleteAll(db,tb, start, end);
	
	if (ret == true)
	{
		var now = new Date();
		var ticks = now.getTime();
		var total = end - start;
		var count = total/unit;
		var st = start;
		var en = st;
		while (en != end)
		{
			en = st + unit;
			if (mode == USE_TRAN)
				db.BeginTrn(trans_bias);
			else if (mode == USE_BULKINSERT)
				tb.BeginBulkInsert(); 
			ret = write(tb, st, en);
			if (mode == USE_BULKINSERT)
				tb.CommitBulkInsert();
			else if (mode == USE_TRAN)
				db.EndTrn();
			if (ret==false) break;
			st = en;
		}
		if (ret == true)
		{
			now = new Date();
			WScript.Echo((now.getTime() - ticks) + " msec" + name);
			return;
		}
	}
	WScript.Echo("Erorr " + name);
	
}
/*--------------------------------------------------------------------------------*/
function Reads(db, tb, start,  end, name, shapshot)
{
	var ret = true;
	var now = new Date();
	var ticks = now.getTime();
	if (shapshot==USE_SNAPSHOT) db.BeginSnapShot();
	for (var i=start;i<end;i++)
	{
		tb.Vlng(fn_id) = i;
		tb.Seek();
		if ((tb.Stat!=0) || (tb.Vlng(0) !=i))
		{
			WScript.Echo("Seek Error Stat = " + tb.Stat + " Value " + i + " = " + tb.Vlng(0));
			ret = false;
			break;
		}   
	}
	if (shapshot==USE_SNAPSHOT) db.EndSnapShot();
	if (ret == true)
	{
		now = new Date();
		WScript.Echo((now.getTime() - ticks) + " msec" + name);
	}else
		WScript.Echo("Erorr " + name);
}
/*--------------------------------------------------------------------------------*/
function ReadRange(db, tb, start, end, name, unit, shapshot)
{
	var qb = new ActiveXObject('transactd.query');
	var ret = true;
	var now = new Date();
	var ticks = now.getTime();
	tb.KeyNum = 0;
	var total = end - start;
	var count = total/unit;
	var st = start;
	if (shapshot == USE_SNAPSHOT) db.BeginSnapShot();
	var en = st;    
	while (en != end)
	{
		en = st + unit;
		tb.ClearBuffer();
		qb.Where("id", ">=", st).And("id", "<", en);
  		tb.SetQuery(qb);
		tb.Vlng(fn_id) = st;
		tb.SeekGreater(true/*orEqual*/);
		for(var i=st;i<en;i++)
		{
			if (tb.Vlng(fn_id) != i)
			{
				WScript.Echo("FindNext Error Stat = " + tb.Stat + " Value " + i + " = " + tb.Vlng(0));  
				ret = false;
				break;
			}
			tb.FindNext();  
		}
		if (ret==false) break;
		st = en;
		
	}
	if (shapshot == USE_SNAPSHOT) db.EndSnapShot();
	if (ret == true)
	{
		now = new Date();
		WScript.Echo((now.getTime() - ticks) + " msec" + name);
	}else
		WScript.Echo("Erorr " + name);
}
/*--------------------------------------------------------------------------------*/
function Updates(db, tb, start,  end, name,  tran, unit)
{
	var ret = true;
	var now = new Date();
	var ticks = now.getTime();
	tb.KeyNum = 0;
	
	var total = end - start;
	var count = total/unit;
	var st = start;
	var en = st;
	while (en != end)
	{
		en = st + unit;
	
		if (tran==USE_TRAN) db.BeginTrn(trans_bias);
		for (var i=st;i<en;i++)
		{
			tb.Vlng(fn_id) = i;
			tb.Seek();
			if ((tb.Stat!=0) || (tb.Vlng(0) !=i))
			{
				WScript.Echo("Seek Error Stat = " + tb.Stat + " Value " + i + " = " + tb.Vlng(0));
				ret = false;    
				break;
			}
			tb.Text("name") = (i+1+tran);
			tb.UpDate(changeCurrentNcc);
			if (tb.Stat!=0)
			{
				showTableError(tb, name);
				ret = false;        
				break;
			}
		}
		if (tran==USE_TRAN) db.EndTrn();
		if (ret==false) break;
		st = en;
	}
	
	if (ret == true)
	{
		now = new Date();
		WScript.Echo((now.getTime() - ticks) + " msec" + name);
	}else
		WScript.Echo("Erorr " + name);
}
/*--------------------------------------------------------------------------------*/
function createTestDataBase(db, uri)
{
	db.Create(uri);
	if (db.Stat!=0)
	{
		WScript.Echo("createTestDataBase erorr:No." + db.Stat + " " + uri);
		return false;
	}
	if (db.Open(uri, TYPE_BDF, OPEN_NORMAL, "", ""))
	{
		var dbdef = db.DbDef;
		var tableid = 1;
		
		var tableDef =  dbdef.InsertTable(tableid);
		tableDef.TableName = "user";
		tableDef.FileName = "user.dat";
		
		var filedIndex = 0;
		var fd =  dbdef.InsertField(tableid, filedIndex);
		fd.Name = "id";
		fd.Type = ft_integer;
		fd.Len = 4;
	
		filedIndex = 1;
		fd =  dbdef.InsertField(tableid, filedIndex);
		fd.Name = "name";
		fd.Type = ft_zstring;
		fd.Len = 33;
	
		var keyNum = 0;
		var key = dbdef.InsertKey(tableid, keyNum);
		var seg1 = key.Segments(0);
		seg1.FieldNum = 0;
		seg1.Flags.Bits(key_extend) = true;    //extended key type
		seg1.Flags.Bits(key_changeable) = true;//chanageable
		key.SegmentCount = 1;
		
		tableDef.PrimaryKeyNum = keyNum;
		dbdef.UpDateTableDef(tableid);
		dbdef = null;
		return true;

	}
	WScript.Echo("open daatabse erorr:No" +  db.stat);
	return false;
}
/* -------------------------------------------------------------------------------- */
function showUsage()
{

   var s = "usage: transactdBench databaseUri processNumber functionNumber\n "
		+ "\t --- Below is list of functionNumber  ---\n"
		+ "\t-1: all function\n"
		+ "\t 0: Insert\n"
		+ "\t 1: Insert in transaction. 20rec x 1000times\n"
		+ "\t 2: Insert by bulkmode. 20rec x 1000times\n"
		+ "\t 3: read each record\n"
		+ "\t 4: read each record with snapshpot\n"
		+ "\t 5: read range. 20rec x 1000times\n"
		+ "\t 6: read range with snapshpot. 20rec x 1000times\n"
		+ "\t 7: update\n"
		+ "\t 8: update in transaction. 20rec x 1000times\n"
		+ "exsample : transactdBench \"tdap://localhost/test?dbfile=test.bdf\" 0 -1\n";
	 WScript.Echo(s);
}
/*--------------------------------------------------------------------------------*/
function main()
{
	
	if (WScript.Arguments.length < 3)
	{
		showUsage();
		return 1;   
	}
	
	var URI  = WScript.Arguments(0);//"tdap://localhost/test?dbfile=test.bdf";
	var db = new ActiveXObject('transactd.database');
	var procID = parseInt(WScript.Arguments(1), 10);
	var execType = parseInt(WScript.Arguments(2), 10);
	
	var count = 20000;
	var start = procID * count + 1; 
	var end = start + count;
	
	if (db.Open(URI, TYPE_BDF, OPEN_NORMAL, "", ""))
	{
		if (execType < 3)
			db.Drop();
	}
	if (db.Stat == 3106)
	{
		WScript.Echo("Error! Maybe MySQL or Tranasactd is stopping! "); 
		return 1;
	}
	if (execType < 3)
	{
		if (!createTestDataBase(db, URI))
			return 1;
	}
	var now = new Date();
	WScript.Echo("Start Bench mark Insert Items = " +  count);
	WScript.Echo(now);
	WScript.Echo(URI);
	WScript.Echo("----------------------------------------");
	
	if (db.Open(URI, TYPE_BDF, OPEN_NORMAL, "", ""))
	{
		var tb = openTable(db, "user");
		if (tb != null)
		{
			if ((execType == -1) || (execType == 0))
				Inserts(db, tb, start,  end, ": Insert", USE_NONE, 1);
			if ((execType == -1) || (execType == 1))
				Inserts(db, tb, start, end, ": Insert in transaction. 20rec ~ 1000times.", USE_TRAN, 20);
			if ((execType == -1) || (execType == 2))
				Inserts(db, tb, start, end, ": Insert by bulkmode. 20rec ~ 1000times.", USE_BULKINSERT, 20)
			if ((execType == -1) || (execType == 3))
				Reads(db, tb,  start, end, ": read each record.", USE_NONE);
			if ((execType == -1) || (execType == 4))
				Reads(db, tb,  start,end, ": read each record in snapshot.", USE_SNAPSHOT)
			if ((execType == -1) || (execType == 5))
				ReadRange(db, tb, start,  end, ": read range. 20rec ~ 1000times.", 20, USE_NONE)
			if ((execType == -1) || (execType == 6))
				ReadRange(db, tb, start,  end, ": read range with snapshpot. 20rec x 1000times.", 20, USE_SNAPSHOT)
			if ((execType == -1) || (execType == 7))
				Updates(db, tb, start, end, ": update.", USE_NONE, 1);
			if ((execType == -1) || (execType == 8))
				Updates(db, tb, start, end, ": update in transaction. 20rec ~ 1000times.", USE_TRAN, 20);
			tb.Close();
		}
		
	}
	if (db.stat!=0)
	{  
		WScript.Echo("open table erorr:No" +  db.stat);
		return 1;
	}

	db.Close();
	WScript.Echo("----------------------------------------");
	return 0;
}
