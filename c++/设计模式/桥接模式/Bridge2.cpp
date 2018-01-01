#include "iostream"

using namespace std;

class OS {
public:
  virtual void InstallOS_impl() = 0;

};

class LinuxOS:public OS{
public:
  virtual void InstallOS_impl(){std::cout << "Install Linux OS" << '\n';}
};

class WindowsOS:public OS{
public:
  virtual void InstallOS_impl(){std::cout << "Install Windows OS" << '\n';}
};

class UnixOS:public OS{
public:
  virtual void InstallOS_impl(){std::cout << "Install Unix OS" << '\n';}
};

class Computer{
public:
  virtual void printInfo() = 0;
  virtual void InstallOS() = 0;
};

class Mac:public Computer{
public:
  virtual void printInfo(){std::cout << "This is Mac," << '\n';}
  Mac(OS *impl){this->impl = impl;}
  void InstallOS(){printInfo();impl->InstallOS_impl();}
private:
  OS *impl;
};

class Lenovo:public Computer{
public:
  virtual void printInfo(){std::cout << "This is Lenovo," << '\n';}
  Lenovo(OS *impl){this->impl = impl;}
  void InstallOS(){printInfo();impl->InstallOS_impl();}
private:
  OS *impl;
};

int main()
{
  OS *os1 = new LinuxOS();
  OS *os2 = new WindowsOS();
  OS *os3 = new UnixOS();

  Computer *c1 = new Mac(os1);
  Computer *c2 = new Lenovo(os2);
  Computer *c3 = new Lenovo(os3);

  c1->InstallOS();
  c2->InstallOS();
  c3->InstallOS();
  
  return 0;
}
