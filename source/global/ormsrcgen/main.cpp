/* =================================================================
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
 ================================================================= */
#pragma hdrstop

#include <string>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#define TRDCL_AUTOLINK
#include <bzs/db/protocol/tdap/client/trdclcppautolink.h>
#include "confParam.h"
#include "srcgen.h"

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace std;
using namespace boost;
using namespace boost::program_options;
namespace fs = boost::filesystem;

#ifdef __BCPLUSPLUS__
#define BZS_LINK_BOOST_SYSTEM
#define BZS_LINK_BOOST_FILESYSTEM
#define BZS_LINK_BOOST_PROGRAM_OPTIONS
#include <bzs/env/boost_bcb_link.h>
#endif
//---------------------------------------------------------------------------

struct cmd_parmas
{
    std::string uri;
    std::string tableName;
    std::string confName;
    std::string className;
};

int readConfig(conf_parmas& pmc, const cmd_parmas& pm)
{
    variables_map values;
    // setup configuration options
    options_description cnf("configration");
    cnf.add_options()("lang", value<string>(&pmc.lang),
                      "lang")("files", value<int>(&pmc.files), "files")(
        "file1", value<string>(&pmc.file[0]),
        "file1")("file2", value<string>(&pmc.file[1]), "file2")(
        "name_space", value<string>(&pmc.name_space), "name_space")(
        "name_space_map", value<string>(&pmc.name_space_map),
        "name_space_map")("saveDir", value<string>(&pmc.saveDir), "saveDir")(
        "setPrefix", value<string>(&pmc.setPrefix),
        "setPrefix")("getPrefix", value<string>(&pmc.getPrefix), "getPrefix")(
        "externWord", value<string>(&pmc.externWord),
        "externWord")("fieldRenameList", value<string>(&pmc.fieldRenameList),
                      "fieldRenameList");

    // read configuration file
    ifstream ifs(pm.confName.c_str());
    store(parse_config_file(ifs, cnf), values);
    notify(values);
    if (pmc.externWord != "")
        pmc.externWord += " ";
    // check template file count
    if (pmc.files > 2)
    {
        cout << "error ! files max is 2." << endl;
        return 1;
    }
    // check template file exists
    for (int i = 0; i < pmc.files; ++i)
    {
        fs::path path = pmc.getFilename(i);
        const bool result = fs::exists(path);
        if (!result)
        {
            cout << path << " is not found." << endl;
            return 1;
        }
    }
    return 0;
}

int makeOne(tabledef* def, cmd_parmas pm, conf_parmas& pmc)
{

    pmc.dataClassName = pm.className;
    cppSrcGen cpp(def, pmc);
    cpp.make();
    return 0;
}

#ifdef _UNICODE
std::wstring toTString(const std::string& s)
{
    wchar_t tmp[MAX_PATH];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s.c_str(), -1, tmp, MAX_PATH);
    return std::wstring(tmp);
}
#else
std::string& toTString(std::string& s)
{
    return s;
}
#endif

#pragma argsused
int _tmain(int argc, char* argv[])
{
    // setup command line options
    cmd_parmas pm;
    options_description opt("command line option");
    opt.add_options()(
        "database_uri,d", value<string>(&pm.uri),
        "database uri ex:tdap://hostname/dbname?dbfile=trnasctd_schema")(
        "table_name,t", value<string>(&pm.tableName), "table name")(
        "class_name,c", value<string>(&pm.className), "class name")(
        "conf_name,f", value<string>(&pm.confName), "configuration filename");
    variables_map values;

    try
    {
        store(parse_command_line(argc, argv, opt), values);
        notify(values);
        if (!values.count("database_uri") || !values.count("table_name"))
        {
            cout << opt << endl;
            return 1;
        }
        cout << pm.uri << endl;

        conf_parmas pmc;
        if (values.count("conf_name"))
        {
            if (readConfig(pmc, pm))
                return 1;
            // database open
            database_ptr db = createDatabaseObject();
            openDatabase(db, toTString(pm.uri).c_str());
            if (pm.tableName == "all")
            {
                for (int i = 0; i <= db->dbDef()->tableCount(); ++i)
                {
                    tabledef* def = db->dbDef()->tableDefs(i);
                    if (def)
                    {
                        pm.className = "";
                        makeOne(def, pm, pmc);
                    }
                }
            }
            else
            {
                int n = db->dbDef()->tableNumByName(
                    toTString(pm.tableName).c_str());
                if (n <= 0)
                {
                    cout << "Invalid table name." << endl;
                    return 1;
                }
                if (pm.className == "")
                    pm.className = pm.tableName;
                tabledef* def = db->dbDef()->tableDefs(n);
                makeOne(def, pm, pmc);
            }
        }
        return 0;
    }

    catch (bzs::rtl::exception& e)
    {
        std::tcout << *bzs::rtl::getMsg(e) << std::endl;
    }

    catch (std::exception& e)
    {
        cout << e.what() << endl;
    }
    return 1;
}
//---------------------------------------------------------------------------
