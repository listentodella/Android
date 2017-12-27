/*引用计数操作*/
#include "iostream"
#include "string"

using namespace std;
class Person{
private:
  int count;
public:
  void incStrong(){count++;}
  void decStrong(){count--;}
  int getStrongCount(){return count;}
  Person():count(0)
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

class sp{
private:
  Person *p;
public:
  sp():p(0){}
  sp(Person *other)
  {
    std::cout << "sp(Person *other)" << '\n';
    p = other;
    p->incStrong();
  }
  sp(const sp &other)
  {
    std::cout << "sp(const sp &other)" << '\n';
    p = other.p;
    p->incStrong();
  }
  ~sp()
  {
    std::cout << "~sp()" << '\n';
    if(p){
      p->decStrong();
      if(p->getStrongCount() == 0){
        delete p;
        p = NULL;
      }
    }

  }

  Person *operator->()//因为p为private,所以对于外部是不能直接使用的，可以通过重载->
  {
    return p;
  }
};

void test_func(sp &other)
{
  sp s = other;
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
  /*相当于
   * 1. Person *p = new Person();
   * 2. sp tmp(p);==>sp(Person *other)
   * 3.1  sp other(tmp);==>sp(sp &other2);
   *  问题在于，sp &other2 = tmp;错误语法，因为引用不能是临时变量
   *            const sp& other2 = tmp;//ok
   *  or
   * 3.2  sp other(tmp==>Person *);==>sp(Person *other)
   */
  sp other = new Person();
  cout<<"before call test_func: "<<other->getStrongCount()<<endl;
  for(int i = 0; i < 2; i++){
    test_func(other);
    cout<<"after call test_func: "<<other->getStrongCount()<<endl;
  }
  return 0;
}
