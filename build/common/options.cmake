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
#   replace CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_replace_cxx_flag)
macro(bz_replace_cxx_flag ptn val build_type)
  if("${build_type}" STREQUAL "_")
    set(build_type "")
  elseif(NOT ("${build_type}" STREQUAL ""))
    string(REGEX MATCH "^_" tmp_match "${build_type}")
    if("${tmp_match}" STREQUAL "")
      set(build_type "_${build_type}")
    endif()
  endif()
  string(REGEX REPLACE "${ptn}" "${val}"
    CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
endmacro()
endif()

# ==========================================================
#   remove CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_remove_cxx_flag)
macro(bz_remove_cxx_flag opt build_type)
  bz_replace_cxx_flag("${opt}" "" "${build_type}")
endmacro()
endif()

# ==========================================================
#   add CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_add_cxx_flag)
macro(bz_add_cxx_flag opt build_type)
  if("${build_type}" STREQUAL "_")
    set(build_type "")
  elseif(NOT ("${build_type}" STREQUAL ""))
    string(REGEX MATCH "^_" tmp_match "${build_type}")
    if("${tmp_match}" STREQUAL "")
      set(build_type "_${build_type}")
    endif()
  endif()
  set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} ${opt}")
endmacro()
endif()

# ==========================================================
#   remove UNICODE or MBCS flag from CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_remove_charset_cxx_flag)
macro(bz_remove_charset_cxx_flag build_type)
  bz_remove_cxx_flag("-DUNICODE"  "${build_type}")
  bz_remove_cxx_flag("/DUNICODE"  "${build_type}")
  bz_remove_cxx_flag("-D_UNICODE" "${build_type}")
  bz_remove_cxx_flag("/D_UNICODE" "${build_type}")
  bz_remove_cxx_flag("-DMBCS"     "${build_type}")
  bz_remove_cxx_flag("/DMBCS"     "${build_type}")
  bz_remove_cxx_flag("-D_MBCS"    "${build_type}")
  bz_remove_cxx_flag("/D_MBCS"    "${build_type}")
endmacro()
endif()

# ==========================================================
#   set UNICODE flag to CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_set_UNICODE_cxx_flag)
macro(bz_set_UNICODE_cxx_flag build_type)
  bz_remove_charset_cxx_flag("${build_type}")
  bz_add_cxx_flag("-DUNICODE -D_UNICODE" "${build_type}")
endmacro()
endif()

# ==========================================================
#   set MBCS flag to CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_set_MBCS_cxx_flag)
macro(bz_set_MBCS_cxx_flag build_type)
  bz_remove_charset_cxx_flag("${build_type}")
  bz_add_cxx_flag("-D_MBCS" "${build_type}")
endmacro()
endif()

# ==========================================================
#   set /MT(d) flag to CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_set_MTMTd_cxx_flag)
macro(bz_set_MTMTd_cxx_flag build_type)
  bz_replace_cxx_flag("/MDd" "/MTd" "${build_type}")
  bz_replace_cxx_flag("/MD"  "/MT"  "${build_type}")
  if(    "${CMAKE_CXX_FLAGS${build_type}}" MATCHES "(.* )?/MT( .*)?$")
  elseif("${CMAKE_CXX_FLAGS${build_type}}" MATCHES "(.* )?/MTd( .*)?$")
  else()
    bz_add_cxx_flag("/MT" "${build_type}")
  endif()
endmacro()
endif()

# ==========================================================
#   set /MD(d) flag to CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_set_MDMDd_cxx_flag)
macro(bz_set_MDMDd_cxx_flag build_type)
  bz_replace_cxx_flag("/MTd" "/MDd" "${build_type}")
  bz_replace_cxx_flag("/MT"  "/MD"  "${build_type}")
  if(    "${CMAKE_CXX_FLAGS${build_type}}" MATCHES "(.* )?/MD( .*)?$")
  elseif("${CMAKE_CXX_FLAGS${build_type}}" MATCHES "(.* )?/MDd( .*)?$")
  else()
    bz_add_cxx_flag("/MD" "${build_type}")
  endif()
endmacro()
endif()
