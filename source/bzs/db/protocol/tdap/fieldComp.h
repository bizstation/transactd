#ifndef BZS_DB_PROTOCOL_TDAP_FIELD_COMP_H
#define BZS_DB_PROTOCOL_TDAP_FIELD_COMP_H
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
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/env/crosscompile.h>
#include <algorithm>
#include <string.h>


inline __int64 changeEndian2(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[1];
    l[1] = r[0];
    return ret;
}

inline __int64 changeEndian3(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[2];
    l[1] = r[1];
    l[2] = r[0];
    return ret;
}

inline __int64 changeEndian4(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[3];
    l[1] = r[2];
    l[2] = r[1];
    l[3] = r[0];
    return ret;
}

inline __int64 changeEndian5(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[4];
    l[1] = r[3];
    l[2] = r[2];
    l[3] = r[1];
    l[4] = r[0];
    return ret;
}

inline __int64 changeEndian6(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[5];
    l[1] = r[4];
    l[2] = r[3];
    l[3] = r[2];
    l[4] = r[1];
    l[5] = r[0];
    return ret;
}

inline __int64 changeEndian7(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[6];
    l[1] = r[5];
    l[2] = r[4];
    l[3] = r[3];
    l[4] = r[2];
    l[5] = r[1];
    l[6] = r[0];
    return ret;
}

inline __int64 changeEndian8(__int64 v)
{
    __int64 ret = 0;
    char* l = (char*)&ret;
    char* r = (char*)&v;
    l[0] = r[7];
    l[1] = r[6];
    l[2] = r[5];
    l[3] = r[4];
    l[4] = r[3];
    l[5] = r[2];
    l[6] = r[1];
    l[7] = r[0];
    return ret;
}

inline __int64 changeEndian(__int64 v, int len)
{
    switch(len)
    {
    case 1: return v;
    case 2: return changeEndian2(v);
    case 3: return changeEndian3(v);
    case 4: return changeEndian4(v);
    case 5: return changeEndian5(v);
    case 6: return changeEndian6(v);
    case 7: return changeEndian7(v);
    case 8: return changeEndian8(v);
    }
    return v;
}

inline int int24toInt(const char* p)
{
    return ((*((int*)p) & 0xFFFFFF) << 8) / 0x100;
}

inline unsigned int int24toUint(const char* p)
{
    return *((unsigned int*)p) & 0xFFFFFF;
}

inline __int64 myBittoInt64(const char* p, int len)
{
    __int64 v = 0;
    memcpy(&v, p, len);
    return changeEndian8(v);
}

inline void storeInt24(int v, char* p)
{
    memcpy(p, &v, 3);
}

inline void storeUint24(unsigned int v, char* p)
{
    memcpy(p, &v, 3);
}

inline int compareUint24(const char* l, const char* r)
{
    unsigned int lv = int24toUint(l);
    unsigned int rv = int24toUint(r);;
    if (lv < rv)
        return -1;
    if (lv > rv)
        return 1;
    return 0;
}

inline int compareInt24(const char* l, const char* r)
{
    int lv = int24toInt(l);
    int rv = int24toInt(r);

    if (lv < rv)
        return -1;
    else if (lv > rv)
        return 1;
    return 0;
}

template <class T> inline int compare(const char* l, const char* r)
{
    if (*((T*)l) < *((T*)r))
        return -1;
    else if (*((T*)l) > *((T*)r))
        return 1;
    return 0;
}

template <class T> inline int bitMask(const char* l, const char* r)
{
    T v = *((T*)l) & *((T*)r);
    v = (*((T*)r) - v);
    /* 
       When T is __int64 then v is incoreect value. 
       Because return size is int.    
    */
    return  (v > 0) ? 1 : ((v < 0) ? -1 : 0); 
}

template <class T> inline int compare(T l, T r)
{
    if (l < r)
        return -1;
    else if (l > r)
        return 1;
    return 0;
}

template <class T>
inline int compareVartype(const char* l, const char* r, bool bin, bool all, bool incase)
{
    int llen = (*(T*)l);
    int rlen = (*(T*)r);
    int tmp = std::min<int>(llen, rlen);
    if (incase)
        tmp = _strnicmp(l + sizeof(T), r + sizeof(T), tmp);
    else if (bin)
        tmp = memcmp(l + sizeof(T), r + sizeof(T), tmp);
    else
        tmp = strncmp(l + sizeof(T), r + sizeof(T), tmp);

    if (all)
        return (tmp == 0) ? compare<int>(llen, rlen) : tmp; // match complete
    return (tmp == 0 && (llen < rlen)) ? -1 : tmp; // match a part
}

