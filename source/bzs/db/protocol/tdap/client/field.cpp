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
#include <bzs/db/protocol/tdap/myDateTime.cpp>
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

    infoImple() : aliasMap(NULL) {}
    infoImple(const infoImple& r)
        : fields(r.fields), map(r.map), aliasMap(r.aliasMap){};
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
    {
        *m_imple = *r.m_imple;
    }
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

void fielddefs::addAllFileds(tabledef* def)
{
    m_imple->fields.clear();
    for (int i = 0; i < def->fieldCount; ++i)
    {
        fielddef* fd = &def->fieldDefs[i];
        fd->setPadCharDefaultSettings();
        push_back(fd);
    }
}

void fielddefs::push_back(const fielddef* p, bool rePosition)
{
    m_imple->fields.push_back(*p);
    int index = (int)m_imple->fields.size() - 1;
    fielddef* pp = &m_imple->fields[index];

    // convert field name of table charset to recordset schema charset.
    _TCHAR tmp[FIELD_NAME_SIZE * 3];
    pp->setName(p->name(tmp));

    if (rePosition)
    {
        if (index == 0)
            pp->pos = 0;
        else
        {
            fielddef* bf = &m_imple->fields[index - 1];
            pp->pos = bf->pos + bf->len;
        }
    }
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
    return fd.pos + fd.len;
}

