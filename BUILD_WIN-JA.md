Transactd ビルドガイド for Windows
============================================================

1. コンパイラの準備
------------------------------------------------------------
Transactd Plugin および Transactd クライアントは、Windows上のVisual Studio
(32bit/64bit)でビルドできます。ここでは、以下の環境でのビルドを想定して進めます
。(クライアントはEmbacaderoのコンパイラーC++ Builder XEシリーズでもビルドできます。
[クライアントのみをビルド]の欄をご覧ください）

* Windows 7 (64bit)
* Visual Studio 2010 Express

Visual Studio 2010 Expressで64bitビルドを行うには、別途Windows SDKのインストール
が必要になります。32bitビルドの場合やExpress版でない場合は不要です。
以下のソフトウェアをインストールします。

* [Visual Studio 2010 Express ダウンロードページ（Microsoft）](
  http://www.microsoft.com/visualstudio/jpn/downloads#d-2010-express)
* [Windows SDK for Windows 7 ダウンロードページ（Microsoft Download Center）](
  http://www.microsoft.com/en-us/download/details.aspx?id=8442)
  （GRMSDKX_EN_DVD.iso をダウンロードし、マウントしてインストーラを実行）

### Visual Studioコマンドプロンプトの起動方法
後述する手順の中で、Visual Studioコマンドプロンプトを開くことがあります。
[スタートメニュー]-[すべてのプログラム]-[Microsoft Visual Studio 2010 Express]-
[Visual Studio コマンドプロンプト (2010)]を選択し起動します。

Visual Studio 2010 Express には、64bit版ビルドツール用の Visual Studioコマンド
プロンプトが付属しません。代わりに、64bit用ビルドツールのパスを設定するための
バッチファイルを作成し、そこから起動します。
以下の内容のファイルを作成し、`set64env.cmd`という名前でデスクトップに保存します
。
```
%comspec% /k "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
```



2. CMakeのダウンロードとインストール
------------------------------------------------------------
[CMake ダウンロードページ](http://www.cmake.org/cmake/resources/software.html)か
らWindows (Win32 Installer) をダウンロードし実行します。

途中、「Add CMake to the system PATH for all(or current) users」を選択し、
cmakeの実行ファイルにパスが通るようにします。



3. Boost C++ Libraries のダウンロードとインストール
------------------------------------------------------------
### 3-a ビルド済みバイナリのダウンロード
[BoostProのダウンロードページ](http://www.boostpro.com/download)から、
BoostPro 1.51.0 Installer (64-bit)をダウンロードし、インストールします。

BoostProインストーラの「Select Default Variants」では以下の項目にチェックを入れ
ます。

* Visual C++ 10.0
* Mlutithread, static runtime
* Mlutithread debug, static runtime

次の「Choose Components」では、以下のコンポーネント以外のチェックは外してもかま
いません。

* chrono
* filesystem
* system
* thread
* timer
* serialization
* program_options

インストールができたら、システム環境変数に `TI_BOOST_ROOT_32` および 
`TI_BOOST_ROOT_64`という変数を追加し、32bitと64bitのフォルダのパスを値として設
定します。自分のビルドするbitに合わせた変数が設定されていれば、両方を設定する必
要はありません。
```
TI_BOOST_ROOT_32 = c:\boost\boost_1_51_32
TI_BOOST_ROOT_64 = c:\boost\boost_1_51_64
```
環境変数の追加は、[コントロールパネル]-[システム]-[詳細設定]タブ-[環境変数]から
行います。


### 3-b ソースからのビルド
自分でboostをビルドする場合は、[Boostのダウンロードページ](
http://www.boost.org/users/download )からソースコードをダウンロードし、解凍しま
す。ここでは、以下のフォルダに保存したものとします。
```
C:\Program Files\boost\boost_1_54_0
```

Visual Studioコマンドプロンプトを起動します。以下のコマンドを実行し、Boostのビル
ドを行います。
```
cd C:\Program Files\boost\boost_1_54_0
bootstrap.bat
bjam.exe toolset=msvc threading=multi address-model=64 architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  variant=debug,release link=static runtime-link=static
bjam.exe toolset=msvc threading=multi address-model=64 architecture=x86 ^
  --with-chrono --with-filesystem --with-system --with-thread --with-timer ^
  variant=debug,release link=static runtime-link=shared
```

インストールができたら、システム環境変数に `TI_BOOST_ROOT_32` および 
`TI_BOOST_ROOT_64`という変数を追加し、32bitと64bitのフォルダのパスを値として設
定します。自分のビルドするbitに合わせた変数が設定されていれば、両方を設定する必
要はありません。
```
TI_BOOST_ROOT_32 = c:\boost\boost_1_51_32
TI_BOOST_ROOT_64 = c:\boost\boost_1_51_64
```
環境変数の追加は、[コントロールパネル]-[システム]-[詳細設定]タブ-[環境変数]から
行います。



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
C:\Users\Public\Documents\mysql-5.6.13
```


### 4-2 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。


ダウンロードしたソースコードを、4-1で展開したMySQLソースコードのpluginフォルダに
展開します。ここでは、以下のようなフォルダ構造になります。
```
C:\Users\Public\Documents\mysql-5.6.13\plugin\transactd
```


### 4-3 MySQLソースコードの修正

#### 4-3.1 パッチの適用
Transactd Pluginソースコードのpatchディレクトリにmysqlのソースコードを修正する
ためのパッチが含まれています。そのパッチをmysqlのソースディレクトリにコピー
します。(パッチはmysql/mariadbのバージョンごとに異なる名前になっています。
パッチ名のバージョンの部分は合ったものに変更してください。)
```
cd C:\Users\Public\Documents\mysql-5.6.13
copy plugin\transactd\patch\transactd-win-mysql-5.6.13.patch *
```

patchコマンドを使用できる環境（Cygwin、Git Bashなど）があるならば、以下の
コマンドでパッチを適用します。
```
cd C:\Users\Public\Documents\mysql-5.6.13
patch -p0 -i transactd-win-mysql-5.6.13.patch
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
cd C:\Users\Public\Documents\mysql-5.6.13
"C:\Program Files (x86)\patc254w\patch.exe" -p0 --binary -i transactd-win-mysql-5.6.13.patch
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
cd C:\Users\Public\Documents\mysql-5.6.13
md bldVC100x64
cd bldVC100x64
cmake .. -G "Visual Studio 10 Win64" ^
  -DWITH_TRANSACTD_SERVER=ON -DWITH_TRANSACTD_CLIENTS=ON ^
  -DBUILD_CONFIG=mysql_release
```


### 4-5 ビルド
CMakeが完了すると、以下のソリューションファイルが生成されています。
```
C:\Users\Public\Documents\mysql-5.6.13\bldVC100x64\MySQL.sln
```
MySQL.slnをVisual Studioで開きます。メニューの[ビルド]-[構成マネージャー]から構
成を「RelWithDebInfo」に変更し、[ビルド]-[ソリューションのビルド]をクリックしま
す。

バイナリは以下のフォルダに出力されます。
```
C:\Users\Public\Documents\mysql-5.6.13\bldVC100x64\sql\lib\plugin
C:\Users\Public\Documents\mysql-5.6.13\bldVC100x64\plugin\transactd\bin
C:\Users\Public\Documents\mysql-5.6.13\bldVC100x64\plugin\transactd\lib
```



5. クライアントのみをビルド
------------------------------------------------------------
### 5-1 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。

ここでは、以下のフォルダに展開したとします。
```
C:\Users\Public\Documents
```
(EmbacaderoのC++BuilderXEシリーズの場合は[5-4 C++BuilderXEシリーズでのビルド]に進んでください。)

### 5-2 CMakeの実行

Visual Studioコマンドプロンプトを起動して、以下のコマンドを実行します。
```
cd C:\Users\Public\Documents\transactd
md bldVC100x64
cd bldVC100x64
cmake .. -G "Visual Studio 10 Win64" ^
  -DWITH_TRANSACTD_SERVER=OFF -DWITH_TRANSACTD_CLIENTS=ON ^
  -DBOOST_ROOT="C:\Program Files\boost\boost_1_54_0"
```


### 5-3 ビルド
CMakeが完了すると、以下のソリューションファイルが生成されています。
```
C:\Users\Public\Documents\transactd\bldVC100x64\tdcl.sln
```
tdcl.slnをVisual Studioで開きます。メニューの[ビルド]-[構成マネージャー]から構
成を「RelWithDebInfo」に変更し、[ビルド]-[ソリューションのビルド]をクリックしま
す。

バイナリは以下のフォルダに出力されます。
```
C:\Users\Public\Documents\transactd\bldVC100x64\bin
C:\Users\Public\Documents\transactd\bldVC100x64\lib
```
### 5-4 C++BuilderXEシリーズでのビルド
EmbacaderoのC++BuilderXEシリーズの場合は

```
C:\Users\Public\Documents\Build\TransactdClient_bcb.groupproj
```
にてXE以降のコンパイラーでコンパイルできます。
コンパイルにはコンパイラー付属のboostライブラリがインストールされている必要が
あります。また、C++ Builderの[ツール]-[オプション]-[環境オプション]-[C++ オプション]
-[パスとディレクトリ]の[システムインクルードパス]に
 * 32Bitの場合 $(CG_BOOST_ROOT)
 * 64Bitの場合 $(CG_64_BOOST_ROOT)
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

XE4 64Bitの場合、コンパイラバージョンがXE3と同じためtdclcppを使用するアプリケーション
の自動リンクでtdclcpp_bc170_64x.libを探そうとします。本来XE4の場合bc180のためリンクエラー
が発生します。XE4 64Bitでtdclcppとtdclstmtをコンパイルした際には、libのファイル名を
tdclcpp_bc180_64x.libとtdclstmt_bc180_64x.libの180部分を170にリネームしてください。

XE6 64Bitの場合、boost_threadのコンパイルが通らないためboostのソースを修正します。
   
```
ファイル:$(CG_BOOST_ROOT)\boost_1_50\boost\asio\detail\impl\win_thread.ipp
52行目:  ::QueueUserAPC(apc_function, thread_, 0); --> ::QueueUserAPC((PAPCFUNC)apc_function, thread_, 0);
```
