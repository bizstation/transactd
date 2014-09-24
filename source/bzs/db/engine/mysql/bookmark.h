#ifndef BZS_DB_ENGINE_MYSQL_BOOKMARK_H
#define BZS_DB_ENGINE_MYSQL_BOOKMARK_H
/* =================================================================
 Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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

#include <vector>
#include <bzs/rtl/exception.h>
#include "mysqlInternal.h"

#ifdef BOOKMARK_USE_MULTIMAP
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
using namespace boost::multi_index;
#endif // BOOKMARK_USE_MULTIMAP

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

#ifdef BOOKMARK_USE_MULTIMAP
/** The version which can reverse-refer to bookmark from raw ref using multi map
 *  In order that this system may use a lot of memories
 , there is a problem in which the fall of performance occurs.
 */

typedef std::vector<unsigned char> charArray;

struct bookmark
{
    unsigned int id;
    charArray ref;

    bookmark(unsigned int id, const charArray& ref) : id(id), ref(ref) {}
};

struct id
{
};

struct ref
{
};

typedef multi_index_container<
    bookmark,
    indexed_by<
        ordered_unique<tag<id>, member<bookmark, unsigned int, &bookmark::id> >,
        ordered_unique<tag<ref>, member<bookmark, charArray,
                                        &bookmark::ref> > > > bookmarkContainer;

typedef bookmarkContainer::index<id>::type id_map;
typedef bookmarkContainer::index<ref>::type ref_map;

class bookmarks
{
    int m_refLen;
    int m_lastbm;
    bookmarkContainer m_bms;
    charArray m_tmpRef;

public:
    bookmarks(int refLen) : m_refLen(refLen), m_tmpRef(refLen), m_lastbm(0) {}

    /** get integer bookmark pointer by raw ref pointer
     */
    const unsigned char* getBookmarkPtr(unsigned char* rawRefPtr)
    {
        m_tmpRef.clear();
        m_tmpRef.insert(m_tmpRef.end(), rawRefPtr, rawRefPtr + m_refLen);
        ref_map& m = m_bms.get<ref>();
        ref_map::iterator it = m.find(m_tmpRef);
        if (it == m.end())
        {
            m_bms.insert(m_bms.end(), bookmark(++m_lastbm, m_tmpRef));
            it = m.find(m_tmpRef);
        }
        return (const unsigned char*)&it->id;
    }

    unsigned int getBookmark(unsigned char* rawRefPtr)
    {
        return *((unsigned int*)getBookmarkPtr(rawRefPtr));
    }

    /** get raw ref pointer by integer bookmark
     */
    const unsigned char* getRefByBm(unsigned int bm)
    {
        id_map& m = m_bms.get<id>();
        id_map::iterator it = m.find(bm);
        if (it != m.end())
            return &(it->ref[0]);
        THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_BOOKMARK,
                                     "invalid bookmark.");
    }
};
#else // NOT BOOKMARK_USE_MULTIMAP

/** The version which returns raw ref to bookmark in simple arrangement
 *  The maximum memory size is specified in MAX_BOOKMARK_MEM_SIZE.
 *  When the memory exceeding it is required, it is considered as an error.
 */

#define MAX_BOOKMARK_MEM_SIZE 500 * 1024 * 1024 // 500MB
#define UNIT_OF_ALLOCCOUNT 10000

class bookmarks
{
    size_t m_refLen;
    size_t m_allocatedCount;
    size_t m_usedCount;
    unsigned char* m_ptr;
    unsigned int m_bookmark;

    void allocBuffer(size_t count)
    {
        if (MAX_BOOKMARK_MEM_SIZE < count * m_refLen)
            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_BOOKMARK,
                                         "invalid bookmark.");
        void* tmp;
        if (m_ptr == NULL)
            tmp = (unsigned char*)td_malloc(count * m_refLen, 0);
        else
            tmp = td_realloc(m_ptr, count * m_refLen, 0);
        if (tmp)
            m_ptr = (unsigned char*)tmp;
        else
            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_BOOKMARK,
                                         "invalid bookmark.");
        m_allocatedCount = count;
    }

public:
    bookmarks(size_t refLen) : m_refLen(refLen), m_usedCount(0), m_ptr(NULL)
    {
        allocBuffer(UNIT_OF_ALLOCCOUNT);
    }

    ~bookmarks() { td_free(m_ptr); }

    /** get integer bookmark pointer by raw ref pointer
     */
    const unsigned char* getBookmarkPtr(unsigned char* rawRefPtr)
    {
        if (m_usedCount == m_allocatedCount)
            allocBuffer(m_usedCount + UNIT_OF_ALLOCCOUNT);

        memcpy(m_ptr + m_usedCount * m_refLen, rawRefPtr, m_refLen);
        m_bookmark = (unsigned int)(++m_usedCount);
        return (const unsigned char*)&m_bookmark;
    }

    unsigned int getBookmark(unsigned char* rawRefPtr)
    {
        return *((unsigned int*)getBookmarkPtr(rawRefPtr));
    }

    /** get raw ref pointer by integer bookmark
     */
    const unsigned char* getRefByBm(unsigned int bm)
    {
        if ((0 == bm) || bm > m_usedCount)
            THROW_BZS_ERROR_WITH_CODEMSG(STATUS_INVALID_BOOKMARK,
                                         "invalid bookmark.");
        return m_ptr + (bm - 1) * m_refLen;
    }
};

#endif // NOT BOOKMARK_USE_MULTIMAP

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_BOOKMARK_H
