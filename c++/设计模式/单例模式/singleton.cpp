#include "iostream"

using namespace std;

class Singleton;

Singleton *gInstance;

class Singleton{
public:
  static Singleton *getInstance()
  {
    if(gInstance == NULL)
      gInstance == new Singleton;
    return gInstance;
  }

  Singleton()
  {
    std::cout << "Singleton()" << '\n';
  }

  void printInfo(){std::cout << "This is Singleton" << '\n';}

};

int main()
{
  Singleton *s1 = Singleton::getInstance();
  s1->printInfo();

  Singleton *s2 = Singleton::getInstance();
  s2->printInfo();

  Singleton *s3 = Singleton::getInstance();
  s3->printInfo();

  return 0;
}
