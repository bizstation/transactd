//---------------------------------------------------------------------------

#ifndef ormMapDataH
#define ormMapDataH
//---------------------------------------------------------------------------
#define USE_VEC         // or  USE_MAP
#define USE_INDEX       // if not define USE_INDEX use sort
#define USE_ANY_TYPE    // or  USE_VARIANT


#include <boost/unordered_map.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/pool/pool_alloc.hpp>

#include <bzs/db/protocol/tdap/client/trdormapi.h>
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/env/tstring.h>


namespace tdc =  bzs::db::protocol::tdap::client;
namespace td =  bzs::db::protocol::tdap;
class map_orm;



#ifdef USE_ANY_TYPE
    /* use boost::any data taype*/
    typedef boost::any var_type;
    #define value_get boost::any_cast
    /*#include "fast_any.h"
    typedef any var_type;
    #define value_get any_cast
    */
#elif USE_VARIANT _TYPE
    /* use boost::valiant data taype*/
    #include <boost/variant.hpp>
    typedef boost::variant<bool, char, short, int, __int64
                , std::_tstring, double, float
                , td::btrDate, td::btrTime, td::btrDateTime> variant_type;
    typedef variant_type var_type;
    #define value_get boost::get
#endif //USE_VARIANT


/*  container of a row  */
#ifdef USE_MAP
    typedef  boost::unordered_map<std::_tstring, var_type> row;
    typedef  std::_tstring key_type;
#else
    typedef  int key_type;
#   ifdef USE_VEC
    typedef  std::vector<var_type> row;
    //typedef  std::vector<var_type , boost::pool_allocator<var_type> > row;
#   else
    #include <boost/shared_array.hpp>
    typedef  boost::shared_array<var_type> row;
#   endif
#endif

typedef boost::shared_ptr<row> row_ptr;
typedef std::vector<row_ptr> row_ptr_list;
row* create(row_ptr_list& m, int)
{
    row* p = new row();
    return p;
}

/* If row containe is vecttor, use original rowset conatiner*/
#ifdef USE_VEC
class dataset
{
    std::vector<std::_tstring> m_header;
    row_ptr_list m_rows;
    std::vector<int> m_index;
public:


    typedef row_ptr_list::iterator iterator;

    dataset(){};
    void clear()
    {
        m_header.clear();
        m_rows.clear();
        m_index.clear();
    }
    std::vector<int>& index(){return m_index;};

    void reserve(int n){m_rows.reserve(n);}
    row_ptr operator[](int index){return m_rows[index];}
    row_ptr at(int index){return m_rows.at(index);}
    size_t size()const {return m_rows.size();}
    std::vector<std::_tstring>& header() {return m_header;};
    row_ptr_list&  rows() {return m_rows;}

    iterator begin(){return m_rows.begin();}
    iterator end(){return m_rows.end();}
    void push_back(const row_ptr v){return m_rows.push_back(v);}

    /*original function*/
    /* if header_type is defined then mdlsHandler handler call readBefore
        function at before reading.
    */
    typedef std::vector<std::_tstring> header_type;
    void readBefore( const tdc::table_ptr tb)
    {
        const td::tabledef* def = tb->tableDef();
        int n = tb->getCurProcFieldCount();

        for (int i=0;i<n;++i)
            m_header.push_back(def->fieldDefs[tb->getCurProcFieldIndex(i)].name());
    }

    typedef key_type group_key_type;
    group_key_type keyTypeValue(const std::_tstring& name, bool noexception=false)
    {
        #ifdef USE_MAP
            return name;
        #else
            for (int i=0;i<m_header.size();++i)
                if (m_header[i] == name)
                    return i;
            if (!noexception)
                throw bzs::rtl::exception(0, _T("groupQuery:Invalid key name"));
            return m_header.size();
        #endif
    }


};

