#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H
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
#include "nsTable.h"
#include "fieldNameAlias.h"

class CField; // atl interface

namespace bzs
{
namespace rtl
{
class stringBuffer;
}
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

class stringConverter;
/** @cond INTERNAL */

/* non copyable */
class DLLLIB fieldShare
{
    friend class field;
    friend class table;
    friend class recordCache;

private:
    struct Imple* m_imple;
    struct
    {
        unsigned char myDateTimeValueByBtrv : 1;
        unsigned char logicalToString : 1;
    };
    fieldShare(const fieldShare&); // no implememt
    fieldShare& operator=(const fieldShare&); // no implememt

protected:
    fieldShare();

    virtual ~fieldShare();
    stringConverter* cv() const;
    bzs::rtl::stringBuffer* strBufs() const;
    void blobPushBack(char* p);
    void blobClear();
};

/** @endcond */

class DLLLIB fielddefs : public fieldShare
{
    friend class table;
    friend class recordsetImple;
    friend class writableRecord;
    friend class memoryRecord;
    friend class recordsetQuery;
    friend struct recordsetQueryImple;
    friend class fieldsBase;
    friend class fields;
    friend class field;

    struct infoImple* m_imple;
    void aliasing(fielddef* p) const;
    fielddefs();
    ~fielddefs();
    fielddefs(const fielddefs& r);
    fielddefs& operator=(const fielddefs& r);
    bool mysqlnullEnable() const;
    bool canUnion(const fielddefs& r) const;
    size_t totalFieldLen() const;
    void resetUpdateIndicator();
    void setAliases(const aliasMap_type* p);
    void push_back(const fielddef* p);
    void calcFieldPos(int startIndex, bool mysqlNull);
    void remove(int index);
    void reserve(size_t size);
public:
    void clear();
    fielddefs* clone() const;
    int indexByName(const std::_tstring& name) const;
    const fielddef& operator[](int index) const;
    const fielddef& operator[](const _TCHAR* name) const;
    const fielddef& operator[](const std::_tstring& name) const;
    bool checkIndex(int index) const;
    size_t size() const;
    void addAllFileds(const tabledef* def);
    void addSelectedFields(const class table* tb);
    void release();
    static fielddefs* create();
};

/** @cond INTERNAL */

typedef int (*compFieldFunc)(const class field& l, const class field& r,
                             char logType);
extern int compWString(const field& l, const field& r, char logType);
extern int compiWString(const field& l, const field& r, char logType);

/** @endcond */

class DLLLIB field
{
    friend class table;
    friend class fieldsBase;
    friend class CField; // atl interface
    friend class memoryRecord; // nullPtr()
    /** @cond INTERNAL */
    friend int compBlob(const field& l, const field& r, char logType);
    /** @endcond */
    fielddef* m_fd;
    unsigned char* m_ptr;
    const class fielddefs* m_fds;
    mutable unsigned char* m_cachedNullPtr;
    mutable unsigned char m_nullbit;

    void nullPtrCache() const;
    int blobLenBytes() const { return m_fd->blobLenBytes(); }
    __int64 readValue64() const;
    void storeValue64(__int64 value);
    double readValueDbl() const;
    void storeValueDbl(double value);
    void storeValueStrA(const char* data);
    const char* readValueStrA() const;
#ifdef _WIN32
    void storeValueStrW(const wchar_t* data);
    const wchar_t* readValueStrW() const;
#endif
    void storeValueNumeric(double data);
    double readValueNumeric() const;
    void storeValueDecimal(double data);
    double readValueDecimal() const;
    void* nullPtr() const;


    //  ---- bigin regacy interfaces ----  //
    const char* getFVAstr() const;
#ifdef _WIN32
    const wchar_t* getFVWstr() const;
#endif
    __int64 getFV64() const;
    double getFVdbl() const;
    void* getFVbin(uint_td& size) const;
    inline unsigned char getFVbyt() const { return (unsigned char)getFV64();}
    inline short getFVsht() const { return (short)getFV64();}
    inline int getFVint() const  { return (int)getFV64();}
    inline int getFVlng() const  { return (int)getFV64();}
    inline float getFVflt() const { return (float)getFVdbl();}

#ifdef _WIN32
    void setFVW(const wchar_t* data);
#endif
    void setFV(double data);
    void setFV(__int64 data);
    void setFVA(const char* data);
    void setFV(const void* data, uint_td size);
#ifdef _UNICODE
    inline const wchar_t* getFVstr() const { return getFVWstr(); };
    inline void setFV(const wchar_t* data) { setFVW(data); };
#else
    inline const char* getFVstr() const { return getFVAstr(); };
    inline void setFV(const char* data) { setFVA(data); };
#endif
    inline void setFV(float data){ setFV((double)data); }
    inline void setFV(unsigned char data) { setFV((__int64)data); }
    inline void setFV(short data)  { setFV((__int64)data); }
    inline void setFV(int data)  { setFV((__int64)data); }
    //  ---- end regacy interfaces ----  //

/** @cond INTERNAL */
#if defined(SWIG) ||                                                           \
    defined(SWIG_BUILDING) // SWIG Wrapper need public constructor
public:
#endif
    inline field() : m_fd(NULL), m_ptr(NULL), m_fds(NULL){};
/** @endcond */

public:
/** @cond INTERNAL */
    // nullPtr and nullbit is all field same.
    inline field(unsigned char* ptr, const fielddef& fd, const fielddefs* fds) 
        : m_fd((fielddef*)&fd), m_ptr(ptr), m_fds(fds),m_cachedNullPtr(NULL), m_nullbit(0)
    {
    }
/** @endcond */
    inline field(const field& r) : m_fd(r.m_fd), m_ptr(r.m_ptr), m_fds(r.m_fds),
            m_cachedNullPtr(NULL), m_nullbit(0)
    {
    }

