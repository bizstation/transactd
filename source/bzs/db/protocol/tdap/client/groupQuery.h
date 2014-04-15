#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H
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
#include <bzs/db/protocol/tdap/client/trdboostapi.h>

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


/*template <class T, typename key_type>
int compByKey(const T& l, const T& r, const key_type& s)
{
    if ((s >= (int)l.size()) || (s >= (int)r.size()))
        throw bzs::rtl::exception(0, _T("groupSort:Invalid key index"));
    return comp_any(l.at(s), r.at(s));
}*/


inline int compByKey(const fieldsBase& l, const fieldsBase& r, const int& s)
{
    if ((s >= (int)l.size()) || (s >= (int)r.size()))
        THROW_BZS_ERROR_WITH_MSG(_T("groupSort:Invalid key index"));//throw bzs::rtl::exception(0, _T("groupSort:Invalid key index"));
    return l[s].comp(r[s], 0);
}


template <class Container>
class grouping_comp
{
    const std::vector<typename Container::key_type>& m_keys;
    Container& m_mdls;

public:
    grouping_comp(Container& mdls
            , const std::vector<typename Container::key_type>& keys)
        :m_mdls(mdls),m_keys(keys) {}

    int operator() (int lv, int rv) const
    {
        const typename Container::row_type& lm = m_mdls[lv] ;
        const typename Container::row_type& rm = m_mdls[rv] ;
        for (int i=0;i<m_keys.size();++i)
        {
            const Container::key_type& s = m_keys[i];
            int ret = compByKey(*lm, *rm, s);
            if (ret) return ret;
        }
        return 0;
    }

    bool isEqual(const typename Container::row_type& lm
                    , const typename Container::row_type& rm)
	{
		for (int i=0;i< m_keys.size();++i)
		{
			const Container::key_type& s = m_keys[i];
            int ret = compByKey(*lm, *rm, s);
            if (ret) return false;
		}
		return true;
	}
};

/*
class groupQuery

--------------------
dataset requirements
--------------------

define types
    dataset::row_type   rowset vector type

    // key_type, if row is object , The key type is a string, a numeric,
    // a function address, etc. which you decided.
    dataset::key_type key_type;


public functions
    // Compair with key, which return int value.
    // row_type& is *row_type. If row_type is shared_ptr<T> then row_type& is T.
    int compByKey(const row_type& l, const row_type& r, const key_type& s)

    dataset::iterator begin(dataset& m)
    dataset::iterator end(dataset& m)
    void push_back(dataset& m, T c)
    void clear(dataset& m)      //clear rows

    // resolvKeyValue is use for convert to key value from filed name
    dataset::key_type resolvKeyValue(dataset&, const std::_tstring& name , bool noexception=false)  const

    void setValue(row_type& row, const key_type& resultKey, const result_type& T);

--------------------
dataset options
--------------------
define types

    // header_type: If this type id defined then
    // readBefore(const tdc::table_ptr tb) function called automaticaly.
    // In almost all the cases It is vector<std::_tstring>.
    dataset:: header_type;


functions
    // This function clled at before reading.
    // You can make heder name list etc.
    // If you define header_type, you are necessary to certainly
    // define this function.
    void readBefore(const tdc::table_ptr tb)

*/
class query;

class groupQuery
{
    std::vector<std::_tstring> m_keyFields;
    std::_tstring m_resultField;
    const queryBase* m_having;

     /* remove none grouping fields */
    template <class Container>
	void removeFileds(typename Container& mdls)
	{
        const tdc::fielddefs& fds = *mdls.fieldDefs();
        for (int i=fds.size()-1;i>=0;--i)
        {
            bool enabled = false;
            for (int j=0;j<m_keyFields.size();++j)
            {
                if (m_keyFields[j] == fds[i].name())
                    enabled = true;
            }
            if (!enabled && (m_resultField == fds[i].name()))
                enabled = true;
            if (!enabled)
                mdls.removeField(i);
        }
	}

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

public:
	groupQuery& reset()
	{
		m_keyFields.clear();
		m_resultField = _T("");
		m_having = NULL;
		return *this;
	}

    groupQuery& keyField(const TCHAR* name, const TCHAR* name1=NULL, const TCHAR* name2=NULL, const TCHAR* name3=NULL
                ,const TCHAR* name4=NULL, const TCHAR* name5=NULL, const TCHAR* name6=NULL, const TCHAR* name7=NULL
                ,const TCHAR* name8=NULL, const TCHAR* name9=NULL, const TCHAR* name10=NULL)
    {
        m_keyFields.clear();
        m_keyFields.push_back(name);
        if (name1) m_keyFields.push_back(name1);
        if (name2) m_keyFields.push_back(name2);
        if (name3) m_keyFields.push_back(name3);
        if (name4) m_keyFields.push_back(name4);
        if (name5) m_keyFields.push_back(name5);
        if (name6) m_keyFields.push_back(name6);
        if (name7) m_keyFields.push_back(name7);
        if (name8) m_keyFields.push_back(name8);
        if (name9) m_keyFields.push_back(name9);
        if (name10) m_keyFields.push_back(name10);
        return *this;
    }

    groupQuery& resultField(const _TCHAR* name)
    {
        m_resultField = name;
        return *this;
    }

    groupQuery& having(const queryBase& q) {m_having = &q;return *this;}
	const std::vector<std::_tstring>& getKeyFields()const {return m_keyFields;}
	const std::_tstring& getResultFields()const {return m_resultField;}
	const queryBase& getHaving() const {return *m_having;}

    template <class Container, class FUNC>
    void grouping(Container& mdls,  FUNC func)
    {
        std::vector<typename Container::key_type> keyFields;

        /* convert key value from filed name */
        for (int i=0;i<m_keyFields.size();++i)
            keyFields.push_back(resolvKeyValue(mdls, m_keyFields[i]));
        typename Container::key_type resultKey = resolvKeyValue(mdls, m_resultField, true);
        func.setResultKey(resultKey);
		grouping_comp<typename Container> groupingComp(mdls, keyFields);

        std::vector<int> index;
        typename Container::iterator it = begin(mdls), ite = end(mdls);
        std::vector<FUNC> funcs;
        int i,n = 0;
		while(it != ite)
        {
            bool found = false;
            i = binary_search(n, index, 0, index.size(), groupingComp, found);
            if (!found)
            {
                index.insert(index.begin() + i, n);
                funcs.insert(funcs.begin() + i, FUNC(func));
            }
            funcs[i](*(*it));
            ++n;
            ++it;
		}
        //real sort by index
        typename Container c(mdls);

        clear(mdls);
        for (int i=0;i<index.size();++i)
        {
            typename Container::row_type cur = c[index[i]];
            setValue(*cur, resultKey, funcs[i].result());
            mdls.push_back(cur);
            //push_back(mdls, cur);
        }
        removeFileds(mdls);
    }
};






}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_GROUPQUERY_H

