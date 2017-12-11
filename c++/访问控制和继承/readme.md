# 继承
```
class A {
  ...
};

class B : public A {

}
这样B就继承了A，可以使用A提供的接口、成员(当然会有权限的限制)

```
* private 外界不可见，不可直接访问
* protected 外界不可见，不能直接访问；派生类可以访问
* public 外界可以直接访问

# 访问控制
* 派生类继承来的财产，他可以捐给别人，也可以私藏————可以提升权限或降低
```
只要是派生类能够“看得见”的成员，可以通过在自己的权限下使用
using Father::xxx;
使得继承来的财产的权限发生改变
```
# 继承方式的区别

| 继承类型 \ 基类访问属性 | public| protected| private|
|:--------:|:---------:|:---------:|:-------:| 
| public| public| protected|隔离| 
| protected| protected| protected |隔离|
| private|private|private|隔离|

* 无论使用哪种继承方式，在派生类内部使用父类时并无差异
* 不通的继承方式，会影响这两方面：外部代码对派生类的使用、派生类的派生类

# 派生类对象的空间分布
```
class A {
	...
public:
	func();
};
class B:public A{
	...
public:
	func();
};
test_func(A &a)
{
	...
	a.func();
}
int main()
{
	A a(...);
	B b(...);
	
	test_func(a);
	test_func(b);/*传递的参数：A &a = b里面的A部分，即a引用的是b继承到的A部分,则使用的函数也是A的*/
	b.func();//b自己的
}

```














