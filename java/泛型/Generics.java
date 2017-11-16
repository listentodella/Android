/*T, N这些符号是自定义的*/
class Person<T, N> {
  private T age;
  private N name;
  public T setAge(T age) {
    this.age = age;
  }

  public T getAge() {
    return this.age;
  }
}

public class Generics {
  public static void main(String[] args) {
    Person<String, String> p = new Person<String, String>();
    p.setAge("3 years old");
    System.out.println(p.getAge());
/*泛型的T只能用类*/
    Person<Integer, String> p2 = new Person<Integer, String>();
    p.setAge(3);
    System.out.println(p2.getAge());
  }
}
