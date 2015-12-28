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
#include <stdlib.h>
#pragma hdrstop

#include "stringBuffers.h"
#include <assert.h>

#pragma package(smart_init)

namespace bzs
{
namespace rtl
{

static const int MEMORY_UNIT = 8192;
static const size_t MAX_STRING_BUFFER = 196608; // MAX_USHORT * 3

stringBuffer::stringBuffer(size_t size) : m_ptr(NULL), m_len(0), m_pos(0)
{
    alloc(size);
}

stringBuffer::~stringBuffer()
{
    if (m_ptr)
        free(m_ptr);
    m_ptr = NULL;
}

void stringBuffer::clear()
{
    m_pos = 0;
}

size_t stringBuffer::alloc(size_t size)
{
    m_ptr = (char*)malloc(size);
    if (m_ptr)
        m_len = size;
    m_pos = 0;
    return m_len;
}

size_t stringBuffer::re_alloc(size_t size)
{
    if ((size - m_pos) < MAX_STRING_BUFFER && m_len > MAX_STRING_BUFFER)
    {
        m_pos = 0;
        return m_len;
    }

    size =
        ((size / MEMORY_UNIT) + ((size % MEMORY_UNIT) ? 1 : 0)) * MEMORY_UNIT;
    m_ptr = (char*)::realloc(m_ptr, size);
    if (m_ptr)
        m_len = size;
    return m_len;
}

char* stringBuffer::getPtrA(size_t size)
{
    char* p = NULL;

    if (m_pos + size > m_len)
        re_alloc(m_pos + size + 1);
    if (m_pos + size <= m_len)
    {
        p = m_ptr + m_pos;
        m_pos += size;
        m_curSize = size;
    }
    assert(p);
    return p;
}

WCHAR* stringBuffer::getPtrW(size_t size /* charnum */)
{
    WCHAR* p = NULL;
    size_t sizetmp = size * sizeof(WCHAR);
    if (m_pos + sizetmp > m_len)
        re_alloc(m_pos + sizetmp + sizeof(WCHAR));
    if (m_pos + sizetmp <= m_len)
    {
        p = (WCHAR*)(m_ptr + m_pos);
        m_pos += sizetmp;
        m_curSize = size;
    }
    assert(p);
    return p;
}

} // namespace rtl
} // namespace bzs
