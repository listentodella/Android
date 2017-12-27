/*线程安全*/
#include "iostream"
#include "string"
#include "RefBase.h"


using namespace std;
using namespace android::RSC;

class Person:public LightRefBase<Person>{
public:
  Person()
  {
    cout << "Person()" << '\n';
  }

  ~Person()
  {
    cout << "~Person()" << '\n';
  }

  void printfinfo(void)
  {
    cout << "just a test func" << endl;
  }
};

template<typename T>
void test_func(sp<T> &other)
{
  sp<T> s = other;
  cout<<"In call test_func: "<<other->getStrongCount()<<endl;
  s->printfinfo();
  /*
    尽管这里使用了new,但是通过sp这种方法创建局部变量的方式
    在函数调用结束时会自动调用sp的析构函数，释放指针
    相当于：
    Person *p = new Person();
    p->printfinfo();
    delete p;
   */
}

int main()
{
/*
  *少用 Person *,用"sp"来代替
  *Person *per;
  *有2种才作：per->xxx or (*per).xxx
  *那么sp也应该有这2种操作
  *sp->xxx or (*sp).xxx
 */
  sp<Person> other = new Person();
  (*other).printfinfo();
  cout<<"before call test_func: "<<other->getStrongCount()<<endl;
  for(int i = 0; i < 2; i++){
    test_func(other);
    cout<<"after call test_func: "<<other->getStrongCount()<<endl;
  }
  return 0;
}
