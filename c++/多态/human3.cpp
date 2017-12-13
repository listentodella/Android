#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Human{
public:
  virtual void eating(void){std::cout << "use hand to eat" << '\n';}
  virtual ~Human(){std::cout << "~Human" << '\n';}
};

class Englishman:public Human{
public:
  void eating(void){std::cout << "use knife to eat" << '\n';}
  virtual ~Englishman(){std::cout << "~Englishman" << '\n';}
};

class Chinese:public Human{
public:
  void eating(void){std::cout << "use chopsticks to eat" << '\n';}
  virtual ~Chinese(){std::cout << "~Chinese" << '\n';}
};

void test_eating(Human &h)
{
  h.eating();
}

int main(int argc, char const *argv[]) {
  Human *h = new Human();
  Englishman *e = new Englishman();
  Chinese *c = new Chinese();

  Human *p[3] = {h, e, c};
  for(int i = 0; i < 3; i++){
    p[i]->eating();
    delete p[i];
  }

  return 0;
}
