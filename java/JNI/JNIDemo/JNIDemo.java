/**
 * export LD_LIBRARY_PATH=...
 * 把要加载的动态库设置好，运行时才能正确加载
 */

public class JNIDemo {
  static {//静态代码块只执行一次，适合用来加载C库
    /*1.加载C库 load*/
    System.loadLibrary("native");//libnative.so
  }

  /*native，本地方法，是指本地的C或者C++提供的方法，而不是java的*/
  public native static void hello();

  public static void main(String[] args) {

    /*2.建立映射 map java hello <--> C c_hello*/
    /*java只是声明，由C去建立*/

    /*3.调用 call*/
    hello();//如果不是static，就需要先实例化对象 JNIDemo d = new JNIDemo();
  }
}
