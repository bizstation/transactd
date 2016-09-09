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
#include "field.h"
#include "fields.h"
#include "nsDatabase.h"
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/protocol/tdap/myDateTime.h>
#include <bzs/db/protocol/tdap/fieldComp.h>
#include "stringConverter.h"
#include <bzs/rtl/stringBuffers.h>
#ifdef BCB_32
#pragma option push
#pragma option -O1
#include <boost/unordered_map.hpp>
#pragma option pop
#else
#include <boost/unordered_map.hpp>
#endif
#include <boost/shared_array.hpp>

#pragma package(smart_init)

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
typedef int int32;
class myDecimal
{
    int32 m_data[8];
    int32 m_sign;
    uchar_td m_digit;
    uchar_td m_dec;
    uchar_td m_intBytes;
    uchar_td m_decBytes;
    uchar_td m_intSurplusBytes;
    uchar_td m_decSurplusBytes;
    uchar_td m_error;

    int32 changeEndian(int32 v, int len)
    {
        int32 ret = 0;
        char* l = (char*)&ret;
        char* r = (char*)&v;
        if (len == 4) {
            l[0] = r[3]; l[1] = r[2]; l[2] = r[1]; l[3] = r[0];
        }else if (len == 3) {
            l[0] = r[2]; l[1] = r[1]; l[2] = r[0];
        }else if (len == 2) {
            l[0] = r[1]; l[1] = r[0];
        }else if (len == 1)
            ret = v;
        return ret;
    }
    
    int32 readInt32One(const char* ptr, int len)
    {
        int32 v = *((int32*)ptr);
        return  changeEndian(v ^ m_sign, len) ;
    }

    int32 readSurplus(const char* ptr, int len)
    {
        assert(len <= 4);
        int32 v = 0;
        switch(len)
        {
        case 1: v = *((char*)ptr); break;
        case 2: v = *((short*)ptr); break;
        case 3: v = (*((int32*)ptr) & 0x00FFFFFF); break;
        case 4: v = *((int32*)ptr); break;
        }
        return readInt32One((const char*)&v, len);
    }
    
    char* write(char* ptr, int len, int32 v)
    {
        v = changeEndian(v, len) ^ m_sign;
        memcpy(ptr, &v , len);
        return ptr + len; 
    }
    
    int32 fromString(const char* p, size_t len)
    {
        assert(len <= 9);
        char tmp[10]={0};
        strncpy(tmp, p, len);
        return atoi(tmp);
    }

public:
    myDecimal(uchar_td digit, uchar_td dec) : m_sign(0), m_digit(digit), m_dec(dec), m_error(0) 
    {
        assert(sizeof(int32) == 4);
        assert(digit > 0);
        assert(m_digit <= 65);
        assert(m_dec <= 30);

        // integer part
        int dig = digit - dec;
        m_intSurplusBytes = (uchar_td)decimalBytesBySurplus[dig % DIGITS_INT32];
        m_intBytes = (uchar_td)((dig / DIGITS_INT32) * sizeof(int32));

        // dec part
        m_decSurplusBytes = (uchar_td)(decimalBytesBySurplus[dec % DIGITS_INT32]);
        m_decBytes = (uchar_td)((dec / DIGITS_INT32) * sizeof(int32));
    }
    
    void store(char* ptr)
    {
        int32* data = m_data;
        char* p = ptr;
        if (m_intSurplusBytes)
            p = write(p, m_intSurplusBytes, *(data++));
        for (int i = 0; i < (int)(m_intBytes/sizeof(int32)) ; ++i)
            p = write(p, sizeof(int32), *(data++));
        for (int i = 0; i < (int)(m_decBytes/sizeof(int32)) ; ++i)
            p = write(p, sizeof(int32), *(data++));
        if (m_decSurplusBytes)
            write(p, m_decSurplusBytes, *data);
        //reverse first bit
        *(ptr) ^= 0x80;
    }
    
    void read(const char* ptr)
    {
        m_sign = ((*ptr) & 0x80) ? 0 : -1;
        int32* data = m_data;
        const char* p = ptr;
        //reverse first bit
        *(const_cast<char*>(ptr)) ^= 0x80;
        if (m_intSurplusBytes)
        {
            *(data++) = readSurplus(p, m_intSurplusBytes);
            p += m_intSurplusBytes;
        }
        for (int i = 0; i < (int)(m_intBytes/sizeof(int32)) ; ++i)
        {
            *(data++) = readInt32One(p, sizeof(int32));
            p += sizeof(int32);
        }
        for (int i = 0; i < (int)(m_decBytes/sizeof(int32)) ; ++i)
        {
            *(data++) = readInt32One(p, sizeof(int32));
            p += sizeof(int32);
        }
        if (m_decSurplusBytes)
            *data = readSurplus(p, m_decSurplusBytes);
        //restore first bit
        *(const_cast<char*>(ptr)) ^= 0x80;
    }

    char* toString(char* ptr, size_t size)
    {
        // This function is internal use only.
        assert(size >= (size_t)m_digit + 1);

        char* p = ptr;
        if (m_sign)
            *(p++) = '-';
        char* p_cache = p;
        *p = '0';
        int32* data = m_data;
        if (m_intSurplusBytes)
        {
            if (*data)
                p += sprintf(p, "%d", *data);
            ++data;
        }
        for (int i = 0; i < (int)(m_intBytes/sizeof(int32)) ; ++i)
        {
            if (*data || p != p_cache)
                p += sprintf(p, p != p_cache ? "%09d" : "%d", *data);
            ++data;
        }
        if (p_cache == p) ++p;
        if (m_dec)
        {
            int dec_n = m_dec;
            *(p++) = '.';
            for (int i = 0; i < (int)(m_decBytes/sizeof(int32)) ; ++i)
            {
                p += sprintf(p, "%09d", *(data++));
                dec_n -= DIGITS_INT32;
            }
            if (m_decSurplusBytes)
                p += sprintf(p, "%0*d", dec_n, *data);
        }
        return ptr;
    }
 
    myDecimal& operator=(const char* p)
    {
        m_error = 0;
        m_sign = (*p) == '-' ? -1 : 0;
        if (m_sign) ++p;
        const char* point = strchr(p, '.');
        
        int32* data = m_data;
        memset(m_data, 0, sizeof(m_data));
        char tmp[67];
        if (m_digit - m_dec)
        {
            size_t intchars = point ? (point - p) : strlen(p);
            int offset = (m_digit - m_dec) - (int)intchars;
            if (offset < 0 || (intchars + offset > 65))
            {
                m_error = STATUS_TOO_LARGE_VALUE;
                return *this;
            }
            if (offset)
            {
                sprintf(tmp, "%0*d%s", offset, 0, p);
                p = tmp;
            }
            int len = (m_digit - m_dec) % DIGITS_INT32;
            *(data++) = fromString(p, len);
            p += len;
            for (int i = 0; i < (int)(m_intBytes/sizeof(int32)) ; ++i)
            {
                *(data++) =  fromString(p, DIGITS_INT32);
                p += DIGITS_INT32;
            }
        }
        if (point)
        {
            p = point + 1;
            size_t len = strlen(p);
            if (len < (size_t)m_dec)
            {
                sprintf(tmp, "%s000000000", p);
                p = tmp;
            }
            const char* endp = len + p;
            for (int i = 0; i < (int)(m_decBytes/sizeof(int32)) ; ++i)
            {
                *(data++) =  fromString(p, DIGITS_INT32);
                p += DIGITS_INT32;
                if (p > endp) break;
            }
            if (p <= endp)
                *data =  fromString(p, endp - p + 1);
        }
        return *this;
    }

