/*T, N这些符号是自定义的*/
interface Person<T> {
  public T setAge(T age);
  public T getAge();
}

/*都使用泛型*/
class Student<T> implements Person<T> {
  T age;
  public T setAge(T age) {
    this.age = age;
  }
  public T getAge() {
    return this.age;
  }
}
/*指定类型*/
class Student2 implements Person<Integer> {
  String age;
  public String setAge(String age) {
    this.age = age;
  }
  public String getAge() {
    return this.age;
  }
}

public class InterfaceGenerics {
  public static void main(String[] args) {

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
