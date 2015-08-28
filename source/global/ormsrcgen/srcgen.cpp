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
#include "srcgen.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;
#pragma package(smart_init)

#define LF "\n"
#define MAP_CLASS_SUFFIX "_map"

bool loadFromFile(const string& filename, string& reval)
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
        if (s.size() && s[s.size() - 1] == '\r')
            s.erase(s.size() - 1);
        reval += s + LF;
    }
    return true;
}

bool saveToFile(const string& filename, const string& val)
{
    ofstream ofs(filename.c_str());
    if (ofs.fail())
    {
        cerr << "Output file is not created.\n";
        return false;
    }
    ofs << val << LF;
    return true;
}

string intReadMenbaName(fielddef* fd)
{
    switch (fd->len)
    {
    case 1:
        return "i8()";
    case 2:
        return "i16()";
    case 3:
        return "i()";
    case 4:
        return "i()";
    }
    return "i64()";
}

cppSrcGen::cppSrcGen(const tabledef* tabledef, conf_parmas& pm)
    : m_tabledef(tabledef), m_pm(pm)
{
}

void cppSrcGen::replace(string& source, const char* convTgtStr,
                        const string& replaceStr)
{
    size_t pos = 0;
    while ((pos = source.find(convTgtStr, pos)) != string::npos)
    {
        source.replace(pos, strlen(convTgtStr), replaceStr);
        pos += replaceStr.size();
    }
}

string cppSrcGen::membaNameGet(string s)
{
    char v[2] = { 0 };
    if ((s.size() > 0) && (m_pm.setPrefix == "get"))
    {
        v[0] = s[0] - 32;
        s.replace(0, 1, v);
    }
    return m_pm.getPrefix + s;
}

string cppSrcGen::membaNameSet(string s)
{
    char v[2] = { 0 };

    if ((s.size() > 0) && (m_pm.setPrefix == "set"))
    {
        v[0] = s[0] - 32;
        s.replace(0, 1, v);
    }
    return m_pm.setPrefix + s;
}

string cppSrcGen::membaName(fielddef* fielddef)
{
    string s = m_fnames.getName(fielddef->nameA());
    char v[2] = { 0 };
    if ((s.size() > 0) && ((s[0] >= 'A') && (s[0] <= 'Z')))
    {
        v[0] = s[0] + 32;
        s.replace(0, 1, v);
    }
    return s;
}

string cppSrcGen::makeFdiMembaString()
{
    string retVal;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        retVal = retVal + "\tshort " + membaName(fd) + ";" LF;
    }
    return retVal;
}

int cppSrcGen::typeNum(int type)
{
    int t = type;
    if (t == ft_autoinc)
        t = 1;
    if (t == ft_autoIncUnsigned)
        t = 1;
    else if (t == ft_logical)
        t = 1;
    else if (t == ft_uinteger)
        t = 1;
    else if (t == ft_currency)
        t = 1;
    else if ((t == ft_note) || (t == ft_zstring) || (t == ft_wzstring) ||
             (t == ft_myvarchar) || (t == ft_mywvarchar) ||
             (t == ft_myvarbinary) || (t == ft_mywvarbinary) ||
             (t == ft_mytext) || (t == ft_guid) || (t == ft_lstring))
        t = 0;
    else if (t == ft_float)
        t = 2;
    else if ((t == ft_date) || (t == ft_mydate))
        t = 3;
    else if ((t == ft_time) || (t == ft_mytime))
        t = 4;
    else if ((t == ft_datetime) || (t == ft_mydatetime))
        t = 30;
    else if ((t == ft_lvar) || (t == ft_blob) || (t == ft_myblob))
        t = ft_lvar;
    return t;
}

