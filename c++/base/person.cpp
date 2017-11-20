#include "iostream"
#include "string"
#include "person.h"

using namespace std;
namespace A {
void Person::setName(string name)
{
  // name = n;
  this->name = name;
}
void Person::printfinfo(void)
{
  cout << "name = " << name << ", age = " << age << ", work = " << work << endl;
}
void Person::setWork(string work)
{
  this->work = work;
}

int Person::setAge(int age)
{
    if(age < 0 || age > 150) {
      this->age = 0;
      std::cout << "invalid age" << endl;
      return -1;
    } else {
      this->age = age;
      return 0;
    }
}

void printVersion()
{
  std::cout << "person_1.1" << endl;
}
}
