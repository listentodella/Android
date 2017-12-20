#include "iostream"
#include "string"
#include "unistd.h"
using namespace std;
#if 0
int &max(int &a, int &b)
{
  return (a < b)? b : a;
}

double &max(double &a, double &b)
{
  return (a < b)? b : a;
}

float &max(float &a, float &b)
{
  return (a < b)? b : a;
}
#endif

template<typename T>
T &mymax(T &a, T &b)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

int main(int argc, char const *argv[]) {
  int ia = 1, ib = 2;
  float fa = 1, fb = 2;
  double da = 1, db = 2;
  mymax(ia, ib);
  mymax(fa, fb);
  mymax(da, da);
  return 0;
}
