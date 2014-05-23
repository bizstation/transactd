/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include <bzs/db/protocol/tdap/client/trdormapi.h>
#include <boost/iterator/iterator_facade.hpp>
#include <iostream>
#include <vector>

/**
@brief Read records and manual O/R mapping example

This program read records of the "user" table where group = 3.
And O/R mapping to the user class.
This program use the filter operation on server side.

Please execute "create database" , "change schema" and "insert records" example
	before execute this example.

*/

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;


class user;
typedef boost::shared_ptr<user> user_ptr;
typedef std::vector<user_ptr> user_ptr_list;
typedef std::vector<user*> user_list;
typedef boost::shared_ptr<user_list> user_list_ptr;

class group;
typedef boost::shared_ptr<group> group_ptr;
typedef std::vector<group_ptr> group_ptr_list;
typedef std::vector<group*> group_list;
typedef boost::shared_ptr<group_list> group_list_ptr;


/** class group
	This is a model
*/
class group
{
	int m_id;
	std::string m_name;
	mutable user_ptr_list* m_users;

protected:
	group(void* owner):m_id(0),m_users(NULL){}

public:
	~group(){delete m_users;}

	int id() const {return m_id;}
	const std::string& name() const {return m_name;}
	void setId(int v){m_id = v;}
	void setName(const char* v){m_name = v;}
	user_ptr_list& users() const
	{
		if (!m_users) m_users = new user_ptr_list;
		return *m_users;
	}

	/* create instance */
	static group* create(void* owner){return new group(owner);};


};

group* create(group_ptr_list& m, int){return group::create(&m);}


/** class group_fdi
	This is a dynamic field index cache resolv from field name.
	An index is cached at instruction execution time,
	it becomes strong to change of a schema.
*/
class group_fdi
{

public:
	short id;
	short name;

	/* Implement the init function which a argument of 'table*'. */
	void init(table* tb)
	{
		id = tb->fieldNumByName(_T("id"));
		name = tb->fieldNumByName(_T("name"));
	}
};

/* Global functions of fdi instance and init  */
group_fdi* createFdi(group_fdi *){return new group_fdi();}
void destroyFdi(group_fdi * p){delete p;}
void initFdi(group_fdi * fdi, table* tb){fdi->init(tb);}

/* model table mapping class */
class group_orm
{
	const group_fdi& m_fdi;

public:

	/* Define  mdl_typename type */
	typedef group       mdl_typename;

	/*  Define fdi_typename type
		When fdi is not used , It is define "typedef fdibase fdi_typename;"
		When other fdi is used, you can the fdi type specify at create activeTable instance
		 like "activeTable<group, other_fdi>" */
	typedef group_fdi   fdi_typename;

	/* Constructor , which a argument of 'fdi_typename fdi&' */
	group_orm(const fdi_typename& fdi):m_fdi(fdi){}

	/* Compare function , only fields of unique keys are compared. */
	bool compKeyValue(group& l, group& r, int keyNum) const
	{
		return l.id() < r.id();
	}

	void setKeyValues(group& g, const fields& fds, int keyNum)
	{
		fds[m_fdi.id] = g.id();
	}

	void writeMap(group& g, const fields& fds, int optipn)
	{
		fds[m_fdi.id] = g.id();
		fds[m_fdi.name] = g.name();
	}

	void readMap(group& g, const fields& fds, int optipn)
	{
		g.setId(fds[m_fdi.id].i());
		g.setName(fds[m_fdi.name].a_str()); //get by ansi string
	}

	void readAuntoincValue(group& g, const fields& fds, int optipn)
	{
		g.setId(fds[m_fdi.id].i());
	}

	/* Get a table name from a target model. */
	const _TCHAR* getTableName(){return _T("group1");}
};

/** User class */
class user
{
	mutable group* m_grp;
	int m_id;
	int m_group_id;
	std::string m_name;
	std::string m_tel;

protected:
	user(void* owner):m_grp(NULL),m_id(0){}

public:
	~user()
	{
		delete m_grp;
	}
	group* grp()const
	{
		if (m_grp == NULL)
		{
			m_grp = group::create(0);
			m_grp->setId(m_group_id);
		}
		return m_grp;
	}
	int id() const {return m_id;}
	int group_id() const {return m_group_id;}
	const std::string& name() const {return m_name;}
	const std::string& tel() const {return m_tel;}
	void setId(int v){m_id = v;}
	void setGroup_id(int v){m_group_id = v;}
	void setName(const char* v){m_name = v;}
	void setTel(const char* v){m_tel = v;}

	static user* create(void* owner){return new user(owner);};
};


class user_fdi
{

public:
	short id;
	short name;
	short tel;
	short group_id;
	void init(table* tb)
	{
		id = tb->fieldNumByName(_T("id"));
		name = tb->fieldNumByName(_T("name"));
		tel = tb->fieldNumByName(_T("tel"));
		group_id = tb->fieldNumByName(_T("group"));
	}
};

