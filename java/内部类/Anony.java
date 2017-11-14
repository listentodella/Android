
interface A {
  public void printInfo();
}

class B implements A {
  public void printInfo() {
    System.out.println("hello world");
  }
}

public class Anony {
  public static void main(String[] args) {
    // B b = new B();
    // testFunc(b);
    testFunc(new B());
    testFunc(new A() {
      public void printInfo() {
        System.out.println("HELLO WORLD");
      }
    });
  }

  public static void testFunc(A a) {
    a.printInfo();
  }
}
