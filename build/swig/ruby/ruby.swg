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
#if defined(SWIGRUBY)  // ======= For RUBY =======
/* ===============================================
      exception handler
=============================================== */
%exception {
  try {
    $action
  } catch (bzs::rtl::exception& e) {
    static VALUE bzs_rtl_error = rb_define_class("BZS_RTL_Error", rb_eStandardError);
    rb_raise(bzs_rtl_error, (* bzs::rtl::getMsg(e)).c_str());
  } catch (std::exception &e) {
    static VALUE cpp_std_error = rb_define_class("CPP_STD_Error", rb_eStandardError);
    rb_raise(cpp_std_error, e.what());
  }
}


/* ===============================================
  General settings for Ruby
=============================================== */
%{
#if defined(__MINGW32__) && defined(__GNUC__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 5)
#define _SYS_TIME_H_
#else
#define _GETTIMEOFDAY_DEFINED
#endif
#endif
%}

// Suppress warning
#pragma SWIG nowarn=SWIGWARN_RUBY_WRONG_NAME

// Ruby can't convert String to c++ wchar_t so define _TCHAR as char.
#ifdef _UNICODE
# undef _UNICODE
#endif
#ifdef UNICODE
# undef UNICODE
#endif
typedef char  _TCHAR;
#define _T(x) x

// add encoding support method for ruby 1.9
%{
#include "ruby/encoding.h"
#undef stat
#include <bzs/env/tstring.h>

rb_encoding* rb_enc_find_from_codepage(int codePage)
{
  if (codePage <= 0 || codePage >= 10000000)
    codePage = GetACP();
  char cpname[10];
  sprintf(cpname, "CP%2d", codePage);
  return rb_enc_find(cpname);
}
%}

// typemaps for encoding support
%typemap(argout) (const char* u8, int u8size, char* mbc, int mbcsize) {
  %append_output(rb_str_new($3, strlen($3)));
}
%typemap(argout) (const char* mbc, int mbcsize, char* u8, int u8size) {
  %append_output(rb_str_new($3, strlen($3)));
}
%typemap(out) const char* {
  $result = rb_enc_str_new($1, strlen($1), rb_enc_find_from_codepage(CP_UTF8));
}
%typemap(out) char* {
  $result = rb_enc_str_new($1, strlen($1), rb_enc_find_from_codepage(CP_UTF8));
}


/* ===============================================
  mark objects with reference counter
=============================================== */
%trackobjects bzs::db::protocol::tdap::client::activeTable;
%markfunc bzs::db::protocol::tdap::client::activeTable "mark_activeTable";
%{
#include <build/swig/referencecounter.h>
referenceCounter g_refCounter;
static void mark_activeTable(void* ptr) {
  g_refCounter.mark();
}
%}


/* ===============================================
  record value mode
=============================================== */
%ignore g_fieldValueMode;
%ignore g_recordValueMode;
%inline %{
#define FIELDVALUEMODE_RETURNNULL   0
#define FIELDVALUEMODE_NORETURNNULL 1
#define RECORD_KEYVALUE_FIELDVALUE  0
#define RECORD_KEYVALUE_FIELDOBJECT 1

static short g_fieldValueMode = FIELDVALUEMODE_NORETURNNULL;
static short g_recordValueMode = RECORD_KEYVALUE_FIELDOBJECT;

short fieldValueMode() {
  return g_fieldValueMode;
}
void setFieldValueMode(short mode) {
  g_fieldValueMode = mode;
}
short recordValueMode() {
  return g_recordValueMode;
}
void setRecordValueMode(short mode) {
  g_recordValueMode = mode;
}
%}


/* ===============================================
  ignore / rename functions
=============================================== */
// * bzs/db/protocol/tdap/client/activeTable.h *
  // add wrappers
%extend bzs::db::protocol::tdap::client::activeTable {
  activeTable(idatabaseManager* mgr, const _TCHAR* tableName) {
    bzs::db::protocol::tdap::client::activeTable* p =
      bzs::db::protocol::tdap::client::activeTable::create(mgr, tableName);
    g_refCounter.add(p, mgr);
    g_vPtrList.add(p->table().get());
    return p;
  }
  activeTable(database* db, const _TCHAR* tableName) {
    bzs::db::protocol::tdap::client::activeTable* p =
      bzs::db::protocol::tdap::client::activeTable::create(db, tableName);
    g_refCounter.add(p, db);
    g_vPtrList.add(p->table().get());
    return p;
  }
  ~activeTable() {
    if (g_vPtrList.remove(self->table().get()))
    {
      if (nsdatabase::testTablePtr(self->table().get()))
        self->table()->nsdb()->setTestPtrIgnore(true);
    }
    g_refCounter.remove(self);
    self->release();
  }
};
  // ignore original functions