namespace bzs{namespace db{namespace protocol{namespace tdap{namespace client
{
template <class T>
inline dataset::iterator begin(dataset& m){return m.begin();}

template <class T>
inline dataset::iterator end(dataset& m){return m.end();}

template <class T>
inline void push_back(dataset& m, T c){m.push_back(c);}

}}}}}


row* create(dataset& m, int)
{
    row* p = new row();
    p->reserve(10);
    return p;
}

#endif


class map_orm_fdi
{
    friend class map_orm;
    const tdc::table* m_tb;
public:
    void init(tdc::table* tb) {m_tb = tb;}
};

map_orm_fdi* createFdi(map_orm_fdi * ){return new map_orm_fdi();}
void destroyFdi(map_orm_fdi * p){delete p;}
void initFdi(map_orm_fdi * fdi, tdc::table* tb){fdi->init(tb);}

int __fastcall compAny(const var_type& l, const var_type& r)
{

    const std::type_info& rt = r.type();
    const std::type_info& lt = l.type();
    if (rt != lt)
        return strcmp(rt.name(), rt.name());
    if (rt == typeid(std::_tstring))
    {
        const std::_tstring& ls = value_get<const std::_tstring&>(l);
        const std::_tstring& rs = value_get<const std::_tstring&>(r);
        return (ls < rs) ? -1 : (ls > rs) ? 1 : 0;

    }
    else if (rt == typeid(int))
        return value_get<int>(l) - value_get<int>(r);
    else if (rt == typeid(__int64))
    {
        __int64 ls = value_get<__int64>(l);
        __int64 rs = value_get<__int64>(r);
        return (ls < rs) ? -1 : (ls > rs) ? 1 : 0;
    }
    else if (rt == typeid(char))
        return value_get<char>(l) - value_get<char>(r);
    else if (rt == typeid(short))
        return value_get<short>(l) - value_get<short>(r);
    /*else if (rt == typeid(unsigned char))
        return (int)value_get<unsigned char>(l) - (int)value_get<unsigned char>(r);
    else if (rt == typeid(unsigned short))
        return (int)value_get<unsigned short>(l) - (int)value_get<unsigned short>(r);
    else if (rt == typeid(unsigned int))
    {
        unsigned int ls = value_get<unsigned int>(l);
        unsigned int rs = value_get<unsigned int>(r);
        if (ls < rs) return -1;
        if (ls > rs) return 1;
        return 0;
    }*/
    else if (rt == typeid(double))
    {
        double ls = value_get<double>(l);
        double rs = value_get<double>(r);
        return (ls < rs) ? -1 : (ls > rs) ? 1 : 0;
    }
    else if (rt == typeid(float))
    {
        float ls = value_get<float>(l);
        float rs = value_get<float>(r);
        return (ls < rs) ? -1 : (ls > rs) ? 1 : 0;
    }
    else if (rt == typeid(bool))
        return (int)value_get<bool>(l) - (int)value_get<bool>(r);
    else if (rt == typeid(td::btrDate))
        return value_get<td::btrDate>(l).i - value_get<td::btrDate>(r).i;
    else if (rt == typeid(td::btrTime))
        return value_get<td::btrTime>(l).i - value_get<td::btrTime>(r).i;
    else if (rt == typeid(td::btrDateTime))
        return value_get<td::btrDateTime>(l).i64 - value_get<td::btrDateTime>(r).i64;
    else if (rt == typeid(void))
        return 1;

    assert(0);
    return 0;
}

