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
if(NOT COMMAND check_for_link_iconv)
macro(check_for_link_iconv)
if(UNIX)
  if(NOT CMAKE_FIND_LIBRARY_PREFIXES)
    set(CMAKE_FIND_LIBRARY_PREFIXES "" lib)
  endif()
  if(NOT CMAKE_FIND_LIBRARY_SUFFIXES)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "\\.so" "\\.a")
  endif()
  # find iconv.h
  if(NOT DEFINED ICONV_INCLUDE_DIR)
    find_path(ICONV_INCLUDE_DIR iconv.h
      "$ENV{ICONV_INCLUDE}"
      ${CMAKE_SYSTEM_INCLUDE_PATH}
      /usr/include
      /usr/local/include
      NO_DEFAULT_PATH
    )
  endif()
  # find libiconv
  if(NOT DEFINED ICONV_LIBRARY)
    find_library(ICONV_LIBRARY NAMES iconv libiconv PATHS
      "$ENV{ICONV_LIBRARY}"
      ${CMAKE_SYSTEM_LIBRARY_PATH}
      /usr/lib
      /usr/local/lib
      NO_DEFAULT_PATH
    )
  endif()
  # if found iconv.h and libiconv then we can use -liconv option
  if(ICONV_INCLUDE_DIR AND ICONV_LIBRARY)
    set(CAN_LINK_ICONV TRUE)
  else()
    set(CAN_LINK_ICONV FALSE)
  endif()
  # `ldconfig -p | grep libiconv.so`
  if(NOT CAN_LINK_ICONV)
    # execute `ldconfig -p`
    execute_process(
      COMMAND ldconfig -p
      OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/check_for_link_iconv.tmp"
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # execute `grep libiconv < (result file of `ldconfig -p`)`
    execute_process(
      COMMAND grep libiconv.so
      INPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/check_for_link_iconv.tmp"
      OUTPUT_VARIABLE check_for_link_iconv_tmp
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT ("${check_for_link_iconv_tmp}" STREQUAL ""))
      set(CAN_LINK_ICONV TRUE)
    endif()
  endif()
else()
  set(CAN_LINK_ICONV FALSE)
endif()
endmacro()
endif()
