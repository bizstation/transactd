Transactd readme
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
Transactd has been constantly updated. This text has also been updated with the 
latest information in this release. Please refer to the RELEASE_NOTE.md the 
changes in this release.


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

  * Windows - transactd-[platform]-2.4.0_[mysql-version].zip
  * Linux - transactd-linux-x86_64-2.4.0_[mysql-version].tar.gz

[platform] is win32 or win64, [mysql-version] is mysql-5.x.x or mariadb-5.5.x.
For example, the URL for Linux-x86_64bit mysql-5.6.14 is as follows:

http://www.bizstation.jp/al/transactd/download/transactd-2.4.0/transactd-linux-x86_64-2.4.0_mysql-5.6.14.tar.gz

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
   cd [TargetFolder]
   ```

2. Extract the tar.gz file and move into it.
   ```
   tar zxf transactd-linux-x86_64-2.4.0_mysql-5.6.14.tar.gz
   cd transactd-linux-x86_64-2.4.0_mysql-5.6.14
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
   cp libtransactd.so /usr/local/mysql/lib/plugin/
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

  * Windows - transactd-client-[platform]_with_sdk-2.4.0.zip
  * Linux -  transactd-client-linux-x86_64_with_sdk-2.4.0.tar.gz

[platform] is win32 or win64.
For example, the URL for Linux-x86_64bit is as follows:

http://www.bizstation.jp/al/transactd/download/transactd-client/transactd-client-linux-x86_64_with_sdk-2.4.0.tar.gz



Installing Transactd clients
-------------------------------------------------------------------------------

### Installing on Windows
1. Open transactd-client-[platform]_with_sdk-2.4.0.zip from explorer.
2. Select the root folder transactd-client-[platform]-with_sdk-2.4.0 and
   copy to a folder of your choice.
3. Run the "install.cmd" in the transactd-client-[platform]_with_sdk-2.4.0 folder.
   This command adds "transactd-client-[platform]_with_sdk-2.4.0\bin" folder to
   the system environment variables "PATH".

C++ clients contains the following three files which is placed in bin folder.

  * tdclc_xx_[version].dll
  * tdclcpp_xx_[Compiler]_[version].dll
  * tdclstmt_xx_[Compiler]_[version].dll

The last two of above files are modules that are compiled with each compilers
in order to export the classes of C++. The benchmark programs and test programs
are also compiled with each compilers. These files are located in the folder
which has the compiler name in the bin folder.
These binaries have been built by the Microsoft Visual studio 2010.
If you want to use in the Embarcadero C ++ Builder.
Please see following information.
http://www.bizstation.jp/en/transactd/documents/BUILD_WIN.html


### Installing on Linux
1. Move to the directory where downloaded tar.gz file is.
   ```
   cd [TargetFolder]
   ```

2. Extract the tar.gz file and move into it.
   ```
   tar zxf transactd-client-linux-x86_64_with_sdk-2.4.0.tar.gz
   cd transactd-client-linux-x86_64_with_sdk-2.4.0
   ```

3. Run the install script.
   ```
   ./install_client.sh
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
"C:\Program Files\MySQL\MySQL Server 5.6\bin\mysqladmin" -u root password 'xxxxx'
```
(Replace xxxxx to your password.)


### Setup root password on Linux
```
/usr/local/mysql/bin/mysqladmin -u root password 'xxxxx'
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

Test script executes the tests in following order:
  * test_tdclcpp_xx_xxm_xxx.exe   A Multibyte modules test
  * test_tdclcpp_xx_xxu_xxx.exe   A Unicode module test(Windows only)
  * bench_tdclcpp_xx.exe          Benchmark of Insert read update 
  * bench_query_xx.exe            Benchmark of aquery
  * querystmtsxx.exe              Test of Query executer

"querystmtsxx.exe" is executed only if the target host is localhost.


### Executing on Windows
1. Move to the client directory.
   ```
   cd transactd-client-[platform]_with_sdk-2.4.0
   ```

2. run test:
   ```
   TestClient.cmd
   ```
   
   Then you will be asked the host name first, please specify
   the host name of the Transactd server. "localhost" will be set
   automatically if you do not specify anything.
   
   Next, select the number of the compiler to test.
   Then tests and benchmarks will be executed continuously.

3. run ActiveX(COM) test:
   ```
   TestClient_ATL.cmd
   ```
   
   Then you will be asked the host name first, please specify
   the host name of the Transactd server. "localhost" will be set
   automatically if you do not specify anything.


### Executing on Linux
1. Move to the client directory.
   ```
   cd transactd-client-linux-x86_64_with_sdk-2.4.0
   ```

2. run test:
   ```
   ./exec_test_all.sh
   ```
   
   Then you will be asked the host name first, please specify
   the host name of the Transactd server. "localhost" will be set
   automatically if you do not specify anything.



Development of applications using Transactd clients
-------------------------------------------------------------------------------
Refer to the SDK documents to develop applications with Transactd clients.

http://www.bizstation.jp/ja/transactd/client/sdk/doc/

There are some simple sample codes in ($installdir)/source/bzs/example.

You can build them with project files in build/example (Windows) or
make_example.sh script (Linux). On Linux, please change to the folder of boost 
that is installed the value of TI_BOOST_ROOT of the Makefile when building.

If you compile with the 64Bit version of the Visual C++ 2010 Express edittion,
change option to "Windows7.1SDK" from "v100" in
[Options] - [platform toolset] - [configuration properties] - [General].

In the C++ Builder, the boost which has been supplied with the compiler
are required. In [Tools] - [Options] - [Environment Options] - [C++ Options] -
[directory and path] - [System Include Path], add the following path:

* For 32Bit: $(CG_BOOST_ROOT)
* For 64Bit: $(CG_64_BOOST_ROOT)



Details of the test program
-------------------------------------------------------------------------------
The executable files whose name starts with test_ is test program.

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
There are two types in the benchmark program. One of them is the benchmark which
read SQL-like query (bench_query_xxx).
Another is basic CRUD operations benchmark (bench_tdclcpp_xxx).

bench_tdclcpp_xxx can be executed in multiple instances, by changing the
processNumber in command line arguments.

The command line options are:
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
bench_tdclcpp_bc200_64u.exe "tdap://localhost/test?dbfile=test.bdf" 0 -1
```

bench_query_xxx measures the speed of getting a similar result with
the following SQL:
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
The command line options are:
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
use this program to generate the model class from the database definition if
you want to use the O/R mapping in C++.

Please refer to the README_ORMSRCGEN.md.



Query executer
-------------------------------------------------------------------------------
querystmts is a program that execute the Transactd query that has been described
in XML file. If you pass an XML file to querystmts, it executes the queries and
outputs the result to the stdout.

To make Transactd query XML file(s), we need the querybuilder program.
But querybuilder has not been released yet. It will be released soon.



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
