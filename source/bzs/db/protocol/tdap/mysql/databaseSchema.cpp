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
#include <boost/filesystem.hpp>
#include <boost/shared_array.hpp>
#include <bzs/db/engine/mysql/database.h>
#include <bzs/db/engine/mysql/mysqlInternal.h>
#include "databaseSchema.h"

#include <bzs/db/protocol/tdap/mysql/characterset.h>
#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>
#endif

namespace fs = boost::filesystem;

namespace bzs
{
	using namespace db::engine::mysql; 
namespace db
{
	using namespace protocol; 
namespace protocol
{
namespace tdap
{
namespace mysql
{

schemaBuilder::schemaBuilder(void)
{
}

schemaBuilder::~schemaBuilder(void)
{
}

void initTableDef(table* tb, tabledef& tdef, int id)
{
	tdef.cleanup();
	tdef.setTableName(tb->name().c_str());
	tdef.setFileName(tb->name().c_str());
	tdef.id = id;
	tdef.schemaCodePage = CP_UTF8;

}

uint_td copyToRecordImage(uchar* rec, void* p, uint_td size, uint_td offset)
{
	memcpy(rec+offset, p, size);
	return offset+size;
}

bool isStringType(char type)
{
	switch(type)
	{
	case ft_zstring:
	case ft_string:
	case ft_mychar:
	case ft_myvarchar:
	case ft_myvarbinary:
	case ft_wzstring:
	case ft_wstring:
	case ft_mywchar:
	case ft_mywvarchar:
	case ft_mywvarbinary:
		return true;
	default:
		return false;
	}
}

uchar_td convFieldType(enum enum_field_types type, uint flags, bool binary, bool unicode)
{
	switch(type)
	{
	case MYSQL_TYPE_DECIMAL:return ft_decimal;
	case MYSQL_TYPE_TINY:
	case MYSQL_TYPE_SHORT:
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_LONGLONG:
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_INT24:
		if (flags & AUTO_INCREMENT_FLAG)
			return ft_autoinc;
		if ((flags & UNSIGNED_FLAG) || (type == MYSQL_TYPE_YEAR))
			return ft_uinteger;
		return ft_integer;
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:return ft_float;
	case MYSQL_TYPE_DATE:return ft_mydate;       
	case MYSQL_TYPE_TIME:return ft_mytime;
	case MYSQL_TYPE_DATETIME:return ft_mydatetime;
	case MYSQL_TYPE_TIMESTAMP:return ft_mytimestamp;
	case MYSQL_TYPE_VARCHAR:
	case MYSQL_TYPE_VAR_STRING: //?
		if (binary)
			return unicode ? ft_mywvarbinary : ft_myvarbinary;
		return unicode ? ft_mywvarchar : ft_myvarchar;
	case MYSQL_TYPE_STRING:
		if (binary)
			return unicode ? ft_wstring : ft_string;
		return  unicode ? ft_mywchar : ft_mychar;
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
		if (flags & BINARY_FLAG)
			return ft_myblob;
		return ft_mytext;
	default:
		return unicode ? ft_wzstring : ft_zstring;
	}
	return 0;
}

bool isUnicode(const CHARSET_INFO& cs)
{
	return (charsetIndex(cs.csname)==CHARSET_UTF16LE);
}

bool isBinary(const CHARSET_INFO& cs)
{
	return (&cs == &my_charset_bin);
}

short schemaBuilder::insertMetaRecord(table* mtb, table* src, int id)
{
	uint_td datalen = 0;
	boost::shared_array<uchar> rec(new uchar[65000]);
	//table
	tabledef& tdef = (tabledef&)(*rec.get());
	initTableDef(src, tdef, id);
	tdef.fieldCount = (uchar_td)src->fields();
	tdef.keyCount = (uchar_td)src->keys();
	tdef.charsetIndex = charsetIndex(src->charset().csname);
	tdef.flags.bit0 = (src->recordFormatType()== RF_FIXED_PLUS_VALIABLE_LEN);//‰Â•Ï’·
	
	tdef.primaryKeyNum = (src->primarykey()<tdef.keyCount) ? src->primarykey():-1;
	if (src->recordFormatType()== RF_FIXED_PLUS_VALIABLE_LEN)
		tdef.fixedRecordLen = (ushort_td)(src->recordLenCl()-src->lastVarFieldDataLen());
	else
		tdef.fixedRecordLen = (ushort_td)src->recordLenCl();
	tdef.maxRecordLen = (ushort_td)src->recordLenCl();
	tdef.pageSize = 2048;
	tdef.optionFlags.bitA = (src->recordFormatType() & RF_VALIABLE_LEN);
	tdef.optionFlags.bitB = src->blobFields()!=0;
	
	datalen+=sizeof(tabledef);
	tdef.fieldDefs = (fielddef*)(rec.get() + datalen);
	//field
	ushort_td pos = 0;
	for (int i=0;i<src->fields();++i)
	{
		if (isNisField(src->fieldName(i)))
			--tdef.fieldCount;
		else
		{
			fielddef fd;
			memset(&fd, 0, sizeof(fielddef));
			fd.setName(src->fieldName(i));
			fd.len = src->fieldLen(i);// filed->pack_length();
			fd.pos = pos;
			fd.type = convFieldType(src->fieldType(i), src->fieldFlags(i)
						,isBinary(src->fieldCharset(i)), isUnicode(src->fieldCharset(i)));
			pos += fd.len;
			datalen = copyToRecordImage(rec.get(), &fd, sizeof(fielddef), datalen);
		}
	}
	
	//index
	for (int i=0;i<src->keys();++i)
	{
		const KEY& key = src->keyDef(i);
		keydef kd;
		memset(&kd, 0, sizeof(keydef));
		kd.segmentCount = key.user_defined_key_parts;
		kd.keyNumber = i;
		int segNum = 0;
		for (int j=0;j<(int)key.user_defined_key_parts;++j)
		{
			keySegment& sg = kd.segments[segNum];
			KEY_PART_INFO& ks = key.key_part[j];
			if (isNisField(ks.field->field_name))
				--kd.segmentCount;
			else
			{
				sg.fieldNum = (uchar_td)ks.field->field_index;
				sg.flags.bit0 = !(key.flags & HA_NOSAME);//duplicate
				sg.flags.bit1 = 1;//change able
				sg.flags.bit4 = (j!=kd.segmentCount-1);//segment
				sg.flags.bit8 = 1;//extend key type
				sg.flags.bit9 = ks.null_bit ? 1:0;//null key
				if (isStringType(convFieldType(src->fieldType(sg.fieldNum), src->fieldFlags(sg.fieldNum)
						,isBinary(src->fieldCharset(i)), isUnicode(src->fieldCharset(sg.fieldNum)))))
					sg.flags.bitA = !(src->fieldFlags(sg.fieldNum) & BINCMP_FLAG);
				if (src->fieldDataLen(sg.fieldNum) != ks.length)
				{
					fielddef& fd = tdef.fieldDefs[sg.fieldNum];
					//suppot prefix key
					fd.keylen = ks.length;
					
				}
				if (sg.fieldNum == tdef.fieldCount-1)
				{
					tdef.flags.bit0 = 0; //NOT valiable length
					tdef.optionFlags.bitA = 0;//NOT RF_VALIABLE_LEN
				}
				++segNum;
			}
		}
		datalen = copyToRecordImage(rec.get(), &kd, sizeof(keydef), datalen);
	}
	mtb->clearBuffer();
	mtb->setRecordFromPacked(rec.get(), datalen, NULL);
	__int64 aincValue = mtb->insert(true);
	return mtb->stat();
}

bool isFrmFile(const std::string& name)
{
	size_t pos = name.find(TRANSACTD_SCHEMANAME);
	if (pos != std::string::npos)
		return false;
	//First of name is '#' that is alter table temp file.
	if (name.size() && name[0] == '#')
		return false;
	pos = name.find(".frm");
	if (pos != std::string::npos)
		return pos == name.size()-4; 
	return false;
}

short schemaBuilder::execute(database* db, table* mtb)
{
	char path[FN_REFLEN + 1];
	build_table_filename(path, sizeof(path) - 1, db->name().c_str(), "", "", 0);
	
	std::string s = path;
	fs::path p = s;
	fs::directory_iterator it(p);
	fs::directory_iterator end;
	int id = 0;
	short stat=0;
	smartTransction trn(db);
	for(fs::directory_iterator it(p); it!=end; ++it)
	{
		if(!is_directory(*it))
		{	
			std::string s = it->path().filename().string();
			if (isFrmFile(s))
			{
				filename_to_tablename(it->path().stem().string().c_str(), path, FN_REFLEN);
				table* tb = db->openTable(path, TD_OPEN_READONLY);
				if (!tb->isView())
				{
					if ((stat = insertMetaRecord(mtb, tb, ++id))!=0)
						return stat;
				}
			}
		}
	}
	trn.end();
	return stat;
}

}//namespace mysql
}//namespace protocol
}//namespace db
}//namespace tdap
}//namespace bzs
