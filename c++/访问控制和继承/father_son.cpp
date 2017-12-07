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

int main()
{
  Son s;
  s.setMoney(10);
  cout<<s.getMoney()<<endl;

  s.it_skill();
  s.play_game();

  return 0;
}
