# 类型转换
## 隐式类型转换
```
double d = 100.1;
int i = d; //double 转换为 int

char *str = "listentodella";
int *p = str; //char * 转换为 int *
```

## 显示类型转换
### C风格的强制转换
```
char *str = "listentodella";
int *p = (int *)str; //强转
```
### reinterpret_cast<T>(expression)
* T必须是一个指针、引用、算术类型、函数指针或成员指针
* 它可以把一个指针转换成一个整数，也可以把一个整数转换为一个指针
* 跟C的强制类型转换相似，没有安全性检查
```
char *str = "listentodella";
int *p = reinterpret_cast<int *>(str);//与上面一样
```
### const_cast<T>(expression)

* 该运算符用来去除原来类型的const或者volatile属性
* 除了const或volatile修饰之外，T和expression的
```
const char *str = "listentodella";
char *str2 = const_cast<char *>(str);
int *p = reinterpret_cast<int *>(str2);//这样就不会有const相关的警告了
```
### dynamic_cast<T>(expression)
* 该运算符必须是类的指针、类的引用或者 void *
* 如果T是类指针类型，那么expression也必须是一个指针
* 如果T是一个引用，那么expression也必须是一个引用(如果转换失败就崩溃了，毕竟指针转换失败还可以置NULL，引用则不行）

1. 用于多态场合，即：必须有虚函数
2. 主要用于类层次间的上行转换和下行转换，还可以用于类之间的交叉转换
3. 在类层次间进行上行转换时，dynamic_cast和static_cast的效果是一样的；在进行下行转换时，dynamic_cast具有类型检查的功能，比static_cast

```
只能用在含有虚函数的类
void test_eating(Human &h)
{
  Englishman *pe;
  Chinese *pc;
  h.eating();
  /*想分辨这个人到底是哪国人*/
  if(pe = dynamic_cast<Englishman *>(&h))
    std::cout << "This human is Englishman" << '\n';
  if(pc = dynamic_cast<Chinese *>(&h))
    std::cout << "This human is Chinese" << '\n';
  if(pg = dynamic_cast<Guangximan *>(&h))
    std::cout << "This human is Guangximan" << '\n';
}

$ ./a.exe
use chopsticks to eat, From Guangxi
This human is Chinese
This human is Guangximan
~Chinese 
~Human

```
![附加信息](%E9%99%84%E5%8A%A0%E4%BF%A1%E6%81%AF.png)

### static_cast<T>(expression)
* 用于类层次结构中基类和子类之间指针或引用的转换
* 进行上行转换（把子类的指针或引用转换成基类表示）是安全的
* 进行下行转换（把基类的指针或引用转换成子类指针或引用）时，由于没有动态类型检查，所以是不安全的
* 用于基本数据类型之间的转换，如把int转换成char，把int转换成enum：这种转换的安全性也要由开发人员来把控
* 把void指针转换成目标类型的指针（不安全！！！）
* 把任何类型的表达式转换成void类型
```
  /*下行转换，不进行检查*/
  pe = static_cast<Englishman *>(&h);//有安全隐患，因为human未必是Englishman

	/*上行转换，会进行检查，因此是安全的*/
  Englishman *pe2 = static_cast<Englishman *>(&g);//Guangximan肯定不能转换成Englishman，因此编译无法通过
  Chinese *pc = static_cast<Chinese *>(&g);
```
#### 注意：static_cast不能转换掉expression的const volatile 或者__unaligned属性