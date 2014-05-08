/*=================================================================
   Copyright (C) 2012 2013 2014 BizStation Corp All rights reserved.

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

#include "appModule.h"
#include "appBuilderImple.h"
#include <bzs/db/engine/mysql/dbManager.h>
#include <bzs/db/engine/mysql/mysqlThd.h>
#include <bzs/db/protocol/tdap/mysql/tdapCommandExecuter.h>
#include <bzs/netsvc/server/iserver.h>
#include <stdlib.h>
#include <boost/asio/ip/address_v4.hpp>
#ifdef USE_HANDLERSOCKET
#include <bzs/db/protocol/hs/hsCommandExecuter.h>
#endif



using namespace bzs::netsvc::server;

IAppModule* IMyPluginModule::create(const boost::asio::ip::tcp::endpoint& endpoint
									, iconnection* connection		
									, bool tpool, int type)
{
	return new bzs::db::transactd::module(endpoint, connection, tpool, type);
}

namespace bzs
{
namespace netsvc
{
namespace server
{

	boost::mutex modulesMutex;
	std::vector<IAppModule*> modules;
	using namespace bzs::netsvc::server;

}//namespace server
}//namespace netsvc

namespace db
{
	using namespace protocol;
namespace transactd
{



#define ASYNCWRITE_DATA_SIZE 16374 //16384 - 10
static const unsigned int segment_mark = 0xFFFFFFFF;


class netAsyncWriter : public netWriter
{
	iconnection* m_conn;
	char* m_buf;
	char* m_data;
	size_t m_defBuffferPos;
	int m_databufLen;
	unsigned short m_rows;

	inline void doWrite()
	{
		size_t asyncDataSize = m_curPtr - m_data;
		size_t size = ASYNCWRITE_DATA_SIZE;
		if (asyncDataSize < ASYNCWRITE_DATA_SIZE)
			size = asyncDataSize;
		int offset = (int)(m_data - m_buf);
		
		*((unsigned short*)(m_data - 2)) =  (unsigned short) size;
		m_conn->asyncWrite(m_buf, size+offset);
		datalen += size;

		asyncDataSize -= size;
		memmove(m_buf + 2, m_data + size, asyncDataSize);
		m_data = m_buf + 2;
		m_curPtr = m_data + asyncDataSize;
	}

	inline void writeEndMark()
	{
		memcpy(m_data, &segment_mark , 4);
		unsigned short tmp = 4;
		memcpy(m_data + tmp, &m_databufLen , 4);
		memcpy(m_data + tmp + 4, &m_rows , 2);
		*((unsigned short*)(m_data - 2)) = tmp;
		m_conn->asyncWrite(m_buf, tmp+2+4+2);

		m_curPtr = m_data;
	}

public:
	netAsyncWriter(iconnection* conn)
		:netWriter(),m_conn(conn),m_defBuffferPos(0)
	{
		m_buf = new char[ASYNCWRITE_DATA_SIZE+65536+6];
		memcpy(m_buf, &segment_mark, sizeof(unsigned int));
		m_data = m_buf + 2 + 4;
	}

	~netAsyncWriter() {delete [] m_buf;}

	void reset(IResultBuffer* retBuf, buffers* optData)
	{
		netWriter::reset(retBuf, optData);
		m_defBuffferPos = 0;

		memcpy(m_buf, &segment_mark, sizeof(unsigned int));
		m_data = m_buf + 2 + 4;
		m_curPtr = m_data;
	
	}

	void beginExt(bool includeBlob)
	{
		short result = 0;
		ushort_td paramMask = getParamMask(includeBlob);

		asyncWrite((const char*)(&paramMask) , sizeof(ushort_td));
		asyncWrite((const char*)(&result) , sizeof(short));
		asyncWrite((const char*)(&m_rows) , sizeof(unsigned short));
		m_databufLen = sizeof(unsigned short);//rows space;
		m_rows = 0;
	}

	bool asyncWrite(const char* p, size_t size, eWriteMode mode=copyOnly)
	{
		size_t asyncDataSize = (m_curPtr - m_data);
		if (resultBuffer->size() < datalen + asyncDataSize + size)
			return false;
		m_databufLen += (int)size;
		if (mode==curSeekOnly)
			m_curPtr += size;
		else if (mode == netwrite)
		{
			if (asyncDataSize  > ASYNCWRITE_DATA_SIZE)
				doWrite();
		}
		else if (mode == writeEnd)
		{
			if (asyncDataSize)
				doWrite();
			writeEndMark();
		}
		else
		{
			memcpy(m_curPtr, p, size);
			m_curPtr += size;
		}		
		return true;	
	}

	//write to default buffer
	bool write(const char* p, size_t n, eWriteMode mode=copyOnly)
	{
		if (mode >= netwrite) return true;
		if (resultBuffer->size() < m_defBuffferPos + n) return false;

		if (mode != curSeekOnly)
			memcpy(resultBuffer->ptr() + m_defBuffferPos, p, n);
		m_defBuffferPos += n;
		datalen += n;
		m_databufLen += (int)n;
		return true;
		
	}

	void incremetRows()
	{
		++m_rows;
	}

	size_t bufferSpace() const
	{
		return ASYNCWRITE_DATA_SIZE+65536+6 - (curPtr() - m_buf);
	}

	unsigned int resultLen() const 
	{
		return (unsigned int)m_databufLen + RETBUF_EXT_ASYNC_RESERVE_SIZE;
	}

	unsigned short getParamMask(bool includeBlob)
	{
		ushort_td paramMask = P_MASK_DATA|P_MASK_FINALDATALEN|P_MASK_FINALRET;
		if (!engine::mysql::table::noKeybufResult)  
			paramMask |= P_MASK_KEYBUF;

		if (includeBlob) paramMask |=P_MASK_BLOBBODY;
		return paramMask;
	}
	
	/* Increment total deta size space only.
	   The header and contents are already sent */
	void writeHeadar(unsigned short paramMask, short result)
	{
		write(NULL, 4, curSeekOnly);
	}

	unsigned int allreadySent()const{return resultLen();}

};


