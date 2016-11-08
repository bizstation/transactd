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
#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H

#include "request.h"
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/db/protocol/tdap/fieldComp.h>
#include <bzs/rtl/exception.h>
#include <bzs/db/engine/mysql/IReadRecords.h>
#include <bzs/db/engine/mysql/fieldAccess.h>
#include <boost/shared_ptr.hpp>


#ifndef MAX_KEY_SEGMENT
#define MAX_KEY_SEGMENT 8
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

#pragma pack(push, 1)
pragma_pack1;

struct logicalField;
struct resultField;

/** calculate record position
 */
class position
{
    engine::mysql::table* m_tb;
    const char* m_record;

public:
    inline position();
    inline void setTable(engine::mysql::table* tb);
    inline char* fieldPtr(const resultField* rf) const;
    inline bool isBlobField(const resultField* rf) const;
    inline void addBlobBuffer(int fieldNum) { m_tb->addBlobBuffer(fieldNum); };
    inline unsigned short packLen(const resultField* rf) const;
    inline const char* record() const { return m_record; }
    inline ulong recordLenCl() const { return m_tb->recordLenCl(); }
    inline int recordFormatType() const { return m_tb->recordFormatType(); };
    inline uint recordPackCopy(char* buf, uint maxlen) const
    {
        return m_tb->recordPackCopy(buf, maxlen);
    }
    int getFieldNumByPos(unsigned short pos) const
    {
        char* start = (char*)m_tb->field(0)->ptr;
        for (int i = 0; i < m_tb->fields(); i++)
        {
            if ((char*)m_tb->field(i)->ptr - start == pos)
                return i;
        }
        return -1;
    }
    inline uint fieldSizeByte(int fieldNum)
    {
        return m_tb->fieldSizeByte(fieldNum);
    }
    inline ushort fieldPackCopy(unsigned char* nullPtr, int& nullbit, unsigned char* dest, short fieldNum)
    {
        return m_tb->fieldPackCopy(nullPtr, nullbit, dest, fieldNum);
    }
    inline bool isMysqlNull() const { return m_tb->isMysqlNull();}
    inline bool isNullable(int index) const { return m_tb->field(index)->null_bit != 0; }
    inline unsigned char nullbit(int index) const { return m_tb->field(index)->null_bit; } 
    inline unsigned char nullOffset(int index) const 
    { 
        Field* fd = m_tb->field(index);
        unsigned char* null_ptr = (unsigned char*)cp_null_ptr(fd, (unsigned char*)m_tb->internalTable()->record[0]);
        return (unsigned char)((unsigned char*)m_tb->field(0)->ptr -  null_ptr); 
    }

    bool isLegacyTimeFormat(int fieldNum) const
    {
        return m_tb->isLegacyTimeFormat(fieldNum);
    }
};

/** If get all field then len = record length.
 *
 */
struct resultField
{
    unsigned short len;
    union
    {
        unsigned short pos;
        unsigned short fieldNum;
    };
};

struct extResultDef
{
    unsigned short maxRows;
    unsigned short fieldCount;
    resultField field[1]; // variable

    int memSize() const
    {
        return 4 + (sizeof(resultField) * fieldCount);
    }

    bool isFieldSelect(ulong clientRecordLen) const 
    {
       return  ((fieldCount > 1) ||
            ((fieldCount == 1) && (field[0].len < clientRecordLen)));
    }

    bool isAllField(ulong clientRecordLen) const
    {
        return  ((fieldCount == 1) && (field[0].len >= clientRecordLen));
    }
};

inline position::position() : m_tb(NULL), m_record(NULL){};
inline void position::setTable(engine::mysql::table* tb)
{
    m_tb = tb;
    m_record = (const char*)m_tb->record();
}

inline char* position::fieldPtr(const resultField* rf) const
{
    return (char*)m_tb->field(rf->fieldNum)->ptr;
}

inline bool position::isBlobField(const resultField* rf) const
{
    return db::engine::mysql::isBlobType(m_tb->field(rf->fieldNum)->type());
}

/** return data length as real rength.
 */
inline unsigned short position::packLen(const resultField* rf) const
{
    return m_tb->fieldDataLen(rf->fieldNum);
}


#define REC_MACTH 0
#define REC_NOMACTH 1
#define REC_NOMACTH_NOMORE 2

struct seek
{
    struct
    {
        unsigned short len  : 15;
        unsigned short null : 1;
    };
    unsigned char ptr[1]; // variable
    seek* next() const
    {
        return (seek*)((char*)this + len + sizeof(unsigned short));
    }

