#ifndef BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
#define BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
 ================================================================= */

#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <string.h>
#include <algorithm>
#include <wchar.h>
#include <stdio.h>
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/blobStructs.h>
#include <assert.h>
#include <bzs/db/protocol/tdap/myDateTime.h>
namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

namespace mysql
{
class schemaBuilder;
}
namespace client
{
class dbdef;
class fielddefs;
class table;
class database;
struct openTablePrams;
struct dbdimple;
class filter;
class recordCache;
class fielddefs;
class fields;
class field;
class memoryRecord;
struct logic;
class recordsetQuery;
class sqlBuilder;
}

#pragma pack(push, 1)
pragma_pack1;

using std::min;
using std::max;

#ifdef SWIG

/* For swig interface
 Export names .
 */
union FLAGS
{
    unsigned short all;

    unsigned short bit0 : 1;
    unsigned short bit1 : 1;
    unsigned short bit2 : 1;
    unsigned short bit3 : 1;
    unsigned short bit4 : 1;
    unsigned short bit5 : 1;
    unsigned short bit6 : 1;
    unsigned short bit7 : 1;
    unsigned short bit8 : 1;
    unsigned short bit9 : 1;
    unsigned short bitA : 1;
    unsigned short bitB : 1;
    unsigned short bitC : 1;
    unsigned short bitD : 1;
    unsigned short bitE : 1;
    unsigned short bitF : 1;
};
#else // NOT SWIG

union FLAGS
{
    unsigned short all;

    struct
    {
        unsigned short bit0 : 1;
        unsigned short bit1 : 1;
        unsigned short bit2 : 1;
        unsigned short bit3 : 1;
        unsigned short bit4 : 1;
        unsigned short bit5 : 1;
        unsigned short bit6 : 1;
        unsigned short bit7 : 1;
        unsigned short bit8 : 1;
        unsigned short bit9 : 1;
        unsigned short bitA : 1;
        unsigned short bitB : 1;
        unsigned short bitC : 1;
        unsigned short bitD : 1;
        unsigned short bitE : 1;
        unsigned short bitF : 1;
    };
};
#endif // NOT SWIG

/* brief Key infomation for create table operation
 */
struct keySpec
{
    ushort_td keyPos; /* key position */
    ushort_td keyLen; /* key length */
    FLAGS keyFlag; /* key flag */
    uint_td keyCount; /* key count */
    uchar_td keyType; /* key type of extended */
    uchar_td nullValue; /* value of null */
    uchar_td reserve2[2]; /* reserved */
    uchar_td keyNo; /* fixed key number */
    uchar_td acsNo; /* no acs */
}; /* total        16 byte */

/* brief File infomation for create table operation
 */
struct fileSpec
{
    ushort_td recLen; /* record length */
    ushort_td pageSize; /* page sise */
    ushort_td indexCount; /* index count */
    uint_td recCount; /* record count for stat */
    FLAGS fileFlag; /* file flags */
    uchar_td reserve1[2]; /* reserved */
    ushort_td preAlloc; /* page allocation count */
    keySpec keySpecs[1]; /* key specs */
}; /* total  ? byte */

/* brief A key segment infomation
 */
struct keySegment
{
    uchar_td fieldNum; // Refarence field buymber
    FLAGS flags; // key flags. 11 to 15bit is not use.
};

/** keySegment::flags
*/
#define kf_duplicatable   bit0
#define kf_changeatable   bit1
#define kf_allseg_nullkey bit3
#define kf_order_desc     bit6
#define kf_extend         bit8
#define kf_seg_nullkey    bit9
#define kf_incase         bitA



/* brief A key infomation
 */
#define MAX_KEY_SEGMENT 8
#define COMP_KEY_FLAGS(l, r, NAME) (l.NAME == r.NAME)
struct keydef
{
    uchar_td segmentCount;
    keySegment segments[MAX_KEY_SEGMENT];
    uchar_td keyNumber;

    bool operator==(const keydef& r) const
    {
        if (this == &r) return true;
        bool ret = (segmentCount == r.segmentCount) && (keyNumber == r.keyNumber);
        if (!ret) return false;
        for (int i = 0;i < segmentCount; ++i)
        {
            FLAGS f = segments[i].flags;
            FLAGS rf = r.segments[i].flags;
            ret = COMP_KEY_FLAGS(f, rf, kf_duplicatable) &&
                  COMP_KEY_FLAGS(f, rf, kf_changeatable) &&
                  COMP_KEY_FLAGS(f, rf, kf_allseg_nullkey) &&
                  COMP_KEY_FLAGS(f, rf, kf_order_desc) &&
                  COMP_KEY_FLAGS(f, rf, kf_seg_nullkey) &&
                  COMP_KEY_FLAGS(f, rf, kf_incase);
            if (!ret) return false;
        }
        return true;
    }

private:
    short synchronize(const keydef* kd);
    friend struct tabledef;
    friend class client::dbdef;
};

