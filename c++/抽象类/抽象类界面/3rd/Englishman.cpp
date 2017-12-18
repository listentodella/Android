#include "Englishman.h"

void Englishman::eating(void)
{
  cout << "use knife to eat" << endl;
}
void Englishman::wearing(void)
{
  cout << "wearing English style" << endl;
}
void Englishman::driving(void)
{
  cout << "driving English style" << endl;
}
Englishman::~Englishman()
{
  cout << "~Englishman" << endl;
}
Englishman::Englishman(){}
Englishman::Englishman(string name, int age, string address)
{
  setName(name);
  this->age = age;
  this->address = address;
}