    extResultDef* resultDef(int count) const
    {
        const seek* s = this;
        while (count--)
            s = s->next();
        return (extResultDef*)s;
    }
};

struct logicalField
{

public:
    unsigned char type;
    unsigned short len;
    unsigned short pos;
    char logType;
    char opr;
    union
    {
        unsigned short offset;
        unsigned char ptr[2]; // variable
    };

    logicalField* next() const
    {
        return (logType & 64) ? (logicalField*)(ptr + 2)
                              : (logicalField*)(ptr + len);
    }

public:

    comp1Func getCompFunc(int sizeByte) const
    {
        return ::getCompFunc(type, len, logType, sizeByte);
    }

    extResultDef* resultDef() const
    {
        if ((opr == 0) || (opr == FILTER_COMBINE_PREPARE))
            return (extResultDef*)next();
        return next()->resultDef();
    }
};

struct extRequest
{
    unsigned int ilen : 28;
    unsigned int itype : 4;
    union
    {
        unsigned short rejectCount;
        unsigned short preparedId;
    };
    unsigned short logicalCount;
    logicalField field;

    extResultDef* resultDef() const
    {
        if (logicalCount)
            return field.resultDef();
        return (extResultDef*)&field;
    }
};

struct extRequestSeeks
{
    unsigned int ilen : 28;
    unsigned int itype : 4;
    unsigned short rejectCount;
    unsigned short logicalCount;
    seek seekData;

    extResultDef* resultDef() const
    {
        if (logicalCount)
            return seekData.resultDef(logicalCount);
        return (extResultDef*)&seekData;
    }
};

#pragma pack(pop)
pragma_pop;

class fields;
class fieldAdapter
{
    const logicalField* m_fd;
    fieldAdapter* m_next;
    judgeFunc m_isMatchFunc;
    comp1Func m_compFunc;
    unsigned short m_placeHolderNum;
    unsigned char m_keySeg;
    char m_judgeType;
    char m_sizeBytes;
    char opr;
    unsigned char m_nullOffset;
    unsigned char m_nullbit;
    unsigned char m_nullOffsetCompFd;
    unsigned char m_nullbitCompFd;
    struct
    {
    mutable bool m_judge : 1;
    mutable bool m_matched : 1;
    mutable bool m_mysqlnull : 1;
    mutable bool m_nulllog : 1;
    };

public:
    friend class fields;

    void reset()
    {
        m_keySeg = 0xff;
        m_judgeType = 0;
        m_sizeBytes = 0;
        m_judge = false;
        m_matched = false;
        m_placeHolderNum = 0xffff;
        m_nullOffset = 0;
        m_nullbit = 0;
        m_nullOffsetCompFd = 0;
        m_nullbitCompFd = 0;
    }

    int init(const logicalField* fd, position& position, const KEY* key, bool forword)
    {
        reset();
        m_fd = fd;
        m_mysqlnull = position.isMysqlNull();
        int num = position.getFieldNumByPos(fd->pos);
        if (num == -1)
            return STATUS_INVALID_FIELD_OFFSET;
        
        if (position.isNullable(num))
        {
            m_nullOffset = position.nullOffset(num);
            m_nullbit = position.nullbit(num);
            if (fd->logType & CMPLOGICAL_FIELD)
            {
                int fnum = position.getFieldNumByPos(fd->offset);
                m_nullOffsetCompFd = position.nullOffset(fnum);
                m_nullbitCompFd = position.nullbit(fnum);
            }
        }
        
        // Chnage compare type
        if (fd->type == ft_mytime || fd->type == ft_mydatetime || fd->type == ft_mytimestamp)
        {
            bool regacy = position.isLegacyTimeFormat(num);
            if (regacy)
            {
                if (fd->type == ft_mytime) const_cast<logicalField*>(fd)->type = ft_mytime_num_cmp; 
                if (fd->type == ft_mydatetime) const_cast<logicalField*>(fd)->type = ft_mydatetime_num_cmp; 
                if (fd->type == ft_mytimestamp) const_cast<logicalField*>(fd)->type = ft_mytimestamp_num_cmp; 
            }
        }
        // Cacheing compare isNull ?
        eCompType log = (eCompType)(m_fd->logType & 0xf);
        m_nulllog = ((log == eIsNull) || (log == eIsNotNull));

        m_sizeBytes = (char)position.fieldSizeByte(num);
        m_placeHolderNum = fd->opr & FILTER_COMBINE_PREPARE;// temporary marking
        if (m_placeHolderNum)
            const_cast<logicalField*>(fd)->opr &= ~FILTER_COMBINE_PREPARE;
        m_compFunc = fd->getCompFunc(m_sizeBytes);
        if (fd->opr == 2)
        {
            m_judgeType = 0;
            return 0;
        }
        if (key)
        {
            int segmentIndex = 0;
            int segments =
                std::min<uint>(MAX_KEY_SEGMENT, key->user_defined_key_parts);
            while (segmentIndex < segments)
            {
                if (key->key_part[segmentIndex].field->field_index == num)
                {
                    eCompType comp = (eCompType)(fd->logType & 0x0f);
                    bool gt = (comp == eGreater) || (comp == eGreaterEq);
                    bool le = (comp == eLess) || (comp == eLessEq);
                    bool valid = !(forword ? gt : le);
                    
                    // case in-sencitive, Index judge need clinet and server are same option.
                    bool is_cl_casein = ((fd->logType & CMPLOGICAL_CASEINSENSITIVE) != 0);
                    bool is_srv_casein = ((key->key_part[segmentIndex].field->flags & BINARY_FLAG) == 0) && 
                                            isStringType(fd->type);
                    if (valid && (is_cl_casein == is_srv_casein))
                    {
                        m_keySeg = (unsigned char)segmentIndex + 1;
                        m_judgeType = (comp == eEqual) ? 2 : 1;
                    }
                    break;
                }
                ++segmentIndex;
            }
        }
        return 0;
    }

