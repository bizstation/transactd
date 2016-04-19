Release note
================================================================================
Version 3.3.0 2016/04/18
================================================================================
New Features
--------------------------------------------------------------------------------
* `records& connMgr::statusvars()` was added. This function can get 
  Transactd status variables.

* `connMgr::slaveStatus()` was made possible to get message which is larger than
  default buffer (67 bytes).

Modifications
--------------------------------------------------------------------------------
* Fixed the conversion failure of 64Bit integer on PHP.

* Fixed a server error that occurs when the host authentication.

* PHP in version 5.4 or less on windows is no longer supported. PHP 5.5 or later 
  are supported. Linux or OSX are supported PHP version 5.4 or later.


Version 3.2.1 2016/04/11
================================================================================
Modifications
--------------------------------------------------------------------------------
* Fix a bug that `connMgr::postDisconnectOne` works wrong.

* Fix a bug that `nstable::getDirURI` returns wrong URI under some conditions.


Version 3.2.0 2016/04/07
================================================================================
New Features
--------------------------------------------------------------------------------
* `connMgr` class was added. This provides following funcitons:
  1. Get connections and usage of databases and tables by Transactd clients.
  2. Force disconnection of Transactd clients.
  3. Get lists of databases, tables and views.
  4. Get status of Transactd server.
  5. Get status of replication slave(s).
  
  See reference of `connMgr` class for more details.

* Creation SQL statements of tables or views can be got by Transactd client.
  These are as same as statements which got by `SHOW CREATE TABLE` SQL. You can
  create same tables and views when you want to copy database.

* The transaction across some databases in same server is available now.
  Use the associate database objects which share connection and transaction.
  See reference of `database::createAssociate()` for more details.

Modifications
--------------------------------------------------------------------------------
* Return `cp932` before `sjis` when search `charsetindex` from `codePage`.

* Increase max table numbers which a `database` object can handle from 50 to 150.

* Fail to connect and return `STATUS_DB_YET_OPEN` to `stat()` if the database
  was opened when `nsdatabase::connect()`.

* Fix a hung-up bug when copy auto generated schema and release `database`.

* Fix a server hung-up bug when specify invalid key number.

* Fix a bug in counting length of `ft_myfixedbinary` field.

* Fix a bug that compare-function was not check field string in full length on
  `recordset:groupBy()`.

* Fix a bug that BLOB field will not be handled well on the table which contains
  both of NULL-able field and BLOB field.


Version 3.1.0 2016/03/03
================================================================================
New Features
--------------------------------------------------------------------------------
* Add option to get the binary log position at the start of CONSISTENT_READ mode
  snapshot.

* Opening database is no longer required to `database::drop`. Specify uri to drop.
  You can drop database that has broken schema table.

* Bulk inserting and multi-record reading are enabled at `database::copyTableData`
  and it increase in speed.

* Add data compression option to creating table process. If `tabledef::flags.bit3`
  is true, data compression is enabled. This feature uses `ROW_FORMAT=COMPRESSED`
  option on MySQL `CREATE TABLE`.

Modifications
--------------------------------------------------------------------------------
* Fix a bug that the number of result records will be less than the number which
  was specified with `query::limit` on reading.

* Fix a bug that can not create table on P.SQL.

* Fix a bug that can not be authenticated on MySQL 5.7 with native_password mode.

* Fix a bug that segfault occurs at re-opening table because invalid pointer was
  held as key number resolver.

* Fix a bug that the number of record reading will increase with inserting or
  updating on record statistics.

* Fix a bug that DDL operations are enabled in transaction.

* Fix a bug that encoding works wrong on the fields which are encoded each
  different character codes.

* Improve transaction and snapshot APIs to return more detailed status.

* Add `table` pointer parameter to `copyDataFn` callback function on `database`
  object.

* Fix the character code which will be returned from `dbdef::getSQLcreateTable`
  to `utf8`.

* Fix a bug that counting NULLable fields will be wrong on the methods like
  `table::find`.

* Fix the default `charsetIndex` of the field which was added with 
  `recordset::appendField` to same as the `charsetIndex` of the first field.


================================================================================
Version 3.0.0 2015/12/26
================================================================================
Upgrade Notes
--------------------------------------------------------------------------------
* Compatibility between server plugin and clients
  Upgrade from the Version 2.4 is easy.
  Server plugins and clients are compatible with 2.4. (except the new features 
  from version 3.0)
  However, the handling of nullable field is different.
  You can have the same handling as the previous version. Before opening the 
  database, please call the database::setCompatibleMode(CMP_MODE_OLD_NULL).

  Upgrade from the older versions than 2.4, please check the previous 
  release notes.