template <class T>
inline int compareWvartype(const char* l, const char* r, bool bin, bool all, bool incase)
{
    int llen = (*(T*)l) / sizeof(char16_t);
    int rlen = (*(T*)r) / sizeof(char16_t);
    int tmp = std::min<int>(llen, rlen);
    if (incase)
        tmp = wcsnicmp16((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)),
                         tmp);
    else if (bin)
        tmp =
            wmemcmp16((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)), tmp);
    else
        tmp = wcsncmp16((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)),
                        tmp);
    if (all)
        return (tmp == 0) ? compare<int>(llen, rlen) : tmp; // match complete
    return (tmp == 0 && (llen < rlen)) ? -1 : tmp; // match a part
}

inline int compareBlobType(const char* l, const char* r, bool bin, bool all, bool incase,
                           int sizeByte)
{
    int llen = 0;
    int rlen = 0;
    memcpy(&llen, l, sizeByte);
    memcpy(&rlen, r, sizeByte);
    int tmp = std::min<int>(llen, rlen);
    const char* lptr = *((const char**)(l + sizeByte));
    const char* rptr = r + sizeByte;
    if (incase)
        tmp = _strnicmp(lptr, rptr, tmp);
    else if (bin)
        tmp = memcmp(lptr, rptr, tmp);
    else
        tmp = strncmp(lptr, rptr, tmp);

    if (all)
        return (tmp == 0) ? compare<int>(llen, rlen) : tmp;
    return (tmp == 0 && (llen < rlen)) ? -1 : tmp;
}

inline int compareBlobType2(const char* l, const char* r, bool bin, bool all, bool incase,
                           int sizeByte)
{
    int llen = 0;
    int rlen = 0;
    memcpy(&llen, l, sizeByte);
    memcpy(&rlen, r, sizeByte);
    int tmp = std::min<int>(llen, rlen);
    const char* lptr = *((const char**)(l + sizeByte));
    const char* rptr = *((const char**)(r + sizeByte));
    if (incase)
        tmp = _strnicmp(lptr, rptr, tmp);
    else if (bin)
        tmp = memcmp(lptr, rptr, tmp);
    else
        tmp = strncmp(lptr, rptr, tmp);

    if (all)
        return (tmp == 0) ? compare<int>(llen, rlen) : tmp;
    return (tmp == 0 && (llen < rlen)) ? -1 : tmp;
}
/* int nullComp(bool lnull, bool rnull, char log)
    
    lnull rnull log        ret 
    -----------------------------------------------
    true  true  isNull      0
    true  true  isNotNull   -1
    true  false 0           -1
    false true  isNull      1
    false true  isNotNull   0
    false false 0           2
    -----------------------------------------------

    real value example

    lval  rval        ret
    -----------------------------------------------
    NULL  isNull      0
    NULL  isNotNull  -1
    NULL  2          -1
    1     isNull      1
    1     isNotNull   0
    1     2           2
    -----------------------------------------------
*/ 
inline int nullComp(bool lnull, bool rnull, char log)
{
    if (lnull)
        return (log == eIsNull) ? 0 : -1;
    else if (rnull)
        return (log == eIsNull) ? 1 : 0;
    return 2;
}

template <class T>
inline int compBitAnd(const char* l, const char* r, int len)
{
    return bitMask<T>(l, r);
}

inline int compBitAnd24(const char* l, const char* r, int len)
{
    int lv = int24toInt(l);
    int rv = int24toInt(r);
    return bitMask<int>((const char*)&lv, (const char*)&rv);
}

inline int compBitAnd64(const char* l, const char* r, int len) 
{
    __int64 lv = 0;
    __int64 rv = 0;
    memcpy(&lv, l, len);
    memcpy(&rv, r, len);
    return bitMask<__int64>((const char*)&lv, (const char*)&rv);
}

template <class T>
inline int compNumber(const char* l, const char* r, int len)
{
    return compare<T>(l, r);
}

