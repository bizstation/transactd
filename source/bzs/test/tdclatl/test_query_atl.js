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
var ft_autoIncUnsigned = 52;

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

var fsum = 0;
var fcount = 1;
var favg = 2;
var fmin = 3;
var fmax = 4;


var none = 0;
var hasOneJoin = 1;
var joinWhereFields = 2;

function createRecordsetQuery()
{
    return new ActiveXObject('transactd.recordsetQuery');
}

	
function createQuery()
{
    return new ActiveXObject('transactd.query');
}

function createDatabaseObject()
{
    return new ActiveXObject("transactd.database");
}

function createGroupQuery()
{
    return new ActiveXObject("transactd.groupQuery");
}

function createActiveTable(db, tableName)
{
    var at =  new ActiveXObject("transactd.activeTable");
    at.SetDatabase(db, tableName);
    return at;
}

function createSortFields()
{
    return new ActiveXObject("transactd.sortFields");
}



var sep = "-------------------------------------------------------------------------------";


var FMT_LEFT = 0;
var FMT_CENTER = 1;
var FMT_RIGHT = 2;
var MAGNIFICATION = 100;

var resultCode = 0;
var q;
var gq;

WScript.quit(main());

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
		
	var filedIndex = 0;
	var fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "id";
	fd.Type = ft_autoinc;
	fd.Len = 4;

	++filedIndex;
	fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "–¼‘O";
	fd.Type = ft_varchar;
	fd.SetLenByCharnum(20);

	++filedIndex;
	fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "group";
	fd.Type = ft_integer;
	fd.Len = 4;

	++filedIndex;
	fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "tel";
	fd.Type = ft_varchar;
	fd.SetLenByCharnum(21);

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
function createGroupTable(db)
{
	var dbdef = db.DbDef;
	var tableid = 2;
	
	var tableDef =  dbdef.InsertTable(tableid);
	tableDef.TableName = "groups";
	tableDef.FileName = "groups";
	tableDef.CharsetIndex = CHARSET_CP932;
	tableDef.SchemaCodePage = CP_UTF8;
	
	var filedIndex = 0;
	var fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "code";
	fd.Type = ft_integer;
	fd.Len = 4;

	++filedIndex;
	fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "name";
	fd.Type = ft_varbinary;
	fd.Len = 33;

	var keyNum = 0;
	var key = dbdef.InsertKey(tableid, keyNum);
	var seg1 = key.Segments(0);
	seg1.FieldNum = 0;
	seg1.Flags.Bits(key_extend) = true;    //extended key type
	seg1.Flags.Bits(key_changeable) = true;//chanageable
	key.SegmentCount = 1;
	tableDef.PrimaryKeyNum = keyNum;
	tableDef.Flags.Bits(table_varlen) = true;
	dbdef.UpDateTableDef(tableid);
	if (dbdef.Stat!=0)
	{
		 WScript.Echo("groups UpDateTableDef erorr:No." + dbdef.Stat);
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
	
	var filedIndex = 0;
	var fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "id";
	fd.Type = ft_integer;
	fd.Len = 4;

	++filedIndex;
	fd =  dbdef.InsertField(tableid, filedIndex);
	fd.Name = "comment";
	fd.Type = ft_varchar;
	fd.SetLenByCharnum(60);


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
	this.show = function(){WScript.Echo("(exec time " + ticks + " sec)\n");}
}



/*--------------------------------------------------------------------------------*/
function initQuery()
{
	q.Reset();
	gq.Reset();
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
		if (!createGroupTable(db))return false;
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
	var tb2 = db.OpenTable("groups", OPEN_NORMAL);
	var tb3 = db.OpenTable("extention", OPEN_NORMAL); 
	
	try
	{
		db.BeginTrn();
		
		tb.ClearBuffer();
		for (var i= 1;i<= 20000;++i)
		{
			tb.Vlng(0) = i;
			tb.Text(1) = i.toString() + " user";
			tb.Vlng("group") = ((i-1) % 5)+1;
			tb.Insert();
		}
		
		tb2.ClearBuffer();
		for (var i= 1;i<= 100;++i)
		{
			tb2.Vlng(0) = i;
			tb2.Text(1) = i.toString() + " group";
			tb2.Insert();
		} 
		
		tb3.ClearBuffer();
		for (var i= 1;i<= 20000;++i)
		{
			tb3.Vlng(0) = i;
			tb3.Text(1) = i.toString() + " comment";
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
function checkEqual(a, b, on)
{
	if (a !== b)
	{
		WScript.Echo("error on " + on + " " + a.toString() + " != " + b.toString());	
		resultCode = 1;
	}
}

/*--------------------------------------------------------------------------------*/
function checkNotEqual(a, b, on)
{
	if (a === b)
	{
		WScript.Echo("error on " + on + " " + a.toString() + " != " + b.toString());	
		resultCode = 1;
	}
}

/*--------------------------------------------------------------------------------*/
function wirteRecord(atg)
{
	//These test are throw exception when error occured. 
	atg.ResetAlias();
	var rec = atg.Index(0).GetWritableRecord();

 
    rec.Field("code").Vlng = 1200;
    rec.Field("name").Text = "–îŒû";
    if (!rec.Read())
        rec.Insert();

    rec.Clear();
    rec.Field("code").Vlng = 1201;
    rec.Field("name").Text = "¬â";
    if (!rec.Read())
        rec.Insert();

    //update changed filed only
    rec.Clear();
    rec.Field("code").Vlng = 1201;
    rec.Field("name").Vlng = 1240;
    rec.Update();

    rec.Del();
    rec.Field("code").Vlng = 1200;
    rec.Del();
}


/*--------------------------------------------------------------------------------*/
function test(atu, atg, ate)
{
	WScript.Echo(" -- Start Test -- ");

	initQuery();
	atu.Alias("–¼‘O", "name");
    q.Select("id", "name","group").Where("id", "<=", 15900);
    var rs = atu.Index(0).KeyValue(1).Read(q);
	checkEqual(rs.Count, 15900, "atu rs.Count = 15900 ");
	
	//Join extention::comment

    initQuery();
	//first reverse
	var last = ate.Index(0).Join(rs, q.Select("comment").Optimize(hasOneJoin), "id").Reverse().First();
	checkEqual(rs.Count, 15900, "ate rs.Count = 15900 ");
	checkEqual(last.Field("id").Vlng, 15900, "last.id = 15900 ");
	checkEqual(last.Field("comment").Text, "15900 comment", "last.comment = 15900 ");

	//Join group::name
	initQuery();
	atg.Alias("name", "group_name");
	atg.Index(0).Join(rs, q.Select("group_name"), "group");

	//last
	var first = rs.Last();
	checkEqual(rs.Count, 15900, "rs.Count = 15900 ");
	checkEqual(first.Field("id").Vlng, 1, "id = 1 ");
	checkEqual(first.Field("comment").Text, "1 comment", "comment = 1 comment");
	checkEqual(first.Field("group_name").Text, "1 group", "group_name = 1 group ");
 	checkEqual(rs.Record(15900 - 9).Field("group_name").Text, "4 group", "group_name = 9 group ");
	
	rs.OrderBy("group_name");
	checkEqual(rs.Record(0).Field("group_name").Text, "1 group", "group_name = 1 group ");
	
	var sortFields = createSortFields();
	sortFields.Add("group_name", false);
	rs.OrderByEx(sortFields);

	sortFields.Clear();
	sortFields.Add("group_name", true);
	rs.OrderByEx(sortFields);
	checkEqual(rs.Record(0).Field("group_name").Text, "1 group", "group_name = 1 group ");
	
	
	var rq = createRecordsetQuery();

	rq.When("group" ,"=", 1);
	gq.KeyField("group").AddFunction(fcount, "", "gropu1_count", rq);
	rs.groupBy(gq);
	checkEqual(rs.Count, 5, "groupBy rs.Count = 5 ");
	var row = rs.Record(0);
	
	checkEqual(row.Field("gropu1_count").Vlng, 3180, "gropu1_count = 3180 ");
	row = rs.Record(1);
	checkEqual(row.Field("gropu1_count").Vlng, 0, "gropu1_count = 0 ");
	
	var count = rs.Count;
		for (var j= 0;j<count;++j)
			var s = rs.Record(j);
			
	
	wirteRecord(atg);
	
	WScript.Echo(" -- End Test -- ");

}

/*--------------------------------------------------------------------------------*/
function main()
{

    var isCreate = 0;
    var host = "localhost";
    if (WScript.arguments.length > 0)
    	isCreate = 	parseInt(WScript.arguments(0), 10);
    
    if (WScript.arguments.length > 1)
    	host = WScript.arguments(1);
    var URI  = "tdap://" + host + "/querytest?dbfile=test.bdf";

    
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
	    var atg = createActiveTable(database, "groups");
		var ate = createActiveTable(database, "extention");		
		
		q = createQuery();;
		gq = createGroupQuery();
	
	    b.report(test, atu, atg, ate);
	    b.show();
	    if (resultCode == 0)
	    	WScript.Echo("*** No errors detected.");
	}
	catch(e)
	{
		WScript.Echo("line " + e.name + " " + e.description);
		return 2;
	}
	
    return resultCode;
}