    inline void supplyValue(const logicalField* p)
    {
        const_cast<logicalField*>(p)->opr = opr;
        m_fd = p;
        m_compFunc = m_fd->getCompFunc(m_sizeBytes);
    }

    inline int checkNomore(bool typeNext, eCompType log) const
    {
        if (m_judge)
        {
            if ((log == eEqual) && m_matched) //==
                return REC_NOMACTH_NOMORE;
            else if (typeNext && (log == eLess || log == eLessEq))
                return REC_NOMACTH_NOMORE;
            else if (!typeNext && (log == eGreater || log == eGreaterEq))
                return REC_NOMACTH_NOMORE;
        }
        return REC_NOMACTH;
    }

    /* nullComp
       @result 1 true
              -1 false
               0 no judge
       
    */
    int nullComp(const char* record, eCompType log) const
    {
        if (m_nullbitCompFd)
        {
            // (? = NULL) return false, ? <> NULL return false, ? > NULL return false
            if ((*(record - m_nullOffsetCompFd) & m_nullbitCompFd) != 0)
                return -1;
        }
        bool rnull = m_nulllog;
        bool lnull = (*(record - m_nullOffset) & m_nullbit) != 0;
        return ::nullComp(lnull, rnull, log);    
    }

    int match(const char* record, bool typeNext) const
    {
        bool ret;
        int v = 2;
       
        if (m_mysqlnull && (m_nullbit || m_nullbitCompFd || m_nulllog))
            v = nullComp(record, (eCompType)(m_fd->logType & 0xf));
        if (v < 2)
            ret = (v == 0) ? true : false;
        else
        {
            const char* r = (const char*)m_fd->ptr;
            const char* l = record + m_fd->pos;
            if (m_fd->logType & CMPLOGICAL_FIELD)
                r = record + m_fd->offset;
            v = (m_compFunc)(l, r, m_fd->len);
            ret = m_isMatchFunc(v);
        }
            
        if (ret && m_judgeType)
        {
            m_matched = true;
            // check  is this logic range of max ?
            // if max then set judge node to next logic
            if ((m_fd->opr != 0) && m_judge && (v == 0) && m_next->m_judgeType)
                m_next->m_judge = true;
        }
        bool end = isEndComp(m_fd->opr, ret);
        if (!end)
            return m_next->match(record, typeNext);
        return ret ? REC_MACTH
                   : checkNomore(typeNext, (eCompType)(m_fd->logType & 0xF));
    }

    bool operator<(const fieldAdapter& r) const
    {
        if (m_judgeType != r.m_judgeType)
            return m_judgeType > r.m_judgeType;
        else if (m_keySeg == r.m_keySeg)
            return this < &r; // no change order
        return m_keySeg < r.m_keySeg;
    }
    void oprCache() { opr = m_fd->opr; }
};

class fields
{
    std::vector<fieldAdapter> m_fields;

public:
    unsigned char nullbytes;    

