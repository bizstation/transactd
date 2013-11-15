#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_DATABASESCHEMA_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_DATABASESCHEMA_H
/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <string>

namespace bzs
{
	
namespace db
{
	namespace engine{namespace mysql{class table;class database;}}	

namespace protocol
{
namespace tdap
{
namespace mysql
{

class schemaBuilder
{
	short insertMetaRecord(engine::mysql::table* mtb, engine::mysql::table* src, int id);
public:
	schemaBuilder();
	~schemaBuilder();
	short execute(engine::mysql::database* db, engine::mysql::table* mtb);
};

}//namespace mysql
}//namespace protocol
}//namespace db
}//namespace tdap
}//namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_DATABASESCHEMA_H