    double d()
    {
        char tmp[100];
        return atof(toString(tmp, 100));
    }

    __int64 i64()
    {
        char tmp[100];
        return _atoi64(toString(tmp, 100));
    }

#ifdef _WIN32
    wchar_t* toString(wchar_t* ptr, int size)
    {
        char tmp[100];
        toString(tmp, 100);
        MultiByteToWideChar(CP_ACP, 0, tmp, -1, ptr, size);
        return ptr;
    }

    myDecimal& operator=(const wchar_t* p)
    {
        char tmp[100];
        WideCharToMultiByte(CP_ACP, 0, p, -1, tmp, 100, NULL, NULL);
        return operator=(tmp);
    }
#endif

    myDecimal& operator=(const double v)
    {
        char tmp[100];
        sprintf_s(tmp, 100, "%.*lf", m_dec, v);
        return operator=(tmp);
    }

    myDecimal& operator=(const __int64 v)
    {
        char tmp[100];
        sprintf_s(tmp, 100, "%lld", v);
        return operator=(tmp);
    }
};


//------------------------------------------------------------------------------
//       class fieldShare
//------------------------------------------------------------------------------

struct blobPtr
{
    char* p;
    blobPtr():p(NULL){}
    blobPtr(char* v) {p = v;}
    ~blobPtr() { if (p) delete [] p;};
};

struct Imple
{
    stringConverter* cv;
    bzs::rtl::stringBuffer strBufs;
    //std::vector<boost::shared_array<char> > blobs;
    std::vector<blobPtr > blobs;

    Imple() : strBufs(4096)
    {
        cv = new stringConverter(nsdatabase::execCodePage(),
                                 nsdatabase::execCodePage());
    }

    ~Imple() { delete cv; }
};

fieldShare::fieldShare()
    : m_imple(new Imple()), myDateTimeValueByBtrv(true), logicalToString(false)
{
}

fieldShare::~fieldShare()
{
    delete m_imple;
}

stringConverter* fieldShare::cv() const
{
    return m_imple->cv;
}

bzs::rtl::stringBuffer* fieldShare::strBufs() const
{
    return &m_imple->strBufs;
}

char** fieldShare::blobPushBack(char* p)
{
    m_imple->blobs.push_back(blobPtr(p));
    blobPtr& bt = m_imple->blobs[m_imple->blobs.size() -1];
    return &bt.p;
    //m_imple->blobs.push_back(boost::shared_array<char>(p));
}

void fieldShare::blobClear()
{
    m_imple->blobs.clear();
}

void fieldShare::blobResize(size_t size)
{
    m_imple->blobs.resize(size);
}

//------------------------------------------------------------------------------
//       class fielddefs
//------------------------------------------------------------------------------

struct infoImple
{
    std::vector<fielddef> fields;
    boost::unordered_map<std::_tstring, int> map;
    const aliasMap_type* aliasMap;
    int  mysqlnullEnable;
    infoImple() : aliasMap(NULL), mysqlnullEnable(0){}
    infoImple(const infoImple& r)
        : fields(r.fields), map(r.map), aliasMap(r.aliasMap), mysqlnullEnable(r.mysqlnullEnable){};
};

fielddefs::fielddefs() : fieldShare(), m_imple(new infoImple)
{
}

// A fieldShare is need no copy member(s)
fielddefs::fielddefs(const fielddefs& r)
    : fieldShare(), m_imple(new infoImple(*r.m_imple))
{
}

// A fieldShare is no copy member(s)
fielddefs& fielddefs::operator=(const fielddefs& r)
{
    if (this != &r)
        *m_imple = *r.m_imple;
    return *this;
}

void fielddefs::aliasing(fielddef* p) const
{
    if (m_imple->aliasMap)
    {
        const _TCHAR* ret = m_imple->aliasMap->get(p->name());
        if (ret[0])
            p->setName(ret);
    }
}

fielddefs::~fielddefs()
{
    delete m_imple;
}

fielddefs* fielddefs::clone() const
{
    fielddefs* p = new fielddefs(*this);
    return p;
}

void fielddefs::setAliases(const aliasMap_type* p)
{
    m_imple->aliasMap = p;
}

/* calcFieldPos used by recordsetQuery::init()
   Query values hold in a memoryRecord.
   And used by this->addSelectedFields().
*/
void fielddefs::calcFieldPos(int startIndex, bool mysqlNull)
{
    ushort_td pos = 0;
    int nullfields = 0;
    for (size_t i = startIndex;i < m_imple->fields.size(); ++i)
    {
        fielddef* fd = &m_imple->fields[i];
        fd->pos = pos;
        pos += fd->len;
        fd->m_nullbytes = 0;
        fd->m_nullbit = 0;
        if (fd->isNullable())
        {
            fd->m_nullbit = nullfields;
            if (mysqlNull)
                ++nullfields;
            else
                fd->setNullable(false);
        }
    }
    if (nullfields)
    {
        int nullbytes = (nullfields + 7) / 8;
        for (size_t i = startIndex ;i < m_imple->fields.size(); ++i)
            m_imple->fields[i].m_nullbytes = nullbytes;
    }
}

void fielddefs::push_back(const fielddef* p)
{
    m_imple->fields.push_back(*p);
    int index = (int)m_imple->fields.size() - 1;
    fielddef* pp = &m_imple->fields[index];

    // convert field name of table charset to recordset schema charset.
    _TCHAR tmp[FIELD_NAME_SIZE * 3];
    pp->setName(p->name(tmp));
    if (pp->isNullable() && pp->nullbytes())
        ++m_imple->mysqlnullEnable;
    // reset update indicator
    pp->enableFlags.bitE = false;

    // For activeTable need replacing name
    aliasing(pp);
    m_imple->map[pp->name()] = index;
}

void fielddefs::remove(int index)
{
    m_imple->fields.erase(m_imple->fields.begin() + index);
    boost::unordered_map<std::_tstring, int>::iterator it =
        m_imple->map.begin();
    while (it != m_imple->map.end())
    {
        if ((*it).second == index)
            it = m_imple->map.erase(it);
        else
        {
            if ((*it).second > index)
                (*it).second--;
            ++it;
        }
    }
}

void fielddefs::reserve(size_t size)
{
    m_imple->fields.reserve(size);
}

void fielddefs::clear()
{
    m_imple->fields.clear();
    m_imple->map.clear();
    m_imple->mysqlnullEnable = 0;
}

void fielddefs::resetUpdateIndicator()
{
    std::vector<fielddef>::iterator it = m_imple->fields.begin();
    while (it != m_imple->fields.end())
    {
        (*it).enableFlags.bitE = false;
        ++it;
    }
}

bool fielddefs::checkIndex(int index) const
{
    return (index >= 0 && index < (int)m_imple->fields.size());
}

void fielddefs::addAliasName(int index, const _TCHAR* name)
{
    assert(checkIndex(index));
    //replace original name
    m_imple->fields[index].setName(name);
    m_imple->map[name] = index;
}

