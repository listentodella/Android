#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Furniture{
private:
  int weight;
public:
  void setWeight(int weight){this->weight = weight;}
  int getWeight(void)const {return weight;}
  Furniture(){std::cout << "Furniture()" << '\n';}
};

class Vertification3C{
public:
  Vertification3C(){std::cout << "Vertification3C()" << '\n';}
};

class Sofa:virtual public Furniture, virtual public Vertification3C{
private:
  int a;
public:
  void watchTV(void){cout << "WatchTV" << '\n';}
  Sofa(){std::cout << "Sofa()" << '\n';}
};

class Bed:virtual public Furniture, virtual public Vertification3C{
private:
  int b;
public:
  void sleep(void){cout << "sleep" << '\n';}
  Bed(){std::cout << "Bed()" << '\n';}
};

class Sofabed:public Sofa, public Bed{
private:
  int c;
public:
  Sofabed(){std::cout << "Sofabed()" << '\n';}
  Sofabed(char *abc){std::cout << "Sofabed(char *abc)" << '\n';}
};

class Date{
public:
  Date(){std::cout << "Date()" << '\n';}
  Date(char *abc){std::cout << "Date(char *abc)" << '\n';}
};
class Type{
public:
  Type(){std::cout << "Type()" << '\n';}
  Type(char *abc){std::cout << "Type(char *abc)" << '\n';}
};

class LeftRightCom{
public:
  LeftRightCom(){std::cout << "LeftRightCom()" << '\n';}
  LeftRightCom(char *abc){std::cout << "LeftRightCom(char *abc)" << '\n';}
};

class LeftRightSofabed:public Sofabed,virtual public LeftRightCom{
private:
  Date data;
  Type type;
public:
  LeftRightSofabed(){std::cout << "LeftRightSofabed()" << '\n';}
  LeftRightSofabed(char *str1, char *str2, char *str3):Sofabed(str1),LeftRightCom(str2),date(str3)
  {std::cout << "LeftRightSofabed(char *str1, char *str2, char *str3)" << '\n';}
};



int main(int argc, char const *argv[]) {
  LeftRightSofabed s("abc", "2343", "yyy");

  return 0;
}