void fielddefs::copyFrom(const table* tb)
{
    int n = tb->getCurProcFieldCount();
    m_imple->fields.reserve(n + size());
    const tabledef* def = tb->tableDef();
    int pos = 0;
    for (int i = 0; i < n; ++i)
    {
        fielddef fd = def->fieldDefs[tb->getCurProcFieldIndex(i)];
        fd.pos = pos;
        push_back(&fd);
        pos += fd.len;
    }

    // Defalut field charset
    cv()->setCodePage(mysql::codePage(def->charsetIndex));
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

// const field fieldDummy(NULL, initDummy(), NULL);

inline __int64 getValue64(const fielddef& fd, const uchar_td* ptr)
{
    __int64 ret = 0;
    if (ptr)
    {
        switch (fd.type)
        {
        case ft_integer:
        case ft_autoinc:
            switch (fd.len)
            {
            case 1:
                ret = *((char*)(ptr + fd.pos));
                break;
            case 2:
                ret = *((short*)(ptr + fd.pos));
                break;
            case 4:
                ret = *((int*)(ptr + fd.pos));
                break;
            case 8:
                ret = *((__int64*)(ptr + fd.pos));
                break;
            }
        case ft_autoIncUnsigned:
        case ft_uinteger:
        case ft_logical:
        case ft_bit:
        case ft_date:
        case ft_time:
        case ft_timestamp:
        case ft_mydate:
        case ft_mytime:
        case ft_mydatetime:
        case ft_mytimestamp:
            switch (fd.len)
            {
            case 1:
                ret = *((unsigned char*)(ptr + fd.pos));
                break;
            case 2:
                ret = *((unsigned short*)(ptr + fd.pos));
                break;
            case 4:
                ret = *((unsigned int*)(ptr + fd.pos));
                break;
            case 3:
            case 5:
            case 6:
            case 7:
                memcpy(&ret, ptr + fd.pos, fd.len);
                break;
            case 8:
                ret = *((__int64*)(ptr + fd.pos));
                break;
            }
            break;
        case ft_currency:
            ret = (*((__int64*)((char*)ptr + fd.pos)) / 10000);
            break;
        case ft_bfloat:
        case ft_float:
            switch (fd.len)
            {
            case 4:
                ret = (__int64)*((float*)((char*)ptr + fd.pos));
                break;
            case 8:
                ret = (__int64)*((double*)((char*)ptr + fd.pos));
            case 10: // long double
                ret = (__int64)*((long double*)((char*)ptr + fd.pos));
                break;
            }
            break;
        }
    }
    return ret;
}

inline void setValue(const fielddef& fd, uchar_td* ptr, __int64 value)
{
    if (!ptr)
        return;
    switch (fd.type)
    {
    case ft_integer:
    case ft_autoinc:
    {
        switch (fd.len)
        {
        case 1:
            *((char*)(ptr + fd.pos)) = (char)value;
            break;
        case 2:
            *((short*)(ptr + fd.pos)) = (short)value;
            break;
        case 4:
            *((int*)(ptr + fd.pos)) = (int)value;
            break;
        case 8:
            *((__int64*)(ptr + fd.pos)) = value;
            break;
        }
    }
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_bit:
    case ft_currency:
    case ft_date:
    case ft_time:
    case ft_timestamp:
    case ft_mytime:
    case ft_mydate:
    case ft_mydatetime:
    case ft_mytimestamp:
        memcpy(ptr + fd.pos, &value, fd.len);
        break;
    }
}

void* field::ptr() const
{
    return m_ptr + m_fd->pos;
}

void field::setFVA(const char* data)
{
    if (!m_ptr)
        return;
    __int64 value;
    double fltValue;

    char* p = (char*)m_ptr + m_fd->pos;
    if (data == NULL)
    {
        memset(p, 0, m_fd->len);
        return;
    }

    switch (m_fd->type)
    {
    case ft_string:
        if (m_fd->usePadChar())
            return store<stringStore, char, char>(p, data, *m_fd, m_fds->cv());
        return store<binaryStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_note:
    case ft_zstring:
        return store<zstringStore, char, char>(p, data, *m_fd, m_fds->cv());
    case ft_wzstring:
        return store<wzstringStore, WCHAR, char>(p, data, *m_fd, m_fds->cv());
    case ft_wstring:
        if (m_fd->usePadChar())
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
        return store<myWvarBinaryStore, WCHAR, char>(p, data, *m_fd,
                                                     m_fds->cv());
    case ft_myblob:
    case ft_mytext:
    {
        char* tmp = blobStore<char>(p, data, *m_fd, m_fds->cv());
        const_cast<fielddefs*>(m_fds)->blobPushBack(tmp);
        return;
    }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:
    case ft_currency: // currecy
    case ft_float: // float double
        fltValue = atof(data);
        setFV(fltValue);
        return;
    case ft_lvar: // Lvar
        return;

    case ft_date: // date mm/dd/yy
        value = /*StrToBtrDate*/ atobtrd((const char*)data).i;
        break;
    case ft_time: // time hh:nn:ss
        value = /*StrToBtrTime*/ atobtrt((const char*)data).i;
        break;
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_integer:
    case ft_autoinc:
    case ft_bit:
        value = _atoi64(data);
        break;
    case ft_logical:
        if (m_fds->logicalToString)
        {
            char tmp[5];
            strncpy(tmp, data, 5);
            if (strcmp(_strupr(tmp), "YES") == 0)
                value = 1;
            else
                value = 0;
        }
        else
            value = atol(data);
        break;
    case ft_timestamp:
    case ft_mytimestamp:
        value = 0;
        break;
    case ft_mydate:
    {
        myDate d;
        d = data;
        value = d.getValue();
        break;
    }
    case ft_mytime:
    {
        myTime t(m_fd->len);
        t = data;
        value = t.getValue();
        break;
    }

    case ft_mydatetime:
    {
        myDateTime t(m_fd->len);
        t = data;
        value = t.getValue();
        break;
    }
    default:
        return;
    }
    setValue(*m_fd, (uchar_td*)m_ptr, value);
}

#ifdef _WIN32

void field::setFVW(const wchar_t* data)
{
    if (!m_ptr)
        return;
    int value;
    double fltValue;

    char* p = (char*)m_ptr + m_fd->pos;
    if (data == NULL)
    {
        memset(p, 0, m_fd->len);
        return;
    }

    switch (m_fd->type)
    {
    case ft_string:
        if (m_fd->usePadChar())
            return store<stringStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
        return store<binaryStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_note:
    case ft_zstring:
        return store<zstringStore, char, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_wzstring:
        return store<wzstringStore, WCHAR, WCHAR>(p, data, *m_fd, m_fds->cv());
    case ft_wstring:
        if (m_fd->usePadChar())
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

    case ft_date: // date mm/dd/yy
        value = /*StrToBtrDate*/ atobtrd(data).i;
        setFV(value);
        break;
    case ft_time: // time hh:nn:ss
        value = /*StrToBtrTime*/ atobtrt(data).i;
        setFV(value);
        return;

    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_integer:
    case ft_autoinc:
    case ft_bit:
    {
        __int64 v = _wtoi64(data);
        setFV(v);
        break;
    }
    case ft_logical:
        if (m_fds->logicalToString)
        {
            wchar_t tmp[5];
            wcsncpy(tmp, data, 5);

            if (wcscmp(_wcsupr(tmp), L"YES") == 0)
                value = 1;
            else
                value = 0;
        }
        else
            value = _wtol(data);
        setFV(value);
        break;

    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:
    case ft_currency:
    case ft_float:
        fltValue = _wtof(data);
        setFV(fltValue);
        break;
    case ft_timestamp:
    {
        __int64 v = 0;
        setFV(v);
        return;
    }
    case ft_mydate:
    {
        myDate d;
        d = data;
        setValue(*m_fd, (uchar_td*)m_ptr, d.getValue());
        return;
    }
    case ft_mytime:
    {
        myTime t(m_fd->len);
        t = data;
        setValue(*m_fd, (uchar_td*)m_ptr, t.getValue());
        return;
    }
    case ft_mydatetime:
    {
        myDateTime t(m_fd->len);
        t = data;
        setFV(t.getValue());
        return;
    }
    case ft_mytimestamp:
    case ft_lvar:
        break;
    }
}

#endif //_WIN32

void field::setFV(unsigned char data)
{

    int value = (int)data;
    setFV(value);
}

void field::setFV(int data)
{
    if (!m_ptr)
        return;
    char buf[50];
    double d;
    int v = data;
    switch (m_fd->type)
    {
    case ft_mydate:
    {
        myDate myd;
        myd.setValue(data, m_fds->myDateTimeValueByBtrv);
        setValue(*m_fd, (uchar_td*)m_ptr, myd.getValue());
        break;
    }
    case ft_mytime:
    {
        myTime myt(m_fd->len);
        myt.setValue(data, m_fds->myDateTimeValueByBtrv);
        setValue(*m_fd, (uchar_td*)m_ptr, myt.getValue());
        break;
    }
    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_bit:
    case ft_mydatetime:
        switch (m_fd->len)
        {
        case 1:
            *((char*)((char*)m_ptr + m_fd->pos)) = (char)v;
            break;
        case 2:
            *((short*)((char*)m_ptr + m_fd->pos)) = (short)v;
            break;
        case 3:
            memcpy((char*)m_ptr + m_fd->pos, &v, 3);
            break;
        case 4:
            *((int*)((char*)m_ptr + m_fd->pos)) = v;
            break;
        case 8:
            *((__int64*)((char*)m_ptr + m_fd->pos)) = v;
            break;
        }
        break;

    case ft_timestamp:
    {
        __int64 v = 0;
        setFV(v);
        return;
    }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_bfloat:
    case ft_numericsts:
    case ft_numericsa:

    case ft_currency:
    case ft_float:
        d = (double)data;
        setFV(d);
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        if (data == 0)
            setFVA("");
        else
        {
            _ltoa_s(data, buf, 50, 10);
            setFVA(buf);
        }
        break;
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
    {
        if (data == 0)
            setFV(_T(""));
        else
        {
            _TCHAR buf[50];
            _ltot_s(data, buf, 50, 10);
            setFV(buf);
        }
        break;
    }
    case ft_lvar:
        break;
    }
}

void field::setFV(double data)
{
    if (!m_ptr)
        return;
    char buf[50];
    __int64 i64;
    switch (m_fd->type)
    {
    case ft_currency: // currency
        i64 = (__int64)(data * 10000 + 0.5);
        setFV(i64);
        break;
    case ft_bfloat: // bfloat
    case ft_float:
        switch (m_fd->len)
        {
        case 4:
            *((float*)((char*)m_ptr + m_fd->pos)) = (float)data;
            break;
        case 8:
            *((double*)((char*)m_ptr + m_fd->pos)) = data;
            break;
        default:
            break;
        }
        break;
    case ft_decimal:
    case ft_money:
        setFVDecimal(data);
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa:
        setFVNumeric(data);
        break;

    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_timestamp:
    case ft_bit:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
        i64 = (__int64)data;
        setFV(i64);
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        if (data == 0)
            setFVA("");
        else
        {
            sprintf(buf, "%f", data);
            setFVA(buf);
            break;
        }
    case ft_lvar:
        break;
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
    case ft_wstring:
    case ft_wzstring:
    {
        if (data == 0)
            setFV(_T(""));
        else
        {
            _TCHAR buf[50];
            _stprintf_s(buf, 50, _T("%f"), data);
            setFV(buf);
        }
        break;
    }
    }
}

void field::setFV(short data)
{
    int value = (int)data;
    setFV(value);
}

void field::setFV(float data)
{
    double value = (double)data;
    setFV(value);
}

short field::getFVsht() const
{
    return (short)getFVlng();
}

int field::getFVlng() const
{
    if (!m_ptr)
        return 0;
    int ret = 0;
    switch (m_fd->type)
    {
    case ft_integer:
    case ft_autoinc:
        switch (m_fd->len)
        {
        case 1:
            ret = *(((char*)m_ptr + m_fd->pos));
            break;
        case 2:
            ret = *((short*)((char*)m_ptr + m_fd->pos));
            break;
        case 3:
            memcpy(&ret, (char*)m_ptr + m_fd->pos, 3);
            ret = ((ret & 0xFFFFFF) << 8) / 0x100;
            break;
        case 8:
        case 4:
            ret = *((int*)((char*)m_ptr + m_fd->pos));
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
    case ft_mydatetime:
    case ft_mytimestamp:
        switch (m_fd->len)
        {
        case 1:
            ret = *((unsigned char*)((char*)m_ptr + m_fd->pos));
            break;
        case 2:
            ret = *((unsigned short*)((char*)m_ptr + m_fd->pos));
            break;
        case 3:
            memcpy(&ret, (char*)m_ptr + m_fd->pos, 3);
            break;
        case 8:
        case 4:
            ret = *((unsigned int*)((char*)m_ptr + m_fd->pos));
            break;
        }
        break;
    case ft_mydate:
    {
        myDate myd;
        myd.setValue((int)getValue64(*m_fd, (const uchar_td*)m_ptr));
        ret = myd.getValue(m_fds->myDateTimeValueByBtrv);
        break;
    }
    case ft_mytime:
    {
        myTime myt(m_fd->len);
        myt.setValue((int)getValue64(*m_fd, (const uchar_td*)m_ptr));
        ret = (int)myt.getValue(m_fds->myDateTimeValueByBtrv);
        break;
        ;
    }
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        ret = atol(getFVAstr());
        break;
    case ft_wstring:
    case ft_wzstring:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
        ret = _ttol(getFVstr());
        break;
    case ft_currency:
        ret = (long)(*((__int64*)((char*)m_ptr + m_fd->pos)) / 10000);
        break;
    case ft_bfloat:
    case ft_float:
        ret = (long)getFVdbl();
        break;
    case ft_decimal:
    case ft_money:
        ret = (long)getFVDecimal();
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa:
        ret = (long)getFVnumeric();
        break;

    case ft_lvar:
        break;
    default:
        return 0;
    }
    return ret;
}

float field::getFVflt() const
{
    return (float)getFVdbl();
}

double field::getFVdbl() const
{
    if (!m_ptr)
        return 0;
    double ret = 0;
    switch (m_fd->type)
    {
    case ft_currency:
        ret = (double)*((__int64*)((char*)m_ptr + m_fd->pos));
        ret = ret / 10000;
        break;

    case ft_bfloat:
    case ft_timestamp:
    case ft_float:
        switch (m_fd->len)
        {
        case 4:
            ret = (double)*((float*)((char*)m_ptr + m_fd->pos));
            break;
        case 10: // long double
        case 8:
            ret = (double)*((double*)((char*)m_ptr + m_fd->pos));
            break;
        }
        break;
    case ft_string:
    case ft_zstring:
    case ft_note:
    case ft_myvarbinary:
    case ft_myvarchar:
    case ft_mychar:
        ret = atof(getFVAstr());
        break;
    case ft_wstring:
    case ft_wzstring:
    case ft_mywvarbinary:
    case ft_mywvarchar:
    case ft_mywchar:
        ret = _ttof(getFVstr());
        break;
    case ft_integer:
    case ft_date:
    case ft_time:
    case ft_autoIncUnsigned:
    case ft_uinteger:
    case ft_logical:
    case ft_autoinc:
    case ft_bit:
    case ft_mydate:
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
        ret = (double)getFV64();
        break;

    case ft_decimal:
    case ft_money:
        ret = getFVDecimal();
        break;
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa:
        ret = getFVnumeric();
        break;
    case ft_lvar:
        break;
    default:
        return 0;
    }
    return ret;
}

unsigned char field::getFVbyt() const
{
    return (unsigned char)getFVlng();
}

#ifdef _WIN32

const wchar_t* field::getFVWstr() const
{
    if (!m_ptr)
        return L"";
    char* data = (char*)m_ptr + m_fd->pos;
    switch (m_fd->type)
    {
    case ft_string:
        return read<stringStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv(), m_fd->trimPadChar());
    case ft_note:
    case ft_zstring:
        return read<zstringStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv());
    case ft_wzstring:
        return read<wzstringStore, WCHAR, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                                 m_fds->cv());
    case ft_wstring:
        return read<wstringStore, WCHAR, WCHAR>(
            data, m_fds->strBufs(), *m_fd, m_fds->cv(), m_fd->trimPadChar());
    case ft_mychar:
        return read<myCharStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv(), m_fd->trimPadChar());
    case ft_myvarchar:
        return read<myVarCharStore, char, WCHAR>(data, m_fds->strBufs(), *m_fd,
                                                 m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, WCHAR>(data, m_fds->strBufs(),
                                                   *m_fd, m_fds->cv());
    case ft_mywchar:
        return read<myWcharStore, WCHAR, WCHAR>(
            data, m_fds->strBufs(), *m_fd, m_fds->cv(), m_fd->trimPadChar());
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, WCHAR>(data, m_fds->strBufs(),
                                                   *m_fd, m_fds->cv());
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, WCHAR>(data, m_fds->strBufs(),
                                                     *m_fd, m_fds->cv());
    case ft_myblob:
    case ft_mytext:
        return readBlob<WCHAR>(data, m_fds->strBufs(), *m_fd, m_fds->cv());
    }
    wchar_t* p = (wchar_t*)m_fds->strBufs()->getPtrW(max(m_fd->len * 2, 50));

    wchar_t buf[10] = L"%0.";

    switch (m_fd->type)
    {

    case ft_integer:
    case ft_bit:
    case ft_autoinc:
        _i64tow_s(getFV64(), p, 50, 10);
        return p;
    case ft_logical:
        if (m_fds->logicalToString)
        {
            if (getFVlng())
                return L"Yes";
            else
                return L"No";
        }
        else
            _i64tow_s(getFV64(), p, 50, 10);

    case ft_bfloat:
    case ft_float:
    case ft_currency:
    {

        swprintf_s(p, 50, L"%lf", getFVdbl());
        int k = (int)wcslen(p) - 1;
        while (k >= 0)
        {
            if (p[k] == L'0')
                p[k] = 0x00;
            else if (p[k] == L'.')
            {
                p[k] = 0x00;
                break;
            }
            else
                break;
            k--;
        }
        break;
    }
    case ft_autoIncUnsigned:
    case ft_uinteger:
        swprintf_s(p, 50, L"%lu", getFV64());
        break;
    case ft_date:
        return btrdtoa(getFVlng(), p);
    case ft_time:
        return btrttoa(getFVlng(), p);
    case ft_mydate:
    {
        myDate d;
        d.setValue((int)getValue64(*m_fd, (const uchar_td*)m_ptr));
        return d.toStr(p, m_fds->myDateTimeValueByBtrv);
    }
    case ft_mytime:
    {

        myTime t(m_fd->len);
        t.setValue(getValue64(*m_fd, (const uchar_td*)m_ptr));
        return t.toStr(p);
    }
    case ft_mydatetime:
    {
        myDateTime t(m_fd->len);
        t.setValue(getFV64());
        return t.toStr(p);
    }
    case ft_mytimestamp:
    {
        myTimeStamp ts(m_fd->len);
        ts.setValue(getFV64());
        return ts.toStr(p);
    }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa:
        _ltow_s(m_fd->decimals, p, 50, 10);
        wcscat(buf, p);
        wcscat(buf, L"lf");
        swprintf(p, 50, buf, getFVdbl());
        break;
    case ft_lvar:
        return NULL;
    case ft_timestamp:
        return btrTimeStamp(getFV64()).toString(p);
    default:
        p[0] = 0x00;
    }
    return p;
}

