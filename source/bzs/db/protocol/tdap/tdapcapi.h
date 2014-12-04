#ifndef BZS_DB_PROTOCOL_TDAP_TDAPCAPI_H
#define BZS_DB_PROTOCOL_TDAP_TDAPCAPI_H
/* =================================================================
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
 ================================================================= */
#include <bzs/env/compiler.h>
#include <tchar.h>

/** data types
 */
typedef unsigned int   uint_td;
typedef unsigned short ushort_td;
typedef void           void_td;
typedef short          short_td;
typedef unsigned char  uchar_td;
typedef char           char_td;
typedef uint_td        bookmark_td;
typedef int            percentage_td;

typedef ushort_td      keylen_td;

/** tdap c interface
 */
#if (defined(__BORLANDC__) && !defined(__clang__))
typedef short __stdcall (*dllUnloadCallback)();
#else
/** @cond INTERNAL */
/** Callback function on a record was deleted. */
typedef short(__STDCALL* dllUnloadCallback)();
/** @endcond */
#endif

#ifdef LIB_TDCLCPP
extern __declspec(dllimport) short_td
    __stdcall BTRCALLID(ushort_td op, void* posb, void* data, uint_td* datalen,
                        void* keybuf, keylen_td keylen, char_td keyNum,
                        uchar_td* clientID);

extern __declspec(dllimport) short_td
    __stdcall CallbackRegist(dllUnloadCallback func);
#endif

typedef short_td(__STDCALL* BTRCALLID_PTR)(ushort_td, void*, void*, uint_td*,
                                           void*, keylen_td, char_td,
                                           uchar_td*);

typedef short_td(__STDCALL* DLLUNLOADCALLBACK_PTR)(dllUnloadCallback func);

/** buffer size
 */
#define POS_BLOCK_SIZE                  128
#ifndef MAX_KEYLEN
#define MAX_KEYLEN                      0X3FF // 1023
#endif

/** operation type
 *
 */
#define TD_OPENTABLE                    0
#define TD_CLOSETABLE                   1
#define TD_REC_INSERT                   2
#define TD_REC_UPDATE                   3
#define TD_REC_DELETE                   4
#define TD_KEY_SEEK                     5
#define TD_KEY_NEXT                     6
#define TD_KEY_PREV                     7
#define TD_KEY_AFTER                    8
#define TD_KEY_OR_AFTER                 9
#define TD_KEY_BEFORE                   10
#define TD_KEY_OR_BEFORE                11
#define TD_KEY_FIRST                    12
#define TD_KEY_LAST                     13
#define TD_CREATETABLE                  14
#define TD_TABLE_INFO                   15
#define TD_SETDIRECTORY                 17
#define TD_GETDIRECTORY                 18
#define TD_BEGIN_TRANSACTION            19
#define TD_END_TRANSACTION              20
#define TD_ABORT_TRANSACTION            21
#define TD_BOOKMARK                     22
#define TD_MOVE_BOOKMARK                23
#define TD_POS_NEXT                     24
#define TD_STOP_ENGINE                  25
#define TD_VERSION                      26
#define TD_UNLOCK                       27
#define TD_RESET_CLIENT                 28
#define TD_SET_OWNERNAME                29
#define TD_CLEAR_OWNERNAME              30
#define TD_BUILD_INDEX                  31
#define TD_DROP_INDEX                   32
#define TD_POS_FIRST                    33
#define TD_POS_LAST                     34
#define TD_POS_PREV                     35
#define TD_KEY_NEXT_MULTI               36
#define TD_KEY_PREV_MULTI               37
#define TD_POS_NEXT_MULTI               38
#define TD_POS_PREV_MULTI               39
#define TD_INSERT_BULK                  40
#define TD_BACKUPMODE                   42
#define TD_MOVE_PER                     44
#define TD_GET_PER                      45
#define TD_UPDATE_PART                  53
#define TD_KEY_EQUAL_KO                 55
#define TD_KEY_NEXT_KO                  56
#define TD_KEY_PREV_KO                  57
#define TD_KEY_GT_KO                    58
#define TD_KEY_GE_KO                    59
#define TD_KEY_LT_KO                    60
#define TD_KEY_LE_KO                    61
#define TD_KEY_FIRST_KO                 62
#define TD_KEY_LAST_KO                  63
#define TD_CREATE_TEMP                  64
#define TD_TABLE_INFO_EX                65
#define TD_REC_UPDATEATKEY              70
#define TD_REC_DELLETEATKEY             71
#define TD_KEY_GE_NEXT_MULTI            72
#define TD_KEY_LE_PREV_MULTI            73
#define TD_FILTER_PREPARE               74
#define TD_CONNECT                      78
#define TD_BEGIN_SHAPSHOT               88
#define TD_END_SNAPSHOT                 89
#define TD_AUTOMEKE_SCHEMA              90
#define TD_GETSERVER_CHARSET            91
#define TD_ADD_SENDBLOB                 92
#define TD_GET_BLOB_BUF                 93
#define TD_STASTISTICS                  94
#define TD_KEY_SEEK_MULTI               95