static const int MYSQL_FDNAME_SIZE = 64;
static const int MYSQL_TBNAME_SIZE = 64;
static const int PERVASIVE_FDNAME_SIZE = 20;
static const int FIELD_NAME_SIZE = MYSQL_FDNAME_SIZE;
static const int TABLE_NAME_SIZE = 32;
static const int FILE_NAME_SIZE = 266;

/** @cond INTERNAL */
#if (defined(__x86_32__) || __APPLE_32__)
static const int TABLEDEF_FILLER_SIZE = 17; // 25-4-4;
#else
static const int TABLEDEF_FILLER_SIZE = 1; // 17-8 -8;
#endif
/** @endcond */

#ifndef MYSQL_DYNAMIC_PLUGIN

/* A field type name that specified by a type is returned.
 */
PACKAGE const _TCHAR* getTypeName(short type);

/* A field alignment that specified by a type is returned.
 */
PACKAGE int getTypeAlign(short type);

/* calcurate byts of char type by charctor number.
 */
PACKAGE ushort_td
    lenByCharnum(uchar_td type, uchar_td charsetIndex, ushort_td charnum);

#endif // MYSQL_DYNAMIC_PLUGIN

/* Is field type string ? */
inline bool isStringTypeForIndex(uchar_td type)
{
    switch (type)
    {
    case ft_string:
    case ft_zstring:
    case ft_wstring:
    case ft_wzstring:
    case ft_mychar:
    case ft_mywchar:
    case ft_myvarchar:
    case ft_myvarbinary:
    case ft_mywvarchar:
    case ft_mywvarbinary:
        return true;
    }
    return false;
}

inline bool isStringType(uchar_td type)
{
    switch (type)
    {
    case ft_mygeometry:
    case ft_myjson:
    case ft_myblob:
    case ft_mytext:
    case ft_lstring:
    case ft_note:
        return true;
    }
    return isStringTypeForIndex(type);
}

/** @cond INTERNAL */
#define PAD_CHAR_OPTION_SAVED   1
#define USE_PAD_CHAR            2
#define TRIM_PAD_CHAR           4

#define FIELD_OPTION_NULLABLE     1
#define FIELD_OPTION_MARIADB      2
#define FIELD_OPTION_REGACY_TIME  4

#define DEFAULT_VALUE_SIZE      8

//For decimals
#define DIGITS_INT32 9
static const int decimalBytesBySurplus[DIGITS_INT32 + 1] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4};

/** @endcond */

class bitset
{
    unsigned __int64 m_i64;

public:
    bitset() : m_i64(0) { }

    bitset(__int64 v) : m_i64(v) { }

    inline void set(int index, bool value)
    {
        unsigned __int64 bits = 1ULL << index;
        m_i64 = value ? m_i64 | bits : m_i64 & ~bits;
    }

    inline bool get(int index) const
    {
        unsigned __int64 bits = 1ULL << index;
        return (m_i64 & bits) != 0;
    }

    inline __int64 internalValue() const { return (__int64)m_i64; }

    inline bool operator[](int index) const {return get(index); };

    inline bool operator==(const bitset& r) const
    {
        return (m_i64 == r.m_i64);
    }

    inline bool contains(const bitset& r, bool all=true) const
    {
        return all ? ((m_i64 & r.m_i64) == r.m_i64) : ((m_i64 & r.m_i64) != 0);
    }

};

/* Mark of ** that BizStation Corp internal use only.
 */
template <int N> struct fielddef_t
{
protected:
    char m_name[N];

public:
    uchar_td type; // type (zstring integer)
    ushort_td len; // length
    uchar_td decimals; // ** decimals
    char viewNum; // ** An order of a list view column
    ushort_td viewWidth; // ** view width pix
    double max; // ** max value
    double min; // ** min value
protected:
    char m_defValue[DEFAULT_VALUE_SIZE];
public:
    uchar_td lookTable; // ** reference table number
    uchar_td lookField; // ** field number of reference table
    uchar_td lookFields[3]; // ** View fields of reference    bit567
    ushort_td pos; // Field offset position from record image
protected:
    uchar_td m_nullbit;    // bit number for null indicator
    uchar_td m_nullbytes;  // byte of null indicator which head of record memory block.
    char m_chainChar[2];

public:
union 
{
    ushort_td ddfid;  // ddf field id
    ushort_td digits; // for ft_myDecimal
};
    ushort_td filterId; // ** filter id for reference
    uchar_td filterKeynum; // ** key number for reference
    uchar_td nullValue; // null value for P.SQL 
    ushort_td userOption; // ** option
    uchar_td lookDBNum; // ** database number of reference bitD
    
    /** The length of the mysql part key
    
       If this field is used by two or more keys, and both this length is used. 
    */
    ushort_td keylen; 

protected:
    uchar_td m_charsetIndex; // charctor set index of this field data
    ushort_td m_schemaCodePage;
    uchar_td m_padCharOptions;
    uchar_td m_options;

public:
    FLAGS enableFlags; // ** enable flags. see below

private:
    inline void setSchemaCodePage(uint_td v) { m_schemaCodePage = (ushort_td)v; };
    friend class client::dbdef;
    friend class client::fielddefs;
    friend struct tabledef;
};