int fielddefs::indexByName(const std::_tstring& name) const
{
    if (m_imple->map.count(name) == 0)
        return -1;

    return m_imple->map.at(name);
}

const fielddef& fielddefs::operator[](int index) const
{
    assert(checkIndex(index));

    return m_imple->fields[index];
}

const fielddef& fielddefs::operator[](const _TCHAR* name) const
{
    return m_imple->fields[indexByName(std::_tstring(name))];
}

const fielddef& fielddefs::operator[](const std::_tstring& name) const
{
    return m_imple->fields[indexByName(name)];
}

size_t fielddefs::size() const
{
    return m_imple->fields.size();
}

size_t fielddefs::totalFieldLen() const
{
    const fielddef& fd = m_imple->fields[m_imple->fields.size() - 1];
    return fd.pos + fd.len + fd.nullbytes();
}

void fielddefs::addAllFileds(const tabledef* def)
{
    m_imple->fields.clear();
    m_imple->mysqlnullEnable = 0;
    short blobCount = 0;
    for (int i = 0; i < def->fieldCount; ++i)
    {
        const fielddef* fd = &def->fieldDefs[i];
        push_back(fd);
        m_imple->fields[m_imple->fields.size() - 1].setPadCharDefaultSettings();
        if (fd->isBlob())
            ++blobCount;
    }
    blobResize(blobCount);
}

void fielddefs::addSelectedFields(const table* tb)
{
    int n = tb->getCurProcFieldCount();
    m_imple->fields.reserve(n + size());
    const tabledef* def = tb->tableDef();
    int  startIndex = (int)m_imple->fields.size();
    for (int i = 0; i < n; ++i)
        push_back(&def->fieldDefs[tb->getCurProcFieldIndex(i)]);
    calcFieldPos(startIndex, (def->nullfields() != 0));
    // Defalut field charset
    cv()->setCodePage(mysql::codePage(def->charsetIndex));
}

bool fielddefs::mysqlnullEnable() const
{
    return (m_imple->mysqlnullEnable != 0);
}

bool fielddefs::canUnion(const fielddefs& src) const
{
    if (size() != src.size())
        return false;
    for (int i = 0; i < (int)m_imple->fields.size(); ++i)
    {
        const fielddef& l = m_imple->fields[i];
        const fielddef& r = src.m_imple->fields[i];
        if (l.pos != r.pos)
            return false;
        if (l.len != r.len)
            return false;
        if (l.type != r.type)
            return false;
        if (l.charsetIndex() != r.charsetIndex())
            return false;
        if (l.isNullable() != r.isNullable())
            return false;
    }
    return true;
}

fielddefs* fielddefs::create()
{
    return new fielddefs();
}

void fielddefs::release()
{
    delete this;
}

//------------------------------------------------------------------------------
//       class field
//------------------------------------------------------------------------------
static fielddef fdDummy;
#define NUMBUFSIZE 70

DLLLIB const fielddef& dummyFd()
{
    fdDummy.type = ft_integer;
    fdDummy.pos = 0;
    fdDummy.len = 2;
    fdDummy.setName(_T("Invalid filed"));
    return fdDummy;
}

/* 
    For not string type
    Cast is faster than memcopy.

    Currency : if get by int64 pure value, if get by double 1/10000 value.
*/
__int64 field::readValue64() const
{
    const uchar_td* ptr = (uchar_td*)m_ptr + m_fd->pos;
    __int64 ret = 0;
    switch (m_fd->type)
    {
    case ft_integer:
    case ft_autoinc:
        switch (m_fd->len)
        {
        case 1:
            ret = *((char*)ptr);
            break;
        case 2:
            ret = *((short*)ptr);
            break;
        case 3:
            ret = int24toInt((const char*)ptr);
            break;
        case 4:
            ret = *((int*)ptr);
            break;
        case 8:
            ret = *((__int64*)ptr);
            break;
        }
        break;
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_currency:
    case ft_myyear:
    case ft_enum: // 1 - 2 byte      0 - 65535
    case ft_set:  // 1 2 3 4 8 byte  64 item
    case ft_bit:  // 1 - 8 byte      64 item
        switch (m_fd->len)
        {
        case 1:
            ret = *((unsigned char*)ptr);
            break;
        case 2:
            ret = *((unsigned short*)ptr);
            break;
        case 3:
            ret = int24toUint((const char*)ptr);
            break;
        case 4:
            ret = *((unsigned int*)ptr);
            break;
        case 8:
            ret = *((__int64*)ptr);
            break;
        case 5:
        case 6:
        case 7:
            memcpy(&ret, ptr, m_fd->len);// for bit
            break;
        }
        break;
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
        //get big endian value
        switch (m_fd->len)
        {
        case 3:
        case 5:
        case 6:
        case 7:
            memcpy(&ret, ptr, m_fd->len);
            break;
        case 4:
            ret = *((unsigned int*)ptr);
            break;
        case 8:
            ret = *((__int64*)ptr);
            break;
        default:
            return ret;
        }
        break;
    default:
        assert(0);
    }
    return ret;
}

void field::storeValue64(__int64 value)
{
    uchar_td* ptr = (uchar_td*)m_ptr + m_fd->pos;
    switch (m_fd->type)
    {
    case ft_integer:
    case ft_autoinc:
    {
        switch (m_fd->len)
        {
        case 1:
            *((char*)ptr) = (char)value;
            break;
        case 2:
            *((short*)ptr) = (short)value;
            break;
        case 3:
            storeInt24((int)value, (char*) ptr);
            break;
        case 4:
            *((int*)ptr) = (int)value;
            break;
        case 8:
            *((__int64*)ptr) = value;
            break;
        }
        break;
    }
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_currency:
    case ft_myyear:
    case ft_enum: // 1 - 2 byte      0 - 65535
    case ft_set:  // 1 2 3 4 8 byte  64 item
    case ft_bit:  // 1 - 8 byte      64 item
    {
        switch (m_fd->len)
        {
        case 1:
            *((char*)ptr) = (char)value;
            break;
        case 2:
            *((unsigned short*)ptr) = (unsigned short)value;
            break;
        case 3:
            storeUint24((int)value, (char*) ptr);
            break;
        case 4:
            *((unsigned int*)ptr) = (unsigned int)value;
            break;
        case 8:
            *((__int64*)ptr) = value;
            break;
        case 5:
        case 6:
        case 7:
            memcpy(ptr, &value, m_fd->len); // for bit
            break;
        }
        break;
    }
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
       switch (m_fd->len)
        {
        case 3:
        case 5:
        case 6:
        case 7:
            memcpy(ptr, &value, m_fd->len);
            break;
        case 4:
            *((unsigned int*)ptr) = (unsigned int)value;
            break;
        case 8:
            *((__int64*)ptr) = value;
            break;
        }
        break;
    }
    default:
        assert(0);
    }
}

double field::readValueDbl() const
{
    const uchar_td* ptr = (uchar_td*)m_ptr + m_fd->pos;
    double ret = 0;
    switch (m_fd->type)
    {
    case ft_bfloat:
    case ft_float:
        switch (m_fd->len)
        {
        case 4:
            ret = (double)*((float*)ptr);
            break;
        case 8:
            ret = (double)*((double*)ptr);
            break;
        case 10: // long double
            ret = (double)*((long double*)ptr);
            break;
        }
        break;
    default:
        assert(0);
    }
    return ret;
}

