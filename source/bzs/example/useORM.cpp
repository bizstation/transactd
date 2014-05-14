#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <vector>
#include <bzs/db/protocol/tdap/client/trdormapi.h>
#include <boost/iterator/iterator_facade.hpp>


using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief Read records and manual O/R mapping example

This program read records of the "user" table where group = 3.
And O/R mapping to the user class.
This program use the filter operation on server side.

Please execute "create database" , "change schema" and "insert records" example
	before execute this example.

*/


static const char_td keynum_group = 1;
static const char_td primary_key = 0;

class group
{
	int m_id;
	std::string m_name;
protected:
	group(void* owner):m_id(0){}
public:

	int id() const {return m_id;}
	const std::string& name() const {return m_name;}
	void setId(int v){m_id = v;}
	void setName(const char* v){m_name = v;}
	static group* create(void* owner){return new group(owner);};


};

typedef boost::shared_ptr<group> group_ptr;
typedef std::vector<group_ptr> group_ptr_list;
typedef std::vector<group*> group_list;
typedef boost::shared_ptr<group_list> group_list_ptr;



group* create(group_ptr_list& m, int){return group::create(&m);}

class group_fdi
{

public:
	short id;
	short name;

	//table*を引数にとるinit関数がなければならない
	void init(table* tb)
	{
		id = tb->fieldNumByName(_T("id"));
		name = tb->fieldNumByName(_T("name"));
	}
	//インスタンスを返すstatic な create関数がなければならない
	//static group_fdi* create(){return new group_fdi();};
};

group_fdi* createFdi(group_fdi *){return new group_fdi();}
void destroyFdi(group_fdi * p){delete p;}
void initFdi(group_fdi * fdi, table* tb){fdi->init(tb);}

class group_orm
{
	const group_fdi& m_fdi;

public:

	/*  mdl_typenameを定義しなければならない*/
	typedef group       mdl_typename;

	/*  fdi_typenameを定義しなければならない
		fdiを使用しない場合はデフォルトのtypedef fdibase fdi_typename; とすること
		ことなるFDIを使用する場合はactiveTable<group, group_fdi>のように
		activeTableのインスタンス作成時にfdiの型を指定する */
	typedef group_fdi   fdi_typename;

	/* fdi_typename fdi&を引数に取るコンストラクタが必須 */
	group_orm(const fdi_typename& fdi):m_fdi(fdi){}

	/* ユニークキーしか呼ばれないないのでその他のキー分は定義しなくてよい*/
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

	/* クラス名からテーブル名を引くための関数getTableNameが必須 */
	const _TCHAR* getTableName(){return _T("group1");}



};

/** User class */
class user
{
	mutable group* m_grp;
	int m_id;
	std::string m_name;
	std::string m_tel;
protected:
	user(void* owner):m_grp(NULL),m_id(0){}
public:
	int count;  //test for group


	group* grp()const
	{
		if (m_grp == NULL)
			m_grp = group::create(0);
		return m_grp;
	}
	int id() const {return m_id;}
	const std::string& name() const {return m_name;}
	const std::string& tel() const {return m_tel;}
	void setId(int v){m_id = v;}
	void setName(const char* v){m_name = v;}
	void setTel(const char* v){m_tel = v;}

	//コレクション操作で必須の関数　ownerにはコレクションのポインタが渡されます。

	static user* create(void* owner){return new user(owner);};


};

typedef boost::shared_ptr<user> user_ptr;
typedef std::vector<user_ptr> user_ptr_list;
typedef std::vector<user*> user_list;
typedef boost::shared_ptr<user_list> user_list_ptr;

/* イテレータをつくるのは面倒だけれども、さまざまなアルゴリズムを使うことを考えると
	作成するのがベター
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
	size_t distance_to(const mdlsIterator &r)const;
	bool equal(const mdlsIterator &r) const;

public:
	mdlsIterator(mdls* m, int index=0);


};





class mdls
{
	mutable std::vector<user*> m_users;
public:
	void clear(){m_users.clear();}
	user* add(user* u)
	{
		m_users.push_back(u);
		return u;
	}
	user*& item(int index)const {return m_users[index];}
	//user** item_ptr(int index)const {return &(m_users[index]);}
	/** オリジナルコレクションの場合は以下の３つの関数と
		アイテムの格納型を示すitem_typeを実装する
		関数の追加が困難な場合はadapterを作ってください。
	*/

	size_t size(){return m_users.size();}
	user* operator[](unsigned int index)const{return item(index);}

	/*
	以下の２つのtypedefを加える必要がある
	typedefはインタフェースに影響しないので、依存コードの再コンパイルは不要です
	*/
	typedef user* item_type;  //保持する型を指定できるが vectorで生の場合は指定する方法がない
	typedef mdlsIterator iterator;//必須
	/*
		push_back() begin() end()は(インタフェースの変更を伴うのが難しい場合)
		実装してもしなくても良い。しない場合は bzs::db::protocol::tdap::client名前空間に
		特殊化した
		template <>
		mdlsIterator push_back(mdls& m);
		mdlsIterator begin(mdls& m);
		mdlsIterator end(mdls& m);
		の３つの関数を作成する。
	*/
	//void push_back(user* u){add(u);}
	//mdlsIterator begin(){return mdlsIterator(*this, 0);}
	//mdlsIterator end(){return mdlsIterator(*this, size());}

	//for grouping
	typedef int key_type;
	typedef user* row_type;



};