std::_tstring toString(var_type& a)
{
        if (a.type() == typeid(std::_tstring))
            return value_get<std::_tstring>(a);
        else if (a.type() == typeid(char))
            return boost::lexical_cast<std::_tstring>(value_get<char>(a));
        else if (a.type() == typeid(short))
            return boost::lexical_cast<std::_tstring>(value_get<short>(a));
        else if (a.type() == typeid(int))
            return boost::lexical_cast<std::_tstring>(value_get<int>(a));
        else if (a.type() == typeid(__int64))
            return boost::lexical_cast<std::_tstring>(value_get<__int64>(a));
        else if (a.type() == typeid(char*))
            return boost::lexical_cast<std::_tstring>(value_get<char*>(a));
        else if (a.type() == typeid(unsigned char))
            return boost::lexical_cast<std::_tstring>(value_get<unsigned char>(a));
        else if (a.type() == typeid(unsigned short))
            return boost::lexical_cast<std::_tstring>(value_get<unsigned short>(a));
        else if (a.type() == typeid(unsigned int))
            return boost::lexical_cast<std::_tstring>((__int64)value_get<unsigned int>(a));
        else if (a.type() == typeid(double))
            return boost::lexical_cast<std::_tstring>(value_get<double>(a));
        else if (a.type() == typeid(float))
            return boost::lexical_cast<std::_tstring>(value_get<float>(a));
        else if (a.type() == typeid(bool))
            return boost::lexical_cast<std::_tstring>((int) value_get<bool>(a));
        else if (a.type() == typeid(td::btrDate))
            return boost::lexical_cast<std::_tstring>(c_str(value_get<td::btrDate>(a)));
        else if (a.type() == typeid(td::btrTime))
            return boost::lexical_cast<std::_tstring>(c_str(value_get<td::btrTime>(a)));
        else if (a.type() == typeid(td::btrDateTime))
            return boost::lexical_cast<std::_tstring>(c_str(value_get<td::btrDateTime>(a)));
        return _T("");
}

const _TCHAR* toString(var_type& a, _TCHAR* buf, size_t size)
{
        if (a.type() == typeid(std::_tstring))
            return value_get<const std::_tstring&>(a).c_str();
        else if (a.type() == typeid(char))
            return _ltot_s((long)value_get<char>(a), buf, size, 10);
        else if (a.type() == typeid(short))
            return _ltot_s((long)value_get<short>(a), buf, size, 10);
        else if (a.type() == typeid(int))
            return _ltot_s((long)value_get<int>(a), buf, size, 10);
        else if (a.type() == typeid(__int64))
            return _i64tot_s(value_get<__int64>(a), buf, size, 10);
        else if (a.type() == typeid(unsigned char))
            return _ultot_s((unsigned long)value_get<unsigned char>(a), buf, size, 10);
        else if (a.type() == typeid(unsigned short))
            return _ultot_s((unsigned long)value_get<unsigned short>(a), buf, size, 10);
        else if (a.type() == typeid(unsigned int))
            return _ultot_s((unsigned long)value_get<unsigned int>(a), buf, size, 10);
        else if (a.type() == typeid(double))
        {
            _stprintf_s(buf, size, _T("%f"), value_get<double>(a));
            return buf;
        }
        else if (a.type() == typeid(float))
        {
            _stprintf_s(buf, size, _T("%f"), value_get<float>(a));
            return buf;
        }
        else if (a.type() == typeid(bool))
            return _ltot_s((long)value_get<bool>(a), buf, size, 10);
        else if (a.type() == typeid(td::btrDate))
            return td::btrdtoa(value_get<td::btrDate>(a), buf);
        else if (a.type() == typeid(td::btrTime))
            return td::btrttoa(value_get<td::btrTime>(a), buf);
        else if (a.type() == typeid(td::btrDateTime))
            return td::btrstoa(value_get<td::btrDateTime>(a), buf);
        return _T("");
}

