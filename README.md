Transactd release note
===============================================================================
Transactd is plugin which adds NoSQL access to MySQL/MariaDB.

This document consists of the following topics.
  * Introduction
  * Preparation of Transactd Plugin binary
  * Installing Transactd Plugin
  * Preparation of Transactd clients binaries
  * Installing Transactd clients
  * Setting host permission to access Transactd
  * Execution of test and benchmark program
  * Development of applications using Transactd clients
  * Details of the test program
  * Details of the benchmark program
  * C++ O/R mapping source code generator
  * Query executer
  * About the license of Transactd Plugin
  * About the license of Transactd Client
  * Bug reporting, requests and questions
  * Copyright of related products



Introduction
-------------------------------------------------------------------------------
Transactd consists of the server side plugin and the client side modules.
The server side plugin is called "Transactd Plugin" and the client side module
is called "Transactd clients".



Preparation of Transactd Plugin binary
-------------------------------------------------------------------------------
Check the version of your MySQL/MariaDB and download suitable Transactd Plugin
binary. If you don't know what is version, start MySQL Command Line client and
 run following command:
```
mysql>show variables like 'version';
+---------------+--------+
| Variable_name | Value  |
+---------------+--------+
| version       | 5.6.14 |
+---------------+--------+
```
The names of file to download are formed under following rules:

  * Windows - transactd-[platform]-2.0.0_[mysql-version].zip
  * Linux - transactd-linux-x86_64-2.0.0_[mysql-version].tar.gz

[platform] is win32 or win64, [mysql-version] is mysql-5.x.x or mariadb-5.5.x.
For example, the URL for Linux-x86_64bit mysql-5.6.14 is as follows:

http://www.bizstation.jp/al/transactd/download/transactd-2.0.0/transactd-linux-x86_64-2.0.0_mysql-5.6.14.tar.gz

You also can download source code archive and build from it.
In this case, the source code of MySQL/MariaDB is also required.

See BUILD_WIN-{lang}.md or BUILD_UNIX-{lang}.md in the source code archive
for how to build.



Installing Transactd Plugin
-------------------------------------------------------------------------------
Transactd Plugin can be installed by copying file, without adding any change to
the binary of MySQL/MariaDB.
You don't have to stop MySQL/MariaDB to install the plugin. It requires
administrator authority.

### Installing on Windows
1. Open the zip file downloaded in Explorer. Check if transactd.dll is exists.

2. Copy transactd.dll to `[MySQL|MariaDB installed directory]/lib/plugin`.
   If you don't know where is [MySQL|MariaDB installed directory], start MySQL
   Command Line client and run following command:
```
mysql>show variables like 'plugin%';
+---------------+----------------------------------------------------+
| Variable_name | Value                                              |
+---------------+----------------------------------------------------+
| plugin_dir    | C:\Program Files\MySQL\MySQL Server 5.6\lib\Plugin |
+---------------+----------------------------------------------------+
```

3. Start MySQL Command Line client and run following command:
```
mysql>INSTALL PLUGIN transactd SONAME 'transactd.dll';
```
Installation of Transactd Plugin is finished now.


### Installing on Linux
1. Move to the directory where downloaded tar.gz file is.
```
shell>cd [TargetFolder]
```

2. Extract the tar.gz file and move into it.
```
shell>tar zxf transactd-linux-x86_64-2.0.0_mysql-5.6.14.tar.gz
shell>cd transactd-linux-x86_64-2.0.0_mysql-5.6.14
```

3. Copy libtransactd.so to `[MySQL|MariaDB installed directory]/lib/plugin`.
   If you don't know where is [MySQL|MariaDB installed directory], start MySQL
   Command Line client and run following command:
```
mysql>show variables like 'plugin%';
+---------------+-----------------------------+
| Variable_name | Value                       |
+---------------+-----------------------------+
| plugin_dir    | /usr/local/mysql/lib/plugin |
+---------------+-----------------------------+
mysql>exit
shell>cp libtransactd.so /usr/local/mysql/lib/plugin/
```

4. Start mysql client and run following command:
```
mysql>INSTALL PLUGIN transactd SONAME 'libtransactd.so';
```
Installation of Transactd Plugin is finished now.