%ignore bzs::db::protocol::tdap::client::activeTable::activeTable;
%ignore bzs::db::protocol::tdap::client::activeTable::~activeTable;

// * bzs/db/protocol/tdap/client/database.h *
%ignore bzs::db::protocol::tdap::client::database::onCopyData;
%ignore bzs::db::protocol::tdap::client::database::setOnCopyData;
%ignore bzs::db::protocol::tdap::client::database::onDeleteRecord;
%ignore bzs::db::protocol::tdap::client::database::setOnDeleteRecord;

// * bzs/db/protocol/tdap/client/field.h *
%ignore bzs::db::protocol::tdap::client::fielddefs::operator[] (const std::_tstring&) const;
%rename(__getitem__) bzs::db::protocol::tdap::client::fielddefs::operator[] (int index) const;
%rename(__getitem__) bzs::db::protocol::tdap::client::fielddefs::operator[] (const _TCHAR* name) const;
  // getBin typemap
%typemap(in, numinputs=0) (uint_td & size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & size) {
  $result = rb_str_new((const char *)result, *$1);
}

// * bzs/db/protocol/tdap/client/fields.h *
%ignore bzs::db::protocol::tdap::client::fieldsBase::operator[] (const std::_tstring&) const;
%rename(getField) bzs::db::protocol::tdap::client::fieldsBase::operator[] (short index) const;
%rename(getField) bzs::db::protocol::tdap::client::fieldsBase::operator[] (const _TCHAR* name) const;
  // typemap and expand for get field value.
%typemap(argout) (short fieldsBase_getitem_index) {
  bzs::db::protocol::tdap::client::field f;
  try {
    f = arg1->operator[]($1);
  } catch (bzs::rtl::exception& e) {
    static VALUE bzs_rtl_error = rb_define_class("BZS_RTL_Error", rb_eStandardError);
    rb_raise(bzs_rtl_error, (* bzs::rtl::getMsg(e)).c_str());
  } catch (std::exception &e) {
    static VALUE cpp_std_error = rb_define_class("CPP_STD_Error", rb_eStandardError);
    rb_raise(cpp_std_error, e.what());
  }
  if (recordValueMode() == RECORD_KEYVALUE_FIELDVALUE) { // value mode
    if (fieldValueMode() == FIELDVALUEMODE_RETURNNULL && f.isNull()) { // return null
      $result = Qnil;
    } else {
      __int64 tmp_i64;
      double tmp_d;
      const _TCHAR* tmp_c_str;
      switch (f.type()) {
        case ft_integer:
        case ft_uinteger:
        case ft_autoinc:
        case ft_autoIncUnsigned:
        case ft_logical:
        case ft_bit:
          tmp_i64 = f.i64();
          $result = SWIG_From_long_SS_long(static_cast< long long >(tmp_i64));
          break;
        case ft_float:
        case ft_decimal:
        case ft_money:
        case ft_numeric:
        case ft_bfloat:
        case ft_numericsts:
        case ft_numericsa:
        case ft_currency:
          tmp_d = f.d();
          $result = SWIG_From_double(static_cast< double >(tmp_d));
          break;
        default:
          tmp_c_str = f.c_str();
          $result = rb_enc_str_new(tmp_c_str, strlen(tmp_c_str), rb_enc_find_from_codepage(CP_UTF8));
      }
    }
  } else { // field object mode
    $result = SWIG_NewPointerObj(new bzs::db::protocol::tdap::client::field(f), SWIGTYPE_p_bzs__db__protocol__tdap__client__field, SWIG_POINTER_OWN |  0 );
  }
}
%apply short fieldsBase_getitem_index { const _TCHAR* fieldsBase_getitem_index };
%extend bzs::db::protocol::tdap::client::fieldsBase {
  void __getitem__(short fieldsBase_getitem_index) const {
  }
  void __getitem__(const _TCHAR* fieldsBase_getitem_index) const {
  }
};
%clear _TCHAR* fieldsBase_getitem_index;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%rename(__getitem__) bzs::db::protocol::tdap::client::fieldNames::operator[] (int index) const;
%rename(__getitem__) bzs::db::protocol::tdap::client::sortFields::operator[];

