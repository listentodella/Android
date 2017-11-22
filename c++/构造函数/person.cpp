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
  Person()
  {
    cout << "void constuct called" << '\n';
  }
  Person(string name)
  {
    this->name = name;
  }
  Person(string name, int age, string work = "none")
  {
    this->name = name;
    this->age = age;
  }
  ~Person(string name, int age, string work = "none")
  {
    if(this->name)
      delete(this->name);
    if(this->age)
      delete(this->age);
  }
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

  Person p2("lisi", 16);

  Person p3;//调用无参构造函数
  Person p4();//相当于函数声明
  return 0;
}
