/**别人处理
 *throws 异常(如算术异常)
 *谁调用这个方法谁去处理
 * java Div 6 2
 * 6 / 2 = 3
 */
public class Throws {
  public static void main(String[] args) {
    int m = Integer.parseInt(args[0]);
    int n = Integer.parseInt(args[1]);
    int r = 0;

    System.out.println("begin for div");
    try {
      r = div(m, n);//如果main不进行处理，程序仍然会崩溃
    } catch (ArithmeException e) {
      System.out.println(e);
    }

    System.out.println("end for div");
    System.out.println(m + "/" + n + "=" +r);
  }

  public static int div(int m, int n) throws ArithmeException{
    int r = 0;
    r = m / n;
    return r;
  }
}
