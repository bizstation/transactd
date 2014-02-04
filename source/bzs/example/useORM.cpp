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



group* create(group_ptr_list& m){return group::create(&m);}

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
/*class mdls;
class mdlsIterator : public std::iterator<std::random_access_iterator_tag, user*>
{
    int m_index;
    mdls& m_mdls;
public:
    //mdlsIterator():m_mdls(*((mdls*)0)){};
    mdlsIterator(mdls& m, int index=0);
    //mdlsIterator& operator=(const mdlsIterator &r) ;
    user* operator*() const;
    mdlsIterator &operator++();
    bool operator!=(const mdlsIterator &r) const;
    //mdlsIterator& operator-(const mdlsIterator &r);
    //mdlsIterator& operator+(const mdlsIterator &r);
};*/

class mdls;

/*user* mdlsIterator::operator*() const{return m_mdls[m_index];}
mdlsIterator& mdlsIterator::operator++() {++m_index; return *this;}
bool mdlsIterator::operator!=(const mdlsIterator &r) const {return m_index != r.m_index;}
*/


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

};

/* mdlsIteratorの実装 */
/*mdlsIterator::mdlsIterator(mdls& m, int index):m_index(index),m_mdls(m){};
user* mdlsIterator::operator*() const{return m_mdls[m_index];}
mdlsIterator& mdlsIterator::operator++() {++m_index; return *this;}
bool mdlsIterator::operator!=(const mdlsIterator &r) const {return m_index != r.m_index;}
*/
void dumpUser2(const user* user)
{
    std::cout << " id           " << user->id()    << std::endl;
    std::cout << " name         " << user->name()  << std::endl;
    std::cout << " group        " << user->grp()->name() << std::endl;
    std::cout << " tel          " << user->tel()   << std::endl << std::endl;

}


user*& mdlsIterator::dereference() const
{
    //for (int i=0;i<m_mdls->size();++i)
    //    dumpUser2((*m_mdls)[i]);

    return m_mdls->item(m_index);
    //return p;
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
}}}}}


user* create(mdls& m){return user::create(&m);}
user* create(user_ptr_list& m){return user::create(&m);}



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

    //static user_fdi* create(){return new user_fdi();}
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

void readUsers(databaseManager& db, std::vector<user_ptr>& users)
{
    int id = 12;
    int find_group_id = 3;

    // user_ormのactiveTableのインスタンスを作成します。
    activeTable<user_orm> ut(db);

    //id=12のユーザーを作成します。
    user_ptr u(user::create(0));
    u->setId(id);
    u->setName("moriwaki");
    u->setTel("81-999-9999");
    u->grp()->setId(1);
    ut.save(*u);

    //id=12のユーザーを読み取り
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.read(*u);

    //id=12のユーザーの電話番号の変更
    u->setTel("81-999-8888");
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.update(*u);

    //id=12のユーザー削除
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.del();


    //カーソルのindexとキー位置を指定します。
    //ここからレコードの検索を開始します。
    ut.cursor().index(keynum_group).keyValue(find_group_id);

    //検索条件を指定します。サーバーフィルターです。
    //rejectで指定したアンマッチレコード数になると検索を中止します。
    query q;
        q.select(_T("*"))
            .where(_T("group"), _T("=") , find_group_id)//.or(_T("group"), _T("=") , _T("a"))
            .reject(1);

    //読み取りを実行。　結果を受け取るコレクションとクエリーを渡します。
    ut.reads(users, q);


    /*
    前回のreadsで使用したコレクションはvectorでした。しかしオリジナルの
    コレクションを使用している場合もあるでしょう
    user_ormにオリジナルコレクションマップの型collection_orm_typenameを
    知らせておけば自動でインスタンスを作成しハンドルしてくれます*/

    mdls m;
    ut.cursor().index(keynum_group).keyValue(find_group_id);
    ut.reads(m, q);

    /*
    オリジナルコレクションマップを自動で作成でなく自分で初期化して
    使いたいこともあるでしょうその時はreadsByを使用します*/

    users_orm users_hdr(m);
    ut.readsBy(users_hdr, q);


    /* クライアント側フィルターも簡単に使えます。
        isMatch関数のような　const fields&を引数に取ってintを返す関数なら何でも
        OKです。
    */
    ut.cursor().index(keynum_group).keyValue(find_group_id);
    ut.reads(users, q, isMatch);


    //groupの読み取り
    group_ptr grp(group::create(0));

    activeTable<group_orm> gt(db);
    gt.cursor().index(0).keyValue(2);

    //shared_ptr<group>のインスタンスを*をつけて渡します。
    gt.read(*grp);

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
    group_list_ptr grps(listup(users, &user::grp));
    gt.cursor().index(primary_key);
    query qe;
    qe.select(_T("id"), _T("name"));
    gt.readEach(*grps, qe);

    //オリジナルのグループリストでもlistup関数は使えます。
    group_list_ptr grps2(listup(m, &user::grp));
    gt.cursor().index(primary_key);
    gt.readEach(*grps2, qe);

    //listup処理を内包して自動で行う
    gt.cursor().index(primary_key);
    qe.select(_T("id"), _T("name"));
    gt.readEach(users, &user::grp, qe);
    gt.readEach(m, &user::grp, qe);

    //IN
    gt.cursor().index(primary_key);
    qe.reset();
    qe.select(_T("id"), _T("name")).in(1, 2, 3);
    std::vector<group_ptr> gmdls;
    gt.reads(gmdls, qe);

    //orderby
    users.clear();
    ut.cursor().index(0).keyValue(0);
    q.all();
    ut.reads(users, q);
    sort(users, &user::name, &user::id);

    //ソート対応のイテレータは大変です。
    m.clear();
    ut.cursor().index(0).keyValue(0);
    ut.reads(m, q);
    sort(m, &user::name);
    std::sort(begin(m), end(m), &sortFunc2);

    std::for_each(begin(m), end(m), dumpUser2);
    //to xml
    //これはクラスの機能なのでクラスをデコレートする？


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