void field::storeValueDbl(double value)
{
    uchar_td* ptr = (uchar_td*)m_ptr + m_fd->pos;
    switch (m_fd->type)
    {
    case ft_bfloat:
    case ft_float:
    {
        switch (m_fd->len)
        {
        case 4:
            *((float*)ptr) = (float)value;
            break;
        case 8:
            *((double*)ptr) = (double)value;
            break;
        case 10: // long double
            *((long double*)ptr) = (long double)value;
            break;
        }
        break;
    }
    default:
        assert(0);
    }
}

void field::storeValueStrA(const char* data) 
{
    char* p = (char*)m_ptr + m_fd->pos;
    m_fds->cv()->setCodePage(mysql::codePage(m_fd->charsetIndex()));
    switch (m_fd->type)
    {
    case ft_string:
        if (m_fd->isUsePadChar())
            return store<stringStore, char, char>(p, data, *m_fd, m_fds->cv());
        return store<binaryStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_note:
    case ft_zstring:
        return store<zstringStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_wzstring:
        return store<wzstringStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_wstring:
        if (m_fd->isUsePadChar())
            return store<wstringStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
        return store<wbinaryStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_mychar:
        return store<myCharStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_myvarchar:
        return store<myVarCharStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return store<myVarBinaryStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_mywchar:
        return store<myWcharStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_mywvarchar:
        return store<myWvarCharStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_mywvarbinary:
        return store<myWvarBinaryStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_myblob:
    case ft_mytext:
    {
        //char* tmp = blobStore<char>(p, data, *m_fd, m_fds->cv());
        char** pp = const_cast<fielddefs*>(m_fds)->blobPushBack(NULL);
        *pp = blobStore<char>(p, data, *m_fd, m_fds->cv(), pp);
        break;
    }
    default:
        assert(0);
    }
}

const char* field::readValueStrA() const
{
    char* data = (char*)m_ptr + m_fd->pos;
    m_fds->cv()->setCodePage(mysql::codePage(m_fd->charsetIndex()));
    switch (m_fd->type)
    {
    case ft_string:
        return read<stringStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                             m_fds->cv(), m_fd->isTrimPadChar());
    case ft_note:
    case ft_zstring:
        return read<zstringStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv());
    case ft_wzstring:
        return read<wzstringStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                                m_fds->cv());
    case ft_wstring:
        return read<wstringStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv(), m_fd->isTrimPadChar());
    case ft_mychar:
        return read<myCharStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                             m_fds->cv(), m_fd->isTrimPadChar());
    case ft_myvarchar:
        return read<myVarCharStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                                m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                                  m_fds->cv());
    case ft_mywchar:
        return read<myWcharStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv(), m_fd->isTrimPadChar());
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                                  m_fds->cv());
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, char>(data, m_fds->strBufs(),
                                                    *m_fd, m_fds->cv());
    case ft_myblob:
    case ft_mytext:
        return readBlob<char>(data, m_fds->strBufs(), *m_fd, m_fds->cv());
    default:
        assert(0);
        return NULL;
    }
}

#ifdef _WIN32
void field::storeValueStrW(const WCHAR* data)
{
    char* p = (char*)m_ptr + m_fd->pos;
    m_fds->cv()->setCodePage(mysql::codePage(m_fd->charsetIndex()));
    switch (m_fd->type)
    {
    case ft_string:
        if (m_fd->isUsePadChar())
            return store<stringStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
        return store<binaryStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_note:
    case ft_zstring:
        return store<zstringStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_wzstring:
        return store<wzstringStore, WCHAR, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_wstring:
        if (m_fd->isUsePadChar())
            return store<wstringStore, WCHAR, WCHAR>(p, data, *m_fd, m_fds->cv());
        return store<wbinaryStore, WCHAR, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_mychar:
        return store<myCharStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_myvarchar:
        return store<myVarCharStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return store<myVarBinaryStore, char, WCHAR>(p, data, *m_fd,
                                                    m_fds->cv());
    case ft_mywchar:
        return store<myWcharStore, WCHAR, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_mywvarchar:
        return store<myWvarCharStore, WCHAR, WCHAR>(p, data, *m_fd,
                                                    m_fds->cv());
    case ft_mywvarbinary:
        return store<myWvarBinaryStore, WCHAR, WCHAR>(p, data, *m_fd,
                                                      m_fds->cv());
    case ft_myblob:
    case ft_mytext:
    {
        //char* tmp = blobStore<WCHAR>(p, data, *m_fd, m_fds->cv());
        //const_cast<fielddefs*>(m_fds)->blobPushBack(tmp);

        char** pp = const_cast<fielddefs*>(m_fds)->blobPushBack(NULL);
        *pp = blobStore<WCHAR>(p, data, *m_fd, m_fds->cv(), pp);
        return;
    }
    default:
        assert(0);
    }
}

const WCHAR* field::readValueStrW() const
{
    char* data = (char*)m_ptr + m_fd->pos;
    m_fds->cv()->setCodePage(mysql::codePage(m_fd->charsetIndex()));
    switch (m_fd->type)
    {
    case ft_string:
        return read<stringStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv(), m_fd->isTrimPadChar());
    case ft_note:
    case ft_zstring:
        return read<zstringStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv());
    case ft_wzstring:
        return read<wzstringStore, WCHAR, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                                 m_fds->cv());
    case ft_wstring:
        return read<wstringStore, WCHAR, WCHAR>(
            data, m_fds->strBufs(), *m_fd, m_fds->cv(), m_fd->isTrimPadChar());
    case ft_mychar:
        return read<myCharStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv(), m_fd->isTrimPadChar());
    case ft_myvarchar:
        return read<myVarCharStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                                 m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, WCHAR>(data, m_fds->strBufs(),
                                                   *m_fd, m_fds->cv());
    case ft_mywchar:
        return read<myWcharStore, WCHAR, WCHAR>(
            data, m_fds->strBufs(), *m_fd, m_fds->cv(), m_fd->isTrimPadChar());
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, WCHAR>(data, m_fds->strBufs(),
                                                   *m_fd, m_fds->cv());
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, WCHAR>(data, m_fds->strBufs(),
                                                     *m_fd, m_fds->cv());
    case ft_myblob:
    case ft_mytext:
        return readBlob<WCHAR>(data, m_fds->strBufs(), *m_fd, m_fds->cv());
    default:
        assert(0);
        return NULL;
    }
}
#endif //_WIN32

