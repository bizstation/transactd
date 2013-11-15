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
# get_boost_libs function
#     Find Boost pathes for /MD(d) or /MT(d) on Visual Studio
# example:
#   set(Boost_USE_MULTITHREADED ON)
#   set(Boost_USE_STATIC_LIBS ON)
#   get_boost_libs("system;chrono")
#   message(STATUS "for /MD  ${Boost_LIBRARIES_FOR_MD}")  # =Boost_LIBRARIES_STATIC_RUNTIME_OFF_NO_DEBUG
#   message(STATUS "for /MDd ${Boost_LIBRARIES_FOR_MDd}") # =Boost_LIBRARIES_STATIC_RUNTIME_OFF_DEBUG
#   message(STATUS "for /MT  ${Boost_LIBRARIES_FOR_MT}")  # =Boost_LIBRARIES_STATIC_RUNTIME_ON_NO_DEBUG
#   message(STATUS "for /MTd ${Boost_LIBRARIES_FOR_MTd}") # =Boost_LIBRARIES_STATIC_RUNTIME_ON_DEBUG
#   get_boost_libs_from_CXXFLAGS("${CMAKE_CXX_FLAGS_DEBUG}")
#   message(STATUS "for DEBUG BUILD ${get_boost_libs_from_CXXFLAGS_return}")
function(get_boost_libs libs)
  # Get pathes STATIC_RUNTIME_ON
  set(Boost_USE_STATIC_RUNTIME ON)
  find_package(Boost COMPONENTS ${libs})
  if(NOT Boost_FOUND)
    message(SEND_ERROR "Boost not found. ${libs}")
  endif()
  set(Boost_LIBRARIES_STATIC_RUNTIME_ON  ${Boost_LIBRARIES})
  set(Boost_LIBRARIES "")
  # Get pathes STATIC_RUNTIME_OFF
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(Boost COMPONENTS ${libs})
  if(NOT Boost_FOUND)
    message(SEND_ERROR "Boost not found. ${libs}")
  endif()
  set(Boost_LIBRARIES_STATIC_RUNTIME_OFF ${Boost_LIBRARIES})
  set(Boost_LIBRARIES "")
  # for /MD
  parse_boost_libs(false "${Boost_LIBRARIES_STATIC_RUNTIME_OFF}")
  set(Boost_LIBRARIES_FOR_MD "${parse_boost_libs_return}" PARENT_SCOPE)
  set(Boost_LIBRARIES_STATIC_RUNTIME_OFF_NO_DEBUG "${parse_boost_libs_return}" PARENT_SCOPE)
  # for /MDd
  parse_boost_libs(true  "${Boost_LIBRARIES_STATIC_RUNTIME_OFF}")
  set(Boost_LIBRARIES_FOR_MDd "${parse_boost_libs_return}" PARENT_SCOPE)
  set(Boost_LIBRARIES_STATIC_RUNTIME_OFF_DEBUG "${parse_boost_libs_return}" PARENT_SCOPE)
  # for /MT
  parse_boost_libs(false "${Boost_LIBRARIES_STATIC_RUNTIME_ON}")
  set(Boost_LIBRARIES_FOR_MT "${parse_boost_libs_return}" PARENT_SCOPE)
  set(Boost_LIBRARIES_STATIC_RUNTIME_ON_NO_DEBUG "${parse_boost_libs_return}" PARENT_SCOPE)
  # for /MTd
  parse_boost_libs(true  "${Boost_LIBRARIES_STATIC_RUNTIME_ON}")
  set(Boost_LIBRARIES_FOR_MTd "${parse_boost_libs_return}" PARENT_SCOPE)
  set(Boost_LIBRARIES_STATIC_RUNTIME_ON_DEBUG "${parse_boost_libs_return}" PARENT_SCOPE)
  # Boost_INCLUDE_DIRS
  set(Boost_INCLUDE_DIRS "${Boost_INCLUDE_DIRS}" PARENT_SCOPE)
endfunction()

function(get_boost_libs_from_CXXFLAGS flags)
  set(get_boost_libs_from_CXXFLAGS_return "" PARENT_SCOPE)
  set(Boost_LIBRARIES_BUILD_TYPE "")
  if("${flags}" MATCHES "(.* )?/MD( .*)?$")
    set(Boost_LIBRARIES_BUILD_TYPE "${Boost_LIBRARIES_FOR_MD}")
  elseif("${flags}" MATCHES "(.* )?/MDd( .*)?$")
    set(Boost_LIBRARIES_BUILD_TYPE "${Boost_LIBRARIES_FOR_MDd}")
  elseif("${flags}" MATCHES "(.* )?/MT( .*)?$")
    set(Boost_LIBRARIES_BUILD_TYPE "${Boost_LIBRARIES_FOR_MT}")
  elseif("${flags}" MATCHES "(.* )?/MTd( .*)?$")
    set(Boost_LIBRARIES_BUILD_TYPE "${Boost_LIBRARIES_FOR_MTd}")
  else()
    set(Boost_LIBRARIES_BUILD_TYPE "${Boost_LIBRARIES_FOR_MD}")
  endif()
  set(get_boost_libs_from_CXXFLAGS_return "${Boost_LIBRARIES_BUILD_TYPE}" PARENT_SCOPE)
endfunction()

function(parse_boost_libs with_debug path)
  set(parse_boost_libs_return "" PARENT_SCOPE)
  set(add_pathes_list OFF)
  set(pathes "")
  foreach(item ${path})
    if("${item}" MATCHES "debug")
      if(with_debug)
        set(add_pathes_list ON)
      endif()
    elseif("${item}" MATCHES "^optimized$")
      if(NOT with_debug)
        set(add_pathes_list ON)
      endif()
    else()
      if(add_pathes_list)
        set(pathes "${pathes}${item};")
      endif()
      set(add_pathes_list OFF)
    endif()
  endforeach()
  set(parse_boost_libs_return "${pathes}" PARENT_SCOPE)
endfunction()