    inline field& operator=(const field& r)
    {
        m_fd = r.m_fd;
        m_ptr = r.m_ptr;
        m_fds = r.m_fds;
        m_cachedNullPtr = r.m_cachedNullPtr;
        m_nullbit = r.m_nullbit;
        return *this;
    }

    void* ptr() const;

    unsigned char type() const { return m_fd->type; }

    unsigned short len() const { return m_fd->len; }

    inline const _TCHAR* c_str() const { return getFVstr(); }

    inline const char* a_str() const { return getFVAstr(); }

    inline int i() const { return getFVint(); }

    inline int i8() const { return getFVbyt(); }

    inline short i16() const { return getFVsht(); }

    inline __int64 i64() const { return getFV64(); }

    inline float f() const { return getFVflt(); }

    inline double d() const { return getFVdbl(); }

    bool isNull() const;

    void setNull(bool v);

    inline field& operator=(const _TCHAR* p)
    {
        setFV(p);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline field& operator=(const std::_tstring& p)
    {
        setFV(p.c_str());
        m_fd->enableFlags.bitE = true;
        return *this;
    }

#ifdef _UNICODE
    inline field& operator=(const char* p)
    {
        setFVA(p);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline field& operator=(const std::string& p)
    {
        setFVA(p.c_str());
        m_fd->enableFlags.bitE = true;
        return *this;
    }

#endif

    inline field& operator=(int v)
    {
        setFV(v);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline field& operator=(__int64 v)
    {
        setFV(v);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline field& operator=(float v)
    {
        setFV(v);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline field& operator=(double v)
    {
        setFV(v);
        m_fd->enableFlags.bitE = true;
        return *this;
    }

    inline bool operator!=(const _TCHAR* p) const { return !operator==(p); }

    inline bool operator==(const _TCHAR* p) const
    {
        return (isNull() == false) && (_tcscmp(p, c_str()) == 0);
    }

    inline bool operator!=(int v) const { return !operator==(v); }
    inline bool operator==(int v) const { return (isNull() == false) && (v == i()); }

    inline bool operator!=(short v) const { return !operator==(v); }
    inline bool operator==(short v) const { return (isNull() == false) && (v == i16()); }

    inline bool operator!=(__int64 v) const { return !operator==(v); }
    inline bool operator==(__int64 v) const { return (isNull() == false) && (v == i64()); }

    inline bool operator!=(float v) const { return !operator==(v); }
    inline bool operator==(float v) const { return (isNull() == false) && (v == f()); }

    inline bool operator!=(double v) const { return !operator==(v); }
    inline bool operator==(double v) const { return (isNull() == false) && (v == d()); }

    inline void setBin(const void* data, uint_td size)
    {
        setFV(data, size);
        m_fd->enableFlags.bitE = true;
    }
    inline void* getBin(uint_td& size) const { return getFVbin(size); };

    int nullComp(const field& r, char log) const;
    int nullComp(char log) const;

    int comp(const field& r, char logType = CMPLOGICAL_VAR_COMP_ALL) const;

    /** @cond INTERNAL */
    bool isCompPartAndMakeValue();
    void offsetBlobPtr(size_t offset);
    compFieldFunc getCompFunc(char logType) const;
    /** @endcond */
};

/** @cond INTERNAL */
/* For template tget type num by type.*/

inline int getFieldType(int)
{
    return ft_integer;
}

inline int getFieldType(__int64)
{
    return ft_integer;
}

inline int getFieldType(short)
{
    return ft_integer;
}

inline int getFieldType(char)
{
    return ft_integer;
}

inline int getFieldType(double)
{
    return ft_float;
}

inline int getFieldType(float)
{
    return ft_float;
}

inline __int64 fieldValue(const field& fd, __int64)
{
    return fd.i64();
}

inline int fieldValue(const field& fd, int)
{
    return fd.i();
}

inline short fieldValue(const field& fd, short)
{
    return (short)fd.i();
}

inline char fieldValue(const field& fd, char)
{
    return (char)fd.i();
}

inline double fieldValue(const field& fd, double)
{
    return fd.d();
}

inline float fieldValue(const field& fd, float)
{
    return fd.f();
}

inline const _TCHAR* fieldValue(const field& fd, const _TCHAR*)
{
    return fd.c_str();
}

DLLLIB const fielddef& dummyFd();

/** @endcond */

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H