    void init(const extRequest& req, position& position, const KEY* key, bool forword)
    {
        nullbytes = 0;
        if (req.logicalCount == 0)
            return ;

        const logicalField* fd = &req.field;
        if (m_fields.size() != req.logicalCount)
            m_fields.resize(req.logicalCount);
        int lastIndex = req.logicalCount;

        for (int i = 0; i < req.logicalCount; ++i)
        {
            fieldAdapter& fda = m_fields[i];
            fda.init(fd, position, key, forword);
            fda.m_placeHolderNum = i;
            fda.m_isMatchFunc = getJudgeFunc((eCompType)fd->logType);
            fd = fd->next();
            if (fda.m_fd->opr == 2 && (lastIndex == req.logicalCount))
                lastIndex = i; // the first 'or' index
        }
        if (key)
        {
            // Sort between first to before first "or";
            std::vector<fieldAdapter>::iterator begin = m_fields.begin();
            std::vector<fieldAdapter>::iterator cur = m_fields.begin();
            std::vector<fieldAdapter>::iterator end = begin + lastIndex;
            char tmpOpr = (lastIndex != req.logicalCount) ? end->m_fd->opr : 0;
            std::sort(begin, end);
            bool flag = true;
            while (cur != end)
            {
                const_cast<logicalField*>(cur->m_fd)->opr = 1; // and
                if (flag && cur->m_judgeType == 2)
                    cur->m_judge = true;
                else
                    flag = false;
                ++cur;
            }

            // if first logic is first segmnet then first logic can judge.
            if ((begin->m_keySeg == 1) && begin->m_judgeType)
                begin->m_judge = true;

            if (lastIndex == req.logicalCount)
                --end;
            const_cast<logicalField*>(end->m_fd)->opr = tmpOpr;
        }
        for (int i = 0; i < req.logicalCount - 1; ++i)
            m_fields[i].m_next = &m_fields[i + 1];
    }

    int match(const char* record, bool typeNext) const
    {
        return m_fields[0].match(record, typeNext);
    }

    bool setSupplyValues(const extRequest& req)
    {
        const logicalField* fd = &req.field;
        for (int i = 0; i < req.logicalCount; ++i)
        {
            for (int j=0;j<(int)m_fields.size();++j)
            {
                fieldAdapter& fa = m_fields[j];
                if (fa.m_placeHolderNum == i)
                {
                    fa.supplyValue(fd);
                    break;
                }
            }
            fd = fd->next();
        }
        return true;
    }

    void setNextPtr()
    {
        if (m_fields.size())
        {
            for (int i = 0; i < (int)m_fields.size() - 1; ++i)
            {
                m_fields[i].m_next = &m_fields[i + 1];
                m_fields[i].oprCache();
            }
            m_fields[m_fields.size() - 1].oprCache();
        }
    }
};


class prepared : public engine::mysql::IPrepare
{
public:
    fields* fds;
    extResultDef* rd;
    std::bitset<256> bits;
    int  blobs;
    unsigned short rejectCount;
    bool wholeRow;
    prepared() : fds(NULL), rd(NULL), wholeRow(false){}

    ~prepared()
    {
        if (fds)
            delete fds;
        if (rd)
            free(rd);
    }

    void assignResultDef(const extResultDef* src)
    {
        assert(rd == NULL);
        rd = (extResultDef*)malloc(src->memSize());
        memcpy(rd, src, src->memSize());
    }

    void assignFields(const fields* src)
    {
        assert(fds == NULL);
        fds = new fields();
        *(fds) = *src;
        fds->setNextPtr();
    }
    
    void release()
    {
        delete this;
    }
};

class resultWriter
{
    netsvc::server::netWriter* m_nw;
    const extResultDef* m_def;
    bool m_noBookmark;
    unsigned char m_nullbytes;

