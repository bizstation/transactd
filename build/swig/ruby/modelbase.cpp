/* =================================================================
 Copyright (C) 2016 BizStation Corp All rights reserved.

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
 ================================================================= */
#ifndef TD_MODELBASE_INCLUDED
#define TD_MODELBASE_INCLUDED
VALUE mTransactd_Model;
VALUE mTransactd_Model_Base;


VALUE getModelBase() {
  VALUE id_base = rb_intern("Base");
  if (rb_const_defined(mTransactd_Model, id_base)) {
    VALUE ret = rb_const_get(mTransactd_Model, id_base);
    if (ret != Qnil) return ret;
  }
  rb_raise(rb_eLoadError, "can not load Transactd::Model::Base class.");
  return Qnil;
}


void setAliasedFields(VALUE klass, VALUE aliases, VALUE fields) {
  int length = RARRAY_LEN(fields);
  VALUE result = rb_ary_new2(length);
  for (size_t i = 0; i < length; ++i) {
    VALUE orig_name = rb_ary_entry(fields, i);
    VALUE aliased_name = Qnil;
    aliased_name = rb_hash_lookup2(aliases, orig_name, Qnil);
    if (aliased_name == Qnil) {
      aliased_name = rb_hash_lookup2(aliases, rb_to_symbol(orig_name), Qnil);
    }
    if (aliased_name != Qnil) {
      rb_ary_store(result, i, aliased_name);
    } else {
      rb_ary_store(result, i, orig_name);
    }
  }
  rb_ivar_set(klass, g_id_aliased_fields, result);
}


VALUE createModelClass(const char* classname, const tdap::fielddef* fds, int length,
                       VALUE aliases, bool undef_orig, bool cache_aliased_fields) {
  if (mTransactd_Model_Base == Qnil) mTransactd_Model_Base = getModelBase();
  VALUE klass = rb_define_class(classname, mTransactd_Model_Base);
  if (! getAccessorInitialized(klass)) {
    VALUE fields = getFieldsArray(fds, length);
    setAccessors(klass, fields, aliases, undef_orig);
    if (cache_aliased_fields) setAliasedFields(klass, aliases, fields);
  }
  return klass;
}


VALUE _wrap_createModelClass(int argc, VALUE *argv, VALUE self) {
  tdap::client::activeTable *arg1_a = (tdap::client::activeTable *) 0 ;
  tdap::client::table *arg1_t = (tdap::client::table *) 0 ;
  void *argp1 = 0 ;
  char *arg2 = 0 ;
  char *buf2 = 0 ;
  int alloc2 = 0 ;
  bool undef_orig = false;
  bool cache_a_f = false;
  
  if (!check_param_count(argc, 4, 4)) return Qnil;
  {
    int res = SWIG_ConvertPtr(argv[0], &argp1, SWIGTYPE_p_bzs__activeTable, 0 | 0 );
    if (SWIG_IsOK(res)) {
      arg1_a = reinterpret_cast< tdap::client::activeTable * >(argp1);
      arg1_t = arg1_a->table().get();
    } else {
      res = SWIG_ConvertPtr(argv[0], &argp1, SWIGTYPE_p_bzs__table, 0 | 0 );
      if (!SWIG_IsOK(res)) {
        SWIG_exception_fail(SWIG_ArgError(res), Ruby_Format_TypeError( "", "activeTable* or table*","createModelClass", 1, argv[0] )); 
      }
      arg1_t = reinterpret_cast< tdap::client::table * >(argp1);
      cache_a_f = true;
    }
  }
  if (TYPE(argv[2]) != T_HASH) {
    rb_raise(rb_eArgError, "argument 3 of type hash.");
  }
  if (TYPE(argv[3]) == T_TRUE) {
    undef_orig = true;
  }
  {
    int res = SWIG_AsCharPtrAndSize(argv[1], &buf2, NULL, &alloc2);
    if (!SWIG_IsOK(res)) {
      SWIG_exception_fail(SWIG_ArgError(res), Ruby_Format_TypeError( "", "char const *","createModelClass", 2, argv[1] ));
    }
    arg2 = reinterpret_cast< char * >(buf2);
  }
  createModelClass(arg2, arg1_t->tableDef()->fieldDefs, arg1_t->tableDef()->fieldCount, argv[2], undef_orig, cache_a_f);
  if (alloc2 == SWIG_NEWOBJ) delete[] buf2;
  return Qnil;
fail:
  if (alloc2 == SWIG_NEWOBJ) delete[] buf2;
  return Qnil;
}

SWIGINTERN VALUE
_wrap_Recordset_to_class_array(int argc, VALUE *argv, VALUE self) {
  tdap::client::recordset *rs = (tdap::client::recordset *) 0 ;
  VALUE fields = Qnil;
  VALUE vresult = Qnil;
  
  if (!check_param_count(argc, 2, 2)) return Qnil;
  rs = selfPtr(self, rs);
  
  if (TYPE(argv[0]) != T_CLASS) {
    rb_raise(rb_eArgError, "argument 1 of type Class.");
  }
  
  if (TYPE(argv[1]) != T_ARRAY) {
    rb_raise(rb_eArgError, "argument 2 of type Array.");
  }
  
  if (rs->size() == 0) return rb_ary_new();
  
  int ctorArgs_size = RARRAY_LEN(argv[1]);
  VALUE* ctorArgs_arr = RARRAY_PTR(argv[1]);
  
  fields = getFieldIdsCache(self, *rs->fieldDefs());
  const size_t fields_size = RARRAY_LEN(fields);
  VALUE* fields_arr = RARRAY_PTR(fields);
  
  try {
    const size_t size = rs->size();
    vresult = rb_ary_new2(size);
    for (size_t index = 0; index < size; ++index) {
      VALUE obj = rb_class_new_instance(ctorArgs_size, ctorArgs_arr, argv[0]);
      for (size_t i = 0; i < fields_size; ++i) {
        rb_ivar_set(obj, fields_arr[i], getFieldValue((*rs)[index][i]));
      }
      rb_ary_store(vresult, index, obj);
    }
    return vresult;
  }
  CATCH_BZS_AND_STD()
fail:
  return Qnil;
}
#endif // ifndef TD_MODELBASE_INCLUDED
