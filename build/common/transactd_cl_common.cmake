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
#   add source files macro
# ==========================================================
if(NOT COMMAND tdcl_add_source_files)
macro(tdcl_add_source_files TRANSACTD_ROOT)
  set(${this_target}_SOURCE_FILES
    ${${this_target}_SOURCE_FILES}
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/btrDate.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/myDateTime.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/tdapSchema.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/activeTable.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/connMgr.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/database.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/databaseFactory.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/dbDef.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/errorMessage.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/field.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/fieldDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/fieldNameAlias.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/fileDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/groupQuery.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/indexDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/memRecord.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/nsDatabase.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/nsTable.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/recordset.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/sharedData.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/table.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/characterset.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/datetime.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/stringBuffers.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/strtrim.cpp
  )
  if(UNIX)
    set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
      ${TRANSACTD_ROOT}/source/bzs/env/crosscompile.cpp
      ${TRANSACTD_ROOT}/source/bzs/env/mbcswchrLinux.cpp
    )
  endif()
endmacro()
endif()


# ==========================================================
#   add include directory macro
# ==========================================================
if(NOT COMMAND tdcl_add_include_directory)
macro(tdcl_add_include_directory TRANSACTD_ROOT)
  include_directories(${TRANSACTD_ROOT}/source;)
  if(UNIX)
    include_directories(${TRANSACTD_ROOT}/source/linux;)
  endif()
endmacro()
endif()

# ==========================================================
#   set compiler flags macro
# ==========================================================
if(NOT COMMAND tdcl_set_compiler_flags)
macro(tdcl_set_compiler_flags)
  foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
    if(MSVC)
      bz_remove_cxx_flag("/EHsc" "${build_type}")
      bz_remove_cxx_flag("/EHs" "${build_type}")
      bz_remove_cxx_flag("/MDd" "${build_type}")
      bz_remove_cxx_flag("/MTd" "${build_type}")
      bz_remove_cxx_flag("/MD" "${build_type}")
      bz_remove_cxx_flag("/MT" "${build_type}")
      bz_add_cxx_flag(" /nologo /Zi /W3 /WX- /Oy- /EHa /errorReport:prompt" "${build_type}")
      bz_add_cxx_flag(" /fp:precise /Zc:wchar_t /Zc:forScope /GS /Gd" "${build_type}")
      bz_add_cxx_flag(" /wd4068 /wd4275 /wd4819 /wd4251" "${build_type}")
      bz_remove_cxx_flag("-DUNICODE"  "${build_type}")
      bz_remove_cxx_flag("/DUNICODE"  "${build_type}")
      bz_remove_cxx_flag("-D_UNICODE" "${build_type}")
      bz_remove_cxx_flag("/D_UNICODE" "${build_type}")
      bz_remove_cxx_flag("-DMBCS"     "${build_type}")
      bz_remove_cxx_flag("/DMBCS"     "${build_type}")
      bz_remove_cxx_flag("-D_MBCS"    "${build_type}")
      bz_remove_cxx_flag("/D_MBCS"    "${build_type}")
    else()
      bz_remove_cxx_flag("-fno-exceptions" "${build_type}")
      bz_remove_cxx_flag("-fno-rtti" "${build_type}")
      bz_remove_cxx_flag("-fno-implicit-templates" "${build_type}")
      if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        bz_add_cxx_flag("-std=c++11 -Wno-unknown-pragmas -Wno-char-subscripts -Wno-deprecated-register" "${build_type}")
      endif()
      bz_add_cxx_flag("-fPIC -fexceptions -fno-omit-frame-pointer" "${build_type}")
      bz_add_cxx_flag("-fno-strict-aliasing -fpermissive -frtti" "${build_type}")
      bz_add_cxx_flag("-D_MBCS" "${build_type}")
    endif()
  endforeach()
  if(MSVC)
    bz_add_cxx_flag(" /ZI /Od /Ob0 /Gm /RTC1" "_DEBUG")
    bz_add_cxx_flag(" /Zi /Ox /Oi /GL /Gm- /Gy" "_RELEASE")
  endif()
endmacro()
endif()
