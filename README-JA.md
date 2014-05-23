Transactd リリースノート
===============================================================================
Transactd PluginはMySQL/MariaDBにNoSQLアクセスを追加するプラグインです。

このドキュメントは以下のトピックスから構成されています。
  * はじめに
  * Transactd Pluginバイナリの準備
  * Transactd Pluginのインストール
  * Transactdクライアントバイナリの準備
  * Transactdクライアントのインストール
  * Transactdへのアクセス可能ホストの設定
  * テストとベンチマークの起動
  * アプリケーションの開発
  * Testプログラム
  * ベンチマークプログラム
  * Transactd Pluginのライセンス
  * Transactdクライアントのライセンス
  * バグ報告・要望・質問など
  * 関連著作権の表示



はじめに
-------------------------------------------------------------------------------
Transactdはプラグインとして動作するサーバー側モジュールとクライアント側で動作す
るクライアント側モジュールから構成されます。
以降はこれらを区別して、サーバー側をTransactd Plugin、クライアント側をTransactd
クライアントと呼びます。



Transactd Pluginバイナリの準備
-------------------------------------------------------------------------------
MySQL/MariaDBのバージョンを確認し、それに合ったビルド済Transactdをダウンロードし
ます。バージョンの確認はMySQLクライントを起動し以下のコマンドを入力します。
```
show variables like 'version';
+---------------+--------+
| Variable_name | Value  |
+---------------+--------+
| version       | 5.6.14 |
+---------------+--------+
```
ダウンロードするファイルは

  * Windowsの場合 transactd-[platform]-1.1.0_[mysql-version].zip
  * Linuxの場合 transactd-linux-x86_64-1.1.0_[mysql-version].tar.gz

といった形式です。
[platform]はwin32またはwin64、[mysql-version]はmysql-5.x.xまたはmariadb-5.5.xです。
たとえば、Linux-x86_64bit mysql-5.6.14用の完全なURLは以下の通りです。

http://www.bizstation.jp/al/transactd/download/transactd-1.1.0/transactd-linux-x86_64-1.1.0_mysql-5.6.14.tar.gz

また、ソースをダウンロードしてビルドすることもできます。その場合は、
MySQL/MariaDBのソースコードも必要です。ビルド方法はソースコード内の
BUILD_WIN-{lang}.mdまたはBUILD_UNIX-{lang}.mdをご覧ください。



Transactd Pluginのインストール
-------------------------------------------------------------------------------
MySQL/MariaDBのバイナリに変更を加えることなく、所定の位置にプラグインのファイル
をコピーするだけでインストールできます。MySQL/MariaDBサーバーは動作している状態
で作業を進めます。また、作業は管理者権限で進めます。

### Windowsでのインストール 
1. ダウンロードしたzipファイルをエクスプローラで開きます。中にtransactd.dllがある
  ことを確認します。

2. [MySQL|MariaDBインストールフォルダ]/lib/pluginに、transactd.dllをコピーします。
   MySQL|MariaDBインストールフォルダが不明な場合は、MySQL Command Line clientを
   起動し以下のコマンドでplugin_dirを確認してください。
```
mysql>show variables like 'Plugin%';
+---------------+----------------------------------------------------+
| Variable_name | Value                                              |
+---------------+----------------------------------------------------+
| plugin_dir    | C:\Program Files\MySQL\MySQL Server 5.6\lib\plugin |
+---------------+----------------------------------------------------+
```

3. MySQL Command Line clientを起動し、以下のコマンドを実行します。
```
mysql>INSTALL PLUGIN transactd SONAME 'transactd.dll';
```
これでPluginのインストールは終了です。


### Linuxでのインストール 
1. ダウンロードしたtar.gzのあるフォルダに移動します。
```
shell>cd [TargetFolder]
```

2. ダウンロードしたtar.gzを解凍し、解凍したフォルダに移動します。
```
shell>tar zxf transactd-linux-x86_64-1.1.0_mysql-5.6.14.tar.gz
shell>cd transactd-linux-x86_64-1.1.0_mysql-5.6.14
```

3. [MySQL|MariaDBインストールフォルダ]/lib/pluginに、libtransactd.soをコピー
   します。MySQL|MariaDBインストールが不明な場合は、mysqlを起動し以下の
   コマンドでplugin_dirを確認してください。
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

4. mysqlを起動し、以下のコマンドを実行します。
```
  mysql>INSTALL PLUGIN transactd SONAME 'libtransactd.so';
```
これでPluginのインストールは終了です。



