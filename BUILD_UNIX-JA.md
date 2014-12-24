Transactd ビルドガイド for Unix
============================================================

1. ビルド環境の準備
------------------------------------------------------------
Transactd Plugin および Transactd クライアントは、Unix上のGCC(64bit)
でビルドできます。ここでは、以下の環境でのビルドを想定して進めます。

* Ubuntu 12.10 x86_64
* gcc 4.7.2

ビルドにはgccバージョン4.4以上が必須です。
また、CentOS(GCC4.4以上 or clang LLVM 3.4)、Mac OS X 10.9(GCC4.4以上
or Xcode5.1 LLVM 3.4)でも同様にビルドできます。



2. CMakeのインストール
------------------------------------------------------------
[CMake](http://www.cmake.org)をインストールします。
```
sudo aptitude install cmake
```

CentOSでは
```
sudo yum install cmake
```

Mac OS Xでは、バイナリ配布をアプリケーションフォルダにインストールして
`/Applications/CMake.app/Contents/bin`にパスを通します。



3. Boost C++ Libraries のダウンロードとビルド
------------------------------------------------------------
[Boostのダウンロードページ](http://www.boost.org/users/download)から
ソースコードをダウンロードし、解凍します。
```
cd ~
wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.gz/download -O boost_1_54_0.tar.gz
tar xzf boost_1_54_0.tar.gz
```

以下のコマンドを実行しBoostのビルドを行います。
```
cd ~/boost_1_54_0
./bootstrap.sh --with-libraries=chrono,filesystem,system,thread,timer,serialization,program_options
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
wget http://cdn.mysql.com/Downloads/MySQL-5.6/mysql-5.6.20.tar.gz
tar xzf mysql-5.6.20.tar.gz
```


### 4-2 Transactd Pluginソースコードのダウンロード
[Transactd Pluginのダウンロードページ](http://www.bizstation.jp/al/transactd/download/index.asp)
からソースコードをダウンロードします。

ダウンロードしたソースコードは、4-1で展開したMySQLソースコードの
pluginディレクトリに展開します。
```
cd ~
wget http://www.bizstation.jp/al/transactd/download/transactd-2.1.0/transactd-source-2.1.0.zip
#Mac OS Xでは
#curl -O http://www.bizstation.jp/al/transactd/download/transactd-2.1.0/transactd-source-2.1.0.zip
unzip -q transactd-source-2.1.0.zip -d transactd
mv transactd ~/mysql-5.6.20/plugin/
```


### 4-3 CMakeの実行
以下のコマンドを実行します。後述するオプションに注意してください。
```
cd ~/mysql-5.6.20
mkdir bldgccx64
cd bldgccx64
cmake .. -DWITH_TRANSACTD_SERVER=ON -DWITH_TRANSACTD_CLIENTS=ON \
  -DBUILD_CONFIG=mysql_release \
  -DBOOST_ROOT=~/boost_1_54_0 \
  -DCMAKE_INSTALL_PREFIX=/usr/local/mysql-5.6.20 \
  -DTRANSACTD_CLIENTS_PREFIX=/usr/lib \
  -DTRANSACTD_PREFIX=/usr/local/transactd
```
* `CMAKE_INSTALL_PREFIX` に指定するのは、これからビルドするMySQLのインストール先
  です。
  ***すでにインストール済みのMySQLがある場合に、それと同じパスを指定すると、上書
  きされてしまいます。上書きしたくないときは異なるパスを指定してください。***
* `TRANSACTD_CLIENTS_PREFIX` に指定するのは、これからビルドするクライアントの
  インストール先です。デフォルトは`/usr/lib`です。
* `TRANSACTD_PREFIX`に指定するのは、これからビルドするテストやベンチマークの
  プログラムのインストール先です。デフォルトは`/usr/local/transactd`です。


### 4-4 ビルド
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
wget http://www.bizstation.jp/al/transactd/download/transactd-2.1.0/transactd-source-2.1.0.zip
#Mac OS Xでは
#curl curl -O http://www.bizstation.jp/al/transactd/download/transactd-2.1.0/transactd-source-2.1.0.zip
unzip -q transactd-source-2.1.0.zip -d transactd
```

### 5-2 CMakeの実行
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
* `TRANSACTD_CLIENTS_PREFIX` に指定するのは、これからビルドするクライアントの
  インストール先です。デフォルトは`/usr/lib`です。
* `TRANSACTD_PREFIX`に指定するのは、これからビルドするテストやベンチマークの
  プログラムのインストール先です。デフォルトは`/usr/local/transactd`です。


### 5-3 ビルド
CMakeが完了するとMakefileが生成されているので、makeコマンドでビルド・インストール
します。
```
make
make install
```
