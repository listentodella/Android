/*T, N这些符号是自定义的*/
class Person<T> {
  private T age;
  public T setAge(T age) {
    this.age = age;
  }

  public T getAge() {
    return this.age;
  }
}

/*都使用泛型*/
class Student<T> extends Person<T> {

}
/*指定类型*/
class Student2 extends Person<Integer> {

}

public class Generics2 {
  public static void main(String[] args) {
    Person<String> p = new Person<String>();
    p.setAge("3 years old");
    // System.out.println(p.getAge());
    printInfo(p);
/*泛型的T只能用类*/
    Person<Integer> p2 = new Person<Integer>();
    p.setAge(3);
    // System.out.println(p2.getAge());
    printInfo(p2);

    Person<?> p3;
    p3 = p;
    // p3.setAge("4 years"); 使用?通配符时不能传输特定的参数
    p3.getAge();

    printInfo2(p);
    printInfo2(p2);
    printInfo2(p3);


    Student<Integer> s = new Student<Integer>();
    s.setAge(10);
    printInfo(s);

    Student2 s2 = new Student2();
    s2.setAge("11 years old");
    printInfo(s2);

  }

  public static void printInfo(Person<?> p) {
    System.out.println(p.getAge());
  }

  public static <T> void printInfo2(Person<T> p) {
    System.out.println(p.getAge());
  }


}