class netStdWriter : public netWriter
{
	unsigned short* m_rowsPos;
public:
	netStdWriter():netWriter(){}

	void beginExt(bool includeBlob)
	{
		m_offset = RETBUF_EXT_RESERVE_SIZE;
		m_curPtr = m_ptr + m_offset;
		m_rowsPos = (unsigned short*)m_curPtr;
		(*m_rowsPos) = 0;
		datalen = sizeof(unsigned short);//rows space;
		m_curPtr += 2;
	}
	

	bool asyncWrite(const char* p, size_t n, eWriteMode mode=copyOnly)
	{
		if (mode >= netwrite) return true;
		if (resultBuffer->size() < datalen + m_offset + n) return false;

		if (mode != curSeekOnly)
			memcpy(m_curPtr, p, n);
		m_curPtr += n;
		datalen += n;
		
		return true;
	}
	
	bool write(const char* p, size_t n, eWriteMode mode=copyOnly)
	{
		return asyncWrite(p, n, mode);
	}

	void incremetRows()
	{
		++(*m_rowsPos);
	}

	size_t bufferSpace() const
	{
		return resultBuffer->size() - (m_curPtr - m_ptr);
	}

	unsigned short getParamMask(bool includeBlob)
	{
		ushort_td paramMask = (engine::mysql::table::noKeybufResult==false) ? 
						P_MASK_READ_EXT : P_MASK_DATA|P_MASK_DATALEN;
		if (includeBlob) paramMask |=P_MASK_BLOBBODY;
		return paramMask;
	}

	void writeHeadar(unsigned short paramMask, short result)
	{
		char* p = ptr() + sizeof(unsigned int);						//4
		memcpy(p, (const char*)(&paramMask), sizeof(ushort_td));	//2
		p += sizeof(ushort_td);
		memcpy(p, (const char*)(&result), sizeof(short_td));		//2
		p += sizeof(short_td);
		memcpy(p, (const char*)&datalen, sizeof(uint_td));		    //4
	}
	unsigned int allreadySent()const{return 0;}
};


