# C++
## 编译器 g++

## 命名空间namespace
``避免冲突，但实际上功能有冲突的还是得加上以区分``
```
using namespace std;

namespace A {
  /*所有 A部分的代码*/
  class 类a{
    FUN();
  }
  func();
}

namespace B {
  /*所有B部分的代码*/
  class 类{
    FUN();
  }
  func();
}
```
### 调用方法1:
```
int main()
{
  A:: func();
  B:: func();
}

```
### 调用方法2:全局命名空间 or 局部命名空间
```
using namespace A::其中的某个类;
using namespace B::其中的某个类;
int main()
{
  //using namespace A::其中的某个类;
  //using namespace B::其中的某个类;
  A.FUN();
  B.FUN();

  A::func();
  B::func();
}
```
## 权限
* private
* protected
* public


## 函数的重载
* 参数类型不同


## 指针
* 与C语言完全一样
```
int add(int *a)
{
  *a = *a + 1;
  return *a;
}
```

## 引用
* 引用必须在定义的时候就初始化，而且只能是变量(或者对象)，不能是哪个具体的值
```
int add(int &b)
{
  b = b + 1;
  return b;
}

int a = 100;
int &b = a;
b 就是 a 的引用
  可以把b看作a的别名，指向的是同一片内存

```
