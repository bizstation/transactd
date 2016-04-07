/*=================================================================
	 Copyright (C) 2015-2016 BizStation Corp All rights reserved.

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

var MULTILOCK_NOGAP_SHARE           = 0;
var MULTILOCK_GAP_SHARE             = 2000;
var CONSISTENT_READ                 = 4000;
var CONSISTENT_READ_WITH_BINLOG_POS = 4200;

var REPL_POSTYPE_NONE       = 0;
var REPL_POSTYPE_MARIA_GTID = 1;
var REPL_POSTYPE_POS        = 2;


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
var ft_myvarchar      = 40;
var ft_myvarbinary    = 41;
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
var ft_autoIncUnsigned = 52;
var ft_myfixedbinary = 53;
var ft_enum         = 54;
var ft_set          = 55;
var ft_myyear       = 59;
var ft_mygeometry   = 60;
var ft_myjson       = 61;
var ft_mydecimal    = 62;

//file flag
var table_varlen   = 0;
				
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

var CHARSET_LATIN1	= 1;
var CHARSET_CP850	= 4;
var CHARSET_ASCII	= 9;
var CHARSET_SJIS	= 11;
var CHARSET_UTF8	= 22;
var CHARSET_USC2	= 23;
var CHARSET_UTF8B4	= 30;
var CHARSET_UTF16LE	= 33;
var CHARSET_CP932	= 38;
var CHARSET_EUCJ	= 40;
var MAX_CHAR_INFO	= 41;

var CP_UTF8 = 65001;
	
// status
var STATUS_SUCCESS              = 0;
var STATUS_PROGRAM_ERROR        = 1;
var STATUS_IO_ERROR             = 2;
var STATUS_FILE_NOT_OPENED      = 3;
var STATUS_NOT_FOUND_TI         = 4;
var STATUS_DUPPLICATE_KEYVALUE  = 5;
var STATUS_INVALID_KEYNUM       = 6;
var STATUS_NO_CURRENT           = 8;
var STATUS_EOF                  = 9;
var STATUS_TABLE_NOTOPEN        = 12;
var STATUS_REQUESTER_DEACTIVE   = 20;
var STATUS_KEYBUFFERTOOSMALL    = 21;
var STATUS_BUFFERTOOSMALL       = 22;
var STATUS_CANT_CREATE          = 25;
var STATUS_NOSUPPORT_OP         = 41;
var STATUS_INVALID_BOOKMARK     = 43;
var STATUS_ACCESS_DENIED        = 46;
var STATUS_INVALID_OWNERNAME    = 51;
var STATUS_TABLE_EXISTS_ERROR   = 59;
var STATUS_LIMMIT_OF_REJECT     = 60;
var STATUS_WARKSPACE_TOO_SMALL  = 61;
var STATUS_REACHED_FILTER_COND  = 64;
var STATUS_INVALID_FIELD_OFFSET = 65;
var STATUS_CHANGE_CONFLICT      = 80;
var STATUS_INVALID_LOCKTYPE     = 83;
var STATUS_LOCK_ERROR           = 84;
var STATUS_FILE_LOCKED          = 85;
var STATUS_CANNOT_LOCK_TABLE    = 88;
var STATUS_INVALID_KEYNAME      = STATUS_INVALID_KEYNUM;
var STATUS_INVALID_DATASIZE     = STATUS_BUFFERTOOSMALL;
var STATUS_INVALID_FIELDNAME    = STATUS_INVALID_FIELD_OFFSET;
var ERROR_TD_INVALID_CLINETHOST = 171;
var ERROR_NOSPECIFY_TABLE       = 176;
var ERROR_LOAD_CLIBRARY         = 200;
var NET_BAD_SRB_FORMAT          = 3021;
var ERROR_TD_HOSTNAME_NOT_FOUND = 3103;
var ERROR_TD_CONNECTION_FAILURE = 3106;
var ERROR_TD_NOT_CONNECTED      = 3110;

var COMP_TYPE_NUMERIC = 1;
var COMP_TYPE_OBJECT = 0;
var COMP_TYPE_DATETIME = 2;

var COMBINE_TYPE_END = 0;
var COMBINE_TYPE_AND = 1;
var COMBINE_TYPE_END = 2;

var COMP_LOG_EQ = 0;
var COMP_LOG_EQ_GR = 1;
var COMP_LOG_EQ_LE = 2;
var COMP_LOG_GR = 3;
var COMP_LOG_LE = 4;
var COMP_LOG_NOT = 5;

var DIRECTION_FORWORD = 0;
var DIRECTION_BACKFORWORD = 1;

var TIMESTAMP_VALUE_CONTROL = 0;
var TIMESTAMP_ALWAYS = 1;

var fsum = 0;
var fcount = 1;
var favg = 2;
var fmin = 3;
var fmax = 4;

var none = 0;
var hasOneJoin = 1;
var joinWhereFields = 2;

var CMP_MODE_MYSQL_NULL = 1;
var CMP_MODE_OLD_NULL = 0;

var clearNull = 0;
var defaultNull = 1;

var DFV_TIMESTAMP_DEFAULT = 1;
var MYSQL_TYPE_MYSQL = 77;//'M'
var MYSQL_TYPE_MARIA = 65;//'A'

/*--------------------------------------------------------------------------------*/
function bench()
{
	var tick=0;

	this.report = function(func, p1, p2, p3, p4, p5, p6, p7, p8)
	{
		var now = new Date();
		ticks = now.getTime();
		var ret = func(p1, p2, p3, p4, p5, p6, p7, p8);
		now = new Date();
		ticks = (now.getTime() - ticks)/1000;
		return ret;
	}

	this.time = function(){return tick;}
	this.show = function(){WScript.Echo("\n(exec time " + ticks + " sec)\n");}
}
/*--------------------------------------------------------------------------------*/
function checkEqual(a, b, on)
{
	if (a !== b)
	{
		if (typeof(on) === 'undefined') on = "";
		try
		{
			WScript.Echo("error on " + on + " " + a.toString() + " != " + b.toString());	
		}
		catch(e)
		{
			WScript.Echo("check object error on " + on);	
		}
		resultCode = 1;
	}else
		WScript.StdOut.Write(".");
}

