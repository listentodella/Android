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

class Son : public Father {
private:
  int toy;
  using Father::it_skill;//私藏
  // using Father::room_key;在这里设置无法通过编译,因为子类本来就看不见
public:
  using Father::room_key;//在这里进行设置，main里就可以直接使用了
  // using Father::money;money是Father私有的，儿子无法看见
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

  /*覆写 override*/
  void it_skill(void){
    cout << "Son's skill" << '\n';
  }

};

int main()
{
  Son s;
  s.setMoney(10);
  cout<<s.getMoney()<<endl;

  s.it_skill();
  s.play_game();

  s.room_key = 1;//protected权限

  return 0;
}