/** create sub operations
 */
#define CR_SUBOP_DROP                   -128
#define CR_SUBOP_RENAME                 -127
#define CR_SUBOP_SWAPNAME               -126
#define CR_SUBOP_BY_FILESPEC            -1
#define CR_SUBOP_BY_FILESPEC_NOCKECK    0
#define CR_SUBOP_BY_TABLEDEF            1
#define CR_SUBOP_BY_TABLEDEF_NOCKECK    2

#define CR_SUB_FLAG_EXISTCHECK          -1

/** TD_ADD_SENDBLOB  sub operations
 */
#define TD_ASBLOB_ENDROW                -125

/** TD_GET_STASTISTICS sub operations
 */
#define TD_STSTCS_READ                  0
#define TD_STSTCS_DISCONNECT_ONE        1
#define TD_STSTCS_DISCONNECT_ALL        2

/** connect sub operation
 */

#define LG_SUBOP_CONNECT                0
#define LG_SUBOP_DISCONNECT             1
#define LG_SUBOP_NEWCONNECT             3


/** field types
 */
#define ft_string                       0
#define ft_integer                      1
#define ft_float                        2
#define ft_date                         3
#define ft_time                         4
#define ft_decimal                      5
#define ft_money                        6
#define ft_logical                      7
#define ft_numeric                      8
#define ft_bfloat                       9
#define ft_lstring                      10
#define ft_zstring                      11
#define ft_note                         12
#define ft_lvar                         13
#define ft_uinteger                     14
#define ft_autoinc                      15
#define ft_bit                          16
#define ft_numericsts                   17
#define ft_numericsa                    18
#define ft_currency                     19
#define ft_timestamp                    20
#define ft_blob                         21
#define ft_reserve22                    22
#define ft_reserve23                    23
#define ft_reserve24                    24
#define ft_wstring                      25
#define ft_wzstring                     26
#define ft_guid                         27
#define ft_datetime                     30
#define ft_myvarchar                    40
#define ft_myvarbinary                  41
#define ft_mywvarchar                   42
#define ft_mywvarbinary                 43
#define ft_mychar                       44
#define ft_mywchar                      45
#define ft_mydate                       46
#define ft_mytime                       47
#define ft_mydatetime                   48
#define ft_mytimestamp                  49
#define ft_mytext                       50
#define ft_myblob                       51
#define ft_autoIncUnsigned              52
#define ft_myfixedbinary                53

#define ft_nullindicator                255

/** charset type number
 */
#define charset_none                    0
#define charset_latin1                  1
#define charset_ascii                   2
#define charset_sjis                    3
#define charset_cp932                   4

#define charset_utf8                    100
#define charset_utf8mb4                 101
#define charset_usc2                    102