class map_orm
{
    const map_orm_fdi& m_fdi;
    short m_autoIncFiled;
    void set(short index, const td::fielddef* fddef, row& m, const tdc::fields& fds)
    {
        #ifdef USE_MAP
        if (m.count(fddef->name()) == 0) return;
        var_type& a = m[fddef->name()];
        #else
        #ifdef USE_VEC
        if (index >= (short)m.size()) return;
        #endif
        var_type& a = m[index];
        #endif
        tdc::field fd = fds.inproc_fd(index);
        if (a.type() == typeid(std::_tstring))
            fd =  value_get<std::_tstring>(a).c_str();
        else if (a.type() == typeid(char))
            fd =  value_get<char>(a);
        else if (a.type() == typeid(short))
            fd =  value_get<short>(a);
        else if (a.type() == typeid(int))
            fd =  value_get<int>(a);
        else if (a.type() == typeid(__int64))
            fd =  value_get<__int64>(a);
        else if (a.type() == typeid(char*))
            fd =  value_get<char*>(a);
        else if (a.type() == typeid(unsigned char))
            fd =  value_get<unsigned char>(a);
        else if (a.type() == typeid(unsigned short))
            fd =  value_get<unsigned short>(a);
        else if (a.type() == typeid(unsigned int))
            fd =  (__int64)value_get<unsigned int>(a);
        else if (a.type() == typeid(double))
            fd =  value_get<double>(a);
        else if (a.type() == typeid(float))
            fd =  value_get<float>(a);
        else if (a.type() == typeid(bool))
            fd = (int) value_get<bool>(a);
        else if (a.type() == typeid(td::btrDate))
            fd = (int) value_get<td::btrDate>(a).i;
        else if (a.type() == typeid(td::btrTime))
            fd = (int) value_get<td::btrTime>(a).i;
        else if (a.type() == typeid(td::btrDateTime))
            fd =  value_get<td::btrDateTime>(a).i64;
   }

    void get(short index, short offset, const td::fielddef* fddef, row& m, const tdc::fields& fds)
    {
        #ifdef USE_MAP
        var_type& a =  m[fddef->name()];
        #else
        #ifdef USE_VEC
        if ((index+ offset) >= (short)m.size())
            m.push_back(var_type());
        #endif

        var_type& a =  m[index + offset];
        #endif
        tdc::field fd = fds.inproc_fd(index);
        //ToDo not suppot binary
        switch(fddef->type)
        {
        case ft_string:
        case ft_lstring:
        case ft_zstring:
        case ft_note:
        case ft_lvar:
        case ft_wstring:
        case ft_wzstring:
        case ft_guid:
        case ft_myvarchar:
        case ft_myvarbinary:
        case ft_mywvarchar:
        case ft_mywvarbinary:
        case ft_mychar:
        case ft_mywchar:
        case ft_mytext:
        case ft_myblob:
        case ft_blob:
            a = std::_tstring(fd.c_str());
            break;
        case ft_date:
        case ft_mydate:
        {
            td::btrDate b;
            b.i = fd.i();
            a = b;
            break;
        }
        case ft_time:
        case ft_mytime:
        {
            td::btrTime b;
            b.i = fd.i();
            a = b;
            break;
        }
        case ft_datetime:
        case ft_mydatetime:
        case ft_timestamp:
        case ft_mytimestamp:
        {
            td::btrDateTime b;
            b.i64 = fd.i64();
            a = b;
            break;
        }
        case ft_integer:
        case ft_uinteger:
        case ft_autoinc:
        case ft_bit:
        case ft_autoIncUnsigned:
            a = fd.i64();
            break;
        case ft_float:
        case ft_bfloat:
        case ft_decimal:
        case ft_money:
        case ft_logical:
        case ft_numeric:
        case ft_numericsts:
        case ft_numericsa:
        case ft_currency:
            a = fd.d();
            break;
        }
    }

    bool comp(row& lm, row& rm, const _TCHAR* name, int index) const
    {
        #ifdef USE_MAP
        if ((lm.count(name) == 0) || (rm.count(name) == 0))//return false;
            assert(0);
        var_type r =  rm[name];
        var_type l =  lm[name];
        #else
        #ifdef USE_VEC
        if ((index >= (short)lm.size()) || (index >= (short)rm.size()))
            assert(0);
        #endif
        var_type r =  rm[index];
        var_type l =  lm[index];
        #endif
        return (compAny(l, r) < 0);
    }


public:
    map_orm(const map_orm_fdi& fdi):m_fdi(fdi),m_autoIncFiled(-2){}