/** The module created for every connection
 *  In the case of a thread pool, thread termination processing is not performed by a destructor.
 */
module::module(const boost::asio::ip::tcp::endpoint& endpoint
			, iconnection* connection, bool tpool, int type)
	:m_endpoint(endpoint), m_connection(connection), m_useThreadPool(tpool)
{
	if (type & PROTOCOL_TYPE_BTRV)
		m_commandExecuter.reset(new protocol::tdap::mysql::commandExecuter((unsigned __int64)this));
#ifdef USE_HANDLERSOCKET
	else if(type & PROTOCOL_TYPE_HS)
		m_commandExecuter.reset(new protocol::hs::commandExecuter((unsigned __int64)this));
#endif
	boost::mutex::scoped_lock lck(modulesMutex);
	modules.push_back(this);
	if (type & PROTOCOL_TYPE_ASYNCWRITE)
		m_nw = new netAsyncWriter(connection);
	else
		m_nw = new netStdWriter();
}

module::~module(void)
{
	boost::mutex::scoped_lock lck(modulesMutex);
	modules.erase( find(modules.begin(), modules.end(), this));
	delete m_nw;
	m_commandExecuter.reset();
	if (m_useThreadPool==false)
	{
		my_thread_end();
		endThread();
	}
}

void module::reset()
{
}

/**  It is called from the handler of async_read. 
 *   A value is analyzed and it is answered whether a lead is completion.
 *	 @return Size required for a lead buffer is returned on the occasion of reading by addition.
 *	         When zero are returned, it is shown that it is not necessary to enlarge a buffer further.
 */
size_t module::onRead(const char* data, size_t size, bool& complete)
{
	m_readBuf = data;
	m_readSize = size;
	return m_commandExecuter->perseRequestEnd(data, size, complete);
}

size_t module::onAccept(char* message, size_t bufsize)
{
	return m_commandExecuter->getAcceptMessage(message, bufsize);
}

static const char* addressMasks[3] = {".0.0.0/255.0.0.0", ".0.0/255.255.0.0", ".0/255.255.255.0" };

char* addressClass(char* buf,int bufsize, const char *host, int type)
{
	strcpy_s(buf, bufsize, host);
	for (int i = 3; i >= type; i--)
	{
		char* p = strrchr(buf, '.');
		if (p == NULL) return buf;
		*p = 0x00;
	}
	strcat_s(buf, bufsize, addressMasks[type-1]);
	return buf;
}

bool isAclUser(const char *host, const char *user)
{
	bool ret = is_acl_user(host, user);
	if (ret)return true;
	for (int i = 1; i <= 3; i++)
	{
		char buf[256];
		ret = is_acl_user( addressClass(buf, 256, host, i), user);
		if (ret)return true;
	}
	return false;
	
}

bool module::checkHost( const char* hostCheckname)
{
	std::string addr = m_endpoint.address().to_string();
	size_t pos = addr.find_last_of(":");
	if (pos != std::string::npos)
		addr = addr.substr(pos + 1);
	
	bool ret = true;
	if (!isAclUser(addr.c_str(), hostCheckname))
	{
		ret = isAclUser(m_endpoint.address().to_string().c_str(), hostCheckname);
		if (!ret && m_endpoint.address().is_v4())
		{
			if (addr ==  std::string("127.0.0.1"))
				ret = isAclUser("localhost", hostCheckname);
		}
	}
	return ret;
}

int module::execute(netsvc::server::IResultBuffer& result, size_t& size, netsvc::server::buffers* optionalData)
{
	m_commandExecuter->parse(m_readBuf, m_readSize);
	m_nw->reset(&result, optionalData);
	boost::mutex::scoped_lock lck(m_mutex);
	int	ret = m_commandExecuter->execute(m_nw);
	if (m_useThreadPool)
		cleanup();
	size = m_nw->datalen;
	return ret;
}

void  module::disconnect()
{
	m_connection->close();
}


}//namespace transactd
}//namespace db
}//namespace bzs
