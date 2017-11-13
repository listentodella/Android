/**自己处理
 *try...catch...finally...
 * java Div 6 2
 * 6 / 2 = 3
 */
public class Try {
  public static void main(String[] args) {
    int m = Integer.parseInt(args[0]);
    int n = Integer.parseInt(args[1]);
    System.out.println("begin for div");
    int r = div(m, n);//如果是非法除数，程序仍会继续运行，不会崩溃
    System.out.println("end for div");
    System.out.println(m + "/" + n + "=" +r);
  }

  public static int div(int m, int n) {
    int r = 0;
    try {
       r = m / n;
    } catch (ArithmeException e) {//算术异常
      System.out.println(e);
    } finally {//不管是否发生异常都会执行
      System.out.println("this is finally div");
    }
    return r;
  }
}
