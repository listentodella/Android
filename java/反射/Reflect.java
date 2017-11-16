/**
 * mkdir -p a/b/c/d
 * mv Reflect.java a/b/c/d
 * javac a/b/c/d/Reflect.java
 * java a.b.c.d.Reflect
 * ---->>>> a.b.c.d.Reflect
 */

package a.b.c.d;

class Person {
  private String name;

  void setName(String name) {
    this.name = name;
  }

  String getName() {
    return this.name = name;
  }
}

public class Reflect {
  public static void main(String[] args) {
    Person p = new Person();
    System.out.System.out.println(p.getClass().getName());
  }
}