/*--------------------------------------------------------------------------------*/
function checkNotEqual(a, b, on)
{
	if (a === b)
	{
		if (typeof(on) === 'undefined') on = "";
		    
		try
		{
			WScript.Echo("error on " + on + " " + a.toString() + " == " + b.toString());	
		}
		catch(e)
		{
			WScript.Echo("check object error on " + on);	
		}
		resultCode = 1;
	}else
		WScript.StdOut.Write(".");
}
/*--------------------------------------------------------------------------------*/
function checkNotNull(a, on)
{
	if (a == null)
	{
		WScript.Echo("error null obkect on " + on );	
		return false;
	}
	WScript.StdOut.Write(".");
	return true;
}

/*--------------------------------------------------------------------------------*/
function isX86()
{
	var shell = new ActiveXObject('WScript.Shell');
	return (shell.Environment('Process').Item('PROCESSOR_ARCHITECTURE') === 'x86');
}
/*--------------------------------------------------------------------------------*/
function createRecordsetQuery()
{
	return new ActiveXObject('transactd.recordsetQuery');
}
/*--------------------------------------------------------------------------------*/
function createQuery()
{
	return new ActiveXObject('transactd.query');
}
/*--------------------------------------------------------------------------------*/
function createDatabaseObject()
{
	return new ActiveXObject("transactd.database");
}
/*--------------------------------------------------------------------------------*/
function createGroupQuery()
{
	return new ActiveXObject("transactd.groupQuery");
}
/*--------------------------------------------------------------------------------*/
function createActiveTable(db, tableName)
{
	var at =  new ActiveXObject("transactd.activeTable");
	at.SetDatabase(db, tableName);
	return at;
}
/*--------------------------------------------------------------------------------*/
function createSortFields()
{
	return new ActiveXObject("transactd.sortFields");
}
/*--------------------------------------------------------------------------------*/
function createFieldNames()
{
	return new ActiveXObject("transactd.fieldNames");
}
/*--------------------------------------------------------------------------------*/
var sep = "-------------------------------------------------------------------------------";
var FMT_LEFT = 0;
var FMT_CENTER = 1;
var FMT_RIGHT = 2;
var MAGNIFICATION = 100;
var resultCode = 0;
var q;

WScript.quit(main());
/*--------------------------------------------------------------------------------*/
function initQuery()
{
	q.Reset();
	gq.Reset();
}
/*--------------------------------------------------------------------------------*/
function isMySQL5_5(db)
{
    var ver = db.GetBtrVersion(1);
    return (db.Stat == 0) && ((5 == ver.MajorVersion) && (5 == ver.MinorVersion));
}
/*--------------------------------------------------------------------------------*/
function isMariaDbWithGtid(db)
{
    var ver = db.GetBtrVersion(1);
    return (db.Stat == 0) && (ver.Type == MYSQL_TYPE_MARIA) &&  (10 == ver.MajorVersion);
}

