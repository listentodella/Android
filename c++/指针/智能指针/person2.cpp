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
  //Person *p = new Person();
  Person per;
  per.printfinfo();
  //局部变量在函数结束调用后会自动释放
}

int main()
{
  test_func();
  return 0;
}