string cppSrcGen::typeName(int t, int len)
{
    if ((t == 0) && (len == 1))
        return "char";
    else if (t == 0)
        return "string";
    else if ((t == 1) && (len == 4))
        return "int";
    else if ((t == 1) && (len == 2))
        return "short";
    else if ((t == 1) && (len == 1))
        return "char";
    else if ((t == 1) && (len == 8))
        return "__int64";
    else if ((t == 2) && (len == 4))
        return "float";
    else if ((t == 2) && (len == 8))
        return "double";
    else if (t == 3)
        return "btrDate";
    else if (t == 4)
        return "btrTime";
    return "";
}

const char* cppSrcGen::typeString(int type, int size)
{
    switch (type)
    {
    case ft_string:
    case ft_note:
    case ft_mychar:
        if (size == 1)
            return "char";
        return "_tstring";
    case ft_wstring:
    case ft_mywchar:
        if (size == 2)
            return "wchar_t";
        return "_tstring";
    case ft_autoinc:
    case ft_integer:
        if (size == 1)
            return "char";
        else if (size == 2)
            return "short";
        else if ((size == 3) || (size == 4))
            return "int";
        else if (size == 8)
            return "__int64";
        THROW_BZS_ERROR_WITH_MSG(_T("Invalid int size"));
    case ft_currency:
    case ft_float:
        if (size == 4)
            return "float";
        else if (size == 8)
            return "double";
        THROW_BZS_ERROR_WITH_MSG(_T("Invalid float size"));
    case ft_date:
    case ft_mydate:
        return "td::btrDate";
    case ft_time:
    case ft_mytime:
        return "td::btrTime";
    case ft_datetime:
    case ft_timestamp:
        return "td::btrDateTime";
    case ft_logical:
        return "char";
    case ft_zstring:
    case ft_wzstring:
    case ft_myvarchar:
    case ft_mywvarchar:
    case ft_myvarbinary:
    case ft_mywvarbinary:
    case ft_mytext:
    case ft_guid:
    case ft_lstring:
        return "_tstring";
    case ft_uinteger:
    case ft_autoIncUnsigned:
        if (size == 1)
            return "unsigned char";
        else if (size == 2)
            return "unsigned short";
        else if ((size == 3) || (size == 4))
            return "unsigned int";
        else if (size == 8)
            return "unsigned __int64";
        THROW_BZS_ERROR_WITH_MSG(_T("Invalid uint size"));
    case ft_lvar:
    case ft_myblob:
        return "void*";
    case ft_numericsts:
    case ft_numericsa:
    case ft_decimal:
    case ft_mydatetime:
    case ft_mytimestamp:
        THROW_BZS_ERROR_WITH_MSG(_T("non support type"));
    default:
        THROW_BZS_ERROR_WITH_MSG(_T("invalid field type"));
    }
}

void removeEndchar(string& s)
{
    while (s.size() && ((s[s.size() - 1] == '\t') || (s[s.size() - 1] == ',') ||
                        (s[s.size() - 1] == '\n') || (s[s.size() - 1] == '\r')))
        s.erase(s.end() - 1);
}

string cppSrcGen::makeDataMembaInitString()
{
    string retVal;
    string retVal2;
    int n = 0;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        int t = typeNum(fd->type);
        string ts = typeString(fd->type, fd->len);
        string name = membaName(fd);
        if ((t != ft_date) && (t != ft_time) && (t != ft_datetime))
        {
            if ((t != ft_string) || (fd->len == 1))
            {
                retVal = retVal + name + "(0),";
                if ((++n % 4) == 0)
                    retVal += LF "\t\t";
            }
        }
        if ((t == ft_time) || (t == ft_date))
            retVal2 = retVal2 + "\t\t" + name + ".i = 0;" LF;
        else if (t == ft_datetime)
            retVal2 = retVal2 + "\t\t" + name + ".i64 = 0;" LF;
    }
    removeEndchar(retVal);
    retVal2 = "\t{" LF + retVal2 + "\t}" LF;
    return retVal + LF + retVal2;
}

string cppSrcGen::makeDataMembaString()
{
    string retVal;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        string ts = typeString(fd->type, fd->len);
        int t = typeNum(fd->type);
        if (t == ft_lvar)
            ts = "vector<char>";
        string name = membaName(fd);
        retVal = retVal + "\t" + ts + " " + name + ";" LF;
    }
    return retVal;
}