#endif //_WIN32

const char* field::getFVAstr() const
{
    if (!m_ptr)
        return "";

    char buf[10] = "%0.";

    char* data = (char*)m_ptr + m_fd->pos;
    switch (m_fd->type)
    {

    case ft_string:
        return read<stringStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                             m_fds->cv(), m_fd->trimPadChar());
    case ft_note:
    case ft_zstring:
        return read<zstringStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                              m_fds->cv());
    case ft_wzstring:
        return read<wzstringStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                                m_fds->cv());
    case ft_wstring:
        return read<wstringStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv(), m_fd->trimPadChar());
    case ft_mychar:
        return read<myCharStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                             m_fds->cv(), m_fd->trimPadChar());
    case ft_myvarchar:
        return read<myVarCharStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                                m_fds->cv());
    case ft_lstring:
    case ft_myvarbinary:
        return read<myVarBinaryStore, char, char>(data, m_fds->strBufs(), *m_fd,
                                                  m_fds->cv());
    case ft_mywchar:
        return read<myWcharStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                               m_fds->cv(), m_fd->trimPadChar());
    case ft_mywvarchar:
        return read<myWvarCharStore, WCHAR, char>(data, m_fds->strBufs(), *m_fd,
                                                  m_fds->cv());
    case ft_mywvarbinary:
        return read<myWvarBinaryStore, WCHAR, char>(data, m_fds->strBufs(),
                                                    *m_fd, m_fds->cv());
    case ft_myblob:
    case ft_mytext:
        return readBlob<char>(data, m_fds->strBufs(), *m_fd, m_fds->cv());
    }
    char* p = m_fds->strBufs()->getPtrA(max(m_fd->len * 2, 50));
    switch (m_fd->type)
    {
    case ft_integer:
    case ft_bit:
    case ft_autoinc:
        _i64toa_s(getFV64(), p, 50, 10);
        return p;
    case ft_logical:
        if (m_fds->logicalToString)
        {
            if (getFVlng())
                return "Yes";
            else
                return "No";
        }
        else
            _i64toa_s(getFV64(), p, 50, 10);
        break;
    case ft_bfloat:
    case ft_float:
    case ft_currency:
    {
        sprintf(p, "%lf", getFVdbl());
        size_t k = strlen(p) - 1;
        while (1)
        {
            if (p[k] == '0')
                p[k] = 0x00;
            else if (p[k] == '.')
            {
                p[k] = 0x00;
                break;
            }
            else
                break;
            k--;
        }
        break;
    }
    case ft_date:
        return btrdtoa(getFVlng(), p);
    case ft_time:
        return btrttoa(getFVlng(), p);
    case ft_autoIncUnsigned:
    case ft_uinteger:
        sprintf_s(p, 50, "%llu", (unsigned __int64)getFV64());
        break;

    case ft_mydate:
    {
        myDate d;
        d.setValue((int)getValue64(*m_fd, (const uchar_td*)m_ptr));
        return d.toStr(p, m_fds->myDateTimeValueByBtrv);
    }
    case ft_mytime:
    {
        myTime t(m_fd->len);
        t.setValue(getValue64(*m_fd, (const uchar_td*)m_ptr));
        return t.toStr(p);
    }
    case ft_mytimestamp:
    {
        myTimeStamp ts(m_fd->len);
        ts.setValue(getFV64());
        return ts.toStr(p);
    }
    case ft_mydatetime:
    {
        myDateTime t(m_fd->len);
        t.setValue(getFV64());
        return t.toStr(p);
    }
    case ft_decimal:
    case ft_money:
    case ft_numeric:
    case ft_numericsts:
    case ft_numericsa:
        _ltoa_s(m_fd->decimals, p, 50, 10);
        strcat(buf, p);
        strcat(buf, "lf");
        sprintf_s(p, 50, buf, getFVdbl());
        break;
    case ft_lvar:
        return NULL;
    case ft_timestamp:
        return btrTimeStamp(getFV64()).toString(p);
    default:
        p[0] = 0x00;
    }
    return p;
}