New Features
--------------------------------------------------------------------------------
* In the field of value, it was able to be a NULL setting and reading
  You can compatible setting, so that the problem does not occur in the 
  difference of NULL handling. Before opening the database, please call the 
  database::setCompatibleMode(CMP_MODE_OLD_NULL). It will be the same as the NULL 
  handling version 2.4.
  Please refer to bellow for more information.
  http://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html

* Supports the default value for the field
  Please refer to bellow for more information.
  http://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html

* Schema table-less access is available
  Please refer to bellow for more information.
  http://www.bizstation.jp/ja/transactd/client/sdk/doc/page_1_0_v3.html

* PHP Extension now supports PHP7

* Added a bitset class to simplify the field access of bit type.
  To get/set of field values, you can do remain bistset class.

* Supports MySQL DECIMAL type

* Supports MySQL TIME / DATETIME / TIMESTAMP type fully compatible
  Data structure is different depending on the version of the 
  MySQL / Mariadb.

* Implemented the offset of the MySQL DATE type 1900 in the client library.

* Supports MySQL5.7 Mariadb 10.1

Modifications
--------------------------------------------------------------------------------
* Server settings
  Added a timestamp_always variable. Please refer to bellow for more information.
  http://www.bizstation.jp/en/transactd/documents/admin_manual.html#mycnf

* Test
  Test for a new function of Version3 has been added.
  (test_tdclcpp_v3.cpp test_v3.js transactd_v3_Test.php transactd_v3_spec.rb)

* Refactoring of libraries of tdclcpp

* Added classes
  class bitset

* Added public method or member
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
 
* Added public constants
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

* Deleted public method
  ```
  ushort_td          dbdef::getRecordLen(short tableIndex) 
  double             field::getFVnumeric() const 
  double             field::getFVDecimal() const 
  void               field::setFVDecimal(double data) 
  void               field::setFVNumeric(double data) 
  ```

* Changed method parameters
  ```
  short             database::copyTableData(table* dest, table* src, bool turbo, short keyNum = -1, int maxSkip = -1) 
  void              table::clearBuffer(eNullReset resetType = defaultNull) 
  ```


================================================================================
Version 2.4.5 2015/10/29
================================================================================
Modifications 
--------------------------------------------------------------------------------
* Support MySQL 5.7.9 and MariaDB 10.1.8.

* Support MYSQL_TYPE_NEWDATE, MYSQL_TYPE_TIME2, MYSQL_TYPE_DATETIME2 and
  MYSQL_TYPE_TIMESTAMP2.


================================================================================
Version 2.4.4 2015/09/08
================================================================================
Modifications 
--------------------------------------------------------------------------------
* In Windows of pooledDbManager, Fixed a bug that may be deadlock in 
  tdclc_xxx.dll to at the end of the process.

* Added a statMsg method to nstable,nsdatabase and dbdef class. This method is 
  the same function as the already tdapErr method. Other than the difference of 
  the arguments and return values

* In PHP and Ruby interface, it was fixed a bug that tdapErr method does not work
  properly. In this two interfaces, tdapErr method has been changed to statMsg.
  Also, we added a statMsg method in C ++ and COM interfaces.

* We added a IErrorInfo the COM interface. In addition, we have set the default
  property to IRecordset,IRecord,IKeyDef,IFlags,IFieldDefs,ISortFields,
  IFieldNames interfaces.


================================================================================
Version 2.4.3 2015/08/31
================================================================================
Modifications (Client only)
--------------------------------------------------------------------------------
* Fix a bug that does not work correctly Join with Union record set.


================================================================================
Version 2.4.2 2015/08/31
================================================================================
New Features
--------------------------------------------------------------------------------
* Added a bit AND operator to queryBase and recordsetQuery.
  In addition to the operators of the past, you can use the '&' and '!&'.
  '&' operator performs the value and bit operation of the field. If the same as 
  the specified value matches.
  '!&' operator performs the value and bit operation of the field. If the not
  same as the specified value matches. 
 
  ex)flags & 8 and flags !& 16


Other Modifications
--------------------------------------------------------------------------------
* Changed method
  field::addAllFileds(tabledef* def) : Chnaged from protected to public.

* Added method parameters
  writableRecord::del and writableRecord::update,added option of "bool noSeek=false". 
  If true the noSeek, current record as an established, omit the read operation 
  in the delete or update operation.

* Added method 
  void queryStatements::move(int from, int to)

* It was fixed a bug that connection timeout does not work as configured on 
  windows.

* If there is no record to Join in HasManyJoin, Fixed a bug that may not be 
  processed correctly.

* In the Join, When the binding key is a string, Fixed a problem that may not
  be able to properly search. 



