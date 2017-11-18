# 反射
### 可以将反射类比于反编译，通过对象获取类的特性

## JVM →加载 所有class →加载进内存创建class object

内存里有 class object
* 包
* 类名称
* 构造方法
* 方法
* 属性

可以用它们来实例化对象或者多个对象
      Person p1 = new Person();

***
## 获取类
* 1.通过类的名称  
      Classc1 = Class.forName("Employee");  
* 2.java中每个类型都有class属性  
      Classc2 = Employee.class;  

* 3.java语言中任何一个java对象都有getClass 方法  
      Employeee = new Employee();  
      Classc3 = e.getClass(); //c3是运行时类 (e的运行时类是Employee)  

## 创建对象
* 获取类以后我们来创建它的对象，利用newInstance：
      Class c =Class.forName("Employee");  
      //创建此Class 对象所表示的类的一个新实例  
      Objecto = c.newInstance(); //调用了Employee的无参数构造方法.  

## 获取方法、构造方法

## 获取属性
* 所有的属性
* 指定的属性-会打破封装

## 可以将包当作配置文件作"更换"，使得程序更加灵活
