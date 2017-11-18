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
/*对于泛型*/
    Class<?> c1 = null;
    try{
       c1 = Class.forName("a.b.c.d.Person");
    } catch (ClassNotFoundException e) {
      System.out.println(e);;
    }

    Class<?> c2 = p.getClass();
    Class<?> c3 = Person.class;

    System.out.System.out.println(c1.getName());
    System.out.System.out.println(c2.getName());
    System.out.System.out.println(c3.getName());
/*对于基本数据类型、数组*/
    int arr[] = {1, 2, 3};
    int arr2[][] = {{1, 2, 3, 4}, {1}};
    Class<?> c4 = arr.getClass();
    Class<?> c5 = arr.getClass();
    Class<?> c6 = int.class;
    System.out.println(c4.getName());
    System.out.println(c5.getName());
    System.out.println(c6.getName());
    System.out.println(c4 == c5);//不管几维，都使用同一个object
    System.out.println(c4 == c6);//但是基本数据类型和数组却是不同的

  }
}