================================================================================
Version 2.4.0 2015/06/03
================================================================================
Upgrade Notes
--------------------------------------------------------------------------------
* Compatibility between server plugin and clients
  
  Upgrade from the Version 2.3 is easy.
  Server plugins and clients are compatible with 2.4.
  (However, except the new features from version 2.4)
  
  Upgrade from the older versions than 2.3, please check the previous 
  release notes.

New Features
--------------------------------------------------------------------------------
* Transactd studio
  
  The following information API has been added to the Transactd.
  * Connection in use
  * Database in use
  * Table in use
  
  Table information shows the number of records which were read, updated,
  inserted, deleted since the table has been opened.

* ActiveTable can obtain a recordset from bookmarks.
  
  You can set multiple bookmark using queryBase::addSeekBookmark.
  
  If you call table::recordCount() after setting query object to table object,
  you will get the number of records which match the conditions, and you also
  get bookmarks of them at the same time. You can access the bookmarks with
  table::bookmarks().

* The following methods in C ++ API were modified to allow simultaneous access
  from multiple threads.
  * table::insertBookmarks
  * table::moveBookmarks
  * table::bookmarksCount
  * table::bookmarks

Other Modifications
--------------------------------------------------------------------------------
* Fix a bug that null key detection is sometimes wrong in P.SQL compatible
  null key access.

* Fix a bug that automatic schema generation sometimes does not work well.

* Improved lock control to get the server statistics. Improve the simultaneous
  effectiveness at monitoring.

* nsdatabase::isReconnected() method was added.
  It indicates whether reconnected to the server.

* Support reconnection in the case of using multiple databases in single
  connection.

* Fix a bug that useless internal null fields will be added when create table
  if there is a null key.

* Fix invalid pointer operations that occurs when an invalid prepared query
  handle are received.

* Fix a bug that can not coexist with the lock by SQL access sometimes.

* Added methods
  * short dbdef::validateTableDef(short TableIndex)
  * ushort_td nstable::bookmarkLen() const
  * bookmark_td tabale::bookmarks(unsigned int index) const
  * recordCountFn Call back function
  * table::setOnRecordCount
  * table::onRecordCount
  * bool queryBase::isSeekByBookmarks() const
  * void queryBase::addSeekBookmark(bookmark_td& bm, ushort_td len, bool reset=false)
  * bool writableRecord::read(bookmark_td& bm)

* Changed method names
  * table::setBookMarks     --> table::insertBookmarks
  * table::moveBookmarksId  --> table::moveBookmarks
  * table::bookMarksCount   --> table::bookmarksCount

* Added method parameters
  * void database::close(bool withDropDefaultSchema = false)
  * activeTable::activeTable(database* db, short tableIndex, 
                                  short mode = TD_OPEN_NORMAL)
  * static activeTable* create(database* db, short tableIndex,
                                  short mode = TD_OPEN_NORMAL);

* Change bookmark type from unsigned int to bookmark_td.
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

* Changed the type of function arguments to reference of bookmark_td from
  bookmark_td.

* openTable method in C++ convenience API supports all arguments of
  original openTable method.

* The following functions are added to C++ convenience API.
  * void deleteTable(dbdef* def, short id)
  * void renumberTable(dbdef* def, short id, short newid)
  * void deleteField(dbdef* def, short tableid, short fieldNum)
  * void deleteKey(dbdef* def, short tableid, short keynum)
  * void validateTableDef(dbdef* def, short tableid)

* The binaries which compiled with Embarcadero C++ Builder are no longer
  included in Transactd Client with SDK for Windows. If you want to use them,
  they are still available by building from source code.
  
  http://www.bizstation.jp/ja/transactd/documents/BUILD_WIN.html



================================================================================
Version 2.3.0 2015-03-20
================================================================================

Upgrade Notes
--------------------------------------------------------------------------------
* Compatibility between server plugin and clients
  
  You have to upgrade both of the server plugin and client libraries.
  
  The protocol that is used communicating between server and client were changed
  for database::reconnect() method. This version of server plugin and clients
  have no compatibility with the past versions of them.
  
  If you access to 2.2 or older version of server plugin with this version of
  clients, error code SERVER_CLIENT_NOT_COMPATIBLE (3003) will be returned.

* Some methods were moved.
  
  table::usePadChar() and table:: trimPadChar() were moved to fielddef structure.
  
  The setter methods of them were put together to setPadCharSettings(bool set, 
  bool trim). By this change, these values can be saved to the schema.
  
  This affects to the field types ft_string, ft_wstring, ft_mychar and ft_mywchar.
  Other field types are not affected.
  
  The default value of table::usePadChar() and trimPadChar() are true. If you did
  not change this value, there is no changes on program behavior.
  If you changed the values, you have to fix program code.
  
  For example, the code `tb->setUsePadChar(false);` has to be modified like this:
  ```
  for (int i = 0 ; i < tb->tableDef()->fieldCount ; ++i)
  {
    fielddef* fd = const_cast<fielddef*>(&tb->tableDef()->fieldDefs[i]);
    fd->setPadCharSettings(false/*set*/, true/*trim*/);
  }
  ```
  
  Note: Changes in this code are volatile, and not saved on schema. Add updating
  schema code if you need.

