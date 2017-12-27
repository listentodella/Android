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

void test_func(void)
{
  Person *p = new Person();
  p->printfinfo();
  //没有释放，会导致内存泄露
}

int main()
{
  test_func();
  return 0;
}
