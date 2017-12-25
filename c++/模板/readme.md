# 模板
## 1.函数模板

* 实际上编译器对不同类型的调用进行了展开

```
template<typename T>
T &mymax(T &a, T &b)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

int main(int argc, char const *argv[]) {
  int ia = 1, ib = 2;
  float fa = 1, fb = 2;
  double da = 1, db = 2;
  max(ia, ib);
  max(fa, fb);
  max(da, da);
  return 0;
}

$ ./a.exe
T& mymax(T&, T&) [with T = int]
T& mymax(T&, T&) [with T = float]
T& mymax(T&, T&) [with T = double]

```

* 普通函数支持隐式转换，但是函数模板不支持

```
  int ia = 1;//没有const的话类型不匹配，无法转换
  const int ib = 2;
  mymax(ia, ib);

  int a = 1;
  double b = 2.1;
  add(a, b);//ok
  mymax(a, b);


$ g++ max2.cpp
max2.cpp: In function 'int main(int, const char**)':
max2.cpp:37:15: error: no matching function for call to 'mymax(int&, const int&)'
   mymax(ia, ib);
               ^
max2.cpp:23:4: note: candidate: template<class T> T& mymax(T&, T&)
 T &mymax(T &a, T &b)
    ^
max2.cpp:23:4: note:   template argument deduction/substitution failed:
max2.cpp:37:15: note:   deduced conflicting types for parameter 'T' ('int' and 'const int')
   mymax(ia, ib);
               ^

max2.cpp:23:4: note:   template argument deduction/substitution failed:
max2.cpp:43:13: note:   deduced conflicting types for parameter 'T' ('int' and 'double')
   mymax(a, b);

```

* 只支持2种隐式类型转换：const转换  数组或函数指针转换

```
template<typename T>
const T &mymax(const T &a, const T &b)
{
  std::cout << __PRETTY_FUNCTION__ << '\n';
  return (a < b)? b : a;
}

int main(int argc, char const *argv[]) {

  int ia = 1;
  int ib = 2;
  mymax(ia, ib);//const可以向下转换，但是下不能往const转换

  return 0;
}

$ ./a.exe
const T& mymax(const T&, const T&) [with T = int]
```

```
max5.cpp

$ ./a.exe
const T& mymax(const T&, const T&) [with T = char [3]]
const T* mymax2(const T*, const T*) [with T = char]
const T* mymax2(const T*, const T*) [with T = char]
void test_func(T) [with T = int (*)(int, int)]
void test_func(T) [with T = int (*)(int, int)]
```

# 函数模板——重载
## 选择规则
* 先列出候选函数，包括普通函数、参数推到成功的模板函数
* 这些候选函数，根据“类型转换”来排序（可以并列）（注意：模板函数只支持有限的类型转换）
* 如果某个候选函数的参数，跟调用时传入的参数更匹配，则选择此候选函数
* 如果这些函数的参数匹配相同
*** 如果只有一个非模板函数，优先选择普通函数
*** 如果只有模板函数，则选择“更特化”的——更具体
*** 否则，最后导致“二义性”