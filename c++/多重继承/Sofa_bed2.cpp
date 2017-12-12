#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Sofa{
private:
  int weight;
public:
  void watchTV(void){cout << "WatchTV" << '\n';}
  void setWeight(int weight){this->weight = weight;}
  int getWeight(void)const {return weight;}
};

class Bed{
private:
  int weight;
public:
  void sleep(void){cout << "sleep" << '\n';}
  void setWeight(int weight){this->weight = weight;}
  int getWeight(void)const {return weight;}
};

class Sofabed:public Sofa, public Bed{

};

int main(int argc, char const *argv[]) {
  Sofabed s;
  s.watchTV();
  s.sleep();

  // s.setWeight(100); error,二义性
  s.Sofa::setWeight(100);//ok，指定基类

  return 0;
}