    short doWrite(position* pos, const unsigned char* bookmark, int bmlen)
    {
        // write recLength space;
        unsigned short recLen = 0;
        unsigned short* recLenPos = (unsigned short*)m_nw->curPtr();
        m_nw->asyncWrite((const char*)&recLen, 2);

        // write bookmark
        if (!m_noBookmark)
            m_nw->asyncWrite((const char*)bookmark, bmlen);

        // if pos ==NULL , that is not found record in a TD_KEY_SEEK_MULTI
        // operation
        // and bookmark has error code also STATUS_NOT_FOUND_TI
        // in the client, fieldCount > 0 buf recLen=0 then this pattern
        if (pos)
        {
            if (m_def->isAllField(pos->recordLenCl()))
            { // write whole row
                int len = pos->recordPackCopy(m_nw->curPtr(),
                                              (uint)m_nw->bufferSpace());
                if (len == 0)
                    return STATUS_BUFFERTOOSMALL;
                if (!m_nw->asyncWrite(NULL, len,
                                      netsvc::server::netWriter::curSeekOnly))
                    return STATUS_BUFFERTOOSMALL;
                recLen += len;
            }
            else
            {
                //null support
                unsigned char* nullPtr = NULL;
                int nullbit = 0;
                if (m_nullbytes)
                {
                    nullPtr = (unsigned char*)m_nw->curPtr();
                    memset(nullPtr, 0, m_nullbytes);
                    m_nw->asyncWrite(NULL, m_nullbytes, netsvc::server::netWriter::curSeekOnly);
                    recLen += m_nullbytes;
                }            
                // write each fields by field num.
                for (int i = 0; i < m_def->fieldCount; i++)
                {
                    const resultField& fd = m_def->field[i];
                    if (m_nw->bufferSpace() > fd.len)
                    {
                        uint len = pos->fieldPackCopy(nullPtr, nullbit,
                            (unsigned char*)m_nw->curPtr(), fd.fieldNum);
                        m_nw->asyncWrite(
                            NULL, len, netsvc::server::netWriter::curSeekOnly);
                        recLen += len;
                        if (pos->isBlobField(&fd))
                            pos->addBlobBuffer(fd.fieldNum);
                    }
                    else
                        return STATUS_BUFFERTOOSMALL;
                }
            }
        }

        // write recLength;
        *recLenPos = recLen;
        m_nw->asyncWrite(NULL, 0, netsvc::server::netWriter::netwrite);
        // rowCount
        m_nw->incremetRows();
        return 0;
    }

public:
    resultWriter() : m_nw(NULL), m_def(NULL), m_noBookmark(false), m_nullbytes(0){}

    void init(netsvc::server::netWriter* nw, const extResultDef* def,
                 bool noBookmark, unsigned char nullbytes)
    {
         m_nw = nw;
         m_def = def;
         m_noBookmark = noBookmark;
         m_nullbytes = nullbytes;
    }

    short write(position* pos, const unsigned char* bookmark, int len)
    {
        return doWrite(pos, bookmark, len);
    }

    inline unsigned int end()
    {
        if (m_nw)
        {
            m_nw->asyncWrite(NULL, 0, netsvc::server::netWriter::writeEnd);
            return m_nw->resultLen();
        }
        return 0;
    }

    const char* resultBuffer() { return m_nw->ptr(); }

};

class ReadRecordsHandler : public engine::mysql::IReadRecordsHandler
{
    resultWriter m_writer;
    const extRequest* m_req;
    position m_position;
    fields* m_fields;
    fields* m_defaultFields;
    unsigned short m_maxRows;
    bool m_seeksMode;

public:
    ReadRecordsHandler():m_defaultFields(new fields()){}

    ~ReadRecordsHandler()
    {
        delete m_defaultFields;
    }

    short beginPreparExecute(engine::mysql::table* tb, const extRequest* req, bool fieldCache,
                netsvc::server::netWriter* nw, bool noBookmark, prepared* p)
    {
        m_seeksMode = !fieldCache;
        m_position.setTable(tb);
        m_req = req;
        m_fields = p->fds;
        const_cast<extRequest*>(m_req)->rejectCount = p->rejectCount;
        if (!m_seeksMode)
        {
            if (!m_fields->setSupplyValues(*req))
                return STATUS_INVALID_SUPPLYVALUES;
        }
        engine::mysql::prepareHandler* ph = tb->getPrepareHandler();
        ph->setReadBitmap(p->bits);
        tb->blobBuffer()->clear();
        tb->setBlobFieldCount(p->blobs);
        nw->beginExt(tb->blobFields() != 0);
        const extResultDef* rd = p->rd;
        m_writer.init(nw, rd, noBookmark, m_fields->nullbytes);
        m_maxRows = p->rd->maxRows;
        ph->ready(p->wholeRow);

        return 0;
    }

    short prepare(engine::mysql::table* tb, const extRequest* req, bool fieldCache,
                netsvc::server::netWriter* nw, bool forword, bool noBookmark, prepared* p)
    {
        // Important! cache resultdef first.
        const extResultDef* srcRd = req->resultDef();
        
        short ret = begin(tb, req, fieldCache, nw, forword, noBookmark);
        p->assignResultDef(srcRd);
        p->assignFields(m_fields);
        engine::mysql::prepareHandler* ph = tb->getPrepareHandler();
        p->bits = ph->getReadBitmap();
        p->blobs = tb->getBlobFieldCount();
        p->rejectCount = m_req->rejectCount;
        p->wholeRow = ph->isWholeRow();
        end();
        return ret;
    }
    
