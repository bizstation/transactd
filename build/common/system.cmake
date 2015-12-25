##=================================================================
#   Copyright (C) 2012 2013 BizStation Corp All rights reserved.
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 2
#   of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
#   02111-1307, USA.
##=================================================================
# ==========================================================
#   get compiler bitness
# ==========================================================
if(NOT COMMAND bz_get_compiler_bitness)
macro(bz_get_compiler_bitness)
  if(CMAKE_SIZEOF_VOID_P)
    set(SIZEOF_VOID_P "${CMAKE_SIZEOF_VOID_P}")
  else()
    include(CheckTypeSize)
    check_type_size("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
  endif()
  if(SIZEOF_VOID_P EQUAL 8)
    set(BZ_BITNESS "64")
  else()
    set(BZ_BITNESS "32")
  endif()
  if(MSVC AND "${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    if("${CMAKE_GENERATOR}" MATCHES "Win64")
      set(BZ_BITNESS "64")
    else()
      set(BZ_BITNESS "32")
    endif()
  endif()
endmacro()
endif()


# ==========================================================
#   get compiler
# ==========================================================
if(NOT COMMAND bz_get_compiler)
macro(bz_get_compiler)
  set(BZ_COMPILER "")
  if(WIN32)
    if(MSVC)
      if(MSVC60)
        set(BZ_COMPILER "_vc60")
      elseif(MSVC70)
        set(BZ_COMPILER "_vc70")
      elseif(MSVC80)
        set(BZ_COMPILER "_vc80")
      elseif(MSVC90)
        set(BZ_COMPILER "_vc90")
      elseif(MSVC10)
        set(BZ_COMPILER "_vc100")
      elseif(MSVC11)
        set(BZ_COMPILER "_vc110")
      elseif(MSVC12)
        set(BZ_COMPILER "_vc120")
      elseif(MSVC14)
        set(BZ_COMPILER "_vc140")
      else()
        MESSAGE(FATAL_ERROR "Unknown Visual Studio version.")
      endif()
    elseif(MINGW)
      string(REPLACE "." ";" tmp_list ${CMAKE_CXX_COMPILER_VERSION})
      list (GET tmp_list 0 MINGW_MAJOR)
      list (GET tmp_list 1 MINGW_MINOR)
      list (GET tmp_list 2 MINGW_RELEASE)
      set(BZ_COMPILER "_mgw${MINGW_MAJOR}${MINGW_MINOR}")
    else()
      MESSAGE(FATAL_ERROR "Unknown compiler.")
    endif()
  endif()
endmacro()
endif()


# ==========================================================
#   get windows system bitness
# ==========================================================
if(NOT COMMAND bz_get_win_bitness)
macro(bz_get_win_bitness)
  if(WIN32)
    set(TMP_ENV_PFX86_VALUENAME "PROGRAMFILES(X86)")
    set(tmp_var "$ENV{${TMP_ENV_PFX86_VALUENAME}}")
    if(DEFINED tmp_var)
      set(BZ_WIN_BITNESS "64")
    else()
      set(BZ_WIN_BITNESS "32")
    endif()
  else()
    set(BZ_WIN_BITNESS "")
  endif()
endmacro()
endif()


# ==========================================================
#   get windows system directories
# ==========================================================
if(NOT COMMAND bz_get_win_sysdir)
macro(bz_get_win_sysdir)
  set(BZ_WIN_SYSDIR_FOR_32BIT_BINARY "")
  set(BZ_WIN_SYSDIR_FOR_64BIT_BINARY "")
  set(BZ_WIN_SYSDIR "")
  if(WIN32)
    bz_get_win_bitness()
    set(tmp_var "$ENV{Systemroot}")
    if(DEFINED tmp_var)
      if("${BZ_WIN_BITNESS}" STREQUAL "32")
        file(TO_CMAKE_PATH "${tmp_var}/System32" BZ_WIN_SYSDIR_FOR_32BIT_BINARY)
      elseif("${BZ_WIN_BITNESS}" STREQUAL "64")
        file(TO_CMAKE_PATH "${tmp_var}/SysWOW64" BZ_WIN_SYSDIR_FOR_32BIT_BINARY)
        file(TO_CMAKE_PATH "${tmp_var}/System32" BZ_WIN_SYSDIR_FOR_64BIT_BINARY)
      endif()
    endif()
  endif()
  bz_get_compiler_bitness()
  if(("${BZ_BITNESS}" STREQUAL "32") OR ("${BZ_BITNESS}" STREQUAL "64"))
    set(BZ_WIN_SYSDIR "${BZ_WIN_SYSDIR_FOR_${BZ_BITNESS}BIT_BINARY}")
  endif()
endmacro()
endif()
