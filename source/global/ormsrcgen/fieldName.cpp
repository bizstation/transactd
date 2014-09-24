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
#pragma hdrstop
#include "fieldName.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <bzs/rtl/exception.h>
using namespace std;

#pragma package(smart_init)

// ---------------------------------------------------------------------------
// class CFiledName
// ---------------------------------------------------------------------------
CFiledName::CFiledName(string oName, string n) : name(n), orignName(oName)
{
}

bool CFiledName::operator<(const CFiledName& rt) const
{
    return orignName < rt.orignName;
}

// ---------------------------------------------------------------------------
// class CFiledNameResolver
// ---------------------------------------------------------------------------
bool CFiledNameResolver::loadFromFile(const string& filename)
{

    ifstream ifs(filename.c_str());
    if (ifs.fail())
    {
        cerr << "File not exist.\n";
        return false;
    }
    string s;
    while (getline(ifs, s))
    {
        size_t pos = s.find("=");
        if (pos != string::npos)
            m_list.push_back(CFiledName(s.substr(0, pos), s.substr(pos + 1)));
        else
            THROW_BZS_ERROR_WITH_MSG(_T("Filed resolver invalid data."));
    }
    sort(m_list.begin(), m_list.end());
    return true;
}

string CFiledNameResolver::getName(const char* orignName)
{

    if (binary_search(m_list.begin(), m_list.end(), CFiledName(orignName, "")))
    {
        std::vector<CFiledName>::iterator p;
        p = lower_bound(m_list.begin(), m_list.end(),
                        CFiledName(orignName, ""));
        return (*p).name;
    }
    return orignName;
}