Transactdクライアントバイナリの準備
-------------------------------------------------------------------------------
Transactd Pluginを介してデータにアクセスするにはTransactdクライアントが必要です。
プラットフォームに合ったビルド済Transactdクライアントをダウンロードします。
ダウンロードするファイルは

  * Windowsの場合 transactd-client-[platform]_with_sdk-1.1.0.msi
  * Linuxの場合 transactd-client-linux-x86_64_with_sdk-1.1.0.tar.gz

といった形式です。[platform]はwin32またはwin64です。
たとえば、LINUXの完全なURLは以下の通りです。

http://www.bizstation.jp/al/transactd/download/transactd-client/transactd-client-linux-x86_64_with_sdk-1.1.0.tar.gz



Transactdクライアントのインストール
-------------------------------------------------------------------------------

### Windowsでのインストール 
1. transactd-client-[platform]_with_sdk-1.1.0.msiをダブルクリックするとWindows
   インストーラによるインストールが始まります。あとは画面に従ってインストールし
   ます。

セットアップタイプの選択画面で[カスタム]を選択するとEmbarcadero C++Builder XE
シリーズ用のクライアントも選択できます。インストールできるコンパイラターゲットは

  * Microsoft Visual studio 2010用 (ActiveX(COM)クライアントを含みます)
  * Embarcadero C++Builder XEシリーズ用

の2種類です。Microsoft Visual studio 2010版については必ずインストールされます。


### Linuxでのインストール 
1. ダウンロードしたtar.gzのあるフォルダに移動します。
```
shell>cd [TargetFolder]
```

2. ダウンロードしたtar.gzを解凍し、解凍したフォルダに移動します。
```
shell>tar zxf transactd-client-linux-x86_64_with_sdk-1.1.0.tar.gz
shell>cd transactd-client-linux-x86_64_with_sdk-1.1.0
```

3. インストールスクリプトを実行します。
```
shell>./install_client.sh
```
これでクライアントのインストールは終了です。



Transactdへのアクセス可能ホストの設定
-------------------------------------------------------------------------------
Transactdへアクセスするには、事前にMySQLのuserテーブルにroot@[host]レコードを
登録しておく必要があります。

* ローカルからのアクセスであれば`root@127.0.0.1`を登録します。
* その他のホスト（たとえば192.168.0.3）であれば`root@192.168.0.3`を登録します。
* 192.168.0.xすべてであれば`root@192.168.0.0/255.255.255.0`とすることもできます。

登録はMySQL Command Line clientを起動し以下のように入力します。
```
mysql>CREATE USER root@'192.168.0.0/255.255.255.0';
```
この操作はrootでのアクセスを可能にしますので、rootのパスワードが未設定の場合は、
必ず設定してください。


### Windowsでのrootパスワードの設定
コマンドプロンプトを開きます。
```
shell>"C:\Program Files\MySQL\MySQL Server 5.6\bin\mysqladmin" -u root password 'xxxxx'
```
(xxxxxは実際のパスワードに置き換えてください)


### Linuxでのrootパスワードの設定
```
shell>/usr/local/mysql/bin/mysqladmin -u root password 'xxxxx'
```
(xxxxxは実際のパスワードに置き換えてください)


### root以外のユーザー名を使用する 
また、rootでのホスト設定ではなく別のユーザー名での登録も可能です。
別のユーザー名にするには、my.cnfまたはmy.iniの[mysqld]セクションに、
```
loose-transactd_hostcheck_username = yourUserName
```
を加えてください。yourUserNameは任意のユーザー名にできます。



テストとベンチマークの起動
-------------------------------------------------------------------------------
Transactd Pluginとクライアントのインストールが済んだら、テストとベンチマーク
を実行できます。MySQL/MariaDBサーバーは動作している状態で作業を進めます。

テストとベンチマークの詳しい内容は、このトピックス以降にあるそれぞれのトピックス
を参照してください。

### Windowsでの起動
1. スタートメニューから[すべてのプログラム]-[BizStation]-[Transactd Client]-
   [Test (compiler)(charset)]または[Benchmark local (compiler)]
   をクリックします。
   (compiler)はVC100またはBCBです。(charset)はUnicodeまたはMultibyteです。


### Linuxでの起動
1. クライアントのインストールで解凍したフォルダに移動します
```
shell>cd transactd-client-linux-x86_64_with_sdk-1.1.0
```

2. テストの起動
```
./exec_test_local.sh
```



