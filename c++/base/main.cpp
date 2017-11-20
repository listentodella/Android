#include "iostream"
#include "string"
#include "person.h"

using namespace std;
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