inline int compNumber24(const char* l, const char* r, int len)
{
    return compareInt24(l, r);
}

inline int compNumberU24(const char* l, const char* r, int len)
{
    return compareUint24(l, r);
}

inline int compMem(const char* l, const char* r, int len)
{
    return memcmp(l, r, len);
}

inline int compString(const char* l, const char* r, int len) 
{
    return strncmp(l, r, len);
}

inline int compiString(const char* l, const char* r, int len) 
{
    return _strnicmp(l, r, len);
}

inline int compWString(const char* l, const char* r, int len) 
{
    return wcsncmp16((char16_t*)l, (char16_t*)r, len/2);
}

inline int compiWString(const char* l, const char* r, int len)
{
    return wcsnicmp16((char16_t*)l, (char16_t*)r, len/2);
}

#define T_BIN true
#define T_STR false

#define T_INCASE true
#define T_CASE false

#define T_CMP_ALL true
#define T_CMP_PART false


template <class T, bool all>
inline int compVarString(const char* l, const char* r, int len)
{
    return compareVartype<T>(l, r, T_STR, all, T_CASE);
}

template <class T, bool all>
inline int compVarString_bin(const char* l, const char* r, int len)
{
    return compareVartype<T>(l, r, T_BIN, all, T_CASE);
}
template <class T, bool all>
inline int compVarString_i(const char* l, const char* r, int len)
{
    return compareVartype<T>(l, r, T_STR, all, T_INCASE);
}

template <class T, bool all>
inline int compVarString_bin_i(const char* l, const char* r, int len)
{
    return compareVartype<T>(l, r, T_BIN, all, T_INCASE);
}

template <class T, bool all>
inline int compWVarString(const char* l, const char* r, int len)
{
    return compareWvartype<T>(l, r, T_STR, all, T_CASE);
}

template <class T, bool all>
inline int compWVarString_bin(const char* l, const char* r, int len) 
{
    return compareWvartype<T>(l, r, T_BIN, all, T_CASE);
}

template <class T, bool all>
inline int compWVarString_i(const char* l, const char* r, int len)
{
    return compareWvartype<T>(l, r, T_STR, all, T_INCASE);
}

template <class T, bool all>
inline int compWVarString_bin_i(const char* l, const char* r, int len) 
{
    return compareWvartype<T>(l, r, T_BIN, all, T_INCASE);
}

