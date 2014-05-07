#ifndef	BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
#define	BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
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
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

namespace client{class dbdef;}

#pragma option -a1
pragma_pack1

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
#else
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
#endif
/* brief Key infomation for create table operation
 */
struct keySpec
{
    ushort_td keyPos;     /* key position */
    ushort_td keyLen;     /* key length */
    FLAGS keyFlag;        /* key flag */
    uint_td keyCount;     /* key count */
    uchar_td keyType;     /* key type of extended */
    uchar_td nullValue;   /* value of null */
    uchar_td reserve2[2]; /* reserved */
    uchar_td keyNo;       /* fixed key number */
    uchar_td acsNo;       /* no acs */
};                        /* total        16 byte */

/* brief File infomation for create table operation
 */
struct fileSpec
{
    ushort_td recLen;     /* record length */
    ushort_td pageSize;   /* page sise */
    ushort_td indexCount; /* index count */
    uint_td recCount;     /* record count for stat */
    FLAGS fileFlag;       /* file flags */
    uchar_td reserve1[2]; /* reserved */
    ushort_td preAlloc;   /* page allocation count */
    keySpec keySpecs[1];  /* key specs */
};                        /* total  ? byte */

/* brief A key segment infomation
 */
struct keySegment
{
    uchar_td fieldNum;      // Refarence field buymber
    FLAGS flags;            // key flags. 11 to 15bit is not use.

};

/* brief A key infomation
 */
#define MAX_KEY_SEGMENT 8
struct keydef
{
    uchar_td segmentCount;					// Number of segment
    keySegment segments[MAX_KEY_SEGMENT];   // key segments . max 8 segments
    uchar_td keyNumber;						// key number
};
// 26byte

static const int MYSQL_FDNAME_SIZE = 64;
static const int MYSQL_TBNAME_SIZE = 64;
static const int PERVASIVE_FDNAME_SIZE = 20;
static const int FIELD_NAME_SIZE = MYSQL_FDNAME_SIZE;
static const int TABLE_NAME_SIZE = 32;
static const int FILE_NAME_SIZE = 266;


#ifdef __x86_32__
static const int TABLEDEF_FILLER_SIZE = 21;//25-4;
#else
static const int TABLEDEF_FILLER_SIZE = 9;//17-8;
#endif

#ifndef MYSQL_DYNAMIC_PLUGIN

/* A field type name that specified by a type is returned.
 */
PACKAGE const _TCHAR* getTypeName(short type);

/* A field alignment that specified by a type is returned.
*/
PACKAGE int getTypeAlign(short type);

/*  calcurate byts of char type by charctor number.
*/
PACKAGE ushort_td lenByCharnum(uchar_td type, uchar_td charsetIndex
                                                    , ushort_td charnum);

#endif

/* Is field type string ?*/
PACKAGE bool isStringType(uchar_td type);

/* Mark of ** that BizStation Corp internal use only.
 */
template <int N>
struct fielddef_t
{
protected:
    char m_name[N];

public:
    uchar_td type;          // type (zstring integer)
    ushort_td len;          // length
    uchar_td decimals;      // ** decimals
    char viewNum;           // ** An order of a list view column
    ushort_td viewWidth;    // ** view width pix
    double max;             // ** max value
    double min;             // ** min value
    double defValue;        // ** default value
    uchar_td lookTable;     // ** reference table number
    uchar_td lookField;     // ** field number of reference table
    uchar_td lookFields[3]; // ** View fields of reference    bit567
    ushort_td pos;          // Field offset position from record image
    ushort_td defViewWidth; // ** default view wifth

protected:
    char m_chainChar[2];

public:
    ushort_td ddfid;       // ddf field id
    ushort_td filterId;    // ** filter id for reference
    uchar_td filterKeynum; // ** key number for reference
    uchar_td nullValue;    // null value
    ushort_td userOption;  // ** option
    uchar_td lookDBNum;    // ** database number of reference bitD
    ushort_td keylen;      // key length for mysql of part key

protected:
    uchar_td m_charsetIndex; // charctor set index of this field data
    uint_td m_schemaCodePage;

public:
    FLAGS enableFlags;     // ** enable flags. see below

private:
    inline void setSchemaCodePage(uint_td v){m_schemaCodePage = v;};
    friend class client::dbdef;
};

 /* This is only for BizStation Corp internal.
     enableFlags
     bit0  show list view
     bit1  enable max value
     bit2  enable min value
     bit3  enable default value
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
 */

typedef fielddef_t<MYSQL_FDNAME_SIZE> fielddef_t_my;
typedef fielddef_t<PERVASIVE_FDNAME_SIZE> fielddef_t_pv;


