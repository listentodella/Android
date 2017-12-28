/*线程安全(仅仅对于引用计数罢了)*/
#include "iostream"
#include "string"
#include <utils/RefBase.h>


using namespace std;
using namespace android;

class Person:public LightRefBase<Person>{
private:
  sp<Person> father;
  sp<Person> son;
public:
  Person()
  {
    cout << "Person()" << '\n';
  }

  ~Person()
  {
    cout << "~Person()" << '\n';
  }

  void setFather(sp<Person> &father)
  {
    this->father = father;
  }
  void setSon(sp<Person> &son)
  {
    this->son = son;
  }

  void printfinfo(void)
  {
    cout << "just a test func" << endl;
  }
};


void test_func()
{
  sp<Person> father = new Person();
  sp<Person> son = new Person();
  father->setSon(son);
  son->setFather(father);
}

int main()
{

  test_func();
  return 0;
}
