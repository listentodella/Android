#ifndef _ENGLISHMAN_H_
#define _ENGLISHMAN_H_

#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Englishman{
private:
  string name;
public:
  void setName(string name);
  string getName();
  void eating(void);
  void wearing(void);
  void driving(void);
  ~Englishman();
};

#endif