/** extruct row comp bias
 */
// In the case of a var type, it is copare as whole length. 
#define CMPLOGICAL_VAR_COMP_ALL         16
#define CMPLOGICAL_CMPACS               32 // no support
// The field for comparison shows not a value but a field number.
#define CMPLOGICAL_FIELD                64
#define CMPLOGICAL_CASEINSENSITIVE      128 // not case-sensitive


/** btrv transaction lock options
 */
#define LOCK_SINGLE_WAIT                100
#define LOCK_SINGLE_NOWAIT              200
#define LOCK_MULTI_WAIT                 300
#define LOCK_MULTI_NOWAIT               400

#define NOWAIT_WRITE                    500
#define PARALLEL_TRN                    1000


/** open mode
 */
#define TD_OPEN_NORMAL                  0
#define TD_OPEN_READONLY                -2
#define TD_OPEN_EXCLUSIVE               -4

/** filed algin
 */
#define BT_AL_LEFT                      0
#define BT_AL_CENTER                    2
#define BT_AL_RIGHT                     1


/** error code
 */
#define STATUS_TABLE_YET_OPEN           -3
#define STATUS_DURING_TRANSACTION       -4
#define STATUS_NO_ACR_UPDATE_DELETE     -5
#define STATUS_NO_ACR_INSERT            -6
#define STATUS_NO_ACR_READ              -7
#define STATUS_CANT_ALLOC_MEMORY        -8
#define STATUS_USE_KEYFIELD             -9
#define STATUS_TOO_MANY_TABLES          -10
#define STATUS_INVARID_PRM_KEY_NUM      -11
#define STATUS_INVARID_PNT_KEY_NUM      -12
#define STATUS_INVARID_REP_KEY_NUM      -13
#define STATUS_INVARID_FIELD_IDX        -14
#define STATUS_ALREADY_DELETED          -15
#define STATUS_LMITS_MAX_TABLES         -16
#define STATUS_DB_YET_OPEN              -17
#define STATUS_TABLENAME_NOTFOUND       -18
#define STATUS_DIFFERENT_DBVERSION      -19
#define STATUS_DUPLICATE_FIELDNAME      -20
#define STATUS_INVALID_TABLE_IDX        -21
#define STATUS_AUTH_DENIED              -22
#define STATUS_TOO_MANY_FIELDS          -23
#define STATUS_FILTERSTRING_ERROR       -24
#define STATUS_INVALID_FIELDLENGTH      -25
#define STATUS_INVALID_KEYTYPE          -26
#define STATUS_LVAR_NOTE_NOT_LAST       -27
#define STATUS_NODEF_FOR_CONVERT        -28
#define STATUS_TRD_NEED_VARLENGTH       -29
#define STATUS_INVALID_VARIABLETABLE    -30
#define STATUS_AUTOINC_SPACE_ERROR      -31
#define STATUS_TOO_LONG_OWNERNAME       -32
#define STATUS_CANT_DEL_FOR_REL         -33
#define STATUS_NO_AUTOINC_SPACE         -34
#define STATUS_INVALID_RECLEN           -35
#define STATUS_INVALID_FIELDVALUE       -36
#define STATUS_INVALID_VALLEN           -37
#define STATUS_FIELDTYPE_NOTSUPPORT     -42


