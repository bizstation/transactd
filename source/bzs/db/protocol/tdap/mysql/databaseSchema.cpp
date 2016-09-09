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
#include <my_config.h>
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

schemaBuilder::schemaBuilder(unsigned char binFdCharset): m_stat(0), m_binFdCharset(binFdCharset)
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
    memcpy(rec + offset, p, size);
    return offset + size;
}

uchar_td convFieldType(enum enum_field_types type, uint flags, bool binary,
                       bool unicode)
{
    switch (type)
    {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
        return ft_mydecimal;
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
        if (flags & AUTO_INCREMENT_FLAG)
            return (flags & UNSIGNED_FLAG) ? ft_autoIncUnsigned : ft_autoinc;
        if ((flags & UNSIGNED_FLAG) || (type == MYSQL_TYPE_YEAR))
            return ft_uinteger;
        return ft_integer;
    case MYSQL_TYPE_ENUM:
        return ft_enum;
    case MYSQL_TYPE_SET:
        return ft_set;
    case MYSQL_TYPE_BIT:
        return ft_bit;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
        return ft_float;
    case MYSQL_TYPE_YEAR:
        return ft_myyear;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_NEWDATE:
        return ft_mydate;
    case MYSQL_TYPE_TIME:
        return ft_mytime;
    case MYSQL_TYPE_DATETIME:
        return ft_mydatetime;
    case MYSQL_TYPE_TIMESTAMP:
        return ft_mytimestamp;
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING: //?
        if (binary)
            return unicode ? ft_mywvarbinary : ft_myvarbinary;
        return unicode ? ft_mywvarchar : ft_myvarchar;
    case MYSQL_TYPE_STRING:
        if (binary)
            return unicode ? ft_wstring : ft_string;
        return unicode ? ft_mywchar : ft_mychar;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
        if (binary)
            return ft_myblob;
        return ft_mytext;
    case MYSQL_TYPE_GEOMETRY:
        return ft_mygeometry;
#if ((MYSQL_VERSION_ID > 50700) && (MYSQL_VERSION_ID < 100000))
    case MYSQL_TYPE_JSON:
        return ft_myjson;
#endif
#if(MYSQL_VERSION_ID > 50600)
    case MYSQL_TYPE_TIME2:
        return ft_mytime;
    case MYSQL_TYPE_DATETIME2:
        return ft_mydatetime;
    case MYSQL_TYPE_TIMESTAMP2:
        return ft_mytimestamp;
#endif
    default: // MYSQL_TYPE_NEWDECIMAL MYSQL_TYPE_GEOMETRY
        return unicode ? ft_wstring : ft_string;
    }
    return 0;
}

bool isUnicode(const CHARSET_INFO& cs)
{
    int index = charsetIndex(cs.csname);
    return (index == CHARSET_UTF16LE) || (index == CHARSET_USC2);
}

bool isBinary(const CHARSET_INFO& cs)
{
    return (&cs == &my_charset_bin);
}

