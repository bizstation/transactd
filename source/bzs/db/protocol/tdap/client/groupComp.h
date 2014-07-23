#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPCOMP_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPCOMP_H
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

/** @cond INTERNAL */

template <class Container, class FUNC>
int binary_search(int key, const Container& a
			, int left, int right, FUNC func, bool& find)
{
	find = false;
	if (right == 0) return 0; // no size

	int mid, tmp, end = right;
	while(left <= right)
	{
		mid = (left + right) / 2;
		if (mid >= end)
			return end;
		if ((tmp = func(a[mid], key)) == 0)
		{
			find = true;
			return mid;
		}
		else if (tmp < 0)
			left = mid + 1;  //keyValue is more large
		else
			right = mid - 1; //keyValue is more small
	}
	return left;
}

inline int compByKey(const fieldsBase& l, const fieldsBase& r, const int& s)
{
	assert((s < (int)l.size()) && (s < (int)r.size()));
	return l.getFieldNoCheck(s).comp(r.getFieldNoCheck(s), 0);
}


template <class Container>
class grouping_comp
{
	typedef std::vector<typename Container::key_type> key_vec;
	const key_vec& m_keys;
	Container& m_mdls;

public:
	grouping_comp(Container& mdls
			, const std::vector<typename Container::key_type>& keys)
		:m_mdls(mdls),m_keys(keys) {}

	int operator() (int lv, int rv) const
	{
		const typename Container::row_pure_type& lm = m_mdls[lv] ;
		const typename Container::row_pure_type& rm = m_mdls[rv] ;
		for (int i=0;i<(int)m_keys.size();++i)
		{
			typename Container::key_type s = m_keys[i];
			int ret = (s == -1) ? 0:compByKey(lm, rm, s);
			if (ret) return ret;
		}
		return 0;
	}

	bool isEqual(const typename Container::row_type& lm
					, const typename Container::row_type& rm)
	{
		for (int i=0;i< m_keys.size();++i)
		{
			typename Container::key_type s = m_keys[i];
			int ret = (s == -1) ? 0:compByKey(*lm, *rm, s);
			if (ret) return false;
		}
		return true;
	}
};

/** @endcond */


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPCOMP_H