Preparation of Transactd client binaries
-------------------------------------------------------------------------------
The Transactd clients are required to access data through Transactd Plugin.
Download the Transactd client binaries for your platform.
The names of file to download are formed under following rules:

  * Windows - transactd-client-[platform]_with_sdk-2.0.0.zip
  * Linux -  transactd-client-linux-x86_64_with_sdk-2.0.0.tar.gz

[platform] is win32 or win64.
For example, the URL for Linux-x86_64bit is as follows:

http://www.bizstation.jp/al/transactd/download/transactd-client/transactd-client-linux-x86_64_with_sdk-2.0.0.tar.gz



Installing Transactd clients
-------------------------------------------------------------------------------

### Installing on Windows
1. Open the transactd-client-[platform]_with_sdk-2.0.0.zip from explorer.
2. Seletct the root folder [transactd-client- [platform] -with_sdk-2.0.0] and Copy to the appropriate folder.
3. Run the "install.cmd" in the transactd-client-[platform]_with_sdk-2.0.0 folder.
This commnad is registered int the system environment variables of "PATH" "transactd-client-[platform]_with_sdk-2.0.0\bin" folder.

C++ client consists of DLL the following three it is placed in the bin folder.

 * tdclc_xx_[version].dll 
 * tdclcpp_xx_[Compiler]_[version].dll 
 * tdclstmt_xx_[Compiler]_[version].dll 

In order to export the class of C++, two of the inner bottom is a module that different for every compiler. In addition, the program of the benchmarks, and other test which uses it is also each compiler. They are located in a folder of the name of the compiler in the bin under. 
If the compiler development have been identified, you can safely delete the unwanted module.

  * Microsoft Visual studio 2010 (Include ActiveX(COM) client)
  * Embarcadero C++Builder XE～XE6series


### Installing on Linux
1. Move to the directory where downloaded tar.gz file is.
```
shell>cd [TargetFolder]
```

2. Extract the tar.gz file and move into it.
```
shell>tar zxf transactd-client-linux-x86_64_with_sdk-2.0.0.tar.gz
shell>cd transactd-client-linux-x86_64_with_sdk-2.0.0
```

3. Run the install script.
```
shell>./install_client.sh
```
Installation of Transactd client is finished now.



Setting host permission to access Transactd
-------------------------------------------------------------------------------
To access data through Transactd, register root@[host] to user table of MySQL.

* `root@127.0.0.1` allows access from a local machine.
* `root@192.168.0.3` allows access from host 192.168.0.3.
* `root@192.168.0.0/255.255.255.0` allows access from host 192.168.0.x.

Start MySQL Command Line client and run following commands to register hosts.
```
mysql>CREATE USER root@'192.168.0.0/255.255.255.0';
```
This operation allow root user to access database. If the root password is not set,
you MUST SET IT.

### Setup root password on Windows
Open command prompt and run following command:
```
shell>"C:\Program Files\MySQL\MySQL Server 5.6\bin\mysqladmin" -u root password 'xxxxx'
```
(Replace xxxxx to your password.)

### Setup root password on Linux
```
shell>/usr/local/mysql/bin/mysqladmin -u root password 'xxxxx'
```
(Replace xxxxx to your password.)

### Using other name than root
It is also possible to register another user name than "root".
In order to use another user name, please add the following to the [mysqld] section
of my.cnf or my.ini.
```
loose-transactd_hostcheck_username = yourUserName
```
Replace yourUserName to an actual user name.



Execution of test and benchmark program
-------------------------------------------------------------------------------
See also the detail of the test and the benchmark program topic.
Test script executes, in order the contents of the following.
 * test_tdclcpp_xx_xxm_xxx.exe   A Multibyte modules test
 * test_tdclcpp_xx_xxu_xxx.exe   A Unicode module test(Windows only)
 * bench_tdclcpp_xx.exe          Benchmark of Insert read update 
 * bench_query_xx.exe            Benchmark of aquery
 * querystmtsxx.exe              Test of Query executer

"querystmtsxx.exe" is executed only if the localshot the target host.

