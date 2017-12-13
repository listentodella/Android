#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Human{
public:
  virtual void eating(void){std::cout << "use hand to eat" << '\n';}
  virtual ~Human(){std::cout << "~Human" << '\n';}
  virtual Human* test(void){std::cout << "Human's test" << '\n';return this;}
};

class Englishman:public Human{
public:
  void eating(void){std::cout << "use knife to eat" << '\n';}
  virtual ~Englishman(){std::cout << "~Englishman" << '\n';}
  virtual Englishman* test(void){std::cout << "Englishman's test" << '\n';return this;}
};

class Chinese:public Human{
public:
  void eating(void){std::cout << "use chopsticks to eat" << '\n';}
  virtual ~Chinese(){std::cout << "~Chinese" << '\n';}
  virtual Chinese* test(void){std::cout << "Chinese's test" << '\n';return this;}
};

void test_eating(Human &h)
{
  h.eating();
}

void test_return(Human &h)
{
  h.test();
}

int main(int argc, char const *argv[]) {
  Human h;
  Englishman e;
  Chinese c;
  test_return(h);
  test_return(e);
  test_return(c);
  return 0;
}