/* This is only for BizStation Corp internal.
 enableFlags
 bit0  show list view
 bit1  enable max value
 bit2  enable min value
 bit3  reserved. not use
 bit4  enable lookTable
 bit5  enable lookFields[0]
 bit6  enable lookFields[1]
 bit7  enable lookFields[2]
 bit8  It append to a front column.
 bit9  enable reference filter
 bitA  call field name rename function.
 bitB  disbale export this field data.
 bitC  not show list view but add select field list
 bitD  enable lookDBNum
 bitE  field value is changed
 bitF  defaultNull
 */

typedef fielddef_t<MYSQL_FDNAME_SIZE> fielddef_t_my;
typedef fielddef_t<PERVASIVE_FDNAME_SIZE> fielddef_t_pv;

#ifdef SWIG
%template(fielddef_t_my) fielddef_t<MYSQL_FDNAME_SIZE>;
#endif // SWIG

struct PACKAGE fielddef : public fielddef_t_my
{

#ifdef _UNICODE
    const wchar_t* name() const; // Return a field name.
    const wchar_t* name(wchar_t* buf) const; // Return a field name to bufffer .
    const wchar_t* chainChar() const; // ** internal use only.
    const wchar_t* defaultValue_str() const;
    void setName(const wchar_t* s);
    void setChainChar(const wchar_t* s); // ** internal use only.
    void setDefaultValue(const wchar_t* s);
#else // NOT _UNICODE

#ifdef MYSQL_DYNAMIC_PLUGIN

    //inline const char* name() const { return m_name; };

    //inline const char* chainChar() const { return m_chainChar; };

    //inline const char* defaultValue_str() const{return m_defValue; }

    inline void setName(const char* s)
    {
        strncpy_s(m_name, FIELD_NAME_SIZE, s, sizeof(m_name) - 1);
    }

    inline void setChainChar(const char* s)
    {
        strncpy_s(m_chainChar, 2, s, sizeof(m_chainChar) - 1);
    }

    inline void setDefaultValue(const char* s)
    {
        if (isBlob())
        {
            memset(m_defValue, 0, DEFAULT_VALUE_SIZE);
            return;
        }

        enableFlags.bitF = false;
        __int64 i64 = 0;
        switch(type)
        {
        case ft_time:
        case ft_mytime:
        {
            myDateTime dt(7, true);
            dt.setTime(s);
            i64 = dt.getValue();
            memcpy(m_defValue, &i64, 7);
            return;
        }
        case ft_date:
        case ft_mydate:
        case ft_datetime:
        case ft_mytimestamp:
        case ft_mydatetime:
            i64 = str_to_64<myDateTime, char>(7, true, s);
            memcpy(m_defValue, &i64, 7);
            return;
        case ft_integer:
        case ft_autoinc:
            *((__int64*)m_defValue) = _atoi64(s);
            return;
        case ft_uinteger:
        case ft_logical:
        case ft_set:
        case ft_bit:
        case ft_enum:
        case ft_autoIncUnsigned:
        case ft_myyear:
            *((unsigned __int64*)m_defValue) = strtoull(s, NULL, 10);
            return;
        }
    
        if (isNumericType())
        {
            *((double*)m_defValue) = atof(s);
            return;
        }
        strncpy_s(m_defValue, 8, s, sizeof(m_defValue) - 1);
    }

    inline void setDefaultValue(double v)
    {
        *((double*)m_defValue) = v;
    }

    inline void setDefaultValue(__int64 v)
    {
        *((__int64*)m_defValue) = v;
    }


#else // NOT MYSQL_DYNAMIC_PLUGIN
    const char* name() const;
    const char* name(char* buf) const;
    const char* chainChar() const;
    const char* defaultValue_str() const;
    void setName(const char* s);
    void setChainChar(const char* s);
#endif // NOT MYSQL_DYNAMIC_PLUGIN
#endif // NOT _UNICODE

    inline const char* nameA() const { return m_name; };

    inline void setNameA(const char* s)
    {
        strncpy_s(m_name, FIELD_NAME_SIZE, s, sizeof(m_name) - 1);
    }

#ifndef MYSQL_DYNAMIC_PLUGIN
    void setDefaultValue(const char* s);

    void setDefaultValue(__int64 v);

    void setDefaultValue(double v);

    void setDefaultValue(const bitset& v)
    {
        setDefaultValue(v.internalValue());
    }

    double defaultValue() const;

    __int64 defaultValue64() const;