user_fdi* createFdi(user_fdi *){return new user_fdi();}
void destroyFdi(user_fdi * p){delete p;}
void initFdi(user_fdi * fdi, table* tb){fdi->init(tb);}

//------------------------------------------------------------------------------
//     The original collection exsample
//------------------------------------------------------------------------------
/*  When the original collection used it is not vector,
	iterator for the collection is required.
*/
class mdls;

class mdlsIterator
	: public boost::iterator_facade<mdlsIterator, user*
							, boost::random_access_traversal_tag>
{
	friend class boost::iterator_core_access;
	size_t m_index;
	mdls* m_mdls;
	user*& dereference() const;
	void increment();
	void decrement();
	void advance(size_t n);
	int distance_to(const mdlsIterator &r)const;
	bool equal(const mdlsIterator &r) const;

public:
	mdlsIterator(mdls* m, size_t index=0);

};

/* This is a exsample of the origianl collection of user model. */
class mdls
{
	mutable std::vector<user*> m_users;
public:
	void clear(){m_users.clear();}

	user*& item(size_t index)const {return m_users[index];}

	user* add(user* u)
	{
		m_users.push_back(u);
		return u;
	}

	size_t size(){return m_users.size();}

	user* operator[](unsigned int index)const{return item(index);}

	/*  Define item_type type */
	typedef user* item_type;

	/*  Define iterator type */
	typedef mdlsIterator iterator;

};

/* implement th iterator */
user*& mdlsIterator::dereference() const
{
	return m_mdls->item(m_index);
}
void mdlsIterator::increment() {++m_index;}
void mdlsIterator::decrement() {--m_index;}
void mdlsIterator::advance(size_t n){m_index+=n;}
int mdlsIterator::distance_to(const mdlsIterator &r)const{return (int)(r.m_index - m_index);}
bool mdlsIterator::equal(const mdlsIterator &r) const {return m_index == r.m_index;}
mdlsIterator::mdlsIterator(mdls* m, size_t index):m_index(index),m_mdls(m){}


/* Implemant global functions of push_back begin end and clear in client namespace */
namespace bzs{namespace db{namespace protocol{namespace tdap{namespace client
{

template <>
inline mdlsIterator begin(mdls& m){return mdlsIterator(&m, 0);}

template <>
inline mdlsIterator end(mdls& m){return mdlsIterator(&m, m.size());}

#if (_MSC_VER || (__BORLANDC__))
inline void push_back(mdls& m, user* u){m.add(u);}
#else
template <>
inline void push_back(mdls& m, user* u){m.add(u);}
#endif

}}}}}

//------------------------------------------------------------------------------

user* create(mdls& m, int){return user::create(&m);}
user* create(user_ptr_list& m, int){return user::create(&m);}



class user_orm
{
	const user_fdi& m_fdi;

public:
	user_orm(const user_fdi& fdi):m_fdi(fdi){}

	bool compKeyValue(user& l, user& r, int keyNum) const
	{
		if (keyNum==0)
			return l.id() < r.id();
		return 1;
	}

	void setKeyValues(user& u, const fields& fds, int keyNum)
	{
		fds[m_fdi.id] = u.id();
	}

	void writeMap(user& u, const fields& fds, int optipn)
	{
		fds[m_fdi.id] = u.id();
		fds[m_fdi.name] = u.name();
		fds[m_fdi.tel] = u.tel();
		fds[m_fdi.group_id] = u.group_id();
	}

	void readMap(user& u, const fields& fds, int optipn)
	{
		u.setId(fds[m_fdi.id].i());
		u.setGroup_id(fds[m_fdi.group_id].i());
		u.setName(fds[m_fdi.name].a_str()); //get by ansi string
		u.setTel(fds[m_fdi.tel].a_str());  //get by ansi string
	}

	void readAuntoincValue(user& u, const fields& fds, int optipn)
	{
		u.setId(fds[m_fdi.id].i());
	}

	const _TCHAR* getTableName(){return _T("user");}

	typedef user        mdl_typename;
	typedef user_fdi    fdi_typename;

	/*  Definition the collection_orm_typename type when using an original collection.*/
	typedef mdlsHandler<user_orm, mdls>   collection_orm_typename;
};

typedef user_orm::collection_orm_typename users_orm;


//------------------------------------------------------------------------------

/* client filter function */
int isMatch(const fields& fds)
{
	return filter_validate_value;
}

/** dump user to screen */
template <class T>
void dumpUser(const T user)
{
	std::cout << " id           " << user->id()    << std::endl;
	std::cout << " name         " << user->name()  << std::endl;
	std::cout << " group        " << user->grp()->name() << std::endl;
	std::cout << " tel          " << user->tel()   << std::endl << std::endl;
}

bool sortFunc2(const user* l, const user* r)
{
	return l->name() < r->name();
}

