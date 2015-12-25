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

//------------------------------------------------------------------------------
//       class fieldShare
//------------------------------------------------------------------------------
struct Imple
{
    stringConverter* cv;
    bzs::rtl::stringBuffer strBufs;
    std::vector<boost::shared_array<char> > blobs;

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

void fieldShare::blobPushBack(char* p)
{
    m_imple->blobs.push_back(boost::shared_array<char>(p));
}

void fieldShare::blobClear()
{
    m_imple->blobs.clear();
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
    aliasing(pp);
    m_imple->map[pp->name()] = index;
}

void fielddefs::remove(int index)
{
    m_imple->map.erase(m_imple->fields[index].name());
    m_imple->fields.erase(m_imple->fields.begin() + index);
    boost::unordered_map<std::_tstring, int>::iterator it =
        m_imple->map.begin();
    while (it != m_imple->map.end())
    {
        if ((*it).second > index)
            (*it).second--;
        ++it;
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
    for (int i = 0; i < def->fieldCount; ++i)
    {
        const fielddef* fd = &def->fieldDefs[i];
        push_back(fd);
        m_imple->fields[m_imple->fields.size() - 1].setPadCharDefaultSettings();
    }
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
    case ft_bit:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_currency:
        switch (m_fd->len)
        {
        case 1:
            ret = *((unsigned char*)ptr);
            break;
        case 2:
            ret = *((unsigned short*)ptr);
            break;
        case 4:
            ret = *((unsigned int*)ptr);
            break;
        case 8:
            ret = *((__int64*)ptr);
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
    case ft_bit:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_currency:
    {
        switch (m_fd->len)
        {
        case 1:
            *((char*)ptr) = (char)value;
            break;
        case 2:
            *((unsigned short*)ptr) = (unsigned short)value;
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
        char* tmp = blobStore<char>(p, data, *m_fd, m_fds->cv());
        const_cast<fielddefs*>(m_fds)->blobPushBack(tmp);
        break;
    }
    default:
        assert(0);
    }
}

const char* field::readValueStrA() const
{
    char* data = (char*)m_ptr + m_fd->pos;
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
void field::storeValueStrW(const wchar_t* data) 
{
    char* p = (char*)m_ptr + m_fd->pos;
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
        char* tmp = blobStore<WCHAR>(p, data, *m_fd, m_fds->cv());
        const_cast<fielddefs*>(m_fds)->blobPushBack(tmp);
        return;
    }
    default:
        assert(0);
    }
}

const wchar_t* field::readValueStrW() const
{
    char* data = (char*)m_ptr + m_fd->pos;
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
#endif

void field::storeValueNumeric(double data)
{ // Double  -> Numeric

    char buf[30] = "%+0";
    char dummy[30];
    int point;
    int n;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    bool sign = false;
    char* t;

    point = m_fd->len + 1;

    _ltoa_s(point, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_fd->decimals, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf(dummy, buf, data);
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
    char buf[20] = { 0x00 };
    char dummy[20];

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
    char buf[30] = "%+0";
    char dummy[30];
    int point;
    bool sign = false;
    unsigned char n;
    int i, k;
    int strl;
    bool offset = false;
    
    point = (m_fd->len) * 2;
    _ltoa_s(point, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, ".");
    _ltoa_s(m_fd->decimals, dummy, 30, 10);
    strcat(buf, dummy);
    strcat(buf, "lf");
    sprintf(dummy, buf, data);
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
    case ft_bit: \
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

inline __int64 logical_str_to_64(bool logicalToString, const wchar_t* data)
{
    if (logicalToString)
    {
        wchar_t tmp[5];
        wcsncpy(tmp, data, 5);
        if (wcscmp(_wcsupr(tmp), L"YES") == 0)
            return 1;
        else
            return 0;
    }
    else
        return _wtol(data);
}

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
    CASE_INT
    CASE_UINT
        value = _atoi64(data);
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
    default: // ft_lvar
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
    CASE_INT
    CASE_UINT
        value = _wtoi64(data);
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
    default: // ft_lvar
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
        //Convert big endian
        if (!m_fd->isLegacyTimeFormat())
        {
            if (m_fd->type == ft_mydatetime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    data = getBigEndianValue<maDateTime>(m_fd->decimals, data);
                else
                    data = getBigEndianValue<myDateTime>(m_fd->decimals, data);
            }
            else if (m_fd->type == ft_mytimestamp)
                data = getBigEndianValue<myTimeStamp>(m_fd->decimals, data);
            else if(m_fd->type == ft_mytime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    data = getBigEndianValue<maTime>(m_fd->decimals, data);
                else
                    data = getBigEndianValue<myTime>(m_fd->decimals, data);
            }
        }
        //fall through
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
    CASE_FLOAT
        storeValueDbl((double)data);
        break;
    CASE_NUMERIC
        storeValueNumeric((double)data);
        break;
    CASE_DECIMAL
        storeValueDecimal((double)data);
        break;
    CASE_TEXTA
    {
        char buf[50];
         _i64toa_s(data, buf, 50, 10);
        storeValueStrA(buf);
        break;
    }
    CASE_TEXTW
    {
        wchar_t buf[50];
         _i64tow_s(data, buf, 50, 10);
        storeValueStrW(buf);
        break;
    }
    default://lvar
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
        //Convert big endian
        if (!m_fd->isLegacyTimeFormat())
        {
            if (m_fd->type == ft_mydatetime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    i64 = getBigEndianValue<maDateTime>(m_fd->decimals, i64);
                else
                    i64 = getBigEndianValue<myDateTime>(m_fd->decimals, i64);
            }
            else if (m_fd->type == ft_mytimestamp)
                i64 = getBigEndianValue<myTimeStamp>(m_fd->decimals, i64);
            else if(m_fd->type == ft_mytime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    i64 = getBigEndianValue<maTime>(m_fd->decimals, i64);
                else
                    i64 = getBigEndianValue<myTime>(m_fd->decimals, i64);
            }
            else if(m_fd->type == ft_currency)
                i64 = (__int64)(data * 10000 + 0.5);
        }
        //fall through
    case ft_date:
    case ft_time:
    case ft_datetime: 
    case ft_mydate:
    case ft_logical:
    CASE_INT
    CASE_UINT
        storeValue64(i64);
        break;
    CASE_NUMERIC
        storeValueNumeric(data);
        break;
    CASE_DECIMAL
        storeValueDecimal(data);
        break;
    CASE_TEXTA
    {
        char buf[100];
        sprintf(buf, "%f", data);
        storeValueStrA(buf);
        break;
    }
    CASE_TEXTW
    {
        wchar_t buf[100];
        swprintf_s(buf, 50, L"%f", data);
        storeValueStrW(buf);
        break;
    }
    default:// lvar
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
    {
        int sizeByte = m_fd->varLenBytes();
        size = std::min<uint_td>((uint_td)(m_fd->len - sizeByte), size);
        memset(p, 0, m_fd->len);
        memcpy(p, &size, sizeByte);
        memcpy(p + sizeByte, data, size);
        break;
    }
    case ft_myblob:
    case ft_mytext:
    {
        int sizeByte = m_fd->len - 8;
        memset(p, 0, m_fd->len);
        memcpy(p, &size, sizeByte);
        memcpy(p + sizeByte, &data, sizeof(char*));
        break;
    }
    case ft_lvar:
    {
        int sizeByte = 2;
        size = std::min<uint_td>((uint_td)(m_fd->len - sizeByte), size);
        memset(p, 0, m_fd->len);
        memcpy(p, &size, sizeByte);
        memcpy(p + sizeByte, data, size);
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
template <class T> 
T* trimZero(T* p, size_t len)
{
    while (1)
    {
        if (p[len] == '0')
            p[len] = 0x00;
        else if (p[len] == '.')
        {
            p[len] = 0x00;
            break;
        }
        else
            break;
        len--;
    }
    return p;
}

const char* field::getFVAstr() const
{
    if (!m_ptr) return "";
  
    switch (m_fd->type)
    {
    CASE_TEXT
        return readValueStrA();
    }

    char* p = m_fds->strBufs()->getPtrA(max(m_fd->len * 2, 50));
    switch (m_fd->type)
    {
    CASE_INT
        _i64toa_s(readValue64(), p, 50, 10);
        return p;
    CASE_UINT
        sprintf_s(p, 50, "%llu", (unsigned __int64)readValue64());
        return p;
    case ft_logical:
    {
        int v = (int)readValue64();
        if (m_fds->logicalToString)
            return v ? "Yes": "No";
        else
            _ltoa_s(v, p, 50, 10);
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
        return date_time_str<myDate, char>(m_fd->len, true, readValue64(), p);
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
        else
            return date_time_str<myTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
        else
            return date_time_str<myDateTime, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
    case ft_mytimestamp:
        return date_time_str<myTimeStamp, char>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
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
    sprintf(p, "%.*lf", m_fd->decimals, v);
    if (m_fd->decimals == 0)
        trimZero(p, strlen(p) - 1);
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

    wchar_t* p = (wchar_t*)m_fds->strBufs()->getPtrW(max(m_fd->len * 2, 50));
    switch (m_fd->type)
    {
    CASE_INT
        _i64tow_s(readValue64(), p, 50, 10);
        return p;
    CASE_UINT
        swprintf_s(p, 50, L"%llu", (unsigned __int64)readValue64());
        return p;
    case ft_logical:
    {
        int v = (int)readValue64();
        if (m_fds->logicalToString)
            return v ? L"Yes": L"No";
        else
            _ltow_s(v, p, 50, 10);
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
        return date_time_str<myDate, wchar_t>(m_fd->decimals, true, readValue64(), p);
    case ft_mytime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
        else
            return date_time_str<myTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
    case ft_mydatetime:
        if (m_fd->m_options & FIELD_OPTION_MARIADB)
            return date_time_str<maDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
        else
            return date_time_str<myDateTime, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
    case ft_mytimestamp:
        return date_time_str<myTimeStamp, wchar_t>(m_fd->decimals, !m_fd->isLegacyTimeFormat(), readValue64(), p);
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
    swprintf_s(p, 50, L"%.0*lf", v, m_fd->decimals);
    if (m_fd->decimals == 0)
        trimZero(p, wcslen(p) - 1);
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
    CASE_INT
    case ft_mydate:
    case ft_time:
    case ft_date:
    case ft_datetime:
    case ft_logical:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
        __int64 v = readValue64();
        if (!m_fd->isLegacyTimeFormat())
        {
            if (m_fd->type ==  ft_mytime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    return (double)getLittleEndianValue<maTime>(m_fd->decimals, v);
                else
                    return (double)getLittleEndianValue<myTime>(m_fd->decimals, v);
            }
            else if (m_fd->type ==  ft_mydatetime)
            {
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    return (double)getLittleEndianValue<maDateTime>(m_fd->decimals, v);
                else
                    return (double)getLittleEndianValue<myDateTime>(m_fd->decimals, v);
            }
            else if (m_fd->type ==  ft_mytimestamp)
                return (double)getLittleEndianValue<myTimeStamp>(m_fd->decimals, v);
        }
        return (double)v;
    }
    CASE_UINT
        return (double)(unsigned __int64)readValue64();
    case ft_currency:
        return ((double)readValue64() / (double)10000);
    CASE_TEXTA
        return atof(readValueStrA());
    CASE_TEXTW
        return _wtof(readValueStrW());
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
        return readValue64();
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
    {
        __int64 v = readValue64();
        if (!m_fd->isLegacyTimeFormat())
        {
            switch (m_fd->type)
            {
            case ft_mytime:
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    return getLittleEndianValue<maTime>(m_fd->decimals, v);
                else
                    return getLittleEndianValue<myTime>(m_fd->decimals, v);
            case ft_mydatetime:
                if (m_fd->m_options & FIELD_OPTION_MARIADB)
                    return getLittleEndianValue<maDateTime>(m_fd->decimals, v);
                else
                    return getLittleEndianValue<myDateTime>(m_fd->decimals, v);
            case ft_mytimestamp:
                return getLittleEndianValue<myTimeStamp>(m_fd->decimals, v);
            }
        }
        return v;
    }
    CASE_FLOAT
    case ft_timestamp:
        return (__int64)readValueDbl();
    CASE_NUMERIC
        return (__int64)readValueNumeric();
    CASE_DECIMAL
        return (__int64)readValueDecimal();
    CASE_TEXTA
        return _atoi64(readValueStrA());
    CASE_TEXTW
        return _wtoi64(readValueStrW());
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
    {
        int sizeByte = m_fd->varLenBytes();
        size = 0;
        memcpy(&size, p, sizeByte);
        return (void*)(p + sizeByte);
    }
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
    case ft_lvar:
    {
        int sizeByte = 2;
        size = 0;
        memcpy(&size, p, sizeByte);
        return (void*)(p + sizeByte);
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
    if (m_nullbit)
    {
         if (v)
            (*m_cachedNullPtr) |= (unsigned char)m_nullbit;
        else
            (*m_cachedNullPtr) &= (unsigned char)~m_nullbit;
        m_fd->enableFlags.bitE = true;
    }
}

template <class T>
inline int compNumber(const field& l, const field& r, char logType)
{
    return compare<T>((const char*)l.ptr(), (const char*)r.ptr());
}

template <class T>
inline int compBitAnd(const field& l, const field& r, char logType)
{
    return bitMask<T>((const char*)l.ptr(), (const char*)r.ptr());
}

inline int compBitAnd24(const field& l, const field& r, char logType)
{

    int lv = ((*((int*)(const char*)l.ptr()) & 0xFFFFFF) << 8) / 0x100;
    int rv = ((*((int*)(const char*)r.ptr()) & 0xFFFFFF) << 8) / 0x100;
    return bitMask<int>((const char*)&lv, (const char*)&rv);
}

inline int compNumber24(const field& l, const field& r, char logType)
{
    return compareInt24((const char*)l.ptr(), (const char*)r.ptr());
}

inline int compNumberU24(const field& l, const field& r, char logType)
{
    return compareUint24((const char*)l.ptr(), (const char*)r.ptr());
}

inline int compMem(const field& l, const field& r, char logType)
{
    return memcmp((const char*)l.ptr(), (const char*)r.ptr(), r.len());
}

inline int compString(const field& l, const field& r, char logType)
{
    return strncmp((const char*)l.ptr(), (const char*)r.ptr(), r.len());
}

inline int compiString(const field& l, const field& r, char logType)
{
    return _strnicmp((const char*)l.ptr(), (const char*)r.ptr(), r.len());
}

int compWString(const field& l, const field& r, char logType)
{
    return wcsncmp16((char16_t*)l.ptr(), (char16_t*)r.ptr(), r.len());
}

int compiWString(const field& l, const field& r, char logType)
{
    return wcsnicmp16((char16_t*)l.ptr(), (char16_t*)r.ptr(), r.len());
}

template <class T>
inline int compVarString(const field& l, const field& r, char logType)
{
    return compareVartype<T>((const char*)l.ptr(), (const char*)r.ptr(),
                             l.type() == ft_myvarbinary, logType);
}

template <class T>
inline int compWVarString(const field& l, const field& r, char logType)
{
    return compareWvartype<T>((const char*)l.ptr(), (const char*)r.ptr(),
                              l.type() == ft_mywvarbinary, logType);
}

inline int compBlob(const field& l, const field& r, char logType)
{
    return compareBlobType((const char*)l.ptr(), (const char*)r.ptr(),
                           l.type() == ft_myblob, logType, l.blobLenBytes());
}

compFieldFunc field::getCompFunc(char logType) const
{
    switch (m_fd->type)
    {
    case ft_integer:
    case ft_autoinc:
    case ft_currency:
    {
        if (logType & eBitAnd)
        {

            switch (m_fd->len)
            {
            case 1:
                return &compBitAnd<char>;
            case 2:
                return &compBitAnd<short>;
            case 3:
                return &compBitAnd24;
            case 4:
                return &compBitAnd<int>;
            case 8:
                return &compBitAnd<__int64>;
            }
        }else
        {
            switch (m_fd->len)
            {
            case 1:
                return &compNumber<char>;
            case 2:
                return &compNumber<short>;
            case 3:
                return &compNumber24;
            case 4:
                return &compNumber<int>;
            case 8:
                return &compNumber<__int64>;
            }
        }
    }
    case ft_mychar:
    case ft_string:
        if (logType & CMPLOGICAL_CASEINSENSITIVE)
            return &compiString;
        return &compMem;
    case ft_zstring:
    case ft_note:
        if (logType & CMPLOGICAL_CASEINSENSITIVE)
            return &compiString;
        return &compString;
    case ft_logical:
    case ft_uinteger:
    case ft_autoIncUnsigned:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_mydate:
    {
        if (logType & eBitAnd)
        {
            switch (m_fd->len)
            {
            case 1:
                return &compBitAnd<unsigned char>;
            case 2:
                return &compBitAnd<unsigned short>;
            case 3:
                return &compBitAnd24;
            case 4:
                return &compBitAnd<unsigned int>;
            case 8:
                return &compBitAnd<unsigned __int64>;
            }
        }else
        {
            switch (m_fd->len)
            {
            case 1:
                return &compNumber<unsigned char>;
            case 2:
                return &compNumber<unsigned short>;
            case 3:
                return &compNumberU24;
            case 4:
                return &compNumber<unsigned int>;
            case 8:
                return &compNumber<unsigned __int64>;
            }
        }
    }
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
        if (m_fd->isLegacyTimeFormat())
        {
            switch (m_fd->len)
            {
            case 3:
                return &compNumberU24;
            case 4:
                return &compNumber<unsigned int>;
            case 8:
                return &compNumber<unsigned __int64>;
            default:
                assert(0);
                break;
            }
        }else
            return &compMem;
    case ft_float:
        switch (m_fd->len)
        {
        case 4:
            return &compNumber<float>;
        case 8:
            return &compNumber<double>;
        }
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
        if (logType & CMPLOGICAL_CASEINSENSITIVE)
            return &compiWString;
        if ((m_fd->type == ft_wstring) || (m_fd->type == ft_mywchar))
            return &compMem;
        return &compWString;
    case ft_lstring:
    case ft_myvarchar:
    case ft_myvarbinary:
        if (m_fd->varLenBytes() == 1)
            return &compVarString<unsigned char>;
        return &compVarString<unsigned short>;
    case ft_mywvarchar:
    case ft_mywvarbinary:
        if (m_fd->varLenBytes() == 1)
            return &compWVarString<unsigned char>;
        return &compWVarString<unsigned short>;
    case ft_mytext:
    case ft_myblob:
        return &compBlob;
    }
    return NULL;
}

int field::nullComp(char log) const
{
    bool rnull = (log == eIsNull) || (log == eIsNotNull);
    bool lnull = isNull();
            
    if (lnull || rnull)
    {
        if (lnull && (log == eIsNull))
            return 0;
        else if (lnull && (log == eIsNotNull))
            return -1;
        else if (log == eIsNull)
            return 1;
        return 0; //(log == (char)eIsNotNull)
    }
    return 2;
}

int field::nullComp(const field& r, char log) const
{
    bool rnull = (log == eIsNull) || (log == eIsNotNull);
    if (!rnull && r.isNull()) log = eIsNull;
    return nullComp(log);
}

int field::comp(const field& r, char log) const
{
    int ret = nullComp(r, log & 0xf);
    if (ret < 2)
        return ret;
    compFieldFunc f = getCompFunc(log);
    return f(*this, r, log);
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