void dumpUser2(const user* user)
{
	std::cout << " id           " << user->id()    << std::endl;
	std::cout << " name         " << user->name()  << std::endl;
	std::cout << " group        " << user->grp()->name() << std::endl;
	std::cout << " tel          " << user->tel()   << std::endl << std::endl;

}


user*& mdlsIterator::dereference() const
{
	return m_mdls->item(m_index);
}
void mdlsIterator::increment() {++m_index;}
void mdlsIterator::decrement() {--m_index;}
void mdlsIterator::advance(size_t n){m_index+=n;}
size_t mdlsIterator::distance_to(const mdlsIterator &r)const{return r.m_index - m_index;}
bool mdlsIterator::equal(const mdlsIterator &r) const {return m_index == r.m_index;}
mdlsIterator::mdlsIterator(mdls* m, int index):m_index(index),m_mdls(m){}



//オリジナルコレクションにpush_back begin end　を実装しない場合は以下の名前
//空間にそれぞれの関数を作成する
namespace bzs{namespace db{namespace protocol{namespace tdap{namespace client
{

inline mdlsIterator begin(mdls& m){return mdlsIterator(&m, 0);}

inline mdlsIterator end(mdls& m){return mdlsIterator(&m, m.size());}

inline void push_back(mdls& m, user* u){m.add(u);}

/* for grouping */
inline void clear(mdls& m)
{
	m.clear();
}

/* for grouping */
inline int resolvKeyValue(mdls&, const std::_tstring& name
		, bool noexception=false)
{
	if (name == _T("grp")) return 1;
	if (name == _T("count")) return 2;
	return 0;
}

const int key_grp = 1;
const int key_count = 2;


int compByKey(const user& l, const user& r, const int& s)
{

	if (s == key_grp)
		return l.grp()->id() -  r.grp()->id();
	if (s == key_count)
		return l.count -  r.count;
	return 0;
}

void setValue(user& u, const int& resultKey, const int& v)
{
	if (resultKey == key_count)
		u.count = v;
}


}}}}}


user* create(mdls& m, int){return user::create(&m);}
user* create(user_ptr_list& m, int){return user::create(&m);}



class user_fdi //: public fdibase
{

public:
	short id;
	short name;
	short tel;
	short group;
	void init(table* tb)
	{
		id = tb->fieldNumByName(_T("id"));
		name = tb->fieldNumByName(_T("name"));
		tel = tb->fieldNumByName(_T("tel"));
		group = tb->fieldNumByName(_T("group"));
	}
};

user_fdi* createFdi(user_fdi *){return new user_fdi();}
void destroyFdi(user_fdi * p){delete p;}
void initFdi(user_fdi * fdi, table* tb){fdi->init(tb);}


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
		fds[m_fdi.group] = u.grp()->id();
	}

	void readMap(user& u, const fields& fds, int optipn)
	{
		u.setId(fds[m_fdi.id].i());
		u.setName(fds[m_fdi.name].a_str()); //get by ansi string
		u.setTel(fds[m_fdi.tel].a_str());  //get by ansi string
		u.grp()->setId(fds[m_fdi.group].i());
	}

	void readAuntoincValue(user& u, const fields& fds, int optipn)
	{
		u.setId(fds[m_fdi.id].i());
	}

	/* クラス名からテーブル名を引くための関数getTableNameが必須 */
	const _TCHAR* getTableName(){return _T("user");}

	typedef user        mdl_typename;
	typedef user_fdi    fdi_typename;

	/* オリジナルコレクションをつかうときに定義する
	   最初の型はこのマップクラス、2番目の型はコレクションクラス
	   これが定義されていてもvector<shared_ptr<T>>のコレクションの
	   も同時に使えます。
	*/
	typedef mdlsHandler<user_orm, mdls>   collection_orm_typename;

};

typedef user_orm::collection_orm_typename users_orm;


int isMatch(const fields& fds)
{
	return filter_validate_value;
}

/** dump user to screen */
void dumpUser(const user_ptr& user)
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


class count
{
	int m_value;
	int m_resultKey;
public:
	count():m_value(0){}

	void setResultKey(int key)
	{
		m_resultKey = key;
	}

	void operator()(const user& u)
	{
		++m_value;
	}
	int result()const{return m_value;}

