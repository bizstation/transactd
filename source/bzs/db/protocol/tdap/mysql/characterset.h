#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_CHARACTERSET_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_CHARACTERSET_H
/*=================================================================
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
=================================================================*/

#include <bzs/env/compiler.h>



#define MAX_CHAR_INFO   41
#define CHARSET_LATIN1   1
#define CHARSET_CP850    4
#define CHARSET_ASCII    9
#define CHARSET_SJIS	11
#define CHARSET_UTF8	22
#define CHARSET_USC2	23
#define CHARSET_UTF8B4	30
#define CHARSET_UTF16LE	33
#define CHARSET_CP932	38
#define CHARSET_EUCJ	40

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace mysql
{

PACKAGE unsigned int charsize(int index);
PACKAGE const char* charsetName(int index);
PACKAGE unsigned int charsetIndex(const char* name);
PACKAGE unsigned int charsetIndex(unsigned short codePage);
PACKAGE unsigned int codePage(unsigned short charsetIndex);

}//namespace mysql
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_CHARACTERSET_H
