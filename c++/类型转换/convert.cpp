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

class Guangximan:public Chinese{
public:
  void eating(void){std::cout << "use chopsticks to eat, From Guangxi" << '\n';}
};

void test_eating(Human &h)
{
  Englishman *pe;
  Chinese *pc;
  Guangximan *pg;
  h.eating();
  /*想分辨这个人到底是哪国人*/
  if(pe = dynamic_cast<Englishman *>(&h))
    std::cout << "This human is Englishman" << '\n';

  if(pc = dynamic_cast<Chinese *>(&h))
    std::cout << "This human is Chinese" << '\n';

  if(pg = dynamic_cast<Guangximan *>(&h))
    std::cout << "This human is Guangximan" << '\n';
}

int main(int argc, char const *argv[]) {
  // Human h;
  // Englishman e;
  // Chinese c;
  Guangximan g;
  // test_eating(h);
  // test_eating(e);
  // test_eating(c);
  test_eating(g);
  return 0;
}