/*--------------------------------------------------------------------------------*/
function isLegacyTimeFormat(db)
{
	var ver = db.GetBtrVersion(1);
	return (db.Stat == 0) && ((5 == ver.MajorVersion) && (5 == ver.MinorVersion)
    	&&  ver.Type == MYSQL_TYPE_MYSQL);
}
/*--------------------------------------------------------------------------------*/
function createUserTable(db)
{
	var dbdef = db.DbDef;
	var tableid = 1;
	
	var tableDef =  dbdef.InsertTable(tableid);
	tableDef.TableName = "user";
	tableDef.FileName = "user";
	tableDef.CharsetIndex = CHARSET_CP932;
	tableDef.SchemaCodePage = CP_UTF8;

	var fieldIndex = 0;
	var fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "id";
	fd.Type = ft_autoinc;
	fd.Len = 4;

	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "名前";
	fd.Type = ft_myvarchar;
	fd.len = 2;
	checkEqual(fd.ValidCharNum, false, "validCharNum 1");
	fd.SetLenByCharnum(20);
	checkEqual(fd.ValidCharNum, true, "validCharNum 2");
	fd.DefaultValue = "John";
	checkEqual(fd.PadCharType, false, "isPadCharType ");
	checkEqual(fd.DateTimeType, false, "isDateTimeType");


	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "group";
	fd.Type = ft_integer;
	fd.Len = 4;
	fd.DefaultValue = 10;

	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "tel";
	fd.Type = ft_myvarchar;
	fd.SetLenByCharnum(21);
	fd.SetNullable(true);
	
	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "update_datetime";
	fd.Type = ft_mytimestamp;
	fd.Len = 7;
	fd.DefaultValue = DFV_TIMESTAMP_DEFAULT;
	fd.TimeStampOnUpdate = true;
	checkEqual(fd.TimeStampOnUpdate, true, "TimeStampOnUpdate 1-");


	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "create_datetime";
	if (isMySQL5_5(db))
	{
		fd.Type = ft_mydatetime;
		fd.Len = 8;
	}
	else
	{
		fd.Type = ft_mytimestamp;
		fd.Len = 4;
		fd.DefaultValue = DFV_TIMESTAMP_DEFAULT;
	}
	fd.TimeStampOnUpdate = false;
	checkEqual(fd.PadCharType, false, "isPadCharType ");
	checkEqual(fd.DateTimeType, true, "isDateTimeType");

	var keyNum = 0;
	var key = dbdef.InsertKey(tableid, keyNum);
	var seg1 = key.Segments(0);
	seg1.FieldNum = 0;
	seg1.Flags.Bits(key_extend) = true;    //extended key type
	seg1.Flags.Bits(key_changeable) = true;//chanageable
	key.SegmentCount = 1;
	tableDef.PrimaryKeyNum = keyNum;

	++keyNum;
	key = dbdef.InsertKey(tableid, keyNum);
	seg1 = key.Segments(0);
	seg1.FieldNum = 2;
	seg1.Flags.Bits(key_duplicate) = true;    
	seg1.Flags.Bits(key_extend) = true;    
	seg1.Flags.Bits(key_changeable) = true;
	key.SegmentCount = 1;

	dbdef.UpDateTableDef(tableid);
	if (dbdef.Stat!=0)
	{
		 WScript.Echo("user UpDateTableDef erorr:No." + dbdef.Stat);
		 return false;
	}
	dbdef = null;
	return true;
}
/*--------------------------------------------------------------------------------*/
function createUserExtTable(db)
{
	var dbdef = db.DbDef;
	var tableid = 3;
	
	var tableDef =  dbdef.InsertTable(tableid);
	tableDef.TableName = "extention";
	tableDef.FileName = "extention";
	tableDef.CharsetIndex = CHARSET_CP932;
	tableDef.SchemaCodePage = CP_UTF8;
	
	var fieldIndex = 0;
	var fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "id";
	fd.Type = ft_integer;
	fd.Len = 4;

	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "comment";
	fd.Type = ft_myvarchar;
	fd.SetLenByCharnum(60);
	fd.SetNullable(true);
	checkEqual(fd.DefaultNull, true, "DefaultNull 1");

	++fieldIndex;
	fd =  dbdef.InsertField(tableid, fieldIndex);
	fd.Name = "bits";
	fd.Type = ft_integer;
	fd.len = 8;


	var keyNum = 0;
	var key = dbdef.InsertKey(tableid, keyNum);
	var seg1 = key.Segments(0);
	seg1.FieldNum = 0;
	seg1.Flags.Bits(key_extend) = true;    //extended key type
	seg1.Flags.Bits(key_changeable) = true;//chanageable
	key.SegmentCount = 1;
	tableDef.PrimaryKeyNum = keyNum;
	dbdef.UpDateTableDef(tableid);
	if (dbdef.Stat!=0)
	{
		 WScript.Echo("extention UpDateTableDef erorr:No." + dbdef.Stat);
		 return false;
	}
	dbdef = null;
	return true;
}

