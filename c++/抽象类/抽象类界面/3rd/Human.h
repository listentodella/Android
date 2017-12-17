#ifndef _HUMAN_H_
#define _HUMAN_H_

#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Human{
private:
  string name;
public:
  void setName(string name);
  string getName();
};

#endif
