#ifndef _CHINESE_H_
#define _CHINESE_H_

#include "iostream"
#include "string.h"
#include "unistd.h"
#include "Human.h"

using namespace std;

class Chinese:public Human{
public:
  void eating(void);
  void wearing(void);
  void driving(void);
  virtual ~Chinese();
};

#endif
