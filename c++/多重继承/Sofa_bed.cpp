#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Sofa{
public:
  void watchTV(void){cout << "WatchTV" << '\n';}

};

class Bed{
public:
  void sleep(void){cout << "sleep" << '\n';}
};

class Sofabed:public Sofa, public Bed{

};

int main(int argc, char const *argv[]) {
  Sofabed s;
  s.watchTV();
  s.sleep();
  return 0;
}
