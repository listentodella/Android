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
  std::cout<<"1:" << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

template<typename T>
const T &mymax(T &a, T &b)
{
  std::cout<<"2:" << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

const int &mymax(int &a, int &b)
{
  std::cout<<"3:" << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

template<typename T>
const T mymax(T *a, T *b)
{
  std::cout<<"4:" << __PRETTY_FUNCTION__ << '\n';
  return (*a < *b)? *b : *a;
}

char *mymax(char *a, char *b)
{
  std::cout<<"5:" << __PRETTY_FUNCTION__ << '\n';
  return strcmp(a, b) < 0? b : a;
}

int main(int argc, char const *argv[]) {
  int ia = 1;
  int ib = 2;

  std::cout << mymax(ia, ib) << '\n';

  int *p1 = &ia;
  int *p2 = &ib;
  std::cout << mymax(p1, p2) << '\n';

  char *str1 = "hello";
  char *str2 = "Hellod";
  std::cout << mymax(str1, str2) << '\n';


  return 0;
}
