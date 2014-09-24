#ifndef SRCGENH
#define SRCGENH
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

#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <vector>
#include "confParam.h"
#include "fieldName.h"

using namespace bzs::db::protocol::tdap;
typedef std::vector<std::string> strings;

class cppSrcGen
{
    const tabledef* m_tabledef;
    conf_parmas& m_pm;
    CFiledNameResolver m_fnames;
    int typeNum(int type);
    std::string typeName(int typeNum, int len);
    std::string membaName(fielddef* fielddef);
    void replace(std::string& source, const char* convTgtStr,
                 const std::string& replaceStr);
    std::string makeFdiMembaString();
    std::string makeFdiResolverString();
    std::string makeDataMembaString();
    std::string makeDataMembaFuncString();
    std::string makeDataMembaFuncDecString();
    std::string makeDataMembaInitString();
    std::string makeMapReadString();
    std::string makeMapReadStringLine(int index);
    std::string makeMapKeyCompString();
    std::string makeMapKeyValueString();
    std::string makeMapWriteString();
    std::string makeMapWriteStringLine(int index);
    std::string makeMapAutoIncString();
    std::string makeMapKeyEnumString();

    std::string makeNameSpaceBegin(const std::vector<std::string>& list);
    std::string makeNameSpaceEnd(const std::vector<std::string>& list);
    std::string makeIncludeGurdeString(bool data);
    std::string makeFileName(bool data, bool header);
    const char* typeString(int type, int size);
    std::string membaNameSet(std::string s);
    std::string membaNameGet(std::string s);
    bool doMake(const std::string templeHeader, const std::string templeCpp,
                bool data);

public:
    cppSrcGen(const tabledef* tabledef, conf_parmas& pm);

    void make();
};

#endif // SRCGENH
