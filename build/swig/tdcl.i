/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software 
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
   02111-1307, USA.
=================================================================*/
/* ===============================================
      Settings specific to each languages.
=============================================== */
#if defined(SWIGRUBY)
  %include "ruby/ruby.swg"
#elif defined(SWIGPHP)
  %include "php/php.swg"
#endif

%module transactd

%include typemaps.i
%include std_string.i
%include wchar.i
%include exception.i

// Suppress warnings
#pragma SWIG nowarn=SWIGWARN_PARSE_USING_UNDEF
#pragma SWIG nowarn=SWIGWARN_PARSE_UNNAMED_NESTED_CLASS
%warnfilter(SWIGWARN_LANG_OVERLOAD_SHADOW) btrstoa;


/* ===============================================
      type defines
=============================================== */
%apply unsigned long long { unsigned __int64 }
%apply long long { __int64 }


/* ===============================================
  validatable pointer
=============================================== */
%{
#include <build/swig/validatablepointer.h>
validatablePointerList g_vPtrList;
%}


/* ===============================================
      HEADERS (for cpp compilation)
=============================================== */
%{
#include <bzs/env/crosscompile.h>
#include <bzs/env/compiler.h>
#include <bzs/db/protocol/tdap/client/export.h>
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/blobStructs.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/transactd/connectionRecord.h>
%}
#ifndef SWIGWIN
%{
#include <linux/linuxTypes.h>
#include <linux/charsetConvert.h>
#include <bzs/env/mbcswchrLinux.h>
%}
#endif
%{
#include <bzs/env/tstring.h>
#include <bzs/rtl/benchmark.h>
#include <bzs/rtl/datetime.h>
#include <bzs/rtl/stringBuffers.h>
#include <bzs/rtl/strtrim.h>
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/protocol/tdap/myDateTime.cpp>
#include <bzs/db/protocol/tdap/client/sharedData.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/db/protocol/tdap/client/nsTable.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>
#include <bzs/db/protocol/tdap/client/bulkInsert.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/nsDatabase.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/protocol/tdap/client/field.h>
#include <bzs/db/protocol/tdap/client/fields.h>
#include <bzs/db/protocol/tdap/client/memRecord.h>
#include <bzs/db/protocol/tdap/client/recordset.h>
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <bzs/db/protocol/tdap/client/groupQuery.h>
#include <bzs/db/protocol/tdap/client/pooledDatabaseManager.h>

using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;
%}


/* ===============================================
      Ruby call_without_gvl support
=============================================== */
#if defined(SWIGRUBY)
  %include "ruby/without_gvl.swg"
#endif


/* ===============================================
      ignore, rename, new/delobject, extend
=============================================== */
// ignore definitions
%ignore LIB_EXTENTION;
%ignore LIB_PREFIX;
%ignore LINUX;
%ignore MBC_CHARSETNAME;
%ignore MB_PRECOMPOSED;
%ignore SHARED_LIB_EXTENTION;
%ignore UTF8_CHARSETNAME;
%ignore WC_COMPOSITECHECK;

// common %newobject and %delobject
%newobject *::clone;
%newobject *::create;
%newobject *::createObject;
%newobject *::openTable;
%newobject *::prepare;
%newobject *::setQuery;
%delobject *::release;

// * bzs/env/mbcswchrLinux.h *
%ignore bzs::env::initCvtProcess;
%ignore bzs::env::deinitCvtProcess;
%ignore bzs::env::getCvt;
%ignore bzs::env::u8mbcvt;
%ignore bzs::env::mbu8cvt;
%ignore bzs::env::mbcscvt;
%ignore bzs::env::wchrcvt;
%ignore bzs::env::u8wccvt;
%ignore bzs::env::wcu8cvt;
%ignore bzs::env::WideCharToMultiByte;
%ignore bzs::env::MultiByteToWideChar;
%ignore bzs::env::u8tombc;
%ignore bzs::env::mbctou8;

// * bzs/db/protocol/tdap/client/activeTable.h *
%ignore bzs::db::protocol::tdap::client::activeTable::create;
%ignore bzs::db::protocol::tdap::client::activeTable::releaseTable;
%ignore bzs::db::protocol::tdap::client::preparedQuery::getFilter;
  // activeTable::read and activeTable::prepare returns new object