* openTable in transaction
  
  The error code STATUS_ALREADY_INTRANSACTION is added. If openTable is called
  in transaction, and binary log is enable, the server returns this error.
  
  The binary log was supported until now, but there was the problem that if
  openTable is called in transaction the binlog map will not be generated and
  replication does not go well. So that, we modified the server plugin to return
  error if openTable is called in transaction with enable binary log. There is
  no changes if binary log is disabled.

New Features
--------------------------------------------------------------------------------
* Add bias parameter ROW_LOCK_S to seek and step read operations in
  MULTILOCK_READ_COMMITED transaction.
  
  By this, you can control lock in more detail with using shared lock.

* Add limit option to finish searching to table query with queryBase.
  
  In the past, the value of limit option meant that max number of records which
  will be got with one operation. The clients automatically called read operation
  many times until the other finish conditions are met. In effect, limit was the
  option that save receive buffer.
  
  If you set true to queryBase::stopAtLimit(), the limit option behaves as finish
  condition. The default value is false, and it is same as the past behavior.

* Add findContinue to enum eFindType for table::find(eFindType type).
  
  If the last find operation has been finished because of the filter conditions
  maxRecord or rejectCount, it is able to continue searching from next record
  with findContinue.
  To check the cause of finishing the last operation, table::statReasonOfFind()
  and table::lastFindDirection() which return more detail status were added.

* Add ActiveTable::readMore(). It is same as above findContinue for ActiveTable.

* Add first and last method to recordset grouping functions. These methods
  return first or last record in each group. Strings and numbers are supported.
  
  These methods are useful when use non-normalized field values to avoid JOIN
  for some reason.

* Add case-insensitive comparison operator to filter and query.
  
  To compare strings without case sensitivity, add "i" after the normal
  comparison operators.
  
  ```
  Case-sensitive:   =,  >,  < ,  >=,  <=,  <>
  Case-insensitive: =i, >i, <i,  >=i, <=i, <>i
  ```
  
  When the comparison field is same as the index field which is using, setting
  case sensitivity same as field definition makes performance better. If case
  sensitivities are different between them, optimization with the indexes is
  disable and full scan will be needed.

* Add database::reconnect() which is used to reconnect to database. You can
  reconnect to database with this method even if server process has been
  restarted unexpectedly.
  
  The reconnection contains reopening tables, recovering cursor positions and
  record locks. Transactions which have not been commited yet will not be
  recovered. You have to run it again.
  
  There is no way to change the reconnect address at present, but tdclc will
  support it in the future.

* Add field types ft_wstring, ft_wzstring, ft_myvarbinary and ft_mywvarbinary
  to fielddef::lenByCharnum(). You can specify field length with number of
  characters.

* Add connectTimeout and netTimeout to client setting file (transactd.ini or
  transactd.cnf). The default values are following:
  
  ```
  connectTimeout = 20
  netTimeout = 180
  ```
  
  connectTimeout means timeout seconds for connection. netTimeout means timeout
  seconds for waiting response from server in a operation.

* The implementation of TCP reading and writing methods in tdclc were changed to
  OS native from boost library. It makes performance better and enable timeout
  settings.

Other Modifications
--------------------------------------------------------------------------------
* Fix a bug that filter works wrong for fixed-length string fields.

* Fix a bug that sortFields and sortField are not in PHP and Ruby interface.

* Fix a bug that Recordset::UnionRecordset() is not in ActiveX interface.

* Fix a bug that the server will crash when set invalid key number to table::find
  operations.

* Fix a bug that when the table is locked by LOCK TABLES SQL command, after a
  transaction with writing failed, an unlock row error will occur in retrying.

* Fix a access violation that occurs in releasing database object in ActiveX
  interface depending on table releasing order.

* Fix a problem that "localhost" alias cannot be used without DNS.

* Fix a bug that i64() method always returns 0 on ft_float field.

* Fix a bug that field::getFVbin() and table::getFVbin() methods can not read
  values on ft_string and ft_wstring fields.

* Fix a bug that the default values of size of shared memory on Windows pipe
  connection are different between server and clients.

* Fix a bug that a invalid pointer will be returned when activeTable and query
  reads ft_text or ft_blob fields without "select fields" filter.

* The source codes of Transactd plugin supports MySQL 5.7.6.
  (Build-scripts do not support it yet. Have to fix cmake scripts to build.)
