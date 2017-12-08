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
* protected 外界不可见，不能直接访问；子类可以访问
* public 外界可以直接访问
