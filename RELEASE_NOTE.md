Release note

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
