#include "iostream"

using namespace std;

class OS {
public:
  virtual void Install() = 0;

};

class LinuxOS:public OS{
public:
  virtual void Install(){std::cout << "Install Linux OS" << '\n';}
};

class WindowsOS:public OS{
public:
  virtual void Install(){std::cout << "Install Windows OS" << '\n';}
};

class Computer{
public:
  virtual void printInfo() = 0;
};

class Mac:public Computer{
public:
  virtual void printInfo(){std::cout << "This is Mac," << '\n';}
};

class MacWithLinux:public Mac, public LinuxOS{
public:
  void InstallOS()
  {
    printInfo();
    Install();
  }
};

class MacWithWindows:public Mac, public WindowsOS{
public:
  void InstallOS()
  {
    printInfo();
    Install();
  }
};

class Lenovo:public Computer{
public:
  virtual void printInfo(){std::cout << "This is Lenovo," << '\n';}
};

class LenovoWithLinux:public Lenovo, public LinuxOS{
public:
  void InstallOS()
  {
    printInfo();
    Install();
  }
};

class LenovoWithWindows:public Lenovo, public WindowsOS{
public:
  void InstallOS()
  {
    printInfo();
    Install();
  }
};

int main()
{
  MacWithLinux a;
  a.InstallOS();

  LenovoWithWindows b;
  b.InstallOS();

  return 0;
}
