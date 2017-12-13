#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Human{
public:
  virtual void eating(void){std::cout << "use hand to eat" << '\n';}
};

class Englishman:public Human{
public:
  void eating(void){std::cout << "use knife to eat" << '\n';}
};

class Chinese:public Human{
public:
  void eating(void){std::cout << "use chopsticks to eat" << '\n';}
};

void test_eating(Human &h)
{
  h.eating();
}

int main(int argc, char const *argv[]) {
  Human h;
  Englishman e;
  Chinese c;
  /*只有virtual过才可以分清是哪个类*/
  test_eating(h);
  test_eating(e);
  test_eating(c);

  std::cout << "sizeof(Human) = "<<sizeof(h) << '\n';
  std::cout << "sizeof(Englishman) = "<<sizeof(e) << '\n';
  std::cout << "sizeof(Chinese) = "<<sizeof(c) << '\n';

  return 0;
}