    inline const _TCHAR* typeName() const { return getTypeName(type); };

    inline int align() const { return getTypeAlign(type); };

    inline void setLenByCharnum(ushort_td charnum)
    {
        len = lenByCharnum(type, m_charsetIndex, charnum);
    }

    void setDecimalDigits(int dig, int dec);
#endif // MYSQL_DYNAMIC_PLUGIN

    inline unsigned int codePage() const
    {
        return mysql::codePage((unsigned short)m_charsetIndex);
    }

    /* Is string type or not.
    */
    inline bool isStringType() const {return tdap::isStringType(type);}

    inline bool isPadCharType() const
    {
        return ((type == ft_mychar) || (type == ft_mywchar) ||
                (type == ft_string) || (type == ft_wstring));
    }

    inline bool isIntegerType() const
    {
        return ((type == ft_integer) || (type == ft_logical) || (type == ft_uinteger) || 
                (type == ft_autoinc) || (type == ft_set) || (type == ft_bit) || 
                (type == ft_enum) || (type == ft_autoIncUnsigned) || (type == ft_myyear));
    }

    inline bool isNumericType() const
    {
        return ((type == ft_integer) || (type == ft_decimal) ||
                (type == ft_money) || (type == ft_logical) || (type == ft_currency) ||
                (type == ft_numeric) || (type == ft_bfloat) || (type == ft_float) ||
                (type == ft_uinteger) || (type == ft_autoinc) || (type == ft_set) ||
                (type == ft_bit) || (type == ft_enum) || (type == ft_numericsts) ||
                (type == ft_numericsa) || (type == ft_autoIncUnsigned) ||
                (type == ft_myyear) || (type == ft_mydecimal));
    }

    inline bool isDateTimeType() const
    {
        return ((type == ft_date) || (type == ft_mydate) ||
                (type == ft_time) || (type == ft_mytime) ||
                (type == ft_datetime) || (type == ft_timestamp) ||
                (type == ft_mydatetime) || (type == ft_mytimestamp));
    }

    /* Charctor numbers from charset.
     */
    unsigned int charNum() const;

    bool isValidCharNum() const;

    inline void setCharsetIndex(uchar_td index)
    {
        m_charsetIndex = index;
        if ((type == ft_wstring) || (type == ft_wzstring) ||
            (type == ft_mywvarchar) || (type == ft_mywvarbinary) ||
            (type == ft_mywchar))
            m_charsetIndex = CHARSET_UTF16LE;
    }

    inline uchar_td charsetIndex() const { return m_charsetIndex; };
    
    inline bool isBlob() const
    {
        return (type == ft_myblob) || (type == ft_mytext) || (type == ft_mygeometry) || (type == ft_myjson);
    }

    inline void setPadCharSettings(bool set, bool trim)
    {
        m_padCharOptions = 0;
        m_padCharOptions |= PAD_CHAR_OPTION_SAVED;
        if ((type == ft_mychar) || (type == ft_mywchar))
        {
            m_padCharOptions |= USE_PAD_CHAR;
            if (trim)
                m_padCharOptions |= TRIM_PAD_CHAR;
        }    // For compatibility with conventional.
        else if ((type == ft_string) || (type == ft_wstring))
        {
             if (set)
                m_padCharOptions |= USE_PAD_CHAR;
             if (trim)
                m_padCharOptions |= TRIM_PAD_CHAR;
        }
    }
    
    /* When ft_string or ft_wstring, fill by pad char at write. */
    inline bool isUsePadChar() const {return (m_padCharOptions & USE_PAD_CHAR) == USE_PAD_CHAR;}

    /* When ft_string or ft_wstring or ft_mychar or  ft_mywchar,
       remove pad char at read.*/
    inline bool isTrimPadChar() const {return (m_padCharOptions & TRIM_PAD_CHAR) == TRIM_PAD_CHAR;}

    inline bool isNullable() const {return (m_options & FIELD_OPTION_NULLABLE) == FIELD_OPTION_NULLABLE;}

    void setNullable(bool v, bool defaultNull = true)
    {
        if (v)
        {
            m_options |= FIELD_OPTION_NULLABLE;
            enableFlags.bitF = defaultNull;
        }
        else
        {
            m_options &= ~FIELD_OPTION_NULLABLE;
            enableFlags.bitF = false;
        }
    }

    void setTimeStampOnUpdate(bool v)
    {
        if (type == ft_mytimestamp || type == ft_mydatetime)
            m_defValue[7] =  v ? 1: 0;
    }

    bool isTimeStampOnUpdate() const 
    { 
        if (type == ft_mytimestamp || type == ft_mydatetime)
            return (m_defValue[7] == 1);
        return false;
    }

    inline bool isDefaultNull() const
    {
        return enableFlags.bitF;
    }

    inline bool isLegacyTimeFormat() const
    {
        return (m_options & FIELD_OPTION_REGACY_TIME) != 0;
    }

