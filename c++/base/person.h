#include "iostream"
#include "string"

using namespace std;
namespace A {
class Person{
private:
  // char *name;
  string name;
  int age;
  string work;

public:
  void setName(string name);
  int setAge(int age);
  // {
  //   if(age < 0 || age > 150) {
  //     this->age = 0;
  //     std::cout << "invalid age" << '\n';
  //     return -1;
  //   } else {
  //     this->age = age;
  //     return 0;
  //   }
  // }
  void printfinfo(void);
  void setWork(string work);
};

void printVersion();
}
