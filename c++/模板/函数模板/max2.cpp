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

void add(int a, int b)
{
  std::cout << "add(int a, int b) = "<< a+b << '\n';
}

int main(int argc, char const *argv[]) {
   // int ia = 1;没有const的话类型不匹配，无法转换
  const int ia = 1;
  const int ib = 2;
  mymax(ia, ib);

  int a = 1;
  double b = 2.1;
  add(a, b);
  // mymax(a, b);
  return 0;
}