    inline uint_td blobLenBytes() const
    {
        if (isBlob())
            return len - 8;
        return 0;
    }

    /* length bytes of var field
     */
    inline uint_td varLenBytes() const
    {
        if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) ||
            type == ft_lstring)
            return len < 256 ? 1 : 2;
        else if (type == ft_lvar)
            return 2;
        return 0;
    }

    bool operator==(const fielddef& r) const;

private:
    const char* defaultValue_strA(char* p, size_t size) const;
    /* data length
     */
    inline uint_td dataLen(const uchar_td* ptr) const
    {
        int blen = varLenBytes();
        if (blen == 0)
            return len;
        else if (blen == 1)
            return *((unsigned char*)ptr);
        return *((unsigned short*)ptr);
    }

    inline uint_td blobDataLen(const uchar_td* ptr) const
    {
        int blen = blobLenBytes();
        if (blen == 0)
            return len;
        uint_td v = 0;
        memcpy(&v, ptr, blen);
        return v;
    }

    inline int maxVarDatalen() const
    {
        if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) ||
            type == ft_lstring)
            return (len < 256) ? len - 1 : len - 2;
        else if (type == ft_lvar)
            return len - 4;
        else if (isBlob())
        {
            switch (len - 8)
            {
            case 1:
                return 0xFF;
            case 2:
                return 0xFFFF;
            case 3:
                return 0xFFFFFF;
            case 4:
                return 0xFFFFFFFF;
            }
            return 0;
        }
        return 0;
    }

    /* data image for key
     * param ptr address of record buffer
     */
    inline const uchar_td* keyData(const uchar_td* ptr) const
    {
        if (isBlob())
            return blobDataPtr(ptr);
        int sizeByte = varLenBytes();
        return ptr + sizeByte;
    }

    inline uint_td keyDataLen(const uchar_td* ptr) const
    {
        if (isBlob())
            return blobDataLen(ptr);
        return dataLen(ptr);
    }

    /* Is variable key type
     */
    inline bool isKeyVarType() const
    {
        return (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || isBlob());
    }

    /* max key segment length. not include sizeBytes.
     */
    inline ushort_td maxKeylen() const { return keylen ? keylen : len; };

    /* copy key data for send to mysql and btrv
     *  return next copy address.
     *  If datalen==0xff then From is field formated (string) type.
     *  If datalen!=0xff then From is none field formated (string) type.
     */
    inline uchar_td* keyCopy(uchar_td* to, const uchar_td* from, ushort_td datalen,
                     bool isNull)
    {
        ushort_td keylen = maxKeylen(); // size of max key segmnet for mysql
        ushort_td keyVarlen = varLenByteForKey(); // size of var sizeByte for record.
        ushort_td copylen = std::min<ushort_td>(keylen, datalen);

        memset(to, 0x00, keylen + 1); //clear plus null byte
        if (isNullable())
        {
            // mysql only
            if (isNull)
            {
                *to = 1;
                return to + 1 + keylen - keyVarlen;
            }else
                ++to;
        }
        if(!isNull)
        {
            if (keyVarlen)
            {
                if (datalen==0xff)
                    copylen = (ushort_td)std::min<uint_td>((uint_td)copylen,
                                                       keyDataLen(from));
                // Var size is allways 2byte for key.
                memcpy(to, &copylen, 2);
                to += 2;
                if (datalen==0xff)
                    from = keyData(from);
            }
            memcpy(to, from, copylen);
        }
        return to + keylen - keyVarlen;// incremnt 2 +  (store_len - varlen)
    }

    inline const uchar_td* blobDataPtr(const uchar_td* ptr) const
    {
        int blen = blobLenBytes();
        if (blen == 0)
            return NULL;
        const uchar_td** p = (const uchar_td**)(ptr + blen);
        return *p;
    }

    inline uint_td unPackCopy(uchar_td* dest, const uchar_td* src) const
    {
        int clen = varLenBytes();
        if (clen == 0)
            clen = len;
        else if (clen == 1)
            clen += *((unsigned char*)src);
        else
            clen += *((unsigned short*)src);
        memcpy(dest, src, clen);
        return clen;
    }

    inline ushort_td varLenByteForKey() const
    {
        if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) ||
            (type == ft_lstring))
            return len < 256 ? 1 : 2;
        else if (isBlob())
            return len - 8;
        return 0;
    }

    /* Get keyValue from keybuf for seek mode error description
     *  return next copy key address.
     */
    inline const uchar_td* getKeyValueFromKeybuf(const uchar_td* from,
                                                 const uchar_td** data,
                                                 ushort_td& size)
    {
        ushort_td keyVarlen =
            varLenByteForKey(); // size of var sizeByte for record.
        if (keyVarlen)
        {
            size = *((ushort_td*)from);
            *data = from + 2;
        }
        else
        {
            size = maxKeylen();
            *data = from;
        }
        return *data + size;
    }

    /* copy blob data for recordset 

      @param blobBlock copy to adddres
      @return new copy to address
        
    */
    inline  unsigned char* setBlobFieldPointer(uchar_td* dest, const blobHeader* hd,
                                    unsigned char* blobBlock, int fieldNum) const
    {
        assert(hd->curRow < hd->rows);
        const blobField* f = hd->nextField;
        int sizeByte = blobLenBytes();
        unsigned int size = f->size;
        //Copy data size
        memcpy(dest, &size, sizeByte);
        //Copy data 
        memcpy(blobBlock, f->data(), size);
        //Copy data ptr
        memcpy(dest + sizeByte, &blobBlock, sizeof(char*));
        hd->nextField = (blobField*)f->next();
        if (fieldNum == hd->fieldCount - 1)
            ++hd->curRow;
        return blobBlock + size;
    }

    inline void setPadCharDefaultSettings()
    {
        if (!isPadCharOptionSaved())
        {
            // For compatibility with conventional.
            if ((type == ft_string) || (type == ft_wstring) ||
                (type == ft_mychar) || (type == ft_mywchar))
            {
                m_padCharOptions |= USE_PAD_CHAR;
                m_padCharOptions |= TRIM_PAD_CHAR;
            }
        }
        else if ((type == ft_mychar) || (type == ft_mywchar))
            m_padCharOptions |= USE_PAD_CHAR;
    }

    /* PadChar options are saved at schema.
        This is for compatibility with conventional.*/
    bool isPadCharOptionSaved() const
    {
        return (m_padCharOptions & PAD_CHAR_OPTION_SAVED) == PAD_CHAR_OPTION_SAVED;
    }
