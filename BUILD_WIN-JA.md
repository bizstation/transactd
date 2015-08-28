Transactd ビルドガイド for Windows
============================================================

1. コンパイラの準備
------------------------------------------------------------
Transactd Plugin および Transactd クライアントは、Windows上のVisual Studio
(32bit/64bit)でビルドできます。
Transactd Pluginは、MySQLとともにビルドしますので、MySQLの要求するコンパイラに
合わせてビルドします。MySQL 5.7は、Visual Studio Express 2013、それ以外の
MySQL/MariaDBのバージョンはVisual Studio Express 2010を使用してビルドします。

クライアントはEmbarcaderoのコンパイラーC++ Builder XEシリーズでもビルドできます。
[クライアントのみをビルド]の項を参照してください。）


MySQL 5.7は以下のコンパイラをダウンロードしてインストールします。
* [Visual Studio Express 2013 with Update 5 for Windows Desktop 
  ダウンロードページ（Microsoft）]
  (http://go.microsoft.com/fwlink/?LinkId=532500&clcid=0x411)

MySQL 5.5/5.6、MariaDB 5.5/10.0は以下のコンパイラとSDKをダウンロードし
てインストールします。

* [Visual Studio 2010 Express ダウンロードページ（Microsoft）](
  http://www.microsoft.com/visualstudio/jpn/downloads#d-2010-express)
* [Windows SDK for Windows 7 ダウンロードページ（Microsoft Download Center）](
  http://www.microsoft.com/en-us/download/details.aspx?id=8442)
  （GRMSDKX_EN_DVD.iso をダウンロードし、マウントしてインストーラを実行）


### Visual Studioコマンドプロンプトの起動方法
後述する手順の中で、Visual Studioコマンドプロンプトを開くことがあります。

MySQL 5.5/5.6、MariaDB 5.5/10.0は以下の手順でVisual Studioコマンドプロンプトを
開きます。

[32Bit]
* [スタートメニュー]-[すべてのプログラム]-[Microsoft Visual Studio 2010 Express]-
[Visual Studio コマンドプロンプト (2010)]を選択し起動します。

[64Bit]
*Visual Studio 2010 Express には、64bit版ビルドツール用の Visual Studioコマンド
プロンプトが付属しません。代わりに、64bit用ビルドツールのパスを設定するための
バッチファイルを作成し、そこから起動します。
以下の内容のファイルを作成し、`set64env.cmd`という名前でデスクトップに保存します。
```
%comspec% /k "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
```

MySQL 5.7は以下の手順でVisual Studioコマンドプロンプトを開きます。
[32Bit]
* [スタートメニュー]-[すべてのプログラム]-[Microsoft Visual Studio 2013]-
[Visual Studio ツール]-[VS2013 x86 Native Tools コマンド プロンプト]を選択し起動します。

[64Bit]
* [スタートメニュー]-[すべてのプログラム]-[Microsoft Visual Studio 2013]-
[Visual Studio ツール]-[VS2013 x64 Cross Tools コマンド プロンプト]を選択し起動します。



2. CMakeのダウンロードとインストール
------------------------------------------------------------
[CMake ダウンロードページ](http://www.cmake.org/cmake/resources/software.html)か
らWindows (Win32 Installer)  cmake-2.8.12.2-win32-x86.exe をダウンロードし実行します。

途中、「Add CMake to the system PATH for all(or current) users」を選択し、
cmakeの実行ファイルにパスが通るようにします。



3. Boost C++ Libraries のダウンロードとインストール
------------------------------------------------------------
boostはソースからビルドします。

### ソースからのビルド
[Boostのダウンロードページ](http://www.boost.org/users/download)からソースコードを
ダウンロードし、解凍します。ここでは、以下のフォルダに保存したものとします。
```
C:\boost\boost_1_58_0
```

Visual Studioコマンドプロンプトを起動します。以下のコマンドを実行し、Boostのビル
ドを行います。
```
cd C:\boost\boost_1_58_0
bootstrap.bat

@REM Visula studio 2010 64bitの場合
bjam.exe toolset=msvc-10.0 threading=multi address-model=64 architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  --with-serialization --with-program_options ^
  variant=debug,release link=static runtime-link=static,shared

@REM Visula studio 2010 32bitの場合
bjam.exe toolset=msvc-10.0 threading=multi architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  --with-serialization --with-program_options ^
  variant=debug,release link=static runtime-link=static,shared

@REM Visula studio 2013 64bitの場合
bjam.exe toolset=msvc-12.0 threading=multi address-model=64 architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  --with-serialization --with-program_options ^
  variant=debug,release link=static runtime-link=static,shared

@REM Visula studio 2013 32bitの場合
bjam.exe toolset=msvc-12.0 threading=multi architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  --with-serialization --with-program_options ^
  variant=debug,release link=static runtime-link=static,shared

```

4. サーバープラグインとクライアントの両方をビルド
------------------------------------------------------------
クライアントのみをビルドしたい場合はこのステップを飛ばしてください。

### 4-1 MySQLソースコードのダウンロード
[MySQL Community Serverのダウンロードページ](http://dev.mysql.com/downloads/mysql)
からソースコードをダウンロードします。
「Select Platform」のプルダウンから「Source Code」を選択し、Windows ZIP形式の
ファイルをダウンロードします。

ダウンロードした圧縮ファイルを解凍します。ここでは、以下のフォルダに保存したも
のとします。
```
C:\Users\Public\Documents\mysql-5.6.25
```


### 4-2 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。
ソースを展開するフォルダ transactd をMySQLのソースツリー内のpluginフォルダに作成
します。
```
md C:\Users\Public\Documents\mysql-5.6.25\plugin\transactd
```
ダウンロードしたソースコードを、上記で作成したtransactdフォルダに展開します。
ここでは、以下のようなフォルダ構造になります。
```
C:\Users\Public\Documents\mysql-5.6.25\plugin\transactd
```


### 4-3 MySQLソースコードの修正

#### 4-3.1 パッチの適用
Transactd Pluginソースコードのpatchディレクトリにmysqlのソースコードを修正する
ためのパッチが含まれています。そのパッチをmysqlのソースディレクトリにコピー
します。(パッチはmysql/mariadbのバージョンごとに異なる名前になっています。
パッチ名のバージョンの部分は合ったものに変更してください。)
```
cd C:\Users\Public\Documents\mysql-5.6.25
copy plugin\transactd\patch\transactd-win-mysql-5.6.25.patch *
```

patchコマンドを使用できる環境（Cygwin、Git Bashなど）があるならば、以下の
コマンドでパッチを適用します。
```
cd C:\Users\Public\Documents\mysql-5.6.25
patch -p0 -i transactd-win-mysql-5.6.25.patch
```

patchコマンドがない場合、[宮坂 賢 氏のGNU patch 2.5.4 (Win32 版)](
http://cetus.sakura.ne.jp/softlab/toolbox1/index.html#difpat)をダウンロードし
使用します。
ここでは、以下のフォルダに保存したものとします。
```
C:\Program Files (x86)\patc254w
```
以下のコマンドでパッチを適用します。
```
cd C:\Users\Public\Documents\mysql-5.6.25
"C:\Program Files (x86)\patc254w\patch.exe" -p0 --binary -i transactd-win-mysql-5.6.25.patch
```

#### 4-3.2 ソースコードのエンコーディングの修正
日本語Windows環境でビルドする場合、一部のソースコードのエンコーディングを変更する
必要があります。以下のファイルをVisual Studioから開き、[名前を付けて保存]-
[上書き保存]の右の▼をクリック-[エンコード付きで保存]を選択し、
「Unicode (UTF-8 シグネチャ付き)-コードページ 65001」で上書き保存します。

* sql\sql_locale.cc
* storage\perfschema\unittest\pfs_connect_attr-t.cc


### 4-4 CMakeの実行
Visual Studioコマンドプロンプトを起動して、以下のコマンドを実行します。
```
@REM Visual studio 2010 64Bitの場合 
cd C:\Users\Public\Documents\mysql-5.6.25
md x64
cd x64
cmake .. -G "Visual Studio 10 Win64" ^
   -DBOOST_ROOT="C:\boost\boost_1_58_0"

@REM Visual studio 2013 64Bitの場合
cd C:\Users\Public\Documents\mysql-5.7.8
md x64
cd x64
cmake .. -G "Visual Studio 12 Win64" ^
   -DWITH_BOOST="C:\boost\boost_1_58_0" ^
   -DBOOST_ROOT="C:\boost\boost_1_58_0"

```


### 4-5 ビルド
CMakeが完了すると、以下のソリューションファイルが生成されています。
```
C:\Users\Public\Documents\mysql-5.6.25\x64\MySQL.sln
```
MySQL.slnをVisual Studioで開きます。メニューの[ビルド]-[構成マネージャー]から構
成を「Release」に変更し、[ビルド]-[ソリューションのビルド]をクリックします。

バイナリは以下のフォルダに出力されます。
```
C:\Users\Public\Documents\mysql-5.6.25\x64\sql\lib\plugin
C:\Users\Public\Documents\mysql-5.6.25\x64\plugin\transactd\bin
C:\Users\Public\Documents\mysql-5.6.25\x64\plugin\transactd\lib
```



5. クライアントのみをビルド
------------------------------------------------------------
### 5-1 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。

ここでは、以下のフォルダに展開したとします。
```
C:\Users\Public\Documents\transactd
```
（EmbarcaderoのC++BuilderXEシリーズの場合は[5-4 C++BuilderXEシリーズでのビルド]に進んでください。）


### 5-2 CMakeの実行
Visual Studioコマンドプロンプトを起動して、以下のコマンドを実行します。
```
@REM Visual studio 2010 64Bitの場合 
cd C:\Users\Public\Documents\transactd
md x64
cd x64
cmake .. -G "Visual Studio 10 Win64" ^
  -DBOOST_ROOT="C:\boost\boost_1_58_0"  ^
  -DWITH_TRANSACTD_SERVER=OFF -DWITH_TRANSACTD_CLIENTS=ON

@REM Visual studio 2013 64Bitの場合 
cd C:\Users\Public\Documents\transactd
md x64
cd x64
cmake .. -G "Visual Studio 12 Win64" ^
  -DBOOST_ROOT="C:\boost\boost_1_58_0" ^
  -DWITH_TRANSACTD_SERVER=OFF -DWITH_TRANSACTD_CLIENTS=ON

```


### 5-3 ビルド
CMakeが完了すると、以下のソリューションファイル (TransactdClinet.sln)が生成されています。
(Version 2.4.0以前は tdcl.sln)
```
C:\Users\Public\Documents\transactd\x64\TransactdClinet.sln
```
TransactdClinet.slnをVisual Studioで開きます。メニューの[ビルド]-[構成マネージャー]から構
成を「Release」に変更し、[ビルド]-[ソリューションのビルド]をクリックしま
す。

バイナリは以下のフォルダに出力されます。
```
C:\Users\Public\Documents\transactd\x64\bin
C:\Users\Public\Documents\transactd\x64\lib
```


### 5-4 C++BuilderXEシリーズでのビルド
EmbarcaderoのC++BuilderXEシリーズの場合は
```
C:\Users\Public\Documents\Build\TransactdClient_bcb.groupproj
```
にてXE以降のコンパイラーでコンパイルできます。

コンパイルにはコンパイラー付属のboostライブラリがインストールされている必要が
あります。また、C++ Builderの[ツール]-[オプション]-[環境オプション]-[C++ オプション]
-[パスとディレクトリ]の[システムインクルードパス]に
```
32Bitの場合 $(CG_BOOST_ROOT)
64Bitの場合 $(CG_64_BOOST_ROOT)
```
を追加します。
ビルド構成は、Unicode版/Ansi版 Release/Debug 32Bit/64Bit があります。
出力は、bin libフォルダに生成されます。64Bitの場合は常に動的RTLとリンクが必要です。

以下はバージョンごとの補足事項です。

32Bit(すべてのXEバージョン)の場合、boost_program_optionsがコンパイルされていないため、
```
build\libboost_program_options-bcb1_39\libboost_program_options-bcb-mt-1_39.cbproj
```
にて事前にlibboost_program_options-bcb-mt-1_39.libを生成してください。
コンパイルの前に、下記のboostのソースの修正が必要です。
```
ファイル:$(CG_BOOST_ROOT)\boost_1_39\libs\program_options\src\variables_map.cpp
71行目  :v = variable_value();  -->  variable_value vr;v = vr;
115行目 :m[key] = variable_value(def, true);  -->  variable_value vr(def, true);m[key] = vr;
```

XE2の場合、boost_serializationがコンパイルされていないため、
```
build\libboost_serialization-bcb-1_39\libboost_boost_serialization-bcb-mt-1_39.cbproj
```
にて事前にlibboost_serialization-bcb-mt-1_39.libを生成してください。

XE4 64Bitの場合、コンパイラバージョンがXE3と同じであるため、tdclcppを使用する
アプリケーションの自動リンクでtdclcpp_bc170_64x.libを探そうとします。しかしXE4で
生成されるdllはtdclcpp_bc180_64x.dllのため、libが見つからずリンクエラーが発生します。
XE4 64Bitでtdclcppとtdclstmtをコンパイルした際には、libのファイル名の180部分を170に
リネームし`tdclcpp_bc180_64x_xx.lib`と`tdclstmt_bc180_64x_xx.lib`にしてください。

XE6 64Bitの場合、boost_threadのコンパイルが通らないためboostのソースを修正します。
```
ファイル:$(CG_BOOST_ROOT)\boost_1_50\boost\asio\detail\impl\win_thread.ipp
52行目:  ::QueueUserAPC(apc_function, thread_, 0); --> ::QueueUserAPC((PAPCFUNC)apc_function, thread_, 0);
```