tabledef* schemaBuilder::getTabledef(engine::mysql::table* src, int id, 
                        bool nouseNullkey, uchar* rec, size_t size)
{   
    
    src->restoreRecord();
    size_t tdSizelen =  (sizeof(tabledef) + (sizeof(fielddef) * src->fields()) +
                            (sizeof(keydef) * src->keys()));

    if (size < tdSizelen)
    {
        m_stat = STATUS_BUFFERTOOSMALL;
        return NULL;
    }
    // index
    keynumConvert kc(&src->keyDef(0), src->keys());

    uint_td datalen = 0;
    // table
    tabledef& td = (*((tabledef*)rec));
    initTableDef(src, td, id);
    td.fieldCount = (uchar_td)src->fields();
    td.keyCount = (uchar_td)src->keys();
    td.charsetIndex = charsetIndex(src->charset().csname);
    td.primaryKeyNum =
        (src->primarykeyNum() < td.keyCount) ? kc.clientKeynum(src->primarykeyNum()) : -1;
#ifdef USE_BTRV_VARIABLE_LEN
    td.flags.bit0 = (src->recordFormatType() & RF_FIXED_PLUS_VALIABLE_LEN);
    if (src->recordFormatType() & RF_FIXED_PLUS_VALIABLE_LEN)
        td.fixedRecordLen =
            (ushort_td)(src->recordLenCl() - src->lastVarFieldDataLen());
    else
#endif
        td.fixedRecordLen = (ushort_td)src->recordLenCl();
    td.m_maxRecordLen = (ushort_td)src->recordLenCl();
    td.optionFlags.bitA = ((src->recordFormatType() & RF_VALIABLE_LEN) != 0);
    td.optionFlags.bitB = (src->blobFields() != 0);
#if defined(MARIADB_BASE_VERSION)
    td.m_useInMariadb = true;
#else
    td.m_useInMariadb = false;
#endif
    td.m_srvMinorVer = (MYSQL_VERSION_ID / 100) % 100;
    datalen += sizeof(tabledef);
    td.fieldDefs = (fielddef*)(rec + datalen);
    // field
    ushort_td pos = 0;
    String str;
    for (int i = 0; i < src->fields(); ++i)
    {
        Field* f = src->field(i);
        if (isNisField(f->field_name))
            --td.fieldCount;
        else
        {
            fielddef fd;
            memset(&fd, 0, sizeof(fd));
            fd.setName(f->field_name);
            fd.len = f->pack_length();
            fd.pos = pos;
            fd.type = convFieldType(f->real_type(), f->flags,
                                    isBinary(*(f->charset())),
                                    isUnicode(*(f->charset())));
            if ((fd.type == ft_mytime) || (fd.type == ft_mydatetime) || (fd.type == ft_mytimestamp))
            {
                if (src->isLegacyTimeFormat(i))
                    fd.m_options |= FIELD_OPTION_REGACY_TIME;

                //for mariadb
                #if defined(MARIADB_BASE_VERSION)
                    if(td.isMariaTimeFormat())
                        fd.m_options |= FIELD_OPTION_MARIADB;
                #endif
            }

            if (fd.type == ft_mydecimal)
                fd.digits = my_decimal_length_to_precision(f->field_length, f->decimals(),
								(f->flags & UNSIGNED_FLAG) != 0);
 
            fd.setPadCharSettings(false, true);
            if (f->has_charset())
                fd.setCharsetIndex(charsetIndex(f->charset()->csname));
            else if (fd.type == ft_string || fd.type == ft_lstring
                || fd.type == ft_myvarbinary || fd.type == ft_myblob)
                fd.setCharsetIndex(m_binFdCharset);

            if ((fd.type == ft_mydatetime || fd.type == ft_mytimestamp) && (f->val_real() == 0))
            {// No constant value
                fd.setDefaultValue(0.0f);
                if (cp_has_insert_default_function(f)) 
                    fd.setDefaultValue(DFV_TIMESTAMP_DEFAULT);
            }
            else if (fd.isIntegerType())
                fd.setDefaultValue((__int64)f->val_int());
            else
            {
                f->val_str(&str, &str);
                fd.setDefaultValue(str.c_ptr());
            }
            fd.setNullable(f->null_bit != 0, f->is_null());
            if (fd.isNullable()) ++td.m_nullfields;
            if ((fd.type == ft_mydatetime || fd.type == ft_mytimestamp) && 
                                    cp_has_update_default_function(f))
                fd.setTimeStampOnUpdate(true); 

            fd.decimals = (uchar_td)f->decimals();
            if (fd.decimals == NOT_FIXED_DEC)
                fd.decimals = 0;

            if (td.isLegacyTimeFormat(fd))
            {
                fd.m_options |= FIELD_OPTION_REGACY_TIME;
                fd.decimals = 0;
            }
            else
                fd.m_options &= ~FIELD_OPTION_REGACY_TIME;
            pos += fd.len;
            datalen =
                copyToRecordImage(rec, &fd, sizeof(fielddef), datalen);
        }
    }
    td.m_nullbytes = (td.m_nullfields + 7) / 8;

    for (int i = 0; i < src->keys(); ++i)
    {
        const KEY& key = src->keyDef(kc.keyNumByMakeOrder(i));
        keydef kd;
        memset(&kd, 0, sizeof(keydef));
        kd.segmentCount = key.user_defined_key_parts;
        kd.keyNumber = i;
        int segNum = 0;
        bool allNullkey = true;
        for (int j = 0; j < (int)key.user_defined_key_parts; ++j)
        {
            keySegment& sg = kd.segments[segNum];
            KEY_PART_INFO& ks = key.key_part[j];
            if (isNisField(ks.field->field_name))
                --kd.segmentCount;
            else
            {
                sg.fieldNum = (uchar_td)ks.field->field_index;
                sg.flags.bit0 = !(key.flags & HA_NOSAME); // duplicate
                sg.flags.bit1 = 1; // change able
                sg.flags.bit4 = (j != kd.segmentCount - 1); // segment
                sg.flags.bit8 = 1; // extend key type
                if (nouseNullkey == false)
                    sg.flags.bit9 = ks.null_bit ? 1 : 0; // null key
                allNullkey = allNullkey & sg.flags.bit9;
                Field* f = src->field(sg.fieldNum);
                if (isStringTypeForIndex(convFieldType(f->type(),  f->flags, isBinary(*(f->charset())),
                        isUnicode(*(f->charset()))))) 
                     sg.flags.bitA = !(f->flags & BINARY_FLAG);// case in-sencitive      
                          
                if (f->pack_length() != ks.length + var_bytes_if(f))
                    td.fieldDefs[sg.fieldNum].keylen = ks.length ; // suppot prefix key
                ++segNum;
            }
        }
        if (allNullkey)
        {
            for (int i=0;i<kd.segmentCount; ++i)
            {
                kd.segments[segNum].flags.bit9 = 0; //part segment null key
                kd.segments[segNum].flags.bit3 = 1; //all segment null key
            }
        }
        datalen = copyToRecordImage(rec, &kd, sizeof(keydef), datalen);
    }
    td = (tabledef&)(*rec);
    td.varSize = datalen - 4;
    td.setFielddefsPtr();
    td.setKeydefsPtr();

    return &td;
}

