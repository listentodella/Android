/**
 * java Div 6 2
 * 6 / 2 = 3
 */
public class Div {
  public static void main(String[] args) {
    int m = Integer.parseInt(args[0]);
    int n = Integer.parseInt(args[1]);
    System.out.println("begin for div");
    int r = div(m, n);//如果是非法除数，那么程序会退出运行
    System.out.println("end for div");
    System.out.println(m + "/" + n + "=" +r);
  }

  public static int div(int m, int n) {
    int r = m / n;
    return r;
  }
}
