#ifndef confParamH
#define confParamH
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
#include <bzs/env/tstring.h>
#include <vector>
#include <bzs/rtl/stl_uty.h>

struct conf_parmas
{
private:
    mutable std::vector<std::string> m_name_spaces;
    mutable std::vector<std::string> m_name_space_maps;
    mutable bool m_namespaceInit;
    void namespaceInit() const
    {
        split(m_name_spaces, name_space, "/");
        split(m_name_space_maps, name_space_map, "/");
        m_namespaceInit = true;
    }

public:
    int files;
    std::string file[2];
    std::string lang;
    std::string fieldRenameList;
    std::string name_space;
    std::string name_space_map;
    std::string saveDir;
    std::string dataClassName;
    std::string setPrefix;
    std::string getPrefix;
    std::string externWord;

    conf_parmas() : m_namespaceInit(false), lang("cpp")
    {
        file[0] = "ormMapClass_template";
        file[1] = "ormDataClass_template";
        files = 2;
    }

    std::string getFilename(int index)
    {
        if (lang == "cpp")
            return file[index] + ".h";
        return file[index];
    }

    const std::vector<std::string>& name_spaces() const
    {
        if (!m_namespaceInit)
            namespaceInit();
        return m_name_spaces;
    }

    const std::vector<std::string>& name_spaces_map() const
    {
        if (!m_namespaceInit)
            namespaceInit();
        return m_name_space_maps;
    }
};

#endif