void field::storeValueNumeric(double data)
{ // Double  -> Numeric

    char buf[NUMBUFSIZE] = "%+0";
    char dummy[NUMBUFSIZE];
    int point;
    int n;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    bool sign = false;
    char* t;

    point = m_fd->len + 1;

    _ltoa_s(point, dummy, NUMBUFSIZE, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_fd->decimals, dummy, NUMBUFSIZE, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf_s(dummy, NUMBUFSIZE, buf, data);
    if (dummy[0] == '-')
        sign = true;

    strcpy(buf, &dummy[point - m_fd->decimals] + 1);
    dummy[point - m_fd->decimals] = 0x00;
    strcat(dummy, buf);

    n = atol(&dummy[m_fd->len]);
    if (sign)
        n += 10;
    t = dummy + 1;
    switch (m_fd->type)
    {
    case ft_numeric:
        dummy[m_fd->len] = dp[n];
        break;
    case ft_numericsa:
        dummy[m_fd->len] = dpsa[n];
        break;
    case ft_numericsts:
        if (sign)
            strcat(dummy, "-");
        else
            strcat(dummy, "+");
        t += 1;
        break;
    default:
        assert(0);
    }
    memcpy((void*)((char*)m_ptr + m_fd->pos), t, m_fd->len);
}

double field::readValueNumeric() const
{
    char* t;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    char* pdp = NULL;
    int i;
    char buf[NUMBUFSIZE] = { 0x00 };
    char dummy[NUMBUFSIZE];

    buf[0] = '+';
    strncpy(buf + 1, (char*)((char*)m_ptr + m_fd->pos), m_fd->len);

    t = &(buf[m_fd->len]);

    switch (m_fd->type)
    {
    case ft_numeric:
        pdp = dp;
        break;
    case ft_numericsa:
        pdp = dpsa;
        break;
    case ft_numericsts:
        buf[0] = *t;
        *t = 0x00;
        break;
    default:
        assert(0);
    }

    if (pdp)
    {
        for (i = 0; i < 21; i++)
        {
            if (*t == pdp[i])
            {
                if (i > 10)
                {
                    buf[0] = '-';
                    *t = (char)(i + 38);
                }
                else
                    *t = (char)(i + 48);
                break;
            }
        }
    }

    t = buf + strlen(buf) - m_fd->decimals;
    strcpy(dummy, t);
    *t = '.';
    strcpy(t + 1, dummy);
    return atof(buf);
}

void field::storeValueDecimal(double data)
{ // Double  -> Decimal
    assert(m_fd->type == ft_decimal || m_fd->type == ft_money);
    char buf[NUMBUFSIZE] = "%+0";
    char dummy[NUMBUFSIZE];
    int point;
    bool sign = false;
    unsigned char n;
    int i, k;
    int strl;
    bool offset = false;
    
    point = (m_fd->len) * 2;
    _ltoa_s(point, dummy, NUMBUFSIZE, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_fd->decimals, dummy, NUMBUFSIZE, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf_s(dummy, NUMBUFSIZE, buf, data);
    if (dummy[0] == '-')
        sign = true;

    strl = (int)strlen(dummy + 1) - 1;
    if (strl % 2 == 1)
        strl = strl / 2;
    else
    {
        strl = strl / 2 + 1;
        offset = true;
    }
    memset(buf, 0, 30);
    k = 0;
    n = 0;
    point = (int)strlen(dummy + 1);
    if (strl <= m_fd->len)
    {
        for (i = 1; i <= point; i++)
        {
            if ((dummy[i] == '-') || (dummy[i] == '.'))
                ;
            else
            {
                if (offset)
                {
                    n = (unsigned char)(n + dummy[i] - 48);
                    buf[k] = n;
                    offset = false;
                    k++;
                }
                else
                {
                    n = (unsigned char)(dummy[i] - 48);
                    n = (unsigned char)(n << 4);
                    offset = true;
                }
            }
        }
        if (sign)
            buf[k] += ((unsigned char)(n + 13));
        else
            buf[k] += ((unsigned char)(n + 12));
    }
    memcpy((void*)((char*)m_ptr + m_fd->pos), buf, m_fd->len);
}

double field::readValueDecimal() const
{
    assert(m_fd->type == ft_decimal || m_fd->type == ft_money);
    unsigned char buf[20] = { 0x00 };
    char result[30] = { 0x00 };
    char n[10];
    int i;
    char* t;
    unsigned char sign;
    int len = m_fd->len;
    result[0] = '+';
    memcpy(buf, (void*)((char*)m_ptr + m_fd->pos), len);
    sign = (unsigned char)(buf[len - 1] & 0x0F);
    buf[len - 1] = (unsigned char)(buf[len - 1] & 0xF0);
    for (i = 0; i < len; i++)
    {
        sprintf_s(n, 10, "%02x", buf[i]);
        strcat(result, n);
    }
    i = (int)strlen(result);

    if (sign == 13)
        result[0] = '-';
    result[i - 1] = 0x00;

    t = result + (m_fd->len * 2) - m_fd->decimals;
    strcpy((char*)buf, t);
    *t = '.';
    strcpy(t + 1, (char*)buf);
    return atof(result);
}

//---------------------------------------------------------------------------
#define CASE_TEXT case ft_string: \
    case ft_note: \
    case ft_zstring: \
    case ft_wzstring: \
    case ft_wstring: \
    case ft_mychar: \
    case ft_myvarchar: \
    case ft_lstring: \
    case ft_myvarbinary: \
    case ft_mywchar: \
    case ft_mywvarchar: \
    case ft_mywvarbinary: \
    case ft_myblob: \
    case ft_mytext: 

#define CASE_TEXTA case ft_string: \
    case ft_note: \
    case ft_zstring: \
    case ft_mychar: \
    case ft_myvarchar: \
    case ft_lstring: \
    case ft_myvarbinary: \
    case ft_myblob: \
    case ft_mytext: 

#define CASE_TEXTW case ft_wzstring: \
    case ft_wstring: \
    case ft_mywchar: \
    case ft_mywvarchar: \
    case ft_mywvarbinary:

#define CASE_INT case ft_integer: \
    case ft_autoinc: 

#define CASE_UINT case ft_uinteger: \
    case ft_autoIncUnsigned:  \
    case ft_set: \
    case ft_enum:


#define CASE_FLOAT case ft_bfloat: \
    case ft_float: 

#define CASE_NUMERIC case ft_numeric: \
    case ft_numericsa: \
    case ft_numericsts:

#define CASE_DECIMAL case ft_decimal: \
    case ft_money: 
   
inline __int64 logical_str_to_64(bool logicalToString, const char* data)
{
    if (logicalToString)
    {
        char tmp[5];
        strncpy(tmp, data, 5);
        if (strcmp(_strupr(tmp), "YES") == 0)
            return 1;
        else
            return 0;
    }
    else
        return atol(data);
}

#ifdef _WIN32
inline __int64 logical_str_to_64(bool logicalToString, const wchar_t* data)
{
    if (logicalToString)
    {
        WCHAR tmp[5];
        wcsncpy((wchar_t*)tmp, data, 5);
        if (wcscmp((const wchar_t*)_wcsupr(tmp), L"YES") == 0)
            return 1;
        else
            return 0;
    }
    else
        return _wtol(data);
}
#endif

//---------------------------------------------------------------------------
//      set functions
//---------------------------------------------------------------------------
void field::setFVA(const char* data)
{
    assert(m_ptr);
        
    __int64 value;
    char* p = (char*)m_ptr + m_fd->pos;
    if (data == NULL)
    {
        memset(p, 0, m_fd->len);
        setNull(true);
        return;
    }
    setNull(false);
    switch(m_fd->type)
    {
    CASE_TEXT
        storeValueStrA(data);
        return;
    CASE_FLOAT
        storeValueDbl(atof(data));
        return;
    CASE_NUMERIC
        storeValueNumeric(atof(data));
        return;
    CASE_DECIMAL
        storeValueDecimal(atof(data));
        return;
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d = data;
        d.store(p);
        return;
    }
    CASE_INT
        value = _atoi64(data);
        break;
    CASE_UINT
        value = (__int64)strtoull(data, NULL, 10);
        break;
    case ft_bit:
        value = changeEndian((__int64)strtoull(data, NULL, 10), m_fd->len);
        break;
    case ft_logical:
        value = logical_str_to_64(m_fds->logicalToString, data);
        break;
    case ft_date: 
        value = atobtrd((const char*)data).i;
        break;
    case ft_time:
        value = atobtrt((const char*)data).i;
        break;
    case ft_datetime:
        value = atobtrs((const char*)data).i64;
        break;
    case ft_myyear:
        value = _atoi64(data);
        if (value > 1900) value -= 1900;
        break;
    case ft_mydate:
        value = str_to_64<myDate, char>(m_fd->decimals, true, data);
        break;
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_mytimestamp:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maTimeStamp, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myTimeStamp, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_currency:
        value = (__int64)(atof(data) * 10000);
        break;
    default: // ft_lvar ft_mygeometry ft_myjson
        return;
    }
    storeValue64(value);
}

#ifdef _WIN32
void field::setFVW(const wchar_t* data)
{
    assert(m_ptr);
        
    __int64 value;
    char* p = (char*)m_ptr + m_fd->pos;
    if (data == NULL)
    {
        memset(p, 0, m_fd->len);
        setNull(true);
        return;
    }
    setNull(false);
    switch(m_fd->type)
    {
    CASE_TEXT
        storeValueStrW(data);
        return;
    CASE_FLOAT
        storeValueDbl(_wtof(data));
        return;
    CASE_NUMERIC
        storeValueNumeric(_wtof(data));
        return;
    CASE_DECIMAL
        storeValueDecimal(_wtof(data));
        return;
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d = data;
        d.store(p);
        return;
    }
    CASE_INT
        value = _wtoi64(data);
        break;
    CASE_UINT
        value = (__int64)wcstoull(data, NULL, 10);
        break;
    case ft_bit:
        value = changeEndian((__int64)wcstoull(data, NULL, 10), m_fd->len);
        break;
    case ft_logical:
        value = logical_str_to_64(m_fds->logicalToString, data);
        break;
    case ft_date: 
        value = atobtrd(data).i;
        break;
    case ft_time:
        value = atobtrt(data).i;
        break;
    case ft_datetime:
        value = atobtrs(data).i64;
        break;
    case ft_myyear:
        value = _wtoi64(data);
        if (value > 1900) value -= 1900;
        break;
    case ft_mydate:
        value = str_to_64<myDate, wchar_t>(m_fd->decimals, true, data);
        break;
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_mytimestamp:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maTimeStamp, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myTimeStamp, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            value = str_to_64<maDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        else
            value = str_to_64<myDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), data);
        break;
    case ft_currency:
        value = (__int64)(_wtof(data) * 10000);
        break;
    default: // ft_lvar ft_mygeometry ft_myjson
        return;
    }
    storeValue64(value);
}
#endif //_WIN32

