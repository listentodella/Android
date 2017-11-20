#include "iostream"
#include "string"

using namespace std;
class Person{
private:
  // char *name;
  string name;
  int age;
  string work;

public:
  void setName(string name)
  {
    // name = n;
    this->name = name;
  }
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
  void printfinfo(void)
  {
    cout << "name = " << name << ", age = " << age << ", work = " << work;
  }
  void setWork(string work)
  {
    this->work = work;
  }
};

int Person::setAge(int age)
{
    if(age < 0 || age > 150) {
      this->age = 0;
      std::cout << "invalid age" << '\n';
      return -1;
    } else {
      this->age = age;
      return 0;
    }
}

int main()
{
  Person p1;
  // p1.name = "zhangsan";
  p1.setName("zhangsan");
  p1.setAge(151);
  p1.setWork("teacher");
  p1.printfinfo();
  return 0;
}
