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

    //table*�������ɂƂ�init�֐����Ȃ���΂Ȃ�Ȃ�
    void init(table* tb)
    {
        id = tb->fieldNumByName(_T("id"));
        name = tb->fieldNumByName(_T("name"));
    }
    //�C���X�^���X��Ԃ�static �� create�֐����Ȃ���΂Ȃ�Ȃ�
    //static group_fdi* create(){return new group_fdi();};
};

group_fdi* createFdi(group_fdi *){return new group_fdi();}
void destroyFdi(group_fdi * p){delete p;}
void initFdi(group_fdi * fdi, table* tb){fdi->init(tb);}

class group_orm
{
    const group_fdi& m_fdi;

public:

    /*  mdl_typename���`���Ȃ���΂Ȃ�Ȃ�*/
    typedef group       mdl_typename;

    /*  fdi_typename���`���Ȃ���΂Ȃ�Ȃ�
        fdi���g�p���Ȃ��ꍇ�̓f�t�H���g��typedef fdibase fdi_typename; �Ƃ��邱��
        ���ƂȂ�FDI���g�p����ꍇ��activeTable<group, group_fdi>�̂悤��
        activeTable�̃C���X�^���X�쐬����fdi�̌^���w�肷�� */
    typedef group_fdi   fdi_typename;

    /* fdi_typename fdi&�������Ɏ��R���X�g���N�^���K�{ */
    group_orm(const fdi_typename& fdi):m_fdi(fdi){}

    /* ���j�[�N�L�[�����Ă΂�Ȃ��Ȃ��̂ł��̑��̃L�[���͒�`���Ȃ��Ă悢*/
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

    /* �N���X������e�[�u�������������߂̊֐�getTableName���K�{ */
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

    //�R���N�V��������ŕK�{�̊֐��@owner�ɂ̓R���N�V�����̃|�C���^���n����܂��B

    static user* create(void* owner){return new user(owner);};


};

typedef boost::shared_ptr<user> user_ptr;
typedef std::vector<user_ptr> user_ptr_list;
typedef std::vector<user*> user_list;
typedef boost::shared_ptr<user_list> user_list_ptr;

/* �C�e���[�^������͖̂ʓ|������ǂ��A���܂��܂ȃA���S���Y�����g�����Ƃ��l�����
    �쐬����̂��x�^�[
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
    /** �I���W�i���R���N�V�����̏ꍇ�͈ȉ��̂R�̊֐���
        �A�C�e���̊i�[�^������item_type����������
        �֐��̒ǉ�������ȏꍇ��adapter������Ă��������B
    */

    size_t size(){return m_users.size();}
    user* operator[](unsigned int index)const{return item(index);}

    /*
    �ȉ��̂Q��typedef��������K�v������
    typedef�̓C���^�t�F�[�X�ɉe�����Ȃ��̂ŁA�ˑ��R�[�h�̍ăR���p�C���͕s�v�ł�
    */
    typedef user* item_type;  //�ێ�����^���w��ł��邪 vector�Ő��̏ꍇ�͎w�肷����@���Ȃ�
    typedef mdlsIterator iterator;//�K�{
    /*
        push_back() begin() end()��(�C���^�t�F�[�X�̕ύX�𔺂��̂�����ꍇ)
        �������Ă����Ȃ��Ă��ǂ��B���Ȃ��ꍇ�� bzs::db::protocol::tdap::client���O��Ԃ�
        ���ꉻ����
        template <>
        mdlsIterator push_back(mdls& m);
        mdlsIterator begin(mdls& m);
        mdlsIterator end(mdls& m);
        �̂R�̊֐����쐬����B
    */
    //void push_back(user* u){add(u);}
    //mdlsIterator begin(){return mdlsIterator(*this, 0);}
    //mdlsIterator end(){return mdlsIterator(*this, size());}

};

/* mdlsIterator�̎��� */
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



