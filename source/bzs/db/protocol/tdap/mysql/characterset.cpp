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
#include "characterset.h"
#include <string.h>
#include <assert.h>
#ifdef __BCPLUSPLUS__
#pragma package(smart_init)
#endif

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

struct SCHARSET_INFO
{
	unsigned short codePage;
	char	name[12];
	char	charSize;

};


SCHARSET_INFO
charsetInfo[MAX_CHAR_INFO] =
{
{1252,"acp",1},         //0
{1252,"latin1",1},      //1
{950,"big5",2},  		//2
{850,"dec8",1},         //3
{850,"cp850",1},        //4
{0,"hp8",1},            //5
{20866,"koi8r",1},      //6
{28592,"latin2",1},     //7
{20107,"swe7",1},       //8
{1252,"ascii",1},       //9
{50220,"ujis",3},       //10
{932,"sjis",2},         //11
{1255,"hebrew",1},      //12
{0,"tis620",1},         //13
{949,"euckr",2},        //14
{20866,"koi8u",1},      //15
{936,"gb2312",2},       //16
{1253,"greek",1},       //17
{1250,"cp1250",1},      //18
{0,"gbk",2},            //19
{0,"latin5",1},         //20
{0,"armscii8",1},       //21
{65001,"utf8",3},       //22
{1200,"ucs2",2},        //23
{866,"cp866",1},        //24
{0,"keybcs2",1},        //25
{0,"macce",1},          //26
{0,"macroman",1},       //27
{852,"cp852",1},        //28
{0,"latin7",1},         //29
{65001,"utf8mb4",4},    //30
{1251,"cp1251",1},      //31
{1201,"utf16",4},       //32
{1200,"utf16le",4},     //33 //5.6.1 or upper
{1256,"cp1256",1},      //34
{1257,"cp1257",1},      //35
{12000,"utf32",4},      //36
{0,"binary",1},         //37
{932,"cp932",2},        //38
{0,"geostd8",1},        //39
{51932,"eucjpms",3}     //40
};
/* 5.5.28
+----------+----------------+---------------------+--------+
| Charset  | Description    | Default collation   | Maxlen |
+----------+----------------+---------------------+--------+
| big5     |                | big5_chinese_ci     |      2 |
| dec8     |                | dec8_swedish_ci     |      1 |
| cp850    |                | cp850_general_ci    |      1 |
| hp8      |                | hp8_english_ci      |      1 |
| koi8r    |                | koi8r_general_ci    |      1 |
| latin1   |                | latin1_swedish_ci   |      1 |
| latin2   |                | latin2_general_ci   |      1 |
| swe7     |                | swe7_swedish_ci     |      1 |
| ascii    |                | ascii_general_ci    |      1 |
| ujis     |                | ujis_japanese_ci    |      3 |
| sjis     |                | sjis_japanese_ci    |      2 |
| hebrew   |                | hebrew_general_ci   |      1 |
| tis620   |                | tis620_thai_ci      |      1 |
| euckr    |                | euckr_korean_ci     |      2 |
| koi8u    |                | koi8u_general_ci    |      1 |
| gb2312   |                | gb2312_chinese_ci   |      2 |
| greek    |                | greek_general_ci    |      1 |
| cp1250   |                | cp1250_general_ci   |      1 |
| gbk      |                | gbk_chinese_ci      |      2 |
| latin5   |                | latin5_turkish_ci   |      1 |
| armscii8 |                | armscii8_general_ci |      1 |
| utf8     |                | utf8_general_ci     |      3 |
| ucs2     |                | ucs2_general_ci     |      2 |
| cp866    |                | cp866_general_ci    |      1 |
| keybcs2  |                | keybcs2_general_ci  |      1 |
| macce    |                | macce_general_ci    |      1 |
| macroman |                | macroman_general_ci |      1 |
| cp852    |                | cp852_general_ci    |      1 |
| latin7   |                | latin7_general_ci   |      1 |
| utf8mb4  | UTF-8 Unicode  | utf8mb4_general_ci  |      4 |
| cp1251   |                | cp1251_general_ci   |      1 |
| utf16    | UTF-16 Unicode | utf16_general_ci    |      4 |
| cp1256   |                | cp1256_general_ci   |      1 |
| cp1257   |                | cp1257_general_ci   |      1 |
| utf32    | UTF-32 Unicode | utf32_general_ci    |      4 |
| binary   |                | binary              |      1 |
| geostd8  |                | geostd8_general_ci  |      1 |
| cp932    |                | cp932_japanese_ci   |      2 |
| eucjpms  |                | eucjpms_japanese_ci |      3 |
+----------+----------------+---------------------+--------+
*/

unsigned int charsize(int index)
{
	if (index < MAX_CHAR_INFO)
		return charsetInfo[index].charSize;
	return 1;
}

const char* charsetName(int index)
{
	if ((index > 0) && index < MAX_CHAR_INFO)
		return charsetInfo[index].name;
	assert(0);
	return "";
}

unsigned int charsetIndex(const char* name)
{
	for (int i=1;i<MAX_CHAR_INFO;++i)
	{
		if (strcmp(charsetInfo[i].name, name)==0)
			return i;
	}
	return -1;
}

unsigned int charsetIndex(unsigned short codePage)
{
	for (int i=1;i<MAX_CHAR_INFO;++i)
	{
		if (charsetInfo[i].codePage == codePage)
			return i;
	}
	return -1;
}

unsigned int codePage(unsigned short charsetIndex)
{
	if ((charsetIndex > 0) && charsetIndex < MAX_CHAR_INFO)
		return charsetInfo[charsetIndex].codePage;
	assert(0);
	return 0;
}

}//namespace mysql
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs
