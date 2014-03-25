#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_STRINGCONVERTER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_STRINGCONVERTER_H
/* =================================================================
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
 ================================================================= */
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/rtl/stringBuffers.h>
#include <bzs/env/crosscompile.h>
#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>
#endif
#include <limits.h>
#include <assert.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

typedef int(*isMbcleadFunc)(unsigned int c);

inline size_t charNumByteUtf8(const unsigned char *p, size_t isize, size_t num)
{
    int n = 0;
    const unsigned char* end = p + isize;
    while (num && (p != end))
    {
        --num;

        if ((*p & 0x80) == 0)
        {
            ++n;
            ++p; // 1byte string
        }
        else
        {
            for (unsigned char tmp = *p & 0xfc; (tmp & 0x80); tmp = tmp << 1)
            {
                ++n;
				if (++p == end)
					break;
            }
        }
    }
    return n;
}

inline size_t charNumByte(isMbcleadFunc func, const unsigned char* p, size_t isize, size_t num)
{
    size_t n = 0;
    const unsigned char* end = p + isize;
    while (num && (p != end))
    {
        if (func(*p))
        {
            ++n;
            ++p;
        }
        ++p;
        ++n;
        --num;
    }
    return n;
}

/** Trim or fill fc charctor and return bytes that not include fill char bytes.
 */
inline size_t validateTrim(isMbcleadFunc func, unsigned char* src, size_t maxBytes, int fc)
{
    unsigned char* p = src + maxBytes - 1;
    unsigned char* tmp = p;
    if (fc == -1)
        fc = 0x00;
    while (func(*tmp) && (--tmp>=src));

    //if (func(*p) && (maxBytes>1) && !func(*(p-1)))
    if ((p-tmp) % 2)
    {
        *p = (unsigned char)fc;
        return maxBytes - 1;
    }
    return maxBytes;

}

/** UTF8 Version: Trim or fill fc charctor and return bytes that not include fill char bytes.
 */
inline size_t validateTrimUTF8(unsigned char* src, size_t maxBytes, int fc)
{
    unsigned char* endpos = src + maxBytes;
    unsigned char* p = endpos - 1;

    if ((*p & 0x80) == 0)
        return maxBytes;

    //If multi byte string then first byte is 0xC0.
    while ((*p & 0xC0) != 0xC0)
    {
        if (--p < src)
            break;
    }
    size_t num = endpos - p;

    unsigned char tmp = *p;
    unsigned int num2 = 0;
    while (tmp & 0x80)
    {
        tmp = tmp << 1;
        ++num2;
    }
    if (num == num2)
        return maxBytes;
    memset(p, fc, endpos - p);
    return p - src;
}

inline int isMbcCP932(unsigned int c) {return ((c >= 0x81) && (c <= 0x9F)) || ((c >= 0xE0) && (c <= 0xFC));
}

// Trim or fill fc charctor and return bytes.
inline size_t charNumTrim(int codePage, char* src, size_t inputsize, size_t maxCharnum, int fc)
{
    size_t size = inputsize;
    switch (codePage)
    {
    case CP_UTF8: size = charNumByteUtf8((unsigned char*)src, inputsize, maxCharnum);
        break;
    case 932: size = charNumByte(isMbcCP932, (unsigned char*)src, inputsize, maxCharnum);
        break;

    }
    if (size && (fc != -1))
        memset(src + size, fc, inputsize - size);
    return size;
}

// Wide version: Trim or fill fc charctor and return wide char number..
inline size_t charNumTrim(int codePage, WCHAR* src, size_t inputsize, size_t maxCharnum, int fc)
{
    WCHAR* end = src + inputsize;
    WCHAR* p = src;
    size_t n = 0;
    while (p != end)
    {
        if (IS_HIGH_SURROGATE(*p))
            ++p;
        ++p;
        ++n;
        --maxCharnum;
        if (maxCharnum == 0)
            break;
    }
    return n;
}

// if invalid end charctor then fill fc char. And return charctor bytes.
inline size_t validateTrim(int codePage, char* src, size_t maxlen, int fc)
{
    switch (codePage)
    {
    case CP_UTF8: return validateTrimUTF8((unsigned char*)src, maxlen, fc);
    case 932: return validateTrim(isMbcCP932, (unsigned char*)src, maxlen, fc);
    }
    return maxlen;
}

// Wide Version if invalid end charctor then fill fc char. And return wide charctor number.
inline size_t validateTrim(int codePage, WCHAR* src, size_t maxlen, int fc)
{
    if (IS_HIGH_SURROGATE(src[maxlen - 1]))
        src[--maxlen] = 0x00;
    return maxlen;
}

