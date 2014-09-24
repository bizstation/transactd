ormsrcgen C++ O/Rマッピングソースコードジェネレータ
===============================================================================
ormsrcgen(32|64) はC++ O/Rマッピングのためのソースコードジェネレータです。

C++では動的にクラスを定義することはできないため、ソースコードジェネレータで
モデルクラスを生成しコンパイルする形式を採ります。
これを使うとC++で簡単に高速なO/Rマッピングを行うことができます。



はじめに
-------------------------------------------------------------------------------
ジェネレータの関連ファイルは source/global/ormsrcgen/templateに格納されています。
それぞれのファイルの役割を説明します。

  * ormDataClass_template.cpp : C++ モデルクラスのテンプレート
  * ormDataClass_template.h   : C++ モデルクラスのテンプレート
  * ormMapClass_template.cpp  : C++ マップクラスのテンプレート
  * ormMapClass_template.h    : C++ マップクラスのテンプレート
  * template.cnf              : 設定ファイルのひな形
  * fieldNameList_sample.txt  : フィールド名変換ファイルのサンプル

このうち、template.cnfとfieldNameList_sample.txtを必要に応じて編集します。



コマンドラインオプション
-------------------------------------------------------------------------------
コマンドラインオプションは以下の通りです。

```
command line options:
  -d [ --database_uri ] database uri ex:tdap://hostname/dbname?dbfile=trnasctd_schema
  -t [ --table_name ] table name
  -c [ --class_name ] class name
  -f [ --conf_name ] configuration filename

-d :データベースを指定します。
-t :生成するモデルのテーブルを指定します。生成はテーブルごとに行います。
-c :-tで指定したテーブルに対応するモデルのクラス名を指定します。
-f :その他の設定項目を記述したテキストファイル(configuration file)です。
```

configuration fileは、多くの場合、テーブルが異なっても共通の内容です。ひな形が
`source/global/ormsrcgen/template/template.cnf`として用意されています。
これをコピーして内容を編集します。



configuration fileの書式
-------------------------------------------------------------------------------
* `lang = (language)`
  
  生成するコードの言語を指定します。現在のバージョンでは`C++`のみサポートされます。

* `files = (number of file)`
  
  1つのモデルのために必要なテンプレートファイルの数を指定します。C++の場合は`2`です。

* `file1 = (file path) file2 = (file path) ... fileN = (file path)`
  
  `files =`で指定した数のテンプレートファイルのパスを指定します。
  デフォルトでは`file1 = ormDataClass_template` `file2 = ormMapClass_template`
  となっていますが、実際に使用する際は絶対パスで指定してください。

* `saveDir = (output dir)`
  
  生成したソースコードを保存するフォルダを指定します。

* `setPrefix = (setter prefix)`
  
  クラスのメンバ変数に値をセットするアクセスメソッド名のプレフィックスを指定します。
  例えば、`setPrefix  = set`とした場合、`name`というフィールドの値をセットする
  メンバ関数は以下のようになります。
  
  ```
  setName(const char* v)
  ```

* `getPrefix = (getter prefix)`
  
  setPrefixと同様に値の取得メソッドにつけられるプレフィックスを指定します。

* `externWord = (extern keyword)`
  
  出力されたクラスをDLLやSOなどのライブラリに含めて共有する場合は、class 宣言の後ろに
  エクスポートのためのキーワードを付加できます。
  
  ```
  class $externWord someModel
  {
  ...
  };
  ```

* `fieldRenameList = (alias list file)`
  
  通常はテーブルに含まれるフィールド名がそのままクラスのメンバ変数名になります。
  異なる名前に変更したい場合は、このfieldRenameListで変換リストの書かれたテキスト
  ファイルを指定できます。変換リストは以下のように、変換前=変換後の形式で指定します。
  
  ```
  customer_id=id
  customer_name=name
  ```

* `name_space = (namespace of model class)`
  
  モデルクラスの名前空間を指定します。

* `name_space_map = (namespace of map class)`
  
  C++の場合、モデルクラスとともに、テーブルとクラスの復元と保存をマッピングする
  mapクラスも同時に生成されます。
  name_space_mapはそのマップクラスの名前空間を指定します。
