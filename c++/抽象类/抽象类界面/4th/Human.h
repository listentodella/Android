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
  virtual void eating(void){std::cout << "use hand to eat" << '\n';}
  virtual void wearing(void){}
  virtual void driving(void){}
  // virtual void eating(void) = 0;结果同上
  // virtual void wearing(void) = 0;结果同上
  // virtual void driving(void) = 0;结果同上
};

Human &CreateEnglishman(string name, int age, string address);
Human &CreateChinese(string name, int age, string address);
#endif