string cppSrcGen::makeDataMembaFuncString()
{
    string retVal1;
    string retVal2;

    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        int t = typeNum(fd->type);
        string ts = typeString(fd->type, fd->len);
        string name = membaName(fd);
        string tmp;
        string tmp2;
        if ((typeNum(fd->type) == 0) && (fd->len != 1))
        {
            tmp = "const _TCHAR* " + m_pm.dataClassName + "::" +
                  membaNameGet(name) + "()const" LF "{" LF "\treturn m_impl->" +
                  name + ".c_str();" LF "}" LF;
            tmp2 = "void " + m_pm.dataClassName + "::" + membaNameSet(name) +
                   "(const _TCHAR* v)" LF "{" LF "\tm_impl->" + name +
                   " = v;" LF "}" LF;
        }
        else if ((t == ft_time) || (t == ft_date))
        {
            tmp = string("const ") + ts + "& " + m_pm.dataClassName + "::" +
                  membaNameGet(name) +
                  "() const" LF "{" LF "\treturn m_impl->" + name + ";" LF
                                                                    "}" LF;
            tmp2 = "void " + m_pm.dataClassName + "::" + membaNameSet(name) +
                   "(int v)" LF "{" LF "\tm_impl->" + name + ".i = v;" LF
                                                             "}" LF;
        }
        else if (t == ft_datetime)
        {
            tmp = string("const ") + ts + "& " + m_pm.dataClassName + "::" +
                  membaNameGet(name) +
                  "() const" LF "{" LF "\treturn m_impl->" + name + ";" LF
                                                                    "}" LF;
            tmp2 = "void " + m_pm.dataClassName + "::" + membaNameSet(name) +
                   "(__int64 v)" LF "{" LF "\tm_impl->" + name + ".i64 = v;" LF
                                                                 "}" LF;
        }
        else if (t == ft_lvar)
        {
            tmp = string() + ts + string(" ") + m_pm.dataClassName + "::" +
                  membaNameGet(name) + "(unsigned int& size) const" LF "{" LF
                                       "\tsize=m_impl->" +
                  name + ".size();" LF "\treturn &(m_impl->" + name + "[0]);" LF
                                                                      "}" LF;
            tmp2 = "void " + m_pm.dataClassName + "::" + membaNameSet(name) +
                   "(" + ts + " v, unsigned int size)" LF "{" LF "\tm_impl->" +
                   name + ".reserve(size);" LF "\tmemcpy(&m_impl->" + name +
                   "[0], v , size);" LF "}";
        }
        else
        {
            tmp = string() + ts + string(" ") + m_pm.dataClassName + "::" +
                  membaNameGet(name) +
                  "() const" LF "{" LF "\treturn m_impl->" + name + ";" LF
                                                                    "}" LF;
            tmp2 = "void " + m_pm.dataClassName + "::" + membaNameSet(name) +
                   "(" + ts + " v)" LF "{" LF "\tm_impl->" + name + " = v;" LF
                                                                    "}" LF;
        }
        retVal1 = retVal1 + tmp + LF;
        retVal2 = retVal2 + tmp2 + LF;
    }
    return retVal1 + retVal2;
}

