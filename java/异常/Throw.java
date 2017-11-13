/**自己处理一部分，剩下的别人处理
 *throws 异常(如算术异常)
 *谁调用这个方法谁去处理
 *
 *如果throws这里处理掉了，那么调用者就捕获不到异常了
 *因此如果想要协同处理异常，那么被调用者需要手动抛出异常，调用者才可以捕获到异常
 *
 * java Div 6 2
 * 6 / 2 = 3
 */
public class Throw {
  public static void main(String[] args) {
    int m = Integer.parseInt(args[0]);
    int n = Integer.parseInt(args[1]);
    int r = 0;

    System.out.println("begin for div");
    try {
      r = div(m, n);//如果main不进行处理，程序仍然会崩溃
    } catch (ArithmeException e) {
      System.out.println("main:"+e);
    }

    System.out.println("end for div");
    System.out.println(m + "/" + n + "=" +r);
  }

  public static int div(int m, int n) throws ArithmeException{
    int r = 0;
    try {
      r = m / n;
    } catch (ArithmeException e) {
      System.out.println("div:"+e);
      throw e;//手动抛出异常，这样调用者才可以捕获到异常，进行剩下的处理
    }

    return r;
  }
}