void readUsers(databaseManager& db, std::vector<user_ptr>& users)
{
	static int find_group_id = 2;
	static const char_td keynum_group = 1;
	static const char_td primary_key = 0;

	/* Create the activeTable instance of user_orm. */
	activeTable<user_orm> ut(db);

	//-------------------------------------------------------------------------
	//   Single model operations
	//-------------------------------------------------------------------------
	/* Create user of id=12. */
	int id = 12;
	user_ptr u(user::create(0));
	u->setId(id);
	u->setName("moriwaki");
	u->setTel("81-999-9999");
	u->setGroup_id(1);

	/* Save a user  */
	ut.index(primary_key);
	ut.save(*u); // insert or update

	/* Read a user of id=12  */
	ut.read(*u);

	/* Update telephone number of the user. */
	u->setTel("81-999-8888");
	ut.update(*u);

	/* Changing unique key value .*/
	ut.index(primary_key).keyValue(u->id());
	u->setId(13);
	bool noKeyValueFromObj = false;
	ut.update(*u, noKeyValueFromObj);/* Dont read key value from model */

	/* Delete a user 1. */
	ut.del(*u);

	/* Delete a user 2. */
	//ut.index(primary_key).keyValue(id).del();

	/* Read a group model.
	   Using shared pointer.
	*/
	group_ptr grp(group::create(0));
	activeTable<group_orm> gt(db);
	gt.index(primary_key).keyValue(2).read(*grp, noKeyValueFromObj);

	/* Using pure pointer. */
	group* g = group::create(0);
	g->setId(2);
	gt.read(*g);
	delete g;

	//-------------------------------------------------------------------------
	//   Multi model operations (find operation)
	//-------------------------------------------------------------------------

	/* Set key number and key value of start record position */
	ut.index(keynum_group).keyValue(find_group_id);

	/* Create query of find */
	query q;
	q.select(_T("*")).where(_T("group"), _T("=") , find_group_id);

	/* Execute read by the query.
	   Users is vector collection .
	*/
	ut.read(users, q);

	/* Using original collection. */
	mdls m;
	ut.index(keynum_group).keyValue(find_group_id).read(m, q);

	/* When Using other map handler, use readRange method.
	*/
	users_orm users_hdr(m);
	ut.readRange(users_hdr, q);


	/* Using clrient filter exsample.
	   Set a client filter function or function object to read method.
	*/
	ut.index(keynum_group).keyValue(find_group_id).read(users, q, isMatch);


	/* Using id list reading, that like 'in' of SQL.
	   Read groups of id= 1, 2, and 3
	*/
	gt.index(primary_key);
	q.reset();
	std::vector<group_ptr> gmdls;
	q.select(_T("id"), _T("name")).in(1, 2, 3);
	gt.read(gmdls, q);


	//-------------------------------------------------------------------------
	//   Like the ActiveRecord operation has_many
	//-------------------------------------------------------------------------
	q.reset().where(_T("group"), _T("="), grp->id());
	ut.index(keynum_group).keyValue(grp->id()).read(grp->users(), q);

	//-------------------------------------------------------------------------
	//   Like the ActiveRecord operations belongs_to and has_one
	//-------------------------------------------------------------------------
	/* Read user models belongs to object of 'group', that use readEach method
	   Specify the get function pointer group model from user model.
	   Group id list is listed automatically.
	*/
	query qe;
	/* Read group name only */
	qe.select(_T("id"), _T("name"));

	gt.index(primary_key);
	gt.readEach(users, &user::grp, qe);

	/* Using the Origianl collection is same as. */
	gt.readEach(m, &user::grp, qe);

	/* Group id list is listed by yourself. */
	group_list_ptr grps(listup(users, &user::grp));
	gt.index(primary_key).readEach(*grps, qe);

	/* Using the Origianl collection is same as. */
	group_list_ptr grps2(listup(m, &user::grp));
	gt.index(primary_key).readEach(*grps2, qe);


	/* Order by */
	users.clear();
	ut.index(0).keyValue(0).read(users, q.all());
	sort(users, &user::name, &user::id);

	/* Order by for original collection */
	m.clear();
	ut.index(0).keyValue(0).read(m, q);
	sort(m, &user::name, &user::id);

	/* Using original sort function */
	std::sort(begin(m), end(m), &sortFunc2);

	/* Using for_each function */
	std::cout << "--- vector for each ---" << std::endl;
	std::for_each(users.begin(), users.end(), dumpUser<user_ptr&>);

	/* Using for_each function of original collection */
	std::cout << "--- original collection for each ---" << std::endl;
	std::for_each(begin(m), end(m), dumpUser<user*>);

	/* Using only table operation */
	table_ptr tb = ut.table();
	tb->clearOwnerName();
}


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatadaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		openDatabase(db, param);

		databaseManager mgr(db);
		std::vector<user_ptr> users;

		readUsers(mgr, users);
		std::cout << "Read records success.\nRecord count = " << users.size() << std::endl;

		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}





