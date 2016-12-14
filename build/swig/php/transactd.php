<?php
/* =================================================================
 Copyright (C) 2014-2016 BizStation Corp All rights reserved.

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
namespace BizStation\Transactd;

// Try to load our extension if it's not already loaded.
if (!extension_loaded('transactd')) {
    if (strtolower(substr(PHP_OS, 0, 3)) === 'win') {
        if (!dl('php_transactd.dll')) {
            return;
        }
    } else {
        // PHP_SHLIB_SUFFIX gives 'dylib' on MacOS X but modules are 'so'.
    if (PHP_SHLIB_SUFFIX === 'dylib') {
        if (!dl('transactd.so')) {
            return;
        }
    } else {
        if (!dl('transactd.'.PHP_SHLIB_SUFFIX)) {
            return;
        }
    }
    }
}

abstract class Transactd
{
    const MYSQL_TYPE_MYSQL = MYSQL_TYPE_MYSQL;

    const MYSQL_TYPE_MARIA = MYSQL_TYPE_MARIA;

    const CP_ACP = CP_ACP;

    const CP_UTF8 = CP_UTF8;

    const ft_string = ft_string;

    const ft_integer = ft_integer;

    const ft_float = ft_float;

    const ft_date = ft_date;

    const ft_time = ft_time;

    const ft_decimal = ft_decimal;

    const ft_money = ft_money;

    const ft_logical = ft_logical;

    const ft_numeric = ft_numeric;

    const ft_bfloat = ft_bfloat;

    const ft_lstring = ft_lstring;

    const ft_zstring = ft_zstring;

    const ft_note = ft_note;

    const ft_lvar = ft_lvar;

    const ft_uinteger = ft_uinteger;

    const ft_autoinc = ft_autoinc;

    const ft_bit = ft_bit;

    const ft_numericsts = ft_numericsts;

    const ft_numericsa = ft_numericsa;

    const ft_currency = ft_currency;

    const ft_timestamp = ft_timestamp;

    const ft_blob = ft_blob;

    const ft_reserve22 = ft_reserve22;

    const ft_reserve23 = ft_reserve23;

    const ft_reserve24 = ft_reserve24;

    const ft_wstring = ft_wstring;

    const ft_wzstring = ft_wzstring;

    const ft_guid = ft_guid;

    const ft_datetime = ft_datetime;

    const ft_myvarchar = ft_myvarchar;

    const ft_myvarbinary = ft_myvarbinary;

    const ft_mywvarchar = ft_mywvarchar;

    const ft_mywvarbinary = ft_mywvarbinary;

    const ft_mychar = ft_mychar;

    const ft_mywchar = ft_mywchar;

    const ft_mydate = ft_mydate;

    const ft_mytime = ft_mytime;

    const ft_mydatetime = ft_mydatetime;

    const ft_mytimestamp = ft_mytimestamp;

    const ft_mytext = ft_mytext;

    const ft_myblob = ft_myblob;

    const ft_autoIncUnsigned = ft_autoIncUnsigned;

    const ft_myfixedbinary = ft_myfixedbinary;

    const ft_myjson = ft_myjson;

    const ft_mygeometry = ft_mygeometry;

    const ft_myyear = ft_myyear;
    
    const ft_mydecimal = ft_mydecimal;

    const ft_nullindicator = ft_nullindicator;

    const CMPLOGICAL_VAR_COMP_ALL = CMPLOGICAL_VAR_COMP_ALL;

    const CMPLOGICAL_CMPACS = CMPLOGICAL_CMPACS;

    const CMPLOGICAL_FIELD = CMPLOGICAL_FIELD;

    const CMPLOGICAL_CASEINSENSITIVE = CMPLOGICAL_CASEINSENSITIVE;

    const LOCK_BIAS_DEFAULT = LOCK_BIAS_DEFAULT;

    const LOCK_SINGLE_WAIT = LOCK_SINGLE_WAIT;

    const LOCK_SINGLE_NOWAIT = LOCK_SINGLE_NOWAIT;

    const LOCK_MULTI_WAIT = LOCK_MULTI_WAIT;

    const LOCK_MULTI_NOWAIT = LOCK_MULTI_NOWAIT;

    const NOWAIT_WRITE = NOWAIT_WRITE;

    const PARALLEL_TRN = PARALLEL_TRN;

    const TRN_ISO_READ_COMMITED = TRN_ISO_READ_COMMITED;

    const TRN_ISO_REPEATABLE_READ = TRN_ISO_REPEATABLE_READ;

    const TRN_ISO_SERIALIZABLE = TRN_ISO_SERIALIZABLE;

    const SINGLELOCK_READ_COMMITED = SINGLELOCK_READ_COMMITED;

    const MULTILOCK_READ_COMMITED = MULTILOCK_READ_COMMITED;

    const MULTILOCK_REPEATABLE_READ = MULTILOCK_REPEATABLE_READ;

    const MULTILOCK_ISO_SERIALIZABLE = MULTILOCK_ISO_SERIALIZABLE;

    const SINGLELOCK_NOGAP = SINGLELOCK_NOGAP;

    const MULTILOCK_NOGAP = MULTILOCK_NOGAP;

    const MULTILOCK_GAP = MULTILOCK_GAP;

    const CONSISTENT_READ = CONSISTENT_READ;

    const CONSISTENT_READ_WITH_BINLOG_POS = CONSISTENT_READ_WITH_BINLOG_POS;

    const MULTILOCK_GAP_SHARE = MULTILOCK_GAP_SHARE;

    const MULTILOCK_NOGAP_SHARE = MULTILOCK_NOGAP_SHARE;

    const REPL_POSTYPE_NONE = REPL_POSTYPE_NONE;

    const REPL_POSTYPE_MARIA_GTID = REPL_POSTYPE_MARIA_GTID;

    const REPL_POSTYPE_POS = REPL_POSTYPE_POS;

    const REPL_POSTYPE_GTID = REPL_POSTYPE_GTID;

    const ROW_LOCK_X = ROW_LOCK_X;

    const ROW_LOCK_S = ROW_LOCK_S;

    const SRV_ISO_READ_UNCOMMITED = SRV_ISO_READ_UNCOMMITED;

    const SRV_ISO_READ_COMMITED = SRV_ISO_READ_COMMITED;

    const SRV_ISO_REPEATABLE_READ = SRV_ISO_REPEATABLE_READ;

    const SRV_ISO_SERIALIZABLE = SRV_ISO_SERIALIZABLE;

    const TD_OPEN_NORMAL = TD_OPEN_NORMAL;

    const TD_OPEN_READONLY = TD_OPEN_READONLY;

    const TD_OPEN_EXCLUSIVE = TD_OPEN_EXCLUSIVE;
    
    const TD_OPEN_READONLY_EXCLUSIVE = TD_OPEN_READONLY_EXCLUSIVE;

    const BT_AL_LEFT = BT_AL_LEFT;

    const BT_AL_CENTER = BT_AL_CENTER;

    const BT_AL_RIGHT = BT_AL_RIGHT;

    const STATUS_TABLE_YET_OPEN = STATUS_TABLE_YET_OPEN;

    const STATUS_DURING_TRANSACTION = STATUS_DURING_TRANSACTION;

    const STATUS_NO_ACR_UPDATE_DELETE = STATUS_NO_ACR_UPDATE_DELETE;

    const STATUS_NO_ACR_INSERT = STATUS_NO_ACR_INSERT;

    const STATUS_NO_ACR_READ = STATUS_NO_ACR_READ;

    const STATUS_CANT_ALLOC_MEMORY = STATUS_CANT_ALLOC_MEMORY;

    const STATUS_USE_KEYFIELD = STATUS_USE_KEYFIELD;

    const STATUS_TOO_MANY_TABLES = STATUS_TOO_MANY_TABLES;

    const STATUS_INVARID_PRM_KEY_NUM = STATUS_INVARID_PRM_KEY_NUM;

    const STATUS_INVARID_PNT_KEY_NUM = STATUS_INVARID_PNT_KEY_NUM;

    const STATUS_INVARID_REP_KEY_NUM = STATUS_INVARID_REP_KEY_NUM;

    const STATUS_INVARID_FIELD_IDX = STATUS_INVARID_FIELD_IDX;

    const STATUS_ALREADY_DELETED = STATUS_ALREADY_DELETED;

    const STATUS_LMITS_MAX_TABLES = STATUS_LMITS_MAX_TABLES;

    const STATUS_DB_YET_OPEN = STATUS_DB_YET_OPEN;

    const STATUS_TABLENAME_NOTFOUND = STATUS_TABLENAME_NOTFOUND;

    const STATUS_DIFFERENT_DBVERSION = STATUS_DIFFERENT_DBVERSION;

    const STATUS_DUPLICATE_FIELDNAME = STATUS_DUPLICATE_FIELDNAME;

    const STATUS_INVALID_TABLE_IDX = STATUS_INVALID_TABLE_IDX;

    const STATUS_AUTH_DENIED = STATUS_AUTH_DENIED;

    const STATUS_TOO_MANY_FIELDS = STATUS_TOO_MANY_FIELDS;

    const STATUS_FILTERSTRING_ERROR = STATUS_FILTERSTRING_ERROR;

    const STATUS_INVALID_FIELDLENGTH = STATUS_INVALID_FIELDLENGTH;

    const STATUS_INVALID_KEYTYPE = STATUS_INVALID_KEYTYPE;

    const STATUS_LVAR_NOTE_NOT_LAST = STATUS_LVAR_NOTE_NOT_LAST;

    const STATUS_NODEF_FOR_CONVERT = STATUS_NODEF_FOR_CONVERT;

    const STATUS_TRD_NEED_VARLENGTH = STATUS_TRD_NEED_VARLENGTH;

    const STATUS_INVALID_VARIABLETABLE = STATUS_INVALID_VARIABLETABLE;

    const STATUS_AUTOINC_SPACE_ERROR = STATUS_AUTOINC_SPACE_ERROR;

    const STATUS_TOO_LONG_OWNERNAME = STATUS_TOO_LONG_OWNERNAME;

    const STATUS_CANT_DEL_FOR_REL = STATUS_CANT_DEL_FOR_REL;

    const STATUS_NO_AUTOINC_SPACE = STATUS_NO_AUTOINC_SPACE;

    const STATUS_INVALID_RECLEN = STATUS_INVALID_RECLEN;

    const STATUS_INVALID_FIELDVALUE = STATUS_INVALID_FIELDVALUE;

    const STATUS_INVALID_VALLEN = STATUS_INVALID_VALLEN;

    const STATUS_FIELDTYPE_NOTSUPPORT = STATUS_FIELDTYPE_NOTSUPPORT;

    const STATUS_INVALID_NULLMODE = STATUS_INVALID_NULLMODE;

    const STATUS_TOO_LARGE_VALUE = STATUS_TOO_LARGE_VALUE;

    const STATUS_SQL_PARSE_ERROR = STATUS_SQL_PARSE_ERROR;

    const STATUS_SUCCESS = STATUS_SUCCESS;

    const STATUS_PROGRAM_ERROR = STATUS_PROGRAM_ERROR;

    const STATUS_IO_ERROR = STATUS_IO_ERROR;

    const STATUS_FILE_NOT_OPENED = STATUS_FILE_NOT_OPENED;

    const STATUS_NOT_FOUND_TI = STATUS_NOT_FOUND_TI;

    const STATUS_DUPPLICATE_KEYVALUE = STATUS_DUPPLICATE_KEYVALUE;

    const STATUS_INVALID_KEYNUM = STATUS_INVALID_KEYNUM;

    const STATUS_NO_CURRENT = STATUS_NO_CURRENT;

    const STATUS_EOF = STATUS_EOF;

    const STATUS_TABLE_NOTOPEN = STATUS_TABLE_NOTOPEN;

    const STATUS_REQUESTER_DEACTIVE = STATUS_REQUESTER_DEACTIVE;

    const STATUS_KEYBUFFERTOOSMALL = STATUS_KEYBUFFERTOOSMALL;

    const STATUS_BUFFERTOOSMALL = STATUS_BUFFERTOOSMALL;

    const STATUS_CANT_CREATE = STATUS_CANT_CREATE;

    const STATUS_NOSUPPORT_OP = STATUS_NOSUPPORT_OP;

    const STATUS_INVALID_BOOKMARK = STATUS_INVALID_BOOKMARK;

    const STATUS_ACCESS_DENIED = STATUS_ACCESS_DENIED;

    const STATUS_INVALID_OWNERNAME = STATUS_INVALID_OWNERNAME;

    const STATUS_TABLE_EXISTS_ERROR = STATUS_TABLE_EXISTS_ERROR;

    const STATUS_LIMMIT_OF_REJECT = STATUS_LIMMIT_OF_REJECT;

    const STATUS_WARKSPACE_TOO_SMALL = STATUS_WARKSPACE_TOO_SMALL;

    const STATUS_REACHED_FILTER_COND = STATUS_REACHED_FILTER_COND;

    const STATUS_INVALID_FIELD_OFFSET = STATUS_INVALID_FIELD_OFFSET;

    const STATUS_CHANGE_CONFLICT = STATUS_CHANGE_CONFLICT;

    const STATUS_INVALID_LOCKTYPE = STATUS_INVALID_LOCKTYPE;

    const STATUS_LOCK_ERROR = STATUS_LOCK_ERROR;

    const STATUS_FILE_LOCKED = STATUS_FILE_LOCKED;

    const STATUS_CANNOT_LOCK_TABLE = STATUS_CANNOT_LOCK_TABLE;

    const STATUS_INVALID_KEYNAME = STATUS_INVALID_KEYNAME;

    const STATUS_INVALID_DATASIZE = STATUS_INVALID_DATASIZE;

    const STATUS_INVALID_FIELDNAME = STATUS_INVALID_FIELDNAME;

    const ERROR_TD_INVALID_CLINETHOST = ERROR_TD_INVALID_CLINETHOST;

    const ERROR_NO_DATABASE = ERROR_NO_DATABASE;

    const ERROR_NOSPECIFY_TABLE = ERROR_NOSPECIFY_TABLE;

    const ERROR_LOAD_CLIBRARY = ERROR_LOAD_CLIBRARY;

    const ERROR_INDEX_RND_INIT = ERROR_INDEX_RND_INIT;

    const STATUS_ALREADY_INSNAPSHOT = STATUS_ALREADY_INSNAPSHOT;

    const STATUS_ALREADY_INTRANSACTION = STATUS_ALREADY_INTRANSACTION;

    const SERVER_CLIENT_NOT_COMPATIBLE = SERVER_CLIENT_NOT_COMPATIBLE;

    const NET_BAD_SRB_FORMAT = NET_BAD_SRB_FORMAT;

    const ERROR_TD_HOSTNAME_NOT_FOUND = ERROR_TD_HOSTNAME_NOT_FOUND;

    const ERROR_TD_CONNECTION_FAILURE = ERROR_TD_CONNECTION_FAILURE;

    const ERROR_TD_NOT_CONNECTED = ERROR_TD_NOT_CONNECTED;

    const ERROR_TD_NET_TIMEOUT = ERROR_TD_NET_TIMEOUT;

    const ERROR_TD_NET_REMOTE_DISCONNECT = ERROR_TD_NET_REMOTE_DISCONNECT;

    const ERROR_TD_NET_TOO_BIGDATA = ERROR_TD_NET_TOO_BIGDATA;

    const ERROR_TD_NET_OTHER = ERROR_TD_NET_OTHER;

    const ERROR_TD_C_CLIENT_UNKNOWN = ERROR_TD_C_CLIENT_UNKNOWN;

    const ERROR_TD_INVALID_SERVER_ROLE = ERROR_TD_INVALID_SERVER_ROLE;

    const ERROR_TD_RECONNECTED = ERROR_TD_RECONNECTED;

    const ERROR_TD_RECONNECTED_OFFSET = ERROR_TD_RECONNECTED_OFFSET;

    const MYSQL_ERROR_OFFSET = MYSQL_ERROR_OFFSET;

    const TRANSACTD_SCHEMANAME = TRANSACTD_SCHEMANAME;

    const TYPE_SCHEMA_BDF = TYPE_SCHEMA_BDF;

    const TYPE_SCHEMA_DDF = TYPE_SCHEMA_DDF;

    const FILTER_CURRENT_TYPE_NOTINC = FILTER_CURRENT_TYPE_NOTINC;

    const FILTER_CURRENT_TYPE_INC = FILTER_CURRENT_TYPE_INC;

    const FILTER_CURRENT_TYPE_NOBOOKMARK = FILTER_CURRENT_TYPE_NOBOOKMARK;

    const NIS_FILED_NAME = NIS_FILED_NAME;

    const OWNERNAME_SIZE = OWNERNAME_SIZE;

    const TD_BACKUP_START = TD_BACKUP_START;

    const TD_BACKUP_END = TD_BACKUP_END;

    const TD_BACKUP_MODE_OK = TD_BACKUP_MODE_OK;

    const TD_BACKUP_MODE_NOT_SUPPORT = TD_BACKUP_MODE_NOT_SUPPORT;

    const TD_BACKUP_MODE_BUSY = TD_BACKUP_MODE_BUSY;

    const TD_BACKUP_MODE_NOT_PERMIT = TD_BACKUP_MODE_NOT_PERMIT;

    const TD_BACKUP_MODE_SERVER_ERROR = TD_BACKUP_MODE_SERVER_ERROR;

    const DFV_TIMESTAMP_DEFAULT = DFV_TIMESTAMP_DEFAULT;

    const HA_ROLE_SLAVE = HA_ROLE_SLAVE;

    const HA_ROLE_MASTER = HA_ROLE_MASTER;

    const HA_ROLE_NONE = HA_ROLE_NONE;

    const HA_RESTORE_ROLE = HA_RESTORE_ROLE;

    const HA_ENABLE_FAILOVER = HA_ENABLE_FAILOVER;

    const CPP_INTERFACE_VER_MAJOR = CPP_INTERFACE_VER_MAJOR;

    const CPP_INTERFACE_VER_MINOR = CPP_INTERFACE_VER_MINOR;

    const CPP_INTERFACE_VER_RELEASE = CPP_INTERFACE_VER_RELEASE;

    const TRANSACTD_VER_MAJOR = TRANSACTD_VER_MAJOR;

    const TRANSACTD_VER_MINOR = TRANSACTD_VER_MINOR;

    const TRANSACTD_VER_RELEASE = TRANSACTD_VER_RELEASE;

    const MAX_KEY_SEGMENT = MAX_KEY_SEGMENT;
    
    const TIMESTAMP_ALWAYS = TIMESTAMP_ALWAYS;
    
    const TIMESTAMP_VALUE_CONTROL = TIMESTAMP_VALUE_CONTROL;
    /**
     *
     * @param int $type Type number of field
     * @return string type name
     */
    public static function getTypeName($type)
    {
        return getTypeName($type);
    }
    /**
     *
     * @param int $type Type number of field
     * @return int alignment type
     */
    public static function getTypeAlign($type)
    {
        return getTypeAlign($type);
    }
    /**
     *
     * @param int $type Type number of field
     * @param int $charsetIndex Index of charset
     * @param type $charnum Number of max char
     * @return int field length(bytes)
     */
    public static function lenByCharnum($type, $charsetIndex, $charnum)
    {
        return lenByCharnum($type, $charsetIndex, $charnum);
    }
    /**
     *
     * @param int $type Type number of field
     * @return bool
     */
    public static function isStringType($type)
    {
        return isStringType($type);
    }

    const eCend = 0;

    const eCand = eCand;

    const eCor = eCor;

    const eEqual = 1;

    const eGreater = 2;

    const eLess = 3;

    const eNotEq = 4;

    const eGreaterEq = 5;

    const eLessEq = 6;

    const eBitAnd = 8;

    const eNotBitAnd = 9;

    const eIsNull = 10;

    const eIsNotNull = 11;

    const TABLE_NUM_TMP = TABLE_NUM_TMP;

    const eMinlen = 0;

    const eMaxlen = eMaxlen;

    const eDefaultlen = eDefaultlen;

    const eDecimals = eDecimals;

    const MAX_CHAR_INFO = MAX_CHAR_INFO;

    const CHARSET_LATIN1 = CHARSET_LATIN1;

    const CHARSET_CP850 = CHARSET_CP850;

    const CHARSET_ASCII = CHARSET_ASCII;

    const CHARSET_SJIS = CHARSET_SJIS;

    const CHARSET_UTF8 = CHARSET_UTF8;

    const CHARSET_USC2 = CHARSET_USC2;

    const CHARSET_UTF8B4 = CHARSET_UTF8B4;

    const CHARSET_UTF16LE = CHARSET_UTF16LE;

    const CHARSET_CP932 = CHARSET_CP932;

    const CHARSET_EUCJ = CHARSET_EUCJ;
    /**
     *
     * @param int $index Index of charset
     * @return int Size bytes of a character
     */
    public static function charsize($index)
    {
        return charsize($index);
    }
    /**
     *
     * @param int $index Index of charcter set
     * @return string Name of charcter set
     */
    public static function charsetName($index)
    {
        return charsetName($index);
    }
    /**
     *
     * @param mixed $name_or_codePage
     * @return int Index of charcter set
     */
    public static function charsetIndex($name_or_codePage)
    {
        return charsetIndex($name_or_codePage);
    }
    /**
     *
     * @param int $charsetIndex Index of charcter set
     * @return int Code page
     */
    public static function codePage($charsetIndex)
    {
        return codePage($charsetIndex);
    }
    /**
     *
     * @param string $date date of "yyyy-mm-dd"
     * @return \BizStation\Transactd\BtrDate
     */
    public static function atobtrd($date)
    {
        $r=atobtrd($date);
        if (is_resource($r)) {
            return new BtrDate($r);
        }
        return $r;
    }
    /**
     *
     * @param string $time Time of "hh:nn:ss.uu"
     * @return \BizStation\Transactd\BtrTime
     */
    public static function atobtrt($time)
    {
        $r=atobtrt($time);
        if (is_resource($r)) {
            return new BtrTime($r);
        }
        return $r;
    }
    /**
     *
     * @param int|\BizStation\Transactd\btrDate $d_or_date
     * @param bool $w3_format
     * @return string
     */
    public static function btrdtoa($d_or_date, $w3_format=false)
    {
        $r=btrdtoa($d_or_date, $w3_format);
        return $r;
    }
    /**
     *
     * @param int|\BizStation\Transactd\btrTime $t_or_time
     * @return string
     */
    public static function btrttoa($t_or_time)
    {
        $r=btrttoa($t_or_time);
        return $r;
    }
    /**
     *
     * @param \BizStation\Transactd\btrDateTime $d
     * @param bool $w3_format
     * @return string
     */
    public static function btrstoa($d, $w3_format=false)
    {
        return btrstoa($d, $w3_format);
    }
    /**
     *
     * @param string $p date tome string "yyyy-mm-dd hh:nn::ss"
     * @return \BizStation\Transactd\BtrDateTime
     */
    public static function atobtrs($p)
    {
        $r=atobtrs($p);
        if (is_resource($r)) {
            return new BtrDateTime($r);
        }
        return $r;
    }
    /**
     *
     * @return int value of btrDate (btrDate.i)
     */
    public static function getNowDate()
    {
        return getNowDate();
    }
    /**
     *
     * @return int value of btrTime (btrTime.i)
     */
    public static function getNowTime()
    {
        return getNowTime();
    }
    
    const FIELD_VALUE_MODE_VALUE = 0;
    
    const FIELD_VALUE_MODE_OBJECT = 1;
    
    public static $fieldValueMode = self::FIELD_VALUE_MODE_OBJECT;
    /**
     *
     * @param int $mode
     */
    public static function setFieldValueMode($mode)
    {
        self::$fieldValueMode = $mode;
    }
    /**
     *
     * @return int
     */
    public static function fieldValueMode()
    {
        return  self::$fieldValueMode;
    }
    
    const NULLVALUE_MODE_RETURNNULL = 0;
        
    const NULLVALUE_MODE_NORETURNNULL = 1;

    public static $nullValueMode = self::NULLVALUE_MODE_NORETURNNULL;
    /**
     *
     * @param int $mode
     */
    public static function setNullValueMode($mode)
    {
        self::$nullValueMode = $mode;
    }
    /**
     *
     * @return int
     */
    public static function nullValueMode()
    {
        return  self::$nullValueMode;
    }
    
    const  FETCH_VAL_NUM       = 1;
    const  FETCH_VAL_ASSOC     = 2;
    const  FETCH_VAL_BOTH      = 3;
    const  FETCH_OBJ           = 4;
    const  FETCH_USR_CLASS     = 8;
    const  FETCH_RECORD_INTO   = 16;
}

