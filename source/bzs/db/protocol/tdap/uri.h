#ifndef BZS_DB_PROTOCOL_TDAP_URI_H
#define BZS_DB_PROTOCOL_TDAP_URI_H
/* =================================================================
 Copyright (C) 2015 BizStation Corp All rights reserved.

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
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>
#include <tchar.h>
#include <boost/version.hpp>

#if BOOST_VERSION > 103901
#include <boost/uuid/sha1.hpp>
#else
#include <global/boost/sha1.hpp>
#endif

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

inline const _TCHAR* hostName(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("@"));
    if (st) 
        ++st;
    else
    {
        st = _tcsstr(uri, _T("://"));
        if (st) st+=3;
    }
    if (st)
    {
        const _TCHAR* en = _tcsstr(st, _T("/"));
        if (en)
        {
            _tcsncpy_s(buf, size, st, en - st);
            buf[en - st] = 0x00;
        }
    }
    return buf;
}

inline const _TCHAR* dbname(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("://"));
    if (st)
    {
        st = _tcsstr(st + 3, _T("/"));
        if (st)
        {
            const _TCHAR* en = _tcsstr(st + 1, _T("?"));
            if (en)
            {
                _tcsncpy_s(buf, size, st + 1, en - (st + 1));
                buf[en - (st + 1)] = 0x00;
            }
        }
    }
    return buf;
}

inline const _TCHAR* schemaTable(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsrchr(uri, _T('='));
    if (st)
    {
        const _TCHAR* en = _tcsrchr(uri, _T('.'));
        if (en)
        {
            _tcsncpy_s(buf, size, st + 1, en - (st + 1));
            buf[en - (st + 1)] = 0x00;
        }
    }
    return buf;
}

inline const _TCHAR* userName(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("://"));
    if (st) st+=3;
 
    if (st)
    {
        const _TCHAR* en = _tcsstr(st, _T("@"));
        if (en)
        {
            _tcsncpy_s(buf, size, st, en - st);
            buf[en - st] = 0x00;
        }
    }
    return buf;
}

inline const _TCHAR* passwd(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("pwd="));
    if (st)
        _tcscpy_s(buf, size, st+4);
    return buf;
}

inline const _TCHAR* stripAuth(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    _tcscpy_s(buf, size, uri);
    _TCHAR* st = _tcsstr(buf, _T("://"));
    if (st)
    {
        const _TCHAR* en = _tcsstr(uri, _T("@"));
        if (en)
        {
            _tcscpy_s(st+3, size, ++en);
            _TCHAR* st2 = _tcsstr(st, _T("&pwd="));
            if (st2)
                *st2 = 0x00;
            else
            {
                st2 = _tcsstr(st, _T("?pwd="));
                if (st2) *st2 = 0x00;
            }
        }
    }
    return buf;
}

inline const _TCHAR* stripPasswd(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    _tcscpy_s(buf, size, uri);
    _TCHAR* st = _tcsstr(buf, _T("://"));
    if (st)
    {
        st = _tcsstr(st, _T("pwd="));
        if (st)
            *(st + 4) = 0x00;
    }
    return buf;
}

inline void reorder_digest(boost::uuids::detail::sha1::digest_type val)
{
    for (int i=0;i<5;++i)
    {
        unsigned char* p = (unsigned char*)&val[i];
        std::swap(*p, *(p+3));
        std::swap(*(p+1), *(p+2));
    }
}

inline void sha1Passwd(boost::uuids::detail::sha1::digest_type retVal , 
                                    const char* src)
{
    boost::uuids::detail::sha1 sha1;
    sha1.reset();
    sha1.process_bytes(src, strlen(src));
    sha1.get_digest(retVal);
    reorder_digest(retVal);
}

inline void cryptKeyGen(boost::uuids::detail::sha1::digest_type retVal, 
             const unsigned int* sha1Passwd, const unsigned char *scramble)
{
    boost::uuids::detail::sha1 sha1;
    sha1.reset();
    sha1.process_bytes(sha1Passwd, MYSQL_SCRAMBLE_LENGTH);
    sha1.get_digest(retVal);
    reorder_digest(retVal);
    sha1.reset();
    sha1.process_bytes(scramble, MYSQL_SCRAMBLE_LENGTH);
    sha1.process_bytes(retVal, MYSQL_SCRAMBLE_LENGTH);
    sha1.get_digest(retVal);
    reorder_digest(retVal);
}

inline void xorCrypt(char *retVal, const unsigned char *src, const unsigned char *key, 
        unsigned int len)
{
    const unsigned char *end = src + len;
    while (src < end)
        *retVal++ = *src++ ^ *key++;
}

/* scramble size = MYSQL_SCRAMBLE_LENGTH only 
   retVal size = MYSQL_SCRAMBLE_LENGTH
*/
inline void mysqlCryptPwd(char *retVal, const char* src, const unsigned char *scramble)
{
    unsigned int sha1pwd[5];
    unsigned int key[5];
    sha1Passwd(sha1pwd, src);
    cryptKeyGen(key, sha1pwd, scramble);
    xorCrypt(retVal, (const unsigned char*)sha1pwd, (const unsigned char*)key, 
                        MYSQL_SCRAMBLE_LENGTH);
}

// in 20byte out 40byte + 1
/*inline char* binToHexSha1(char* retVal, const unsigned char *src)
{
    const unsigned char* end = src + MYSQL_SCRAMBLE_LENGTH;
    char* p = retVal;
    for (unsigned char* n = src; n != end; ++n)
    {
	    unsigned int v = *n / 16;
	    *p++ = (v < 10) ? '0' + v : *p = 'A';
	    v = *n % 16;
	    *p++ = (v < 10) ? '0' + v : *p = 'A';
    }
    *p = 0x00;
    return retVal;
}*/


} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_URI_H
