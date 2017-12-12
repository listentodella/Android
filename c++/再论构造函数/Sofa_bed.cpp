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
};

class Date{
public:
  Date(){std::cout << "Date()" << '\n';}
};
class Type{
public:
  Type(){std::cout << "Type()" << '\n';}
};

class LeftRightCom{
public:
  LeftRightCom(){std::cout << "LeftRightCom()" << '\n';}
};

class LeftRightSofabed:public Sofabed,public LeftRightCom{
private:
  Date data;
  Type type;
public:
  LeftRightSofabed(){std::cout << "LeftRightSofabed()" << '\n';}
};



int main(int argc, char const *argv[]) {
  LeftRightSofabed s;

  return 0;
}
