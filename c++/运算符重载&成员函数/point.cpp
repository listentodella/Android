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

  Point add2(Point &p1, Point &p2)
  {
    Point n;
    n.x = (p1.x + p2.x);
    n.y = (p1.y + p2.y);

    return n;
  }

/**
 * 可以写作 p.operator???(...),因此可以放在类内
 */

  Point operator+(Point &p)
  {
    cout << "operator +" << endl;
    Point n;
    n.x = this->x + p.x;
    n.y = this->y + p.y;
    return n;
  }

  /*
   *由于函数重载是靠参数区别的，因此2种++只能通过改变参数来区别
   */

  /*Point p(1, 2); ++p;*/
  // Point operator++(Point &p)
  // 从log里可以看出，返回引用效率更高
  Point& operator++(void)
  {
    cout << "++p" << '\n';
    this->x += 1;
    this->y += 1;
    return *this;
  }
  /*Point p(1, 2); p++;*/
  Point operator++(int a)
  {
    cout << "p++" << '\n';
    Point n;
    n = *this;
    this->x += 1;
    this->y += 1;
    return n;
  }

  /**
   * 无法写作 p.operator<<(...)，因此只能放在类外
   */

  friend ostream& operator<<(ostream &o, Point p);
};

Point add(Point &p1, Point &p2)
{
  Point n;
  n.setX(p1.getX() + p2.getX());
  n.setY(p1.getY() + p2.getY());

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
  Point m, n;

  m = p1 + p2;//m = p1.operator+(p2);
  cout <<"add p1, p2 = "<<m<<endl;
  cout <<"begin"<<endl;
  m = ++p1;//m = p1.operator++();
  cout<<"m = "<<m<<" "<<"p1 = "<<p1<<endl;
  cout << "****************" << '\n';

  n = p1++;//m = p1.operator++(0);
  cout<<"n = "<<n<<" "<<"p1 = "<<p1<<endl;
  cout <<"end"<<endl;

  return 0;
}