/* PHP Proxy Classes */
class Bookmark
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('BOOKMARK_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_BOOKMARK') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_BOOKMARK();
    }
    /**
     *
     * @return bool
     */
    public function isEmpty()
    {
        return BOOKMARK_isEmpty($this->cPtr);
    }
}

/**
  * @property int $bit0
  * @property int $bit1
  * @property int $bit2
  * @property int $bit3
  * @property int $bit4
  * @property int $bit5
  * @property int $bit6
  * @property int $bit7
  * @property int $bit8
  * @property int $bit9
  * @property int $bitA
  * @property int $bitB
  * @property int $bitC
  * @property int $bitD
  * @property int $all
  */
class Flags
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'FLAGS_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        $func = 'FLAGS_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('FLAGS_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__FLAGS') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_FLAGS();
    }
}
/**
 * @property int $fieldNum
 * @property \BizStation\Transactd\Flags $flags
 */
class KeySegment
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'fieldNum') {
            return keySegment_fieldNum_set($this->cPtr, $value);
        }
        if ($var === 'flags') {
            return keySegment_flags_set($this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'flags') {
            return new Flags(keySegment_flags_get($this->cPtr));
        }
        if ($var === 'fieldNum') {
            return keySegment_fieldNum_get($this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('keySegment_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__keySegment') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_keySegment();
    }
}

/**
 * @property int $keyNumber
 * @property int $segmentCount
 */
class Keydef
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'keydef_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        /*if ($var === 'segments') {
            return new keySegment(keydef_segments_get($this->cPtr));
        }*/
        if ($var === 'keyNumber') {
            return keydef_keyNumber_get($this->cPtr);
        }
        if ($var === 'segmentCount') {
            return keydef_segmentCount_get($this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('keydef_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param int $index
     * @return \BizStation\Transactd\KeySegment
     */
    public function segment($index)
    {
        $r=keydef_segment($this->cPtr, $index);
        if (is_resource($r)) {
            return new KeySegment($r);
        }
        return $r;
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__keydef') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_keydef();
    }
}

/**
 * @property int $decimals
 * @property \BizStation\Transactd\Flags $enableFlags
 * @property int $keylen
 * @property int $len
 * @property int $nullValue Use only for P.SQL
 * @property int $pos
 * @property int $type
 */
class Fielddef_t_my
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var == 'digits') {
            $var = 'ddfid';
        }// union name
        $func = 'fielddef_t_my_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var == 'digits') {
            $var = 'ddfid';
        }// union name
        $func = 'fielddef_t_my_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var == 'digits') {
            $var = 'ddfid';
        }// union name
        if (function_exists('fielddef_t_my_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__fielddef_tT_64_t') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_fielddef_t_my();
    }
}
/**
 */