### Executing on Windows
1. Move to the client directory.
```
shell>cd transactd-client-[platform]_with_sdk-2.0.0
```

2. run test:
```
TestClient.cmd
```
Since you will be asked the host name first, please specify the host name 
of the server Transactd. Localhost will be set automatically if you do not 
specify anything. 
Next, please select the number the compiler to test. 
It is run testing, and benchmark continuously.

You can start with the following command test of ActiveX (COM)
```
>TestClient_ATL.cmd
```
Since you will be asked the host name first, please specify the host name 
of the server Transactd. Localhost will be set automatically if you do not 
specify anything. 


### Executing on Linux
1. Move to the client directory.
```
shell>cd transactd-client-linux-x86_64_with_sdk-2.0.0
```

2. run test:
```
./exec_test_all.sh
```
Since you will be asked the host name first, please specify the host name 
of the server Transactd. Localhost will be set automatically if you do not 
specify anything. 
Next, please select the number the compiler to test. 
It is run testing, and benchmark continuously.



Development of applications using Transactd clients
-------------------------------------------------------------------------------
Refer to the following SDK documents for development of the applications
using Transactd clients.

http://www.bizstation.jp/ja/transactd/client/sdk/doc/

The ($installdir)/source/bzs/example folder has easy sample codes. 
You can actually be built in a build/example folder by project files (Windows)
according to a compiler, or a make_example.sh (LINUX) script. 

In addition, when you compile the 64Bit of the Visual C++ 2010 Express edittion, 
of each project [Options] - [platform toolset] - [configuration properties] - [General]
make changes to the "Windows7.1SDK" from "v100".

In the case of C++ Builder, installation requires the boost supplied with the compiler
in advance. In addition, [Tools] - [Options] - [Environment Options] - [C++ Options]
- [directory and path] - [System Include Path] add the following

 * For 32Bit: $(CG_BOOST_ROOT)
 * For 64Bit: $(CG_64_BOOST_ROOT)

Details of the test program
-------------------------------------------------------------------------------
The executable file which starts with test_ is a Test program.

Please be sure to run test with only an instance. It will fail if test runs two or
more instances.
If others access to test database during test runs, fail to drop database and can't
release database.
In such a case, reboot a server or run following command in MySQL command line client
to drop a database.
```
mysql>drop database test;
```

If `-- show_progress=yes` is passed, it shows progress like:
 ```
0%   10   20   30   40   50   60   70   80   90   100%
|----|----|----|----|----|----|----|----|----|----|
************
```

Using `-- host=xxxxx` can specify host.
The test using Japanese database name, table name and field name.
To pass this test, set `character-set-server` as cp932 or utf8 in my.cnf.
```
[mysqld]
character-set-server=utf8
```



Details of the benchmark program
-------------------------------------------------------------------------------
The benchmark program, there are two types. It is the benchmark of reading 
SQL-like query benchmarks(bench_query_xxx) and CURD basic operations (bench_tdclcpp_xxx).

The bench_tdclcpp_xxx benchmark program, which can be measured by multiple instances
running at the same time by changing the processNumber of command line arguments.
The command line option is:
```
bench_tdclcpp_xxx.exe databaseUri processNumber functionNumber

|----------------|--------------------------------------------------------|
| option name    | description                                            |
|----------------|--------------------------------------------------------|
| databaseUri    | URI of a database.                                     |
|----------------|--------------------------------------------------------|
| processNumber  | Specify different number to run two or more instances. |
|----------------|--------------------------------------------------------|
| functionNumber | Specify the operation number.                          |
|                | The operations:                                        |
|                | -1: all of following                                   |
|                |  0: Insert                                             |
|                |  1: Insert in transaction. 20rec x 1000times           |
|                |  2: Insert by bulkmode. 20rec x 1000times              |
|                |  3: read each record                                   |
|                |  4: read each record with snapshot                     |
|                |  5: read range. 20rec x 1000times                      |
|                |  6: read range with snapshot . 20rec x 1000times       |
|                |  7: update                                             |
|                |  8: update in transaction. 20rec x 1000times           |
|----------------|--------------------------------------------------------|
ex)
shell>bench_tdclcpp_bc200_64u.exe "tdap://localhost/test?dbfile=test.bdf" 0 -1
```

