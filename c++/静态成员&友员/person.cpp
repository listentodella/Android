#include "iostream"
#include "string"

using namespace std;
class Person{
private:
  // char *name;
  string name;
  int age;
  static int cnt;//静态成员声明
  string work;

public:
  static int getCount(void)
  {
    // cout << name;静态成员函数不能访问非静态成员变量
    return cnt;
  }
  Person()
  {
    cnt++;
    cout << "void constuct called" << '\n';
  }
  Person(string name)
  {
    cnt++;
    this->name = name;
  }
  Person(string name, int age, string work = "none")
  {
    cnt++;
    this->name = name;
    this->age = age;
  }
  ~Person(string name, int age, string work = "none")
  {
    cnt--;
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

int Person::cnt = 0;//定义和初始化，不需要再写static了

int main()
{
  Person per1;
  Person per2;
  Person per3;
  Person per4;

  Person *per5 = new Person[10];

  Person::getCountcnt;

  return 0;
}
