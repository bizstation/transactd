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

#include <tchar.h>
#pragma hdrstop

#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/protocol/tdap/myDateTime.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>
using namespace bzs::env;
#endif
#pragma package(smart_init)

#undef USETLS
#if ((defined(_WIN32) && _MSC_VER) || (__APPLE__ && !defined(__BCPLUSPLUS__)))
#ifdef __APPLE__
#include <pthread.h>
#endif
#define USETLS
#endif

#ifdef USETLS
extern tls_key g_tlsiID_SC1;
#else
wchar_t __THREAD g_nameBuf[266] = { 0 };
#endif

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

inline wchar_t* namebuf()
{
#ifdef USETLS
    wchar_t* p = (wchar_t*)tls_getspecific(g_tlsiID_SC1);
    if (p == NULL)
    {
        p = (wchar_t*)new wchar_t[256];
        tls_setspecific(g_tlsiID_SC1, p);
    }
    return p;
#else
    return g_nameBuf;
#endif
}


char* doubleToStr(double v, int decimals, char* p, int psize)
{
    if (decimals)
    {
        char buf[10] = "%0.";
        _ltoa_s(decimals, p, psize, 10);
        strcat_s(buf, 10, p);
        strcat_s(buf, 10, "lf");
        sprintf_s(p, psize, buf, v);
    }else
        sprintf_s(p, psize, "%0.0lf", v);
    return p;
}

wchar_t* doubleToStr(double v, int decimals, wchar_t* p, int psize)
{
    if (decimals)
    {
        wchar_t buf[10] = L"%0.";
        _ltow_s(decimals, p, psize, 10);
        wcscat_s(buf, 10, p);
        wcscat_s(buf, 10, L"lf");
        swprintf(p, psize, buf, v);
    }else
        swprintf(p, psize, L"%0.0lf", v);
    return p;
}


inline char* namebufA()
{
    return (char*)namebuf();
}

#ifdef _UNICODE

const wchar_t* fielddef::name() const
{
    return name(namebuf());
}