string cppSrcGen::makeDataMembaFuncDecString()
{

    string retVal1;
    string retVal2;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        int t = typeNum(fd->type);
        string ts = typeString(fd->type, fd->len);
        string name = membaName(fd);
        string tmp;
        string tmp2;
        if ((t == 0) && (fd->len != 1))
        {
            tmp = "const _TCHAR* " + membaNameGet(name) + "()const;" LF;
            tmp2 = "void " + membaNameSet(name) + "(const _TCHAR* v);" LF;
        }
        else if ((t == ft_time) || (t == ft_date))
        {
            tmp = string("const ") + ts + "& " + membaNameGet(name) +
                  "() const;" LF;
            tmp2 = "void " + membaNameSet(name) + "(int v);" LF;
        }
        else if (t == ft_datetime)
        {
            tmp = string("const ") + ts + "& " + membaNameGet(name) +
                  "() const;" LF;
            tmp2 = "void " + membaNameSet(name) + "(__int64 v);" LF;
        }
        else if (t == ft_lvar)
        {

            tmp = ts + " " + membaNameGet(name) +
                  "(unsigned int& size) const;" LF;
            tmp2 = "void " + membaNameSet(name) +
                   "(void* v, unsigned int size);" LF;
        }
        else
        {
            tmp = ts + string(" ") + membaNameGet(name) + "() const;" LF;
            tmp2 = "void " + membaNameSet(name) + "(" + ts + " v);" LF;
        }
        retVal1 = retVal1 + "\t" + tmp;
        retVal2 = retVal2 + "\t" + tmp2;
    }
    return retVal1 + retVal2;
}

string cppSrcGen::makeFdiResolverString()
{
    string retVal;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &m_tabledef->fieldDefs[i];
        string name = membaName(fd);
        retVal = retVal + "\t" + name + " = tb->fieldNumByName(_T(\"" +
                 m_tabledef->fieldDefs[i].nameA() + "\"));" LF;
    }
    removeEndchar(retVal);
    return retVal;
}

string cppSrcGen::makeMapReadStringLine(int i)
{

    fielddef* fd = &m_tabledef->fieldDefs[i];
    string name = membaName(fd);

    int t = typeNum(fd->type);
    int n = fd->len;
    string retVal = "\tm." + membaNameSet(name) + "(fds[m_fdi." + name + "].";
    char tmp[30];
    if ((t == ft_string) && (n == 1))
        retVal += string("i8()");
    else if (t == ft_string)
        retVal += string("c_str()");
    else if ((t == ft_float) && (n == 4))
        retVal += string("f()");
    else if ((t == ft_float) && (n == 8))
        retVal += string("d()");
    else if ((t == ft_integer) || (t == ft_date) || (t == ft_time))
        retVal += intReadMenbaName(fd);
    else if (t == ft_lvar)
    {
        retVal.insert(1, "unsigned int size;" LF "\t");
        retVal += string("getBin(size), size");
    }
    else
    {
        _ltoa_s(t, tmp, 30, 10);
        retVal = string("unknown type:") + tmp;
    }
    retVal += string(");" LF);
    return retVal;
}

string cppSrcGen::makeMapReadString()
{
    string retVal;

    for (int i = 0; i < m_tabledef->fieldCount; i++)
        retVal = retVal + makeMapReadStringLine(i);
    removeEndchar(retVal);
    return retVal;
}

string cppSrcGen::makeMapWriteStringLine(int index)
{
    fielddef* fd = &m_tabledef->fieldDefs[index];
    string name = membaName(fd);
    int t = typeNum(fd->type);

    string s = "\tfds[m_fdi." + name + "] = m." + membaNameGet(name) + "()";
    if ((t == ft_time) || (t == ft_date))
        s += string(".i");
    else if (t == ft_datetime)
        s += string(".i64");
    else if (t == ft_lvar)
    {
        s = "\t{" LF "\t\tunsigned int size;" LF;
        s += "\t\tvoid* p = m." + membaNameGet(name) + "(size);" LF;
        s += "\t\tfds[m_fdi." + name + "].setBin(p, size);" LF "\t}";
    }
    s += string(";" LF);
    return s;
}

string cppSrcGen::makeMapWriteString()
{
    string retVal;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
        retVal = retVal + makeMapWriteStringLine(i);
    removeEndchar(retVal);
    return retVal;
}