// * bzs/db/protocol/tdap/client/memRecord.h *
%ignore bzs::db::protocol::tdap::client::field::operator=(const char*);
%ignore bzs::db::protocol::tdap::client::field::operator=(const std::string&);
%ignore bzs::db::protocol::tdap::client::field::operator=(int);
%ignore bzs::db::protocol::tdap::client::field::operator=(float);
%rename(str) bzs::db::protocol::tdap::client::field::c_str;
%alias bzs::db::protocol::tdap::client::field::getBin "bin";
%rename(__setitem__) bzs::db::protocol::tdap::client::field::operator=(const _TCHAR*);
%rename(__setitem__) bzs::db::protocol::tdap::client::field::operator=(__int64);
%rename(__setitem__) bzs::db::protocol::tdap::client::field::operator=(double);
%rename(__setitem__) bzs::db::protocol::tdap::client::field::setBin;
%alias bzs::db::protocol::tdap::client::field::setBin "setBin";
%alias bzs::db::protocol::tdap::client::field::setBin "setValue";
%extend bzs::db::protocol::tdap::client::writableRecord {
  void __setitem__(short writableRecord_setitem_index, const _TCHAR* v) {
    self->operator[](writableRecord_setitem_index) = v;
  }
  void __setitem__(const _TCHAR* writableRecord_setitem_index, const _TCHAR* v) const {
    self->operator[](writableRecord_setitem_index) = v;
  }
  void __setitem__(short writableRecord_setitem_index, __int64 v) {
    self->operator[](writableRecord_setitem_index) = v;
  }
  void __setitem__(const _TCHAR* writableRecord_setitem_index, __int64 v) const {
    self->operator[](writableRecord_setitem_index) = v;
  }
  void __setitem__(short writableRecord_setitem_index, double v) {
    self->operator[](writableRecord_setitem_index) = v;
  }
  void __setitem__(const _TCHAR* writableRecord_setitem_index, double v) const {
    self->operator[](writableRecord_setitem_index) = v;
  }
  // binary
};

// * bzs/db/protocol/tdap/client/recordset.h *
%rename(__getitem__) bzs::db::protocol::tdap::client::recordset::operator[];

// * bzs/db/protocol/tdap/client/table.h *
  // define setFVbin
%ignore bzs::db::protocol::tdap::client::table::setFV(short, const void *, uint_td);
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR*, const void *, uint_td);
%typemap(in, numinputs=0) (uint_td & table_setFVbin_size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & table_setFVbin_size) {
  $result = rb_str_new((const char *)result, *$1);
}
%extend bzs::db::protocol::tdap::client::table {
  void setFV(short index, const char * data, uint_td table_setFVbin_size) {
    return self->setFV(index, (void const *) data, table_setFVbin_size);
  }
  void setFV(const _TCHAR *fieldName, const char * data, uint_td table_setFVbin_size) {
    return self->setFV(fieldName, (void const *) data, table_setFVbin_size);
  }
};
  // fix getFVbin
%typemap(in, numinputs=0) (uint_td & table_getFVbin_size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & table_getFVbin_size) {
  $result = rb_str_new((const char *)result, *$1);
}
%extend bzs::db::protocol::tdap::client::table {
  void* getFVbin(short index, uint_td& table_getFVbin_size)
  {
    return self->getFVbin(index, table_getFVbin_size);
  }
  void* getFVbin(const _TCHAR *fieldName, uint_td& table_getFVbin_size)
  {
    return self->getFVbin(fieldName, table_getFVbin_size);
  }
};
%ignore bzs::db::protocol::tdap::client::table::getFVbin(short, uint_td&);
%ignore bzs::db::protocol::tdap::client::table::getFVbin(const _TCHAR *, uint_td&);

// * bzs/db/protocol/tdap/tdapSchema.h *
%rename(defaultValue) bzs::db::protocol::tdap::fielddef::defaultValue_str;


/* ===============================================
  add code to init section
=============================================== */
%init %{
    bzs::db::protocol::tdap::client::nsdatabase::setCheckTablePtr(true);
#if HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION
    if (MYTICALLID_ORIGINAL == NULL)
    {
        MYTICALLID_ORIGINAL = bzs::db::protocol::tdap::client::getTrnsctdEntryPoint();
        if (MYTICALLID_ORIGINAL)
            bzs::db::protocol::tdap::client::setTrnsctdEntryPoint((BTRCALLID_PTR) MYTICALLID_WITHOUT_GVL);
        else
            bzs::db::protocol::tdap::client::setTrnsctdEntryPoint((BTRCALLID_PTR) NULL);
        
        BTRVCALLID_ORIGINAL = bzs::db::protocol::tdap::client::getBtrvEntryPoint();
        if (BTRVCALLID_ORIGINAL)
            bzs::db::protocol::tdap::client::setBtrvEntryPoint((BTRCALLID_PTR) BTRVCALLID_WITHOUT_GVL);
        else
            bzs::db::protocol::tdap::client::setBtrvEntryPoint((BTRCALLID_PTR) NULL);
    }
#endif // HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION
%}
#endif // =============== For RUBY ===============