class Fielddef extends fielddef_t_my
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        fielddef_t_my::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return fielddef_t_my::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return fielddef_t_my::__isset($var);
    }
    /**
     *
     * @return string
     */
    public function defaultValue()
    {
        return fielddef_defaultValue($this->cPtr);
    }
    /**
     *
     * @param string $s
     */
    public function setName($s)
    {
        fielddef_setName($this->cPtr, $s);
    }
    /**
     *
     * @return string
     */
    public function typeName()
    {
        return fielddef_typeName($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function align()
    {
        return fielddef_align($this->cPtr);
    }
    /**
     *
     * @param int $charnum Number of charcter.
     */
    public function setLenByCharnum($charnum)
    {
        fielddef_setLenByCharnum($this->cPtr, $charnum);
    }
    /**
     *
     * @param int $dig Digits of total.
     * @param int $dec Number of decimal.
     */
    public function setDecimalDigits($dig, $dec)
    {
        fielddef_setDecimalDigits($this->cPtr, $dig, $dec);
    }
    /**
     *
     * @return int
     */
    public function codePage()
    {
        return fielddef_codePage($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isStringType()
    {
        return fielddef_isStringType($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isPadCharType()
    {
        return fielddef_isPadCharType($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isIntegerType()
    {
        return fielddef_isIntegerType($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isNumericType()
    {
        return fielddef_isNumericType($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isDateTimeType()
    {
        return fielddef_isDateTimeType($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function charNum()
    {
        return fielddef_charNum($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isValidCharNum()
    {
        return fielddef_isValidCharNum($this->cPtr);
    }
    /**
     *
     * @param int $index
     */
    public function setCharsetIndex($index)
    {
        fielddef_setCharsetIndex($this->cPtr, $index);
    }
    /**
     *
     * @return int
     */
    public function charsetIndex()
    {
        return fielddef_charsetIndex($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isNullable()
    {
        return fielddef_isNullable($this->cPtr);
    }
    /**
     *
     * @param bool $v Nullable
     * @param bool $defaultNull Is default null.
     */
    public function setNullable($v, $defaultNull=true)
    {
        fielddef_setNullable($this->cPtr, $v, $defaultNull);
    }
    /**
     *
     * @param int|string|\Bizstation\Transactd\Bitset $s_or_v
     */
    public function setDefaultValue($s_or_v)
    {
        fielddef_setDefaultValue($this->cPtr, $s_or_v);
    }
    /**
     *
     * @param bool $v
     */
    public function setTimeStampOnUpdate($v)
    {
        fielddef_setTimeStampOnUpdate($this->cPtr, $v);
    }
    /**
     *
     * @return bool
     */
    public function isTimeStampOnUpdate()
    {
        return fielddef_isTimeStampOnUpdate($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isDefaultNull()
    {
        return fielddef_isDefaultNull($this->cPtr);
    }
    /**
     *
     * @return string Field name.
     */
    public function name()
    {
        return fielddef_name($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isTrimPadChar()
    {
        return fielddef_isTrimPadChar($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isUsePadChar()
    {
        return fielddef_isUsePadChar($this->cPtr);
    }
    /**
     *
     * @param bool $set Fill the surplus in the blank?
     * @param bool $trim Return value, removing the blank automatically.
     */
    public function setPadCharSettings($set, $trim)
    {
        fielddef_setPadCharSettings($this->cPtr, $set, $trim);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__fielddef') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_fielddef();
    }
}
/**
 * @property int $pageSize
 * @property int $varSize
 * @property int $charsetIndex
 * @property int $ddfid
 * @property int $fieldCount
 * @property int fixedRecordLen
 * @property \BizStation\Transactd\Flags $flags
 * @property int $id
 * @property int $keyCount
 * @property \BizStation\Transactd\Flags $optionFlags
 * @property int $primaryKeyNum
 * @property int $preAlloc
 * @property int $schemaCodePage
 * @property int $version
 */
class Tabledef
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'tabledef_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'flags') {
            return new Flags(tabledef_flags_get($this->cPtr));
        }
        if ($var === 'parent') {
            return new Dbdef(tabledef_parent_get($this->cPtr));
        }
        $func = 'tabledef_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('tabledef_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__tabledef') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_tabledef();
    }
    /**
     * Set all attribute to default value.
     */
    public function cleanup()
    {
        tabledef_cleanup($this->cPtr);
    }
    /**
     *
     * @return string
     */
    public function fileName()
    {
        return tabledef_fileName($this->cPtr);
    }
    /**
     *
     * @return string
     */
    public function tableName()
    {
        return tabledef_tableName($this->cPtr);
    }
    /**
     *
     * @param string $s
     */
    public function setFileName($s)
    {
        tabledef_setFileName($this->cPtr, $s);
    }
    /**
     *
     * @param string $s
     */
    public function setTableName($s)
    {
        tabledef_setTableName($this->cPtr, $s);
    }
    /**
     *
     * @return int
     */
    public function nullfields()
    {
        return tabledef_nullfields($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function inUse()
    {
        return tabledef_inUse($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function size()
    {
        return tabledef_size($this->cPtr);
    }
    /**
     *
     * @param string $name
     * @return int
     */
    public function fieldNumByName($name)
    {
        return tabledef_fieldNumByName($this->cPtr, $name);
    }
    /**
     *
     * @return int
     */
    public function recordlen()
    {
        return tabledef_recordlen($this->cPtr);
    }
    /**
     *
     * @param bool $isMariadb
     * @param int $srvMinorVersion
     */
    public function setValidationTarget($isMariadb, $srvMinorVersion)
    {
        tabledef_setValidationTarget($this->cPtr, $isMariadb, $srvMinorVersion);
    }
    /**
     *
     * @return bool
     */
    public function isMysqlNullMode()
    {
        return tabledef_isMysqlNullMode($this->cPtr);
    }
    /**
     *
     * @param int $index
     * @return \BizStation\Transactd\Fielddef
     */
    public function fieldDef($index)
    {
        $r=tabledef_fieldDef($this->cPtr, $index);
        if (is_resource($r)) {
            return new Fielddef($r);
        }
        return $r;
    }
    /**
     *
     * @param int $index
     * @return \BizStation\Transactd\Keydef
     */
    public function keyDef($index)
    {
        $r=tabledef_keyDef($this->cPtr, $index);
        if (is_resource($r)) {
            return new Keydef($r);
        }
        return $r;
    }
}
/**
 * @property int $majorVersion
 * @property int $minorVersion
 * @property int $type
 */
class BtrVersion
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'btrVersion_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        $func = 'btrVersion_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('btrVersion_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @return string
     */
    public function moduleVersionShortString()
    {
        return btrVersion_moduleVersionShortString($this->cPtr);
    }
    /**
     *
     * @return string
     */
    public function moduleTypeString()
    {
        return btrVersion_moduleTypeString($this->cPtr);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__btrVersion') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_btrVersion();
    }
}

class BtrVersions
{
    public $cPtr=null;
    /**
     *
     * @param int $index
     * @return \BizStation\Transactd\BtrVersion
     */
    public function version($index)
    {
        $r=btrVersions_version($this->cPtr, $index);
        if (is_resource($r)) {
            return new BtrVersion($r);
        }
        return $r;
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__btrVersions') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_btrVersions();
    }
}

abstract class Nstable
{
    public $cPtr=null;
    /*protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }*/
    /**
     *
     * @param resource|null $res
     */
    public function __construct($h)
    {
        $this->cPtr=$h;
    }

    const changeCurrentCc = 0;

    const changeCurrentNcc = nstable_changeCurrentNcc;

    const changeInKey = nstable_changeInKey;

    const findForword = 0;

    const findBackForword = nstable_findBackForword;

    const findContinue = nstable_findContinue;

    const inkey = nstable_inkey;
    /**
     *
     * @return \BizStation\Transactd\Nsdatabase
     */
    public function nsdb()
    {
        $r=nstable_nsdb($this->cPtr);
        if (is_resource($r)) {
            return new Nsdatabase($r);
        }
        return $r;
    }
    /**
     *
     * @return int
     */
    public function tableid()
    {
        return nstable_tableid($this->cPtr);
    }
    /**
     *
     * @param int $v
     */
    public function setTableid($v)
    {
        nstable_setTableid($this->cPtr, $v);
    }
    /**
     *
     * @return bool
     */
    public function isOpen()
    {
        return nstable_isOpen($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function isUseTransactd()
    {
        return nstable_isUseTransactd($this->cPtr);
    }
    /**
     *
     * @param int $curd
     */
    public function setAccessRights($curd)
    {
        nstable_setAccessRights($this->cPtr, $curd);
    }
    /**
     *
     * @return int
     */
    public function datalen()
    {
        return nstable_datalen($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function stat()
    {
        return nstable_stat($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function keyNum()
    {
        return nstable_keyNum($this->cPtr);
    }
    /**
     *
     * @param int $v
     */
    public function setKeyNum($v)
    {
        nstable_setKeyNum($this->cPtr, $v);
    }
    /**
     *
     * @return bool
     */
    public function canRead()
    {
        return nstable_canRead($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function canWrite()
    {
        return nstable_canWrite($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function canInsert()
    {
        return nstable_canInsert($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function canDelete()
    {
        return nstable_canDelete($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function getWriteImageLen()
    {
        return nstable_getWriteImageLen($this->cPtr);
    }
    /**
     *
     */
    public function close()
    {
        nstable_close($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function updateConflictCheck()
    {
        return nstable_updateConflictCheck($this->cPtr);
    }
    /**
     *
     * @param bool $v
     * @return bool This table has a update able timestamp field.
     */
    public function setUpdateConflictCheck($v)
    {
        return nstable_setUpdateConflictCheck($this->cPtr, $v);
    }
    /**
     *
     * @param int $type
     */
    public function update($type=nstable::changeCurrentCc)
    {
        nstable_update($this->cPtr, $type);
    }
    /**
     *
     * @param bool $in_key
     */
    public function del($in_key=false)
    {
        nstable_del($this->cPtr, $in_key);
    }
    /**
     *
     * @param bool $ncc
     * @return int
     */
    public function insert($ncc=false)
    {
        return nstable_insert($this->cPtr, $ncc);
    }
    /**
     *
     * @param int $specifyKeyNum
     */
    public function createIndex($specifyKeyNum=false)
    {
        nstable_createIndex($this->cPtr, $specifyKeyNum);
    }
    /**
     *
     * @param int $norenumber
     */
    public function dropIndex($norenumber=false)
    {
        nstable_dropIndex($this->cPtr, $norenumber);
    }
    /**
     *
     * @param bool $estimate
     * @param bool $fromCurrent
     * @return int
     */
    public function recordCount($estimate=true, $fromCurrent=false)
    {
        return nstable_recordCount($this->cPtr, $estimate, $fromCurrent);
    }
    /**
     *
     * @param int $maxBuflen
     */
    public function beginBulkInsert($maxBuflen)
    {
        nstable_beginBulkInsert($this->cPtr, $maxBuflen);
    }
    /**
     *
     */
    public function abortBulkInsert()
    {
        nstable_abortBulkInsert($this->cPtr);
    }
    /**
     *
     * @param bool $autoCommit
     * @return int
     */
    public function commitBulkInsert($autoCommit=false)
    {
        return nstable_commitBulkInsert($this->cPtr, $autoCommit);
    }
    /**
     *
     * @param int $lockBias
     */
    public function seekFirst($lockBias=0)
    {
        nstable_seekFirst($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function seekLast($lockBias=0)
    {
        nstable_seekLast($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function seekPrev($lockBias=0)
    {
        nstable_seekPrev($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function seekNext($lockBias=0)
    {
        nstable_seekNext($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function seek($lockBias=0)
    {
        nstable_seek($this->cPtr, $lockBias);
    }
    /**
     *
     * @param bool $orEqual
     * @param int $lockBias
     */
    public function seekGreater($orEqual, $lockBias=LOCK_BIAS_DEFAULT)
    {
        nstable_seekGreater($this->cPtr, $orEqual, $lockBias);
    }
   /**
     *
     * @param bool $orEqual
     * @param int $lockBias
     */
    public function seekLessThan($orEqual, $lockBias=0)
    {
        nstable_seekLessThan($this->cPtr, $orEqual, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function stepFirst($lockBias=0)
    {
        nstable_stepFirst($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function stepLast($lockBias=0)
    {
        nstable_stepLast($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function stepPrev($lockBias=0)
    {
        nstable_stepPrev($this->cPtr, $lockBias);
    }
    /**
     *
     * @param int $lockBias
     */
    public function stepNext($lockBias=0)
    {
        nstable_stepNext($this->cPtr, $lockBias);
    }
    /**
     *
     * @return int
     */
    public function bookmarkLen()
    {
        return nstable_bookmarkLen($this->cPtr);
    }
    /**
     *
     * @return \BizStation\Transactd\Bookmark
     */
    public function bookmark()
    {
        return nstable_bookmark($this->cPtr);
    }
    /**
     *
     * @param \BizStation\Transactd\Bookmark $bm
     * @param int $lockBias
     */
    public function seekByBookmark($bm=null, $lockBias=0)
    {
        switch (func_num_args()) {
        case 0: nstable_seekByBookmark($this->cPtr); break;
        default: nstable_seekByBookmark($this->cPtr, $bm, $lockBias);
        }
    }
    /**
     *
     * @param \BizStation\Transactd\Bookmark $bm
     * @return int 0 to 10000
     */
    public function getPercentage($bm=null)
    {
        switch (func_num_args()) {
        case 0: $r=nstable_getPercentage($this->cPtr); break;
        default: $r=nstable_getPercentage($this->cPtr, $bm);
        }
        return $r;
    }
    /**
     *
     * @param int $pc
     */
    public function seekByPercentage($pc=null)
    {
        switch (func_num_args()) {
        case 0: nstable_seekByPercentage($this->cPtr); break;
        default: nstable_seekByPercentage($this->cPtr, $pc);
        }
    }
    /**
     *
     * @param string $name
     * @param int $enctype
     */
    public function setOwnerName($name, $enctype=0)
    {
        nstable_setOwnerName($this->cPtr, $name, $enctype);
    }
    /**
     *
     */
    public function clearOwnerName()
    {
        nstable_clearOwnerName($this->cPtr);
    }
    /**
     *
     * @return int
     */
    public function recordLength()
    {
        return nstable_recordLength($this->cPtr);
    }
    /**
     *
     * @param type $databuffer
     * @param type $buflen
     * @param type $estimate
     */
    public function stats($databuffer, $buflen, $estimate=true)
    {
        nstable_stats($this->cPtr, $databuffer, $buflen, $estimate);
    }
    /**
     *
     * @return string
     */
    public function getCreateSql()
    {
        return nstable_getCreateSql($this->cPtr);
    }
    /**
     *
     * @param \BizStation\Transactd\Bookmark $bm
     */
    public function unlock($bm=null)
    {
        switch (func_num_args()) {
        case 0: nstable_unlock($this->cPtr); break;
        default: nstable_unlock($this->cPtr, $bm);
        }
    }
    /**
     *
     * @return int
     */
    public function mode()
    {
        return nstable_mode($this->cPtr);
    }
    /**
     *
     * @param int $mode
     */
    public function setTimestampMode($mode)
    {
        nstable_setTimestampMode($this->cPtr, $mode);
    }
    /**
     *
     * @return string
     */
    public function statMsg()
    {
        return nstable_statMsg($this->cPtr);
    }
    /**
     *
     * @param string $uri
     * @return string
     */
    public static function getFileName($uri)
    {
        return nstable_getFileName($uri);
    }
    /**
     *
     * @param string $uri
     * @return string
     */
    public static function getDirURI($uri)
    {
        return nstable_getDirURI($uri);
    }
    /**
     *
     * @param string $filename
     * @return bool
     */
    public static function existsFile($filename)
    {
        return nstable_existsFile($filename);
    }
}

abstract class RangeIterator implements \Iterator
{
    protected $curIndex = 0;
    protected $startIndex = -1;
    protected $endIndex = -1;
    /**
     *
     * @param int $start
     * @param int $end
     */
    public function __construct($start, $end)
    {
        $this->curIndex = 0;
        $this->startIndex = $start;
        $this->endIndex = $end;
    }
    /**
     *
     */
    public function rewind()
    {
        $this->curIndex = $this->startIndex;
    }
    /**
     *
     * @return bool
     */
    public function valid()
    {
        return $this->curIndex < $this->endIndex;
    }

    abstract public function current();
    /**
     *
     * @return int
     */
    public function key()
    {
        return $this->curIndex;
    }
    /**
     *
     */
    public function next()
    {
        ++$this->curIndex;
    }
}
/**
 * @property int $delCount
 * @property int $insCount
 * @property int $conId
 * @property int $longValue
 * @property int $db
 * @property int $port
 * @property int $readCount
 * @property int $type
 * @property int $updCount
 * @property bool $inSnapshot
 * @property bool $inTransaction
 * @property bool $openEx
 * @property bool $openNormal
 * @property bool $openReadOnly
 * @property bool $openReadOnlyEx
 * @property int $id
 * @property string|null $name
 * @property string $value
 */
 class ConnRecord
 {
     public $cPtr=null;
     protected $pData=array();

     public function __set($var, $value)
     {
         if ($var === "port") {
             $var = "db";
         }
         $func = 'connRecord_'.$var.'_set';
         if (function_exists($func)) {
             return call_user_func($func, $this->cPtr, $value);
         }
         if ($var === 'thisown') {
             return swig_transactd_alter_newobject($this->cPtr, $value);
         }
         $this->pData[$var] = $value;
     }

     public function __get($var)
     {
         if ($var === "port") {
             $var = "db";
         }
         $func = 'connRecord_'.$var.'_get';
         if (function_exists($func)) {
             return call_user_func($func, $this->cPtr);
         }
         if ($var === 'thisown') {
             return swig_transactd_get_newobject($this->cPtr);
         }
         return $this->pData[$var];
     }

     public function __isset($var)
     {
         if ($var === "port") {
             $var = "db";
         }
         if (function_exists('connRecord_'.$var.'_get')) {
             return true;
         }
         if ($var === 'thisown') {
             return true;
         }
         return array_key_exists($var, $this->pData);
     }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__db__transactd__connection__record') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_connRecord();
    }
 }

class ConnRecordsIterator extends RangeIterator
{
    private $connRecordsPtr = null;
    /**
     *
     * @param resource $ConnRecords_ptr
     * @param int $start
     * @param int $end
     */
    public function __construct($ConnRecords_ptr, $start, $end)
    {
        $this->connRecordsPtr = $ConnRecords_ptr;
        parent::__construct($start, $end);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecord
     */
    public function current()
    {
        $r = ConnRecords_getRecord($this->connRecordsPtr, $this->curIndex);
        if (is_resource($r)) {
            return new ConnRecord($r);
        }
        return $r;
    }
}

class ConnRecords implements \ArrayAccess, \Countable , \IteratorAggregate
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @return int
     */
    public function size()
    {
        return ConnRecords_size($this->cPtr);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__connMgr__records') {
            $this->cPtr=$res;
            return;
        }
        throw new \BadMethodCallException();
    }

    /**
     *
     * @return \BizStation\Transactd\ConnRecordsIterator
     */
    public function getIterator()
    {
        return new ConnRecordsIterator($this->cPtr, 0, ConnRecords_size($this->cPtr));
    }

    /**
     *
     * @param int $offset
     * @return bool
     */
    public function offsetExists($offset)
    {
        return ($offset >= 0 && $offset < ConnRecords_size($this->cPtr));
    }
    /**
     *
     * @param int $offset
     * @return \BizStation\Transactd\ConnRecord
     * @throws \OutOfRangeException
     */
    public function offsetGet($offset)
    {
        if (\gettype($offset) !== "integer" ||
            $offset < 0 || $offset >= ConnRecords_size($this->cPtr)) {
            throw new \OutOfRangeException();
        }
        $r = ConnRecords_getRecord($this->cPtr, $offset);
        if (is_resource($r)) {
            return new ConnRecord($r);
        }
        return $r;
    }
    /**
     *
     * @param int $offset
     * @param \BizStation\Transactd\connRecord $value
     * @throws \BadMethodCallException
     */
    public function offsetSet($offset, $value)
    {
        throw new \BadMethodCallException();
    }
    /**
     *
     * @param int $offset
     * @throws \BadMethodCallException
     */
    public function offsetUnset($offset)
    {
        throw new \BadMethodCallException();
    }

    /**
     *
     * @return int
     */
    public function count()
    {
        return ConnRecords_size($this->cPtr);
    }
}

class ConnMgr
{
    public $cPtr=null;
    protected $pData=array();
    
    private function getRecords($r)
    {
        if (is_resource($r)) {
            $c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
            if (class_exists($c)) {
                return new $c($r);
            }
            return new ConnRecords($r);
        }
        return $r;
    }

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param \BizStataion\Transactd\Database $db
     */
    public function __construct($db)
    {
        $this->cPtr=connMgr_create($db);
    }
    /**
     *
     * @param string $uri
     * @return bool
     */
    public function connect($uri)
    {
        return connMgr_connect($this->cPtr, $uri);
    }
    /**
     *
     */
    public function disconnect()
    {
        connMgr_disconnect($this->cPtr);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function databases()
    {
        $r=connMgr_databases($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @param string $dbname
     * @return \BizStation\Transactd\ConnRecords
     */
    public function tables($dbname)
    {
        $r=connMgr_tables($this->cPtr, $dbname);
        return $this->getRecords($r);
    }
    /**
     *
     * @param string $dbname
     * @return \BizStation\Transactd\ConnRecords
     */
    public function views($dbname)
    {
        $r=connMgr_views($this->cPtr, $dbname);
        return $this->getRecords($r);
    }
    /**
     *
     * @param string $dbname
     * @return \BizStation\Transactd\ConnRecords
     */
    public function schemaTables($dbname)
    {
        $r=connMgr_schemaTables($this->cPtr, $dbname);
        return $this->getRecords($r);
    }
    /**
     *
     * @param string $channel
     * @return \BizStation\Transactd\ConnRecords
     */
    public function slaveStatus($channel=null)
    {
        switch (func_num_args()) {
        case 0: $r=connMgr_slaveStatus($this->cPtr); break;
        default: $r=connMgr_slaveStatus($this->cPtr, $channel);
        }
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function sysvars()
    {
        $r=connMgr_sysvars($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function statusvars()
    {
        $r=connMgr_statusvars($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @param bool $withLock
     * @return \BizStation\Transactd\ConnRecords
     */
    public function channels($withLock=false)
    {
        $r=connMgr_channels($this->cPtr, $withLock);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function slaveHosts()
    {
        $r=connMgr_slaveHosts($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function extendedvars()
    {
        $r=connMgr_extendedvars($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function connections()
    {
        $r=connMgr_connections($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function inUseDatabases($connid)
    {
        $r=connMgr_inUseDatabases($this->cPtr, $connid);
        return $this->getRecords($r);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function inUseTables($connid, $dbid)
    {
        $r=connMgr_inUseTables($this->cPtr, $connid, $dbid);
        return $this->getRecords($r);
    }
    /**
     *
     * @param int $connid
     */
    public function postDisconnectOne($connid)
    {
        connMgr_postDisconnectOne($this->cPtr, $connid);
    }
    /**
     *
     */
    public function postDisconnectAll()
    {
        connMgr_postDisconnectAll($this->cPtr);
    }
    /**
     *
     * @return bool
     */
    public function haLock()
    {
        return connMgr_haLock($this->cPtr);
    }
    /**
     *
     */
    public function haUnlock()
    {
        connMgr_haUnlock($this->cPtr);
    }
    /**
     *
     * @param int $v
     * @return bool
     */
    public function setRole($v)
    {
        return connMgr_setRole($this->cPtr, $v);
    }
    /**
     *
     * @param bool $v
     * @return bool
     */
    public function setTrxBlock($v)
    {
        return connMgr_setTrxBlock($this->cPtr, $v);
    }
    /**
     *
     * @param bool $v
     * @return bool
     */
    public function setEnableFailover($v)
    {
        return connMgr_setEnableFailover($this->cPtr, $v);
    }
    /**
     *
     * @return int
     */
    public function stat()
    {
        return connMgr_stat($this->cPtr);
    }
    /**
     *
     * @return \BizStation\Transactd\ConnRecords
     */
    public function db()
    {
        $r=connMgr_db($this->cPtr);
        return $this->getRecords($r);
    }
    /**
     *
     * @return bool
     */
    public function isOpen()
    {
        return connMgr_isOpen($this->cPtr);
    }
    /**
     *
     * @param int $index
     * @return string
     */
    public function slaveStatusName($index)
    {
        return connMgr_slaveStatusName($this->cPtr, $index);
    }
    /**
     *
     * @param \BizStation\Transactd\ConnRecords $recs
     */
    public static function removeSystemDb($recs)
    {
        connMgr_removeSystemDb($recs);
    }
    /**
     *
     * @param int $index
     * @return string
     */
    public static function sysvarName($index)
    {
        return connMgr_sysvarName($index);
    }
    /**
     *
     * @param int $index
     * @return string
     */
    public static function statusvarName($index)
    {
        return connMgr_statusvarName($index);
    }
    /**
     *
     * @param int $index
     * @return string
     */
    public static function extendedVarName($index)
    {
        return connMgr_extendedVarName($index);
    }
}

class Dbdef
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource $res
     */
    public function __construct($res)
    {
        $this->cPtr=$res;
    }
    /**
     * 
     * @return int Return max table id.
     */
    public function tableCount()
    {
        return dbdef_tableCount($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function openMode()
    {
        return dbdef_openMode($this->cPtr);
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\Tabledef
     */
    public function tableDefs($index)
    {
        $r=dbdef_tableDefs($this->cPtr, $index);
        if (is_resource($r)) {
            return new Tabledef($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $v
     */
    public function setVersion($v)
    {
        dbdef_setVersion($this->cPtr, $v);
    }
    /**
     * 
     * @return int
     */
    public function version()
    {
        return dbdef_version($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function stat()
    {
        return dbdef_stat($this->cPtr);
    }
    /**
     * 
     * @param int $tableIndex
     * @return int
     */
    public function validateTableDef($tableIndex)
    {
        return dbdef_validateTableDef($this->cPtr, $tableIndex);
    }
    /**
     * 
     * @param int $tableIndex
     * @param bool $forPsqlDdf
     */
    public function updateTableDef($tableIndex, $forPsqlDdf=true)
    {
        dbdef_updateTableDef($this->cPtr, $tableIndex, $forPsqlDdf);
    }
    /**
     * 
     * @param int $tableIndex
     * @param int $insertIndex
     * @return \BizStation\Transactd\Fielddef
     */
    public function insertField($tableIndex, $insertIndex)
    {
        $r=dbdef_insertField($this->cPtr, $tableIndex, $insertIndex);
        if (is_resource($r)) {
            return new Fielddef($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $tableIndex
     * @param int $deleteIndex
     */
    public function deleteField($tableIndex, $deleteIndex)
    {
        dbdef_deleteField($this->cPtr, $tableIndex, $deleteIndex);
    }
    /**
     * 
     * @param int $tableIndex
     * @param int $insertIndex
     * @return \BizStation\Transactd\Keydef
     */
    public function insertKey($tableIndex, $insertIndex)
    {
        $r=dbdef_insertKey($this->cPtr, $tableIndex, $insertIndex);
        if (is_resource($r)) {
            return new Keydef($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $tableIndex
     * @param int $deleteIndex
     */
    public function deleteKey($tableIndex, $deleteIndex)
    {
        dbdef_deleteKey($this->cPtr, $tableIndex, $deleteIndex);
    }
    /**
     * 
     * @param \BizStation\Transactd\Tabledef $tableDef
     */
    public function insertTable($tableDef)
    {
        dbdef_insertTable($this->cPtr, $tableDef);
    }
    /**
     * 
     * @param int $tableIndex
     */
    public function deleteTable($tableIndex)
    {
        dbdef_deleteTable($this->cPtr, $tableIndex);
    }
    /**
     * 
     * @param int $oldIndex
     * @param int $newIndex
     */
    public function renumberTable($oldIndex, $newIndex)
    {
        dbdef_renumberTable($this->cPtr, $oldIndex, $newIndex);
    }
    /**
     * 
     * @param string $tableName
     * @return int
     */
    public function tableNumByName($tableName)
    {
        return dbdef_tableNumByName($this->cPtr, $tableName);
    }
    /**
     * 
     * @param int $tableIndex
     * @param int $index
     * @return int
     */
    public function findKeynumByFieldNum($tableIndex, $index)
    {
        return dbdef_findKeynumByFieldNum($this->cPtr, $tableIndex, $index);
    }
    /**
     * 
     * @param int $tableIndex
     * @param string $name
     * @return int
     */
    public function fieldNumByName($tableIndex, $name)
    {
        return dbdef_fieldNumByName($this->cPtr, $tableIndex, $name);
    }
    /**
     * 
     * @param int $query eMinlen or eMaxlen or eDefaultlen or eDecimals.
     * @param int $fieldType
     * @return int
     */
    public function fieldValidLength($query, $fieldType)
    {
        return dbdef_fieldValidLength($this->cPtr, $query, $fieldType);
    }
    /**
     * 
     * @return string
     */
    public function statMsg()
    {
        return dbdef_statMsg($this->cPtr);
    }
    /**
     * 
     * @param int $mode
     */
    public function reopen($mode=-2)
    {
        dbdef_reopen($this->cPtr, $mode);
    }
    /**
     * 
     * @return int
     */
    public function mode()
    {
        return dbdef_mode($this->cPtr);
    }
    /**
     * 
     * @param int $tableIndex
     */
    public function pushBackup($tableIndex)
    {
        dbdef_pushBackup($this->cPtr, $tableIndex);
    }
    /**
     * 
     * @param int $tableIndex
     */
    public function popBackup($tableIndex)
    {
        dbdef_popBackup($this->cPtr, $tableIndex);
    }
    /**
     * 
     * @param int $tableIndex
     */
    public function synchronizeSeverSchema($tableIndex)
    {
        dbdef_synchronizeSeverSchema($this->cPtr, $tableIndex);
    }
}


class TableIterator implements \Iterator
{
    protected $table = null;
    protected $pos = 0;

    public function __construct($table)
    {
        $this->table = $table;
    }
    /**
     * 
     */
    public function rewind()
    {
        $this->pos = 0;
    }
    /**
     * 
     * @return \BizStation\Transactd\Record
     */
    public function current()
    {
        return $this->table->fields();
    }
    /**
     * 
     * @return int
     */
    public function key()
    {
        return $this->pos;
    }
    /**
     * 
     */
    public function next()
    {
        $this->table->findNext();
        ++$this->pos;
    }
    /**
     * 
     * @return bool
     */
    public function valid()
    {
        return ($this->table->stat() === 0);
    }
}


class Table extends Nstable
{
    public $fetchMode = transactd::FETCH_RECORD_INTO;
    public $fetchClass = 'stdClass';
    public $ctorArgs = null;
    public $cPtr=null;
    
    private function getBookmark($r)
    {
        if (is_resource($r)) {
            $c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
            if (class_exists($c)) {
                return new $c($r);
            }
            return new Bookmark($r);
        }
        return $r;
    }
    /**
     * 
     * @return \BizStation\Transactd\TableIterator
     */
    public function getIterator()
    {
        return new TableIterator($this);
    }

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        nstable::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return nstable::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return nstable::__isset($var);
    }
    /**
     * 
     * @param resource $h
     */
    public function __construct($h)
    {
        $this->cPtr=$h;
        parent::__construct($h);
    }
    
    const clearNull = 0;

    const defaultNull = table_defaultNull;
    /**
     * 
     * @return \BizStation\Transactd\Tabledef
     */
    public function tableDef()
    {
        $r=table_tableDef($this->cPtr);
        if (is_resource($r)) {
            return new Tabledef($r);
        }
        return $r;
    }
    /**
     * 
     * @return bool
     */
    public function valiableFormatType()
    {
        return table_valiableFormatType($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function blobFieldUsed()
    {
        return table_blobFieldUsed($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function logicalToString()
    {
        return table_logicalToString($this->cPtr);
    }
    /**
     * 
     * @param bool $v
     */
    public function setLogicalToString($v)
    {
        table_setLogicalToString($this->cPtr, $v);
    }
    /**
     * 
     * @return bool
     */
    public function myDateTimeValueByBtrv()
    {
        return table_myDateTimeValueByBtrv($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function bookmarksCount()
    {
        return table_bookmarksCount($this->cPtr);
    }
    /**
     * 
     * @param int $index
     */
    public function moveBookmarks($index)
    {
        table_moveBookmarks($this->cPtr, $index);
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\Bookmark
     */
    public function bookmarks($index)
    {
        $r=table_bookmarks($this->cPtr, $index);
        return $this->getBookmark($r);
    }
    /**
     * 
     * @param int $resetType
     */
    public function clearBuffer($resetType=table_defaultNull)
    {
        table_clearBuffer($this->cPtr, $resetType);
    }
    /**
     * 
     * @return int
     */
    public function getRecordHash()
    {
        return table_getRecordHash($this->cPtr);
    }
    /**
     * 
     */
    public function smartUpdate()
    {
        table_smartUpdate($this->cPtr);
    }
    /**
     * 
     * @param mixed $kv1
     * @param mixed $kv2
     * @param mixed $kv3
     * @param mixed $kv4
     * @param mixed $kv5
     * @param mixed $kv6
     * @param mixed $kv7
     * @param mixed $kv8
     * @return int
     */
    public function seekKeyValue($kv1=null, $kv2=null, $kv3=null, $kv4=null, $kv5=null, $kv6=null, $kv7=null, $kv8=null)
    {
        if (is_array($kv1)) {
            //return table_seekKeyValue($this->cPtr, ...$kv1);
            switch (count($kv1)) {
            case 1: return table_seekKeyValue($this->cPtr, $kv1[0]);
            case 2: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1]);
            case 3: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2]);
            case 4: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2], $kv1[3]);
            case 5: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2], $kv1[3], $kv1[4]);
            case 6: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2], $kv1[3], $kv1[4], $kv1[5]);
            case 7: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2], $kv1[3], $kv1[4], $kv1[5], $kv1[6]);
            case 8: return table_seekKeyValue($this->cPtr, $kv1[0], $kv1[1], $kv1[2], $kv1[3], $kv1[4], $kv1[5], $kv1[6], $kv1[7]);
            }
        }
        return table_seekKeyValue($this->cPtr, $kv1, $kv2, $kv3, $kv4, $kv5, $kv6, $kv7, $kv8);
    }
    /**
     * 
     * @param int $type
     * @return mixed The result set in the type specified by the $this->fetchMode
     */
    public function findAll($type=null)
    {
        return table_findAll($this->cPtr, $type, $this->fetchMode, $this->fetchClass,  $this->ctorArgs);
    }
    /**
     * 
     * @param int $type
     */        
    public function find($type=null)
    {
        table_find($this->cPtr, $type);
    }

    public function findFirst()
    {
        table_findFirst($this->cPtr);
    }

    public function findLast()
    {
        table_findLast($this->cPtr);
    }
    /**
     * 
     * @param bool $notIncCurrent
     */
    public function findNext($notIncCurrent=true)
    {
        table_findNext($this->cPtr, $notIncCurrent);
    }
    /**
     * 
     * @param bool $notIncCurrent
     */
    public function findPrev($notIncCurrent=true)
    {
        table_findPrev($this->cPtr, $notIncCurrent);
    }
    /**
     * 
     * @return int
     */
    public function statReasonOfFind()
    {
        return table_statReasonOfFind($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function lastFindDirection()
    {
        return table_lastFindDirection($this->cPtr);
    }
    /**
     * 
     * @return \BizStation\Transactd\Bookmark
     */
    public function bookmarkFindCurrent()
    {
        $r=table_bookmarkFindCurrent($this->cPtr);
        return $this->getBookmark($r);
    }
    /**
     * 
     * @param string $str
     * @param int $rejectCount
     * @param int $cacheCount
     * @param bool $autoEscape
     */
    public function setFilter($str, $rejectCount, $cacheCount, $autoEscape=true)
    {
        table_setFilter($this->cPtr, $str, $rejectCount, $cacheCount, $autoEscape);
    }
    /**
     * 
     * @param string $name
     * @return int
     */
    public function fieldNumByName($name)
    {
        return table_fieldNumByName($this->cPtr, $name);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return int
     */
    public function getFVbyt($index_or_fieldName)
    {
        return table_getFVint($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return int
     */
    public function getFVsht($index_or_fieldName)
    {
        return table_getFVint($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return int
     */
    public function getFVint($index_or_fieldName)
    {
        return table_getFVint($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return int
     */
    public function getFVlng($index_or_fieldName)
    {
        return table_getFVint($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return int|string
     */
    public function getFV64($index_or_fieldName)
    {
        return table_getFV64($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return double
     */
    public function getFVflt($index_or_fieldName)
    {
        return table_getFVdbl($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return double
     */
    public function getFVdbl($index_or_fieldName)
    {
        return table_getFVdbl($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return bool
     */
    public function getFVNull($index_or_fieldName)
    {
        return table_getFVNull($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @param bool $v
     */
    public function setFVNull($index_or_fieldName, $v)
    {
        table_setFVNull($this->cPtr, $index_or_fieldName, $v);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return string
     */
    public function getFVstr($index_or_fieldName)
    {
        return table_getFVstr($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     */
    public function fields()
    {
        if ($this->fetchMode === transactd::FETCH_RECORD_INTO) {
            return new Record(table_fields($this->cPtr, transactd::FETCH_RECORD_INTO, null,  null));
        }
        return table_fields($this->cPtr, $this->fetchMode, $this->fetchClass,  $this->ctorArgs);
    }
    /**
     * 
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     */
    public function getRow()
    {
        if ($this->fetchMode === transactd::FETCH_RECORD_INTO) {
            return new Record(table_fields($this->cPtr, transactd::FETCH_RECORD_INTO, null,  null));
        }
        return table_fields($this->cPtr, $this->fetchMode, $this->fetchClass,  $this->ctorArgs);
    }
    /**
     * 
     * @return \BizStation\Transactd\Record
     */
    public function getRecord()
    {
        return new Record(table_fields($this->cPtr, transactd::FETCH_RECORD_INTO, null,  null));
    }

    public function setFV($index_or_fieldName, $data, $size=null)
    {
        switch (func_num_args()) {
        case 2: table_setFV($this->cPtr, $index_or_fieldName, $data); break;
        default: table_setFV($this->cPtr, $index_or_fieldName, $data, $size);
        }
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return string 
     */
    public function getFVbin($index_or_fieldName)
    {
        return table_getFVbin($this->cPtr, $index_or_fieldName);
    }
    /**
     * 
     * @return string
     */
    public function keyValueDescription()
    {
        return table_keyValueDescription($this->cPtr);
    }
    /**
     * 
     * @param \BizStation\Transactd\QueryBase $q
     * @param bool $serverPrepare
     * @return \BizStation\Transactd\PreparedQuery
     */
    public function prepare($q, $serverPrepare=false)
    {
        return table_prepare($this->cPtr, $q, $serverPrepare);
    }
    /**
     * 
     * @param \BizStation\Transactd\QueryBase $q
     * @param bool $serverPrepare
     * @return \BizStation\Transactd\PreparedQuery
     */
    public function setQuery($q, $serverPrepare=false)
    {
        return table_setQuery($this->cPtr, $q, $serverPrepare);
    }
    /**
     * 
     * @param  \BizStation\Transactd\PreparedQuery $q
     */
    public function setPrepare($q)
    {
        table_setPrepare($this->cPtr, $q);
    }
    /**
     * 
     * @param int|string $index_or_fieldName
     * @return \BizStation\Transactd\Bitset
     */
    public function getFVBits($index_or_fieldName)
    {
        $r=table_getFVBits($this->cPtr, $index_or_fieldName);
        if (!is_resource($r)) {
            return $r;
        }
        switch (get_resource_type($r)) {
        case '_p_bzs__bitset': return new Bitset($r);
        default: return new Bitset($r);
        }
    }
    
    public function setAlias($orign, $alias)
    {
        table_setAlias($this->cPtr, $orign, $alias);
        return $this;
    }
    /**
     * 
     * @param object $obj
     * @param bool $ncc No currency change.
     * @return bool
     */
    public function insertByObject($obj, $ncc = false)
    {
        return table_insertByObject($this->cPtr, $obj, $ncc);
    }
    /**
     * 
     * @param object $obj
     * @param null|int $keyNum null : It is used primary key automatically. int : key number
     * @return bool
     */
    public function readByObject($obj, $keyNum = null)
    {
        return table_readByObject($this->cPtr, $obj, $keyNum);
    }
    /**
     * 
     * @param object $obj
     * @param bool $updateType When you specify the 'changeInKey', It is used primary key automatically. 
     *                         Other wise, it is not changed key number.
     * @return bool
     */
    public function updateByObject($obj, $updateType = Nstable::changeInKey)
    {
        return table_updateByObject($this->cPtr, $obj, $updateType);
    }
    /**
     * 
     * @param object $obj
     * @param bool $inKey When you specify true, It is used primary key automatically. 
     *                         Other wise, it is not changed key number.
     * @return bool
     */
    public function deleteByObject($obj, $inKey = true)
    {
        return table_deleteByObject($this->cPtr, $obj, $inKey);
    }
    /**
     * 
     * @param object $obj
     * @return int 0: Error. For more information can be get by stat().
     *             2: Success by insert.
     *             3: Success by update.
     */
    public function saveByObject($obj)
    {
        return table_saveByObject($this->cPtr, $obj);
    }

    public function release()
    {
        table_release($this->cPtr);
    }
}

abstract class QueryBase
{
    const FULL_SCAN = 0xffff;
    public $cPtr=null;
    protected $pData=array();
    protected $bookmarks=array(); //Holding bookmarks

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @param resource $h
     */
    public function __construct($h)
    {
        $this->cPtr=$h;
    }

    const none = 0;

    const joinHasOneOrHasMany = 1;

    const combineCondition = 2;

    public function clearSeekKeyValues()
    {
        queryBase_clearSeekKeyValues($this->cPtr);
        $this->bookmarks=array();
    }

    public function clearSelectFields()
    {
        queryBase_clearSelectFields($this->cPtr);
    }
    /**
     * 
     * @param int|double|string|null $value
     * @param bool $reset
     */
    public function addSeekKeyValue($value, $reset=false)
    {
        if ($reset === true) {
            $this->bookmarks=array();
        }
        queryBase_addSeekKeyValue($this->cPtr, $value, $reset);
    }
    /**
     * 
     * @param \BizStation\Transactd\Bookmark $bookmark
     * @param int $len Length of a bookmark.
     * @param bool $reset
     */
    public function addSeekBookmark($bookmark, $len, $reset=false)
    {
        if ($reset === true) {
            $this->bookmarks=array();
        }
        queryBase_addSeekBookmark($this->cPtr, $bookmark, $len, $reset);
        array_push($this->bookmarks, $bookmark);
    }
    /**
     * 
     * @param int $v
     */
    public function reserveSeekKeyValueSize($v)
    {
        queryBase_reserveSeekKeyValueSize($this->cPtr, $v);
    }
    /** Set the query by string.
     * 
     * @param string $str
     * @param bool $autoEscape
     * @return \BizStation\Transactd\QueryBase
     */
    public function queryString($str, $autoEscape=false)
    {
        queryBase_queryString($this->cPtr, $str, $autoEscape);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function reject($v)
    {
        queryBase_reject($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function limit($v)
    {
        queryBase_limit($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function direction($v)
    {
        queryBase_direction($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @return \BizStation\Transactd\QueryBase
     */
    public function all()
    {
        queryBase_all($this->cPtr);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function optimize($v)
    {
        queryBase_optimize($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param bool $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function bookmarkAlso($v)
    {
        queryBase_bookmarkAlso($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @return string
     */
    public function toString()
    {
        return queryBase_toString($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function getDirection()
    {
        return queryBase_getDirection($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function getReject()
    {
        return queryBase_getReject($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function getLimit()
    {
        return queryBase_getLimit($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isAll()
    {
        return queryBase_isAll($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function getJoinKeySize()
    {
        return queryBase_getJoinKeySize($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function getOptimize()
    {
        return queryBase_getOptimize($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isBookmarkAlso()
    {
        return queryBase_isBookmarkAlso($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isSeekByBookmarks()
    {
        return queryBase_isSeekByBookmarks($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function selectCount()
    {
        return queryBase_selectCount($this->cPtr);
    }
    /**
     * 
     * @param int $index
     * @return string
     */
    public function getSelect($index)
    {
        return queryBase_getSelect($this->cPtr, $index);
    }
    /**
     * 
     * @return int
     */
    public function whereTokens()
    {
        return queryBase_whereTokens($this->cPtr);
    }
    /**
     * 
     * @param int $index
     * @return string
     */
    public function getWhereToken($index)
    {
        return queryBase_getWhereToken($this->cPtr, $index);
    }
    /**
     * 
     * @param int $index
     * @param string $v
     */
    public function setWhereToken($index, $v)
    {
        queryBase_setWhereToken($this->cPtr, $index, $v);
    }
    /**
     * 
     * @param string $alias
     * @param string $src
     */
    public function reverseAliasName($alias, $src)
    {
        queryBase_reverseAliasName($this->cPtr, $alias, $src);
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function joinKeySize($v)
    {
        queryBase_joinKeySize($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param bool $v
     * @return \BizStation\Transactd\QueryBase
     */
    public function stopAtLimit($v)
    {
        queryBase_stopAtLimit($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @return bool
     */
    public function isStopAtLimit()
    {
        return queryBase_isStopAtLimit($this->cPtr);
    }
}

class Query extends QueryBase
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        queryBase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return queryBase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return queryBase::__isset($var);
    }
    /**
     * 
     * @return \BizStation\Transactd\Query
     */
    public function reset()
    {
        query_reset($this->cPtr);
        $this->bookmarks=array();
        return $this;
    }
    /**
     * 
     * @param string $name 
     * @param string $name1 (optional)
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @param string $name9 (optional)
     * @param string $name10 (optional)
     * @return \BizStation\Transactd\Query
     */
    public function select($name, $name1=null, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null, $name9=null, $name10=null)
    {
        query_select($this->cPtr, $name, $name1, $name2, $name3, $name4, $name5, $name6, $name7, $name8, $name9, $name10);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function whereIsNull($name)
    {
        query_whereIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function whereIsNotNull($name)
    {
        query_whereIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function andIsNull($name)
    {
        query_andIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function andIsNotNull($name)
    {
        query_andIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function orIsNull($name)
    {
        query_orIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Query
     */
    public function orIsNotNull($name)
    {
        query_orIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\Query
     */
    public function segmentsForInValue($v)
    {
        queryBase_joinKeySize($this->cPtr, $v);
        return $this;
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__query') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_query();
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param int|double|string|null $value
     * @return \BizStation\Transactd\Query
     */
    public function where($name, $qlogic, $value)
    {
        query_where($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param int|double|string|null $value
     * @return \BizStation\Transactd\Query
     */
    public function and_($name, $qlogic, $value)
    {
        query_and_($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param int|double|string|null $value
     * @return \BizStation\Transactd\Query
     */
    public function or_($name, $qlogic, $value)
    {
        query_or_($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
    /**
     * 
     * @param int|double|string|null $kv0
     * @param int|double|string|null $kv1 (optional)
     * @param int|double|string|null $kv2 (optional)
     * @param int|double|string|null $kv3 (optional)
     * @param int|double|string|null $kv4 (optional)
     * @param int|double|string|null $kv5 (optional)
     * @param int|double|string|null $kv6 (optional)
     * @param int|double|string|null $kv7 (optional)
     * @return \BizStation\Transactd\Query
     */
    public function in($kv0, $kv1=null, $kv2=null, $kv3=null, $kv4=null, $kv5=null, $kv6=null, $kv7=null)
    {
        query_in($this->cPtr, $kv0, $kv1, $kv2, $kv3, $kv4, $kv5, $kv6, $kv7);
        return $this;
    }
}

class Bitset implements \ArrayAccess
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__bitset') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_bitset();
    }
    /**
     * 
     * @param int $index
     * @param bool $value
     */
    public function set($index, $value)
    {
        bitset_set($this->cPtr, $index, $value);
    }
    /**
     * 
     * @param int $index
     * @return bool
     */
    public function get($index)
    {
        return bitset_get($this->cPtr, $index);
    }
    /**
     * 
     * @param \BizStation\Transactd\Bitset $r
     * @return bool
     */
    public function equals($r)
    {
        return bitset_equals($this->cPtr, $r);
    }
    /**
     * 
     * @param \BizStation\Transactd\Bitset $r
     * @param bool $all
     * @return bool
     */
    public function contains($r, $all=true)
    {
        return bitset_contains($this->cPtr, $r, $all);
    }
    /**
     * 
     * @param int $offset
     * @return bool
     */
    public function offsetExists($offset)
    {
        return $offset >= 0 && $offset < 64;
    }
    /**
     * 
     * @param int $offset
     * @return bool
     */
    public function offsetGet($offset)
    {
        return bitset_get($this->cPtr, $offset);
    }
    /**
     * 
     * @param int $offset
     * @param bool $value
     */
    public function offsetSet($offset, $value)
    {
        bitset_set($this->cPtr, $offset, $value);
    }
    /**
     * 
     * @param int $offset
     */
    public function offsetUnset($offset)
    {
        bitset_set($this->cPtr, $offset, false);
    }
}
/**
 * @property string $filename Binlog filename 
 * @property string $gtid GTID
 * @property int|string $pos Binlog position
 * @property int $type Type. (File or GTID) 
 **/

class BinlogPos
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'binlogPos_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
    }

    public function __get($var)
    {
        if ($var === 'type') {
            return binlogPos_type_get($this->cPtr);
        }
        elseif ($var === 'pos') {
            return binlogPos_pos_get($this->cPtr);
        }
        elseif ($var === 'filename') {
            return binlogPos_filename_get($this->cPtr);
        }
        elseif ($var === 'gtid') {
            return binlogPos_gtid_get($this->cPtr);
        }
        elseif ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('binlogPos_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__binlogPos') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_binlogPos();
    }
}

class Nsdatabase
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__nsdatabase') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_nsdatabase();
    }
    /**
     * 
     * @return bool
     */
    public function enableTrn()
    {
        return nsdatabase_enableTrn($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function stat()
    {
        return nsdatabase_stat($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function clientID()
    {
        return nsdatabase_clientID($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function openTableCount()
    {
        return nsdatabase_openTableCount($this->cPtr);
    }
    /**
     * 
     * @return string Database uri
     */
    public function uri()
    {
        return nsdatabase_uri($this->cPtr);
    }
    /** 
     * 
     * @return bool 
     */
    public function uriMode()
    {
        return nsdatabase_uriMode($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function lockWaitCount()
    {
        return nsdatabase_lockWaitCount($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function lockWaitTime()
    {
        return nsdatabase_lockWaitTime($this->cPtr);
    }
    /**
     * 
     * @param int $v
     */
    public function setLockWaitCount($v)
    {
        nsdatabase_setLockWaitCount($this->cPtr, $v);
    }
    /**
     * 
     * @param int $v
     */
    public function setLockWaitTime($v)
    {
        nsdatabase_setLockWaitTime($this->cPtr, $v);
    }
    /**
     * 
     * @param bool $v
     */
    public function setLocalSharing($v)
    {
        nsdatabase_setLocalSharing($this->cPtr, $v);
    }
    /**
     * 
     * @param string $uri
     */
    public function dropTable($uri)
    {
        nsdatabase_dropTable($this->cPtr, $uri);
    }
    /**
     * 
     * @param string $oldUri
     * @param string $newUri
     */
    public function rename($oldUri, $newUri)
    {
        nsdatabase_rename($this->cPtr, $oldUri, $newUri);
    }
    /**
     * 
     * @param string $uri1
     * @param string $uri2
     */
    public function swapTablename($uri1, $uri2)
    {
        nsdatabase_swapTablename($this->cPtr, $uri1, $uri2);
    }
    /**
     * 
     * @param int $bias
     */
    public function beginTrn($bias=null)
    {
        if ($bias === null) {
            nsdatabase_beginTrn($this->cPtr);
        } else {
            nsdatabase_beginTrn($this->cPtr, $bias);
        }
    }

    public function endTrn()
    {
        nsdatabase_endTrn($this->cPtr);
    }

    public function abortTrn()
    {
        nsdatabase_abortTrn($this->cPtr);
    }
    /**
     * 
     * @param int $bias (optional)
     * @return \BizStation\Transactd\BinlogPos
     */
    public function beginSnapshot($bias=CONSISTENT_READ)
    {
        $r=nsdatabase_beginSnapshot($this->cPtr, $bias);
        if (is_resource($r)) {
            return new BinlogPos($r);
        }
        return $r;
    }
    
    public function endSnapshot()
    {
        nsdatabase_endSnapshot($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function trxIsolationServer()
    {
        return nsdatabase_trxIsolationServer($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function trxLockWaitTimeoutServer()
    {
        return nsdatabase_trxLockWaitTimeoutServer($this->cPtr);
    }
    /**
     * 
     * @return string
     */
    public function statMsg()
    {
        return nsdatabase_statMsg($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function useLongFilename()
    {
        return nsdatabase_useLongFilename($this->cPtr);
    }
    /**
     * 
     * @param bool $value
     */
    public function setUseLongFilename($value)
    {
        nsdatabase_setUseLongFilename($this->cPtr, $value);
    }
    /**
     * 
     * @return bool
     */
    public function setUseTransactd()
    {
        return nsdatabase_setUseTransactd($this->cPtr);
    }
    /**
     * 
     * @param string $uri
     * @return bool
     */
    public function isTransactdUri($uri)
    {
        return nsdatabase_isTransactdUri($this->cPtr, $uri);
    }
    /**
     * 
     * @return bool
     */
    public function isUseTransactd()
    {
        return nsdatabase_isUseTransactd($this->cPtr);
    }
    /**
     * 
     * @return string
     */
    public function readDatabaseDirectory()
    {
        return nsdatabase_readDatabaseDirectory($this->cPtr);
    }
    /**
     * 
     * @param string $uri
     * @param bool $newConnection
     * @return bool
     */
    public function connect($uri, $newConnection=false)
    {
        return nsdatabase_connect($this->cPtr, $uri, $newConnection);
    }
    /**
     * 
     * @param string $uri (optinal)
     * @return bool
     */
    public function disconnect($uri="")
    {
        return nsdatabase_disconnect($this->cPtr, $uri);
    }
    /**
     * 
     * @return bool
     */
    public function disconnectForReconnectTest()
    {
        return nsdatabase_disconnectForReconnectTest($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function reconnect()
    {
        return nsdatabase_reconnect($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isAssociate()
    {
        return nsdatabase_isAssociate($this->cPtr);
    }
    /**
     * 
     * @param string $name
     * @return string
     */
    public function getCreateViewSql($name)
    {
        return nsdatabase_getCreateViewSql($this->cPtr, $name);
    }
    /**
     * 
     * @return bool
     */
    public static function trnsactionFlushWaitStatus()
    {
        return nsdatabase_trnsactionFlushWaitStatus();
    }
    /**
     * 
     * @param int $codepage
     */
    public static function setExecCodePage($codepage)
    {
        nsdatabase_setExecCodePage($codepage);
    }
    /**
     * 
     * @return int
     */
    public static function execCodePage()
    {
        return nsdatabase_execCodePage();
    }
    /**
     * 
     * @return bool
     */
    public static function enableAutoReconnect()
    {
        return nsdatabase_enableAutoReconnect();
    }
    /**
     * 
     * @param bool $v
     */
    public static function setEnableAutoReconnect($v)
    {
        nsdatabase_setEnableAutoReconnect($v);
    }
    /**
     * 
     * @param bool $v
     */
    public static function setCheckTablePtr($v)
    {
        nsdatabase_setCheckTablePtr($v);
    }
}

class Database extends Nsdatabase
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        nsdatabase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return nsdatabase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return nsdatabase::__isset($var);
    }
    /**
     * 
     * @return \BizStation\Transactd\Dbdef
     */
    public function dbDef()
    {
        $r=database_dbDef($this->cPtr);
        if (is_resource($r)) {
            return new Dbdef($r);
        }
        return $r;
    }
    /**
     * 
     * @return string
     */
    public function rootDir()
    {
        return database_rootDir($this->cPtr);
    }
    /**
     * 
     * @param string $directory
     */
    public function setRootDir($directory)
    {
        database_setRootDir($this->cPtr, $directory);
    }
    /**
     * 
     * @return bool
     */
    public function tableReadOnly()
    {
        return database_tableReadOnly($this->cPtr);
    }
    /**
     * 
     * @param bool $value
     */
    public function setTableReadOnly($value)
    {
        database_setTableReadOnly($this->cPtr, $value);
    }
    /**
     * 
     * @param string $uri
     * @param int $schemaType (optional)
     * @param int $mode (optional)
     * @param string $dir (optional)
     * @param string $ownerName (optional)
     * @return bool
     */
    public function open($uri, $schemaType=0, $mode=-2, $dir=null, $ownerName=null)
    {
        return database_open($this->cPtr, $uri, $schemaType, $mode, $dir, $ownerName);
    }

    public function __clone()
    {
        $r=database___clone($this->cPtr);
        $this->__construct($r);
    }
    /**
     * 
     * @param string|int $utf8Sql_or_fileNum
     * @param string $uri (optional)For fileNum
     * @return bool
     */
    public function createTable($utf8Sql_or_fileNum, $uri=null)
    {
        switch (func_num_args()) {
        case 1: $r=database_createTable($this->cPtr, $utf8Sql_or_fileNum); break;
        default: $r=database_createTable($this->cPtr, $utf8Sql_or_fileNum, $uri);
        }
        return $r;
    }
    /**
     * 
     * @param string $utf8Sql
     * @return bool
     */
    public function execSql($utf8Sql)
    {
        return $this->createTable($utf8Sql);
    }
    /**
     * 
     * @param string $tableName
     * @return string
     */
    public function getSqlStringForCreateTable($tableName)
    {
        return database_getSqlStringForCreateTable($this->cPtr, $tableName);
    }
    /**
     * 
     * @param string $uri
     * @param int $type (optional)
     */
    public function create($uri, $type=0)
    {
        database_create($this->cPtr, $uri, $type);
    }
    /**
     * 
     * @param string $uri (optional)
     */
    public function drop($uri=null)
    {
        database_drop($this->cPtr, $uri);
    }
    /**
     * 
     * @param string $tableName
     */
    public function dropTable($tableName)
    {
        database_dropTable($this->cPtr, $tableName);
    }
    /**
     * 
     * @param bool $withDropDefaultSchema (optional)
     */
    public function close($withDropDefaultSchema=false)
    {
        database_close($this->cPtr, $withDropDefaultSchema);
    }
    /**
     * 
     * @return int
     */
    public function aclReload()
    {
        return database_aclReload($this->cPtr);
    }
    /**
     * 
     * @param int $op
     * @param bool $inclideRepfile (optional)
     * @return int
     */
    public function continuous($op=0, $inclideRepfile=false)
    {
        return database_continuous($this->cPtr, $op, $inclideRepfile);
    }
    /**
     * 
     * @param \BizStation\Transactd\Dbdef $src
     * @return int
     */
    public function assignSchemaData($src)
    {
        return database_assignSchemaData($this->cPtr, $src);
    }
    /**
     * 
     * @param \BizStation\Transactd\Table $dest
     * @param \BizStation\Transactd\Table $src
     * @param bool $turbo
     * @param int $keyNum (optional)
     * @param int $maxSkip (optional)
     * @return int
     */
    public function copyTableData($dest, $src, $turbo, $keyNum=-1, $maxSkip=-1)
    {
        return database_copyTableData($this->cPtr, $dest, $src, $turbo, $keyNum, $maxSkip);
    }
    /**
     * 
     * @param int $tableIndex
     * @param bool $turbo
     * @param string $ownerName (optional)
     */
    public function convertTable($tableIndex, $turbo, $ownerName=null)
    {
        switch (func_num_args()) {
        case 2: database_convertTable($this->cPtr, $tableIndex, $turbo); break;
        default: database_convertTable($this->cPtr, $tableIndex, $turbo, $ownerName);
        }
    }
    /**
     * 
     * @param int $tableIndex
     * @param string $ownerName (optional)
     * @return bool
     */
    public function existsTableFile($tableIndex, $ownerName=null)
    {
        switch (func_num_args()) {
        case 1: $r=database_existsTableFile($this->cPtr, $tableIndex); break;
        default: $r=database_existsTableFile($this->cPtr, $tableIndex, $ownerName);
        }
        return $r;
    }
    /**
     * 
     * @param \BizStation\Transactd\BtrVersions $versions
     */
    public function getBtrVersion($versions)
    {
        database_getBtrVersion($this->cPtr, $versions);
    }
    /**
     * 
     * @return bool
     */
    public function isOpened()
    {
        return database_isOpened($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function mode()
    {
        return database_mode($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function autoSchemaUseNullkey()
    {
        return database_autoSchemaUseNullkey($this->cPtr);
    }
    /**
     * 
     * @param bool $v
     */
    public function setAutoSchemaUseNullkey($v)
    {
        database_setAutoSchemaUseNullkey($this->cPtr, $v);
    }
    /**
     * 
     * @return \BizStation\Transactd\Database
     */
    public function createAssociate()
    {
        $r=database_createAssociate($this->cPtr);
        if (is_resource($r)) {
            $c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
            if (class_exists($c)) {
                return new $c($r);
            }
            return new Database($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $mode
     */
    public static function setCompatibleMode($mode)
    {
        database_setCompatibleMode($mode);
    }
    /**
     * 
     * @return int
     */
    public static function compatibleMode()
    {
        return database_compatibleMode();
    }

    const CMP_MODE_MYSQL_NULL = database_CMP_MODE_MYSQL_NULL;

    const CMP_MODE_OLD_NULL = database_CMP_MODE_OLD_NULL;
    
    const CMP_MODE_BINFD_DEFAULT_STR = database_CMP_MODE_BINFD_DEFAULT_STR;
    
    const CMP_MODE_OLD_BIN = 3;//database_CMP_MODE_MYSQL_NULL | database_CMP_MODE_BINFD_DEFAULT_STR;
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__database') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_database();
    }
    /**
     * 
     * @param string|int $tableName_or_fileNum
     * @param int $mode (optional)
     * @param bool $autoCreate (optional)
     * @param string $ownerName (optional)
     * @param string $uri (optional)
     * @return \BizStation\Transactd\Table
     */
    public function openTable($tableName_or_fileNum, $mode=0, $autoCreate=true, $ownerName=null, $uri=null)
    {
        $r=database_openTable($this->cPtr, $tableName_or_fileNum, $mode, $autoCreate, $ownerName, $uri);
        if (!is_resource($r)) {
            return $r;
        }
        return new Table($r);
    }
}

class Benchmark
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }

    public static function start()
    {
        benchmark_start();
    }
    /**
     * 
     * @return int
     */
    public static function stop()
    {
        return benchmark_stop();
    }
    /**
     * 
     * @param int $result
     * @param string $name
     */
    public static function showTimes($result, $name)
    {
        benchmark_showTimes($result, $name);
    }
    /**
     * 
     * @param int $result
     * @param string $name
     */
    public static function showTimeSec($result, $name)
    {
        benchmark_showTimeSec($result, $name);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__rtl__benchmark') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_benchmark();
    }
}
/**
 * @property int $dd
 * @property int $mm
 * @property int $yy
 * @property int $i
 */
class BtrDate
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'btrDate_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        $func = 'btrDate_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('btrDate_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__btrDate') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_btrDate();
    }
}
/**
 * @property int $hh
 * @property int $nn
 * @property int $ss
 * @property int $uu
 * @property int $i
 */
class BtrTime
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'btrTime_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        $func = 'btrTime_'.$var.'_get';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('btrTime_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
   public function __construct($res=null)
   {
       if (is_resource($res) && get_resource_type($res) === '_p_bzs__btrTime') {
           $this->cPtr=$res;
           return;
       }
       $this->cPtr=new_btrTime();
   }
}
/**
 * @property \BizStation\Transactd\BtrDate $date
 * @property \BizStation\Transactd\BtrTime $time
 * @property int|string|double $i64
 */
class BtrDateTime
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        $func = 'btrDateTime_'.$var.'_set';
        if (function_exists($func)) {
            return call_user_func($func, $this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'time') {
            return new BtrTime(btrDateTime_time_get($this->cPtr));
        }
        if ($var === 'date') {
            return new BtrDate(btrDateTime_date_get($this->cPtr));
        }
        if ($var === 'i64') {
            return btrDateTime_i64_get($this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('btrDateTime_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
   public function __construct($res=null)
   {
       if (is_resource($res) && get_resource_type($res) === '_p_bzs__btrDateTime') {
           $this->cPtr=$res;
           return;
       }
       $this->cPtr=new_btrDateTime();
   }
}
/**
 * @property int|string|double $i64
 */
class BtrTimeStamp
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'i64') {
            return btrTimeStamp_i64_set($this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'i64') {
            return btrTimeStamp_i64_get($this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('btrTimeStamp_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }

    public function __construct($i_or_p_or_d, $t=null)
    {
        if (is_resource($i_or_p_or_d) && get_resource_type($i_or_p_or_d) === '_p_bzs__btrTimeStamp') {
            $this->cPtr=$i_or_p_or_d;
            return;
        }
        switch (func_num_args()) {
        case 1: $this->cPtr=new_btrTimeStamp($i_or_p_or_d); break;
        default: $this->cPtr=new_btrTimeStamp($i_or_p_or_d, $t);
        }
    }
    /**
     * 
     * @return string
     */
    public function toString()
    {
        return btrTimeStamp_toString($this->cPtr);
    }
    /**
     * 
     * @param string $p
     */
    public function fromString($p)
    {
        btrTimeStamp_fromString($this->cPtr, $p);
    }
}

class FielddefsIterator extends RangeIterator
{
    private $fielddefsPtr = null;

    public function __construct($fielddefsPtr, $start, $end)
    {
        $this->fielddefsPtr = $fielddefsPtr;
        parent::__construct($start, $end);
    }
    /**
     * 
     * @return \BizStation\Transactd\Fielddef
     */
    public function current()
    {
        $r = fielddefs_getFielddef($this->fielddefsPtr, $this->curIndex);
        if (is_resource($r)) {
            return new Fielddef($r);
        }
        return $r;
    }
}

class Fielddefs implements \ArrayAccess, \Countable, \IteratorAggregate
{
    public $cPtr=null;
    protected $pData=array();

    /**
     * 
     * @return \BizStation\Transactd\FielddefsIterator
     */
    public function getIterator()
    {
        return new FielddefsIterator($this->cPtr, 0, fielddefs_size($this->cPtr));
    }

    /**
     * 
     * @param int $offset
     * @return bool
     */
    public function offsetExists($offset)
    {
        return ($offset >= 0 && $offset < fielddefs_size($this->cPtr));
    }
    /**
     * 
     * @param int $offset
     * @return \BizStation\Transactd\Fielddef
     * @throws \OutOfRangeException
     */
    public function offsetGet($offset)
    {
        if ($offset < 0 || $offset >= fielddefs_size($this->cPtr)) {
            throw new \OutOfRangeException();
        }
        $r = fielddefs_getFielddef($this->cPtr, $offset);
        if (is_resource($r)) {
            return new Fielddef($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $offset
     * @param \BizStation\Transactd\Fielddef $value
     * @throws \BadMethodCallException
     */
    public function offsetSet($offset, $value)
    {
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @param int $offset
     * @throws \BadMethodCallException
     */
    public function offsetUnset($offset)
    {
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @return int
     */
    public function count()
    {
        return fielddefs_size($this->cPtr);
    }
    /**
     * 
     * @param int $start
     * @param int $end
     * @return \BizStation\Transactd\FielddefsIterator
     */
    public function range($start = null, $end = null)
    {
        $count = fielddefs_size($this->cPtr);
        if ($start < 0) {
            $start = 0;
        }
        if ($end < 0 || $end > $count) {
            $end = $count;
        }
        return new FielddefsIterator($this->cPtr, (int)$start, (int)$end);
    }

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }

    public function __clone()
    {
        $r=fielddefs___clone($this->cPtr);
        $this->__construct($r);
    }
    /**
     * 
     * @param string $name
     * @return int
     */
    public function indexByName($name)
    {
        return fielddefs_indexByName($this->cPtr, $name);
    }
    /**
     * 
     * @param int|string $index_or_name
     * @return \BizStation\Transactd\Fielddef
     */
    public function getFielddef($index_or_name)
    {
        $r=fielddefs_getFielddef($this->cPtr, $index_or_name);
        if (is_resource($r)) {
            return new Fielddef($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $index
     * @return bool
     */
    public function checkIndex($index)
    {
        return fielddefs_checkIndex($this->cPtr, $index);
    }
    /**
     * 
     * @return int
     */
    public function size()
    {
        return fielddefs_size($this->cPtr);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__fielddefs') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_fielddefs();
    }
    /**
     * 
     * @return array
     */
    public function toArray()
    {
        return fielddefs_toArray($this->cPtr);
    }
}

class Field
{
    public $cPtr=null;
    /**
     * 
     * @return mixed
     */
    public function getFV()
    {
        if ((transactd::nullValueMode() === transactd::NULLVALUE_MODE_RETURNNULL)
                && $this->isNull() === true) {
            return null;
        }
        switch ($this->type()) {
            case transactd::ft_integer:
            case transactd::ft_uinteger:
            case transactd::ft_autoinc:
            case transactd::ft_autoIncUnsigned:
            case transactd::ft_logical:
            case transactd::ft_bit:
                return field_i64($this->cPtr);
            case transactd::ft_float:
            case transactd::ft_decimal:
            case transactd::ft_money:
            case transactd::ft_numeric:
            case transactd::ft_bfloat:
            case transactd::ft_numericsts:
            case transactd::ft_numericsa:
            case transactd::ft_currency:
                return field_d($this->cPtr);
            default:
                return field_c_str($this->cPtr);
        }
        return null;
    }

    public function __set($var, $value)
    {
        if ($var === 'value') {
            return field_setFV($this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        throw new \BadMethodCallException();
    }

    public function __get($var)
    {
        if ($var === 'value') {
            return getFV();
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        throw new \BadMethodCallException();
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return false;
    }

    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__field') {
            $this->cPtr=$res;
            return;
        }
        if (func_num_args() == 0) {
            $this->cPtr=new_field();
        } else {
            $this->cPtr=new_field($res);
        }
    }

    public function __toString()
    {
        return field_c_str($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function type()
    {
        return field_type($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function len()
    {
        return field_len($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isNull()
    {
        return field_isNull($this->cPtr);
    }
    /**
     * 
     * @param bool $v
     */
    public function setNull($v)
    {
        field_setNull($this->cPtr, $v);
    }
    /**
     * 
     * @param mixed $p_or_v_or_data
     * @param int $size (optional) For string value.
     */
    public function setFV($p_or_v_or_data, $size=null)
    {
        switch (func_num_args()) {
        case 1: field_setFV($this->cPtr, $p_or_v_or_data); break;
        default: field_setFV($this->cPtr, $p_or_v_or_data, $size);
        }
    }
    /**
     * 
     * @return string
     */
    public function getBin()
    {
        return field_getBin($this->cPtr);
    }
    /**
     * 
     * @param string $str
     */
    public function setBin($str)
    {
        field_setFV($this->cPtr, $str, strlen($str));
    }
    /**
     * 
     * @param \BizStation\Transactd\Field $field
     * @param int $logType
     * @return int
     */
    public function comp($field, $logType=16)
    {
        return field_comp($this->cPtr, $field, $logType);
    }
    /**
     * 
     * @return \BizStation\Transactd\Bitset
     */
    public function getBits()
    {
        $r=field_getBits($this->cPtr);
        if (is_resource($r)) {
            $c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
            if (class_exists($c)) {
                return new $c($r);
            }
            return new Bitset($r);
        }
        return $r;
    }
    /**
     * 
     * @return int
     */
    public function i()
    {
        return field_i($this->cPtr);
    }
    /**
     * 
     * @return int|string
     */
    public function i64()
    {
        return field_i64($this->cPtr);
    }
    /**
     * 
     * @return double
     */
    public function d()
    {
        return field_d($this->cPtr);
    }
    /**
     * 
     * @return string
     */
    public function str()
    {
        return field_c_str($this->cPtr);
    }
    /**
     * 
     * @return string
     */
    public function bin()
    {
        return field_getBin($this->cPtr);
    }
    /**
     * 
     * @param mixed $p_or_v_or_data
     * @param int $size (optional) For string value.
     */    
    public function setValue($p_or_v_or_data, $size=null)
    {
        switch (func_num_args()) {
        case 1: field_setFV($this->cPtr, $p_or_v_or_data); break;
        default: field_setFV($this->cPtr, $p_or_v_or_data, $size);
        }
    }
}

class RecordIterator implements \Iterator
{
    private $recordPtr = null;
    private $curIndex = 0;
    private $count = -1;
    private $field = null;
    private $fielddefs = null;
    /**
     * 
     * @param \BizStation\Transactd\Record $record_cPtr
     * @param \BizStation\Transactd\Fielddefs $fielddefs
     */
    public function __construct($record_cPtr, $fielddefs)
    {
        $this->recordPtr = $record_cPtr;
        $this->curIndex = 0;
        $this->count = Record_size($record_cPtr);
        $this->fielddefs = $fielddefs;
        $this->field = new Field();
    }
    
    public function rewind()
    {
        $this->curIndex = 0;
    }
    /**
     * 
     * @return bool
     */
    public function valid()
    {
        return $this->curIndex < $this->count;
    }
    /**
     * 
     * @return mixed
     */
    public function current()
    {
        if (transactd::$fieldValueMode === transactd::FIELD_VALUE_MODE_VALUE) {
            return Record_getFV($this->recordPtr, $this->curIndex);
        }
        Record_getFieldRef($this->recordPtr, $this->curIndex, $this->field);
        return $this->field;
    }
    /**
     * 
     * @return string
     */
    public function key()
    {
        return $this->fielddefs->getFielddef($this->curIndex)->name();
    }

    public function next()
    {
        $this->curIndex++;
    }
}

class Record implements \ArrayAccess, \Countable, \IteratorAggregate
{
    protected $field = null;
    protected $fielddefs = null;

    public function __clone()
    {
        $this->field = new Field();
    }
    /**
     * 
     * @return \BizStation\Transactd\RecordIterator
     */
    public function getIterator()
    {
        return new RecordIterator($this->cPtr, $this->fielddefs);
    }
    /**
     * 
     * @param int $offset
     * @return bool
     */
    public function offsetExists($offset)
    {
        switch (\gettype($offset)) {
            case "integer":
                return $offset >= 0 && $offset < $this->count();
            case "string":
                return Record_indexByName($this->cPtr, $offset) >= 0;
            default:
                return false;
        }
    }
    /**
     * 
     * @param int $offset
     * @return mixed
     */
    public function offsetGet($offset)
    {
        if (transactd::$fieldValueMode === transactd::FIELD_VALUE_MODE_VALUE) {
            return Record_getFV($this->cPtr, $offset);
        }
        Record_getFieldRef($this->cPtr, $offset, $this->field);
        return $this->field;
    }
    /**
     * 
     * @param int $offset
     * @param mixed $value
     * @throws \BadMethodCallException
     */
    public function offsetSet($offset, $value)
    {
        /*if (transactd::$fieldValueMode === transactd::FIELD_VALUE_MODE_VALUE) {
            return Record_setFV($this->cPtr, $offset, $value);
        }*/
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @param int $offset
     * @throws \BadMethodCallException
     */
    public function offsetUnset($offset)
    {
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @return int
     */
    public function count()
    {
        return Record_size($this->cPtr);
    }
    /**
     * 
     * @return \BizStation\Transactd\FielddefsIterator
     */
    public function keys()
    {
        return new FielddefsIterator($this->fielddefs, 0, Record_size($this->cPtr));
    }
    /**
     * 
     * @return \BizStation\Transactd\RecordIterator
     */
    public function values()
    {
        return new RecordIterator($this->cPtr, $this->fielddefs);
    }

    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        throw new \BadMethodCallException();
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        throw new \BadMethodCallException();
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return false;
    }
    /**
     * 
     * @param resource $h
     * @param \BizStation\Transactd\Fielddefs $fielddefsPtr (optional)
     */
    public function __construct($h, $fielddefsPtr=null)
    {
        $this->cPtr=$h;
        if ($fielddefsPtr === null) {
            $fielddefsPtr=Record_fieldDefs($this->cPtr);
        } else {
            $this->fielddefs = $fielddefsPtr;
        }
        $this->field = new Field();
    }
    /**
     * 
     * @return bool
     */
    public function isInvalidRecord()
    {
        return Record_isInvalidRecord($this->cPtr);
    }
    /**
     * 
     * @param int|string $index_or_name
     * @return \BizStation\Transactd\Field
     */
    public function getField($index_or_name)
    {
        $r=Record_getField($this->cPtr, $index_or_name);
        if (is_resource($r)) {
            return new Field($r);
        }
        return $r;
    }
    /**
     * 
     * @return int
     */
    public function size()
    {
        return Record_size($this->cPtr);
    }
    /**
     * 
     * @param string $name
     * @return int
     */
    public function indexByName($name)
    {
        return Record_indexByName($this->cPtr, $name);
    }
    /**
     * 
     * @return \BizStation\Transactd\Fielddefs
     */
    public function fieldDefs()
    {
        return $this->fielddefs;
    }

    public function clear()
    {
        Record_clear($this->cPtr);
    }
    /**
     * 
     * @param object $obj
     */
    public function setValueByObject($obj)
    {
        Record_setValueByObject($this->cPtr, $obj);
    }
}

class WritableRecord extends Record
{
    /**
     * 
     * @param int $offset
     * @param mixed $value
     */
    public function offsetSet($offset, $value)
    {
        Record_getFieldRef($this->cPtr, $offset, $this->field);
        $this->field->setFV($value);
    }

    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        Record::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return Record::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return Record::__isset($var);
    }
    /**
     * 
     * @param resource $h
     */
    public function __construct($h)
    {
        $this->cPtr=$h;
        $this->fielddefs = $this->fieldDefs();
        $this->field = new Field();
    }
    /**
     * 
     * @param bool|\BizStation\Transactd\Bookmark $KeysetAlrady_or_bm (optional)
     * @return bool
     */
    public function read($KeysetAlrady_or_bm=false)
    {
        return writableRecord_read($this->cPtr, $KeysetAlrady_or_bm);
    }

    public function insert()
    {
        writableRecord_insert($this->cPtr);
    }
    /**
     * 
     * @param bool $KeysetAlrady (optional)
     * @param bool $noSeek (optional)
     */
    public function del($KeysetAlrady=false, $noSeek = false)
    {
        writableRecord_del($this->cPtr, $KeysetAlrady, $noSeek);
    }
    /**
     * 
     * @param bool $KeysetAlrady (optional)
     * @param bool $noSeek (optional)
     */
    public function update($KeysetAlrady = false, $noSeek = false)
    {
        writableRecord_update($this->cPtr, $KeysetAlrady, $noSeek);
    }

    public function save()
    {
        writableRecord_save($this->cPtr);
    }
}

class ConnectParams
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @param string|resource $protocol_or_uri
     * @param string $hostOrIp
     * @param string $dbname (optional)
     * @param string $schemaTable (optional)
     * @param string $username (optional)
     * @param string $passwd (optional)
     */
    public function __construct($protocol_or_uri, $hostOrIp=null, $dbname=null, $schemaTable=null,
                            $username=null, $passwd=null)
    {
        if (is_resource($protocol_or_uri) && get_resource_type($protocol_or_uri) === '_p_bzs__client__connectParams') {
            $this->cPtr=$protocol_or_uri;
            return;
        }
        switch (func_num_args()) {
        case 1: $this->cPtr=new_connectParams($protocol_or_uri); break;
        case 2: $this->cPtr=new_connectParams($protocol_or_uri, $hostOrIp); break;
        case 3: $this->cPtr=new_connectParams($protocol_or_uri, $hostOrIp, $dbname); break;
        case 4: $this->cPtr=new_connectParams($protocol_or_uri, $hostOrIp, $dbname, $schemaTable);break;
        case 5: $this->cPtr=new_connectParams($protocol_or_uri, $hostOrIp, $dbname, $schemaTable, $username);break;
        default: $this->cPtr=new_connectParams($protocol_or_uri, $hostOrIp, $dbname, $schemaTable, $username, $passwd);
        }
    }
    /**
     * 
     * @param int $v
     */
    public function setMode($v)
    {
        connectParams_setMode($this->cPtr, $v);
    }
    /**
     * 
     * @param int $v
     */
    public function setType($v)
    {
        connectParams_setType($this->cPtr, $v);
    }
    /**
     * 
     * @return string
     */
    public function uri()
    {
        return connectParams_uri($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function mode()
    {
        return connectParams_mode($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function type()
    {
        return connectParams_type($this->cPtr);
    }
}

class FieldNames
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @return \BizStation\Transactd\FieldNames
     */
    public function reset()
    {
        fieldNames_reset($this->cPtr);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $name1 (optional)
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @param string $name9 (optional)
     * @param string $name10 (optional)
     * @return \BizStation\Transactd\FieldNames
     */
    public function keyField($name, $name1=null, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null, $name9=null, $name10=null)
    {
        fieldNames_keyField($this->cPtr, $name, $name1, $name2, $name3, $name4, $name5, $name6, $name7, $name8, $name9, $name10);
        return $this;
    }
    /**
     * 
     * @return int
     */
    public function count()
    {
        return fieldNames_count($this->cPtr);
    }
    /**
     * 
     * @param int $index
     * @return string
     */
    public function getFieldName($index)
    {
        return fieldNames_getFieldName($this->cPtr, $index);
    }
    /**
     * 
     * @param int $index
     * @return string
     */
    public function getValue($index)
    {
        return fieldNames_getValue($this->cPtr, $index);
    }
    /**
     * 
     * @param string $v
     */
    public function addValue($v)
    {
        fieldNames_addValue($this->cPtr, $v);
    }
    /**
     * 
     * @param string $values
     * @param string $delmi
     */
    public function addValues($values, $delmi)
    {
        fieldNames_addValues($this->cPtr, $values, $delmi);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__fieldNames') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_fieldNames();
    }
}
/**
 * @property bool $asc
 * @property string $name
 */

class SortField
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'name') {
            return sortField_name_set($this->cPtr, $value);
        }
        if ($var === 'asc') {
            return sortField_asc_set($this->cPtr, $value);
        }
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'name') {
            return sortField_name_get($this->cPtr);
        }
        if ($var === 'asc') {
            return sortField_asc_get($this->cPtr);
        }
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if (function_exists('sortField_'.$var.'_get')) {
            return true;
        }
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__sortField') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_sortField();
    }
}

class SortFields
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @param string $name
     * @param bool $asc
     * @return \BizStation\Transactd\SortFields
     */
    public function add($name, $asc)
    {
        sortFields_add($this->cPtr, $name, $asc);
        return $this;
    }
    /**
     * 
     * @return int
     */
    public function size()
    {
        return sortFields_size($this->cPtr);
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\SortField
     */
    public function getSortField($index)
    {
        $r=sortFields_getSortField($this->cPtr, $index);
        if (is_resource($r)) {
            $c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
            if (class_exists($c)) {
                return new $c($r);
            }
            return new SortField($r);
        }
        return $r;
    }
    /**
     * 
     * @return \BizStation\Transactd\SortFields
     */
    public function clear()
    {
        sortFields_clear($this->cPtr);
        return $this;
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__sortFields') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_sortFields();
    }
}

class RecordsetQuery
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function reset()
    {
        recordsetQuery_reset($this->cPtr);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function whenIsNull($name)
    {
        recordsetQuery_whenIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function whenIsNotNull($name)
    {
        recordsetQuery_whenIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function andIsNull($name)
    {
        recordsetQuery_andIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function andIsNotNull($name)
    {
        recordsetQuery_andIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function orIsNull($name)
    {
        recordsetQuery_orIsNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function orIsNotNull($name)
    {
        recordsetQuery_orIsNotNull($this->cPtr, $name);
        return $this;
    }
    /**
     * 
     * @return string
     */
    public function toString()
    {
        return recordsetQuery_toString($this->cPtr);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__recordsetQuery') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_recordsetQuery();
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param mixed $value
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function when($name, $qlogic, $value)
    {
        recordsetQuery_when($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param mixed $value
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function and_($name, $qlogic, $value)
    {
        recordsetQuery_and_($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $qlogic
     * @param mixed $value
     * @return \BizStation\Transactd\RecordsetQuery
     */
    public function or_($name, $qlogic, $value)
    {
        recordsetQuery_or_($this->cPtr, $name, $qlogic, $value);
        return $this;
    }
}

abstract class GroupFuncBase extends RecordsetQuery
{
    public $cPtr=null;
    protected $resultName = '';

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        recordsetQuery::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return recordsetQuery::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return recordsetQuery::__isset($var);
    }
    /**
     * 
     * @param resource $h
     */
    public function __construct($h)
    {
        $this->cPtr=$h;
        parent::__construct($h);
    }
    /**
     * 
     * @return \BizStation\Transactd\FieldNames
     */
    public function targetNames()
    {
        $r=groupFuncBase_targetNames($this->cPtr);
        if (is_resource($r)) {
            return new FieldNames($r);
        }
        return $r;
    }
    /**
     * 
     * @return string
     */
    public function resultName()
    {
        return groupFuncBase_resultName($this->cPtr);
    }
    /**
     * 
     * @param string $v
     */
    public function setResultName($v)
    {
        $this->resultName = $v;
        groupFuncBase_setResultName($this->cPtr, $this->resultName);
    }
    /**
     * 
     * @return int
     */
    public function resultKey()
    {
        return groupFuncBase_resultKey($this->cPtr);
    }
    
    public function reset()
    {
        groupFuncBase_reset($this->cPtr);
    }

    public function __clone()
    {
        $r=groupFuncBase___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
}

class GroupQuery
{
    public $cPtr=null;
    protected $pData=array();
    protected $funcs = array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @return \BizStation\Transactd\GroupQuery
     */
    public function reset()
    {
        groupQuery_reset($this->cPtr);
        $this->funcs = array();
        return $this;
    }
    /**
     * 
     * @param \BizStation\Transactd\GroupFuncBase  $func
     * @return \BizStation\Transactd\GroupQuery
     */
    public function addFunction($func)
    {
        groupQuery_addFunction($this->cPtr, $func);
        array_push($this->funcs, $func);
        return $this;
    }
    /**
     * 
     * @param string $name
     * @param string $name1 (optional)
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @param string $name9 (optional)
     * @param string $name10 (optional)
     * @return \BizStation\Transactd\GroupQuery
     */
    public function keyField($name, $name1=null, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null, $name9=null, $name10=null)
    {
        groupQuery_keyField($this->cPtr, $name, $name1, $name2, $name3, $name4, $name5, $name6, $name7, $name8, $name9, $name10);
        return $this;
    }
    /**
     * 
     * @return \BizStation\Transactd\FieldNames
     */
    public function getKeyFields()
    {
        $r=groupQuery_getKeyFields($this->cPtr);
        if (is_resource($r)) {
            return new FieldNames($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\GroupFuncBase
     */
    public function getFunction($index)
    {
        return $this->funcs[$index];
    }
    /**
     * 
     * @return int
     */
    public function functionCount()
    {
        return groupQuery_functionCount($this->cPtr);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__groupQuery') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_groupQuery();
    }
}

class Sum extends GroupFuncBase
{
    public $cPtr=null;
    protected $targetNames = null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        groupFuncBase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return groupFuncBase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return groupFuncBase::__isset($var);
    }

    public function __clone()
    {
        $r=sum___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__sum') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_sum($this->targetNames); break;
        default: $this->cPtr=new_sum($this->targetNames, $this->resultName);
        }
    }
}

class First extends GroupFuncBase
{
    public $cPtr=null;
    protected $targetNames = null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        groupFuncBase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return groupFuncBase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return groupFuncBase::__isset($var);
    }

    public function __clone()
    {
        $r=first___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__first') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_first($this->targetNames); break;
        default: $this->cPtr=new_first($this->targetNames, $this->resultName);
        }
    }
}

class Last extends GroupFuncBase
{
    public $cPtr=null;
    protected $targetNames = null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        groupFuncBase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return groupFuncBase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return groupFuncBase::__isset($var);
    }

    public function __clone()
    {
        $r=last___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__last') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_last($this->targetNames); break;
        default: $this->cPtr=new_last($this->targetNames, $this->resultName);
        }
    }
}

class Count extends GroupFuncBase
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        groupFuncBase::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return groupFuncBase::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return groupFuncBase::__isset($var);
    }

    public function __clone()
    {
        $r=count___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $resultName
     */
    public function __construct($resultName)
    {
        if (is_resource($resultName) && get_resource_type($resultName) === '_p_bzs__client__count') {
            $this->cPtr=$resultName;
            return;
        }
        $this->resultName = $resultName;
        $this->cPtr=new_count($this->resultName);
    }
}

class Avg extends Sum
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        sum::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return sum::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return sum::__isset($var);
    }

    public function __clone()
    {
        $r=avg___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__avg') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_avg($this->targetNames); break;
        default: $this->cPtr=new_avg($this->targetNames, $this->resultName);
        }
    }
}

class Min extends Sum
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        sum::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return sum::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return sum::__isset($var);
    }

    public function __clone()
    {
        $r=min___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__min') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_min($this->targetNames); break;
        default: $this->cPtr=new_min($this->targetNames, $this->resultName);
        }
    }
}

class Max extends Sum
{
    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        sum::__set($var, $value);
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return sum::__get($var);
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return sum::__isset($var);
    }

    public function __clone()
    {
        $r=max___clone($this->cPtr);
        $this->cPtr = $r;
        return $this;
    }
    /**
     * 
     * @param string $targetNames
     * @param string $resultName (optional)
     */
    public function __construct($targetNames, $resultName=null)
    {
        if (is_resource($targetNames) && get_resource_type($targetNames) === '_p_bzs__client__max') {
            $this->cPtr=$targetNames;
            return;
        }
        $this->targetNames = $targetNames;
        $this->resultName = $resultName;
        switch (func_num_args()) {
        case 1: $this->cPtr=new_max($this->targetNames); break;
        default: $this->cPtr=new_max($this->targetNames, $this->resultName);
        }
    }
}


class RecordsetIterator extends RangeIterator
{
    private $recordset = null;
    /**
     * 
     * @param \BizStation\Transactd\Recordset $recordset
     * @param int $start
     * @param int $end
     */
    public function __construct($recordset, $start, $end)
    {
        $this->recordset = $recordset;
        parent::__construct($start, $end);
    }
    /**
     * 
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the Recordset::fetchMode
     */
    public function current()
    {
        return $this->recordset->offsetGet($this->curIndex);
    }
}


class Recordset implements \ArrayAccess, \Countable, \IteratorAggregate
{
    private $record = null;
    private $fielddefs = null;

    public $fetchMode = transactd::FETCH_RECORD_INTO;
    public $fetchClass = 'stdClass';
    public $ctorArgs = null;

    /**
     * 
     * @return array
     */
    public function toArray()
    {
        return Recordset_toArray($this->cPtr, $this->fetchMode, $this->fetchClass, $this->ctorArgs);
    }
    /**
     * 
     * @return \BizStation\Transactd\RecordsetIterator
     */
    public function getIterator()
    {
        return new RecordsetIterator($this, 0, Recordset_count($this->cPtr));
    }
    /**
     * 
     * @param int $start
     * @param int $end
     * @return \BizStation\Transactd\RecordsetIterator
     */
    public function range($start = null, $end = null)
    {
        $count = Recordset_count($this->cPtr);
        if ($start < 0) {
            $start = 0;
        }
        if ($end < 0 || $end > $count) {
            $end = $count;
        }
        return new RecordsetIterator($this, (int)$start, (int)$end);
    }
    /**
     * 
     * @param int $offset
     * @return bool
     */
    public function offsetExists($offset)
    {
        return $offset >= 0 && $offset < $this->count();
    }
    /**
     * 
     * @param int $offset
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     */
    public function offsetGet($offset)
    {
        if ($this->fetchMode === transactd::FETCH_RECORD_INTO) {
            return Recordset_getRow($this->cPtr, $offset, transactd::FETCH_RECORD_INTO, $this->record);
        }
        return Recordset_getRow($this->cPtr, $offset, $this->fetchMode, $this->fetchClass, $this->ctorArgs);
    }
    /**
     * 
     * @param int $offset
     * @param mixed $value
     * @throws \BadMethodCallException
     */
    public function offsetSet($offset, $value)
    {
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @param int $offset
     * @throws \BadMethodCallException
     */
    public function offsetUnset($offset)
    {
        throw new \BadMethodCallException();
    }
    /**
     * 
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     * @throws \OutOfBoundsException
     */
    public function first()
    {
        if ($this->count() <= 0) {
            throw new \OutOfBoundsException('no records in recordset');
        }
        return $this->offsetGet(0);
    }
    /**
     * 
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     * @throws \OutOfBoundsException
     */
    public function last()
    {
        if ($this->count() <= 0) {
            throw new \OutOfBoundsException('no records in recordset');
        }
        return $this->offsetGet($this->count() - 1);
    }

    public $cPtr=null;

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        throw new \BadMethodCallException();
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        throw new \BadMethodCallException();
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return false;
    }

    public function __clone()
    {
        $r=Recordset___clone($this->cPtr);
        if (is_resource($r)) {
            $this->cPtr = $r;
        } else {
            $this->cPtr = $r->cPtr;
        }
        $this->record = new Record(memoryRecord_createRecord($this->fielddefs), $this->fielddefs);
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\Record
     */
    public function getRecord($index)
    {
        return Recordset_getRow($this->cPtr, $index, transactd::FETCH_RECORD_INTO, $this->record);
    }
    /**
     * 
     * @param int $index
     * @return \BizStation\Transactd\Record|array|object The record in the type specified by the $this->fetchMode
     */
    public function getRow($index)
    {
        return Recordset_getRow($this->cPtr, $index, $this->fetchMode, $this->fetchClass, $this->ctorArgs);
    }
    /**
     * 
     * @return int
     */
    public function size()
    {
        return Recordset_size($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function count()
    {
        return Recordset_count($this->cPtr);
    }

    public function clearRecords()
    {
        Recordset_clearRecords($this->cPtr);
    }
    /**
     * 
     * @return \BizStation\Transactd\Fielddef
     */
    public function fieldDefs()
    {
        return $this->fielddefs;
    }

    public function clear()
    {
        Recordset_clear($this->cPtr);
    }
    /**
     * 
     * @param \BizStation\Transactd\Recordset $recordset
     * @param int $n
     * @return \BizStation\Transactd\Recordset
     */
    public function top($recordset, $n)
    {
        $r=Recordset_top($this->cPtr, $recordset, $n);
        if (is_resource($r)) {
            return new Recordset($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $index
     */
    public function erase($index)
    {
        Recordset_erase($this->cPtr, $index);
    }
    /**
     * 
     * @param int $index
     */
    public function removeField($index)
    {
        Recordset_removeField($this->cPtr, $index);
    }
    /**
     * @param \BizStation\Transactd\Recordset $rs
     * @param \BizStation\Transactd\RecordsetQuery $rq
     * @return \BizStation\Transactd\Recordset
     */
    public function join($rs, $rq)
    {
        Recordset_join($this->cPtr, $rs, $rq);
        return $this;
    }
    /**
     * @param \BizStation\Transactd\Recordset $rs
     * @param \BizStation\Transactd\RecordsetQuery $rq
     * @return \BizStation\Transactd\Recordset
     */
    public function outerJoin($rs, $rq)
    {
        Recordset_outerJoin($this->cPtr, $rs, $rq);
        return $this;
    }
    /**
     * 
     * @param \BizStation\Transactd\RecordsetQuery $rq
     * @return \BizStation\Transactd\Recordset
     */
    public function matchBy($rq)
    {
        Recordset_matchBy($this->cPtr, $rq);
        return $this;
    }
    /**
     * 
     * @param \BizStation\Transactd\GroupQuery $gq
     * @return \BizStation\Transactd\Recordset
     */
    public function groupBy($gq)
    {
        Recordset_groupBy($this->cPtr, $gq);
        return $this;
    }
    /**
     * 
     * @param \BizStation\Transactd\SortFields|string $name1_or_orders
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @return \BizStation\Transactd\Recordset
     */
    public function orderBy($name1_or_orders, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null)
    {
        if (Recordset_size($this->cPtr) < 2) {
            return $this;
        }
        Recordset_orderBy($this->cPtr, $name1_or_orders, $name2, $name3, $name4, $name5, $name6, $name7, $name8);
        return $this;
    }
    /**
     * 
     * @return \BizStation\Transactd\Recordset
     */
    public function reverse()
    {
        Recordset_reverse($this->cPtr);
        return $this;
    }
    /**
     * 
     * @param string|\BizStation\Transactd\Fielddef $nameOrFielddef
     * @param int $type
     * @param int $len
     */
    public function appendField($nameOrFielddef, $type=null, $len=null)
    {
        if (is_string($nameOrFielddef)) {
            if (is_null($type) || is_null($len)) {
                throw new \InvalidArgumentException();
            }
            Recordset_appendField($this->cPtr, $nameOrFielddef, $type, $len);
        } else {
            Recordset_appendField($this->cPtr, $nameOrFielddef);
        }
    }
    /**
     * 
     * @param \BizStation\Transactd\Recordset $recordset
     * @return \BizStation\Transactd\Recordset
     */
    public function unionRecordset($recordset)
    {
        Recordset_unionRecordset($this->cPtr, $recordset);
        return $this;
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__recordset') {
            $this->cPtr=$res;
            $this->fielddefs = new Fielddefs(Recordset_fieldDefs($this->cPtr));
            $this->record = new Record(memoryRecord_createRecord($this->fielddefs), $this->fielddefs);
            return;
        }
        $this->cPtr=new_Recordset();
        $this->fielddefs = new Fielddefs(Recordset_fieldDefs($this->cPtr));
        $this->record = new Record(memoryRecord_createRecord($this->fielddefs), $this->fielddefs);
    }
}

class PreparedQuery
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     *
     * @param resource|null $res
     */
    public function __construct($res)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__preparedQuery') {
            $this->cPtr=$res;
            return;
        }
        $this->cPtr=new_preparedQuery($res);
    }
    /**
     * 
     * @param int $index
     * @param mixed $v
     * @return bool
     */
    public function supplyValue($index, $v)
    {
        return preparedQuery_supplyValue($this->cPtr, $index, $v);
    }
    /**
     * 
     * @param mixed $v
     * @return bool
     */
    public function addValue($v)
    {
        return preparedQuery_addValue($this->cPtr, $v);
    }

    public function resetAddIndex()
    {
        preparedQuery_resetAddIndex($this->cPtr);
    }
}

class ActiveTable
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @param string $orign
     * @param string $alias
     * @return \BizStation\Transactd\ActiveTable
     */
    public function alias($orign, $alias)
    {
        activeTable_alias($this->cPtr, $orign, $alias);
        return $this;
    }
    /**
     * 
     * @return \BizStation\Transactd\ActiveTable
     */
    public function resetAlias()
    {
        activeTable_resetAlias($this->cPtr);
        return $this;
    }
    /**
     * 
     * @return \BizStation\Transactd\WritableRecord
     */
    public function getWritableRecord()
    {
        $r=activeTable_getWritableRecord($this->cPtr);
        if (is_resource($r)) {
            return new WritableRecord($r);
        }
        return $r;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\ActiveTable
     */
    public function index($v)
    {
        activeTable_index($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param int $v
     * @return \BizStation\Transactd\ActiveTable
     */
    public function option($v)
    {
        activeTable_option($this->cPtr, $v);
        return $this;
    }
    /**
     * 
     * @param \BizStation\Transactd\QueryBase|\BizStation\Transactd\PreparedQuery $q
     * @param mixed $v0 (optional) Ssupply value for prepared query.
     * @param mixed $v1 (optional) Ssupply value for prepared query.
     * @param mixed $v2 (optional) Ssupply value for prepared query.
     * @param mixed $v3 (optional) Ssupply value for prepared query.
     * @param mixed $v4 (optional) Ssupply value for prepared query.
     * @param mixed $v5 (optional) Ssupply value for prepared query.
     * @param mixed $v6 (optional) Ssupply value for prepared query.
     * @param mixed $v7 (optional) Ssupply value for prepared query.
     * @return \BizStation\Transactd\Recordset
     */
    public function read($q, $v0=null, $v1=null, $v2=null, $v3=null, $v4=null, $v5=null, $v6=null, $v7=null)
    {
        $r=activeTable_read($this->cPtr, $q, $v0, $v1, $v2, $v3, $v4, $v5, $v6, $v7);
        if (is_resource($r)) {
            return new Recordset($r);
        }
        return $r;
    }
    /**
     * 
     * @return \BizStation\Transactd\Recordset
     */
    public function readMore()
    {
        $r = activeTable_readMore($this->cPtr);
        if (is_resource($r)) {
            return new Recordset($r);
        }
        return $r;
    }
    /**
     * 
     * @param \BizStation\Transactd\QueryBase $q
     * @param false $serverPrepare
     * @return \BizStation\Transactd\PreparedQuery
     */
    public function prepare($q, $serverPrepare=false)
    {
        $r=activeTable_prepare($this->cPtr, $q, $serverPrepare);
        if (is_resource($r)) {
            return new PreparedQuery($r);
        }
        return $r;
    }
    /**
     * 
     * @param \BizStation\Transactd\Recordset $rs
     * @param BizStation\Transactd\QueryBase|\BizStation\Transactd\PreparedQuery $q
     * @param string $name1
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @return \BizStation\Transactd\Recordset
     */
    public function join($rs, $q, $name1, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null)
    {
        activeTable_join($this->cPtr, $rs, $q, $name1, $name2, $name3, $name4, $name5, $name6, $name7, $name8);
        return $rs;
    }
    /**
     * 
     * @param \BizStation\Transactd\Recordset $rs
     * @param BizStation\Transactd\QueryBase|\BizStation\Transactd\PreparedQuery $q
     * @param string $name1
     * @param string $name2 (optional)
     * @param string $name3 (optional)
     * @param string $name4 (optional)
     * @param string $name5 (optional)
     * @param string $name6 (optional)
     * @param string $name7 (optional)
     * @param string $name8 (optional)
     * @return \BizStation\Transactd\Recordset
     */
    public function outerJoin($rs, $q, $name1, $name2=null, $name3=null, $name4=null, $name5=null, $name6=null, $name7=null, $name8=null)
    {
        activeTable_outerJoin($this->cPtr, $rs, $q, $name1, $name2, $name3, $name4, $name5, $name6, $name7, $name8);
        return $rs;
    }
    /**
     * 
     * @param \BizStation\Transactd\Database|\BizStation\Transactd\PooledDbManager $mgr_or_db
     * @param string $tableName
     * @param int $mode
     * @return type
     */
    public function __construct($mgr_or_db, $tableName, $mode=transactd::TD_OPEN_READONLY)
    {
        if (is_resource($mgr_or_db) && get_resource_type($mgr_or_db) === '_p_bzs__client__activeTable') {
            $this->cPtr=$mgr_or_db;
            return;
        }
        $this->cPtr=new_activeTable($mgr_or_db, $tableName, $mode);
    }

    public function release()
    {
        activeTable_release($this->cPtr);
    }
    /**
     * 
     * @return \BizStation\Transactd\Table
     */
    public function table()
    {
        $r=activeTable_table($this->cPtr);
        if (is_resource($r)) {
            return new Table($r);
        }
        return $r;
    }
    /**
     * 
     * @param mixed $kv0
     * @param mixed $kv1 (optional)
     * @param mixed $kv2 (optional)
     * @param mixed $kv3 (optional)
     * @param mixed $kv4 (optional)
     * @param mixed $kv5 (optional)
     * @param mixed $kv6 (optional)
     * @param mixed $kv7 (optional)
     * @return \BizStation\Transactd\ActiveTable
     */
    public function keyValue($kv0, $kv1=null, $kv2=null, $kv3=null, $kv4=null, $kv5=null, $kv6=null, $kv7=null)
    {
        activeTable_keyValue($this->cPtr, $kv0, $kv1, $kv2, $kv3, $kv4, $kv5, $kv6, $kv7);
        return $this;
    }
}

class PooledDbManager
{
    public $cPtr=null;
    protected $pData=array();

    public function __set($var, $value)
    {
        if ($var === 'thisown') {
            return swig_transactd_alter_newobject($this->cPtr, $value);
        }
        $this->pData[$var] = $value;
    }

    public function __get($var)
    {
        if ($var === 'thisown') {
            return swig_transactd_get_newobject($this->cPtr);
        }
        return $this->pData[$var];
    }

    public function __isset($var)
    {
        if ($var === 'thisown') {
            return true;
        }
        return array_key_exists($var, $this->pData);
    }
    /**
     * 
     * @param resource $res
     */
    public function __construct($res=null)
    {
        if (is_resource($res) && get_resource_type($res) === '_p_bzs__client__pooledDbManager') {
            $this->cPtr=$res;
            return;
        }
        switch (func_num_args()) {
        case 0: $this->cPtr=new_pooledDbManager(); break;
        default: $this->cPtr=new_pooledDbManager($res);
        }
    }
    /**
     * 
     * @param \BizStation\Transactd\ConnectParams  $param
     */
    public function c_use($param=null)
    {
        pooledDbManager_c_use($this->cPtr, $param);
    }

    public function unUse()
    {
        pooledDbManager_unUse($this->cPtr);
    }
    /**
     * 
     * @param int $v
     */
    public function reset($v)
    {
        pooledDbManager_reset($this->cPtr, $v);
    }
    /**
     * 
     * @return \BizStation\Transactd\Database
     */
    public function db()
    {
        $r=pooledDbManager_db($this->cPtr);
        if (is_resource($r)) {
            return new Database($r);
        }
        return $r;
    }
    /**
     * 
     * @return string
     */
    public function uri()
    {
        return pooledDbManager_uri($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function mode()
    {
        return pooledDbManager_mode($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function isOpened()
    {
        return pooledDbManager_isOpened($this->cPtr);
    }
    /**
     * 
     * @param mixed $v
     */
    public function setOption($v)
    {
        pooledDbManager_setOption($this->cPtr, $v);
    }
    /**
     * 
     * @return mixed
     */
    public function option()
    {
        return pooledDbManager_option($this->cPtr);
    }
    /**
     * 
     * @param int $bias
     */
    public function beginTrn($bias=null)
    {
        if ($bias === null) {
            pooledDbManager_beginTrn($this->cPtr);
        } else {
            pooledDbManager_beginTrn($this->cPtr, $bias);
        }
    }
    
    public function endTrn()
    {
        pooledDbManager_endTrn($this->cPtr);
    }

    public function abortTrn()
    {
        pooledDbManager_abortTrn($this->cPtr);
    }
    /**
     * 
     * @return bool
     */
    public function enableTrn()
    {
        return pooledDbManager_enableTrn($this->cPtr);
    }
    /**
     * 
     * @param int $bias
     * @return \BizStation\Transactd\BinlogPos
     */
    public function beginSnapshot($bias=CONSISTENT_READ)
    {
        $r=pooledDbManager_beginSnapshot($this->cPtr, $bias);
        if (is_resource($r)) {
            return new BinlogPos($r);
        }
        return $r;
    }

    public function endSnapshot()
    {
        pooledDbManager_endSnapshot($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function stat()
    {
        return pooledDbManager_stat($this->cPtr);
    }
    /**
     * 
     * @return int
     */
    public function clientID()
    {
        return pooledDbManager_clientID($this->cPtr);
    }
    /**
     * 
     * @param int $maxWorkerNum
     */
    public static function setMaxConnections($maxWorkerNum)
    {
        pooledDbManager_setMaxConnections($maxWorkerNum);
    }
    /**
     * 
     * @return int
     */
    public static function maxConnections()
    {
        return pooledDbManager_maxConnections();
    }
    /**
     * 
     * @param int $size
     * @param \BizStation\Transactd\ConnectParams  $param
     */
    public static function reserve($size, $param)
    {
        pooledDbManager_reserve($size, $param);
    }
    /**
     * 
     * @param string $name
     * @return \BizStation\Transactd\Table
     */
    public function table($name)
    {
        $r=pooledDbManager_table($this->cPtr, $name);
        if (is_resource($r)) {
            return new Table($r);
        }
        return $r;
    }
    /**
     * 
     * @return int
     */
    public function usingCount()
    {
        return pooledDbManager_usingCount($this->cPtr);
    }
}

class HaNameResolver
{
    /**
     * 
     * @param string $master
     * @param string $slaves
     * @param string $slaveHostsWithPort
     * @param int $slaveNum
     * @param string $userName
     * @param string $password
     * @param int $option
     * @return int
     */
    public static function start($master, $slaves, $slaveHostsWithPort, $slaveNum, $userName, $password, $option=0)
    {
        return haNameResolver_start($master, $slaves, $slaveHostsWithPort, $slaveNum, $userName, $password, $option);
    }
    /**
     * 
     * @param int $mysqlPort
     * @param int $transactdPort
     */
    public static function addPortMap($mysqlPort, $transactdPort)
    {
        haNameResolver_addPortMap($mysqlPort, $transactdPort);
    }
    
    public static function clearPortMap()
    {
        haNameResolver_clearPortMap();
    }

    public static function stop()
    {
        haNameResolver_stop();
    }
    /**
     * 
     * @return string
     */
    public static function master()
    {
        return haNameResolver_master();
    }
    /**
     * 
     * @return string
     */
    public static function slave()
    {
        return haNameResolver_slave();
    }
}
