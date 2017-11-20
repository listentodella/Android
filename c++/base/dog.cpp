#include "iostream"
#include "string"
#include "dog.h"

using namespace std;
namespace B {
  void Dog::setName(string name)
  {
    this->name = name;
  }
  void Dog::printfinfo(void)
  {
    cout << "name = " << name << ", age = " << age <<endl;
  }

  int Dog::setAge(int age)
  {
      if(age < 0 || age > 20) {
        this->age = 0;
        std::cout << "invalid age" << '\n';
        return -1;
      } else {
        this->age = age;
        return 0;
      }
  }

void printVersion()
{
  std::cout << "dog_1.1" << endl;
}
}