tabledef* schemaBuilder::getTabledef(database* db, const char* tablename, uchar* rec, size_t size)
{
    table* tb = db->openTable(tablename, TD_OPEN_READONLY, NULL, "");
    if (db->stat()) 
    {
        m_stat = db->stat();
        return NULL;
    }
    tabledef* td = getTabledef(tb, 0, true, rec, size);
    db->closeTable(tb);
    return td;
}

short schemaBuilder::insertMetaRecord(table* mtb, table* src, int id, bool nouseNullkey)
{
    boost::shared_array<uchar> rec(new uchar[65000]);
    
    tabledef* td = getTabledef(src, id, nouseNullkey, rec.get(), 65000);
    mtb->clearBuffer();
    mtb->setRecordFromPacked(rec.get(), td->varSize + 4, NULL);
    mtb->insert(true);
    return mtb->stat();
}

bool isFrmFile(const std::string& name, bool notschema=true)
{
    size_t pos = name.find(TRANSACTD_SCHEMANAME);
    if (pos != std::string::npos && notschema)
        return false;
    // First of name is '#' that is alter table temp file.
    if (name.size() && name[0] == '#')
        return false;
    pos = name.find(".frm");
    if (pos != std::string::npos)
        return pos == name.size() - 4;
    return false;
}


int isView(database* db, const char *name)
{
	int ret = -1;
#ifndef NO_LOCK_OPEN
    safe_mysql_mutex_lock lck(&LOCK_open);
#endif
    TABLE_SHARE* s = cp_get_cached_table_share(db->thd(), db->name().c_str(), name);
    if (s)
    {
        ret = (int)s->is_view;
#ifdef NO_LOCK_OPEN
        cp_tdc_release_share(s);
#endif
    }
    return ret;
}

table* getTable(database* db, const char *name)
{
    table* tb = NULL;
    try
    {
        tb = db->openTable(name, TD_OPEN_READONLY, NULL, "");
    }
    catch(...){}
    return tb;
}