/*--------------------------------------------------------------------------------*/
function createDatabase(db, uri)
{
	db.Create(uri);
	if (db.Stat!=0)
	{
		WScript.Echo("createDatabase erorr:No." + db.Stat + " " + uri);
		return false;
	}
	if (db.Open(uri, TYPE_BDF, OPEN_NORMAL, "", ""))
	{
		if (!createUserTable(db))return false;
		if (!createUserExtTable(db))return false;
		return true;
	}else
		WScript.Echo("open daatabse erorr:No" +  db.stat);
	return false;
}
/*--------------------------------------------------------------------------------*/
function insertData(db)
{
	var tb = db.OpenTable("user", OPEN_NORMAL);
	checkEqual(db.Stat, 0, "OpenTable user"); 
	var tb3 = db.OpenTable("extention", OPEN_NORMAL); 
	checkEqual(db.Stat, 0, "OpenTable extention"); 

	try
	{
		db.BeginTrn();
		tb.ClearBuffer();
		for (var i= 1;i<= 1000;++i)
		{
			tb.SetFV(0, i);
			tb.SetFV(1, i.toString() + " user");
			tb.SetFV("group", ((i-1) % 5)+1);
			tb.Insert();
		}
		
		tb3.ClearBuffer();
		for (var i= 1;i<= 1000;++i)
		{
			tb3.SetFV(0, i);
			tb3.SetFV(1, i.toString() + " comment");
			tb3.Insert();
		}
		db.EndTrn();
	}
	catch(e)
	{
		db.AbortTrn();
		throw e;
	}
}
/*--------------------------------------------------------------------------------*/
function todayStr()
{
	var d = new Date();
	var m =  d.getMonth()+1;
	var dt = d.getDate()
	if (m < 10)	m = "0" + m;
	if (dt < 10) dt = "0" + dt;
	return d.getFullYear() + "-" + m + "-" + dt;
}
/*--------------------------------------------------------------------------------*/
function test_bit(ate, db)
{
	var tb = ate.table();

	tb.KeyNum = 0;
	tb.setFV('id', 1);
	tb.seek();
	checkEqual(tb.Stat, 0);
	var bits = new  ActiveXObject("transactd.Bitset");
	/*
	bits.bit(63, true);
	bits.bit(2, true);
	bits.bit(5, true);
	*/
	bits(63) = true;
	bits(2) =  true;
	bits(5) =  true;
	
	tb.setFV('bits', bits);
	tb.update();
	checkEqual(tb.Stat, 0);
	
	initQuery();
	q.Where('id', '=', 1);
	var rs = ate.index(0).keyValue(1).read(q);
	checkEqual(rs.size , 1);
	bits = rs(0)('bits').GetBits();

	checkEqual(bits.bit(63), true);
	checkEqual(bits.bit(2), true);
	checkEqual(bits.bit(5), true);
	checkEqual(bits.bit(62), false);
	checkEqual(bits.bit(0), false);
	checkEqual(bits.bit(12), false);
	
	checkEqual(bits(63), true);
	checkEqual(bits(2), true);
	checkEqual(bits(5), true);
	checkEqual(bits(62), false);
	checkEqual(bits(0), false);
	checkEqual(bits(12), false);

	var wr = ate.getWritableRecord();
	wr('id').SetValue(1);
	/*
	bits.bit(63) = false;
	bits.bit(12) = true;
	bits.bit(0) = true;
	bits.bit(62) = true;
	*/
	bits(63) = false;
	bits(12) = true;
	bits(0) =  true;
	bits(62) =  true;

	wr('bits').SetValue(bits);
	wr.update();
	tb.setFV('id', 1);
	tb.seek();
	checkEqual(tb.Stat, 0);
	bits = tb.getFVbits('bits');
	
	checkEqual(bits.bit( 63), false);
	checkEqual(bits.bit( 2), true);
	checkEqual(bits.bit( 5), true);
	checkEqual(bits.bit( 12), true);
	checkEqual(bits.bit( 0), true);
	checkEqual(bits.bit( 62), true);
	checkEqual(bits.bit( 11), false);
	checkEqual(bits.bit( 13), false);
	
	checkEqual(bits(63), false);
	checkEqual(bits(2), true);
	checkEqual(bits(5), true);
	checkEqual(bits(12), true);
	checkEqual(bits(0), true);
	checkEqual(bits(62), true);
	checkEqual(bits(11), false);
	checkEqual(bits(13), false);

}
/*--------------------------------------------------------------------------------*/
function test_bitset()
{
	var bits1 = new  ActiveXObject("transactd.Bitset");
	var bits2 = new  ActiveXObject("transactd.Bitset");
	bits1(0) = true;
	bits1(1) = true;
	bits1(63) = true;
	
	bits2(0) = true;
	bits2(1) = false;
	bits2(63) = true;
	
	checkEqual(bits1.equals(bits2), false);
	checkEqual(bits1.contains(bits2), true);
	checkEqual(bits2.contains(bits1), false);
	
	var all = false;
	checkEqual(bits2.contains(bits1, all), true);
}