bench_query_xxx benchmark program to measure the speed of obtaining a similar 
result with the following SQL.
```
select
   `user`.`id`
   ,`user`.`name`
   ,`extention`.`comment`
   ,`groups`.`name` as `group_name`
from
   `user` INNER JOIN `extention` 
      ON `user`.`id`  = `extention`.`id`
    LEFT JOIN `groups` 
      ON `user`.`group`  = `groups`.`code`
where
    `user`.`id` > 0 and `user`.`id` <= 15000;

```
Command line options are as follows.
```
bench_query_xxx createdb hostname type n
|----------------|--------------------------------------------------------|
| option name    | description                                            |
|----------------|--------------------------------------------------------|
| createdb       | Specify 0 or 1 to determine whether or not to create a |
|                | new test database. The default is 1.                   |
|----------------|--------------------------------------------------------|
| hostname       | Specify the name or IP address of the location database|
|                | .The default is localhost.                             |
|----------------|--------------------------------------------------------|
| type           | Specific numbers, the contents of the query.           |
|                | The default is 15.                                     |
|                | 1: Read 15,000 records from the user table             |
|                | 3: JOIN the comment field of the extention table on the| 
|                |    results of the 1                                    |
|                | 7: JOIN the name of the table groups the results of the|
|                |    3                                                   |
|                | 5: JOIN the name of the groups on the results of the 1 |
|                |+8: Display the execution progress of repeat            |
|----------------|--------------------------------------------------------|
| n              | Specifies the number of executions of the query        |
|                | specified by type. The default is 100.                 |
|----------------|--------------------------------------------------------|
ex)
bench_query_xxx 0 localhost 15 100

```



C++ O/R mapping source code generator
-------------------------------------------------------------------------------
ormsrcgen (32 | 64) is a source code generator for C++ O/R mapping. You can 
use this program to generate from the database defines the model class if you 
want to use the O/R mapping in C++.
Please refer to the README_ORMSRCGEN.md details.



Query executer
-------------------------------------------------------------------------------
Querystmts is a program for executing the Transactd query that has been described
in the XML file. If you pass an XML file to Querystmts, and outputs the result to
the standard output to execute the query according to its contents. 
XML file(s), created by a Querybuilder. Querybuilder has not been released yet.
It will be released soon.



About the license of Transactd Plugin
-------------------------------------------------------------------------------
According to the license of MySQL/MariaDB, Transactd Plugin can be used under
General Public License (GPLv2). The license information is in COPYING.



About the license of Transactd clients
-------------------------------------------------------------------------------
You can select the license of Transactd clients from General Public License
(GPLv2) or commercial license.

* The license information of GPLv2 is in COPYING file.
* You can examine a commercial license which not restricted by GPLv2 if wants.
* Please see BizStation website about the details of licenses.
  [BizStation website](http://www.bizstation.jp/en/transactd/support/)

See also "Transactd clients OSS Exception" for OSS projects not subject to GPLv2.
  - [Transactd clients OSS Exception](http://www.bizstation.jp/en/transactd/support/ossex.html)



Bug reporting, requests and questions
-------------------------------------------------------------------------------
* If you find any bugs or if you have any requests, please send it to Issues
  tracker on github. It requires github account(free).
  - [Transactd Issues](https://github.com/bizstation/transactd/issues)
* If you have any questions, please see our website or send to Issues tracker
  on github above.
  - [Transactd Documents](http://www.bizstation.jp/ja/transactd/documents/)
  - [Transactd License/Support](http://www.bizstation.jp/ja/transactd/support/)



********************************************************************************

The copyrights of products related to Transactd Plugin and clients are below.

********************************************************************************
The following software may be included in Transactd Plugin and Client:
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.


********************************************************************************
The following software may be included in Transactd Plugin:
HandlerSocket plugin for MySQL

Copyright (c) 2010 DeNA Co.,Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of DeNA Co.,Ltd. nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY DeNA Co.,Ltd. "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL DeNA Co.,Ltd. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

********************************************************************************