#ifdef SWIG
	%template(fielddef_t_my) fielddef_t<MYSQL_FDNAME_SIZE>;
#endif


struct PACKAGE fielddef : public fielddef_t_my
{
#ifdef _UNICODE
    const wchar_t* name() const ;               // Return a field name.
    const wchar_t* name(wchar_t* buf) const ;   // Return a field name to bufffer .
    const wchar_t* chainChar() const ;          // ** internal use only.
    void setName(const wchar_t* s);
    void setChainChar(const wchar_t* s);        // ** internal use only.
#else

    inline const char* name() const {return m_name;};

    inline const char* chainChar() const {return m_chainChar;};

    inline void setName(const char* s) {strncpy_s(m_name, FIELD_NAME_SIZE, s, sizeof(m_name) - 1);};

    inline void setChainChar(const char* s) {strncpy_s(m_chainChar, 2, s, sizeof(m_chainChar) - 1);};
#endif

    inline const char* nameA() const {return m_name;};

    inline void setNameA(const char* s) {strncpy_s(m_name, FIELD_NAME_SIZE, s, sizeof(m_name) - 1);};
#ifndef MYSQL_DYNAMIC_PLUGIN

    inline const _TCHAR* typeName() const {return getTypeName(type);};

    inline int align() const {return getTypeAlign(type);};

	inline void setLenByCharnum(ushort_td charnum)
    {
        len = lenByCharnum(type, m_charsetIndex, charnum);
    }

#endif

private:

    /* Is variable key type
     */
    inline bool isKeyVarType() const
    {
        return (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || (type == ft_myblob) ||
            (type == ft_mytext));
    }

    /* max key segment length. not include sizeBytes.
     */
    inline ushort_td maxKeylen() const {return keylen ? keylen : len;};

    inline ushort_td keyFromVarlen() const
    {
        if ((type >= ft_myvarchar) && (type <= ft_mywvarbinary))
            return len < 256 ? 1 : 2;
        else if ((type == ft_myblob) || (type == ft_mytext))
            return len - 8;
        return 0;
    }

public:

    inline unsigned int codePage() const {return mysql::codePage((unsigned short)m_charsetIndex);}

    /* data image for key
     * param ptr address of record buffer
     */
    inline const uchar_td* keyData(const uchar_td* ptr) const
    {
        if ((type == ft_myblob) || (type == ft_mytext))
            return blobDataPtr(ptr);
        int sizeByte = varLenBytes();
        return ptr + sizeByte;
    }

    inline uint_td keyDataLen(const uchar_td* ptr) const
    {
        if ((type == ft_myblob) || (type == ft_mytext))
            return blobDataLen(ptr);
        return dataLen(ptr);
    }

    /* copy key data for send to mysql
     *  return next copy address.
     */
    inline uchar_td* keyCopy(uchar_td* to, const uchar_td* from)
    {

        ushort_td kl = maxKeylen(); // size of max key segmnet for mysql
        ushort_td copylen = kl;
        memset(to, 0x00, kl);
        ushort_td keyVarlen = keyFromVarlen(); // size of var sizeByte for record.
        if (keyVarlen)
        {
            copylen = (ushort_td)std::min<uint_td>((uint_td)copylen, keyDataLen(from));
            memcpy(to, &copylen, 2);
            to += 2;
            from = keyData(from);
        }
        memcpy(to, from, copylen);
        return to + kl - keyVarlen;
    }

	/*  Get keyValue from keybuf for seek mode error description
     *  return next copy key address.
     */
    inline const uchar_td* getKeyValueFromKeybuf(const uchar_td* from, const uchar_td** data, ushort_td& size)
    {
		ushort_td keyVarlen = keyFromVarlen(); // size of var sizeByte for record.
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

    /* length bytes of var field
     */
    inline int varLenBytes() const
    {
        if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || type == ft_lstring)
            return len < 256 ? 1 : 2;
        return 0;
    }

    inline uint_td blobLenBytes() const
    {
        if ((type == ft_myblob) || (type == ft_mytext))
            return len - 8;
        return 0;
    }

