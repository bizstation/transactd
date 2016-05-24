リリースノート
================================================================================
Version 3.4.1 2016/05/23
================================================================================
修正と変更点
--------------------------------------------------------------------------------
* `bookmark`の取得で長さが112バイトを超えるプライマリキーの場合、正しく動作しな
  い不具合を修正しました。

* `dbdef`クラスにおけるフィールド定義の検査で、`mychar`の長さチェックが文字数で
  はなくバイト数で行われていた不具合を修正しました。

* データベースがアソシエイトデータベースだった場合、`database::getCreateView`が
  `stat`に12を返す不具合を修正しました。


================================================================================
Version 3.4.0 2016/05/11
================================================================================
新機能
--------------------------------------------------------------------------------
* `beginSnapshot()`でMySQL5.6と5.7のGTIDに対応しました。binlogポジションとともに
  GTIDを返します。

* `connMgr::slaveStatus()`でMySQLおよびMariaDBのGTIDで拡張されたステータスを取得
  可能にしました。

* 結果セットの無いSQLを実行する`database::execSql()`関数を追加しました。この関数
  で成功/不成功のみを返すSQL文を実行可能です。

修正と変更点
--------------------------------------------------------------------------------
* Windows 10 64bitのnamed pipe接続で、共有メモリ名が正しくないため接続できない
  不具合を修正しました。

* named pipe接続で標準以外のポート番号のサーバーに正しく接続できない不具合を修正
  しました。

* `stripAuth()`関数でバッファーオーバランする不具合を修正しました。

* `tdclcpp:fielddef::setSchemaCodePage()`メソッドをパブリックメソッドに変更しま
  した。


================================================================================
Version 3.3.0 2016/04/18
================================================================================
新機能
--------------------------------------------------------------------------------
* `const records& connMgr::statusvars()`が追加されました。この関数はTransactd 
  status variable を取得可能です。

* `connMgr::slaveStatus()`メソッドでデフォルトバッファの67バイトを超えるメッセージ
  を取得可能にしました。

修正と変更点
--------------------------------------------------------------------------------
* PHPで64Bit Integerの変換不具合を修正しました。

* ホスト認証時にサーバーエラーが発生する不具合を修正しました。

* WindowsのPHPでバージョン 5.4 以下はサポートされなくなりました。WindowsではPHP 
  5.5以降がサポートされます。Unix系OSではPHP5.4もサポートされます。


================================================================================
Version 3.2.1 2016/04/11
================================================================================
修正と変更点
--------------------------------------------------------------------------------
* `connMgr::postDisconnectOne`が正しく動作しない不具合を修正しました。

* `nstable::getDirURI`が条件によって正しいURIを返さない不具合を修正しました。


================================================================================
Version 3.2.0 2016/04/07
================================================================================
新機能
--------------------------------------------------------------------------------
* `connMgr`クラスが追加されました。このクラスは以下の情報取得や処理が行えます。
  1. Transactdクライアントの接続、データベース、テーブルの使用状況情報の取得
  2. Transactdクライアントの強制切断
  3. データベース、テーブル、ビューなどの一覧情報の取得
  4. Transactdサーバー状態の取得
  5. レプリケーションスレーブ状態の取得
  
  詳細はリファレンスの`connMgr`クラスを参照してください。

* 存在するテーブルとビューの生成SQL文を取得可能にしました。これらは
  `SHOW CREATE TABLE`で返されるものと同様な内容です。データベースのコピー処理など
  で、オリジナルと全く同じものを生成することを可能にします。

* 同じサーバー内の2つ以上のデータベースにまたがるトランザクションを処理可能にし
  ました。接続とトランザクションを共有するアソシエイトデータベースオブジェクトを
  生成することで可能です。詳細はリファレンスの`database::createAssociate()`を参照
  してください。

修正と変更点
--------------------------------------------------------------------------------
* `codePage`から`charsetindex`を検索する際に、`cp932`は`sjis`に優先して返すように
  変更しました。

