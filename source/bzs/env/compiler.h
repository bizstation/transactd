#ifndef	BZS_ENV_COMPILER_H
#define	BZS_ENV_COMPILER_H
/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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
// function type
#if (__BCPLUSPLUS__)
#	if (defined(__APPLE__) || defined(__clang__))
#		define __THREAD __declspec(thread)
#		define __STDCALL
#	else
#		define __THREAD __thread
#		define __STDCALL __stdcall
#	endif
#else
#	define __THREAD __declspec(thread)
#	if (_MSC_VER || __MINGW32__)
#		define __STDCALL __stdcall
#	else
#		define __STDCALL
#	endif
#endif


// operating system
#ifdef _WIN32
#	ifdef _WIN64
#		ifndef __x86_64__
#			define __x86_64__
#		endif
#	else
#		define __x86_32__
#	endif
#else
#	ifndef LINUX
#		define LINUX 1//Support 64bit only
#	endif
#endif
#if (__BCPLUSPLUS__ && defined(__APPLE__))
#	define __APPLE_32__ 1
#endif
// thread strage
#ifndef __THREAD_BCB
#	undef __THREAD
#	if (__BCPLUSPLUS__ && !defined(__clang__))
#		define __THREAD_BCB __thread
#		define __THREAD
#	else
#		define __THREAD_BCB
#		if (_MSC_VER || (__BORLANDC__ && __clang__))
#			define __THREAD __declspec(thread)
#		else
#			define __THREAD __thread
#		endif
#	endif
#endif//__THREAD_BCB

// BCB_64
#if (__BCPLUSPLUS__ && defined(__clang__))
#	define BCB_64
#endif

//BCB32
#if (__BCPLUSPLUS__ && defined(__x86_32__))
#	define BCB_32
#endif

// export 
#undef PACKAGE
#ifdef __BCPLUSPLUS__
#	ifndef PACKAGE
#		ifdef __x86_64__
#			define PACKAGE __declspec(dllexport)
#		else
#			define PACKAGE __declspec(package)
#		endif
#	endif
#else
#	if (_WIN32 && !defined(__MINGW32__))
#		define PACKAGE __declspec(dllexport)
#	else
#		define PACKAGE
#	endif
#endif

#ifdef PACKAGE_NO_EXPORT
#	undef PACKAGE
#	define PACKAGE
#endif

#define AGRPACK PACKAGE

//bcb_osx
#if (__BCPLUSPLUS__ && defined(__APPLE__) && !defined(__clang__))
#	define PACKAGE_OSX  PACKAGE
#else
#   define PACKAGE_OSX
#endif

// import
#if (defined(LINUX) || __MINGW32__)
#	define PACKAGE_IMPORT
#else
#	define PACKAGE_IMPORT __declspec(dllimport)
#endif



// data alignment
#if (_MSC_VER || (__BORLANDC__ && __clang__ ))
#	define pragma_pack1 __pragma(pack(push, 1))
#	define pragma_pop __pragma(pack(pop))
#else
#	ifdef __BCPLUSPLUS__
#		define pragma_pack1
#		define pragma_pop
#	else
#		define pragma_pack1 _Pragma("pack(1)")
#		define pragma_pop _Pragma("pack()")
#	endif
#endif

//compiler name
#if (_MSC_VER == 1600) 
#	define COMPILER_VERSTR "vc100"
#endif

#if (_MSC_VER == 1700)
#	define COMPILER_VERSTR "vc110"
#endif

#if (_MSC_VER == 1800)
#	define COMPILER_VERSTR "vc120"
#endif


#if (__BCPLUSPLUS__ >= 0x630 && (__BCPLUSPLUS__ < 0x640))
#	define COMPILER_VERSTR  "bc150"
#endif

#if (__BCPLUSPLUS__ >= 0x640 && (__BCPLUSPLUS__ < 0x650))
#	define COMPILER_VERSTR  "bc160"
#endif

#if (__BCPLUSPLUS__ >= 0x650 && (__BCPLUSPLUS__ < 0x660))
#  	define COMPILER_VERSTR  "bc170"
#endif

#if (__BCPLUSPLUS__ >= 0x660 && (__BCPLUSPLUS__ < 0x670))
#  	define COMPILER_VERSTR  "bc180"
#endif

#if (__BCPLUSPLUS__ >= 0x670 && (__BCPLUSPLUS__ < 0x680))
#  	define COMPILER_VERSTR  "bc190"
#endif

#if (__BCPLUSPLUS__ >= 0x680 && (__BCPLUSPLUS__ < 0x690))
#  	define COMPILER_VERSTR  "bc200"
#endif

#if (__APPLE__)
#	define SHARED_LIB_EXTENTION ".dylib"
#else
#	if( defined(LINUX) || (__BORLANDC__ && __clang__))
#		define LIB_EXTENTION ".so"
#	else
#   	define LIB_EXTENTION ".lib"
#	endif
#endif

#if( defined(LINUX) || (__BORLANDC__ && __clang__))
#	define LIB_EXTENTION ".a"
#else
#  	define LIB_EXTENTION ".lib"
#endif


#ifdef LINUX
#	define LIB_PREFIX "lib"
#else
#	define LIB_PREFIX
#endif

#endif//BZS_ENV_COMPILER_H