#define CP_UTF16 1200

class stringConverter
{
    unsigned int m_codePage;
    unsigned int m_exec_codePage;

public:
    stringConverter(unsigned int src_codPage, unsigned int exec_codePage)
        : m_codePage(src_codPage), m_exec_codePage(exec_codePage) {}

    inline void setCodePage(unsigned int src_codPage) {m_codePage = src_codPage;}

    inline void setExecCodePage(unsigned int codPage) {m_exec_codePage = codPage;}

    inline unsigned int execCodePage() const {return m_exec_codePage;};

    // wide to codepage. Result is no need Nullterminate.
    inline size_t convert(char* to, size_t tsize, const WCHAR* from, size_t fsize)
    {
        to[tsize - 1] = 0x00;
        size_t len = WideCharToMultiByte(m_codePage, (m_codePage == CP_UTF8) ? 0 :
            WC_COMPOSITECHECK, from, (int)fsize, to, (int)tsize, NULL, NULL);
        if (len == 0)
            return tsize - (to[tsize - 1] ? 0 : 1);
        return len;
    }

    inline size_t convert(WCHAR* to, size_t tsize, const char* from, size_t fsize)
    {
        to[tsize - 1] = 0x00;
        size_t len = MultiByteToWideChar(m_exec_codePage, (m_exec_codePage == CP_UTF8) ? 0 :
            MB_PRECOMPOSED, from, (int)fsize, to, (int)tsize);
        if (len == 0)
            return tsize - (to[tsize - 1] ? 0 : 1);
        return len;

    }

    inline size_t convert(WCHAR* to, size_t tsize, const WCHAR* from, size_t fsize)
    {
        assert(0);
        return fsize;
    }

    inline size_t convert(char* to, size_t tsize, const char* from, size_t fsize)
    {
        size_t size = 0;
#ifdef _WIN32
        size = MultiByteToWideChar(m_exec_codePage, (m_exec_codePage == CP_UTF8) ? 0 :
            MB_PRECOMPOSED, from, (int)fsize, NULL, (int)0);
        WCHAR* ws = new WCHAR[++size];
        size = MultiByteToWideChar(m_exec_codePage, (m_exec_codePage == CP_UTF8) ? 0 :
            MB_PRECOMPOSED, from, (int)fsize, ws, (int)size);

        to[tsize - 1] = 0x00;
        size = WideCharToMultiByte(m_codePage, (m_codePage == CP_UTF8) ? 0 : WC_COMPOSITECHECK, ws,
            (int)size, to, (int)tsize, NULL, NULL);
        delete[]ws;
        if (size == 0)
            size = tsize - (to[tsize - 1] ? 0 : 1);
#else
        if (m_exec_codePage == CP_UTF8)
            size = bzs::env::u8tombc(from, fsize, to, tsize);
        else
            size = bzs::env::mbctou8(from, fsize, to, tsize);
#endif
        return size;
    }