string cppSrcGen::makeMapKeyCompString()
{
    string ret;
    if (m_tabledef->primaryKeyNum < m_tabledef->keyCount)
    {
        keydef* kd = &m_tabledef->keyDefs[m_tabledef->primaryKeyNum];
        string idt = "\t";
        size_t pos = 0;
        string tostr = "";
        string tostrend = "";
        for (int i = 0; i < kd->segmentCount; i++)
        {
            fielddef* fd = &(m_tabledef->fieldDefs[kd->segments[i].fieldNum]);

            string s = membaName(fd) + "()";
            int t = typeNum(fd->type);
            if (t == 0 && (fd->len > 1))
            {
                tostr = "std::string(";
                tostrend = ")";
            }
            else
            {
                tostr = "";
                tostrend = "";
            }
            if ((t == ft_time) || (t == ft_date))
                s += string(".i");
            else if (t == ft_datetime)
                s += string(".i64");
            string tmp;
            size_t size = 0;
            if (i != kd->segmentCount - 1)
            {
                tmp = idt + "if (" + tostr + "l." + s + tostrend + " == " +
                      tostr + "r." + s + tostrend + ")" LF + idt + "{" LF;
                size = tmp.size();
                if (i == 0)
                    tmp += idt + "}" LF;
                else
                    tmp += idt + "}" LF + idt + "else" LF "\t";
            }
            tmp += idt + "return (" + tostr + "l." + s + tostrend + " < " +
                   tostr + "r." + s + tostrend + ");" LF;
            ret.insert(pos, tmp);
            idt += "\t";
            pos += size;
        }
    }
    else
        ret = "\t return true;" LF;
    removeEndchar(ret);
    return ret;
}

string cppSrcGen::makeMapKeyValueString()
{
    string s = "\tswitch (keyNum)" LF "\t{" LF;
    char buf[20];

    for (int j = 0; j < m_tabledef->keyCount; ++j)
    {
        _ltoa_s(j, buf, 20, 10);
        s += "\tcase " + string(buf) + ":" LF;
        for (int i = 0; i < m_tabledef->keyDefs[j].segmentCount; i++)
            s += "\t" + makeMapWriteStringLine(
                            m_tabledef->keyDefs[j].segments[i].fieldNum);
        s += "\t\tbreak;" LF;
    }
    s += "\t}" LF;
    removeEndchar(s);
    return s;
}

string cppSrcGen::makeMapAutoIncString()
{
    string retVal;
    for (int i = 0; i < m_tabledef->fieldCount; i++)
    {
        fielddef* fd = &(m_tabledef->fieldDefs[i]);
        if ((fd->type == ft_autoinc) || (fd->type == ft_autoIncUnsigned))
        {
            string name = membaName(fd);
            retVal = retVal + "\tm." + membaNameSet(name) + "(fds[m_fdi." +
                     name + "]." + intReadMenbaName(fd) + ");" LF;
        }
    }
    removeEndchar(retVal);
    return retVal;
}

string cppSrcGen::makeMapKeyEnumString()
{
    string s = "\t";
    for (int j = 0; j < m_tabledef->keyCount; ++j)
    {
        const keydef& kd = m_tabledef->keyDefs[j];
        int n = std::min<int>(4, kd.segmentCount);
        for (int i = 0; i < n; ++i)
        {
            fielddef* fd = &(m_tabledef->fieldDefs[kd.segments[i].fieldNum]);
            s += membaName(fd) + "_";
        }
        s.erase(s.end() - 1);
        s += string("," LF "\t\t");
    }
    removeEndchar(s);
    return s;
}

string cppSrcGen::makeFileName(bool data, bool header)
{
    string s = m_pm.dataClassName;
    if (!data)
        s += string(MAP_CLASS_SUFFIX);
    if (header)
        return m_pm.saveDir + PSEPARATOR_A + s + ".h";
    else
        return m_pm.saveDir + PSEPARATOR_A + s + ".cpp";
}

string cppSrcGen::makeNameSpaceBegin(const vector<string>& list)
{
    vector<string>::const_iterator it = list.begin();
    string s;
    while (it != list.end())
        s += string("namespace ") + *it++ + LF "{" LF;
    return s;
}

