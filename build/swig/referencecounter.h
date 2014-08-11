#ifndef TRANSACTD_SWIG_REFERENCECOUNTER_H
#define TRANSACTD_SWIG_REFERENCECOUNTER_H
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

struct referencePtrPair
{
	void* referer;
	void* referred_ptr;
};

class referenceCounter
{
	boost::mutex m_mutex;

public:
	std::vector<referencePtrPair> ptrs;

	inline void add(void* referer, void* referred_ptr)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		referencePtrPair v = {referer, referred_ptr};
		ptrs.push_back(v);
	}

	inline void remove(void* referer)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs[i].referer == referer)
				ptrs.erase(ptrs.begin()+i);
		}
	}

	inline int getReferenceCount(void* referred_ptr)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		int cnt = 0;
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs[i].referred_ptr == referred_ptr)
				cnt++;
		}
		return cnt;
	}

#ifdef SWIGRUBY
	inline void mark()
	{
		boost::mutex::scoped_lock lck(m_mutex);
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			VALUE object = SWIG_RubyInstanceFor(ptrs[i].referred_ptr);
			if (object != Qnil)
				rb_gc_mark(object);
		}
	}
#endif
};
#endif //not TRANSACTD_SWIG_REFERENCECOUNTER_H
