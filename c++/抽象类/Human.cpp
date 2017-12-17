#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Human{
private:
  int a;
public:
  virtual void eating(void) = 0;
  virtual void wearing(void) = 0;
  virtual void driving(void) = 0;
  virtual ~Human(){std::cout << "~Human" << '\n';}
  virtual Human* test(void){std::cout << "Human's test" << '\n';return this;}
};

class Englishman:public Human{
public:
  void eating(void){std::cout << "use knife to eat" << '\n';}
  void wearing(void){std::cout << "wearing English style" << '\n';}
  void driving(void){std::cout << "driving English style" << '\n';}
  virtual ~Englishman(){std::cout << "~Englishman" << '\n';}
  virtual Englishman* test(void){std::cout << "Englishman's test" << '\n';return this;}
};

class Chinese:public Human{
public:
  void eating(void){std::cout << "use chopsticks to eat" << '\n';}
  void wearing(void){std::cout << "wearing Chinese style" << '\n';}
  void driving(void){std::cout << "driving Chinese style" << '\n';}
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
  // Human h;抽象类不能实例化对象
  Englishman e;
  Chinese c;
  return 0;
}