void field::setFV(__int64 data)
{
    assert(m_ptr);
    setNull(false);

    switch(m_fd->type)
    {
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
        //Convert big endian
        bool bigendian = !m_fd->isLegacyTimeFormat();
        if (m_fd->type == ft_mydatetime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                data = getStoreValue<maDateTime>(m_fd->decimals, bigendian, data);
            else
                data = getStoreValue<myDateTime>(m_fd->decimals, bigendian, data);
        }
        else if (m_fd->type == ft_mytimestamp)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                data = getStoreValue<maTimeStamp>(m_fd->decimals, bigendian, data);
            else
                data = getStoreValue<myTimeStamp>(m_fd->decimals, bigendian, data);
        }
        else if(m_fd->type == ft_mytime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                data = getStoreValue<maTime>(m_fd->decimals, bigendian, data);
            else
                data = getStoreValue<myTime>(m_fd->decimals, bigendian, data);
        }
        //fall through
    }
    case ft_date:
    case ft_time:
    case ft_datetime:
    case ft_mydate:
    case ft_currency:
    case ft_logical:
    CASE_INT
    CASE_UINT
        storeValue64(data);
        break;
    case ft_bit:
        storeValue64(changeEndian(data, m_fd->len));
        break;
    case ft_myyear:
        if (data > 1900) data -= 1900;
        storeValue64(data);
        break;
    CASE_FLOAT
        storeValueDbl((double)data);
        break;
    CASE_NUMERIC
        storeValueNumeric((double)data);
        break;
    CASE_DECIMAL
        storeValueDecimal((double)data);
        break;
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d = data;
        d.store((char*)m_ptr + m_fd->pos);
        break;
    }
#ifdef LINUX
    CASE_TEXTW
#endif
    CASE_TEXTA
    {
        char buf[NUMBUFSIZE];
         _i64toa_s(data, buf, NUMBUFSIZE, 10);
        storeValueStrA(buf);
        break;
    }
#ifndef LINUX
    CASE_TEXTW
    {
        WCHAR buf[NUMBUFSIZE];
         _i64tow_s(data, buf, NUMBUFSIZE, 10);
        storeValueStrW(buf);
        break;
    }
#endif
    default: // ft_lvar ft_mygeometry ft_myjson
        break;
    }
}

void field::setFV(double data)
{
    assert(m_ptr);
    setNull(false);
    __int64 i64 = (__int64)data;
    switch (m_fd->type)
    {
    CASE_FLOAT
        storeValueDbl(data);
        break;
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    case ft_currency:
    {
        //Convert big endian
        bool bigendian = !m_fd->isLegacyTimeFormat();
        if (m_fd->type == ft_mydatetime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                i64 = getStoreValue<maDateTime>(m_fd->decimals, bigendian, i64);
            else
                i64 = getStoreValue<myDateTime>(m_fd->decimals, bigendian, i64);
        }
        else if (m_fd->type == ft_mytimestamp)
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                i64 = getStoreValue<maTimeStamp>(m_fd->decimals, bigendian, i64);
            else
                i64 = getStoreValue<myTimeStamp>(m_fd->decimals, bigendian, i64);
        else if(m_fd->type == ft_mytime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                i64 = getStoreValue<maTime>(m_fd->decimals, bigendian, i64);
            else
                i64 = getStoreValue<myTime>(m_fd->decimals, bigendian, i64);
        }
        else if(m_fd->type == ft_currency)
            i64 = (__int64)(data * 10000 + 0.5);
        //fall through
    }
    case ft_date:
    case ft_time:
    case ft_datetime: 
    case ft_mydate:
    case ft_logical:
    CASE_INT
    CASE_UINT
        storeValue64(i64);
        break;
    case ft_bit:
        storeValue64(changeEndian(i64, m_fd->len));
        break;
    case ft_myyear:
        if (i64 > 1900) i64 -= 1900;
        storeValue64(i64);
        break;
    CASE_NUMERIC
        storeValueNumeric(data);
        break;
    CASE_DECIMAL
        storeValueDecimal(data);
        break;
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d = data;
        d.store((char*)m_ptr + m_fd->pos);
        break;
    }
#ifdef LINUX
    CASE_TEXTW
#endif
    CASE_TEXTA
    {
        char buf[NUMBUFSIZE];
        sprintf_s(buf, NUMBUFSIZE, "%lf", data);
        storeValueStrA(buf);
        break;
    }
#ifndef LINUX
    CASE_TEXTW
    {
        WCHAR buf[NUMBUFSIZE];
        swprintf_s((wchar_t*)buf, NUMBUFSIZE, L"%lf", data);
        storeValueStrW(buf);
        break;
    }
#endif
    default: // ft_lvar ft_mygeometry ft_myjson
        break;
    }
}