int field::getFVint() const
{
    return (int)getFVlng();
}

__int64 field::getFV64() const
{
    if (!m_ptr)
        return 0;

    switch (m_fd->len)
    {
    case 8:
        switch (m_fd->type)
        {
        case ft_autoIncUnsigned:
        case ft_uinteger:
        case ft_integer:
        case ft_logical:
        case ft_autoinc:
        case ft_bit:
        case ft_currency:
        case ft_timestamp:
        case ft_mydatetime:
        case ft_mytimestamp:
            return (__int64) * ((__int64*)((char*)m_ptr + m_fd->pos));
        case ft_float:
            return (__int64) *((double*)((char*)m_ptr + m_fd->pos));
        }
        return 0;
    case 7:
    case 6:
    case 5:
        switch (m_fd->type)
        {
        case ft_mytime:
        case ft_mydatetime:
        case ft_mytimestamp:
        {
            __int64 v = 0;
            memcpy(&v, (char*)m_ptr + m_fd->pos, m_fd->len);
            return v;
        }
        }
        return 0;
    default:

        return (__int64)getFVlng();
    }
}

void field::setFV(__int64 data)
{
    if (!m_ptr)
        return;

    switch (m_fd->len)
    {
    case 8:
        switch (m_fd->type)
        {
        case ft_autoIncUnsigned:
        case ft_uinteger:
        case ft_integer:
        case ft_logical:
        case ft_autoinc:
        case ft_bit:
        case ft_currency:
        case ft_mydatetime:
        case ft_mytimestamp:
            *((__int64*)((char*)m_ptr + m_fd->pos)) = data;
            break;
        case ft_timestamp:
        {
            btrDate d;
            d.i = getNowDate();
            btrTime t;
            t.i = getNowTime();
            *((__int64*)((char*)m_ptr + m_fd->pos)) = btrTimeStamp(d, t).i64;
            break;
        }
        case ft_float:
        {
            double d = (double)data;
            setFV(d);
            break;
        }
        }
        break;
    case 7:
    case 6:
    case 5:
        switch (m_fd->type)
        {
        case ft_mytime:
        case ft_mydatetime:
        case ft_mytimestamp:
            memcpy((char*)m_ptr + m_fd->pos, &data, m_fd->len);
            break;
        default:
            break;
        }
        break;
    default:
    {
        int value = (int)data;
        setFV(value);
        break;
    }
    }
}

