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

typedef int (*compFieldFunc)(const class field& l, const class field& r, char logType);

class AGRPACK field
{
	const fielddef& m_fd;
	unsigned char* m_ptr;
    class fieldInfo* m_fds;

    compFieldFunc getCompFunc(char logType);
protected:
    double getFVnumeric() const;
    double getFVDecimal() const;
    void setFVDecimal(double data);
    void setFVNumeric(double data);

public:
	//  ---- bigin regacy interfaces ----  //
    unsigned char getFVbyt() const;
    short getFVsht() const;
    int getFVint() const;
    int getFVlng() const;
    __int64 getFV64() const;
    float getFVflt() const;
    double getFVdbl() const;
    void* getFVbin(uint_td& size ) const;
    const char* getFVAstr() const;
#ifdef _WIN32
    const wchar_t* getFVWstr() const;
    void setFVW(const wchar_t* data);
#endif
    void setFV(float data);
    void setFV(double data);
    void setFV(bool data);
    void setFV(unsigned char data);
    void setFV(short data);
    void setFV(int data);
    void setFV(__int64 data);
    void setFVA(const char* data);
    void setFV(const void* data, uint_td size);
#ifdef _UNICODE
    inline const wchar_t* getFVstr() const {return getFVWstr();};
    inline void setFV(const wchar_t* data) {setFVW(data);};
#else
    inline const char* getFVstr() const {return getFVAstr();};
    inline void setFV(const char* data) {setFVA(data);};
#endif
    void* ptr() const;
	//  ---- end regacy interfaces ----  //

public:
    unsigned char type() const {return m_fd.type;}
    unsigned short len() const {return m_fd.len;}
    int varLenBytes() const {return m_fd.varLenBytes();}
    int blobLenBytes() const {return m_fd.blobLenBytes();}

    inline field(unsigned char* ptr, const fielddef& fd, fieldInfo* fds)
            : m_ptr(ptr), m_fd(fd), m_fds(fds) {};

    inline const _TCHAR* c_str() const {return getFVstr();}

    inline const char* a_str() const {return getFVAstr();}

    inline int i() const {return getFVint();}

    inline int i8() const {return getFVbyt();}

    inline short i16() const {return getFVsht();}

    inline __int64 i64() const {return getFV64();}

    inline float f() const {return getFVflt();}

    inline double d() const {return getFVdbl();}

    inline field& operator = (const _TCHAR* p)
    {
        setFV(p);
        return *this;
    }

    inline field& operator = (const std::_tstring& p)
    {
        setFV(p.c_str());
        return *this;
    }

#ifdef _UNICODE
    inline field& operator = (const char* p)
    {
        setFVA(p);
        return *this;
    }

    inline field& operator = (const std::string& p)
    {
        setFVA( p.c_str());
        return *this;
    }

#endif

    inline field& operator = (int v)
    {
        setFV( v);
        return *this;
    }

    inline field& operator = (__int64 v)
    {
        setFV(v);
        return *this;
    }

    inline field& operator = (float v)
    {
        setFV(v);
        return *this;
    }

    inline field& operator = (double v)
    {
        setFV(v);
        return *this;
    }

    inline bool operator != (const _TCHAR* p) {return (_tcscmp(p, c_str()) != 0);};
    inline bool operator == (const _TCHAR* p) {return (_tcscmp(p, c_str())==0);};

    inline bool operator != (int v) {return (v != i());};
    inline bool operator == (int v) {return (v == i());};

    inline bool operator != (short v) {return (v != i16());};
    inline bool operator == (short v) {return (v == i16());};

    inline bool operator != (__int64 v) {return (v != i64());};
    inline bool operator == (__int64 v) {return (v == i64());};

    inline bool operator != (float v) {return (v != f());};
    inline bool operator == (float v) {return (v == f());};

    inline bool operator != (double v) {return (v != d());};
    inline bool operator == (double v) {return (v == d());};

    inline void setBin(const void* data, uint_td size){setFV(data, size);}
    inline void* getBin(uint_td& size){return getFVbin(size);};

    int comp(const field& r, char logType);
};





}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H