%newobject bzs::db::protocol::tdap::client::activeTable::prepare;
%newobject bzs::db::protocol::tdap::client::activeTable::read;
%extend bzs::db::protocol::tdap::client::activeTable {
  recordset* read(queryBase& q) {
    recordset* rs = recordset::create();
    self->read(*rs, q);
    return rs;
  }

  //read(preparedQuery* q) It can give place holder values(0-8)
  recordset* read(preparedQuery* q, int v) {
    recordset* rs = recordset::create();
    self->read(*rs, q->getFilter(), v);
    return rs;
  }
  preparedQuery* prepare(queryBase& q, bool serverPrepare=false) {
    preparedQuery* p = new preparedQuery(self->prepare(q, serverPrepare));
    return p;
  }
  // join and outerJoin with preparedQuery.
#if defined(SWIGPHP)
  // For PHP fixed number of paramatars.
  activeTable& join(recordset& rs, preparedQuery* q, const _TCHAR* name1,
                      const _TCHAR* name2, const _TCHAR* name3,
                      const _TCHAR* name4, const _TCHAR* name5,
                      const _TCHAR* name6, const _TCHAR* name7,
                      const _TCHAR* name8) {
    self->join(rs, q->getFilter(), name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& outerJoin(recordset& rs, preparedQuery* q, const _TCHAR* name1,
                      const _TCHAR* name2, const _TCHAR* name3,
                      const _TCHAR* name4, const _TCHAR* name5,
                      const _TCHAR* name6, const _TCHAR* name7,
                      const _TCHAR* name8) {
    self->outerJoin(rs, q->getFilter(), name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& join(recordset& rs, queryBase& q, const _TCHAR* name1,
                      const _TCHAR* name2, const _TCHAR* name3,
                      const _TCHAR* name4, const _TCHAR* name5,
                      const _TCHAR* name6, const _TCHAR* name7,
                      const _TCHAR* name8) {
    self->join(rs, q, name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& outerJoin(recordset& rs, queryBase& q, const _TCHAR* name1,
                      const _TCHAR* name2, const _TCHAR* name3,
                      const _TCHAR* name4, const _TCHAR* name5,
                      const _TCHAR* name6, const _TCHAR* name7,
                      const _TCHAR* name8) {
    self->outerJoin(rs, q, name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
#else
  activeTable& join(recordset& rs, preparedQuery* q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL) {
    self->join(rs, q->getFilter(), name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& outerJoin(recordset& rs, preparedQuery* q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL) {
    self->outerJoin(rs, q->getFilter(), name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& join(recordset& rs, queryBase& q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL) {
    self->join(rs, q, name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
  activeTable& outerJoin(recordset& rs, queryBase& q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL) {
    self->outerJoin(rs, q, name1, name2, name3, name4, name5, name6,
               name7, name8);
    return *self;
  }
#endif
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::activeTable::read;
%ignore bzs::db::protocol::tdap::client::activeTable::prepare;
%ignore bzs::db::protocol::tdap::client::activeTable::join;
%ignore bzs::db::protocol::tdap::client::activeTable::outerJoin;
  // create and release methods for activeTable class
%extend bzs::db::protocol::tdap::client::activeTable {
  activeTable(idatabaseManager* mgr, const _TCHAR* tableName) {
    bzs::db::protocol::tdap::client::activeTable* p =
      bzs::db::protocol::tdap::client::activeTable::create(mgr, tableName);
    g_vPtrList.add(p->table().get());
    return p;
  }
  activeTable(database* db, const _TCHAR* tableName) {
    bzs::db::protocol::tdap::client::activeTable* p =
      bzs::db::protocol::tdap::client::activeTable::create(db, tableName);
    g_vPtrList.add(p->table().get());
    return p;
  }
  ~activeTable() {
    if (g_vPtrList.remove(self->table().get()))
    {
      if (nsdatabase::testTablePtr(self->table().get()))
        self->table()->nsdb()->setTestPtrIgnore(true);
    }
    self->release();
  }
  void release() {
    self->releaseTable();
  }
  table* table() {
    return self->table().get();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::activeTable::activeTable;
%ignore bzs::db::protocol::tdap::client::activeTable::~activeTable;
%ignore bzs::db::protocol::tdap::client::activeTable::table;

// * bzs/db/protocol/tdap/btrDate.h *
%ignore bzs::db::protocol::tdap::bdate;
%ignore bzs::db::protocol::tdap::c_str;

// * bzs/db/protocol/tdap/client/client.h *
%ignore bzs::db::protocol::tdap::client::create;

// * bzs/db/protocol/tdap/client/connectionPool.h *
%ignore bzs::db::protocol::tdap::client::busyWaitArguments;
%ignore bzs::db::protocol::tdap::client::busyWait;
%ignore bzs::db::protocol::tdap::client::scopedLock;
%ignore bzs::db::protocol::tdap::client::connectionPool;
%ignore bzs::db::protocol::tdap::client::stdDbCconnectionPool;
%ignore bzs::db::protocol::tdap::client::stdDbmCconnectionPool;
%ignore bzs::db::protocol::tdap::client::stdCconnectionPool;
%ignore bzs::db::protocol::tdap::client::dllUnloadCallbackFunc;
%ignore bzs::db::protocol::tdap::client::releaseConnection;
%ignore bzs::db::protocol::tdap::client::cpool;

// * bzs/db/protocol/tdap/client/database.h *
%ignore bzs::db::protocol::tdap::client::database::operator=;
%ignore bzs::db::protocol::tdap::client::database::defaultAutoIncSpace;
  // NOTE: ignore * STATIC * create only
%ignore bzs::db::protocol::tdap::client::database::create();
  // create and release methods for database class
%extend bzs::db::protocol::tdap::client::database {
  database() {
    bzs::db::protocol::tdap::client::nsdatabase::setExecCodePage(CP_UTF8);
    return bzs::db::protocol::tdap::client::database::create();
  }
  ~database() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::database::database;
%ignore bzs::db::protocol::tdap::client::database::~database;
  // overwrite openTable
%extend bzs::db::protocol::tdap::client::database {
  bzs::db::protocol::tdap::client::table* openTable(const _TCHAR* tableName,
    short mode = 0, bool autoCreate = true, const _TCHAR* ownerName = NULL, const _TCHAR* uri = NULL)
  {
    bzs::db::protocol::tdap::client::table* tb =
      self->openTable(tableName, mode, autoCreate, ownerName, uri);
    g_vPtrList.add(tb);
    return tb;
  }
  bzs::db::protocol::tdap::client::table* openTable(short fileNum,
    short mode = TD_OPEN_NORMAL, bool autoCreate = true, const _TCHAR* ownerName = NULL, const _TCHAR* uri = NULL)
  {
    bzs::db::protocol::tdap::client::table* tb =
      self->openTable(fileNum, mode, autoCreate, ownerName, uri);
    g_vPtrList.add(tb);
    return tb;
  }
};
  // ignore original method
%ignore bzs::db::protocol::tdap::client::database::openTable;

// * bzs/db/protocol/tdap/client/dbDef.h *
%ignore bzs::db::protocol::tdap::client::dbdef::allocRelateData;
%ignore bzs::db::protocol::tdap::client::dbdef::cacheFieldPos;
%ignore bzs::db::protocol::tdap::client::dbdef::compAsBackup;
%ignore bzs::db::protocol::tdap::client::dbdef::fieldNumByViewNum;
%ignore bzs::db::protocol::tdap::client::dbdef::getFieldPosition;
%ignore bzs::db::protocol::tdap::client::dbdef::getFileSpec;
%ignore bzs::db::protocol::tdap::client::dbdef::relateData;
%ignore bzs::db::protocol::tdap::client::dbdef::popBackup;
%ignore bzs::db::protocol::tdap::client::dbdef::pushBackup;
%ignore bzs::db::protocol::tdap::client::dbdef::setStat;

// * bzs/db/protocol/tdap/client/field.h *
%ignore bzs::db::protocol::tdap::client::compBlob;
%ignore bzs::db::protocol::tdap::client::dummyFd;
%ignore bzs::db::protocol::tdap::client::fieldValue;
%ignore bzs::db::protocol::tdap::client::fieldShare;
%ignore bzs::db::protocol::tdap::client::field::operator=(const field&);
%ignore bzs::db::protocol::tdap::client::field::operator!=;
%ignore bzs::db::protocol::tdap::client::field::operator==;
%ignore bzs::db::protocol::tdap::client::field::operator=(const std::_tstring&);
%ignore bzs::db::protocol::tdap::client::field::initialize;
%ignore bzs::db::protocol::tdap::client::field::isCompPartAndMakeValue;
%ignore bzs::db::protocol::tdap::client::field::ptr;
%ignore bzs::db::protocol::tdap::client::fielddefs::create;
%ignore bzs::db::protocol::tdap::client::getFieldType;
  // create and release methods for fielddefs class
%extend bzs::db::protocol::tdap::client::fielddefs {
  fielddefs() {
    return bzs::db::protocol::tdap::client::fielddefs::create();
  }
  ~fielddefs() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::fielddefs::fielddefs;
%ignore bzs::db::protocol::tdap::client::fielddefs::~fielddefs;

// * bzs/db/protocol/tdap/client/fields.h *
%ignore bzs::db::protocol::tdap::client::MEM_ALLOC_TYPE_NONE;
%ignore bzs::db::protocol::tdap::client::MEM_ALLOC_TYPE_ONE;
%ignore bzs::db::protocol::tdap::client::MEM_ALLOC_TYPE_ARRAY;
%ignore bzs::db::protocol::tdap::client::fields;
%ignore bzs::db::protocol::tdap::client::fieldsBase::fd;
%ignore bzs::db::protocol::tdap::client::fieldsBase::getFieldNoCheck;
%ignore bzs::db::protocol::tdap::client::fieldsBase::setInvalidRecord;
  // add methods
%extend bzs::db::protocol::tdap::client::fieldsBase {
  void getFieldByIndexRef(short index, field& return_field) const {
    return_field = self->operator[](index);
  }
  void getFieldByNameRef(const _TCHAR* name, field& return_field) const {
    return_field = self->operator[](name);
  }
};
%rename(Record) bzs::db::protocol::tdap::client::fieldsBase;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%ignore bzs::db::protocol::tdap::client::avg::create;
%ignore bzs::db::protocol::tdap::client::count::create;
%ignore bzs::db::protocol::tdap::client::fieldNames::operator=;
%ignore bzs::db::protocol::tdap::client::fieldNames::create;
%ignore bzs::db::protocol::tdap::client::groupFuncBase::operator=;
%ignore bzs::db::protocol::tdap::client::groupFuncBase::operator();
%ignore bzs::db::protocol::tdap::client::groupQuery::operator=;
%ignore bzs::db::protocol::tdap::client::groupQuery::create;
%ignore bzs::db::protocol::tdap::client::max::create;
%ignore bzs::db::protocol::tdap::client::min::create;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::operator=;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::create;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::internalQuery;
%ignore bzs::db::protocol::tdap::client::sortField;
%ignore bzs::db::protocol::tdap::client::sortFields;
%ignore bzs::db::protocol::tdap::client::sortFields::operator[];
%ignore bzs::db::protocol::tdap::client::sum::create;
  // create and release methods for fieldNames class
%extend bzs::db::protocol::tdap::client::fieldNames {
  fieldNames() {
    return bzs::db::protocol::tdap::client::fieldNames::create();
  }
  ~fieldNames() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::fieldNames::fieldNames;
%ignore bzs::db::protocol::tdap::client::fieldNames::~fieldNames;
  // create and release methods for groupQuery class
%extend bzs::db::protocol::tdap::client::groupQuery {
  groupQuery() {
    return bzs::db::protocol::tdap::client::groupQuery::create();
  }
  ~groupQuery() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::groupQuery::groupQuery;
%ignore bzs::db::protocol::tdap::client::groupQuery::~groupQuery;
  // create and release methods for recordsetQuery class
%extend bzs::db::protocol::tdap::client::recordsetQuery {
  recordsetQuery() {
    return bzs::db::protocol::tdap::client::recordsetQuery::create();
  }
  ~recordsetQuery() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::recordsetQuery::recordsetQuery;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::~recordsetQuery;
  // create and release methods for avg class
%extend bzs::db::protocol::tdap::client::avg {
  avg(const fieldNames& targetNames, const _TCHAR* resultName = NULL) {
    return bzs::db::protocol::tdap::client::avg::create(targetNames, resultName);
  }
  ~avg() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::avg::avg;
%ignore bzs::db::protocol::tdap::client::avg::~avg;
  // create and release methods for count class
%extend bzs::db::protocol::tdap::client::count {
  count(const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::count::create(resultName);
  }
  ~count() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::count::count;
%ignore bzs::db::protocol::tdap::client::count::~count;
  // create and release methods for max class
%extend bzs::db::protocol::tdap::client::max {
  max(const fieldNames& targetNames, const _TCHAR* resultName = NULL) {
    return bzs::db::protocol::tdap::client::max::create(targetNames, resultName);
  }
  ~max() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::max::max;
%ignore bzs::db::protocol::tdap::client::max::~max;
  // create and release methods for min class
%extend bzs::db::protocol::tdap::client::min {
  min(const fieldNames& targetNames, const _TCHAR* resultName = NULL) {
    return bzs::db::protocol::tdap::client::min::create(targetNames, resultName);
  }
  ~min() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::min::min;
%ignore bzs::db::protocol::tdap::client::min::~min;
  // create and release methods for sum class
%extend bzs::db::protocol::tdap::client::sum {
  sum(const fieldNames& targetNames, const _TCHAR* resultName = NULL) {
    return bzs::db::protocol::tdap::client::sum::create(targetNames, resultName);
  }
  ~sum() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::sum::sum;
%ignore bzs::db::protocol::tdap::client::sum::~sum;

// * bzs/db/protocol/tdap/client/memRecord.h *
%ignore bzs::db::protocol::tdap::client::autoMemory;
%ignore bzs::db::protocol::tdap::client::autoMemory::operator=;
%ignore bzs::db::protocol::tdap::client::JOINLIMIT_PER_RECORD;
%ignore bzs::db::protocol::tdap::client::memoryRecord::clear;
%ignore bzs::db::protocol::tdap::client::memoryRecord::create;
%ignore bzs::db::protocol::tdap::client::memoryRecord::setRecordData;
%rename(createRecord) bzs::db::protocol::tdap::client::memoryRecord::create(fielddefs&);
  // create and release methods for memoryRecord class
%extend bzs::db::protocol::tdap::client::memoryRecord {
  memoryRecord(fielddefs& fds) {
    return bzs::db::protocol::tdap::client::memoryRecord::create(fds);
  }
  ~memoryRecord() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::memoryRecord::memoryRecord;
%ignore bzs::db::protocol::tdap::client::memoryRecord::~memoryRecord;

// * bzs/db/protocol/tdap/client/nsDatabase.h *
%ignore bzs::db::protocol::tdap::client::nsdatabase::btrvFunc;
%ignore bzs::db::protocol::tdap::client::nsdatabase::operator=;
%ignore bzs::db::protocol::tdap::client::nsdatabase::createTable;
%ignore bzs::db::protocol::tdap::client::nsdatabase::getBtrVersion(btrVersions*, uchar_td*);
%ignore bzs::db::protocol::tdap::client::nsdatabase::getDllUnloadCallbackFunc;
%ignore bzs::db::protocol::tdap::client::nsdatabase::isTestPtrIgnore;
%ignore bzs::db::protocol::tdap::client::nsdatabase::localSharing;
%ignore bzs::db::protocol::tdap::client::nsdatabase::maxtables;
%ignore bzs::db::protocol::tdap::client::nsdatabase::setTestPtrIgnore;
%ignore bzs::db::protocol::tdap::client::nsdatabase::testTablePtr;

// * bzs/db/protocol/tdap/client/nsTable.h *
%ignore bzs::db::protocol::tdap::client::nstable::buflen;
%ignore bzs::db::protocol::tdap::client::nstable::data;
%ignore bzs::db::protocol::tdap::client::nstable::setBuflen;
%ignore bzs::db::protocol::tdap::client::nstable::setData;
%ignore bzs::db::protocol::tdap::client::nstable::setStat;
%ignore bzs::db::protocol::tdap::client::nstable::tdap;
%ignore bzs::db::protocol::tdap::client::nstable::test;
%ignore bzs::db::protocol::tdap::client::nstable::throwError;
%rename(tdapLastErr) bzs::db::protocol::tdap::client::nstable::tdapErr(HWND, _TCHAR*);

// * bzs/db/protocol/tdap/client/pooledDatabaseManager.h *
%ignore bzs::db::protocol::tdap::client::xaTransaction;
%extend bzs::db::protocol::tdap::client::pooledDbManager {
  table* table(const _TCHAR* name) {
    return self->table(name).get();
  }
}
%ignore bzs::db::protocol::tdap::client::pooledDbManager::table;

// * bzs/db/protocol/tdap/client/recordset.h *
%ignore bzs::db::protocol::tdap::client::recordset::operator=;
%ignore bzs::db::protocol::tdap::client::recordset::create;
%rename(Recordset) bzs::db::protocol::tdap::client::recordset;
%rename(unionRecordset) bzs::db::protocol::tdap::client::recordset::operator+=;
  // add method
%extend bzs::db::protocol::tdap::client::recordset {
  void getRow(size_t index, bzs::db::protocol::tdap::client::fieldsBase** return_record){
    *return_record = &(self->operator[](index));
  }
};
  // create and release methods for recordset class
%extend bzs::db::protocol::tdap::client::recordset {
  recordset() {
    return bzs::db::protocol::tdap::client::recordset::create();
  }
  ~recordset() {
    self->release();
  }
};
%ignore bzs::db::protocol::tdap::client::recordset::recordset;
%ignore bzs::db::protocol::tdap::client::recordset::~recordset;

// * bzs/db/protocol/tdap/client/table.h *
%ignore bzs::db::protocol::tdap::client::keyValuePtr;
%ignore bzs::db::protocol::tdap::client::makeSupplyValues;
%ignore bzs::db::protocol::tdap::client::mra_nojoin;
%ignore bzs::db::protocol::tdap::client::mra_first;
%ignore bzs::db::protocol::tdap::client::mra_nextrows;
%ignore bzs::db::protocol::tdap::client::mra_innerjoin;
%ignore bzs::db::protocol::tdap::client::mra_outerjoin;
%ignore bzs::db::protocol::tdap::client::mra_current_block;
%ignore bzs::db::protocol::tdap::client::multiRecordAlocator;
%ignore bzs::db::protocol::tdap::client::query::create;
%ignore bzs::db::protocol::tdap::client::queryBase::reset;
%ignore bzs::db::protocol::tdap::client::queryBase::operator=;
%ignore bzs::db::protocol::tdap::client::queryBase::addField;
%ignore bzs::db::protocol::tdap::client::queryBase::addLogic;
%ignore bzs::db::protocol::tdap::client::queryBase::addSeekKeyValuePtr;
%ignore bzs::db::protocol::tdap::client::queryBase::create;
%ignore bzs::db::protocol::tdap::client::queryBase::joinKeySize;
%ignore bzs::db::protocol::tdap::client::queryBase::queryBase;
%ignore bzs::db::protocol::tdap::client::queryBase::~queryBase;
%ignore bzs::db::protocol::tdap::client::queryBase::reserveSeekKeyValuePtrSize;
%ignore bzs::db::protocol::tdap::client::supplyInValues;
%ignore bzs::db::protocol::tdap::client::supplyValue;
%ignore bzs::db::protocol::tdap::client::supplyValues;
%ignore bzs::db::protocol::tdap::client::table::buflen;
%ignore bzs::db::protocol::tdap::client::table::fieldPtr;
%ignore bzs::db::protocol::tdap::client::table::getCurProcFieldCount;
%ignore bzs::db::protocol::tdap::client::table::getCurProcFieldIndex;
%ignore bzs::db::protocol::tdap::client::table::mra;
%ignore bzs::db::protocol::tdap::client::table::tdap;
%ignore bzs::db::protocol::tdap::client::table::setMra;
%ignore bzs::db::protocol::tdap::client::table::setFVA;
%ignore bzs::db::protocol::tdap::client::table::getFVAstr;

  // create and release methods for query class
%extend bzs::db::protocol::tdap::client::query {
  query() {
    return bzs::db::protocol::tdap::client::query::create();
  }
  ~query() {
    self->release();
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::query::query;
%ignore bzs::db::protocol::tdap::client::query::~query;
  // ignore keyValueDescription and re-define with expand.
%ignore bzs::db::protocol::tdap::client::table::keyValueDescription(_TCHAR* buf, int bufsize);
%typemap(in,numinputs=0) (_TCHAR* keyValueDescription_buf) (_TCHAR tmpbuf[1024 * 8]) {
  $1 = tmpbuf;
}
%extend bzs::db::protocol::tdap::client::table {
  const _TCHAR* keyValueDescription(_TCHAR* keyValueDescription_buf) {
    self->keyValueDescription(keyValueDescription_buf, 1024 * 8);
    return keyValueDescription_buf;
  }
};
  // table::prepare and table::setQuery returns new object
%newobject bzs::db::protocol::tdap::client::table::prepare;
%newobject bzs::db::protocol::tdap::client::table::setQuery;
%extend bzs::db::protocol::tdap::client::table {
  preparedQuery* prepare(const queryBase* q, bool serverPrepare=false) {
    preparedQuery* p = new preparedQuery(self->setQuery(q, serverPrepare));
    return p;
  }
  preparedQuery* setQuery(const queryBase* q, bool serverPrepare=false) {
    preparedQuery* p = new preparedQuery(self->setQuery(q, serverPrepare));
    return p;
  }
  void setPrepare(preparedQuery* q) {
    self->setPrepare(q->getFilter());
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::table::prepare;
%ignore bzs::db::protocol::tdap::client::table::setQuery;
%ignore bzs::db::protocol::tdap::client::table::setPrepare;
  // create and release methods for table class
%extend bzs::db::protocol::tdap::client::table {
  void release() {
    if (bzs::db::protocol::tdap::client::nsdatabase::testTablePtr(self))
      self->release();
  }
  ~table() {
    if (!g_vPtrList.remove(self))
    {
      if (bzs::db::protocol::tdap::client::nsdatabase::testTablePtr(self))
        self->release();
    }
  }
};
  // ignore original methods
%ignore bzs::db::protocol::tdap::client::table::table;
%ignore bzs::db::protocol::tdap::client::table::~table;

// * bzs/db/protocol/tdap/client/trdboostapi.h *
%ignore bzs::db::protocol::tdap::client::autoBulkinsert;
%ignore bzs::db::protocol::tdap::client::createDatabaseObject;
%ignore bzs::db::protocol::tdap::client::eFindCurrntType;
%ignore bzs::db::protocol::tdap::client::eIndexOpType;
%ignore bzs::db::protocol::tdap::client::eStepOpType;
%ignore bzs::db::protocol::tdap::client::filterdIterator;
%ignore bzs::db::protocol::tdap::client::qlogic;
%ignore bzs::db::protocol::tdap::client::snapshot;
%ignore bzs::db::protocol::tdap::client::tableIterator;
%ignore bzs::db::protocol::tdap::client::transaction;
%ignore bzs::db::protocol::tdap::client::connect;
%ignore bzs::db::protocol::tdap::client::connectOpen;
%ignore bzs::db::protocol::tdap::client::convertTable;
%ignore bzs::db::protocol::tdap::client::createDatabase;
%ignore bzs::db::protocol::tdap::client::createDatabaseForConnectionPool;
%ignore bzs::db::protocol::tdap::client::createDatadaseObject;
%ignore bzs::db::protocol::tdap::client::deleteRecord;
%ignore bzs::db::protocol::tdap::client::disconnect;
%ignore bzs::db::protocol::tdap::client::dropDatabase;
%ignore bzs::db::protocol::tdap::client::filterdIterator;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator++;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator==;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator!=;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator*;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator->;
%ignore bzs::db::protocol::tdap::client::find;
%ignore bzs::db::protocol::tdap::client::findRv;
%ignore bzs::db::protocol::tdap::client::for_each;
%ignore bzs::db::protocol::tdap::client::getFindIterator;
%ignore bzs::db::protocol::tdap::client::getTable;
%ignore bzs::db::protocol::tdap::client::insertField;
%ignore bzs::db::protocol::tdap::client::insertKey;
%ignore bzs::db::protocol::tdap::client::insertRecord;
%ignore bzs::db::protocol::tdap::client::insertTable;
%ignore bzs::db::protocol::tdap::client::isSameUri;
%ignore bzs::db::protocol::tdap::client::lexical_cast;
%ignore bzs::db::protocol::tdap::client::openDatabase;
%ignore bzs::db::protocol::tdap::client::openTable;
%ignore bzs::db::protocol::tdap::client::prepare;
%ignore bzs::db::protocol::tdap::client::readIndex;
%ignore bzs::db::protocol::tdap::client::readIndex_v;
%ignore bzs::db::protocol::tdap::client::readIndexRv;
%ignore bzs::db::protocol::tdap::client::readIndexRv_v;
%ignore bzs::db::protocol::tdap::client::readStep;
%ignore bzs::db::protocol::tdap::client::readStepRv;
%ignore bzs::db::protocol::tdap::client::releaseDatabase;
%ignore bzs::db::protocol::tdap::client::releaseTable;
%ignore bzs::db::protocol::tdap::client::setQuery;
%ignore bzs::db::protocol::tdap::client::snapshot;
%ignore bzs::db::protocol::tdap::client::tableIterator;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator++;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator--;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator==;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator!=;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator*;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator->;
%ignore bzs::db::protocol::tdap::client::transaction;
%ignore bzs::db::protocol::tdap::client::updateRecord;
%ignore bzs::db::protocol::tdap::client::updateTableDef;
%ignore bzs::db::protocol::tdap::client::filter_validate_value;
%ignore bzs::db::protocol::tdap::client::filter_validate_block;
%ignore bzs::db::protocol::tdap::client::filter_invalidate_value;

// * bzs/db/protocol/tdap/client/trdormapi.h *
%ignore bzs::db::protocol::tdap::client::setValue;
%ignore bzs::db::protocol::tdap::client::begin;
%ignore bzs::db::protocol::tdap::client::end;
%ignore bzs::db::protocol::tdap::client::push_back;
%ignore bzs::db::protocol::tdap::client::readBefore;
%ignore bzs::db::protocol::tdap::client::mdlsHandler;
%ignore bzs::db::protocol::tdap::client::compFunc;
%ignore bzs::db::protocol::tdap::client::sortFunc;
%ignore bzs::db::protocol::tdap::client::sortFuncBase;
%ignore bzs::db::protocol::tdap::client::sortFunctor;
%ignore bzs::db::protocol::tdap::client::sort;
%ignore bzs::db::protocol::tdap::client::mraResetter;

// * bzs/db/protocol/tdap/tdapcapi.h *
%ignore BOOKMARK_ALLOC_SIZE;
%ignore BTRV_MAX_DATA_SIZE;
%ignore C_INTERFACE_VER_MAJOR;
%ignore C_INTERFACE_VER_MINOR;
%ignore C_INTERFACE_VER_RELEASE;
%ignore C_INTERFACE_VERSTR;
%ignore FILTER_COMBINE_NOPREPARE;
%ignore FILTER_COMBINE_PREPARE;
%ignore FILTER_TYPE_SEEKS;
%ignore FILTER_TYPE_SUPPLYVALUE;
%ignore FILTER_TYPE_FORWORD;
%ignore STATUS_LMIT_OF_PREPAREED;
%ignore STATUS_INVALID_EX_DESC;
%ignore STATUS_INVALID_EX_INS;
%ignore STATUS_INVALID_PREPAREID;
%ignore STATUS_INVALID_SUPPLYVALUES;
%ignore TDAP_MAX_DATA_SIZE;
%ignore TDCLC_LIBNAME;
%ignore TD_CPP_LIB_PRE;
%ignore TD_FILTER_PREPARE;
%ignore TD_LIB_PART;

// * bzs/db/protocol/tdap/tdapSchema.h *
%ignore DLLUNLOADCALLBACK_PTR;
%ignore dllUnloadCallback;
%ignore bzs::db::protocol::tdap::keySpec;
%ignore bzs::db::protocol::tdap::fileSpec;
%ignore bzs::db::protocol::tdap::fielddef::blobDataPtr;
%ignore bzs::db::protocol::tdap::fielddef::blobDataLen;
%ignore bzs::db::protocol::tdap::fielddef::chainChar;
%ignore bzs::db::protocol::tdap::fielddef::dataLen;
%ignore bzs::db::protocol::tdap::fielddef::getKeyValueFromKeybuf;
%ignore bzs::db::protocol::tdap::fielddef::keyCopy;
%ignore bzs::db::protocol::tdap::fielddef::keyData;
%ignore bzs::db::protocol::tdap::fielddef::keyDataLen;
%ignore bzs::db::protocol::tdap::fielddef::maxVarDatalen;
%ignore bzs::db::protocol::tdap::fielddef::nameA;
%ignore bzs::db::protocol::tdap::fielddef::setChainChar;
%ignore bzs::db::protocol::tdap::fielddef::setNameA;
%ignore bzs::db::protocol::tdap::fielddef::unPackCopy;
%ignore bzs::db::protocol::tdap::fielddef::varLenByteForKey;
%ignore bzs::db::protocol::tdap::fielddef_t::defValue;
%ignore bzs::db::protocol::tdap::fielddef_t::defViewWidth;
%ignore bzs::db::protocol::tdap::fielddef_t::enableFlags;
%ignore bzs::db::protocol::tdap::fielddef_t::filterId;
%ignore bzs::db::protocol::tdap::fielddef_t::filterKeynum;
%ignore bzs::db::protocol::tdap::fielddef_t::lookDBNum;
%ignore bzs::db::protocol::tdap::fielddef_t::lookField;
%ignore bzs::db::protocol::tdap::fielddef_t::lookFields;
%ignore bzs::db::protocol::tdap::fielddef_t::lookTable;
%ignore bzs::db::protocol::tdap::fielddef_t::userOption;
%ignore bzs::db::protocol::tdap::fielddef_t::varLenByteForKey;
%ignore bzs::db::protocol::tdap::fielddef_t::viewNum;
%ignore bzs::db::protocol::tdap::fielddef_t::viewWidth;
%ignore bzs::db::protocol::tdap::tabledef::autoIncExSpace;
%ignore bzs::db::protocol::tdap::tabledef::convertFileNum;
%ignore bzs::db::protocol::tdap::tabledef::fileNameA;
%ignore bzs::db::protocol::tdap::tabledef::filler0;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex2;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex3;
%ignore bzs::db::protocol::tdap::tabledef::optionFlags;
%ignore bzs::db::protocol::tdap::tabledef::parentKeyNum;
%ignore bzs::db::protocol::tdap::tabledef::replicaKeyNum;
%ignore bzs::db::protocol::tdap::tabledef::reserved;
%ignore bzs::db::protocol::tdap::tabledef::tableNameA;
%ignore bzs::db::protocol::tdap::tabledef::treeIndex;
%ignore bzs::db::protocol::tdap::tabledef::setFileNameA;
%ignore bzs::db::protocol::tdap::tabledef::setTableNameA;
  // add methods
%extend bzs::db::protocol::tdap::keydef {
  keySegment* segment(const int index)
  {
    return &(self->segments[index]);
  }
};
%extend bzs::db::protocol::tdap::tabledef {
  fielddef* fieldDef(const int index)
  {
    return &(self->fieldDefs[index]);
  }
  keydef* keyDef(const int index)
  {
    return &(self->keyDefs[index]);
  }
};
%extend bzs::db::protocol::tdap::fielddef {
  const char* name() const
  {
     return self->name();
  }
}
%ignore bzs::db::protocol::tdap::fielddef::name;
%extend bzs::db::protocol::tdap::btrVersions {
  btrVersion* version(const int index) {
    return &(self->versions[index]);
  }
};

// * bzs/rtl/benchmark.h *
%ignore bzs::rtl::benchmark::report;
%ignore bzs::rtl::benchmark::report2;
%ignore bzs::rtl::benchmarkMt;

// common %ignore
%ignore *::addref;
%ignore *::destroy;
%ignore *::refCount;
%ignore *::release;

/* ===============================================
      external symbols
=============================================== */
#define DLLLIB
%include bzs/env/compiler.h
#ifdef SWIGWIN
#define CP_ACP  0
#define CP_UTF8 65001
#else
%include bzs/env/mbcswchrLinux.h
#endif
#undef pragma_pack1
#define pragma_pack1
#undef pragma_pop
#define pragma_pop
%include bzs/db/protocol/tdap/tdapcapi.h
%include bzs/db/protocol/tdap/tdapSchema.h
%include bzs/db/protocol/tdap/client/nsTable.h
%include bzs/db/protocol/tdap/client/dbDef.h
%include bzs/db/protocol/tdap/client/table.h
%include bzs/db/protocol/tdap/client/nsDatabase.h
%include bzs/db/protocol/tdap/client/database.h
%include bzs/rtl/benchmark.h
%include bzs/db/protocol/tdap/mysql/characterset.h
// typemap for btrTimeStamp::toString/btrdtoa/btrttoa/btrstoa --
%typemap(in,numinputs=0) (char * retbuf) (char tmpbuf[255]) { $1=tmpbuf; }
%include bzs/db/protocol/tdap/btrDate.h
%clear char * retbuf;
// clear typemap for typemap for btrTimeStamp::toString/btrdtoa/btrttoa/btrstoa --
%include bzs/db/protocol/tdap/client/field.h
%include bzs/db/protocol/tdap/client/fields.h
%include bzs/db/protocol/tdap/client/memRecord.h
%include bzs/db/protocol/tdap/client/trdboostapi.h
%include bzs/db/protocol/tdap/client/groupQuery.h
%include bzs/db/protocol/tdap/client/recordset.h
%include bzs/db/protocol/tdap/client/activeTable.h
%include bzs/db/protocol/tdap/client/pooledDatabaseManager.h


/* ===============================================
      template
=============================================== */
/*
C++ templates are not supported in SWIG.
Template methods are declared only one signature pattern here.
We need to change the wrap_cpp manually to expand template codes.
*/

// * bzs/db/protocol/tdap/client/activeTable.h *
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%template(when) bzs::db::protocol::tdap::client::recordsetQuery::when<_TCHAR*>;
%template(and_) bzs::db::protocol::tdap::client::recordsetQuery::and_<_TCHAR*>;
%template(or_)  bzs::db::protocol::tdap::client::recordsetQuery::or_<_TCHAR*>;

// * bzs/db/protocol/tdap/client/table.h *
%template(where) bzs::db::protocol::tdap::client::query::where<_TCHAR*>;
%template(and_) bzs::db::protocol::tdap::client::query::and_<_TCHAR*>;
%template(or_)  bzs::db::protocol::tdap::client::query::or_<_TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
/* ===============================================
      cpointer
=============================================== */
#if defined(SWIGPHP)
%include "cpointer.i"
%pointer_functions(bzs::db::protocol::tdap::client::fieldsBase*, fieldsBase_p_p)
#endif
