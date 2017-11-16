/**
 * export LD_LIBRARY_PATH=...
 * 把要加载的动态库设置好，运行时才能正确加载
 */

public class JNIDemo {
  static {//静态代码块只执行一次，适合用来加载C库
    /*1.加载C库 load*/
    System.loadLibrary("native");//libnative.so,这里写作"native"可以看作是一种约定，libnative.so是本地C编译成库时的名字
  }

  /*native，本地方法，是指本地的C或者C++提供的方法，而不是java的*/
  public native static int hello1(int m);
  public native static String hello2(String str);
  public native static int hello3(int []a);
  public native static int[] hello4(int []a);
  public static void main(String[] args) {

    int []a = {1, 2, 3};
    int []b = null;
    int i = 0;
    /*2.建立映射 map java hello <--> C c_hello*/
    /*java只是声明，由C去建立*/

    /*3.调用 call*/
    System.out.println(hello1(123));//如果不是static，就需要先实例化对象 JNIDemo d = new JNIDemo();
    System.out.println(hello2("this is java"));
    System.out.println(hello3(a));

    b = hello4(a);
    for(i = 0; i < b.length; i++)
      System.out.println(b[i]);
  }
}
