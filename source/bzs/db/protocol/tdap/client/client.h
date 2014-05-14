#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
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
#include "sqlBuilder.h"
#include "request.h"
#include <bzs/db/blobBuffer.h>
#include <boost/thread/mutex.hpp>
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/netsvc/client/tcpClient.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/env/compiler.h>
#include <bzs/rtl/stl_uty.h>
#include <vector>
#ifdef _WIN32
#include <mbstring.h>
#else
#include <pthread.h> 
#endif

extern bzs::netsvc::client::connections* m_cons;

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{
#define TABELNAME_PREFIX "dbfile="

class client;
void setClientThread(client* v);




class client
{

	mutex m_mutex;
	request m_req;
	posblk m_tmpPbk;
	ushort_td m_op;
	ushort_td m_preResult;
	int m_charsetIndexServer;
	std::string m_sql;
	std::string m_serverCharData;
	std::string m_serverCharData2;
	uint_td m_tmplen;
	bool m_logout;
	blobBuffer m_blobBuffer;
	bool m_disconnected;

	std::vector<char> m_sendbuf;
	inline bzs::netsvc::client::connection* con(){return m_req.cid->con;};
	inline void setCon(bzs::netsvc::client::connection* con){m_req.cid->con=con;};
	
	inline void disconnect()
	{
		if (!con())
			m_req.result = 1;
		else
			m_disconnected = m_cons->disconnect(con());
		if (m_req.result==0)
			setCon(NULL);
	}
	std::string getHostName(const char* uri)
	{
		std::vector<std::string> ss ;
		split(ss, uri, "/"); /* btrv://serverName/dbName?dbfile=xxx */
		if (ss.size()<3)
			return "";
		return ss[2];
	}
	std::string getTableName(const char* uri)
	{
		if (uri)
		{
			mbchar* en = (mbchar*)uri;
			mbchar* st = _mbsstr(en, (mbchar*)TABELNAME_PREFIX);
			if (st)
			{
				st +=strlen(TABELNAME_PREFIX);
				mbchar* tmp = _mbsstr(st, (mbchar*)PSEPARATOR_A);
				if (tmp)
					st = tmp+1;
				en = _mbsstr(st, (mbchar*)".");
				if (en==NULL)
					en = st + strlen((char*)st);
				return std::string((const char*)st, en-st);
			}
		}
		return "";
	}
	static void addSecondCharsetData(unsigned int destCodePage, std::string& src);
public:
	client():m_charsetIndexServer(-1),m_disconnected(true)
	{
	}
	void cleanup()
	{
		if (m_disconnected)
			setClientThread(NULL);
	}
	request& req(){return m_req;}
	inline void setParam(ushort_td op, posblk*   pbk,
		   void_td* data, uint_td* datalen,
		   void_td* keybuf, keylen_td keylen,
		   char_td keyNum, clientID* cid)
		   {
			   m_op = op;
			   m_req.op = op;
			   m_req.pbk = pbk;
			   m_req.data = data;
			   m_req.datalen = datalen;
			   m_req.keybuf = keybuf;
			   m_req.keylen = keylen;
			   m_req.keyNum = keyNum;
			   m_req.cid = cid;
			   m_req.paramMask = 0;
			   m_preResult = 0;
			   m_logout = false;
			   if (m_req.pbk==NULL) m_req.pbk = &m_tmpPbk;

			}
	inline short_td result(){return m_preResult;}

	inline bool stop_if()
	{
		if (m_op==TD_STOP_ENGINE)
		{
			m_req.result = 0;
			disconnect();
			return !m_req.result;
		}
		return false;
	}
	inline void connect()
	{
		if (!m_req.cid->con)
		{
			std::string host = getHostName((const char*)m_req.keybuf);
			if (host=="")
				m_req.result = ERROR_TD_HOSTNAME_NOT_FOUND;
			setCon(m_cons->connect(host));  //if error throw exception

		}
		m_disconnected = !m_req.cid->con;
	}
	inline void create()
	{
		m_req.paramMask = P_MASK_ALL;
		if ((m_req.datalen==NULL) || *m_req.datalen == 0)
			m_req.paramMask = P_MASK_KEYONLY;
		else if (m_req.keyNum > CR_SUBOP_SWAPNAME) //-126 swap -127 is rename. -128 is drop.
		{
			m_req.paramMask &= ~P_MASK_POSBLK;
			std::string name = getTableName((const char*)m_req.keybuf);
			if ((m_req.keyNum==1)|| (m_req.keyNum==2)) //make by tabledef
			{
				m_sql = sqlCreateTable(name.c_str(), (tabledef*)m_req.data, m_charsetIndexServer);
				m_req.keyNum -= 2; // 1= exists check 2 = no exists check
			}else
				m_sql = sqlCreateTable(name.c_str(), (fileSpec*)m_req.data, m_charsetIndexServer);
			m_req.data =(ushort_td*) m_sql.c_str();
			m_tmplen = (uint_td)(m_sql.size()+1);
			m_req.datalen = &m_tmplen;
		}else if ((m_req.keyNum == CR_SUBOP_SWAPNAME)||(m_req.keyNum == CR_SUBOP_RENAME))
		{
			readServerCharsetIndex();
			m_sql = (char*)m_req.data;
			addSecondCharsetData(mysql::codePage(m_charsetIndexServer), m_sql);
			m_req.data =(ushort_td*) m_sql.c_str();
			m_tmplen = (uint_td)(m_sql.size()+1);
			m_req.datalen = &m_tmplen;
		}
	}
	inline void cmdConnect()
	{
		if ((m_req.keyNum == LG_SUBOP_CONNECT)|| (m_req.keyNum == LG_SUBOP_NEWCONNECT))
		{
			if (con() && con()->isConnected())
				m_preResult = 1;
			else
			{
				std::string host = getHostName((const char*)m_req.keybuf);
				if (host=="")
					m_preResult = ERROR_TD_HOSTNAME_NOT_FOUND;
				setCon(m_cons->connect(host, (m_req.keyNum == LG_SUBOP_NEWCONNECT))); //if error throw exception
				if (readServerCharsetIndex() == false)
					m_preResult = SERVER_CLIENT_NOT_COMPATIBLE;
				else
				    buildDualChasetKeybuf();
			}
		}else if (m_req.keyNum == LG_SUBOP_DISCONNECT)
		{
			if (con())
				m_logout = true;
			else if (m_op==TD_CONNECT)
				m_preResult = 1;
		}
		m_req.paramMask = P_MASK_KEYONLY;
	}
	inline ushort_td execute()
	{
		if (result()==0)
		{
			if (!con())
				m_preResult = ERROR_TD_NOT_CONNECTED;
			else
			{
				char* p = con()->sendBuffer(m_req.sendLenEstimate());
				unsigned int size = m_req.serialize(p);
				short stat = 0;
				if ((m_req.paramMask & P_MASK_BLOBBODY) && m_blobBuffer.blobs())
					size = m_req.serializeBlobBody(&m_blobBuffer
						, p, size, con()->sendBufferSize(), con()->optionalBuffers(), stat);
				if (stat == 0)
				{
					if (m_req.paramMask & P_MASK_DATALEN)
						con()->setReadBufferSizeIf(*m_req.datalen);
					p = con()->asyncWriteRead(size);
					m_req.parse(p, con()->datalen(), con()->rows());
				}else
					m_req.result = stat;
				if (m_logout)
					disconnect();
				m_preResult = m_req.result;
			}

		}
		return result();
	}
	inline ushort_td getBlobBuffer(const blobHeader** data)
	{
		*data = m_req.blobHeader;
		return *data ? 0:1;;
	}

	inline ushort_td addBlob(const blob* data, bool endRow)
	{
		if (endRow)
		{
			if (m_blobBuffer.fieldCount()==0)
				m_blobBuffer.setFieldCount((unsigned int)m_blobBuffer.blobs());
			return 0;
		}
		if (data==NULL)
			m_blobBuffer.clear();
		else
			m_blobBuffer.addBlob(*data);
		return 0;
	}
	bool readServerCharsetIndex();

	bool buildDualChasetKeybuf();
	
};

#define USETLS //USE TLS ALL



#ifdef USETLS
extern tls_key g_tlsiID;
#else
extern __THREAD client* __THREAD_BCB g_client;
#endif



inline client* getClientThread()
{
#ifdef USETLS
	client* p = (client*)tls_getspecific(g_tlsiID);
	if (p == NULL)
	{
		p = new client();
		tls_setspecific(g_tlsiID, p);
	}
	return p;
#else
	if (g_client == NULL)
		g_client = new client();
	return g_client;
#endif	
}

inline void setClientThread(client* v)
{
#ifdef USETLS
	if (v == NULL)
	{
		client* p = (client*)tls_getspecific(g_tlsiID);
		delete p;
	}
	tls_setspecific(g_tlsiID, v);
#else
	if (v == NULL)
		delete g_client;
	g_client = v;
#endif
}


}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_CLIENT_H