* 1つの`database`オブジェクトで扱えるテーブルの最大数を50から150に増やしました。

* `nsdatabase::connect()`で既にデータベースがオープンされている場合は、接続に失敗
  し、`stat()`に`STATUS_DB_YET_OPEN`を返すようにしました。

* 動的に生成したスキーマをコピーすると、コピー先の`database`の解放でハングするバ
  グを修正しました。

* 不正なキー番号を指定するとサーバーがハングすることがあるバグを修正しました。

* `ft_myfixedbinary`フィールドタイプの長さ判定のバグを修正しました。

* `recordset:groupBy()`のフィールド値の比較関数で、文字列の場合すべての長さが比較
  されていないバグを修正しました。

* NULLを許可するフィールドとBLOBフィールドを含むテーブルで、BLOBフィールドを正し
  く処理できないことがある不具合を修正しました。


================================================================================
Version 3.1.0 2016/03/03
================================================================================
新機能
--------------------------------------------------------------------------------
* CONSISTENT_READモードのスナップショット開始時に、バイナリログのポジションが取得
  可能になりました。

* `database::drop`で事前にデータベースを開かずにuriを指定してdropできるようになり
  ました。スキーマテーブルが損傷していた場合などでもdropできます。

* `database::copyTableData`でバルクインサートとマルチレコード読み取りを可能にし、
  高速化しました。

* テーブルの作成処理でテーブルのデータ圧縮の指定を可能にしました。
  `tabledef::flags.bit3`がONの場合データ圧縮が有効になります。これはMySQLの
  `CREATE TABLE`の`ROW_FORMAT=COMPRESSED`オプションを使用した機能です。

修正と変更点
--------------------------------------------------------------------------------
* レコードの読み取りで、`query::limit`で指定したレコードに満たない場合がある不具
  合を修正しました。

* P.SQLにおいてテーブルが作成できない不具合を修正しました。

* MySQL 5.7で native_passwordモードの際に認証できないエラーを修正しました。

* キー番号リゾルバに無効なポインタが保持されるため、テーブルのリオープン時に
  segfaultが発生することがある不具合を修正しました。

* レコード数の統計値で挿入・更新時に読み取り数が加算されてしまう不具合を修正しま
  した。

* トランザクション内でDDL処理を開始できてしまう不具合を修正しました。

* フィールドごとに文字コードが異なる場合正しくエンコードされないことがある不具合
  を修正しました。

* トランザクションとスナップショット関連のAPIの結果ステータスをより詳細に返すよ
  うに改善しました。

* `database`オブジェクトの`copyDataFn`コールバック関数のパラメータに`table`への
  ポインタを追加しました。

* `dbdef::getSQLcreateTable`が返す文字列のエンコードを`utf8`固定にしました。

* `table::find`系メソッドのフィールドの選択でNULLを許可するフィールドの数のカウ
  ントミスを修正しました。

* `recordset::appendField`で追加されたフィールドの`charsetIndex`をデフォルトで
  先頭フィールドと同じ値にするようにしました。


