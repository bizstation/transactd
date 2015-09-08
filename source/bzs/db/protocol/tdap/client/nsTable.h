#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_NSTABLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_NSTABLE_H
/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/env/tstring.h>
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/rtl/exception.h>
#include "export.h"

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

class nsdatabase;
class nstable;
class bulkInsert;

#define BULKBUFSIZE USHRT_MAX - 1000

class DLLLIB nstable
{
    friend class nsdatabase; // for destroy()
    friend class filter;

public:
    enum eUpdateType
    {
        changeCurrentCc,
        changeCurrentNcc,
        changeInKey
    };
    enum eFindType
    {
        findForword,
        findBackForword,
        findContinue
    };
    static const bool inkey = true;

private:
    struct nstimpl* m_impl;

    nstable(const nstable&);
    const nstable& operator=(const nstable&);
    void doUpdate(eUpdateType type);
    ushort_td doInsert(bool ncc);
    static _TCHAR* getErrorMessage(int errorCode, _TCHAR* buf, size_t size);

protected:
    void* m_pdata;
    uint_td m_buflen;
    uint_td m_datalen;
    void* m_keybuf;
    ushort_td m_op;
    keylen_td m_keybuflen;
    keylen_td m_keylen;
    mutable short_td m_stat;
    char_td m_keynum;

    union
    {
        struct
        {
            uchar_td m_read : 1;
            uchar_td m_write : 1;
            uchar_td m_insart : 1;
            uchar_td m_delete : 1;
            uchar_td m_reserverd : 4;
        };
        uchar_td m_curd;
    };

    virtual ~nstable();
    const _TCHAR* uri() const;
    const uchar_td* posblk() const;
    void setIsOpen(bool v);

    bulkInsert* bulkIns() const;
    virtual bool isUniqeKey(char_td keynum)
    {
        return false;
    } // for chacek updates deletes at key
    virtual void writeRecordData() = 0;
    virtual int onInsertBefore()
    {
        writeRecordData();
        return 0;
    };
    virtual void onInsertAfter(int beforeResult);
    virtual bool onUpdateCheck(eUpdateType type) { return true; };
    virtual int onUpdateBefore()
    {
        writeRecordData();
        return 0;
    };
    virtual void onUpdateAfter(int beforeResult){};
    virtual bool onDeleteCheck(bool in_key) { return true; };
    virtual void doDel(bool inkey);
    virtual keylen_td writeKeyData() { return 0; };
    virtual void onReadAfter() = 0;
    virtual uint_td doGetWriteImageLen() { return m_buflen; };
    virtual void doOpen(const _TCHAR* name, char_td mode,
                        const _TCHAR* ownername);
    virtual void doClose();
    virtual void doCreateIndex(bool specifyKeyNum);
    virtual uint_td doRecordCount(bool estimate, bool fromCurrent);
    virtual short_td doBtrvErr(HWND hWnd, _TCHAR* retbuf);
    virtual ushort_td doCommitBulkInsert(bool autoCommit);
    virtual void doAbortBulkInsert();
    inline void open(const _TCHAR* uri, char_td mode = 0,
                     const _TCHAR* ownerName = NULL)
    {
        doOpen(uri, mode, ownerName);
    }

    /*
            This method is ignore refarence count of nstable and force delete.
            Use in nsdatabase::reset()
    */
    void destroy();
    void setShared();
    void seekByBookmark(bookmark_td* bm, ushort_td lockBias = LOCK_BIAS_DEFAULT);
public:
    explicit nstable(nsdatabase* pbe);
    void addref(void);
    void release();
    int refCount() const;
    nsdatabase* nsdb() const;
    short tableid() const;
    void setTableid(short v);
    bool isOpen() const;
    bool isUseTransactd() const;
    inline void setAccessRights(unsigned char curd) { m_curd = curd; };
    inline const void* data() const { return m_pdata; }
    inline void setData(void* v) { m_pdata = v; }
    inline uint_td buflen() const { return m_buflen; }
    inline void setBuflen(uint_td v) { m_buflen = v; }
    inline uint_td datalen() const { return m_datalen; }
    inline short_td stat() const { return m_stat; }
    inline void setStat(short_td v) { m_stat = v; }
    inline char_td keyNum() const { return m_keynum; }
    inline void setKeyNum(char_td v) { m_keynum = v; }
    inline bool canRead() const { return (bool)m_read; }
    inline bool canWrite() const { return (bool)m_write; }
    inline bool canInsert() const { return (bool)m_insart; }
    inline bool canDelete() const { return (bool)m_delete; }
    inline uint_td getWriteImageLen() { return doGetWriteImageLen(); };
    inline void close() { doClose(); }
    inline void update(eUpdateType type = changeCurrentCc) { doUpdate(type); }
    inline void del(bool in_key = false) { doDel(in_key); }
    inline ushort_td insert(bool ncc = false) { return doInsert(ncc); };
    inline void createIndex(bool specifyKeyNum = false)
    {
        doCreateIndex(specifyKeyNum);
    }
    void dropIndex(bool norenumber = false);
    inline uint_td recordCount(bool estimate = true, bool fromCurrent = false)
    {
        return doRecordCount(estimate, fromCurrent);
    }
    inline short_td tdapErr(HWND hWnd, _TCHAR* retbuf = NULL)
    {
        return doBtrvErr(hWnd, retbuf);
    }
    inline _TCHAR* statMsg(_TCHAR* retbuf)
    {
        doBtrvErr(0, retbuf);
        return retbuf;
    }

    void beginBulkInsert(int maxBuflen);
    void abortBulkInsert() { doAbortBulkInsert(); }
    inline ushort_td commitBulkInsert(bool autoCommit = false)
    {
        return doCommitBulkInsert(autoCommit);
    }
    void tdap(ushort_td op);
    void seekFirst(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekLast(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekPrev(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekNext(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seek(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekGreater(bool orEqual, ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekLessThan(bool orEqual, ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void stepFirst(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void stepLast(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void stepPrev(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void stepNext(ushort_td lockBias = LOCK_BIAS_DEFAULT);
    ushort_td bookmarkLen() const ;
    bookmark_td bookmark();
    void seekByBookmark(bookmark_td& bm, ushort_td lockBias = LOCK_BIAS_DEFAULT);
    void seekByBookmark();
    percentage_td getPercentage();
    percentage_td getPercentage(bookmark_td& bm);
    void seekByPercentage();
    void seekByPercentage(percentage_td pc);
    void setOwnerName(const _TCHAR* name, char_td enctype = 0);
    void clearOwnerName();
    ushort_td recordLength();
    void stats(void* databuffer, uint_td buflen, bool estimate = true);
    void unlock(bookmark_td& bm);
    void unlock();
    char_td mode() const;
    static _TCHAR* getFileName(const _TCHAR* uri, _TCHAR* retbuf);
    static short_td tdapErr(HWND hWnd, short_td status,
                            const _TCHAR* tableName = NULL,
                            _TCHAR* retbuf = NULL);
    static void throwError(const _TCHAR* caption, short statusCode);
    static void throwError(const _TCHAR* caption, nstable* tb);
    static _TCHAR* getDirURI(const _TCHAR* uri, _TCHAR* retbuf);
    static bool existsFile(const _TCHAR* filename);
    /** @cond INTERNAL */
    static bool test(nstable* p);
    /** @endcond*/
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_NSTABLE_H
