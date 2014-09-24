#ifndef BZS_ENV_BOOST_LINK_BCB_H
#define BZS_ENV_BOOST_LINK_BCB_H
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
#ifdef __BCPLUSPLUS__
#	include <bzs/db/protocol/tdap/tdapcapi.h>
#	ifdef _WIN64
#		if (__BCPLUSPLUS__ >= 0x690)
#			define TD_BOOST_VER_STR "1_55"
#		else
#           define TD_BOOST_VER_STR "1_50"
#		endif
#		ifdef _RTLDLL
#			ifdef BZS_LINK_BOOST_SYSTEM
#				pragma comment(lib, "libboost_system-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_FILESYSTEM
#				pragma comment(lib, "libboost_filesystem-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_SERIALIZATION
#				pragma comment(lib, "libboost_serialization-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_CHRONO
#				pragma comment(lib, "libboost_chrono-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_TIMER
#				pragma comment(lib, "libboost_timer-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_THREAD
#				pragma comment(lib, "libboost_thread-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_PROGRAM_OPTIONS
#				pragma comment(lib, "libboost_program_options-bcb64-mt-" TD_BOOST_VER_STR ".a")
#			endif
#		else
#			ifdef BZS_LINK_BOOST_SYSTEM
#				pragma comment(lib, "libboost_system-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_FILESYSTEM
#				pragma comment(lib, "libboost_filesystem-bcb64-mt-s-" TD_BOOST_VER_STR ".a)
#			endif
#			ifdef BZS_LINK_BOOST_SERIALIZATION
#				pragma comment(lib, "libboost_serialization-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_CHRONO
#				pragma comment(lib, "libboost_chrono-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_TIMER
#				pragma comment(lib, "libboost_timer-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_THREAD
#				pragma comment(lib, "libboost_thread-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#			ifdef BZS_LINK_BOOST_PROGRAM_OPTIONS
#				pragma comment(lib, "libboost_program_options-bcb64-mt-s-" TD_BOOST_VER_STR ".a")
#			endif
#		endif
#	else
#		if (defined(__APPLE__))
#       		define LIB_EXT "a"
#			define THREADNAME "thread_pthread"
#		else
#			define LIB_EXT "lib"
#			define THREADNAME "thread"
#		endif
#		ifdef _RTLDLL
#			ifdef BZS_LINK_BOOST_SYSTEM
#				pragma comment(lib, "libboost_system-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_FILESYSTEM
#				pragma comment(lib, "libboost_filesystem-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_SERIALIZATION
#				pragma comment(lib, "libboost_serialization-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_CHRONO
#				pragma comment(lib, "libboost_chrono-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_TIMER
#				pragma comment(lib, "libboost_timer-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_THREAD
#				pragma comment(lib, "libboost_"THREADNAME"-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_PROGRAM_OPTIONS
#				pragma comment(lib, "libboost_program_options-bcb-mt-1_39"LIB_EXTENTION)
#			endif
#		else
#			ifdef BZS_LINK_BOOST_SYSTEM
#				pragma comment(lib, "libboost_system-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_FILESYSTEM
#				pragma comment(lib, "libboost_filesystem-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_SERIALIZATION
#				pragma comment(lib, "libboost_serialization-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_CHRONO
#				pragma comment(lib, "libboost_chrono-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_TIMER
#				pragma comment(lib, "libboost_timer-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_THREAD
#				pragma comment(lib, "libboost_"THREADNAME"-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#			ifdef BZS_LINK_BOOST_PROGRAM_OPTIONS
#				pragma comment(lib, "libboost_program_options-bcb-mt-s-1_39"LIB_EXTENTION)
#			endif
#		endif
#	endif
#endif


#endif//BZS_ENV_BOOST_LINK_BCB_H