    // codepage to wide. Result is need Nullterminate.
    inline size_t revert(WCHAR* to, size_t tsize, const char* from, size_t fsize) {
        return MultiByteToWideChar(m_codePage, (m_codePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, from,
            (int)fsize, to, (int)tsize);}

    inline size_t revert(char* to, size_t tsize, const WCHAR* from, size_t fsize) {
        return WideCharToMultiByte(m_exec_codePage, (m_exec_codePage == CP_UTF8) ? 0 :
            WC_COMPOSITECHECK, from, (int)fsize, to, (int)tsize, NULL, NULL);}

    inline size_t revert(WCHAR* to, size_t tsize, const WCHAR* from, size_t fsize)
    {
        assert(0);
        return fsize;
    }

    inline size_t revert(char* to, size_t tsize, const char* from, size_t fsize)
    {
        size_t size = 0;
#ifdef _WIN32
        size = MultiByteToWideChar(m_codePage, (m_codePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, from,
            (int)fsize, NULL, (int)0);
        WCHAR* ws = new WCHAR[++size];
        size = MultiByteToWideChar(m_codePage, (m_codePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, from,
            (int)fsize, ws, (int)size);
        size = WideCharToMultiByte(m_exec_codePage, (m_exec_codePage == CP_UTF8) ? 0 :
            WC_COMPOSITECHECK, ws, (int)size, to, (int)tsize, NULL, NULL);
        delete[]ws;
#else
        if (m_exec_codePage == CP_UTF8)
            size = bzs::env::mbctou8(from, fsize, to, tsize);
        else
            size = bzs::env::u8tombc(from, fsize, to, tsize);
#endif
        return size;
    }

    inline bool isNeedConvert() const {return (m_codePage != m_exec_codePage);}

};

typedef stringConverter converter_type;

class myCharStoreBase
{
    const fielddef& m_fd;

public:
    inline myCharStoreBase(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len;};

    inline int padChar() const {return 0x20;};

    inline int storeOffsetBytes() const {return 0;};

    inline int maxCharNum() const {return m_fd.charNum();}

    inline bool isNeedReadCopy() const {return true;}

};

typedef myCharStoreBase myWcharStore;
typedef myCharStoreBase myCharStore;

class myVarCharStoreBase
{
    const fielddef& m_fd;

public:
    inline myVarCharStoreBase(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len - m_fd.varLenBytes();};

    inline int padChar() const {return -1;};

    inline int storeOffsetBytes() const {return m_fd.varLenBytes();};

    inline int maxCharNum() const {return m_fd.charNum();}

    inline bool isNeedReadCopy() const {return true;}

};

typedef myVarCharStoreBase myWvarCharStore;
typedef myVarCharStoreBase myVarCharStore;


class myVarBinaryStoreBase
{
    const fielddef& m_fd;

public:
    inline myVarBinaryStoreBase(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len - m_fd.varLenBytes();};

    inline int padChar() const {return -1;};

    inline int storeOffsetBytes() const {return m_fd.varLenBytes();};

    inline int maxCharNum() const {return -1;}

    inline bool isNeedReadCopy() const {return true;}
};


typedef myVarBinaryStoreBase myWvarBinaryStore;
typedef myVarBinaryStoreBase myVarBinaryStore;

class myBinaryStoreBase
{
    const fielddef& m_fd;

public:
    inline myBinaryStoreBase(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len;};

    inline int padChar() const {return 0;};

    inline int storeOffsetBytes() const {return 0;};

    inline int maxCharNum() const {return -1;}

    inline bool isNeedReadCopy() const {return true;}
};

typedef myBinaryStoreBase myWbinaryStore;
typedef myBinaryStoreBase myBinaryStore;

class zstringStore
{
    const fielddef& m_fd;

public:
    inline zstringStore(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len - 1;};

    inline int padChar() const {return 0;};

    inline int storeOffsetBytes() const {return 0;};

    inline int maxCharNum() const {return -1;}

    inline bool isNeedReadCopy() const {return false;}
};

class wzstringStore
{
    const fielddef& m_fd;

public:
    inline wzstringStore(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len -sizeof(short);};

    inline int padChar() const {return 0;};

    inline int storeOffsetBytes() const {return 0;};

    inline int maxCharNum() const {return -1;}

    inline bool isNeedReadCopy() const {return false;}
};

class stringStoreBase
{
    const fielddef& m_fd;

public:
    inline stringStoreBase(const fielddef& fd) : m_fd(fd) {};

    inline size_t maxStoreBytes() const {return m_fd.len;};

    inline int padChar() const {return 0x20;};

    inline int storeOffsetBytes() const {return 0;};

    inline int maxCharNum() const {return -1;}

    inline bool isNeedReadCopy() const {return true;}
};


typedef stringStoreBase wstringStore;
typedef stringStoreBase stringStore;


inline size_t strlen_t(const WCHAR* p) {return strlen16(p);}

inline size_t strlen_t(const char* p) {return strlen(p);}

inline void* memcpy_t(void *dest, const void *src, size_t count) {return memcpy(dest, src, count);}

inline WCHAR* memcpy_t(WCHAR* dest, const WCHAR *src, size_t count) {
    return wmemcpy(dest, src, count);}

inline void* memset_t(void *dest, int c, size_t count) {return memset(dest, c, count);}

inline WCHAR* memset_t(WCHAR* dest, int c, size_t count) {return wmemset16(dest, (WCHAR)c, count);}

template<typename _SF, typename store_type, typename T>
void store(char* ptr, const T* data, const fielddef& fd, stringConverter* cv, bool usePad = true)
{
    _SF sf(fd);

    size_t maxlen = sf.maxStoreBytes() / sizeof(store_type);
    size_t len = strlen_t(data);
    int offset = sf.storeOffsetBytes();
    store_type* strPtr = (store_type*)(ptr + offset);

    if (len == 0)
        strPtr[0] = 0x00;
    else
    {
        // convert
        if ((typeid(T) != typeid(store_type)) ||
            (cv->isNeedConvert() && (typeid(T) == typeid(char))))
            len = cv->convert(strPtr, maxlen, data, len);
        else
        {
            len = std::min<size_t>(maxlen, len);
            memcpy_t(strPtr, data, len);
        }
    }
    int fc = usePad ? sf.padChar() : -1;

    // Trim by max charctor number (not char type length) and invalid mbc lead byte.
    int maxCharnum = sf.maxCharNum();
    if (maxCharnum != -1)
        len = charNumTrim(fd.codePage(), strPtr, len, maxCharnum, fc);
    else if (maxlen == len)
        len = validateTrim(fd.codePage(), strPtr, maxlen, fc);
    else
        strPtr[len] = 0x00;
    // fill after byte.
    if ((fc != -1) && (maxlen != len))
        memset_t((store_type*)(strPtr + len), fc, (maxlen - len));

    if (offset)
    {
        len *= sizeof(store_type);
        memcpy(ptr, &len, offset);
    }
}

template<class T>
T* trim(T* src, T* end, int padChar)
{
    while (src <= --end)
    {
        if (*end != padChar)
            break;
        *end = 0x00;
    }
    return src;
}

#pragma warn -8008
#pragma warn -8066

template<class _SF, typename store_type, typename T>
const T* read(char* ptr, ::bzs::rtl::stringBuffer* strBufs, const fielddef& fd, stringConverter* cv,
    bool isTrimPadChar = false)
{
    _SF sf(fd);

    int offset = sf.storeOffsetBytes();
    T* result = (T*)(ptr + offset);
    //convert
    size_t len;
    if ((typeid(T) != typeid(store_type)) || (cv->isNeedConvert() && (typeid(T) == typeid(char))))
    {
        len = fd.dataLen((const uchar_td*)ptr) / sizeof(store_type);
        size_t olen = len * 2 * sizeof(store_type) + 1; // utf8‚Ö‚Í2”{‚Ì‰Â”\«‚ª‚ ‚é
        result = strBufs->getPtr<T>(olen);
        len = cv->revert(result, olen, (const store_type*)(ptr + offset), len);

    }
    else if (sf.isNeedReadCopy())
    {
        len = fd.dataLen((const uchar_td*)ptr);
        if (len == sf.maxStoreBytes())
        {
            result = (T*)strBufs->getPtrA(len + 2);
            memcpy(result, ptr + offset, len);
        }
        len /= sizeof(store_type);

    }
    else
        return result; // zstring;
    result[len] = 0x00;
    if (sf.padChar() && isTrimPadChar)
        trim(result, result + len, sf.padChar());
    return result;
}
#pragma warn .8008
#pragma warn .8066

template<typename T>
char* blobStore(char* ptr, const T* data, const fielddef& fd, stringConverter* cv)
{
    size_t len = strlen_t(data);
    int offset = fd.len - 8;
    char* p = NULL;
    size_t maxlen = (offset == 1) ? 255 : (offset == 2) ? USHRT_MAX : (offset == 3) ?
        USHRT_MAX * 255 : UINT_MAX;
    if (len != 0)
    {
        if ((typeid(T) != typeid(char)) || (cv->isNeedConvert() && (typeid(T) == typeid(char))))
        {
            maxlen = std::min<size_t>(maxlen, len * 2 * sizeof(T) + 1);
            p = new char[maxlen];
            len = cv->convert(p, maxlen, data, len);

        }
        else
        {
            maxlen = std::min<size_t>(maxlen, len + 1);
            p = new char[maxlen];
            memcpy_t(p, data, len);
            p[len] = 0x00;
        }
    }
    memset(ptr, 0, fd.len);
    memcpy(ptr, &len, offset);
    if (p)
        memcpy(ptr + offset, &(p), sizeof(char*));
    return p;

}
#pragma warn -8004
template<typename T>
const T* readBlob(char* ptr, ::bzs::rtl::stringBuffer* strBufs, const fielddef& fd, stringConverter* cv)
{

    int offset = fd.len - 8;
    T* result = (T*)(ptr + offset);
    char** pc = (char**)(ptr + fd.blobLenBytes());

    if ((typeid(T) != typeid(char)) || (cv->isNeedConvert() && (typeid(T) == typeid(char))))
    {
        size_t len = fd.blobDataLen((const uchar_td*)ptr);
        size_t olen = len * 2 + 1;
        result = strBufs->getPtr<T>(olen);
        len = cv->revert(result, olen, *pc, len);
        result[len] = 0x00;

    }
    else
        result = (T*)*pc;
    return result;
}
#pragma warn .8004

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_STRINGCONVERTER_H