#ifdef SP_SCOPE_FIELD_TEST
public:
#endif
    inline uchar_td nullbit() const {return m_nullbit;} // bit number for null indicator
    inline uchar_td nullbytes() const {return m_nullbytes;} // byte of null indicator which head of record memory block.
    inline void setOptions(uchar_td v) {m_options = v;}
private:

    /** Length of compare
     * if part of string or zstring then return strlen * sizeof(char or wchar).
     */
    inline uint_td compDataLen(const uchar_td* ptr, bool part) const
    {
        uint_td length = keyDataLen(ptr);
        if (part)
        {
            if ((type == ft_string) || (type == ft_zstring) ||
                            (type == ft_note) || (type == ft_mychar))
                length = (uint_td)strlen((const char*)ptr);
            else if ((type == ft_wstring) || (type == ft_wzstring) ||
                            (type == ft_mywchar))
                length = (uint_td)strlen16((char16_t*)ptr)*sizeof(char16_t);
        }
        return length;
    }
    
    short synchronize(const fielddef* td);

    void fixCharnum_bug();

    friend class client::database;
    friend class client::dbdef;
    friend class client::field;
    friend class client::fields;
    friend class client::recordCache;
    friend class client::fielddefs;
    friend class client::memoryRecord;
    friend class client::filter;
    friend class client::table;
    friend struct client::logic;
    friend class client::recordsetQuery;
    friend class client::sqlBuilder;
    friend class mysql::schemaBuilder;
    friend struct tabledef;

/** @cond INTERNAL */
    friend uint_td dataLen(const fielddef& fd, const uchar_td* ptr);
    friend uint_td blobDataLen(const fielddef& fd, const uchar_td* ptr);
    friend uint_td blobLenBytes(const fielddef& fd);
/** @endcond */
};

/** @cond INTERNAL */
inline void updateTimeStampStr(const fielddef* fd, char* p, size_t size)
{
    if (fd->isLegacyTimeFormat())
        sprintf_s(p, 64, " ON UPDATE CURRENT_TIMESTAMP");
    else
        sprintf_s(p, 64, " ON UPDATE CURRENT_TIMESTAMP(%d)", fd->decimals);
}

#ifdef _WIN32
inline void updateTimeStampStr(const fielddef* fd, wchar_t* p, size_t size)
{
    if (fd->isLegacyTimeFormat())
        swprintf_s(p, 64, L" ON UPDATE CURRENT_TIMESTAMP");
    else
    {
        int dec = (fd->type == ft_mytimestamp) ? (fd->len - 4) * 2 : (fd->len - 5) * 2;
        swprintf_s(p, 64, L" ON UPDATE CURRENT_TIMESTAMP(%d)",dec);
    }
}
#endif

inline uint_td dataLen(const fielddef& fd, const uchar_td* ptr)
{
    return fd.dataLen(ptr);
}

inline uint_td blobDataLen(const fielddef& fd, const uchar_td* ptr)
{
    return fd.blobDataLen(ptr);
}

inline uint_td blobLenBytes(const fielddef& fd)
{
    return fd.blobLenBytes();
}

