# 面向对象三大特性
## 1.封装性：把属性、方法封装为一个整体,还有权限的限制
* public 本类可访问
* default 本包可访问
* protected 本包、其他包的子类可访问
* public 所有

## 2.继承性：复用代码
    class 子类 extends 父类 {

    }

* final 类不能有子类
* final 方法不能被复写
* final 变量变成了常量，不能被修改

### 继承的限制：
* 子类无法直接操作父类的private
* 子类无法使用父类的私有方法
* 子类不能私吞祖传的方法——即仍可以继续继承

### 抽象方法：不能实例化对象，子类必须复写所有抽象方法
    abstract class 类名 {
      属性
      普通方法 {

      }

      //抽象方法，只需要声明，不需要实现
      访问权限 abstract 返回值类型 方法名(参数);
    }

### 接口，跟抽象类相似，起“模板”作用，子类可以继承多个接口，突破“单继承”的限制
    interface

## 3.多态性

* 向上转换
      父类只能调用被子类复写的方法，除非子类没去复写
      父类不能调用子类独有的方法


* 向下转换
      *在进行对象的向下转换前，必须首先发生对象的向上转换
      Father f = new Son();
      Son son = (Son)f;

      *需要使用  对象 instanceof 类 进行判断 对象是否是类的实例化
      	该对象与类如果是亲缘关系便可以认为是实例化