/*--------------------------------------------------------------------------------*/
function test_decimal(fd)
{
	fd.type = ft_mydecimal;
	fd.setDecimalDigits(65, 30);
	checkEqual(fd.digits, 65);
	checkEqual(fd.decimals, 30);
	checkEqual(fd.isIntegerType, false);
	checkEqual(fd.isNumericType, true);
	var bits1 = new  ActiveXObject("transactd.Bitset");
	bits1(2) = true;
	fd.type = ft_integer;
	fd.len = 4;
	fd.DefaultValue = bits1;
	checkEqual(fd.DefaultValue, '4');
}

function testBinlogPos(db)
{
	var bpos = db.beginSnapshot(CONSISTENT_READ_WITH_BINLOG_POS);
	if (checkNotNull(bpos, "beginSnapshot result "))
	{
		if (isMariaDbWithGtid(db))
			checkEqual(bpos.type, REPL_POSTYPE_MARIA_GTID,  "bpos.type");
		else
			checkEqual(bpos.type, REPL_POSTYPE_POS,  "bpos.type");
		checkNotEqual(bpos.pos, 0,  "bpos.pos");
		checkNotEqual(bpos.filename, "",  "bpos.filename");
		WScript.Echo("\nBinlog pos = " + bpos.filename + ":" + bpos.pos);
	}
	db.endSnapshot();
}

function testConnMgr(uri)
{
	var mgr = new ActiveXObject('transactd.connMgr');
	var db  = createDatabaseObject();
	mgr.setDatabase(db);
	mgr.connect(uri);
	checkEqual(db.stat , 0,  "mgr.connect");
	
	//connections
	var recs =  mgr.Connections();
	checkEqual(mgr.stat , 0,  "mgr.Connections");
	//InUseDatabases
	var recs1 = mgr.InUseDatabases(recs(0).conid);
	checkEqual(mgr.stat , 0,  "mgr.InUseDatabases");
	//InUseTables
	var recs2 = mgr.InUseTables(recs(0).conid, recs(0).db);
	checkEqual(mgr.stat , 0,  "mgr.InUseTables");

	checkEqual(recs.size , 1, "mgr.Connections.size");
	checkEqual(recs1.size , 1, "mgr.InUseDatabases.size");
	checkEqual(recs2.size , 4, "mgr.InUseTables.size");
	
	//tables
	recs =  mgr.tables("test_v3");
	checkEqual(mgr.stat , 0,  "mgr.tables");
	checkEqual(recs.size , 3, "mgr.tables.size");
	
	//views
	recs = mgr.views("test_v3");
	checkEqual(mgr.stat , 0,  "mgr.views");
	checkEqual(recs.size , 1, "mgr.views.size");
	checkEqual(recs(0).name , "idlessthan5");
	
	//schemaTables
	recs = mgr.schemaTables("test_v3");
	checkEqual(mgr.stat , 0,  "mgr.schemaTables");
	checkEqual(recs.size , 1, "mgr.schemaTables.size");
	checkEqual(recs(0).name , "test");
	
	//databases
	recs = mgr.databases();
	checkEqual(mgr.stat , 0,  "mgr.databases");
	var size = recs.size;
	mgr.RemoveSystemDb(recs);
	checkNotEqual(size , recs.size,  "RemoveSystemDb recs");
	
	//slaveStatus
	recs = mgr.slaveStatus();
	checkEqual(mgr.stat , 0,  "mgr.slaveStatus");
	var status = "";
	/*for (var i = 0; i<recs.size; ++i)
	   status += (mgr.SlaveStatusName(i) + "\t:" + recs(i).value + "\n");
	*/
	mgr.disconnect();
	checkEqual(mgr.stat , 0,  "mgr.disconnect");
	WScript.Echo("\n\n" + status);
}