    bool compKeyValue(row& l, row& r, int keyNum) const
    {
        const td::tabledef* def = m_fdi.m_tb->tableDef();
        const td::keydef* kd = &def->keyDefs[keyNum];
        for (int i=0;i<kd->segmentCount;++i)
        {
            short n =  kd->segments[i].fieldNum;
            const td::fielddef* fd = &def->fieldDefs[n];
            int ret = comp(l, r, fd->name(), n);
            if (ret)return (ret < 0);

        }
        return 0;
    }

    void setKeyValues(row& m, const tdc::fields& fds, int keyNum)
    {
        const td::tabledef* def = fds.tb().tableDef();
        const td::keydef* kd = &def->keyDefs[keyNum];
        for (int i=0;i<kd->segmentCount;++i)
        {
            short n =  kd->segments[i].fieldNum;
            const td::fielddef* fd = &def->fieldDefs[n];
            set(n, fd, m, fds);
        }

    }

    void writeMap(row& m, const tdc::fields& fds, int optipn)
    {
        //フィールドのSelectを加える
        const td::tabledef* def = fds.tb().tableDef();
        for (int i=0;i<def->fieldCount;++i)
        {
            const td::fielddef* fd = &def->fieldDefs[i];
            set(i, fd, m, fds);
        }
    }

    void readMap(row& m, const tdc::fields& fds, int optipn)
    {
        //フィールドのSelectを加える
        const tdc::table& tb = fds.tb();
        const td::tabledef* def = tb.tableDef();
        int n = fds.inproc_size();

        #ifndef USE_VEC
        #ifndef USE_MAP
            m.reset(new var_type[n]);
        #endif
        #endif
        if (m.size() < n)
            m.resize(n);

        for (int i=0;i<n;++i)
        {
            const td::fielddef* fd = &def->fieldDefs[tb.getCurProcFieldIndex(i)];
            get(i ,optipn, fd, m, fds);
        }

    }

    void readAuntoincValue(row& m, const tdc::fields& fds, int optipn)
    {
        const td::tabledef* def = fds.tb().tableDef();
        if (m_autoIncFiled == -2)
		{
			for (int i=0;i<def->keyCount;++i)
			{
				const td::keydef* kd = &def->keyDefs[i];
                const td::fielddef* fd = &def->fieldDefs[kd->segments[0].fieldNum];
				if ((fd->type == ft_autoinc) || (fd->type == ft_autoIncUnsigned))
				{
					m_autoIncFiled = i;
					break;
				}
			}
			if (m_autoIncFiled == -2)
				m_autoIncFiled = -1;
		}
        if (m_autoIncFiled >= 0)
            get(m_autoIncFiled, optipn, &def->fieldDefs[m_autoIncFiled], m, fds);
    }


    typedef row         mdl_typename;
    typedef map_orm_fdi fdi_typename;
    typedef tdc::mdlsHandler< map_orm, dataset> collection_orm_typename;

    typedef row_ptr     mdl_pure_typename;
    /* for group by key type*/
    int compByGroupKey(row& l, row& r, const dataset::group_key_type& s)
    {
        #ifdef USE_MAP
            if ((l->count(s) == 0) || (r->count(s) == 0))
                throw bzs::rtl::exception(0, _T("groupSort:Invalid key name"));
        #else
            if ((s >= l.size()) || (s >= r.size()))
                throw bzs::rtl::exception(0, _T("groupSort:Invalid key index"));
        #endif
        return compAny(l.at(s), r.at(s));
    }

};










template <class T>
class sum
{
    T m_value;
public:
    sum():m_value(0){}
    void operator()(var_type& v)
    {
        m_value += value_get<T>(v);
    }
    T result()const{return m_value;}
    void reset(){m_value = 0;}
    typedef typename T value_type;
};
typedef sum<__int64> sum64;

#endif
