#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Point {
private:
  int x;
  int y;
public:
  Point() {cout << "  Point()" << '\n';}
  Point(int x, int y) : x(x), y(y) {cout << "  Point(int x, int y)" << '\n';}
  Point(const Point& p)
  {
    cout << "Point(const Point& p)" << '\n';
    x = p.x;
    y = p.y;
  }
  ~Point()
  {
    cout << "~Point()" << endl;
  }
  void setX(int x){this->x = x;}
  void setY(int y){this->y = y;}
  int getX(){return x;}
  int getY(){return y;}

  void printInfo()
  {
    cout <<"("<<x<<","<<y<<")" << endl;
  }
  friend Point add2(Point &p1, Point &p2);
  friend Point operator+(Point &p1, Point &p2);
  //friend Point operator++(Point &p);
  friend Point& operator++(Point &p);
  friend Point operator++(Point &p, int a);
  friend ostream& operator<<(ostream &o, Point p);
};

Point add(Point &p1, Point &p2)
{
  Point n;
  n.setX(p1.getX() + p2.getX());
  n.setY(p1.getY() + p2.getY());

  return n;
}

Point add2(Point &p1, Point &p2)
{
  Point n;
  n.x = (p1.x + p2.x);
  n.y = (p1.y + p2.y);

  return n;
}

Point operator+(Point &p1, Point &p2)
{
  cout << "operator +" << endl;
  Point n;
  n.x = (p1.x + p2.x);
  n.y = (p1.y + p2.y);

  return n;
}

/*
 *由于函数重载是靠参数区别的，因此2种++只能通过改变参数来区别
 */

/*Point p(1, 2); ++p;*/
// Point operator++(Point &p)
// 从log里可以看出，返回引用效率更高
Point& operator++(Point &p)
{
  cout << "++p" << '\n';
  p.x += 1;
  p.y += 1;
  return p;
}
/*Point p(1, 2); p++;*/
Point operator++(Point &p, int a)
{
  cout << "p++" << '\n';
  Point n;
  n = p;
  p.x += 1;
  p.y += 1;
  return n;
}

ostream& operator<<(ostream &o, Point p)
{
  cout<<"("<<p.x<<", "<<p.y<<")";
  return o;
}

int main(int argc, char const *argv[])
{
  Point p1(1, 2);
  Point p2(2, 3);

  Point sum = add(p1, p2);
  sum.printInfo();
  Point sum2 = add2(p1, p2);
  sum2.printInfo();
  Point sum3 = p1 + p2;//operator+(p1, p2)
  sum3.printInfo();

  cout << "begin" << endl;
  Point n = ++p1;//operator++(p1)
  n.printInfo();
  p1.printInfo();
  std::cout << "***************" << '\n';
  Point m = p2++;//operator++(p2, ?),这里的？可以是任意值，反正只是用来区别重载的
  m.printInfo();
  p2.printInfo();
  cout << "end" << endl;

  cout << n << endl;//operator<<(cout, p)
  cout << m << endl;

  return 0;
}
