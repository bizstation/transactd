#ifndef BZS_RTL_STRINGBUFFERS_H
#define BZS_RTL_STRINGBUFFERS_H
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
#include <bzs/env/compiler.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <linuxTypes.h>
#include <typeinfo>
#endif

namespace bzs
{
namespace rtl
{

class stringBuffer
{
    char* m_ptr;
    size_t m_len;
    size_t m_pos;
    size_t m_curSize;

public:
    stringBuffer(size_t size);
    ~stringBuffer();
    void clear();
    size_t alloc(size_t size);
    size_t realloc(size_t size);
    char* getPtrA(size_t size);
    WCHAR* getPtrW(size_t size /* charnum */);
    size_t size() const { return m_curSize; }
    template <class T> T* getPtr(size_t size){};
};

template <> inline WCHAR* stringBuffer::getPtr<WCHAR>(size_t size)
{
    return getPtrW(size);
};
template <> inline char* stringBuffer::getPtr<char>(size_t size)
{
    return getPtrA(size);
};

} // namespace rtl
} // namespace bzs

#endif // BZS_RTL_STRINGBUFFERS_H