/*--------------------------------------------------------------------------------*/
function test(atu, ate, db)
{
	//WScript.Echo(" -- Start Test -- ");
	var x86 = isX86();
	
	db.AutoSchemaUseNullkey = true;
	checkEqual(db.AutoSchemaUseNullkey, true, "AutoSchemaUseNullkey");
	db.AutoSchemaUseNullkey = false;
	checkEqual(db.AutoSchemaUseNullkey, false, "AutoSchemaUseNullkey");
	
	checkEqual(db.CompatibleMode, CMP_MODE_MYSQL_NULL, "CompatibleMode 1");

	db.CompatibleMode = CMP_MODE_OLD_NULL;
	checkEqual(db.CompatibleMode, CMP_MODE_OLD_NULL, "CompatibleMode 2");

	db.CompatibleMode = CMP_MODE_MYSQL_NULL;
	checkEqual(db.CompatibleMode, CMP_MODE_MYSQL_NULL, "CompatibleMode 3");
	

	
	var dbdef = db.DbDef;
	var td = dbdef.TableDef(1);
	
	//MysqlNullMode
	checkEqual(td.MysqlNullMode , true, "MysqlNullMode");
	
	var len = 104;
	if (isMySQL5_5(db))
		len += 4;
	if (isLegacyTimeFormat(db))
		len -= 3;
	checkEqual(td.RecordLen , len, "recordlen");
	//InUse
	checkEqual(td.InUse , 2, "InUse");

	//nullfields
	checkEqual(td.Nullfields , 1, "nullfields");

	//size()
	checkEqual(td.Size , 1184, "size");

	//fieldNumByName
	checkEqual(td.FieldNumByName("tel") , 3, "fieldNumByName");

	//default value
	var fd = td.FieldDef(1);
	checkEqual(fd.DefaultValue, "John", "default value 1");
	fd = td.FieldDef(2);
	checkEqual(fd.DefaultValue, "10", "default value 2");
	fd = td.FieldDef(3);
	checkEqual(fd.DefaultNull, true, "DefaultNull");
	fd = td.FieldDef(4);
	checkEqual(fd.DefaultValue, String(DFV_TIMESTAMP_DEFAULT), "default value timestamp");
	checkEqual(fd.TimeStampOnUpdate, true, "TimeStampOnUpdate 1");
	fd = td.FieldDef(5);
	checkEqual(fd.TimeStampOnUpdate, false, "TimeStampOnUpdate 2");


	fd = td.FieldDef(1);
	// synchronizeSeverSchema
	var len = fd.Len;

	fd.SetLenByCharnum(19);
	checkNotEqual(len, fd.Len, "synchronizeSeverSchema 1");
	dbdef.SynchronizeSeverSchema(1); 
	td = dbdef.TableDef(1);
	fd = td.FieldDef(1);
	checkEqual(len, fd.Len, "synchronizeSeverSchema 2");
	
	// syncronize default value
	fd = td.FieldDef(1);
	checkEqual(fd.DefaultValue, "John", "default value 2-1");
	fd = td.FieldDef(2);
	checkEqual(fd.DefaultValue, "10", "default value 2-2");
	fd = td.FieldDef(3);
	checkEqual(fd.DefaultNull, true, "DefaultNull 2");
	fd = td.FieldDef(4);
	checkEqual(fd.TimeStampOnUpdate, true, "TimeStampOnUpdate 2-1");
	fd = td.FieldDef(5);
	checkEqual(fd.TimeStampOnUpdate, false, "TimeStampOnUpdate 2-2");
	
	// nullable
	fd = td.FieldDef(3);
	checkEqual(fd.Nullable, true, "Nullable");

	// getSqlStringForCreateTable
	var sql = db.GetSqlStringForCreateTable("extention");
	checkEqual(db.Stat, 0, "GetSqlStringForCreateTable");
	checkEqual(sql, 'CREATE TABLE `extention` (`id` INT NOT NULL ,`comment` VARCHAR(60) binary NULL DEFAULT NULL,`bits` BIGINT NOT NULL , UNIQUE key0(`id`)) ENGINE=InnoDB default charset=cp932',
		 "GetSqlStringForCreateTable");

	// setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
	td = dbdef.TableDef(1);
	td.SetValidationTarget(true, 0);

	// isNull setNull
	initQuery();
	
	// segmentsForInValue
	checkEqual(q.SegmentsForInValue(3).GetJoinKeySize(), 3, "SegmentsForInValue");
	q.Reset();
	checkEqual(q.GetJoinKeySize(), 0, "GetJoinKeySize");

	atu.Alias("名前", "name");
	q.Select("id", "name", "group", "tel").Where("id", "<=", 10);
	var rs = atu.Index(0).KeyValue(null).Read(q);
	checkEqual(rs.Count, 10, "atu rs.Count = 10 ");
	var rec = rs.First();
	checkEqual(rec(3).IsNull(), true, "NULL true");
	rec(3).setNull(false);
	checkEqual(rec(3).IsNull(), false, "NULL false");
	
	//Join null
	initQuery();
	var last = ate.Index(0).Join(rs, q.Select("comment").Optimize(hasOneJoin), "id").Reverse().First();
	checkEqual(rs.Count, 10, "ate rs.Count = 10 ");
	checkEqual(last.Field("id").i(), 10, "last.id = 10 ");
	if (!x86)
		checkEqual(last.Field("id").i64(), 10, "last.id = 10 ");
	checkEqual(last.Field("id").d(), 10, "last.id = 10 ");
	checkEqual(rec(4).IsNull(), false, "Join NULL1");
	rec(4).setValue(null);
	checkEqual(rec(4).IsNull(), true, "Join NULL2");
	
	//WritableRecord.clear()
	var wr = atu.getWritableRecord();
	wr.Clear();
	wr("id").setValue(5);
	wr("tel").setValue("0236-99-9999");
	wr.Update();
	wr.Clear();
	wr("id").setValue(5);

	checkEqual(wr.Read(), true, "wr.Read");
	checkEqual(wr("tel").str(), "0236-99-9999", "tel ");

	//whereIsNull
	initQuery();
	q.Select("id", "tel").WhereIsNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 999, "atu rs.Count = 999 ");

	//whereIsNotNull
	initQuery();
	q.Select("id", "tel").WhereIsNotNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 1, "atu rs.Count = 1 ");

	//AndIsNull
	initQuery();
	q.Select("id", "tel").Where("id", "<=", 10).AndIsNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 9, "atu rs.Count = 9 ");

	//AndIsNotNull
	initQuery();
	q.Select("id", "tel").Where("id", "<", 10).AndIsNotNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 1, "atu rs.Count = 1 ");

	//OrIsNull
	initQuery();
	q.Select("id", "tel").Where("id", "<=", 10).OrIsNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 1000, "atu rs.Count = 1000 ");

	//OrIsNotNull
	initQuery();
	q.Select("id", "tel").Where("id", "<=", 10).OrIsNotNull("tel").Reject(0xFFFF);
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 10, "atu rs.Count = 10 ");

	//test recordset query
	q.Reset();
	q.Select("id", "name", "group", "tel");
	rs = atu.Index(0).KeyValue(0).Read(q);
	checkEqual(rs.Count, 1000, "rs.Count = 1000 ");
	
	// recordset whenIsNull
	var rq = createRecordsetQuery();
	rq.WhenIsNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 999, "rs.Count = 999 ");
	
	//recordset whenIsNotNull
	rq.Reset();
	rq.WhenIsNotNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 1, "rs.Count = 1 ");
	
	//recordset andIsNull
	rq.Reset();
	rq.When("id", "<=", 10).AndIsNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 9, "rs.Count = 9 ");
	
	//recordset andIsNotNull
	rq.Reset();
	rq.When("id", "<", 10).AndIsNotNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 1, "rs.Count = 1 ");
	
	// recordset orIsNull
	rq.Reset();
	rq.When("id", "<=", 10).OrIsNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 1000, "rs.Count = 1000 ");
	
	//recordset orIsNotNull
	rq.Reset();
	rq.When("id", "<=", 10).OrIsNotNull("tel");
	rs2 = rs.Clone();
	rs2 = rs2.MatchBy(rq);
	checkEqual(rs2.Count, 10, "rs.Count = 10 ");

	//setBin bin
	var bin = String.fromCharCode(0xFF01,0xFF02);
	wr("tel").SetBin(bin);
	var ret = wr("tel").Bin();
	checkEqual(ret.charCodeAt(0), 0xFF01, "SetBin Bin");
	checkEqual(ret.charCodeAt(1), 0xFF02, "SetBin Bin");

	
	// table::default NULL
	var tb = db.OpenTable("user");
	checkEqual(db.Stat, 0, "");
	tb.KeyNum = 0;
	tb.ClearBuffer();
	checkEqual(tb.GetFVNull(3), true, "Default NULL");
	
	tb.ClearBuffer(clearNull);
	checkEqual(tb.GetFVNull(3), false, "clearNull NULL");

	// table NULL
	tb.SetFV("id", 1);
	tb.Seek();
	checkEqual(tb.Stat, 0, "Seek");
	checkEqual(tb.GetFVNull(3), true, "Default NULL");
	checkEqual(tb.GetFVNull("tel"), true, "Default NULL");
	tb.SetFVNull(3, false);
	checkEqual(tb.GetFVNull(3), false, "tb.Null");
	tb.SetFVNull("tel", true);
	checkEqual(tb.GetFVNull("tel"), true, "tb.Null");
	if (x86)
	{
		tb.SetFV("id", 10);
		checkEqual(tb.GetFVint("id"), 10, "tb.SetFV");
		tb.SetFV("id", "10");
		checkEqual(tb.GetFVint("id"), 10, "tb.SetFV");
		tb.SetFV("id", 10.00);
		checkEqual(tb.GetFVint("id"), 10, "tb.SetFV");
	}
	else
	{
		tb.SetFV("id", 10);
		checkEqual(tb.GetFV64("id"), 10, "tb.SetFV");
		tb.SetFV("id", "10");
		checkEqual(tb.GetFV64("id"), 10, "tb.SetFV");
		tb.SetFV("id", 10.00);
		checkEqual(tb.GetFV64("id"), 10, "tb.SetFV");
	}
	checkEqual(tb.GetFVstr("id"), "10", "tb.SetFV");

	// timestamp format
	var date = todayStr();
	checkEqual(tb.getFVstr("update_datetime").substr(0, 10), date);
	if (!isMySQL5_5(db))
		checkEqual(tb.getFVstr("create_datetime").substr(0, 10), date);

	// setTimestampMode
	tb.SetTimestampMode(TIMESTAMP_VALUE_CONTROL);
	tb.SetTimestampMode(TIMESTAMP_ALWAYS);

	// MysqlNullMode
	checkEqual(tb.TableDef.MysqlNullMode , true, "MysqlNullMode 2");
	checkEqual(td.InUse , 2, "InUse2");
	
	test_bit(ate, db);
	test_bitset();
	test_decimal(ate.TableDef.FieldDef(0));
	
	//binlogPos
	testBinlogPos(db);
	
	//getCreateViewSql
	db.createTable("create view idlessthan5 as select * from user where id < 5");
	var view = db.getCreateViewSql("idlessthan5");
	checkNotEqual(view.indexOf("idlessthan5") , -1, "getCreateViewSql");
	checkNotEqual(view.indexOf("名前") , -1, "getCreateViewSql 2");
	//getCreateSql2
	var sql = tb.getCreateSql();
	checkNotEqual(sql.indexOf("CREATE TABLE") , -1, "getCreateSql");
	checkNotEqual(view.indexOf("名前") , -1, "getCreateSql 2");
	
	//createAssociate()
	var dba = db.createAssociate();
	checkEqual(db.stat , 0, "createAssociate");
	checkEqual(dba.IsAssociate , true, "IsAssociate");
	// connMgr
	testConnMgr(db.uri);
	
	//WScript.Echo(" -- End Test -- ");
}
/*--------------------------------------------------------------------------------*/
function main()
{
	var isCreate = 1;
	var host = "localhost";
	var user = "root";
	var pwd = "";

	if (WScript.arguments.length > 0)
		isCreate = 	parseInt(WScript.arguments(0), 10);

	if (WScript.arguments.length > 1)
		host = WScript.arguments(1);
	if (WScript.arguments.length > 2)
		user = WScript.arguments(2);
	if (WScript.arguments.length > 3)
		pwd = WScript.arguments(3);

	var URI  = "tdap://" + user + "@" + host + "/test_v3?dbfile=test.bdf&pwd=" + pwd;


	WScript.Echo(URI);
	var b = new bench();
	try
	{
		var database  = createDatabaseObject();
		if (database == null)
		{
			WScript.Echo("transactd.database ActiveXObject erorr.");
			return 1;
		}
		if (database.Open(URI, TYPE_BDF, OPEN_NORMAL, "", ""))
		{
				
			if (isCreate > 0)
				database.Drop();
		}else
		{
			if (database.Stat != STATUS_TABLE_NOTOPEN )
			{
				WScript.Echo("database erorr: " + database.Stat);
				return 1;
			}
			isCreate = true;
		}
		if (isCreate > 0)
		{
			WScript.Echo("Creating test data. Please wait ...");
			if (createDatabase(database, URI))
				insertData(database);
		}

		if (database.Stat !== 0)
		{
			WScript.Echo("open table erorr:No" +  database.stat);
			return 2;
		}
		var atu = createActiveTable(database, "user");
		var ate = createActiveTable(database, "extention");

		q = createQuery();;
		gq = createGroupQuery();

		b.report(test, atu, ate, database);
		b.show();
		if (resultCode == 0)
			WScript.Echo("*** No errors detected.");
	}
	catch(e)
	{
		WScript.Echo("Error:" +  e.name + " " + e.description);
		return 2;
	}

	return resultCode;
}

