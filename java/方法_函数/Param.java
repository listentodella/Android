/**
 * 传参与C一样
 */

public class Param {
  public static void main(String[] args) {
    int x = 1;
    System.out.println("before fun: x =  " +x);
    fun(x);
    System.out.println("after fun: x =  " +x);


    int p[] = new int[1];
    p[0] = 123;
    System.out.println("before fun2: p[0] =  " +p[0]);
    fun2(p);
    System.out.println("after fun2: p[0] = " +p[0]);
  }

  public static void fun(int x) {
    x = 100;
  }
  public static void fun2(int[] p) {
    p[0] = 200;;
  }
}