================================================================================
Version 3.0.0 2015/12/26
================================================================================
バージョンアップ上の注意点
--------------------------------------------------------------------------------
* サーバー クライアントのバージョン互換性
  サーバー/クライアントともに2.4系との相互運用が可能です。(3.0の新機能を除く）
  但し、NULL許可フィールドの扱いが異なっています。従来と同じ扱いのモードが用意され
  ています。データベースを開く前に、
  database::setCompatibleMode(CMP_MODE_OLD_NULL)を呼び出してください。

新機能
--------------------------------------------------------------------------------
* フィールドの値でNULLの設定、読み取りを可能にしました。
  NULLを許可するフィールドの扱いの違いが問題にならないよう、従来と同じモードでの
  動作も可能です。データベースを開く前に、
  database::setCompatibleMode(CMP_MODE_OLD_NULL)を呼び出してください。NULLの扱い
  が2.4系と同じになります。
  詳細はhttp://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html
  をご覧ください。

* フィールドのデフォルト値に対応しました。
  詳細はhttp://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html
  をご覧ください。

* スキーマテーブルを使用しないアクセスを可能にしました。
  詳細はhttp://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html
  をご覧ください。

* PHP ExtensionがPHP7に対応しました。

* ビット型のフィールドアクセスを簡単にするbitsetクラスを追加しました。
  フィールドの値の取得/設定に、bistsetクラスでの受け取りと設定を行うことができま
  す。

* MySQL DECIMAL型に対応しました。

* MySQL/Mariadbのバージョンによってデータ構造が異なる、
  TIME / DATETIME / TIMESTAMP 型に完全に対応しました。

* MySQL DATE型の1900年のオフセットをクライアントライブラリ内に実装しました。

* MySQL 5.7 Mariadb 10.1 系に完全対応

修正と変更点
--------------------------------------------------------------------------------
* サーバー設定
  timestamp_always 変数を追加しました。詳しくは以下を参照してください。
  http://www.bizstation.jp/ja/transactd/documents/admin_manual.html#mycnf

* テストの追加
  Version3の新たな機能をテストするためのテストが追加されました。
  (test_tdclcpp_v3.cpp test_v3.js transactd_v3_Test.php transactd_v3_spec.rb)

* リファクタリング
  tdclcppのクラスのリファクタリングが行われています。

* クラスの追加
  class bitset

* パブリックメソッド、メンバーの追加
  ```
  void               fielddef::setDefaultValue(const wchar_t* s) 
  inline void        fielddef::setDefaultValue(const char* s) 
  void               fielddef::setDefaultValue(double v) 
  inline const char* fielddef::defaultValue_str() const 
  const char*        fielddef::defaultValue_strA() const 
  const wchar_t*     fielddef::defaultValue_str() const 
  inline bool        fielddef::isPadCharType() const 
  inline bool        fielddef::isDateTimeType() const 
  bool               fielddef::isValidCharNum() const
  inline bool        fielddef::isNullable() const
  void               fielddef::setNullable(bool v, bool defaultNull = true) 
  void               fielddef::setTimeStampOnUpdate(bool v) 
  bool               fielddef::isTimeStampOnUpdate() const  
  inline double      fielddef::defaultValue() const 
  inline double      fielddef::isDefaultNull() const 
  uint_td            fielddef::varLenBytes() const
  uint_td            fielddef::blobLenBytes() const 
  void               fielddef::setDecimalDigits(int dig, int dec)
  bool               fielddef::isIntegerType() const
  void               fielddef::setDefaultValue(__int64 v)
  void               fielddef::setDefaultValue(bitset& v)
  __int64            fielddef::defaultValue64() const
  bool               fielddef::operator==(const fielddef& r) const
  const wchar_t*     fielddef::defaultValue_str() const 
  inline double      fielddef::defaultValue() const 
  ushort_td          fielddef::digits 
  inline uchar_td    tabledef::nullbytes() const 
  inline uchar_td    tabledef::nullfields() const 
  inline uchar_td    tabledef::inUse() const 
  inline bool        tabledef::isMysqlNullMode() const
  int                tabledef::size() const 
  short              tabledef::fieldNumByName(const _TCHAR* name) const 
  inline ushort_td   tabledef::recordlen() const 
  void               tabledef::setValidationTarget(bool isMariadb, uchar_td srvMinorVersion)
  bool               tabledef::isLegacyTimeFormat(const fielddef& fd) const
  bool               tabledef::operator==(const tabledef& r) const
  bool               keydef::operator==(const keydef& r) const
  void               dbdef::synchronizeSeverSchema(short tableIndex) 
  bool               database::autoSchemaUseNullkey() const 
  void               database::setAutoSchemaUseNullkey(bool v) 
  static void        database::setCompatibleMode(int mode) 
  static int         database::compatibleMode() 
  bool               database::createTable(const char* sql)
  char*              database::getSqlStringForCreateTable(const _TCHAR* tableName, char* retbuf, uint_td*  size)
  static const int   database::CMP_MODE_MYSQL_NULL = 1
  static const int   database::CMP_MODE_OLD_NULL =  0
  void               nstable::test_store(const char* values)
  void               nstable::setTimestampMode(int mode)
  bool               table::getFVNull(short index) const 
  bool               table::getFVNull(const _TCHAR* fieldName) const
  void               table::setFVNull(short index, bool v) 
  void               table::setFVNull(const _TCHAR* fieldName, bool v) 
  bitset             table::getFVbits(const _TCHAR* fieldName)
  bitset             table::getFVbits(short index)
  void               table::setFV(short index, const bitset& )
  void               table::setFV(const fieldName, const bitset& )
  enum               table::eNullReset::clearNull
  enum               table::eNullReset::defaultNull
  void               fielddefs::addAllFileds(const tabledef* def) 
  void               fielddefs::addSelectedFields(const class table* tb) 
  bool               field::isNull() const 
  void               field::setNull(bool v) 
  bitset             field::getBits()
  void               field::operator=(const bitset&)
  int                field::i()
  __int64            field::i64()
  double             field::d()
  const _TCHAR*      field::str()
  const void*        field::bin()
  void               field::setValue()
  void               field::setBin()
  query&             query::whereIsNull(const _TCHAR* name) 
  query&             query::whereIsNotNull(const _TCHAR* name) 
  query&             query::andIsNull(const _TCHAR* name) 
  query&             query::andIsNotNull(const _TCHAR* name) 
  query&             query::orIsNull(const _TCHAR* name) 
  query&             query::orIsNotNull(const _TCHAR* name) 
  query&             query::segmentsForInValue(int v)
  void               activeTable::keyValue(const bitset& )
  recordsetQuery&    recordsetQuery::whenIsNull
  recordsetQuery&    recordsetQuery::whenIsNotNull
  recordsetQuery&    recordsetQuery::andIsNull
  recordsetQuery&    recordsetQuery::andIsNotNull
  recordsetQuery&    recordsetQuery::orIsNull
  recordsetQuery&    recordsetQuery::orIsNotNull
  bool               btrVersion::isSupportDateTimeTimeStamp() const
  bool               btrVersion::isSupportMultiTimeStamp() const
  bool               btrVersion::isMariaDB() const
  bool               btrVersion::isMysql56TimeFormat() const
  bool               btrVersion::isFullLegacyTimeFormat() const
  ```
 
* パブリック定数の追加
  ```
  enum   eCompType::eBitAnd = 8,
  enum   eCompType::eNotBitAnd = 9,
  enum   eCompType::eIsNull = 10,
  enum   eCompType::eIsNotNull = 11
  #define ft_myyear                       59
  #define ft_mygeometry                   60
  #define ft_myjson                       61
  #define ft_mydecimal                    62
  #define TIMESTAMP_VALUE_CONTROL         0
  #define TIMESTAMP_ALWAYS                1
  #define STATUS_TOO_LARGE_VALUE          -44
  ```

* 削除されたパブリックメソッド
  ```
  ushort_td          dbdef::getRecordLen(short tableIndex) 
  double             field::getFVnumeric() const 
  double             field::getFVDecimal() const 
  void               field::setFVDecimal(double data) 
  void               field::setFVNumeric(double data) 
  ```

* メソッド引数の変更
  ```
  short             database::copyTableData(table* dest, table* src, bool turbo, short keyNum = -1, int maxSkip = -1) 
  void              table::clearBuffer(eNullReset resetType = defaultNull) 
  ```


================================================================================
Version 2.4.5 2015/10/29
================================================================================
修正と変更点
--------------------------------------------------------------------------------
* MySQL 5.7.9、MariaDB 10.1.8に対応しました。

* MYSQL_TYPE_NEWDATE、MYSQL_TYPE_TIME2、MYSQL_TYPE_DATETIME2、
  MYSQL_TYPE_TIMESTAMP2に対応しました。


================================================================================
Version 2.4.4 2015/09/08
================================================================================
修正と変更点
--------------------------------------------------------------------------------
* pooledDbManagerのWindowsにおいて、プロセス終了時にtdclc_xxx.dll内でデッドロック
  するが発生することがある不具合を修正しました。

* nstable、nsdatabase、dbdefの３つのクラスにstatMsgメソッドを追加しました。この
  メソッドは、既にあるtdapErrメソッドと引数と戻り値の違い以外は同じ機能です。
* nsdatabase::readDatabaseDirectoryが正しく動作しない不具合を修正しました。

* PHPとRubyインターフェースでtdapErrメソッドが正しく動作しない不具合を修正しまし
  た。この２つのインターフェースでは、tdapErrメソッドは、statMsgに変更されました。
  また、C++とCOMインターフェースにstatMsgを追加しました。

* COMインターフェースでIErrorInfoをサポートしました。またIRecordset, IRecord, 
  IKeyDef, IFlags, IFieldDefs, ISortFields, IFieldNames でデフォルトプロパティー
  を設定しました。


================================================================================
Version 2.4.3 2015/08/31
================================================================================
修正と変更点 (Client only)
--------------------------------------------------------------------------------
* ユニオンされたレコードセットのJoinが正しく動作しない不具合を修正しました。


================================================================================
Version 2.4.2 2015/08/31
================================================================================
新機能
--------------------------------------------------------------------------------
* queryBase と recordsetQueryの演算子にビットAND演算子を追加しました。
  従来からある6つの演算子 ( =, >, < , >= ,<= ,<> )に加えて '&' と '!&'が使用でき
  ます。
  '&'演算子はフィールドの値とビット演算を行い指定した値と同じであればマッチします。
  '!&'演算子はフィールドの値とビット演算を行い指定した値でなければマッチします。
  例 : flags & 8 and flags !& 16

その他の修正と変更点
--------------------------------------------------------------------------------
* メソッドの変更
  field::addAllFileds(tabledef* def)をprotectedからpublicに変更

* メソッドパラメータの追加
  writableRecord::delとwritableRecord::updateにbool noSeek=false
  オプションを追加しました。noSeekをtrueにすると、削除や更新の際にカレントレコード
  は確立済として、読み取りオペレーションを省略します。

* メソッドの追加
  void queryStatements::move(int from, int to)

* Windowsでの接続タイムアウトが設定どおりに働かない不具合を修正しました。

* HasManyJoinにおいてJoinするレコードがなかった場合、正しく処理されないことがある
  不具合を修正しました。

* Joinにおいて、結合キーが文字列の場合正しく検索できないことがある不具合を修正しま
  した。


================================================================================
Version 2.4.0 2015/06/03
================================================================================
バージョンアップ上の注意点
--------------------------------------------------------------------------------
* サーバー クライアントのバージョン互換性
  
  Version 2.3系からのバージョンアップは特に注意する点はありません。サーバー、クラ
  イアントともに、2.4系との相互運用が可能です。（但し、2.4の新機能を除く）
  
  Version 2.3より前のバージョンからのアップグレードは以前のリリースノートを確認
  してください。

新機能
--------------------------------------------------------------------------------
* Transactd studio
  
  Transactdに、使用中のコネクション、データベース、テーブル情報を表示するための
  APIが実装されました。テーブル情報では、テーブルをオープンしてからの読み取り、更
  新、追加、削除したレコード数などを得ることができます。

* activeTableでbookmarkからのrecordsetの取得を可能にしました。
  
  queryBase::addSeekBookmarkを使って複数のbookmarkをセットできます。
  
  tableオブジェクトにqueryをセットしてからtable::recordCount()を呼び出すと、条件
  にマッチしたレコードを数えるのと同時に、table::bookmarks()でそれらのbookmarkを
  取得できます。

* C++APIにおいて以下のメソッドを、フリースレッド対応しました。複数のスレッドから
  同時アクセス可能です。
  * table::insertBookmarks
  * table::moveBookmarks
  * table::bookmarksCount
  * table::bookmarks

その他の修正と変更点
--------------------------------------------------------------------------------
* P.SQL互換のヌルキーアクセスにおいて、ヌルキーかどうかの判定ミスすることがある不
  具合を修正しました。

* 既存のテーブルの自動スキーマ生成がうまくいかない場合がある不具合を修正しました。

* サーバー統計情報を取得するためのロック制御を改善しました。モニタリング時の同時実
  効性を向上させました。

* nsdatabase::isReconnected()メソッドを追加しました。
  サーバーへの再接続を行ったかどうかを示します。

* １つの接続で複数データベースを使用した場合の reconnectをサポートしました。

* テーブルの作成時にヌルキーがあった場合、不要な内部用ヌルフィールドを追加してしま
  うことがある不具合を修正しました。

* 無効なプリペアードクエリハンドル受取ったときに、無効なポインタ操作してしまうバグ
  を修正しました。

* SQLアクセスによるロックと共存できないことがある不具合を修正しました。

* メソッドの追加
  * short dbdef::validateTableDef(short TableIndex)
  * ushort_td nstable::bookmarkLen() const
  * bookmark_td tabale::bookmarks(unsigned int index) const
  * recordCountFn Call back function
  * table::setOnRecordCount
  * table::onRecordCount
  * bool queryBase::isSeekByBookmarks() const
  * void queryBase::addSeekBookmark(bookmark_td& bm, ushort_td len, bool reset=false)
  * bool writableRecord::read(bookmark_td& bm)

* メソッド名の変更 
  * table::setBookMarks     --> table::insertBookmarks
  * table::moveBookmarksId  --> table::moveBookmarks
  * table::bookMarksCount   --> table::bookmarksCount

* メソッドパラメータの追加
  * void database::close(bool withDropDefaultSchema = false)
  * activeTable::activeTable(database* db, short tableIndex, 
                                  short mode = TD_OPEN_NORMAL)
  * static activeTable* create(database* db, short tableIndex,
                                  short mode = TD_OPEN_NORMAL);

* bookmark_td型の変更 unsigned int から
  
  ```
  struct BOOKMARK
  {
      uchar_td val[MAX_BOOKMARK_SIZE];
      bool empty;
      BOOKMARK():empty(true){ }
      bool isEmpty(){ return empty; }
      void set(uchar_td* p, int len)
      {
          memcpy(val, p, len);
          empty = false;
      }
  };
  ```

* bookmark_td型を引数に取る関数で、bookmark_td型を参照に変更しました。

* コンビニエンスAPIのopenTableの引数をフル引数対応しました。

* コンビニエンスAPIに以下の関数を追加しました。
  * void deleteTable(dbdef* def, short id)
  * void renumberTable(dbdef* def, short id, short newid)
  * void deleteField(dbdef* def, short tableid, short fieldNum)
  * void deleteKey(dbdef* def, short tableid, short keynum)
  * void validateTableDef(dbdef* def, short tableid)

* Transactd Client with SDK のWindows版に、Embarcadero C++Builder用バイナリは
  含まれなくなりました。Embarcadero C++Builderで使用する場合は、ソースからビルド
  することで引き続き利用可能です。
  
  http://www.bizstation.jp/ja/transactd/documents/BUILD_WIN.html


================================================================================
Version 2.3.0 2015/03/20
================================================================================
バージョンアップ上の注意点
--------------------------------------------------------------------------------
* サーバー クライアントのバージョン互換性
  
  バージョンアップはサーバーとクライアントともに行う必要があります。
  
  database::reconnect()の実装のために、クラインアント サーバー間の通信プロトコ
  ルが変更されました。このため、このバージョンのプラグインとクライアントは過去
  のバージョンと互換性がありません。
  
  新しいクライアントでバージョン2.2以前のサーバーにアクセスすると、
  SERVER_CLIENT_NOT_COMPATIBLE (3003) エラーが返ります。

* メソッドの移動
  
  table::usePadChar()とtable::trimPadChar()関数はfielddef構造体に移動しました。
  
  また、値の設定は、それぞれ別にあった関数が、
  setPadCharSettings(bool set, bool trim)にまとめられました。これによって、これ
  らの値をスキーマに保存できるようになりました。
  
  この変更は、フィールド型がft_string ft_wstring ft_mychar ft_mywcharにのみ影響
  があります。それ以外の型には影響ありません。
  
  table::usePadChar()とtrimPadChar()の返す値はどちらもデフォルトでtrueでしたが
  この設定をコードで変更していなければ動作に変更はありません。
  変更していた場合は、そのテーブルの上記フィールド型を持つフィールドに対して設定
  を行うようコードの修正が必要です。
  
  たとえば、tb->setUsePadChar(false)としていた場合、
  ```
  for (int i = 0 ; i < tb->tableDef()->fieldCount ; ++i)
  {
    fielddef* fd = const_cast<fielddef*>(&tb->tableDef()->fieldDefs[i]);
    fd->setPadCharSettings(false/*set*/, true/*trim*/);
  }
  ```
  とします。
  
  ただし、このコードは、揮発性でスキーマには保存されていません。必要に応じて保存する
  コードを追加してください。

* トランザクション中のテーブルオープン
  
  バイナリーログが有効で、トランザクション中にテーブルをオープンすると
  STATUS_ALREADY_INTRANSACTIONエラーを返すようになりました。
  
  従来よりバイナリーログはサポートされていましたが、トランザクション中にテーブル
  をオープンすると、そのテーブルに対するbinlogマップが生成されず、正しくレプリケ
  ーションできない問題がありました。そのため、バイナリーログが有効な場合、トラン
  ザクション中のテーブルオープンはエラーになるようにしました。
  バイナリーログが無効な場合は、動作に変更はありません。

新機能
--------------------------------------------------------------------------------
* MULTILOCK_READ_COMMITEDのトランザクション中のseek step系の読み取りオペレーショ
  ンのbiasパラメータにROW_LOCK_Sを指定可能にしました。
  
  これにより、MULTILOCK_READ_COMMITEDでも、よりロック競合の少ない共有ロックを使用
  した細かなロック制御が可能です。

* queryBaseクラスを使ったテーブルクエリーで、limitで設定した値で検索を終了する設
  定を可能にしました。
  
  従来、limitの値は一回のオペレーションで取得するレコードの最大値を示し、クライア
  ント側でlimit以外の終了条件を満たすまで自動で繰り返し取得オペレーションが行われ
  ていました。事実上limitは受信データバッファを節約するためだけのものでした。
  
  新しく加えた、queryBase::stopAtLimit()関数にtrueをセットすることで、マッチした
  レコードがlimitに達するとそこで検索を終了できます。
  デフォルトのfalseの場合の動作は従来と同様です。

* table::find(eFindType type)で使用されるeFindTypeにfindContinueが追加されました。
  
  直前のfind系オペレーションが、フィルター条件のmaxRecordまたはrejectCount
  によって終了した場合、findContinueを指定すると、その続きから同じ条件で検索を続
  けることができます。また、直前のfind系オペレーションがそのような条件で終了した
  かどうかを調べるために、より詳しいステータスを示すtable::statReasonOfFind()と
  table::lastFindDirection()メソッドが追加されました。

* ActiveTableにreadMore()メソッドが追加されました。これは上記、findContinueの
  ActiveTable実装です。

* recordsetのグルーピング関数に、firstとlast関数が追加されました。これらは、各グ
  ループの最初または最後のレコードのフィールド値をそのまま返します。
  値は、数値と文字列の双方をサポートします。
  
  これを使用すると非正規化されたフィールドの値を用いてidなどに対する説明のフィー
  ルドの値をそのまま利用し、JOINを不要にしたりすることができます。

* フィルターとクエリーの比較演算子に、大文字/小文字を区別しない
  (case-insensitive)比較の指定ができるようになりました。
  
  文字列において大文字/小文字の区別なしに比較するには、従来の6種類の演算子
  の後ろにi(アイ)を付加します。
  
  ```
  Case-sensitive:   =,  >,  < ,  >=,  <=,  <>
  Case-insensitive: =i, >i, <i,  >=i, <=i, <>i
  ```
  
  テーブルのフィルターにおいて、比較対象フィールドが、使用するインデックスのフィ
  ールドの時は、大文字/小文字区別の指定をテーブル作成時のフィールド定義と同じに
  するとパフォーマンスが良くなります。異なるとインデックスによる読み取り範囲の最
  適化が行えずフルスキャンを行う必要が出てきます。

* ネットワーク接続の再接続コマンド、database::reconnect()が追加されました。不意
  にサーバープロセスが再起動された場合でも、このメソッドによって 再接続が可能に
  なりました。再接続は、開いていたテーブルの再オープン、カーソル位置の復元、レ
  コードロックの復元が行われます。ネットワーク接続の切断時に実行中だったトランザ
  クションは復元されません。再度実行する必要があります。
  
  現在のところ、再接続先の変更は行えません。将来的にはtdclcライブラリ内で接続先の
  変更がサポートされる予定です。

* fielddef::lenByCharnum()メソッドは従来、ft_mychar ft_myvarcharのみのサポートで
  したが、加えてft_wstring ft_wzstring ft_myvarbinary ft_mywvarbinaryもサポート
  されました。これらのフィールド型の場合も文字数で長さを指定できます。

* クライアントの設定ファイル(transactd.iniまたはtransactd.cnf)で、connectTimeout
  とnetTimeoutが追加されました。何も設定しない場合のデフォルト値はそれぞれ以下の
  通りです。
  
  ```
  connectTimeout = 20
  netTimeout = 180
  ```
  
  connectTimeoutは接続時のタイムアウト(秒)です。netTimeoutは、オペレーション実行
  時に、サーバーから応答が返るまでのタイムアウト(秒)です。

* tdclc内のTCP read writeの実装がboostからOS nativeに変更されました。これによりパ
  フォーマンスとタイムアウトの設定を可能にしました。

その他の修正と変更点
--------------------------------------------------------------------------------
* 固定長文字列型フィールドのフィルタリングが正しく行われない場合がある不具合を修
  正しました。

* PHPとRubyインタフェースにsortFields sortFieldクラスが欠落していた不具合を修正
  しました。

* Active XインターフェースにRecordset::UnionRecordset()メソッドが欠落していた不
  具合を修正しました。

* table::find系オペレーション時に不正なキー番号をセットすると、サーバーがクラッ
  シュする不具合を修正しました。

* テーブルがSQLコマンドLOCK TABLESでロックされた状態で、書き込みトランザクション
  が失敗したあと、再トライ時にunlock rowでエラーが発生する不具合を修正しました。

* ActiveXインターフェースのdatabaseオブジェクトの解放で、テーブルの解放順との順序
  関係によって、access violationが発生することがある不具合を修正しました。

* DNSが使用できないクライアントにおいて、接続先の指定にlocalhostが使用できない問
  題を修正しました。

* ft_float型のフィールドでi64()による読み取りが常にゼロを返す不具合を修正しました。

* ft_string型とft_wstring型のフィールドでfield::getFVbin()とtable::getFVbin()に
  よる値の読み取りができない不具合を修正しました。

* Windowsのパイプ接続時の共有メモリサイズのデフォルト値が、サーバーとクライアント
  で異なる不具合を修正しました。

* activeTableクエリーによるft_textとft_blobフィールドの読み取りで、フィールドの
  絞り込みが無い場合、不正なポインターを返す不具合を修正しました。

* Transactdプラグインのソースコードを MySQL 5.7.6対応にしました。cmakeは現在未対応
  です。ビルドにはcmakeの修正が必要です。
