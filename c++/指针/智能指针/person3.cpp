#include "iostream"
#include "string"

using namespace std;
class Person{
public:
  Person()
  {
    cout << "void constuct called" << '\n';
  }

  ~Person()
  {}

  void printfinfo(void)
  {
    cout << "just a test func" << endl;
  }
};

class sp{
private:
  Person *p;
public:
  sp():p(0){}
  sp(Person *other)
  {
    std::cout << "sp(Person *other)" << '\n';
    p = other;
  }
  ~sp()
  {
    std::cout << "~sp()" << '\n';
    if(p)
      delete p;
  }

  Person *operator->()//因为p为private,所以对于外部是不能直接使用的，可以通过重载->
  {
    return p;
  }
};

void test_func(void)
{
  sp s = new Person();
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
  test_func();
  return 0;
}
