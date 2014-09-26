/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
      Support call_without_gvl
=============================================== */
%{
#if HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION

#include <build/swig/ruby/threadBlockRegionWrapper.h>

#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
    #include <ruby/thread.h>
    #define TRANSACTD_RB_CALL_WITHOUT_GVL( func, arg ) \
        rb_thread_call_without_gvl((void *(*)(void *))func, &arg, RUBY_UBF_IO, NULL)
#else
    // use rb_thread_blocking_region only if rb_thread_call_without_gvl is not defined.
    #define TRANSACTD_RB_CALL_WITHOUT_GVL( func, arg ) \
        rb_thread_blocking_region((rb_blocking_function_t*) func, &arg, RUBY_UBF_IO, NULL)
#endif

BTRCALLID_PTR MYTICALLID_ORIGINAL = NULL;
BTRCALLID_PTR BTRVCALLID_ORIGINAL = NULL;

void MYTICALLID_ArgumentWrapper(BTRCALLIDArgs* args)
{
    args->result = MYTICALLID_ORIGINAL(args->op, args->pbk, args->data,
        args->datalen, args->keybuf, args->keylen, args->keyNum, args->cid);
}

short_td __STDCALL MYTICALLID_WITHOUT_GVL(ushort_td op, void* pbk, void* data,
  uint_td* datalen, void* keybuf, keylen_td keylen, char_td keyNum, uchar_td* cid)
{
    BTRCALLIDArgs args;
    args.op = op;
    args.pbk = pbk;
    args.data = data;
    args.datalen = datalen;
    args.keybuf = keybuf;
    args.keylen = keylen;
    args.keyNum = keyNum;
    args.cid = cid;
    args.result = 0;
    TRANSACTD_RB_CALL_WITHOUT_GVL(MYTICALLID_ArgumentWrapper, args);
    return args.result;
}

void BTRCALLID_ArgumentWrapper(BTRCALLIDArgs* args)
{
    args->result = BTRVCALLID_ORIGINAL(args->op, args->pbk, args->data,
        args->datalen, args->keybuf, args->keylen, args->keyNum, args->cid);
}

short_td __STDCALL BTRVCALLID_WITHOUT_GVL(ushort_td op, void* pbk, void* data,
  uint_td* datalen, void* keybuf, keylen_td keylen, char_td keyNum, uchar_td* cid)
{
    BTRCALLIDArgs args;
    args.op = op;
    args.pbk = pbk;
    args.data = data;
    args.datalen = datalen;
    args.keybuf = keybuf;
    args.keylen = keylen;
    args.keyNum = keyNum;
    args.cid = cid;
    args.result = 0;
    TRANSACTD_RB_CALL_WITHOUT_GVL(BTRCALLID_ArgumentWrapper, args);
    return args.result;
}

#endif // HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION
%}
