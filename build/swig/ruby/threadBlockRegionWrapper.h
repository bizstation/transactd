#ifndef TRANSACTD_THREADBLOCKREGIONWRAPPER_H
#define TRANSACTD_THREADBLOCKREGIONWRAPPER_H
/* =================================================================
 Copyright (C) 2000-2014 BizStation Corp All rights reserved.

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
#undef TRANSACTD_RB_CALL_WITHOUT_GVL

#if HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION

#include <ruby.h>
#undef stat
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
#include <ruby/thread.h>
#define TRANSACTD_RB_CALL_WITHOUT_GVL( func, arg ) \
	rb_thread_call_without_gvl((void *(*)(void *))func, &arg, RUBY_UBF_IO, NULL)
#else
// use rb_thread_blocking_region only if rb_thread_call_without_gvl is not defined.
#define TRANSACTD_RB_CALL_WITHOUT_GVL( func, arg ) \
	rb_thread_blocking_region((rb_blocking_function_t*) func, &arg, RUBY_UBF_IO, NULL)
#endif

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

struct BTRCALLIDArgs
{
	ushort_td op;
	void* pbk;
	void* data;
	uint_td* datalen;
	void* keybuf;
	keylen_td keylen;
	char_td keyNum;
	uchar_td* cid;
	short_td result;
};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif //HAVE_RB_THREAD_CALL_WITHOUT_GVL || HAVE_RB_THREAD_BLOCKING_REGION
#endif //not TRANSACTD_THREADBLOCKREGIONWRAPPER_H
