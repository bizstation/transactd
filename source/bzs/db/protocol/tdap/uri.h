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
#include <bzs/db/protocol/tdap/tdapcapi.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

/** @cond INTERNAL */

inline const _TCHAR* protocol(const _TCHAR* uri)
{
    const _TCHAR* st = _tcsstr(uri, _T("tdap://"));
    if (st)
        return _T("tdap");
    else
    {
        st = _tcsstr(uri, _T("btrv://"));
        if (st)
            return _T("btrv");
    }
    return NULL;
}

inline void endPoint(const _TCHAR* uri, 
                                _TCHAR* host, size_t hostSize, 
                                _TCHAR* port, size_t portSize)
{
    if (host) host[0] = 0x00;
    if (port) port[0] = 0x00;
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
        const _TCHAR* en = _tcsstr(st, _T(":"));
        if (en && en > st)
        {
            if (host)
            {
                _tcsncpy_s(host, hostSize, st, en - st);
                host[en - st] = 0x00;
            }
            if (port)
            {
                st = en + 1;
                en = _tcsstr(st, _T("/"));
                if (en && en > st)
                {
                    _tcsncpy_s(port, portSize, st, en - st);
                    port[en - st] = 0x00;
                }
            }
        }
        else if (host)
        {
            en = _tcsstr(st, _T("/"));
            if (en && en > st)
            {
                _tcsncpy_s(host, hostSize, st, en - st);
                host[en - st] = 0x00;
            }
        }
    }
}

inline const _TCHAR* hostName(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    endPoint(uri, buf, size, NULL, 0);
    return buf;
}

inline const _TCHAR* port(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    endPoint(uri, NULL, 0, buf, size);
    return buf;
}

inline const _TCHAR* dbname(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("://"));
    if (st)
    {
        st = _tcsstr(st + 3, _T("/"));
        if (st && *(++st))
        {
            const _TCHAR* en = _tcsstr(st, _T("?"));
            if (!en)
                en = _tcslen(st) + st;
            if (en && en > st)
            {
                _tcsncpy_s(buf, size, st, en - st);
                buf[en - st] = 0x00;
                int n = (int)(en - st -1);
                if (n >= 0 && buf[n] == '/') buf[n] = 0x00;
            }
        }
    }
    return buf;
}

inline const _TCHAR* schemaTable(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    _TCHAR* st = _tcsstr((_TCHAR*)uri, _T("dbfile="));
    if (st)
    {
        st+= 7;
        _tcscpy_s(buf, size, st);
        st = _tcschr(buf, _T('&'));
        if (st) *st = 0x00;
        st = buf;
        const _TCHAR* en = _tcsrchr(st, _T('.'));
        if (en && en > st)
            buf[en - st] = 0x00;
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
        if (en && en > st)
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
    _TCHAR* st = _tcsstr((_TCHAR*)uri, _T("pwd="));
    if (st)
    {
        _tcscpy_s(buf, size, st+4);
        st = _tcschr(buf, _T('&'));
        if (st) *st = 0x00;
    }
    return buf;
}

inline const _TCHAR* connectName(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    _TCHAR pwd[MAX_PATH];
    passwd(uri, pwd, MAX_PATH);

    buf[0] = 0x00;
    const _TCHAR* st = _tcsstr(uri, _T("://"));
    if (st)
    {
        st = _tcsstr(st + 3, _T("/"));
        _tcsncpy_s(buf, size, uri, ++st - uri);
        buf[st - uri] = 0x00;
        if (pwd[0])
        {
            _tcscat_s(buf, MAX_PATH, _T("?pwd="));
            _tcscat_s(buf, MAX_PATH, pwd);
        }

    }
    return buf;
}

inline _TCHAR* stripParam(const _TCHAR* uri, _TCHAR* buf, size_t size)
{
    buf[0] = 0x00;
    _TCHAR* st = _tcsstr((_TCHAR*)uri, _T("://"));
    if (st)
    {
        st = _tcsstr(st + 3, _T("/"));
        if (st && *(++st))
        {
            _tcscpy_s(buf, size, uri);
            _TCHAR* en = _tcschr(buf, _T('?'));
            if (en) *en = 0x00;
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
                if (st2) *(st2 + 1) = 0x00;
            }
        }
    }
    return buf;
}

inline const _TCHAR* appendAuth(const _TCHAR* uri, const _TCHAR* user, const _TCHAR* passwd,
                                    _TCHAR* buf, size_t size)
{
    memset(buf, 0, size);
    if (_tcslen(uri) + _tcslen(user) + _tcslen(passwd) + _tcslen(_T("@&pwd=")) > size -1)
        return buf;

    const  _TCHAR* st = _tcsstr(uri, _T("://"));
    _tcsncpy_s(buf, size, uri, st + 3 - uri);
    _tcscat_s(buf, size, user);
    _tcscat_s(buf, size, _T("@"));
    _tcscat_s(buf, size, st+3);
    _tcscat_s(buf, size, _T("&pwd="));
    _tcscat_s(buf, size, passwd);
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

// binary to ascii return bytes = size * 2
inline char* binToHex(char* retVal, const unsigned char *src, int size)
{
    const unsigned char* end = src + size;
    char* p = retVal;
    for (const unsigned char* n = src; n != end; ++n)
    {
	    unsigned int v = (*n >> 4);
	    *p++ = (v < 10) ? '0' + v : 'A' + (v - 10);
	    v = *n & 0x0F;
	    *p++ = (v < 10) ? '0' + v : 'A' + (v - 10);
    }
    *p = 0x00;
    return retVal;
}

// ascii to binary return bytes = size / 2
inline unsigned char* hexTobin(unsigned char* retVal, const char *src, int size)
{
    const char* end = src + size;
    unsigned char* p = retVal;
    for (const char* n = src; n != end; ++n)
    {
        if (*n >= '0' && *n <= '9')
            *p = *n - '0';
        else
            *p = (*n - 'A')+10;
        ++n;
        *p = *p << 4;
        if (*n >= '0' && *n <= '9')
            *p += *n - '0';
        else
            *p += (*n - 'A')+10;
        ++p;
    }
    return retVal;
}

/** @endcond */

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_URI_H
