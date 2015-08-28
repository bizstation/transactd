﻿リリースノート

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