//�I���W�i���R���N�V������push_back begin end�@���������Ȃ��ꍇ�͈ȉ��̖��O
//��Ԃɂ��ꂼ��̊֐����쐬����
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

    /* �N���X������e�[�u�������������߂̊֐�getTableName���K�{ */
    const _TCHAR* getTableName(){return _T("user");}

    typedef user        mdl_typename;
    typedef user_fdi    fdi_typename;

    /* �I���W�i���R���N�V�����������Ƃ��ɒ�`����
       �ŏ��̌^�͂��̃}�b�v�N���X�A2�Ԗڂ̌^�̓R���N�V�����N���X
       ���ꂪ��`����Ă��Ă�vector<shared_ptr<T>>�̃R���N�V������
       �������Ɏg���܂��B
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

    // user_orm��activeTable�̃C���X�^���X���쐬���܂��B
    activeTable<user_orm> ut(db);

    //id=12�̃��[�U�[���쐬���܂��B
    user_ptr u(user::create(0));
    u->setId(id);
    u->setName("moriwaki");
    u->setTel("81-999-9999");
    u->grp()->setId(1);
    ut.save(*u);

    //id=12�̃��[�U�[��ǂݎ��
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.read(*u);

    //id=12�̃��[�U�[�̓d�b�ԍ��̕ύX
    u->setTel("81-999-8888");
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.update(*u);

    //id=12�̃��[�U�[�폜
    ut.cursor().index(primary_key).keyValue(u->id());
    ut.del();


    //�J�[�\����index�ƃL�[�ʒu���w�肵�܂��B
    //�������烌�R�[�h�̌������J�n���܂��B
    ut.cursor().index(keynum_group).keyValue(find_group_id);

    //�����������w�肵�܂��B�T�[�o�[�t�B���^�[�ł��B
    //reject�Ŏw�肵���A���}�b�`���R�[�h���ɂȂ�ƌ����𒆎~���܂��B
    query q;
        q.select(_T("*"))
            .where(_T("group"), _T("=") , find_group_id)//.or(_T("group"), _T("=") , _T("a"))
            .reject(1);

    //�ǂݎ������s�B�@���ʂ��󂯎��R���N�V�����ƃN�G���[��n���܂��B
    ut.reads(users, q);


    /*
    �O���reads�Ŏg�p�����R���N�V������vector�ł����B�������I���W�i����
    �R���N�V�������g�p���Ă���ꍇ������ł��傤
    user_orm�ɃI���W�i���R���N�V�����}�b�v�̌^collection_orm_typename��
    �m�点�Ă����Ύ����ŃC���X�^���X���쐬���n���h�����Ă���܂�*/

    mdls m;
    ut.cursor().index(keynum_group).keyValue(find_group_id);
    ut.reads(m, q);

    /*
    �I���W�i���R���N�V�����}�b�v�������ō쐬�łȂ������ŏ���������
    �g���������Ƃ�����ł��傤���̎���readsBy���g�p���܂�*/

    users_orm users_hdr(m);
    ut.readsBy(users_hdr, q);


    /* �N���C�A���g���t�B���^�[���ȒP�Ɏg���܂��B
        isMatch�֐��̂悤�ȁ@const fields&�������Ɏ����int��Ԃ��֐��Ȃ牽�ł�
        OK�ł��B
    */
    ut.cursor().index(keynum_group).keyValue(find_group_id);
    ut.reads(users, q, isMatch);


    //group�̓ǂݎ��
    group_ptr grp(group::create(0));

    activeTable<group_orm> gt(db);
    gt.cursor().index(0).keyValue(2);

    //shared_ptr<group>�̃C���X�^���X��*�����ēn���܂��B
    gt.read(*grp);

    /*
    ���|�C���^�̃|�C���^�̎��� *�����ēn���܂��B
    shared_ptr�ł����|�C���^�ł�*�����ē����悤�ɓn���܂��B
    */
    group* g = group::create(0);
    gt.read(*g);
    delete g;


    //--------------------------------------------------------
    //  Join�̂悤�ȏ���
    //--------------------------------------------------------
    /*
    users����O���[�v�̃��X�g���쐬����user�Ɋ֘A�t�����O���[�v
    ��ǂݎ��܂��B
    list�֐��ɃR���N�V������user->grp()�֐��̃A�h���X��n���܂��B
    activeTable<group_orm>��readEach�֐��ɂ��̃��X�g��n���܂��B
    */
    group_list_ptr grps(listup(users, &user::grp));
    gt.cursor().index(primary_key);
    query qe;
    qe.select(_T("id"), _T("name"));
    gt.readEach(*grps, qe);

    //�I���W�i���̃O���[�v���X�g�ł�listup�֐��͎g���܂��B
    group_list_ptr grps2(listup(m, &user::grp));
    gt.cursor().index(primary_key);
    gt.readEach(*grps2, qe);

    //listup���������Ď����ōs��
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

    //�\�[�g�Ή��̃C�e���[�^�͑�ςł��B
    m.clear();
    ut.cursor().index(0).keyValue(0);
    ut.reads(m, q);
    sort(m, &user::name);
    std::sort(begin(m), end(m), &sortFunc2);

    std::for_each(begin(m), end(m), dumpUser2);
    //to xml
    //����̓N���X�̋@�\�Ȃ̂ŃN���X���f�R���[�g����H


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





