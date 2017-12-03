#include "iostream"
#include "string.h"
#include "unistd.h"

using namespace std;

class Point {
private:
  int x;
  int y;
public:
  Point() {}
  Point(int x, int y) : x(x), y(y) {}

  void setX(int x){this->x = x;}
  void setY(int y){this->y = y;}
  int getX(){return x;}
  int getY(){return y;}

  void printInfo()
  {
    cout <<"("<<x<<","<<y<<")" << endl;
  }
  friend Point add2(Point &p1, Point &p2);
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

int main(int argc, char const *argv[])
{
  Point p1(1, 2);
  Point p2(2, 3);

  Point sum = add(p1, p2);
  Point sum2 = add2(p1, p2);
  return 0;
}
