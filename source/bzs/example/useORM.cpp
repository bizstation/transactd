#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <vector>
#include <bzs/db/protocol/tdap/client/trdormapi.h>

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

static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;
static const char_td keynum_group = 1;



class group_fdi : public fdibase
{

public:
    short id;
    short name;
    void resolv(table* tb)
    {
        id = tb->fieldNumByName(_T("id"));
        name = tb->fieldNumByName(_T("name"));
    }
};



/** Group class */
class group;
const _TCHAR* getTableName(group* )
{
    return _T("group1");
}

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

typedef boost::function<void (group& , const fields&, int )> group_map_functor;

fdibase* createFdi(group* )
{
    return new group_fdi();
}


class user;
const _TCHAR* getTableName(user* )
{
    return _T("user");
}



class user_fdi : public fdibase
{

public:
    short id;
    short name;
    void resolv(table* tb)
    {
        id = tb->fieldNumByName(_T("id"));
        name = tb->fieldNumByName(_T("name"));
    }
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

    static user* create(void* owner){return new user(owner);};

};

typedef boost::shared_ptr<user> user_ptr;
typedef boost::function<void (user& , const fields&, int )> user_map_functor;
fdibase* createFdi(user* )
{
    return new user_fdi();
}


/** dump user to screen */
void dumpUser(const user_ptr& user)
{
    std::cout << " id           " << user->id()    << std::endl;
    std::cout << " name         " << user->name()  << std::endl;
    std::cout << " group        " << user->grp()->name() << std::endl;
    std::cout << " tel          " << user->tel()   << std::endl << std::endl;

}

user& trdReadormap(user& u, const fields& fds, int)
{
    u.setId(fds[fieldnum_id].i());
    u.setName(fds[fieldnum_name].a_str()); //get by ansi string
    u.setTel(fds[fieldnum_tel].a_str());  //get by ansi string
    u.grp()->setId(fds[fieldnum_group].i());
    return u;
}

void trdWriteormap(const fields& fds , user& u, int)
{
    fds[fieldnum_id] = u.id();
    fds[fieldnum_name] = u.name();
    fds[fieldnum_tel] = u.tel();
    fds[fieldnum_group] = u.grp()->id();
}


group& trdReadormap(group& g, const fields& fds , int)
{
    g.setId(fds[fieldnum_id].i());
    g.setName(fds[fieldnum_name].a_str()); //get by ansi string
    return g;
}

void trdWriteormap(const fields& fds , group& g, int)
{
    fds[fieldnum_id] = g.id();
    fds[fieldnum_name] = g.name();
}

void trdReadormapg(group& g, const fields& fds , int)
{
    g.setId(fds[fieldnum_id].i());
    g.setName(fds[fieldnum_name].a_str()); //get by ansi string
}

/** OR mapping functional object*/
class groupMappper
{

public:
    groupMappper(){}
    void operator()(group& grp, const fields& fds, int optipn)
    {
       trdReadormap(grp , fds, optipn);
    }
};

//独自のコレクション

class mdls
{
public:
    user* add(){return user::create(0);};

};

class cstmMdlsMapper
{
    mdls& m_mdls;
    int m_optipn;
public:
    cstmMdlsMapper(mdls& m, int option)
        :m_mdls(m),m_optipn(option){}
    void operator()(const fields& fds)
    {
        user* u = m_mdls.add();
        trdReadormap(*u, fds, m_optipn);
    }
};

int isMatch(const fields& fds)
{
    return filter_validate_value;
}

void readUsers(databaseManager& db, std::vector<user_ptr>& users)
{
    int maxid = 3;

    // userのactiveTableのインスタンスを作成します。
    activeTable<user> ut(db);

    //カーソルのindexとキー位置を指定します。
    //ここからレコードの検索を開始します。
    ut.cursor().index(keynum_group).position(maxid);

    //検索条件を指定します。サーバーフィルターです。
    //rejectで指定したアンマッチレコード数になると検索を中止します。
    query q;
        q.select(_T("*"))
            .where(_T("group"), _T("=") , maxid)//.or(_T("group"), _T("=") , _T("a"))
            .reject(1);

    //読み取りを実行。　結果を受け取るコレクションとクエリーを渡します。
    ut.reads(users, q);


    //前回のreadsで使用したコレクションはvectorでした。しかしオリジナルの
    //コレクションを使用している場合もあるでしょう
    //ループで使うハンドラを規定の型でつくれば簡単に取り替えできます。

    mdls m;
    ut.cursor().index(keynum_group).position(maxid);
    ut.reads<mdls, cstmMdlsMapper>(m, q);

    //クライアント側フィルターも簡単に使えます。
    ut.cursor().index(keynum_group).position(maxid);
    ut.reads(users, q, isMatch);



    // userの復元
    ut.cursor().index(0).position(2);
    user_ptr u(user::create(0));
    ut.read(*u);

    // group の復元
    group_ptr grp(group::create(0));

    activeTable<group> gt(db);
    gt.cursor().index(0).position(2);

    //デフォルトマップ関数 + shared_ptr
    gt.read(*grp);

    //独自のマップファンクタ+ 生ポインタ
    group_map_functor f = groupMappper();
    group* g = group::create(0);
    gt.readfunc(&f).read(*g);

    //関数ポインタもOK
    f = trdReadormapg;
    gt.readfunc(&f).read(*grp);

    //デフォルトマップ関数に戻してよみとり
    gt.readfunc((group_map_functor*)NULL).read(*grp);

    //グループ名を変更
    grp->setName("group3");
    gt.cursor().index(0).position(2);
    gt.update(*grp);

    //削除
    gt.cursor().index(0).position(2);
    //gt.del();

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