    short begin(engine::mysql::table* tb, const extRequest* req, bool fieldCache,
                netsvc::server::netWriter* nw, bool forword, bool noBookmark)
    {
        short ret = 0;
        m_seeksMode = !fieldCache;
        m_position.setTable(tb);
        m_req = req;
        m_fields = m_defaultFields;
        const extResultDef* rd = m_seeksMode ? ((extRequestSeeks*)m_req)->resultDef()
                                : m_req->resultDef();
        if (fieldCache)
        {
            const KEY* key = NULL;
            if (tb->keyNum() >= 0)
                key = &tb->keyDef(tb->keyNum());
            m_fields->init(*m_req, m_position, key, forword);
        }
        bool whole_row = true;
        if (rd->isFieldSelect(m_position.recordLenCl()))
        {
            ret = convResultPosToFieldNum(tb, noBookmark, rd, m_seeksMode, 
                            (req->itype & FILTER_TYPE_SEEKS_BOOKMARKS) != 0);
            if (ret) return ret;
            whole_row = false;
        }
            
        nw->beginExt(tb->blobFields() != 0);
        m_writer.init(nw, rd, noBookmark, m_fields->nullbytes);
        m_maxRows = rd->maxRows; 
        
        engine::mysql::prepareHandler* ph = tb->getPrepareHandler();
        ph->ready(whole_row);
        // DEBUG_RECORDS_BEGIN(m_resultDef, m_req)
        return ret;
    }
    
    // TODO This convert is move to client. but legacy app is need this
    short convResultPosToFieldNum(engine::mysql::table* tb, bool noBookmark,
                                  const extResultDef* rd, bool seeksMode, bool seekBookmark)
    {
        int blobs = 0;
        int nullfields = 0;
        m_fields->nullbytes = 0;
        engine::mysql::prepareHandler* ph = tb->getPrepareHandler();
        ph->clearReadBitmap();
        for (int i = 0; i < rd->fieldCount; i++)
        {
            const resultField& fd = rd->field[i];
            int num = m_position.getFieldNumByPos(fd.pos);
            if (num == -1)
                return STATUS_INVALID_FIELD_OFFSET;
            const_cast<resultField&>(fd).fieldNum = num;
            ph->setReadBitmap(num);
            if (m_position.isBlobField(&fd))
                ++blobs;
            if (m_position.isNullable(num) && m_position.isMysqlNull())
                ++nullfields;
                
        }
        m_fields->nullbytes = (nullfields + 7)/8;

        if (!seeksMode)
        {
            const logicalField* fd = &m_req->field;
            for (int i = 0; i < m_req->logicalCount; ++i)
            {
                ph->setReadBitmap(m_position.getFieldNumByPos(fd->pos));
                fd = fd->next();
            }
        }
        else if (!seekBookmark)
            ph->makeCurrentKeyFieldBitmap();

        // if need bookmark , add primary key fields
        if (!noBookmark)
            ph->makePrimaryKeyFieldBitmap();

        tb->blobBuffer()->clear();
        tb->setBlobFieldCount(blobs);
        return 0;
    }

    unsigned int end()
    {
        // DEBUG_RECORDS_END(m_writer.get())
        return m_writer.end();
    }

    int match(bool typeNext) const
    {
        if (m_req->logicalCount)
            return m_fields->match(m_position.record(), typeNext);
        return REC_MACTH;
    }

    short write(const unsigned char* bookmarkPtr,
                unsigned int bmlen /*, short stat=0*/)
    {
        unsigned int bookmark = 0;
        // if bmPtr ==NULL , that is not found record in a TD_KEY_SEEK_MULTI
        // operation
        // and set error code to bookmark also STATUS_NOT_FOUND_TI
        if (bookmarkPtr == NULL)
        {
            // bookmark = stat;
            return m_writer.write(NULL, (const unsigned char*)&bookmark, sizeof(bookmark));
        }
        else
            return m_writer.write(&m_position, bookmarkPtr, bmlen);
    }
    unsigned short rejectCount() const { return m_req->rejectCount; };
    unsigned short maxRows() const { return m_maxRows; };
};

} // namespace mysql
} // namespace protocol
} // namespace db
} // namespace tdap
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H