/* if blob and text ,set binary data that is only set pointer. it is not copied.
 *  Caller must hold data until it sends to the server.
 */
void field::setFV(const void* data, uint_td size)
{
    assert(m_ptr);
    char* p = (char*)m_ptr + m_fd->pos;
    if (data == NULL)
    {
        memset(p, 0, m_fd->len);
        setNull(true);
        return;
    }
    setNull(false);
    switch (m_fd->type)
    {
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_lstring:
    case ft_lvar:
    {
        int sizeByte = m_fd->varLenBytes();
        size = std::min<uint_td>((uint_td)(m_fd->len - sizeByte), size);
        memset(p, 0, m_fd->len);
        memcpy(p, &size, sizeByte);
        memcpy(p + sizeByte, data, size);
        break;
    }
    case ft_myjson:
    case ft_mygeometry:
    case ft_myblob:
    case ft_mytext:
    {
        int sizeByte = m_fd->len - 8;
        memset(p, 0, m_fd->len);
        memcpy(p, &size, sizeByte);
        memcpy(p + sizeByte, &data, sizeof(char*));
        break;
    }
    default:
        size = std::min<uint_td>((uint_td)m_fd->len, size);
        memset(p, 0, m_fd->len);
        memcpy(p, data, size);
    }
}

//---------------------------------------------------------------------------
//                       getFV functions
//---------------------------------------------------------------------------

const char* field::getFVAstr() const
{
    if (!m_ptr) return "";
  
    switch (m_fd->type)
    {
    CASE_TEXT
        return readValueStrA();
    }

    char* p = m_fds->strBufs()->getPtrA(max(m_fd->len * 2, NUMBUFSIZE));
    switch (m_fd->type)
    {
    CASE_INT
        _i64toa_s(readValue64(), p, NUMBUFSIZE, 10);
        return p;
    CASE_UINT
        _ui64toa_s(readValue64(), p, NUMBUFSIZE, 10);
        return p;
    case ft_bit:
        _ui64toa_s(changeEndian(readValue64(), m_fd->len), p, NUMBUFSIZE, 10);
        break;
    case ft_myyear:
    {
        __int64 v = readValue64();
        if (v) v += 1900;
        _i64toa_s(v , p, NUMBUFSIZE, 10);
        return p;
    }
    case ft_logical:
    {
        int v = (int)readValue64();
        if (m_fds->logicalToString)
            return v ? "Yes": "No";
        else
            _ltoa_s(v, p, NUMBUFSIZE, 10);
        return p;
    }
    case ft_date:
        return btrdtoa((int)readValue64(), p);
    case ft_time:
        return btrttoa((int)readValue64(), p);
    case ft_datetime: 
        return btrstoa(readValue64(), p);
    case ft_timestamp:
        return btrTimeStamp(readValue64()).toString(p);
    case ft_mydate:
        return date_time_str<myDate, char>(m_fd->len, true, readValue64(), p, NUMBUFSIZE);
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mytimestamp:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTimeStamp, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myTimeStamp, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mygeometry:
    case ft_myjson:
        return "";
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d.read((char*)m_ptr + m_fd->pos);
        return d.toString(p, NUMBUFSIZE);
    }
    }

    double v = 0;
    switch (m_fd->type)
    {
    case ft_currency:
        v = readValue64() / (double)10000;
        break;
    CASE_FLOAT
        v = readValueDbl();
        break;
    CASE_NUMERIC
        v = readValueNumeric();
        break;
    CASE_DECIMAL
        v = readValueDecimal();
        break;

    }
    sprintf_s(p, NUMBUFSIZE, "%.*lf", m_fd->decimals, v);
    return p;
}

#ifdef _WIN32
const wchar_t* field::getFVWstr() const
{
    if (!m_ptr) return L"";

    switch (m_fd->type)
    {
    CASE_TEXT
        return readValueStrW();
    }

    wchar_t* p = (wchar_t*)m_fds->strBufs()->getPtrW(max(m_fd->len * 2, NUMBUFSIZE));
    switch (m_fd->type)
    {
    CASE_INT
        _i64tow_s(readValue64(), p, NUMBUFSIZE, 10);
        return p;
    CASE_UINT
        _ui64tow_s(readValue64(), p, NUMBUFSIZE, 10);
        return p;
    case ft_bit:
        _ui64tow_s(changeEndian(readValue64(), m_fd->len), p, NUMBUFSIZE, 10);
        break;
    case ft_myyear:
    {
         __int64 v = readValue64();
        if (v) v += 1900;
        _i64tow_s(v, p, NUMBUFSIZE, 10);
        return p;
    }
    case ft_logical:
    {
        int v = (int)readValue64();
        if (m_fds->logicalToString)
            return v ? L"Yes": L"No";
        else
            _ltow_s(v, p, NUMBUFSIZE, 10);
        return p;
    }
    case ft_date:
        return btrdtoa((int)readValue64(), p);
    case ft_time:
        return btrttoa((int)readValue64(), p);
    case ft_datetime: 
        return btrstoa(readValue64(), p);
    case ft_timestamp:
        return btrTimeStamp(readValue64()).toString(p);
    case ft_mydate:
        return date_time_str<myDate, wchar_t>(m_fd->decimals, true, readValue64(), p, NUMBUFSIZE);
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mytimestamp:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTimeStamp, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
        else
            return date_time_str<myTimeStamp, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p, NUMBUFSIZE);
    case ft_mygeometry:
    case ft_myjson:
        return L"";
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d.read((char*)m_ptr + m_fd->pos);
        return d.toString(p, NUMBUFSIZE);
    }
    }

    double v = 0;
    switch (m_fd->type)
    {
    case ft_currency:
        v = readValue64() / (double)10000;
        break;
    CASE_FLOAT
        v = readValueDbl();
        break;
    CASE_NUMERIC
        v = readValueNumeric();
        break;
    CASE_DECIMAL
        v = readValueDecimal();
        break;
    }
    swprintf_s(p, NUMBUFSIZE, L"%.*lf",m_fd->decimals, v);

    return p;
}
#endif //_WIN32

double field::getFVdbl() const
{
    if (!m_ptr) return 0;
    double ret = 0;
    switch (m_fd->type)
    {
    CASE_FLOAT
    case ft_timestamp:
        return (double)readValueDbl();
    CASE_NUMERIC
        return (double)readValueNumeric();
    CASE_DECIMAL
        return (double)readValueDecimal();
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d.read((char*)m_ptr + m_fd->pos);
        return d.d();
    }
    CASE_INT
        return  (double)readValue64();
    case ft_myyear:
    {
         __int64 v = readValue64();
        if (v) v += 1900;
        return (double)(v);
    }
    case ft_currency:
        return ((double)readValue64() / (double)10000);
    CASE_UINT
    case ft_mydate:
    case ft_time:
    case ft_date:
    case ft_datetime:
    case ft_logical:
        return  (double)((unsigned __int64)readValue64());
    case ft_bit:
        return (double)((unsigned __int64)changeEndian(readValue64(), m_fd->len));
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
        __int64 v = readValue64();
        bool bigendian = !m_fd->isLegacyTimeFormat();
        if (m_fd->type ==  ft_mytime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return (double)((unsigned __int64)getInternalValue<maTime>(m_fd->decimals, bigendian, v));
            else
                return (double)((unsigned __int64)getInternalValue<myTime>(m_fd->decimals, bigendian, v));
        }
        else if (m_fd->type ==  ft_mydatetime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return (double)((unsigned __int64)getInternalValue<maDateTime>(m_fd->decimals, bigendian, v));
            else
                return (double)((unsigned __int64)getInternalValue<myDateTime>(m_fd->decimals, bigendian, v));
        }
        else if (m_fd->type ==  ft_mytimestamp)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return (double)((unsigned __int64)getInternalValue<maTimeStamp>(m_fd->decimals, bigendian, v));
            else
                return (double)((unsigned __int64)getInternalValue<myTimeStamp>(m_fd->decimals, bigendian, v));
        }
        assert(0);
    }