inline _TCHAR* timeStampDefaultStr(const fielddef& fd, _TCHAR* buf, size_t bufsize)
{
    if (fd.type == ft_mytimestamp || fd.type == ft_mydatetime)
    {
        buf[0] = 0x00;
        if (fd.isLegacyTimeFormat())
            _stprintf_s(buf, bufsize, _T("CURRENT_TIMESTAMP"));
        else
            _stprintf_s(buf, bufsize, _T("CURRENT_TIMESTAMP(%d)"), fd.decimals);
    }
    return buf;
}


/** @endcond */

#define FORMAT_VERSON_BTRV_DEF 0
#define FORMAT_VERSON_CURRENT 1


/* Mark of ** that BizStation Corp internal use only.
 */
struct PACKAGE tabledef
{
    friend class client::dbdef; // for formatVersion
    friend class client::table; // for inUse
    friend class client::database; // for m_mysqlNullMode
    friend struct client::openTablePrams;
    friend struct client::dbdimple;
    friend class client::filter;
    friend class client::recordCache;
    friend class client::fielddefs;
    friend class client::sqlBuilder;
    friend class mysql::schemaBuilder;


    tabledef() 
    {
        cleanup(); 
    }
    void cleanup()
    {
        memset(this, 0, sizeof(tabledef));
        formatVersion = FORMAT_VERSON_CURRENT;
        primaryKeyNum = -1;
        parentKeyNum = -1;
        replicaKeyNum = -1;
        pageSize = 2048;
        schemaCodePage = 65001;//CP_UTF8

        // set temp server version
        m_useInMariadb = true;
    }

#ifdef _UNICODE
    const wchar_t* fileName() const; // file name
    const wchar_t* tableName() const; // table name
    void setFileName(const wchar_t* s);
    void setTableName(const wchar_t* s);
private:
    const char* toChar(char* buf, const wchar_t* s, int size) const;
#else
#ifdef MYSQL_DYNAMIC_PLUGIN

    const char* fileName() const { return m_fileName; };

    const char* tableName() const { return m_tableName; };

    inline void setFileName(const char* s) { setFileNameA(s); };

    inline void setTableName(const char* s) { setTableNameA(s); };
private:
    inline const char* toChar(char* buf, const char* s, int size) const
    {
        strncpy_s(buf, size, s, size - 1);
        return buf;
    };
#else
    const char* fileName() const;
    const char* tableName() const;
    void setFileName(const char* s);
    void setTableName(const char* s);
private:
    const char* toChar(char* buf, const char* s, int size) const;
#endif // MYSQL_DYNAMIC_PLUGIN
#endif
public:
    const char* fileNameA() const { return m_fileName; };

    const char* tableNameA() const { return m_tableName; };

    inline void setFileNameA(const char* s)
    {
        strncpy_s(m_fileName, FILE_NAME_SIZE, s, FILE_NAME_SIZE - 1);
    }

    inline void setTableNameA(const char* s)
    {
        strncpy_s(m_tableName, TABLE_NAME_SIZE, s, sizeof(m_tableName) - 1);
    }

    inline uchar_td nullfields() const { return m_nullfields;}

    inline uchar_td nullbytes() const { return m_nullbytes; }

    inline uchar_td inUse() const { return m_inUse; }

    int size() const;
    short fieldNumByName(const _TCHAR* name) const;

    inline ushort_td recordlen() const { return m_maxRecordLen; }

    uint_td unPack(char* ptr, size_t size) const;

    inline void setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
    {
        m_useInMariadb = isMariadb;
        m_srvMinorVer = srvMinorVersion;
    }

    inline bool isMysqlNullMode() const { return m_mysqlNullMode; }

    inline bool isLegacyTimeFormat(const fielddef& fd) const
    {
        if (fd.type == ft_mytime || fd.type == ft_mydatetime ||
                            fd.type == ft_mytimestamp)
        {
            if (m_useInMariadb)
            {
                if (fd.decimals == 0 && (m_srvMinorVer == 5 || m_srvMinorVer == 0)) return true;
            }
            else
            {
                if (m_srvMinorVer < 6) return true;
            }
        }
        return false;
    }
    bool operator==(const tabledef& r) const;

private:
    short synchronize(const tabledef* td);
    bool isNullKey(const keydef& key) const;
    uint_td pack(char* ptr, size_t size) const;
    short findKeynumByFieldNum(short fieldNum) const;
    inline ushort_td recordlenServer() const
    {
        if (optionFlags.bitC) return m_maxRecordLen + 2;
        return m_maxRecordLen;
    }
    bool isNeedNis(const keydef& key) const;
    bool isNULLFieldFirstKeySegField(const keydef& key) const;
    bool isNullValueZeroAll(const keydef& key) const;
    void setMysqlNullMode(bool v);
    void calcReclordlen(bool force= false);
    inline keydef* setKeydefsPtr()
    {
        return keyDefs = (keydef*)((char*)this + sizeof(tabledef) +
                         (fieldCount * sizeof(fielddef)));
    }