アプリケーションの開発
-------------------------------------------------------------------------------
Transactdクライアントによるアプリケーションの開発は以下のSDKドキュメントを
参照してください。

http://www.bizstation.jp/ja/transactd/client/sdk/doc/

source/bzs/exampleフォルダに、簡単なサンプルコードがあります。
build/exampleフォルダにコンパイラに応じたプロジェクトファイル(Windows)、
または、make_example.sh(LINUX)スクリプトでこれらを実際にビルドできます。

Testプログラム
-------------------------------------------------------------------------------
test_で始まる実行ファイルはTestプログラムです。

実行は必ず1つのプログラムのインスタンスで行ってください。複数のインスタンスで
同時にtestを行うと失敗します。また、現在のバージョンではdrop databaseの際に他の
クライアントが動作しているとロックされたまま解放できない場合があります。そのよう
な場合はサーバーを再起動するか、MySQLコマンドラインクライアントにて 
```
mysql>drop database test;
```
を実行しデータベースを事前に削除してください。

コマンドライン引数に `--show_progress=yes` を追加すると下記のように実行状況をプロ
グレス表示します。
```
0%   10   20   30   40   50   60   70   80   90   100%
|----|----|----|----|----|----|----|----|----|----|
************
```

また、`--host=xxxxx` とするとテストを実行するサーバーを指定できます。

テストの中に、データベース名、テーブル名、フィールド名に日本語を使ったテストがあ
ります。これらが成功するには、my.cnfにてcharacter-set-serverがcp932またはutf8に
設定されている必要があります。
```
[mysqld]
character-set-server=utf8
```



ベンチマークプログラム
-------------------------------------------------------------------------------
ベンチマークプログラムは、コマンドライン引数のprocessNumberを変えることで複数の
インスタンスを同時に実行して計測することができます。
ベンチマークプログラムのコマンドラインオプションは以下の通りです。

```
bench_tdclcpp_xxx.exe databaseUri processNumber functionNumber

|----------------|--------------------------------------------------------|
| パラメータ     | 内容                                                   |
|----------------|--------------------------------------------------------|
| databaseUri    | データベースURIを指定します。                          |
|----------------|--------------------------------------------------------|
| processNumber  | 追加するデータの範囲を0からnで指定します。             |
|                | 複数インスタンスによる同時実行の際に異なる番号を指定   |
|                | することで、処理するレコードが競合しないようにします。 |
|----------------|--------------------------------------------------------|
| functionNumber | テストする処理を番号で指定します。                     |
|                | 処理は以下の通りです。                                 |
|                | -1: all function                                       |
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
shell>bench_tdclcpp_c_bcb_64.exe "tdap://localhost/test?dbfile=test.bdf" 0 -1
```



Transactd Pluginのライセンス
-------------------------------------------------------------------------------
MySQL/MariaDBのGPLv2ライセンスにより、Transactd PluginはGeneral Public License
 (GPLv2)のもとで利用可能です。ライセンス情報はCOPYINGをご覧ください。



Transactdクライアントのライセンス
-------------------------------------------------------------------------------
TransactdクライアントのライセンスはGeneral Public License (GPLv2)と商用ライセ
ンスのいずれから選ぶことができます。

* GPLv2のライセンス情報はCOPYINGファイルをご覧ください。
* Transactdクライアントを利用して作成したアプリケーションについて、GPLv2の
  制限を受けたくない場合は、商用ライセンスをご検討ください。
* 商用ライセンスの購入については、[BizStationホームページ](
  http://www.bizstation.jp/ja/transactd/support/)をご覧ください。

GPLv2以外のオープンソースプロジェクト向けの、TransactdクライアントOSS例外規定
も参照してください。
  - [TransactdクライアントOSS例外規定](http://www.bizstation.jp/ja/transactd/support/ossex.html)



バグ報告・要望・質問など
-------------------------------------------------------------------------------
* バグ報告・要望は、github上のIssueトラッカーにお寄せください。Issueトラッカー
  の利用にはgithubアカウント（無料）が必要です。
  - [Transactd Issues](https://github.com/bizstation/transactd/issues)
* 質問については、ウェブサイトを参照するか、上記のIssueトラッカーにお寄せくだ
  さい。
  - [Transactd Documents](http://www.bizstation.jp/ja/transactd/documents/)
  - [Transactd License/Support](http://www.bizstation.jp/ja/transactd/support/)



********************************************************************************

以下では、Transactd Plugin とTransactdクライアントに関係するBizStation以外
の著作権について表示します。

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