string cppSrcGen::makeNameSpaceEnd(const vector<string>& list)
{
    vector<string>::const_iterator it = list.begin();
    string s;
    while (it != list.end())
        s.insert(0, string("}//") + "namespace " + *it++ + LF);
    return s;
}

string cppSrcGen::makeIncludeGurdeString(bool data)
{
    const vector<string>& list =
        data ? m_pm.name_spaces() : m_pm.name_spaces_map();

    vector<string>::const_iterator it = list.begin();
    string s;
    while (it != list.end())
        s += *it++ + string("_");
    s += m_pm.dataClassName;
    boost::algorithm::to_upper(s);
    return s;
}

bool cppSrcGen::doMake(const string templeHeader, const string templeCpp,
                       bool data)
{
    // header
    {
        string hd;
        loadFromFile(templeHeader, hd);
        replace(hd, "%className%", m_pm.dataClassName);
        replace(hd, "%extern%", m_pm.externWord);
        replace(hd, "%includegurde%", makeIncludeGurdeString(data));
        if (data)
        {
            replace(hd, "%dataClassMembaFuncDec%",
                    makeDataMembaFuncDecString());
            replace(hd, "%nameSpaceBegin%",
                    makeNameSpaceBegin(m_pm.name_spaces()));
            replace(hd, "%nameSpaceEnd%", makeNameSpaceEnd(m_pm.name_spaces()));
        }
        else
        {
            replace(hd, "%keyEnum%", makeMapKeyEnumString());
            replace(hd, "%tableName%", string(m_tabledef->tableNameA()));
            replace(hd, "%fdiMemba%", makeFdiMembaString());
            replace(hd, "%nameSpaceMapBegin%",
                    makeNameSpaceBegin(m_pm.name_spaces_map()));
            replace(hd, "%nameSpaceMapEnd%",
                    makeNameSpaceEnd(m_pm.name_spaces_map()));
        }
        saveToFile(makeFileName(data, true), hd);
    }

    // cpp
    {

        string cpp;
        loadFromFile(templeCpp, cpp);
        replace(cpp, "%className%", m_pm.dataClassName);
        if (data)
        {
            replace(cpp, "%datamemba%", makeDataMembaString());
            replace(cpp, "%menbaInit%", makeDataMembaInitString());
            replace(cpp, "%dataClassMembaFunc%", makeDataMembaFuncString());
            replace(cpp, "%nameSpaceBegin%",
                    makeNameSpaceBegin(m_pm.name_spaces()));
            replace(cpp, "%nameSpaceEnd%",
                    makeNameSpaceEnd(m_pm.name_spaces()));
        }
        else
        {
            replace(cpp, "%fdiResolver%", makeFdiResolverString());
            replace(cpp, "%read%", makeMapReadString());
            replace(cpp, "%writeKeyValue%", makeMapKeyValueString());
            replace(cpp, "%write%", makeMapWriteString());
            replace(cpp, "%autoinc%", makeMapAutoIncString());
            replace(cpp, "%keyComp%", makeMapKeyCompString());
            replace(cpp, "%nameSpaceMapBegin%",
                    makeNameSpaceBegin(m_pm.name_spaces_map()));
            replace(cpp, "%nameSpaceMapEnd%",
                    makeNameSpaceEnd(m_pm.name_spaces_map()));
        }
        saveToFile(makeFileName(data, false), cpp);
    }
    return true;
}

void cppSrcGen::make()
{

    if (m_pm.fieldRenameList != "")
        m_fnames.loadFromFile(m_pm.fieldRenameList);
    if (m_pm.dataClassName == "")
        m_pm.dataClassName = m_fnames.getName(m_tabledef->tableNameA());
    if (m_pm.file[0] != "")
        doMake(m_pm.file[0] + ".h", m_pm.file[0] + ".cpp", true);
    if (m_pm.file[1] != "")
        doMake(m_pm.file[1] + ".h", m_pm.file[1] + ".cpp", false);
}