#define STATUS_SUCCESS                  0
#define STATUS_PROGRAM_ERROR            1
#define STATUS_IO_ERROR                 2
#define STATUS_FILE_NOT_OPENED          3
#define STATUS_NOT_FOUND_TI             4
#define STATUS_DUPPLICATE_KEYVALUE      5
#define STATUS_INVALID_KEYNUM           6
#define STATUS_NO_CURRENT               8
#define STATUS_EOF                      9
#define STATUS_TABLE_NOTOPEN            12
#define STATUS_REQUESTER_DEACTIVE       20
#define STATUS_KEYBUFFERTOOSMALL        21
#define STATUS_BUFFERTOOSMALL           22
#define STATUS_CANT_CREATE              25
#define STATUS_NOSUPPORT_OP             41
#define STATUS_INVALID_BOOKMARK         43
#define STATUS_ACCESS_DENIED            46
#define STATUS_INVALID_OWNERNAME        51
#define STATUS_TABLE_EXISTS_ERROR       59
#define STATUS_LIMMIT_OF_REJECT         60
#define STATUS_WARKSPACE_TOO_SMALL      61
#define STATUS_INVALID_PREPAREID        62
#define STATUS_LMIT_OF_PREPAREED        63
#define STATUS_REACHED_FILTER_COND      64
#define STATUS_INVALID_FIELD_OFFSET     65
#define STATUS_CHANGE_CONFLICT          80
#define STATUS_INVALID_LOCKTYPE         83
#define STATUS_LOCK_ERROR               84
#define STATUS_FILE_LOCKED              85
#define STATUS_INVALID_SUPPLYVALUES     86
#define STATUS_CANNOT_LOCK_TABLE        88
#define STATUS_INVALID_KEYNAME          STATUS_INVALID_KEYNUM
#define STATUS_INVALID_DATASIZE         STATUS_BUFFERTOOSMALL
#define STATUS_INVALID_FIELDNAME        STATUS_INVALID_FIELD_OFFSET
#define ERROR_TD_INVALID_CLINETHOST     171
#define ERROR_NOSPECIFY_TABLE           176
#define ERROR_LOAD_CLIBRARY             200
#define ERROR_INDEX_RND_INIT            201
#define SERVER_CLIENT_NOT_COMPATIBLE    3003
#define NET_BAD_SRB_FORMAT              3021
#define ERROR_TD_HOSTNAME_NOT_FOUND     3103
#define ERROR_TD_CONNECTION_FAILURE     3106
#define ERROR_TD_NOT_CONNECTED          3110

#define TRANSACTD_SCHEMANAME            _T("transactd_schema")
#define TYPE_SCHEMA_BDF                 0
#define TYPE_SCHEMA_DDF                 1

#define FILTER_CURRENT_TYPE_NOTINC      0
#define FILTER_CURRENT_TYPE_INC         1
#define FILTER_CURRENT_TYPE_NOBOOKMARK  2
#define FILTER_TYPE_SUPPLYVALUE         4
#define FILTER_TYPE_FORWORD             4 //at preparing only 
#define FILTER_TYPE_SEEKS               8


/* No need export for client */
#define FILTER_COMBINE_NOPREPARE        0
#define FILTER_COMBINE_PREPARE          32


#define NIS_FILED_NAME                  "$nf"

/** max ownwr name size + 1 */
#define OWNERNAME_SIZE                  12

#define TD_BACKUP_START                 0
#define TD_BACKUP_END                   2
#define TD_BACKUP_MODE_OK               STATUS_SUCCESS
#define TD_BACKUP_MODE_NOT_SUPPORT      STATUS_PROGRAM_ERROR
#define TD_BACKUP_MODE_BUSY             STATUS_CANNOT_LOCK_TABLE
#define TD_BACKUP_MODE_NOT_PERMIT       41
#define TD_BACKUP_MODE_SERVER_ERROR     91


/** @cond INTERNAL */
struct trdVersiton
{
    char cherserServer[128];
    ushort_td clMajor;
    ushort_td clMinor;
    ushort_td clRelease;
    ushort_td srvMajor;
    ushort_td srvMinor;
    ushort_td srvRelease;
};
/** @endcond */

/* In the case of "tdclcppxxx" library of msvc, The ($TargetName) is not changed automatically.
 If you change this version then you need change The ($TargetName) project options too.
 */
#define C_INTERFACE_VER_MAJOR "2"//##1 Build marker! Don't remove
#define C_INTERFACE_VER_MINOR "1"//##2 Build marker! Don't remove
#define C_INTERFACE_VER_RELEASE "0"//##3 Build marker! Don't remove

