#ifndef _CHINESE_H_
#define _CHINESE_H_

#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Chinese{
private:
  string name;
public:
  void setName(string name);
  string getName();
  void eating(void);
  void wearing(void);
  void driving(void);
  ~Chinese();
};

#endif
