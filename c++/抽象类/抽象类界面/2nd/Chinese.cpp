#include "Chinese.h"
void Chinese::setName(string name)
{
  this->name = name;
}

string Chinese::getName()
{
  return this->name;
}

void Chinese:: eating(void)
{
  cout << "use chopsticks to eat" << endl;
}
void Chinese:: wearing(void)
{
  cout << "wearing Chinese style" << endl;
}
void Chinese:: driving(void)
{
  cout << "driving Chinese style" << endl;
}
Chinese:: ~Chinese()
{
  cout << "~Chinese" << endl;
}