    inline int maxVarDatalen() const
    {
        if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || type == ft_lstring)
            return (len < 256) ? len-1 : len-2;
        else if (type == ft_lvar)
            return len - 4;
        else  if ((type == ft_myblob) || (type == ft_mytext))
        {
            switch(len - 8)
            {
            case 1:return 0xFF;
            case 2:return 0xFFFF;
            case 3:return 0xFFFFFF;
            case 4:return 0xFFFFFFFF;
            }
            return 0;
        }
        return 0;
    }
    /* data length
     */
    inline uint_td dataLen(const uchar_td* ptr) const
    {
        int blen = varLenBytes();
        if (blen == 0)
            return len;
        else if (blen == 1)
            return*((unsigned char*)ptr);
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

    inline const uchar_td* blobDataPtr(const uchar_td* ptr) const
    {
        int blen = blobLenBytes();
        if (blen == 0)
            return NULL;
        const uchar_td** p = (const uchar_td * *)(ptr + blen);
        return *p;
    }

    /* Is string type or not.
     */
    bool isStringType() const ;

    /* Charctor numbers from charset.
     */
    unsigned int charNum() const ;

    inline void setCharsetIndex(uchar_td index)
    {
        m_charsetIndex = index;
        if ((type == ft_wstring) || (type == ft_wzstring) || (type == ft_mywvarchar) ||
            (type == ft_mywvarbinary) || (type == ft_mywchar))
            m_charsetIndex = CHARSET_UTF16LE;
    }

    inline uchar_td charsetIndex()const {return m_charsetIndex;};

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

};



namespace client{class dbdef;}

/* Mark of ** that BizStation Corp internal use only.
 */
struct PACKAGE tabledef
{
    friend class client::dbdef;       //for formatVersion
    
	tabledef()
    {
		cleanup();
    }

	void cleanup()
	{
        memset(this, 0, sizeof(tabledef));
        formatVersion = 1;
        primaryKeyNum = -1;
        parentKeyNum = -1;
        replicaKeyNum = -1;
        pageSize = 2048;
	}

#ifdef _UNICODE
    const wchar_t* fileName() const ;  // file name
    const wchar_t* tableName() const ; // table name
    void setFileName(const wchar_t* s);
    void setTableName(const wchar_t* s);
    const char* toChar(char* buf, const wchar_t* s, int size);

#else

    const char* fileName() const {return m_fileName;};

    const char* tableName() const {return m_tableName;};

    inline void setFileName(const char* s) {setFileNameA(s);};

    inline void setTableName(const char* s) {setTableNameA(s);};

    inline const char* toChar(char* buf, const char* s, int size){strncpy_s(buf, size, s, size-1);return buf;};

#endif

    const char* fileNameA() const {return m_fileName;};

    const char* tableNameA() const {return m_tableName;};

    inline void setFileNameA(const char* s) 
	{
		strncpy_s(m_fileName, FILE_NAME_SIZE, s, FILE_NAME_SIZE - 1);
    }

    inline void setTableNameA(const char* s) 
	{
        strncpy_s(m_tableName, TABLE_NAME_SIZE, s, sizeof(m_tableName) - 1);
	}

    ushort_td id;         // table id
    ushort_td pageSize;   // page size
    ushort_td preAlloc;   // pre alloc page seize
    ushort_td fieldCount; // Number of field
    uchar_td keyCount;    // Number of key

private:
    char m_fileName[FILE_NAME_SIZE];
    char m_tableName[TABLE_NAME_SIZE];

public:
    short version;            // table version
    uchar_td charsetIndex;    // SCHARSET_INFO vector index;
    uchar_td filler0[17];     // reserved
    FLAGS flags;              // file flags
    uchar_td primaryKeyNum;      // Primary key number. -1 is no primary.
    uchar_td parentKeyNum;    // ** Key number for listview. -1 is no use.
    uchar_td replicaKeyNum;   // ** Key number for repdata. -1 is no use.
    FLAGS optionFlags;        // ** optional flags
    ushort_td convertFileNum; // ** not use
    ushort_td maxRecordLen;   // max record length of var size table.
    uchar_td treeIndex;       // ** View index for listview.
    uchar_td iconIndex;       // ** Icon index for listview.
    ushort_td ddfid;
    ushort_td fixedRecordLen; // Fixed record length for var size table.
    int autoIncExSpace;       //
    uchar_td iconIndex2;      //
    uchar_td iconIndex3;      //
    uint_td schemaCodePage;   // Code page of this schema stirng data.
private:
    char    formatVersion;    //
public:
	client::dbdef* parent; 
    uchar_td reserved[TABLEDEF_FILLER_SIZE]; // reserved

    fielddef* fieldDefs; // Pointer cahche of first field.
    keydef* keyDefs;     // Pointer cahche of first key.
};

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

 */

struct PACKAGE btrVersion
{
    ushort_td majorVersion;
    ushort_td minorVersion;
    unsigned char type;
	const _TCHAR* moduleVersionShortString(_TCHAR* buf);
	const _TCHAR* moduleTypeString();
};

struct btrVersions
{
    btrVersion versions[4];
};

#pragma option -a.
pragma_pop

/*filter cobine type*/
enum combineType{eCend, eCand, eCor};

PACKAGE uchar_td getFilterLogicTypeCode(const _TCHAR* cmpstr);

}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_TDAPSCHEMA_H