/* if blob and text ,set binary data that is only set pointer. it is not copied.
 *  Caller must hold data until it sends to the server.
 */
void field::setFV(const void* data, uint_td size)
{
    if (!m_ptr)
        return;
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

/* offset is writen at data that is first address of data
 *  text is not converted to unicode form stored charset.
 */
void* field::getFVbin(uint_td& size) const
{
    if (!m_ptr)
        return 0;

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

double field::getFVDecimal() const
{
    if (!m_ptr)
        return 0;

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

double field::getFVnumeric() const
{
    if (!m_ptr)
        return 0;

    char* t;
    char dp[] = "{ABCDEFGHI}JKLMNOPQR";
    char dpsa[] = "PQRSTUVWXYpqrstuvwxy";
    char* pdp = NULL;
    char i;
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

void field::setFVDecimal(double data)
{ // Double  -> Decimal
    if (!m_ptr)
        return;

    char buf[30] = "%+0";
    char dummy[30];
    int point;
    bool sign = false;
    unsigned char n;
    int i, k;
    int strl;
    bool offset = false;
    ;
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
                    ;
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

void field::setFVNumeric(double data)
{ // Double  -> Numeric
    if (!m_ptr)
        return;

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
    }

    memcpy((void*)((char*)m_ptr + m_fd->pos), t, m_fd->len);
}

template <class T>
inline int compNumber(const field& l, const field& r, char logType)
{
    return compare<T>((const char*)l.ptr(), (const char*)r.ptr());
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
    case ft_mytime:
    case ft_mydatetime:
    case ft_mytimestamp:
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

int field::comp(const field& r, char logType) const
{
    compFieldFunc f = getCompFunc(logType);
    return f(*this, r, logType);
}

bool field::isCompPartAndMakeValue()
{
    bool ret = false;
    if (m_fd->isStringType())
    {
        m_fd->setPadCharSettings(false, true);
        /*bool trim = m_fd->trimPadChar();
        bool use = m_fd->usePadChar();
        bool sp = (!trim || use);
        if (sp)
            m_fd->setPadCharSettings(false, true);*/
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
        /*if (sp)
            m_fd->setPadCharSettings(use, trim); */
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