void schemaBuilder::listTable(database* db, std::vector<std::string>& tables, int type)
{
    char path[FN_REFLEN + 1];
    tables.clear();
    build_table_filename(path, sizeof(path) - 1, db->name().c_str(), "", "", 0);
    std::string s = path;
    fs::path p = s;
    if (exists(p) == false) return;

    fs::directory_iterator it(p);
    fs::directory_iterator end;
    

    for (fs::directory_iterator it(p); it != end; ++it)
    {
        if (!is_directory(*it))
        {
            std::string s = it->path().string(); // fullpath
            if (isFrmFile(it->path().filename().string(), false)) // filename
            {
                enum legacy_db_type not_used;
                
                frm_type_enum ftype = dd_frm_type(db->thd(), (char*)s.c_str(), &not_used);
                filename_to_tablename(it->path().stem().string().c_str(), path, FN_REFLEN);
                std::string tablename = path;
                
                if (((type & TABLE_TYPE_VIEW) && (ftype == FRMTYPE_VIEW)) ||
                    ((type & TABLE_TYPE_NORMAL_TABLE) && (ftype == FRMTYPE_TABLE)))
                    tables.push_back(tablename);  
                else if ((ftype == FRMTYPE_TABLE) && type == TABLE_TYPE_TD_SCHEMA)
                {
                    {
                        #ifndef NO_LOCK_OPEN
                        safe_mysql_mutex_lock lck(&LOCK_open);
                        #endif
                        TABLE_SHARE* s = cp_get_cached_table_share(db->thd(), db->name().c_str(), tablename.c_str());
                        if (s)
                        {
                            LEX_STRING* comment = &s->comment;
                            if (comment && (comment->length > 8) && strstr(comment->str,"%@%02.000"))
                            tables.push_back(tablename);
                            #ifdef NO_LOCK_OPEN
                            cp_tdc_release_share(s);
                            #endif
                            continue;
                        }
                    }
                    
                    {
                        table* tb = getTable(db, tablename.c_str()); 
                        if (!tb) break;
                        LEX_STRING* comment = &tb->internalTable()->s->comment; 
                        if (comment && (comment->length > 8) && strstr(comment->str,"%@%02.000"))
                        tables.push_back(tablename);    
                        if (tb) tb->close();
                    }
                }
            }
        }
    }
}

short schemaBuilder::execute(database* db, table* mtb, bool nouseNullkey)
{
    char path[FN_REFLEN + 1];
    build_table_filename(path, sizeof(path) - 1, db->name().c_str(), "", "", 0);

    std::string s = path;
    fs::path p = s;
    if (exists(p) == false) return STATUS_TABLE_NOTOPEN;
    fs::directory_iterator it(p);
    fs::directory_iterator end;
    int id = 0;
    short stat = 0;
    std::vector<table*> tables;
    
    for (fs::directory_iterator it(p); it != end; ++it)
    {
        if (!is_directory(*it))
        {
            std::string s = it->path().filename().string();
            if (isFrmFile(s))
            {
                enum legacy_db_type not_used;
                frm_type_enum ftype = dd_frm_type(db->thd(), (char*)it->path().string().c_str(), &not_used);
                if (ftype == FRMTYPE_TABLE)
                {
                    filename_to_tablename(it->path().stem().string().c_str(), path,
                                          FN_REFLEN);
                    table* tb = db->openTable(path, TD_OPEN_READONLY, NULL, "");
                    if (!tb) break;
                    tables.push_back(tb);
                }
            }
        }
    }
    if (db->stat()) return db->stat();

    {
        smartTransction trn(db);
        for (size_t i = 0; i < tables.size(); ++i)
        {
            if ((stat = insertMetaRecord(mtb, tables[i], ++id, nouseNullkey)) != 0)
                    break;
        }
        if (stat == 0) trn.end();
            
    }
    for (size_t i = 0; i < tables.size(); ++i)
        db->closeTable(tables[i]);
    return stat;
}

} // namespace mysql
} // namespace protocol
} // namespace db
} // namespace tdap
} // namespace bzs
