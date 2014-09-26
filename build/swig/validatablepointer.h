#ifndef TRANSACTD_SWIG_VALIDATABLEPOINTER_H
#define TRANSACTD_SWIG_VALIDATABLEPOINTER_H
/* =================================================================
 Copyright (C) 2000-2014 BizStation Corp All rights reserved.

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
#include <boost/thread/mutex.hpp>

/* database::close() deletes table pointers, but does NOT delete
   variables on target-languages (PHP, Ruby, etc).
   
   If the same address is allocated for a new table object by
   database::openTable, then two variables on target-language will
   hold the same table pointer (but the older one is invalid).
   
   When GC destroys the old variable, the new table object will be
   deleted because old variable holds the pointer to it.
   
   To avoid this problem, global validatablePointerList holds list
   of pointer and the availability, and delete only the old one.
*/

struct validatablePointer
{
    void* ptr;
    bool  invalid;
};

class validatablePointerList
{
    boost::mutex m_mutex;
    std::vector<validatablePointer> ptrs;
    
    int find(void* p)
    {
        for (size_t i = 0; i < ptrs.size(); ++i)
        {
            if (ptrs[i].ptr == p)
                return (int)i;
        }
        return -1;
    }
    
public:
    void add(void* p)
    {
        boost::mutex::scoped_lock lck(m_mutex);
        /* If there are some pointers which has same address with p,
            set them as invalid. */
        for (size_t i = 0; i < ptrs.size(); ++i)
        {
            if (ptrs[i].ptr == p)
                ptrs[i].invalid = true;
        }
        validatablePointer v = {p, false};
        ptrs.push_back(v);
    }
    
    /*
    @return true : a invalid pointer or no pointers has been removed.
            false: a valid pointer has been removed.
    */
    bool remove(void* p) 
    {
        boost::mutex::scoped_lock lck(m_mutex);
        int index = find(p);
        bool ret = true;
        if (index != -1)
        {
            ret = ptrs[index].invalid; 
            ptrs.erase(ptrs.begin() + index);
        }
        return ret;
    }
};
#endif //not TRANSACTD_SWIG_VALIDATABLEPOINTER_H
