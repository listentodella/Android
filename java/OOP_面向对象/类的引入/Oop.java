
class Person {
  //类本身的属性，不需要实例化对象就可以获取
  static int count;

  String name;
  int age;
  String getName() {
    return name;
  }
  //构造块，一定会被调用，调用的时机与位置有关
  //每实例化一个对象前都会执行，先于构造方法
  {
    count++;
  }
  //静态块，实例化第一个对象前执行并且只执行一次，先于构造块
  static {
    System.out.println("static block");
  }
//构造方法，连返回值类型都不能写
  public Person(String n) {
    name = n;
  }

  public Person() {
    name = "null";
    age = 0;
  }

  public Person(String n, int a) {
    name = n;
    age = a;
  }
//防止混淆？this关键字，当前对象、当前类
  public Person(String name, int age) {
    this.name = name;
    this.age = age;
  }
//类方法，调用的时候不需要实例化对象
  static void  printPerson() {
    System.out.println("This is a class of Person");
  }

}


public class Oop {
  //从上面可以看出，main方法属于类方法，不需要实例化对象就可以调用
  public static void main(String[] args) {
    Person p1 = new Person("zhangsan");
    Person p2 = new Person("lisi");
    Person p3 = new Person();
    Person p4 = new Person("wangwu", 10);

    System.out.println(p1.getName());
    System.out.println(p2.getName());

    Person.printPerson();

  }
}
