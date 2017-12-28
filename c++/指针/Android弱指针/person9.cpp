/*线程安全(仅仅对于引用计数罢了)*/
#include "iostream"
#include "string"
#include "RefBase.h"


using namespace std;
using namespace android::RSC;

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

/**
 * 如果对象里含有其他对象成员：
 * 构造时：先构造其他对象成员，再构造对象本身
 * 析构时：顺序刚好相反
 */

void test_func()
{
  /**
   * 1.对于 new Person
   * 1.1 Person对象里的father先被构造
   * 1.2 Person对象里的son被构造
   * 1.3 Person对象本身
   *
   * 2. Person对象的指针传给"sp<Person> father"
   *    导致：sp(T* other)被调用
   *    它增加了这个Person对象的引用计数（现在此值等于1）
   */
  sp<Person> father = new Person();

  /**
   * 1.对于 new Person
   * 1.1 Person对象里的father先被构造
   * 1.2 Person对象里的son被构造
   * 1.3 Person对象本身
   *
   * 2. Person对象的指针传给"sp<Person> son"
   *    导致：sp(T* other)被调用
   *    它增加了这个Person对象的引用计数（现在此值等于1）
   */
  sp<Person> son = new Person();

  /**
   * 它是一个"="操作: this->son = son;
   * "="被重载，它会再次增加该Person对象的引用技术
   * 所以son对应的Person对象的引用计数增加为2
   */
  father->setSon(son);

  /**
   * 它是一个"="操作: this->father = father;
   * "="被重载，它会再次增加该Person对象的引用技术
   * 所以father对应的Person对象的引用计数增加为2
   */
  son->setFather(father);

  /**
   * 当test_func执行完时，father和son被析构
   * 1.先看father：
   *  ~sp():decStrong,里面会将计数值减1,所以father对应的Person的计数值等于1,还没等于0,所以没有delete
   * 2.对于son：
   *  ~sp():decStrong,里面会将计数值减1,所以son对应的Person的计数值等于1,还没等于0,所以没有delete
   */
}

int main()
{

  test_func();
  return 0;
}
