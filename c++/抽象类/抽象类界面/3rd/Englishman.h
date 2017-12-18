#ifndef _ENGLISHMAN_H_
#define _ENGLISHMAN_H_

#include "iostream"
#include "string.h"
#include "unistd.h"
#include "Human.h"

using namespace std;

class Englishman:public Human{
private:
  int age;
  string address;
public:
  void eating(void);
  void wearing(void);
  void driving(void);
  Englishman();
  Englishman(string name, int age, string address);
  ~Englishman();
};

#endif
