//---------------------------------------------------------------------------

#ifndef ormMapDataH
#define ormMapDataH
//---------------------------------------------------------------------------

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



/* use boost::any data taype*/
typedef boost::any var_type;
#define value_get boost::any_cast

/*  container of a row for read */
typedef int key_type;
typedef std::vector<var_type> row;
typedef boost::shared_ptr<row> row_ptr;
typedef std::vector<row_ptr> row_ptr_list;


/*  container of a row for update insert  */
typedef boost::unordered_map<std::_tstring, var_type> row_n;
typedef std::_tstring key_type_n;
typedef boost::shared_ptr<row_n> row_n_ptr;


int comp_any(const var_type& l, const var_type& r)
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

template <class T>
void setValue(row& row, key_type key, const T& value)
{
	row[key] = value;
}


row* create(row_ptr_list& m, int)
{
	row* p = new row();
	return p;
}

/* If row containe is vecttor, use original rowset conatiner*/
class dataset
{
	std::vector<std::_tstring> m_header;
	row_ptr_list m_rows;
public:
	typedef row_ptr_list::iterator iterator;

	dataset(){};
	void clear()
	{
		m_rows.clear();
	}
	void reset()
	{
		m_header.clear();
		m_rows.clear();
	}

	inline void reserve(int n){m_rows.reserve(n);}
	inline row_ptr operator[](int index){return m_rows[index];}
	inline row_ptr at(int index){return m_rows.at(index);}
	inline size_t size()const {return m_rows.size();}
	inline std::vector<std::_tstring>& header() {return m_header;};
	inline row_ptr_list&  rows() {return m_rows;}

	inline iterator begin(){return m_rows.begin();}
	inline iterator end(){return m_rows.end();}
	inline void push_back(const row_ptr v){return m_rows.push_back(v);}
	inline iterator erase(const iterator& it){return m_rows.erase(it);}

	/*original function*/
	/* if header_type is defined then mdlsHandler handler call readBefore
		function at before reading.
	*/
	//ToDo not inplemnts alias
	typedef std::vector<std::_tstring> header_type;
	void readBefore( const tdc::table_ptr tb, const tdc::aliasMap_type* alias)
	{
		const td::tabledef* def = tb->tableDef();
		int n = tb->getCurProcFieldCount();
		for (int i=0;i<n;++i)
			m_header.push_back(def->fieldDefs[tb->getCurProcFieldIndex(i)].name());
	}

	/* resolv key value from name*/
	typedef ::key_type key_type;
	key_type resolvKeyValue(const std::_tstring& name, bool noexception=false)
	{
		for (int i=0;i<(int)m_header.size();++i)
			if (m_header[i] == name)
				return i;
		if (!noexception)
			throw bzs::rtl::exception(0, _T("groupQuery:Invalid key name"));
		return m_header.size();
	}

	/* define row type*/
	typedef row_ptr row_type;
};


namespace bzs{namespace db{namespace protocol{namespace tdap{namespace client
{

inline dataset::iterator begin(dataset& m){return m.begin();}
inline dataset::iterator end(dataset& m){return m.end();}

inline void push_back(dataset& m, row_ptr c)
{
	m.push_back(c);
}

/* for groupby */
inline void clear(dataset& m)
{
	return m.clear();
}

/* for groupby */
inline dataset::key_type resolvKeyValue(dataset& m, const std::_tstring& name
	, bool noexception=false)
{
	return m.resolvKeyValue(name, noexception);
}

}}}}}


row* create(dataset& m, int)
{
	row* p = new row();
	p->reserve(10);
	return p;
}



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
	void set(short index, const td::fielddef* fddef, row_n& m, const tdc::fields& fds)
	{
		if (m.count(fddef->name()) == 0) return;

		var_type& a = m[fddef->name()];

		//if (index >= (short)m.size()) return;
		//var_type& a = m[index];

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

	inline var_type& getFiled(short index, const _TCHAR* name , row_n& m)
	{
		return m[name];
	}

	inline var_type& getFiled(short index, const _TCHAR* name , row& m)
	{
		if (index >= (short)m.size())
			m.push_back(var_type());
		return m[index];
	}

	template <class T>
	void get(short index, short offset, const td::fielddef* fddef, T& m, const tdc::fields& fds)
	{
		//var_type& a =  m[fddef->name()];
		//var_type& a =  m[index + offset];

		var_type& a =  getFiled(index + offset, fddef->name(), m);

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
		if ((index >= (short)lm.size()) || (index >= (short)rm.size()))
			assert(0);
		var_type r =  rm[index];
		var_type l =  lm[index];
		return (comp_any(l, r) < 0);
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

	void setKeyValues(row_n& m, const tdc::fields& fds, int keyNum)
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

	void writeMap(row_n& m, const tdc::fields& fds, int optipn)
	{
		//フィールドのSelectを加える
		const td::tabledef* def = fds.tb().tableDef();
		for (int i=0;i<def->fieldCount;++i)
		{
			const td::fielddef* fd = &def->fieldDefs[i];
			set(i, fd, m, fds);
		}
	}

	inline void resizeRow(row& m, int n)
	{
		if ((int)m.size() < n)
		 m.resize(n);
	}

	inline void resizeRow(row_n& m, int n)
	{

	}

	template <class T>
	void readMap(T& m, const tdc::fields& fds, int optipn)
	{
		//フィールドのSelectを加える
		const tdc::table& tb = fds.tb();
		const td::tabledef* def = tb.tableDef();
		int n = fds.inproc_size();

		resizeRow(m, n);

		for (int i=0;i<n;++i)
		{
			const td::fielddef* fd = &def->fieldDefs[tb.getCurProcFieldIndex(i)];
			get(i ,optipn, fd, m, fds);
		}

	}


	void readAuntoincValue(row_n& m, const tdc::fields& fds, int optipn)
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


};

template <class row_type=row, class key_type=int, class value_type_=__int64>
class sum
{
	value_type_ m_value;
	key_type m_resultKey;
public:
	sum():m_value(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		m_value += value_get<value_type>(row[m_resultKey]);
	}
	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;}
	typedef typename value_type_ value_type;
};


template <class row_type=row, class key_type=int, class value_type_=__int64>
class count
{
	value_type_ m_value;
	key_type m_resultKey;
public:
	count():m_value(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		++m_value;
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;}
	typedef typename value_type_ value_type;
};


template <class row_type=row, class key_type=int, class value_type_=__int64>
class avg
{
	value_type_ m_value;
	value_type_ m_count;
	key_type m_resultKey;
public:
	avg():m_value(0),m_count(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		++m_count;
		m_value += value_get<value_type>(row[m_resultKey]);
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;m_count=0;}
	typedef typename value_type_ value_type;
};


template <class row_type=row, class key_type=int, class value_type_=__int64>
class min
{
	value_type_ m_value;
	key_type m_resultKey;
public:
	min():m_value(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		m_value = std::min(m_value, value_get<value_type>(row[m_resultKey]));
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;}
	typedef typename value_type_ value_type;
};

template <class row_type=row, class key_type=int, class value_type_=__int64>
class max
{
	value_type_ m_value;
	key_type m_resultKey;
public:
	max():m_value(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		m_value = std::max(m_value, value_get<value_type>(row[m_resultKey]));
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;}
	typedef typename value_type_ value_type;
};


#endif