const wchar_t* fielddef::name(wchar_t* buf) const
{
    MultiByteToWideChar(m_schemaCodePage,
                        (m_schemaCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                        m_name, -1, buf, MYSQL_FDNAME_SIZE);
    return buf;
}

const wchar_t* fielddef::chainChar() const
{
    wchar_t* p = namebuf();
    MultiByteToWideChar(m_schemaCodePage,
                        (m_schemaCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                        m_chainChar, -1, p, 2);
    return p;
}

const wchar_t* fielddef::defaultValue_str() const
{
    if (*((__int64*)m_defValue) == 0) return L"";
    wchar_t* p = namebuf();
    if (isStringTypeForIndex(type))
    {
        wchar_t* p = namebuf();
        MultiByteToWideChar(m_schemaCodePage,
                            (m_schemaCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                            m_defValue, -1, p, MYSQL_FDNAME_SIZE);
        return p;
    }
    if (isDateTimeType())
    {
        bool tsu = isTimeStampOnUpdate();
        *(const_cast<char*>(m_defValue) + 7) = 0x00;
        __int64 i64 = *((__int64*)m_defValue);
        myDateTime dt(4, true);
        dt.setValue(i64);
        p[0] = 0x00;
        switch(type)
        {
        case ft_date:
        case ft_mydate:
            if (dt.internalValue()) 
                dt.dateStr(p);
            break;
        case ft_time:
        case ft_mytime:
            if (dt.internalValue())
                dt.timeStr(p);
            break;
        case ft_datetime:
        case ft_timestamp:
            if (dt.internalValue())
                dt.toString(p);
            break;
        case ft_mydatetime:
        case ft_mytimestamp:
        {
            if (i64 == DFV_TIMESTAMP_DEFAULT)
                p = DFV_TIMESTAMP_DEFAULT_WSTR;
            else if (dt.internalValue()) 
                dt.toString(p);
            break;
        }
        default:
            assert(0);    
        }
        //restore
        *(const_cast<char*>(m_defValue) + 7) = tsu ? 1: 0;
    }else
         doubleToStr(*((double*)m_defValue), decimals, p, MYSQL_FDNAME_SIZE);
    return p;
}


void fielddef::setName(const wchar_t* s)
{
    WideCharToMultiByte(m_schemaCodePage,
                        (m_schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK,
                        s, -1, m_name, MYSQL_FDNAME_SIZE, NULL, NULL);
}

void fielddef::setChainChar(const wchar_t* s)
{
    WideCharToMultiByte(m_schemaCodePage,
                        (m_schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK,
                        s, -1, m_chainChar, 2, NULL, NULL);
}

void fielddef::setDefaultValue(const wchar_t* s)
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
        i64 = str_to_64<myDateTime, wchar_t>(7, true, s);
        memcpy(m_defValue, &i64, 7);
        return;
    }

    if (isNumericType())
    {
        double d = _wtof(s);
        setDefaultValue(d);
        return;
    } 
    WideCharToMultiByte(m_schemaCodePage,
                        (m_schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK,
                        s, -1, m_defValue, 8, NULL, NULL);
   
}

const wchar_t* tabledef::fileName() const
{
    wchar_t* p = namebuf();
    MultiByteToWideChar(schemaCodePage,
                        (schemaCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                        m_fileName, -1, p, FILE_NAME_SIZE);
    return p;
}

const wchar_t* tabledef::tableName() const
{
    wchar_t* p = namebuf();
    MultiByteToWideChar(schemaCodePage,
                        (schemaCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED,
                        m_tableName, -1, p, TABLE_NAME_SIZE);
    return p;
}

const char* tabledef::toChar(char* buf, const wchar_t* s, int size) const
{
    WideCharToMultiByte(schemaCodePage,
                        (schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK, s,
                        -1, buf, size, NULL, NULL);
    return buf;
}

void tabledef::setFileName(const wchar_t* s)
{
    WideCharToMultiByte(schemaCodePage,
                        (schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK, s,
                        -1, m_fileName, FILE_NAME_SIZE, NULL, NULL);
}

void tabledef::setTableName(const wchar_t* s)
{
    WideCharToMultiByte(schemaCodePage,
                        (schemaCodePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK, s,
                        -1, m_tableName, TABLE_NAME_SIZE, NULL, NULL);
}
#else // NOT _UNICODE

const char* fielddef::name() const
{
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
    {
        char* p = namebufA();
        mbctou8(m_name, strlen(m_name), p, MYSQL_FDNAME_SIZE);
        return p;
    }
#endif
    return m_name;
}

const char* fielddef::name(char* buf) const
{
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
    {
        mbctou8(m_name, strlen(m_name), buf, MYSQL_FDNAME_SIZE);
        return buf;
    }
#endif
    return m_name;
}

const char* fielddef::chainChar() const
{
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
    {
        char* p = namebufA();
        mbctou8(m_chainChar, strlen(m_chainChar), p, 2);
        return p;
    }
#endif
    return m_chainChar;
}

const char* fielddef::defaultValue_strA() const
{
    if (*((__int64*)m_defValue) == 0) return "";
    char* p = namebufA();
    if (isStringTypeForIndex(type))
    {
    #ifdef LINUX
        if (m_schemaCodePage != CP_UTF8)
        {
            char* p = namebufA();
            mbctou8(m_defValue, strlen(m_defValue), p, MYSQL_FDNAME_SIZE);
            return p;
        }
    #endif
        return m_defValue;
    }

    if (isDateTimeType())
    {
        bool tsu = isTimeStampOnUpdate();
        *(const_cast<char*>(m_defValue) + 7) = 0x00;
        __int64 i64 = *((__int64*)m_defValue);
        myDateTime dt(4, true);
        dt.setValue(i64); // i64 not equal dt.internalValue();
        p[0] = 0x00;

        switch(type)
        {
        case ft_date:
        case ft_mydate:
             if (dt.internalValue())
                dt.dateStr(p);
             break;
        case ft_time:
        case ft_mytime:
            if (dt.internalValue())
                dt.timeStr(p);
            break;
        case ft_datetime:
        case ft_timestamp:
            if (dt.internalValue())
                dt.toString(p);
            break;
        case ft_mydatetime:
        case ft_mytimestamp:
        {
            if (i64 == DFV_TIMESTAMP_DEFAULT)
                p = DFV_TIMESTAMP_DEFAULT_ASTR;
            else if (dt.internalValue()) 
                dt.toString(p);
            break;
        }
        default:
            assert(0);
        }
         //restore
        *(const_cast<char*>(m_defValue) + 7) = tsu ? 1: 0;
    }
    else
        doubleToStr(*((double*)m_defValue), decimals, p, MYSQL_FDNAME_SIZE);
   
    return p;
}

void fielddef::setName(const char* s)
{
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
        u8tombc(s, strlen(s), m_name, FIELD_NAME_SIZE);
    else
#endif
        strncpy_s(m_name, FIELD_NAME_SIZE, s, sizeof(m_name) - 1);
}

void fielddef::setChainChar(const char* s)
{
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
        u8tombc(s, strlen(s), m_chainChar, 2);
    else
#endif
        strncpy_s(m_chainChar, 2, s, sizeof(m_chainChar) - 1);
}

void fielddef::setDefaultValue(const char* s)
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
    }
    
    if (isNumericType())
    {
        double d = atof(s);
        setDefaultValue(d);
        return;
    }
#ifdef LINUX
    if (m_schemaCodePage != CP_UTF8)
        u8tombc(s, strlen(s), m_defValue, 8);
    else
#endif
        strncpy_s(m_defValue, 8, s, sizeof(m_defValue) - 1);
    
}

const char* tabledef::fileName() const
{
#ifdef LINUX
    if (schemaCodePage != CP_UTF8)
    {
        char* p = namebufA();
        mbctou8(m_fileName, strlen(m_fileName), p, FILE_NAME_SIZE);
        return p;
    }
#endif
    return m_fileName;
}

const char* tabledef::tableName() const
{
#ifdef LINUX
    if (schemaCodePage != CP_UTF8)
    {
        char* p = namebufA();
        mbctou8(m_tableName, strlen(m_tableName), p, TABLE_NAME_SIZE);
        return p;
    }
#endif
    return m_tableName;
}

void tabledef::setFileName(const char* s)
{
#ifdef LINUX
    if (schemaCodePage != CP_UTF8)
        u8tombc(s, strlen(s), m_fileName, FILE_NAME_SIZE);
    else
#endif
        setFileNameA(s);
}

void tabledef::setTableName(const char* s)
{
#ifdef LINUX
    if (schemaCodePage != CP_UTF8)
        u8tombc(s, strlen(s), m_tableName, TABLE_NAME_SIZE);
    else
#endif
        setTableNameA(s);
}

const char* tabledef::toChar(char* buf, const char* s, int size) const
{
#ifdef LINUX
    if (schemaCodePage != CP_UTF8)
    {
        u8tombc(s, strlen(s), buf, size);
        return buf;
    }
#endif
    strncpy_s(buf, size, s, size - 1);
    return buf;
}

#endif // NOT _UNICODE


//--------------------------------------------------------------------
//   struct keydef
//--------------------------------------------------------------------
short keydef::synchronize(const keydef* kd)
{
    for (int i = 0; i < kd->segmentCount; ++i)
    {
        if (i < segmentCount)
        {
            if (segments[i].fieldNum == kd->segments[i].fieldNum)
            {
                const FLAGS f = kd->segments[i].flags;
                segments[i].flags.bit1 = f.bit1;
                segments[i].flags.bit2 = f.bit2;
                segments[i].flags.bit3 = f.bit3;
                segments[i].flags.bit7 = f.bit7;
                segments[i].flags.bit8 = f.bit8;
                segments[i].flags.bit9 = f.bit9;
            }
        }
    }
    keyNumber = kd->keyNumber;
    return 0;
}

bool keydef::operator==(const keydef& r) const
{
    if (this == &r) return true;
    bool ret = (segmentCount == r.segmentCount) && (keyNumber == keyNumber);
    if (ret)
       ret = memcmp(segments, r.segments, sizeof(keySegment) * segmentCount) == 0;
    return ret;
}

//--------------------------------------------------------------------
//   struct fielddef
//--------------------------------------------------------------------
bool fielddef::operator==(const fielddef& r) const
{
    //ignore  m_nullbit m_nullbytes
    if (this == &r) return true;
    if (isStringType() && (m_charsetIndex != r.m_charsetIndex))
    if (isPadCharType() && 
        ((isUsePadChar() != r.isUsePadChar()) || (isTrimPadChar() != r.isTrimPadChar())))
        return false;

    _TCHAR tmp[256];
    _tcscpy_s(tmp, 256, r.defaultValue_str());
    bool ret =  _tcscmp(defaultValue_str(), tmp) == 0;
    return (ret &&
            _tcscmp(name(), r.name(tmp)) == 0) &&
            (type == r.type) &&
            (len == r.len) &&
            (decimals == r.decimals) &&
            (viewNum == r.viewNum) &&
            (viewWidth == r.viewWidth) &&
            (max == r.max) &&
            (min == r.min) &&
            (lookTable == r.lookTable) &&
            (lookField == r.lookField) &&
            (memcmp(lookFields, r.lookFields, 3) == 0) &&
            (pos == r.pos) &&
            (m_nullbit == r.m_nullbit) &&
            (m_nullbytes == r.m_nullbytes) &&
            (memcmp(m_chainChar, r.m_chainChar, 2) == 0) &&
            (ddfid == r.ddfid) &&
            (filterId == r.filterId) &&
            (filterKeynum == r.filterKeynum) &&
            (nullValue == r.nullValue) &&
            (userOption == r.userOption) &&
            (lookDBNum == r.lookDBNum) &&
            (keylen == r.keylen) &&
            ((m_options & ~FIELD_OPTION_MARIADB) == (r.m_options & ~FIELD_OPTION_MARIADB)) &&
            (enableFlags.all == r.enableFlags.all);
}

unsigned int fielddef::charNum() const
{
    if (type == ft_mychar)
        return (unsigned int)len / mysql::charsize(m_charsetIndex);
    else if (type == ft_mywchar)
        return (unsigned int)len / mysql::charsize(CHARSET_UTF16LE);
    else if (type == ft_myvarchar)
        return (unsigned int)(len - varLenBytes()) /
               mysql::charsize(m_charsetIndex);
    else if (type == ft_mywvarchar)
        return (unsigned int)(len - varLenBytes()) /
               mysql::charsize(CHARSET_UTF16LE);
    return len;
}

bool fielddef::isValidCharNum() const
{
    unsigned int num = charNum();
    if (type == ft_mychar)
        return ((unsigned int)len == num * mysql::charsize(m_charsetIndex));
    else if (type == ft_mywchar)
        return ((unsigned int)len == num * mysql::charsize(CHARSET_UTF16LE));
    else if (type == ft_myvarchar)
        return ((unsigned int)(len - varLenBytes()) == num * mysql::charsize(m_charsetIndex));
    else if (type == ft_mywvarchar)
        return ((unsigned int)(len - varLenBytes()) == num * mysql::charsize(CHARSET_UTF16LE));
    return true;
}

void fielddef::fixCharnum_bug()
{
    unsigned int num = charNum();
    if (type == ft_mychar)
        len = num * mysql::charsize(m_charsetIndex);
    else if (type == ft_mywchar)
        len = num * mysql::charsize(CHARSET_UTF16LE);
    else if (type == ft_myvarchar)
        len = num * mysql::charsize(m_charsetIndex) + varLenBytes();
    else if (type == ft_mywvarchar)
        len = num * mysql::charsize(CHARSET_UTF16LE) + varLenBytes();
}

/** Length of compare
 * if part of string or zstring then return strlen * sizeof(char or wchar).
 */
inline uint_td fielddef::compDataLen(const uchar_td* ptr, bool part) const
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

bool isCompatibleType(uchar_td l, uchar_td r, ushort_td rlen, uchar_td rchar)
{
	if (l == ft_integer) 
    {
        if ((r == ft_currency) && (rlen == 8)) return true;
	    if ((r == ft_currency) && (rlen == 8)) return true;
	    if ((r == ft_date) && (rlen == 4)) return true;
	    if ((r == ft_time) && (rlen == 4)) return true;
	    if ((r == ft_datetime) && (rlen == 8)) return true;
	    if ((r == ft_timestamp) && (rlen == 8)) return true;
    }
	else if (l == ft_uinteger)
    {
        if ((r == ft_logical) && (rlen <= 2)) return true;
	    if ((r == ft_bit) && (rlen <= 8)) return true;
	    if ((r == ft_enum) && (rlen <= 8)) return true;
	    if ((r == ft_set) && (rlen <= 8)) return true;
    }
	

	// mywchar --> mywchar OK!
	// string  --> string  OK!
	// mychar  --> mychar  OK!
	// mywchar --> mywchar OK!

	if (l == ft_myvarbinary)
    {
        if (r == ft_myvarbinary) return true;
        if (r == ft_mywvarbinary && rchar == CHARSET_UTF16LE) return true;
	    if (r == ft_lstring) return true;
	    if (r == ft_lvar) return true;
	    if (r == ft_note) return true;
	    if (r == ft_myfixedbinary) return true;
    }
	else if (l == ft_string)
    {
         if (r == ft_string) return true;

         // zstring --> string
	     if ((r == ft_zstring) && (rchar != CHARSET_UTF16LE)) return true;
	
	     // wzstring --> string 
	     if ((r == ft_wzstring) && (rchar == CHARSET_UTF16LE)) return true;

	     // wstring --> string 
	     if ((r == ft_wstring) && (rchar == CHARSET_UTF16LE)) return true;
         if (r == ft_decimal) return true;
         if (r == ft_money) return true;
         if (r == ft_numeric) return true;
         if (r == ft_bfloat) return true;
         if (r == ft_numericsts) return true;
         if (r == ft_numericsa) return true;
         if (r == ft_guid) return true;
    }
    return false;
}

short fielddef::synchronize(const fielddef* fd)
{
    viewNum = fd->viewNum;
    viewWidth = fd->viewWidth;
    max = fd->max;
    min = fd->min;
    lookTable = fd->lookTable;
    lookField = fd->lookField;
    memcpy(lookFields, fd->lookFields, sizeof(uchar_td) * 3);
    m_chainChar[0] = fd->m_chainChar[0];
    m_chainChar[1] = fd->m_chainChar[1];
    ddfid = fd->ddfid;
    filterId = fd->filterId;
    filterKeynum = fd->filterKeynum;
    nullValue = fd->nullValue;
    userOption = fd->userOption;
    lookDBNum = fd->lookDBNum;
    uchar_td nullable = m_options & FIELD_OPTION_NULLABLE;
    m_options = fd->m_options;
    m_options |= nullable;
    bool defaultNull = enableFlags.bitF;
    enableFlags = fd->enableFlags;
    enableFlags.bitF = defaultNull;
    if (type == ft_mychar || type == ft_string || type == ft_mywchar || type == ft_wstring)
        m_padCharOptions = fd->m_padCharOptions;
    if (fd->type == ft_myfixedbinary && len - 2 == fd->len)
    {
        len = fd->len ;
        type = fd->type;
    }
    if (len == fd->len && isCompatibleType(type, fd->type, fd->len, fd->m_charsetIndex))
    {
        type = fd->type;
        m_charsetIndex = fd->m_charsetIndex;
    }
    if (type == ft_lvar || type == ft_myfixedbinary)
        setDefaultValue(0.0f);
    return 0;
}

//--------------------------------------------------------------------
//   struvt tabledef
//--------------------------------------------------------------------
bool tabledef::operator==(const tabledef& r) const
{
    if (this == &r) return true;
    // remove file extention
    _TCHAR tmp[256];
    _TCHAR tmp2[256];
    _tcscpy_s(tmp, 256, r.fileName());
    _tcscpy_s(tmp2, 256, fileName());
    _tcslwr_s(tmp, 256);
    _tcslwr_s(tmp2, 256);
    _TCHAR* p = _tcsrchr(tmp, _T('.'));
    _TCHAR* p2 = _tcsrchr(tmp2, _T('.'));
    if (p && !p2) *p = 0x00;
    if( !p && p2) *p2 = 0x00;

    bool ret =  _tcscmp(tmp2, tmp) == 0;
    if (!ret) return ret;
    _tcscpy_s(tmp, 256, r.tableName());
    ret =  _tcscmp(tableName(), tmp) == 0;

    return  (ret) &&
            (varSize == r.varSize) &&
            (preAlloc == r.preAlloc) &&
            (fieldCount == r.fieldCount) &&
            (keyCount == r.keyCount) &&
            (version == r.version) &&
            (charsetIndex == r.charsetIndex) &&
            (m_nullfields == r.m_nullfields) &&
            (m_nullbytes == r.m_nullbytes) &&
            (flags.all == r.flags.all) &&
            (primaryKeyNum == r.primaryKeyNum) &&
            (parentKeyNum == r.parentKeyNum) &&
            (replicaKeyNum == r.replicaKeyNum) &&
            (optionFlags.all == r.optionFlags.all) &&
            (convertFileNum == r.convertFileNum) &&
            (treeIndex == r.treeIndex) &&
            (iconIndex == r.iconIndex) &&
            (ddfid == r.ddfid) &&
            (iconIndex2 == r.iconIndex2) &&
            (iconIndex3 == r.iconIndex3) &&
            (formatVersion == r.formatVersion) &&
            (varSize == r.varSize) ;
}

bool tabledef::isNullKey(const keydef& key) const
{
    if (key.segments[0].flags.bit3 || key.segments[0].flags.bit9)
    {
        for (int j=0;j < key.segmentCount; ++j)
        {
            const fielddef& fd = fieldDefs[key.segments[j].fieldNum];
            if (fd.nullValue != 0x00) 
                return false;
        }
        return true;
    }
    return false;
}

bool tabledef::isNeedNis(const keydef& key) const
{
    /* bit3 all segment NULL key
       bit9 part segment NULL key
       if fd.nullValue != 0x00 then this field is type of not null.
    */
    if (!isNULLFieldFirstKeySegField(key))
        return isNullKey(key);
     return false;
}

bool tabledef::isNULLFieldFirstKeySegField(const keydef& key) const
{
    // logical 1 byte and segmentCount = 1 and nullValue = 0x00
    if ((key.segments[0].flags.bit3 || key.segments[0].flags.bit9) && key.segmentCount == 1)
    {
        const fielddef& fd = fieldDefs[key.segments[0].fieldNum];
        return ((fd.len == 1) && (fd.type == ft_logical) && fd.nullValue == 0x00);
    }
    return false;
}

void tabledef::setMysqlNullMode(bool v)
{
    if (m_mysqlNullMode != v)
    {
        m_mysqlNullMode = v;
        calcReclordlen();
    }
}

void tabledef::calcReclordlen(bool force)
{
    if (m_inUse == 0 || force)
    {
        if (force) m_inUse = 0;
        int nisFieldNum = 0;
        bool firstTimeStamp = true;
        m_nullfields = 0;
        m_nullbytes = 0;
        m_maxRecordLen = 0;

        // Check Null Key
        for (int i = 0; i < keyCount; i++)
        {
            keydef& kd = keyDefs[i];
            if (isNullKey(kd))
            {
                if (m_mysqlNullMode)
                    nisFieldNum += (isNeedNis(kd) ? 1 : 0);
                if (isNULLFieldFirstKeySegField(kd))
                {
                    fielddef& fd = fieldDefs[kd.segments[0].fieldNum];
                    fd.setNullable(true, true);
                }
            }
        }

        for (int i = 0; i < fieldCount; i++)
        {
            fielddef& fd = fieldDefs[i];
            fd.m_nullbytes = 0;
            fd.m_nullbit = 0;
            if (fd.isNullable())
            {
                fd.m_nullbit = m_nullfields;
                if (m_mysqlNullMode)
                    ++m_nullfields;
            }
            double defaultValue = fd.defaultValue();
            if (fd.type == ft_mytimestamp && fd.isNullable() == false &&
                (defaultValue == 0 || defaultValue == DFV_TIMESTAMP_DEFAULT) && firstTimeStamp)
            {
                fd.setDefaultValue(DFV_TIMESTAMP_DEFAULT);
                fd.setTimeStampOnUpdate(true);
                firstTimeStamp = false;
            }

            if (fd.type == ft_mytimestamp && fd.decimals == 0)
                fd.decimals = (fd.len - 4) * 2;
            else if (fd.type == ft_mydatetime)
            {
                if (isMariaTimeFormat())
                    fd.m_options |= FIELD_OPTION_MARIADB;
                else
                    fd.m_options &= ~FIELD_OPTION_MARIADB;
                // datetime decimals is different by server version 
                if (fd.decimals == 0)
                    fd.decimals = (fd.len - 5) * 2;
            }
            else if (fd.type == ft_mytime && fd.decimals == 0)
                fd.decimals = (fd.len - 3) * 2;
            else if (defaultValue && fd.isBlob())
                fd.setDefaultValue(0.0f);

            if (isLegacyTimeFormat(fd))
            {
                fd.m_options |= FIELD_OPTION_REGACY_TIME;
                fd.decimals = 0;
                if (fd.type == ft_mydatetime)
                    fd.len = 8;
                else if (fd.type == ft_mytime)
                    fd.len = 3;
                else if (fd.type == ft_mytimestamp)
                    fd.len = 4;
            }
            else
                fd.m_options &= ~FIELD_OPTION_REGACY_TIME;
            fd.pos = m_maxRecordLen;
            fd.fixCharnum_bug();
            m_maxRecordLen += fd.len;
        }

        m_nullfields += nisFieldNum;

        if (m_nullfields)
        {
            m_nullbytes = (m_nullfields + 7) / 8;
            for (int i = 0; i < fieldCount; i++)
                fieldDefs[i].m_nullbytes = m_nullbytes;
        }
        m_maxRecordLen += m_nullbytes;
        // If valible length then specifing fixed length.
        if ((fixedRecordLen == 0) || (flags.bit0 == false))
            fixedRecordLen = m_maxRecordLen;
    }else
        ;//assert(0);
}

uint_td tabledef::unPack(char* ptr, size_t size) const
{
    char* pos = ptr + m_nullbytes;
    const char* end = pos + size;
    const char* max = pos + m_maxRecordLen;
    int movelen;

    // if null then not recieved field data
    unsigned char null_bit = 1;
    unsigned char* null_ptr = (unsigned char*)ptr;

    for (int i = 0; i < fieldCount; i++)
    {
        fielddef& fd = fieldDefs[i];
        bool isNull = false;
        if (fd.isNullable() && m_nullbytes)
        {
            isNull = (*null_ptr & null_bit) != 0;
            if (null_bit == (unsigned char)128)
            {
                ++null_ptr;
                null_bit = 1;
            }else
                null_bit = null_bit << 1;
        }

        if (fd.type == ft_myfixedbinary)
        {
            int dl = *((unsigned short*)(pos));
            memmove(pos, pos + 2, dl);
            pos += fd.len - 2;
            *((unsigned short*)(pos)) = 0x00;
            pos += 2;
        }
        else
        {
            int blen = fd.varLenBytes();
            int dl = fd.len; // length
            if (isNull) 
                dl = 0;
            else
            {
                if (blen == 1)
                    dl = *((unsigned char*)(pos)) + blen;
                else if (blen == 2)
                    dl = *((unsigned short*)(pos)) + blen;
            }
                
            if ((movelen = fd.len - dl) != 0)
            {
                if (max < end + movelen)
                    return 0;
                char* src = pos + dl;
                memmove(pos + fd.len, src, end - src);
                memset(src, 0, movelen);
                end += movelen;
            }
            pos += fd.len;
        }
    }
    return (uint_td)(pos - ptr);
}

uint_td tabledef::pack(char* ptr, size_t size) const
{
    char* pos = ptr + m_nullbytes;
    char* end = pos + size;
    int movelen;

    // if null then not copy field image (field length move)
    unsigned char null_bit = 1;
    unsigned char* null_ptr = (unsigned char*)ptr;
    for (int i = 0; i < fieldCount; i++)
    {
        fielddef& fd = fieldDefs[i];
        bool isNull = false;
        if (fd.isNullable() && m_nullbytes)
        {
            isNull = (*null_ptr & null_bit) != 0;
            if (null_bit == (unsigned char)128)
            {
                ++null_ptr;
                null_bit = 1;
            }else
                null_bit = null_bit << 1;
        }
        
        if (fd.type == ft_myfixedbinary)
        {
            memmove(pos + 2, pos, fd.len - 2); // move as size pace in the field
            *((unsigned short*)(pos)) = fd.len - 2; // fixed size
            pos += fd.len;
        }
        else
        {
            int blen = fd.varLenBytes();
            int dl = fd.len; // length
            if (blen == 1)
                dl = *((unsigned char*)(pos)) + blen;
            else if (blen == 2)
                dl = *((unsigned short*)(pos)) + blen;
            if (isNull)
                dl = 0;
            else
                pos += dl;
            if ((movelen = fd.len - dl) != 0)
            {
                end -= movelen;
                memmove(pos, pos + movelen, end - pos);
            }
        }
        
    }
    return (uint_td)(pos - ptr);
}

int tabledef::size() const
{
    int len =  (int)(sizeof(tabledef) + (sizeof(fielddef) * fieldCount) +
                    (sizeof(keydef) * keyCount));
    const ushort_td* p = &varSize;
    *(const_cast<ushort_td*>(p)) = len - 4;
    return len;
}

short tabledef::fieldNumByName(const _TCHAR* name) const
{
    char buf[74];
    const char* p = toChar(buf, name, 74);
    for (short i = 0; i < fieldCount; i++)
    {
        if (strcmp(fieldDefs[i].nameA(), p) == 0)
            return i;
    }
    return -1;
}

short tabledef::findKeynumByFieldNum(short fieldNum) const
{
    for (short i = 0; i < keyCount; i++)
    {
        if (keyDefs[i].segments[0].fieldNum == fieldNum)
            return i;
    }
    return -1;
}

void tabledef::setDefaultCharsetIfZero()
{
    if (charsetIndex == 0)
        charsetIndex = mysql::charsetIndex(GetACP());

    for (short i = 0; i < fieldCount; i++)
    {
        fielddef& fd = fieldDefs[i];
        if (fd.charsetIndex() == 0)
            fd.setCharsetIndex(charsetIndex);
        fd.setSchemaCodePage(schemaCodePage);
    }
}

short tabledef::synchronize(const tabledef* td)
{
    id = td->id;
    setTableName(td->tableName());
    preAlloc = td->preAlloc;
    version = td->version;
    m_inUse = td->m_inUse;
    m_mysqlNullMode = td->m_mysqlNullMode;
    parentKeyNum = td->parentKeyNum;
    replicaKeyNum = td->replicaKeyNum;
    optionFlags = td->optionFlags;
    convertFileNum = td->convertFileNum;
    treeIndex = td->treeIndex;
    iconIndex = td->iconIndex;
    ddfid = td->ddfid;
    autoIncExSpace = td->autoIncExSpace;
    iconIndex2 = td->iconIndex2;
    iconIndex3 = td->iconIndex3;
    formatVersion = td->formatVersion;
    parent = td->parent;
    flags = td->flags;
    for (int i=0;i<td->fieldCount; ++i)
    {
        fielddef* fd = &td->fieldDefs[i];
        short index = fieldNumByName(fd->name());
        if (index != -1)
            fieldDefs[index].synchronize(fd);
    }
    for (int i=0;i<td->keyCount; ++i)
    {
        if (i < keyCount)
            keyDefs[i].synchronize(&td->keyDefs[i]);
    }
    return 0;
}

ushort_td lenByCharnum(uchar_td type, uchar_td charsetIndex, ushort_td charnum)
{
    ushort_td len = charnum;
    if ((type == ft_wstring) || (type == ft_wzstring) || (type == ft_mywchar) ||
                    (type == ft_mywvarchar)|| (type == ft_mywvarbinary))
        len = (ushort_td)(mysql::charsize(CHARSET_UTF16LE) * charnum);
    else if (charsetIndex && ((type == ft_mychar) || (type == ft_myvarchar)|| 
            (type == ft_string) || (type == ft_zstring)|| (type == ft_myvarbinary) ))
        len = (ushort_td)(mysql::charsize(charsetIndex) * charnum);
    if ((type == ft_myvarchar) || (type == ft_mywvarchar) || (type == ft_myvarbinary) || (type == ft_mywvarbinary))
        len += ((len >= 256) ? 2 : 1);
    else
        len = std::min<ushort_td>(len, 255);
    return len;
}

const _TCHAR* getTypeName(short type)
{
    switch (type)
    {
    case ft_string:
        return _T("String");
    case ft_integer:
        return _T("Integer");
    case ft_float:
        return _T("Float");
    case ft_date:
        return _T("Date");
    case ft_time:
        return _T("Time");
    case ft_decimal:
        return _T("Decimal");
    case ft_money:
        return _T("Money");
    case ft_logical:
        return _T("Logical");
    case ft_numeric:
        return _T("Numeric");
    case ft_bfloat:
        return _T("BFloat");
    case ft_lstring:
        return _T("LString");
    case ft_zstring:
        return _T("ZString");
    case ft_note:
        return _T("Note");
    case ft_lvar:
        return _T("Lvar");
    case ft_uinteger:
        return _T("Unsigned Binary");
    case ft_autoinc:
        return _T("AutoIncrement");
    case ft_bit:
        return _T("Bit");
    case ft_enum:
        return _T("Enum");
    case ft_set:
        return _T("Set");
    case ft_numericsts:
        return _T("Numericsts");
    case ft_numericsa:
        return _T("Numericsa");
    case ft_currency:
        return _T("Currency");
    case ft_timestamp:
        return _T("TimeStamp");
    case ft_blob:
        return _T("Blob");
    case ft_wstring:
        return _T("WString");
    case ft_wzstring:
        return _T("WZstring");
    case ft_datetime:
        return _T("DateTime");
    case ft_guid:
        return _T("Guid");
    case ft_myvarchar:
        return _T("myVarChar");
    case ft_myvarbinary:
        return _T("myVarBinary");
    case ft_mywvarchar:
        return _T("myWVarChar");
    case ft_mywvarbinary:
        return _T("myWVarBinary");
    case ft_mychar:
        return _T("myChar");
    case ft_mywchar:
        return _T("myWChar");
    case ft_myyear:
        return _T("myYear");
    case ft_mydate:
        return _T("myDate");
    case ft_mytime:
        return _T("myTime");
    case ft_mydatetime:
        return _T("myDateTime");
    case ft_mytimestamp:
        return _T("myTimeStamp");
    case ft_mytext:
        return _T("myText");
    case ft_myblob:
        return _T("myBlob");
    case ft_mygeometry:
        return _T("myGeometry");
    case ft_myjson:
        return _T("myJson");
    case ft_autoIncUnsigned:
        return _T("AutoIncUnsigned");
    case ft_myfixedbinary:
        return _T("myFixedBinary");
    case ft_nullindicator:
        return _T("Nullindicator");
    default:
        return _T("Unknown");
    }
}

int getTypeAlign(short type)
{
    switch (type)
    {
    case ft_string:
    case ft_date:
    case ft_time:
    case ft_lstring:
    case ft_zstring:
    case ft_note:
    case ft_timestamp:
    case ft_wstring:
    case ft_wzstring:
    case ft_myvarchar:
    case ft_myvarbinary:
    case ft_mychar:
    case ft_mywchar:
    case ft_mywvarchar:
    case ft_mywvarbinary:
    case ft_myyear:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    case ft_myblob:
    case ft_mygeometry:
    case ft_myjson:
    case ft_myfixedbinary:
    case ft_mytext:
        return BT_AL_LEFT;
    }
    return BT_AL_RIGHT;
}

const _TCHAR* btrVersion::moduleTypeString()
{
    switch (type)
    {
    case 'N':
        return _T("Requester");
    case 'D':
        return _T("DOS Workstation MicroKernel");
    case 'W':
        return _T("Windows Workstation MicroKernel");
    case 'O':
        return _T("OS/2 WorkStation MicroKernel");
    case '3':
    case '9':
        return _T("Windows 32Bit Workstation MicroKernel");
    case 'S':
        return _T("Netware Server MicroKernel");
    case 'T':
        return _T("Windows Server MicroKernel");
    case 'L':
        return _T("Linux Server MicroKernel");
    case 'F':
        return _T("FileSharing WorkStation MicroKernel");
    }
    return _T("Unknown Type");
}

const _TCHAR* btrVersion::moduleVersionShortString(_TCHAR* buf)
{
#pragma warning(disable : 4996)
    switch (type)
    {
    case 'N':
        _stprintf(buf, _T("Reqster %d.%d"), majorVersion, minorVersion);
        break;
    case 'D':
        _stprintf(buf, _T("DosLocal %d.%d"), majorVersion, minorVersion);
        break;
    case 'W':
        _stprintf(buf, _T("W1Local %d.%d"), majorVersion, minorVersion);
        break;
    case 'O':
        _stprintf(buf, _T("OS/2Local %d.%d"), majorVersion, minorVersion);
        break;
    case '3':
    case '9':
        _stprintf(buf, _T("W3Local %d.%d"), majorVersion, minorVersion);
        break;
    case 'S':
        _stprintf(buf, _T("NServer %d.%d"), majorVersion, minorVersion);
        break;
    case 'T':
        _stprintf(buf, _T("Server %d.%d"), majorVersion, minorVersion);
        break;
    case 'L':
        _stprintf(buf, _T("Lerver %d.%d"), majorVersion, minorVersion);
        break;
    default:
        return _T("Unknown Type");
    }
#pragma warning(default : 4996)
    return buf;
}

PACKAGE uchar_td getFilterLogicTypeCode(const _TCHAR* cmpstr)
{
    if (_tcscmp(cmpstr, _T("=")) == 0)
        return (uchar_td)eEqual;
    else if (_tcscmp(cmpstr, _T(">")) == 0)
        return (uchar_td)eGreater;
    else if (_tcscmp(cmpstr, _T("<")) == 0)
        return (uchar_td)eLess;
    else if (_tcscmp(cmpstr, _T("<>")) == 0)
        return (uchar_td)eNotEq;
    else if (_tcscmp(cmpstr, _T("=>")) == 0)
        return (uchar_td)eGreaterEq;
    else if (_tcscmp(cmpstr, _T(">=")) == 0)
        return (uchar_td)eGreaterEq;
    else if (_tcscmp(cmpstr, _T("=<")) == 0)
        return (uchar_td)eLessEq;
    else if (_tcscmp(cmpstr, _T("<=")) == 0)
        return (uchar_td)eLessEq;
    else if (_tcscmp(cmpstr, _T("&")) == 0)
        return (uchar_td)eBitAnd;
    else if (_tcscmp(cmpstr, _T("!&")) == 0)
        return (uchar_td)eNotBitAnd;
    else if (_tcscmp(cmpstr, _T("=i")) == 0)
        return (uchar_td)eEqual | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T(">i")) == 0)
        return (uchar_td)eGreater | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("<i")) == 0)
        return (uchar_td)eLess | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("<>i")) == 0)
        return (uchar_td)eNotEq | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("=>i")) == 0)
        return (uchar_td)eGreaterEq | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T(">=i")) == 0)
        return (uchar_td)eGreaterEq | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("=<i")) == 0)
        return (uchar_td)eLessEq | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("<=i")) == 0)
        return (uchar_td)eLessEq | CMPLOGICAL_CASEINSENSITIVE;
    else if (_tcscmp(cmpstr, _T("<==>")) == 0)
        return (uchar_td)eIsNull;
    else if (_tcscmp(cmpstr, _T("<!=>")) == 0)
        return (uchar_td)eIsNotNull;
    return 255;
}

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
