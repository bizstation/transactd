# From http://code.google.com/p/esteid/ 
# Used under the LGPL 


# - FindPHPLibs
# Find PHP interpreter includes and library
#
#  PHPLIBS_FOUND      - True if PHP libs found
#  PHP_VERSION_STRING - The version of PHP found (x.y.z)
#  PHP_LIBRARIES      - Libaries (standard variable)
#  PHP_INCLUDE_DIRS   - List of include directories
#  PHP_INCLUDE_DIR    - Main include directory prefix
#  PHP_EXTENSION_DIR  - Location of PHP extension DSO-s
#  PHP_EXECUTABLE     - PHP executable
#  PHP_INSTALL_PREFIX - PHP install prefix, as reported
INCLUDE(CMakeFindFrameworks)

IF( NOT PHP_CONFIG_EXECUTABLE )

FIND_PROGRAM(PHP_CONFIG_EXECUTABLE
  NAMES php5-config php-config
  )
ENDIF( NOT PHP_CONFIG_EXECUTABLE )

MACRO(GET_FROM_PHP_CONFIG args variable)
  EXECUTE_PROCESS(COMMAND ${PHP_CONFIG_EXECUTABLE} ${args}
    OUTPUT_VARIABLE ${variable})
 STRING(REPLACE "\n" "" ${variable} "${${variable}}")
ENDMACRO(GET_FROM_PHP_CONFIG cmd variable)

IF(PHP_CONFIG_EXECUTABLE)
  GET_FROM_PHP_CONFIG("--version"       PHP_VERSION_STRING)
  GET_FROM_PHP_CONFIG("--php-binary"    PHP_EXECUTABLE)
  GET_FROM_PHP_CONFIG("--include-dir"   PHP_INCLUDE_DIR)
  GET_FROM_PHP_CONFIG("--extension-dir" PHP_EXTENSION_DIR)
  GET_FROM_PHP_CONFIG("--includes"      PHP_INCLUDE_DIRS)
  GET_FROM_PHP_CONFIG("--prefix"        PHP_INSTALL_PREFIX)
  GET_FROM_PHP_CONFIG("--configure-options" PHP_ZTS)
  STRING(REPLACE "-I" "" PHP_INCLUDE_DIRS "${PHP_INCLUDE_DIRS}")
  STRING(REPLACE " " ";" PHP_INCLUDE_DIRS "${PHP_INCLUDE_DIRS}")
  if(${PHP_ZTS} MATCHES ".*--enable-maintainer-zts")
     set(PHP_ZTS "ON")
  else()
     set(PHP_ZTS "OFF")
  endif()
ENDIF(PHP_CONFIG_EXECUTABLE)

# Find PHP include path by php -i

IF(PHP_EXECUTABLE)
  MACRO(GET_FROM_PHP_INFO args variable)
    EXECUTE_PROCESS(COMMAND ${PHP_EXECUTABLE} ${args}
      OUTPUT_VARIABLE ${variable})
   STRING(REPLACE "\n" "" ${variable} "${${variable}}")

  ENDMACRO(GET_FROM_PHP_INFO cmd variable)


  GET_FROM_PHP_INFO("-i" PHP_INFO_INCLUDE)
  if (WIN32)
    string(REGEX MATCH "include_path \\=\\> \\.\\;([^\\=]*) \\=\\>" PHP_INFO_INCLUDE ${PHP_INFO_INCLUDE})
  else()
    string(REGEX MATCH "include_path \\=\\> \\.\\:([^\\=]*) \\=\\>" PHP_INFO_INCLUDE ${PHP_INFO_INCLUDE})
  endif()
  set(PHP_INFO_INCLUDE ${CMAKE_MATCH_1})
  string(FIND ${PHP_INFO_INCLUDE} : POS1)
  if (NOT (${POS1} EQUAL -1))
	string(SUBSTRING ${PHP_INFO_INCLUDE} 0 ${POS1} PHP_INFO_INCLUDE)
  endif()

#  Is transactd.so enabled in php.ini ?

#   get php.ini file name
  MACRO(GET_FROM_PHP_INFO2 args variable)
    EXECUTE_PROCESS(COMMAND ${PHP_EXECUTABLE} ${args}
      OUTPUT_VARIABLE ${variable})
  ENDMACRO(GET_FROM_PHP_INFO2 cmd variable)

  GET_FROM_PHP_INFO2("-i" PHP_INFO_FILE)
  if (WIN32)
    string(REGEX MATCH "Loaded Configuration File \\=\\> ([^ ]*)\\\n" PHP_INFO_FILE ${PHP_INFO_FILE})
  else()
    string(REGEX MATCH "Loaded Configuration File \\=\\> ([^ ]*)\\\n" PHP_INFO_FILE ${PHP_INFO_FILE})
  endif()
  set(PHP_INFO_FILE ${CMAKE_MATCH_1})
  if(${PHP_INFO_FILE} STREQUAL \(none\))
    set(PHP_INFO_FILE "/etc/php.ini")
  endif()

# Read transactd extension info from php.ini
  execute_process(COMMAND grep -i transactd ${PHP_INFO_FILE}
    OUTPUT_VARIABLE PHP_TRANSACTD_ENABLED)
  if (${PHP_TRANSACTD_ENABLED} MATCHES "php_transactd.so")
    set(PHP_TRANSACTD_ENABLED "YES")
  else()
    set(PHP_TRANSACTD_ENABLED "NO")
  endif()
ENDIF(PHP_EXECUTABLE)


# FIXME: Maybe we need all this crap that php-config --libs puts out,
#        however after building a few swig bindings without them,
#        I seriously doubt it.
SET(PHP_LIBRARIES "")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PHPLibs DEFAULT_MSG PHP_INCLUDE_DIRS)

MARK_AS_ADVANCED(PHP_LIBRARIES PHP_INCLUDE_DIRS) 