    inline fielddef* setFielddefsPtr()
    {
        return fieldDefs = (fielddef*)((char*)this + sizeof(tabledef));
    }

    inline bool isMariaTimeFormat() const
    {
        return (m_useInMariadb && (m_srvMajorVer == 5 || m_srvMinorVer == 0));
    }

public:
    ushort_td id; // table id

#ifdef SWIG
    /* For swig interface
     export field names.
     */
    ushort_td pageSize; // page size
    ushort_td varSize; // second field length
#else

    union
    {
        ushort_td pageSize; // page size
        ushort_td varSize; // second field length
    };
#endif

    ushort_td preAlloc; // pre alloc page seize
    ushort_td fieldCount; // Number of field
    uchar_td keyCount; // Number of key

private:
    char m_fileName[FILE_NAME_SIZE];
    char m_tableName[TABLE_NAME_SIZE];

public:
    short version;            // table version
    uchar_td charsetIndex;    // SCHARSET_INFO vector index;
private:
    uchar_td m_nullfields;    // number of nullable field
    uchar_td m_nullbytes;     // number of null indicator byte
    uchar_td m_inUse;
    bool m_mysqlNullMode ;    // use in mysqlnull mode
    bool m_useInMariadb  ;    // use in mariadb
    uchar_td m_srvMajorVer;   // server major version;
    uchar_td m_srvMinorVer;   // server minor version;
    uchar_td m_filler0[10];   // reserved
public:
    FLAGS flags; // file flags
    uchar_td primaryKeyNum; // Primary key number. -1 is no primary.
    uchar_td parentKeyNum; // ** Key number for listview. -1 is no use.
    uchar_td replicaKeyNum; // ** Key number for repdata. -1 is no use.
    FLAGS optionFlags; // ** optional flags
    ushort_td convertFileNum; // ** not use
private:
    ushort_td m_maxRecordLen; // max record length of var size table.
public:
    uchar_td treeIndex; // ** View index for listview.
    uchar_td iconIndex; // ** Icon index for listview.
    ushort_td ddfid;
    ushort_td fixedRecordLen; // Fixed record length for var size table.
    int autoIncExSpace;
    uchar_td iconIndex2;
    uchar_td iconIndex3;
    uint_td schemaCodePage; // Code page of this schema string data.

private:
    char formatVersion;
    client::dbdef* parent;
    void* defaultImage;
    uchar_td reserved[TABLEDEF_FILLER_SIZE]; // reserved
public:
    fielddef* fieldDefs; // Pointer cahche of first field.
    keydef* keyDefs; // Pointer cahche of first key.
};

// sizeof(fielddef) * 255 + sizeof(keydef) * 64 + sizeof(tabledef)
// (124 * 255) + (26 * 64) + 388 = 31620 + 1664 + 388 = 33672 
#define MAX_SCHEMASIZE  33672 

/* optionFlags   BizStation internal use only.
 Bit0 send windows messege.
 Bit1 show tree view.
 Bit2 is master table or not
 Bit3 is replicate or not
 Bit4 is backup target or not.
 Bit5 is encript or not
 Bit6 is this table destination of convert .
 Bit7 is support short cut in listveiw.
 Bit8 is call validate function at delete record.
 Bit9 Can export this table.
 BitA is this table include valiable fields (varchar or varbinary is used)
 BitB is this table include blob field (Blob is used)
 BitC is this table include fixedbinary
 */

struct PACKAGE btrVersion
{
    ushort_td majorVersion;
    ushort_td minorVersion;
    unsigned char type;

    const _TCHAR* moduleVersionShortString(_TCHAR* buf);
    const _TCHAR* moduleTypeString();

    inline bool isSupportDateTimeTimeStamp() const
    {
        if (majorVersion >= 10) return true;
        if ((majorVersion == 5) && (minorVersion > 5)) return true;
        return false;
    }

    inline bool isSupportMultiTimeStamp() const
    {
        return isSupportDateTimeTimeStamp();
    }

    inline bool isMariaDB() const { return type == MYSQL_TYPE_MARIA; }

    inline bool isMysql56TimeFormat() const 
    {
        if ((majorVersion == 10) && (minorVersion >= 1)) return true;
        if ((majorVersion == 5) && (minorVersion >= 6)) return true;
        return false;
    }

    inline bool isFullLegacyTimeFormat() const 
    {
        return !isMariaDB() && (minorVersion < 6);
    }

};

struct btrVersions
{
    btrVersion versions[4];
};

#define VER_IDX_CLINET    0
#define VER_IDX_DB_SERVER 1
#define VER_IDX_PLUGIN    2




#pragma pack(pop)
pragma_pop;


PACKAGE uchar_td getFilterLogicTypeCode(const _TCHAR* cmpstr);

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
