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
public class Runtime {
  public static void main(String[] args) {
    int m = 0;
    int n = 0;
    int r = 0;

    System.out.println("begin for div");
    try {
      m = Integer.parseInt(args[0]);
      n = Integer.parseInt(args[1]);
      r = div(m, n);//如果main不进行处理，程序仍然会崩溃
    } catch (ArithmeException e) {
      System.out.println("main:"+e);
    } catch (NumberFormatException e) {
      System.out.println("main:"+e);
      /**
       * 实际异常的情况有很多，不可能都想到，可以使用这个父类异常
       * 但是不能放在最前面，否则编译都过不了，所以前面要有几个子类异常
       */
    } catch (RuntimeException e) {
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
    } finally {
      System.out.println("finally div:"+e);
      /**
       * finally里的代码是比throw先执行的
       * 如果在finally里返回，则异常处理就已经完成了，
       * 那么throw是没有机会执行的，那么main是捕捉不到异常的
       * 因此try or catch里throw，那么finally就不应该有return
       */
      return r;
    }

    // return r;
  }
}