#ifdef LINUX
    CASE_TEXTW
#endif
    CASE_TEXTA
        return atof(readValueStrA());
#ifndef LINUX
    CASE_TEXTW
        return _wtof((const wchar_t*)readValueStrW());
#endif
    default: // ft_lvar ft_mygeometry ft_myjson
         break;
    }
    return ret;
}

__int64 field::getFV64() const
{
    if (!m_ptr) return 0;
    switch (m_fd->type)
    {
    CASE_INT
    CASE_UINT
    case ft_mydate:
    case ft_time:
    case ft_date:
    case ft_datetime:
    case ft_logical:
    case ft_currency:
        return readValue64();
    case ft_bit:
        return changeEndian(readValue64(), m_fd->len);
    case ft_myyear:
    {
         __int64 v = readValue64();
        if (v) v += 1900;
        return v;
    }
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
        __int64 v = readValue64();
        bool bigendian = !m_fd->isLegacyTimeFormat();
        if (m_fd->type ==  ft_mytime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return getInternalValue<maTime>(m_fd->decimals, bigendian, v);
            else
                return getInternalValue<myTime>(m_fd->decimals, bigendian, v);
        }
        else if (m_fd->type ==  ft_mydatetime)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return getInternalValue<maDateTime>(m_fd->decimals, bigendian, v);
            else
                return getInternalValue<myDateTime>(m_fd->decimals, bigendian, v);
        }
        else if (m_fd->type ==  ft_mytimestamp)
        {
            if (m_fd->m_options & FIELD_OPTION_MARIADB)
                return getInternalValue<maTimeStamp>(m_fd->decimals, bigendian, v);
            else
                return getInternalValue<myTimeStamp>(m_fd->decimals, bigendian, v);
        }
        assert(0);
    }
    CASE_FLOAT
    case ft_timestamp:
        return (__int64)readValueDbl();
    CASE_NUMERIC
        return (__int64)readValueNumeric();
    CASE_DECIMAL
        return (__int64)readValueDecimal();
    case ft_mydecimal:
    {
        myDecimal d((uchar_td)m_fd->digits, m_fd->decimals);
        d.read((char*)m_ptr + m_fd->pos);
        return d.i64();
    }
#ifdef LINUX
    CASE_TEXTW
#endif
    CASE_TEXTA
        return _atoi64(readValueStrA());
#ifndef LINUX
    CASE_TEXTW
        return _wtoi64(readValueStrW());
#endif
    default: // ft_lvar ft_mygeometry ft_myjson
         break;
    }
    return 0;
}

/* offset is writen at data that is first address of data
 *  text is not converted to unicode form stored charset.
 */
void* field::getFVbin(uint_td& size) const
{
    if (!m_ptr) return 0;

    char* p = (char*)m_ptr + m_fd->pos;
    switch (m_fd->type)
    {
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_lstring:
    case ft_lvar:
    {
        int sizeByte = m_fd->varLenBytes();
        size = 0;
        memcpy(&size, p, sizeByte);
        return (void*)(p + sizeByte);
    }
    case ft_myjson:
    case ft_mygeometry:
    case ft_myblob:
    case ft_mytext:
    {
        int sizeByte = m_fd->len - 8;
        size = 0;
        memcpy(&size, p, sizeByte);
        if (size)
        {
            char** ptr = (char**)(p + sizeByte);
            return (void*)*ptr;
        }
        return NULL;
    }
    default:
        size = m_fd->len;
        return (void*)(p);
    }
}

void* field::ptr() const
{
    return m_ptr + m_fd->pos;
}

void* field::nullPtr() const
{
    return m_ptr - m_fd->nullbytes();
}

void field::nullPtrCache() const
{
    if (m_nullbit == 0 && m_fd->isNullable() && m_fd->nullbytes())
    {
        m_cachedNullPtr =  (unsigned char*)nullPtr();
        m_cachedNullPtr += (m_fd->nullbit() / 8);
        m_nullbit = (1L << (m_fd->nullbit() % 8));
    }
}

bool field::isNull() const
{
    nullPtrCache();
    if (m_nullbit)
        return (*m_cachedNullPtr & m_nullbit) != 0;
    return false;
}

void field::setNull(bool v)
{
    nullPtrCache();
    if (m_nullbit && m_cachedNullPtr != &m_nullSign)
    {
         if (v)
            (*m_cachedNullPtr) |= (unsigned char)m_nullbit;
        else
            (*m_cachedNullPtr) &= (unsigned char)~m_nullbit;
        m_fd->enableFlags.bitE = true;
    }
}

int field::nullComp(char log) const
{
    bool rnull = (log == eIsNull) || (log == eIsNotNull);
    bool lnull = isNull();
    return ::nullComp(lnull, rnull, log);
} 

int field::nullComp(const field& r, char log) const
{
    if ((log == eIsNull) || (log == eIsNotNull))
        return nullComp(log);

    bool lnull = isNull();
    bool rnull = r.isNull();
    if (lnull || rnull)
    {
        if (lnull > rnull) return -1;
        if (lnull < rnull) return 1;
        return 0;
    }
    return 2;
}

int field::comp(const field& r, char log) const
{
    int ret = nullComp(r, log & 0xf);
    if (ret < 2)
        return ret;
    comp1Func f = getCompFunc(m_fd->type, m_fd->len, log | CMPLOGICAL_FIELD,
                         m_fd->varLenBytes() + m_fd->blobLenBytes());
    return f((const char*)ptr(), (const char*)r.ptr(), r.len());
}

bool field::isCompPartAndMakeValue()
{
    bool ret = false;
    if (m_fd->isStringType())
    {
        m_fd->setPadCharSettings(false, true);
        _TCHAR* p = (_TCHAR*)getFVstr();
        if (p)
        {
            size_t n = _tcslen(p);
            if (n)
            {
                if (p[n - 1] == _T('*'))
                {
                    p[n - 1] = 0x00;
                    if (m_fd->type == ft_mychar)
                        m_fd->type = ft_string;
                    else if (m_fd->type == ft_mywchar)
                        m_fd->type = ft_wstring;
                    m_fd->setPadCharSettings(false, true);
                    setFV(p);

                    ret = true;
                }
            }
        }
        else
            setFV(_T(""));
    }
    return ret;
}

void field::offsetBlobPtr(size_t offset)
{
    int size = m_fd->blobLenBytes();
    char** p = (char**)((char*)m_ptr + size);
    *p += offset; 
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
