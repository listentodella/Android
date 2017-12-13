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
  // Englishman &pe = dynamic_cast<Englishman &>(h);编译可以通过，但是运行会异常
  Chinese &pc = dynamic_cast<Chinese &>(h);
  Guangximan &pg = dynamic_cast<Guangximan &>(h);
  h.eating();
}

int main(int argc, char const *argv[]) {
  Human h;
  // Englishman e;
  // Chinese c;
  Guangximan g;

  Englishman *pe;
  /*下行转换*/
  pe = static_cast<Englishman *>(&h);//有安全隐患，因为human未必是Englishman
  // Englishman *pe2 = static_cast<Englishman *>(&g);//Guangximan肯定不能转换成Englishman，因此编译无法通过
  Chinese *pc = static_cast<Chinese *>(&g);

  return 0;
}
