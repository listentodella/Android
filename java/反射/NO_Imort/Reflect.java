//import a.b.c.d;
import java.lang.reflect.Construct;
import java.lang.reflect.Method;
import java.lang.reflect.Field;

public class Reflect {
  public static void main(String[] args) throws Exception{
    Class<?> c = null;
    try{
/*通过包的名称获取类的对象*/
       c = Class.forName("a.b.c.d.Person");
    } catch (ClassNotFoundException e) {
      System.out.println(e);;
    }
/*通过对象获取类的构造方法*/
/*因为本文件中没有Person类且没有import，故直接实例化是肯定编译错误的*/
/*Object类是所有类的父类，因此是可行的*/
    // Person p = c.newInstance();
    Object p = null;
    try {
      p = c.newInstance();//调用其无参的构造方法
    } catch (InstantiationException e) {
      System.out.println(e);
    }

    Construct<?> con = c.getConstruct(String.class);
    Object p2 = con.newInstance("Della");
/*获取方法*/
/*如果方法不是public，编译无法通过*/
    Method set = c.getMethod("setName", String.class);
    set.invoke(p2, "123");
    set.invoke(p, "abc");

    Method get = c.getMethod("getName");

    System.out.println(get.invoke(p));
    System.out.println(get.invoke(p2));
/*获取属性*/
    Field name = c.getDeclareField("name");
    //如果不设置accessible不能通过编译，除非修改该属性为public
    //但是该方法容易破坏封装性，所以尽量不去使用，而是使用其提供的方法去修改属性
    //name.setAccessible(true);
    name.set(p, "www");
    name.set(p2, "100ask");
    System.out.println(name.get(p));
    System.out.println(name.get(p2));
  }
}
