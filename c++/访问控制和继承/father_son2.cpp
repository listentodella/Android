#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Father {
private:
  int money;

protected:
  int room_key;

public:
  int address;
  void it_skill(void)
  {
    cout<<"father's skill"<<endl;
  }

  int getMoney(void)
  {
    return money;
  }
  void setMoney(int money)
  {
    this->money = money;
  }
};

class Son_pub : public Father {
private:
  int toy;
public:
  void play_game(void)
  {
    /**
     * money -= 1;
     * 错误：不能直接拿父类的私房钱
     * 但是可以问他要
     */
    int m = getMoney();
    m--;
    cout<<"son play game"<<endl;

    /**
     * 外人不能拿父亲的房间钥匙，儿子可以
     * @return [description]
     */
    room_key = 1;
  }
};

class Son_pro : protected Father {
private:
  int toy;
public:
  void play_game(void)
  {
    /**
     * money -= 1;
     * 错误：不能直接拿父类的私房钱
     * 但是可以问他要
     */
    int m = getMoney();
    m--;
    cout<<"son play game"<<endl;

    /**
     * 外人不能拿父亲的房间钥匙，儿子可以
     * @return [description]
     */
    room_key = 1;
  }
};

class Son_pri : private Father {
private:
  int toy;
public:
  void play_game(void)
  {
    /**
     * money -= 1;
     * 错误：不能直接拿父类的私房钱
     * 但是可以问他要
     */
    int m = getMoney();
    m--;
    cout<<"son play game"<<endl;

    /**
     * 外人不能拿父亲的房间钥匙，儿子可以
     * @return [description]
     */
    room_key = 1;
  }
};

class Grandson_pub:public Son_pub{
public:
  void test(void) {
    room_key = 1;//protected
    address = 2;//public
  }
};

class Grandson_pro:protected Son_pro{
public:
  void test(void) {
    room_key = 1;//protected
    address = 2;//protected
  }
};

class Grandson_pri:private Son_pri{
public:
  void test(void) {
    // room_key = 1;error
    // address = 2;error
  }
};


int main()
{
  Son_pub s_pub;
  Son_pro s_pro;
  Son_pri s_pri;

  s_pub.play_game();
  s_pro.play_game();
  s_pri.play_game();

  s_pub.it_skill();
  // s_pro.it_skill();protected继承，相当于把public变为protected
  // s_pri.it_skill();private继承，所有的都变为private

  Grandson_pub gs_pub;
  Grandson_pro gs_pro;

  gs_pub.address = 2;
  // gs_pro.address = 2;此时已经是protected了，不能直接访问

  return 0;
}