/* dnamic load library name.
 The default extention of Mac is ".boudle", Therefore ".so" is popular. */
#ifdef LINUX
#ifdef __APPLE__
#define C_INTERFACE_VERSTR                                                     \
    "." C_INTERFACE_VER_MAJOR "." C_INTERFACE_VER_MINOR ".so" // use loadlibrary
#else // NOT __APPLE__
#define C_INTERFACE_VERSTR                                                     \
    ".so." C_INTERFACE_VER_MAJOR "." C_INTERFACE_VER_MINOR // use loadlibrary
#endif // NOT __APPLE__
#else // NOT LINUX
#define C_INTERFACE_VERSTR                                                     \
    "_" C_INTERFACE_VER_MAJOR "_" C_INTERFACE_VER_MINOR ".dll" // use
// loadlibrary
#endif // NOT LINUX

#if (defined(__x86_64__) || (defined(LINUX) && !defined(__BORLANDC__)))
#define TDCLC_LIBNAME "tdclc_64" C_INTERFACE_VERSTR // use loadlibrary
#else //__x86_32__
#define TDCLC_LIBNAME "tdclc_32" C_INTERFACE_VERSTR // use loadlibrary
#endif //__x86_32__

/* Cpp library name prefix */
#define TD_CPP_LIB_PRE "tdclcpp"

#if ((defined(_MSC_VER) && defined(_DLL)) ||                                   \
     (defined(__BORLANDC__) && defined(_RTLDLL)))
#define RTL_PART "r"
#else
#define RTL_PART
#endif

/* Cpp library name middle part */
#if (defined(__x86_64__) || (defined(LINUX) && !defined(__BORLANDC__)))
#ifdef _UNICODE
#define TD_LIB_PART "64u" RTL_PART
#else // NOT _UNICODE
#define TD_LIB_PART "64m" RTL_PART
#endif // NOT _UNICODE
#else //__x86_32__
#ifdef _UNICODE
#define TD_LIB_PART "32u" RTL_PART
#else // NOT _UNICODE
#define TD_LIB_PART "32m" RTL_PART
#endif // NOT _UNICODE
#endif //__x86_32__

/* Cpp library name version part
 In the case of "tdclcppxxx" library of msvc , The ($TargetName) is not changed
 automatically.
 If you change this version then you need change The ($TargetName) project
 options too.

 In the case of "tdclcppxxx" library of gcc , The -soname option is not changed
 automatically.
 If you change this version then you need change The -soname option project
 options too.

 */

#define CPP_INTERFACE_VER_MAJOR "2"//##4 Build marker! Don't remove
#define CPP_INTERFACE_VER_MINOR "1"//##5 Build marker! Don't remove
#define CPP_INTERFACE_VER_RELEASE "0"//##6 Build marker! Don't remove

/* use autolink tdclcpp */
#if (__BCPLUSPLUS__ || _MSC_VER)
#ifdef __APPLE__
#define CPP_INTERFACE_VERSTR                                                   \
    "_" COMPILER_VERSTR "_" TD_LIB_PART "." CPP_INTERFACE_VER_MAJOR            \
    "." CPP_INTERFACE_VER_MINOR
#else
#define CPP_INTERFACE_VERSTR                                                   \
    "_" COMPILER_VERSTR "_" TD_LIB_PART "_" CPP_INTERFACE_VER_MAJOR            \
    "_" CPP_INTERFACE_VER_MINOR
#endif
#endif

#define TRANSACTD_VER_MAJOR 2//##7 Build marker! Don't remove
#define TRANSACTD_VER_MINOR 1//##8 Build marker! Don't remove
#define TRANSACTD_VER_RELEASE 0//##9 Build marker! Don't remove

#endif // BZS_DB_PROTOCOL_TDAP_TDAPCAPI_H
