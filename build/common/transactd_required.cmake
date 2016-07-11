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
# require 2.8.3 on windows
# ==========================================================
if(WIN32)
  set(tmp_verstr "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
  if ("2.8.3" GREATER "${tmp_verstr}")
    message(SEND_ERROR "CMake 2.8.3 or higher is required.  You are running version ${tmp_verstr}")
  endif()
  include(CMakeParseArguments)
endif()

# set policies
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
  cmake_policy(SET CMP0054 NEW)
endif()
