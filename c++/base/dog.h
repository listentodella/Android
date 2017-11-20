#include "iostream"
#include "string"

using namespace std;
namespace B {
class Dog{
private:
  string name;
  int age;

public:
  void setName(string name);
  int setAge(int age);
  void printfinfo(void);
  void setWork(string work);
};

void printVersion();
}
