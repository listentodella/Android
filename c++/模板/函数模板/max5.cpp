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
const T &mymax(const T &a, const T &b)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

template<typename T>
const T *mymax2(const T *a, const T *b)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

template<typename T>
void test_func(T f)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
}

int f1(int a, int b)
{
  return 0;
}

int main(int argc, char const *argv[]) {
  char a[] = "ab";
  char b[] = "cd";
  mymax(a, b);
  mymax2(a, b);

  char a2[] = "abc";
  char b2[] = "cd";
  // mymax(a2, b2);//mymax(char[4], char[3],无法推导出T:mymax(char& [4], char& [3]))
  mymax2(a2, b2);//mymax2(char[4], char[3]),推导:mymax2(const char *, const char *)

  test_func(f1);
  test_func(&f1);
  return 0;
}