	void reset(){m_value = 0;}

};



void readUsers(databaseManager& db, std::vector<user_ptr>& users)
{
	int id = 12;
	int find_group_id = 3;
	bool readMyKeyValue = false;


	// user_ormのactiveTableのインスタンスを作成します。
	activeTable<user_orm> ut(db);

	//id=12のユーザーを作成します。
	user_ptr u(user::create(0));
	u->setId(id);
	u->setName("moriwaki");
	u->setTel("81-999-9999");
	u->grp()->setId(1);
	ut.index(primary_key);
	ut.save(*u);

	//id=12のユーザーを読み取り
	ut.read(*u);

	//id=12のユーザーの電話番号の変更
	u->setTel("81-999-8888");
	ut.update(*u);

	//キー値を変えるときは
	ut.index(primary_key).keyValue(u->id());
	u->setId(13);
	ut.update(*u, readMyKeyValue);

	//id=13のユーザー削除
	ut.del(*u);

	//削除はキー値だけでもOK
	ut.index(primary_key).keyValue(12).del(); //エラー　レコードなし



	//カーソルのindexとキー位置を指定します。
	//ここからレコードの検索を開始します。
	ut.index(keynum_group).keyValue(find_group_id);

	//検索条件を指定します。サーバーフィルターです。
	//rejectで指定したアンマッチレコード数になると検索を中止します。
	query q;
	q.select(_T("*")).where(_T("group"), _T("=") , find_group_id)//.or(_T("group"), _T("=") , _T("a"))
			.reject(1);

	//読み取りを実行。　結果を受け取るコレクションとクエリーを渡します。
	ut.read(users, q);

	/*
	前回のreadsで使用したコレクションはvectorでした。しかしオリジナルの
	コレクションを使用している場合もあるでしょう
	user_ormにオリジナルコレクションマップの型collection_orm_typenameを
	知らせておけば自動でインスタンスを作成しハンドルしてくれます*/

	mdls m;
	ut.index(keynum_group).keyValue(find_group_id).read(m, q);

	//グルーピング グループごとに属する人数を数える
	/*groupQuery gq;
	gq.keyField(_T("grp")).resultField(_T("count"));
	gq.grouping(m, count());
	*/

	/*
	オリジナルコレクションマップを自動で作成でなく自分で初期化して
	使いたいこともあるでしょうその時はreadsByを使用します*/

	users_orm users_hdr(m);
	ut.readRange(users_hdr, q);


	/* クライアント側フィルターも簡単に使えます。
		isMatch関数のような　const fields&を引数に取ってintを返す関数なら何でも
		OKです。
	*/
	ut.index(keynum_group).keyValue(find_group_id).read(users, q, isMatch);



	//groupの読み取り
	group_ptr grp(group::create(0));

	activeTable<group_orm> gt(db);
	//shared_ptr<group>のインスタンスを*をつけて渡します。
	gt.index(0).keyValue(2).read(*grp);



	/*
	生ポインタのポインタの時も *をつけて渡します。
	shared_ptrでも生ポインタでも*をつけて同じように渡せます。
	*/
	group* g = group::create(0);
	gt.read(*g);
	delete g;


	//--------------------------------------------------------
	//  Joinのような処理
	//--------------------------------------------------------
	/*
	usersからグループのリストを作成してuserに関連付けたグループ
	を読み取ります。
	list関数にコレクションとuser->grp()関数のアドレスを渡します。
	activeTable<group_orm>のreadEach関数にそのリストを渡します。
	*/
	query qe;
	group_list_ptr grps(listup(users, &user::grp));
	gt.index(primary_key).readEach(*grps, qe.select(_T("id"), _T("name")));


	//オリジナルのグループリストでもlistup関数は使えます。
	group_list_ptr grps2(listup(m, &user::grp));
	gt.index(primary_key).readEach(*grps2, qe);


	//listup処理を内包して自動で行う
	gt.index(primary_key);
	gt.readEach(users, &user::grp, qe.select(_T("id"), _T("name")));
	gt.readEach(m, &user::grp, qe);

	//IN
	gt.index(primary_key);
	qe.reset();
	std::vector<group_ptr> gmdls;
	gt.read(gmdls, qe.select(_T("id"), _T("name")).in(1, 2, 3));

	//orderby
	users.clear();
	ut.index(0).keyValue(0).read(users, q.all());
	sort(users, &user::name, &user::id);

	//groupby
	//ある値を取り出して

	//ソート対応のイテレータは大変です。
	m.clear();
	ut.index(0).keyValue(0).read(m, q);

	sort(m, &user::name);
	std::sort(begin(m), end(m), &sortFunc2);

	std::for_each(begin(m), end(m), dumpUser2);

	//テーブルだけの操作
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

		//dump users to screen.
		std::for_each(users.begin(), users.end(), dumpUser);

		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}