template <int sizeByte, bool all>
inline int compBlob(const char* l, const char* r, int len)
{
    return compareBlobType(l, r, T_STR, all, T_CASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob_bin(const char* l, const char* r, int len)
{
    return compareBlobType(l, r, T_BIN, all, T_CASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob_i(const char* l, const char* r, int len)
{
    return compareBlobType(l, r, T_STR, all, T_INCASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob_bin_i(const char* l, const char* r, int len)
{
    return compareBlobType(l, r, T_BIN, all, T_INCASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob2(const char* l, const char* r, int len)
{
    return compareBlobType2(l, r, T_STR, all, T_CASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob2_bin(const char* l, const char* r, int len)
{
    return compareBlobType2(l, r, T_BIN, all, T_CASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob2_i(const char* l, const char* r, int len)
{
    return compareBlobType2(l, r, T_STR, all, T_INCASE, sizeByte);
}

template <int sizeByte, bool all>
inline int compBlob2_bin_i(const char* l, const char* r, int len)
{
    return compareBlobType2(l, r, T_BIN, all, T_INCASE, sizeByte);
}


typedef int (*comp1Func)(const char* l, const char* r,int len);

typedef bool (*judgeFunc)(int);

inline comp1Func getCompFunc(uchar_td type, ushort_td len, char logType, int sizeByte)
{
    bool compAll = (logType & CMPLOGICAL_VAR_COMP_ALL) != 0;
    bool incase = (logType & CMPLOGICAL_CASEINSENSITIVE) != 0;
    switch (type)
    {
    case ft_integer:
    case ft_autoinc:
    case ft_currency:
    {
        if (logType & (char)eBitAnd)
        {
            switch (len)
            {
            case 1:
                return &compBitAnd<char>;
            case 2:
                return &compBitAnd<short>;
            case 3:
                return &compBitAnd24;
            case 4:
                return &compBitAnd<int>;
            case 8:
                return &compBitAnd<__int64>;
            }
        }else
        {
            switch (len)
            {
            case 1:
                return &compNumber<char>;
            case 2:
                return &compNumber<short>;
            case 3:
                return &compNumber24;
            case 4:
                return &compNumber<int>;
            case 8:
                return &compNumber<__int64>;
            }
        }
    }
    case ft_mychar:
    case ft_string:
        if (incase)
            return &compiString;
        return &compMem;
    case ft_zstring:
    case ft_note:
        if (incase)
            return &compiString;
        return &compString;
    case ft_logical:
    case ft_uinteger:
    case ft_autoIncUnsigned:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_myyear:
    case ft_mydate:
    case ft_mytime_num_cmp:
    case ft_mydatetime_num_cmp:
    case ft_mytimestamp_num_cmp:
    case ft_set:
    case ft_enum:
    {
        if (logType & (char)eBitAnd)
        {
            switch (len)
            {
            case 1:
                return &compBitAnd<char>;
            case 2:
                return &compBitAnd<short>;
            case 3:
                return &compBitAnd24;
            case 4:
                return &compBitAnd<int>;
            case 8:
                return &compBitAnd<__int64>;
            }
        }else
        {
            switch (len)
            {
            case 1:
                return &compNumber<unsigned char>;
            case 2:
                return &compNumber<unsigned short>;
            case 3:
                return &compNumberU24;
            case 4:
                return &compNumber<unsigned int>;
            case 8:
                return &compNumber<unsigned __int64>;
            }
        }
    }
    case ft_bit:
        if (logType & (char)eBitAnd)
            return &compBitAnd64;
        return &compMem;
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    case ft_mydecimal:
        return &compMem;
    case ft_float:
        switch (len)
        {
        case 4:
            return &compNumber<float>;
        case 8:
            return &compNumber<double>;
        }
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
        if (incase)
            return &compiWString;
        if ((type == ft_wstring) || (type == ft_mywchar))
            return &compMem;
        return &compWString;
    case ft_lstring:
    case ft_myvarchar:
        if (sizeByte == 1)
        {
            if (incase)
            {
                if (compAll)
                    return &compVarString_i<unsigned char, T_CMP_ALL>;
                return &compVarString_i<unsigned char, T_CMP_PART>;
            }else
            {
                if (compAll)
                    return &compVarString<unsigned char, T_CMP_ALL>;
                return &compVarString<unsigned char, T_CMP_PART>;
            }
        }
        if (incase)
        {
            if (compAll)
                return &compVarString_i<unsigned short, T_CMP_ALL>;
            return &compVarString_i<unsigned short, T_CMP_PART>;
        }
        if (compAll)
            return &compVarString<unsigned short, T_CMP_ALL>;
        return &compVarString<unsigned short, T_CMP_PART>;
    case ft_myvarbinary:
        if (sizeByte == 1)
        {
            if (incase)
            {
                if (compAll)
                    return &compVarString_bin_i<unsigned char, T_CMP_ALL>;
                return &compVarString_bin_i<unsigned char, T_CMP_PART>;
            }
            if (compAll)
                return &compVarString_bin<unsigned char, T_CMP_ALL>;
            return &compVarString_bin<unsigned char, T_CMP_PART>;
        }
        if (incase)
        {
            if (compAll)
                return &compVarString_bin_i<unsigned short, T_CMP_ALL>;
            return &compVarString_bin_i<unsigned short, T_CMP_PART>;
        }
        if (compAll)
            return &compVarString_bin<unsigned short, T_CMP_ALL>;
        return &compVarString_bin<unsigned short, T_CMP_PART>;
    case ft_mywvarchar:
        if (sizeByte == 1)
        {
            if (incase)
            {
                if (compAll)
                    return &compWVarString_i<unsigned char, T_CMP_ALL>;
                return &compWVarString_i<unsigned char, T_CMP_PART>;
            }
            if (compAll)
                return &compWVarString<unsigned char, T_CMP_ALL>;
            return &compWVarString<unsigned char, T_CMP_PART>;
        }
        if (incase)
        {
            if (compAll)
                return &compWVarString_i<unsigned short, T_CMP_ALL>;
            return &compWVarString_i<unsigned short, T_CMP_PART>;
        }
        if (compAll)
            return &compWVarString<unsigned short, T_CMP_ALL>;
        return &compWVarString<unsigned short, T_CMP_PART>;
    case ft_mywvarbinary:
        if (sizeByte == 1)
        {
            if (incase)
            {
                if (compAll)
                    return &compWVarString_bin_i<unsigned char, T_CMP_ALL>;
                return &compWVarString_bin_i<unsigned char, T_CMP_PART>;
            }
            if (compAll)
                return &compWVarString_bin<unsigned char, T_CMP_ALL>;
            return &compWVarString_bin<unsigned char, T_CMP_PART>;
        }
        if (incase)
        {
            if (compAll)
                return &compWVarString_bin_i<unsigned short, T_CMP_ALL>;
            return &compWVarString_bin_i<unsigned short, T_CMP_PART>;
        }
        if (compAll)
            return &compWVarString_bin<unsigned short, T_CMP_ALL>;
        return &compWVarString_bin<unsigned short, T_CMP_PART>;
    case ft_mytext:
    {
        bool rblob =  (logType & CMPLOGICAL_FIELD) != 0;
        if (rblob)
        {
            if (compAll)
            {
                if (incase)
                {
                    switch(sizeByte)
                    {
                    case 1:return &compBlob2_i<1, T_CMP_ALL>;
                    case 2:return &compBlob2_i<2, T_CMP_ALL>;
                    case 3:return &compBlob2_i<3, T_CMP_ALL>;
                    case 4:return &compBlob2_i<4, T_CMP_ALL>;
                    }
                }
                switch(sizeByte)
                {
                case 1:return &compBlob2<1, T_CMP_ALL>;
                case 2:return &compBlob2<2, T_CMP_ALL>;
                case 3:return &compBlob2<3, T_CMP_ALL>;
                case 4:return &compBlob2<4, T_CMP_ALL>;
                }
            }
            if (incase)
            {
                switch(sizeByte)
                {
                case 1:return &compBlob2_i<1, T_CMP_PART>;
                case 2:return &compBlob2_i<2, T_CMP_PART>;
                case 3:return &compBlob2_i<3, T_CMP_PART>;
                case 4:return &compBlob2_i<4, T_CMP_PART>;
                }
            }
            switch(sizeByte)
            {
            case 1:return &compBlob2<1, T_CMP_PART>;
            case 2:return &compBlob2<2, T_CMP_PART>;
            case 3:return &compBlob2<3, T_CMP_PART>;
            case 4:return &compBlob2<4, T_CMP_PART>;
            }
        }else
        {
            if (compAll)
            {
                if (incase)
                {
                    switch(sizeByte)
                    {
                    case 1:return &compBlob_i<1, T_CMP_ALL>;
                    case 2:return &compBlob_i<2, T_CMP_ALL>;
                    case 3:return &compBlob_i<3, T_CMP_ALL>;
                    case 4:return &compBlob_i<4, T_CMP_ALL>;
                    }
                }
                switch(sizeByte)
                {
                case 1:return &compBlob<1, T_CMP_ALL>;
                case 2:return &compBlob<2, T_CMP_ALL>;
                case 3:return &compBlob<3, T_CMP_ALL>;
                case 4:return &compBlob<4, T_CMP_ALL>;
                }
            }
            if (incase)
            {
                switch(sizeByte)
                {
                case 1:return &compBlob_i<1, T_CMP_PART>;
                case 2:return &compBlob_i<2, T_CMP_PART>;
                case 3:return &compBlob_i<3, T_CMP_PART>;
                case 4:return &compBlob_i<4, T_CMP_PART>;
                }
            }
            switch(sizeByte)
            {
            case 1:return &compBlob<1, T_CMP_PART>;
            case 2:return &compBlob<2, T_CMP_PART>;
            case 3:return &compBlob<3, T_CMP_PART>;
            case 4:return &compBlob<4, T_CMP_PART>;
            }
        }
    }
    case ft_myblob:
    case ft_myjson:      //TODO Json binary comp
    case ft_mygeometry:  //TODO geometory binary comp
    {
        bool rblob =  (logType & CMPLOGICAL_FIELD) != 0;
        if (rblob)
        {
            if (compAll)
            {
                if (incase)
                {
                    switch(sizeByte)
                    {
                    case 1:return &compBlob2_bin_i<1, T_CMP_ALL>;
                    case 2:return &compBlob2_bin_i<2, T_CMP_ALL>;
                    case 3:return &compBlob2_bin_i<3, T_CMP_ALL>;
                    case 4:return &compBlob2_bin_i<4, T_CMP_ALL>;
                    }
                }
                switch(sizeByte)
                {
                case 1:return &compBlob2_bin<1, T_CMP_ALL>;
                case 2:return &compBlob2_bin<2, T_CMP_ALL>;
                case 3:return &compBlob2_bin<3, T_CMP_ALL>;
                case 4:return &compBlob2_bin<4, T_CMP_ALL>;
                }
            }
            if (incase)
            {
                switch(sizeByte)
                {
                case 1:return &compBlob2_bin_i<1, T_CMP_PART>;
                case 2:return &compBlob2_bin_i<2, T_CMP_PART>;
                case 3:return &compBlob2_bin_i<3, T_CMP_PART>;
                case 4:return &compBlob2_bin_i<4, T_CMP_PART>;
                }
            }
            switch(sizeByte)
            {
            case 1:return &compBlob2_bin<1, T_CMP_PART>;
            case 2:return &compBlob2_bin<2, T_CMP_PART>;
            case 3:return &compBlob2_bin<3, T_CMP_PART>;
            case 4:return &compBlob2_bin<4, T_CMP_PART>;
            }
        }
        else
        {
            if (compAll)
            {
                if (incase)
                {
                    switch(sizeByte)
                    {
                    case 1:return &compBlob_bin_i<1, T_CMP_ALL>;
                    case 2:return &compBlob_bin_i<2, T_CMP_ALL>;
                    case 3:return &compBlob_bin_i<3, T_CMP_ALL>;
                    case 4:return &compBlob_bin_i<4, T_CMP_ALL>;
                    }
                }
                switch(sizeByte)
                {
                case 1:return &compBlob_bin<1, T_CMP_ALL>;
                case 2:return &compBlob_bin<2, T_CMP_ALL>;
                case 3:return &compBlob_bin<3, T_CMP_ALL>;
                case 4:return &compBlob_bin<4, T_CMP_ALL>;
                }
            }
            if (incase)
            {
                switch(sizeByte)
                {
                case 1:return &compBlob_bin_i<1, T_CMP_PART>;
                case 2:return &compBlob_bin_i<2, T_CMP_PART>;
                case 3:return &compBlob_bin_i<3, T_CMP_PART>;
                case 4:return &compBlob_bin_i<4, T_CMP_PART>;
                }
            }
            switch(sizeByte)
            {
            case 1:return &compBlob_bin<1, T_CMP_PART>;
            case 2:return &compBlob_bin<2, T_CMP_PART>;
            case 3:return &compBlob_bin<3, T_CMP_PART>;
            case 4:return &compBlob_bin<4, T_CMP_PART>;
            }
        }
        assert(0);
    }
    }
    return NULL;
}

inline bool isMatch1(int v) // eEqual eBitAnd
{
    return (v == 0);
}

inline bool isMatch2(int v) // eGreater
{
    return (v > 0);
}

inline bool isMatch3(int v) // eLess
{
    return (v < 0);
}

inline bool isMatch4(int v) // eNotEq eNotBitAnd
{
    return (v != 0);
}

inline bool isMatch5(int v) // eGreaterEq
{
    return (v >= 0);
}

inline bool isMatch6(int v) // eLessEq
{
    return (v <= 0);
}

inline judgeFunc getJudgeFunc(eCompType log)
{
    switch (log & 0xF)
    {
    case eEqual:
    case eBitAnd: 
        return isMatch1;
    case eGreater: 
        return isMatch2;
    case eLess: 
        return isMatch3;
    case eNotEq: 
    case eNotBitAnd:
        return isMatch4;
    case eGreaterEq:
        return isMatch5;
    case eLessEq: 
        return isMatch6;
    case eIsNull:
    case eIsNotNull:
        return NULL;
    }
    assert(0);
    return NULL;
}

inline bool isEndComp(uchar_td opr, bool ret)
{
    return (opr == eCend) || (!ret && (opr == eCand)) || (ret && (opr == eCor));
}

#endif
