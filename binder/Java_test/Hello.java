
/**
 * 编译方法：
 * javac Hello.java
 * --->>> hello hello.c hello.class
 *
 * 运行：
 *  java hello
 */


public class Hello {
  public static void main(String args[]) {
    for(int i = 0; i < 3; i++)
      System.out.println("hello world");
    while(true) {

        try {
            Thread.sleep(100);
        } catch (Exception e) {

        }
    }
  }
}
