#include "Englishman.h"

void Englishman::setName(string name)
{
  this->name = name;
}

string Englishman::getName()
{
  return this->name;
}

void Englishman:: eating(void)
{
  cout << "use knife to eat" << endl;
}
void Englishman:: wearing(void)
{
  cout << "wearing English style" << endl;
}
void Englishman:: driving(void)
{
  cout << "driving English style" << endl;
}
Englishman:: ~Englishman()
{
  cout << "~Englishman" << endl;
}
