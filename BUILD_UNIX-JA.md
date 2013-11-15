Transactd ビルドガイド for Unix
============================================================

1. ビルド環境の準備
------------------------------------------------------------
Transactd Plugin および Transactd クライアントは、Unix上のGCC(64bit)
でビルドできます。ここでは、以下の環境でのビルドを想定して進めます。

* Ubuntu 12.10 x86_64
* gcc 4.7.2



2. CMakeのインストール
------------------------------------------------------------
[CMake](http://www.cmake.org)をインストールします。
```
sudo aptitude install cmake
```



3. Boost C++ Libraries のダウンロードとビルド
------------------------------------------------------------
[Boostのダウンロードページ](http://www.boost.org/users/download )からソースコー
ドをダウンロードし、解凍します。
```
cd ~
wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.gz/download -O boost_1_54_0.tar.gz
tar -xzf boost_1_54_0.tar.gz
```

以下のコマンドを実行しBoostのビルドを行います。
```
cd ~/boost_1_54_0
./bootstrap.sh --with-libraries=chrono,filesystem,system,thread,timer
./b2 cxxflags=-fPIC
```



4. サーバープラグインとクライアントの両方をビルド
------------------------------------------------------------
クライアントのみをビルドしたい場合はこのステップを飛ばしてください。

### 4-1 MySQLソースコードのダウンロード
[MySQL Community Serverのダウンロードページ](http://dev.mysql.com/downloads/mysql)
からソースコードをダウンロードし、解凍します。
```
cd ~
wget http://cdn.mysql.com/Downloads/MySQL-5.6/mysql-5.6.13.tar.gz
tar -xzf mysql-5.6.13.tar.gz
```


### 4-2 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。

ダウンロードしたソースコードは、4-1で展開したMySQLソースコードのpluginディレ
クトリに展開します。
```
cd ~
wget http://www.bizstation.jp/al/transactd/download/transactd-1.1.0/transactd-source-1.1.0.zip
unzip -q transactd-source-1.1.0.zip
mv transactd ~/mysql-5.6.13/plugin/
```


### 4-3 BOMの削除（gcc 4.3 未満の場合）
gccのバージョンが4.3未満の場合、UTF-8 BOM付きのソースコードをビルドできない
ため、BOMを削除します。
バージョンは以下のコマンドで確認できます。
```
gcc --version
```

gccのバージョンが4.3未満だった場合、以下のコマンドで
で`transactd/source/bzs/test/trdclengn/test_trdclengn.cpp` のBOMを削除します。
```
cd ~/mysql-5.6.13/plugin/transactd/source/bzs/test/trdclengn/
cp test_trdclengn.cpp test_trdclengn.cpp.backup
file_length=`wc -c < test_trdclengn.cpp.backup`
tail -c `expr ${file_length} - 3` test_trdclengn.cpp.backup > test_trdclengn.cpp
```


### 4-4 CMakeの実行
以下のコマンドを実行します。後述するオプションに注意してください。
```
cd ~/mysql-5.6.13
mkdir bldgccx64
cd bldgccx64
cmake .. -DWITH_TRANSACTD_SERVER=ON -DWITH_TRANSACTD_CLIENTS=ON \
  -DBUILD_CONFIG=mysql_release \
  -DBOOST_ROOT=~/boost_1_54_0 \
  -DCMAKE_INSTALL_PREFIX=/usr/local/mysql-5.6.13 \
  -DTRANSACTD_CLIENTS_PREFIX=/usr/lib \
  -DTRANSACTD_PREFIX=/usr/local/transactd
```
* CMAKE_INSTALL_PREFIXに指定するのは、これからビルドするMySQLのインストール先で
  す。
  ***すでにインストール済みのMySQLがある場合に、それと同じパスを指定すると、上書
  きされてしまいます。上書きしたくないときは異なるパスを指定してください。***
* TRANSACTD_CLIENTS_PREFIXに指定するのは、これからビルドするクライアントのインス
  トール先です。デフォルトは「/usr/lib」です。
* TRANSACTD_PREFIXに指定するのは、これからビルドするテストやベンチマークのプログ
  ラムのインストール先です。デフォルトは「/usr/local/transactd」です。


### 4-5 ビルド
CMakeが完了するとMakefileが生成されているので、makeコマンドでビルド・インストール
します。
```
make
make install
```



5. クライアントライブラリのみをビルド
------------------------------------------------------------
### 5-1 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。
```
cd ~
wget http://www.bizstation.jp/al/transactd/download/transactd-1.1.0/transactd-source-1.1.0.zip
unzip -q transactd-source-1.1.0.zip
```


### 5-2 BOMの削除（gcc 4.3 未満の場合）
gccのバージョンが4.3未満の場合、UTF-8 BOM付きのソースコードをビルドできない
ため、BOMを削除します。
バージョンは以下のコマンドで確認できます。
```
gcc --version
```

gccのバージョンが4.3未満だった場合、以下のコマンドで
`transactd/source/bzs/test/trdclengn/test_trdclengn.cpp` のBOMを削除します。
```
cd ~/transactd/source/bzs/test/trdclengn/
cp test_trdclengn.cpp test_trdclengn.cpp.backup
file_length=`wc -c < test_trdclengn.cpp.backup`
tail -c `expr ${file_length} - 3` test_trdclengn.cpp.backup > test_trdclengn.cpp
```


### 5-3 CMakeの実行
以下のコマンドを実行します。後述するオプションに注意してください。
```
cd ~/transactd
mkdir bldgccx64
cd bldgccx64
cmake .. -DWITH_TRANSACTD_SERVER=OFF -DWITH_TRANSACTD_CLIENTS=ON \
  -DBOOST_ROOT=~/boost_1_54_0 \
  -DTRANSACTD_CLIENTS_PREFIX=/usr/lib \
  -DTRANSACTD_PREFIX=/usr/local/transactd
```
* TRANSACTD_CLIENTS_PREFIXに指定するのは、これからビルドするクライアントのインス
  トール先です。デフォルトは「/usr/lib」です。
* TRANSACTD_PREFIXに指定するのは、これからビルドするテストやベンチマークのプログ
  ラムのインストール先です。デフォルトは「/usr/local/transactd」です。


### 5-4 ビルド
CMakeが完了するとMakefileが生成されているので、makeコマンドでビルド・インストール
します。
```
make
make install
```
