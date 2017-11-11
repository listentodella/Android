public class Function {
  public static void main(String[] args) {
    System.out.println(add(1, 2));
    System.out.println(add(1, 2, 3));
    System.out.println(add(1.0f, 2.0f));
  }

  public static int add(int x, int y) {
    return x + y;
  }
  // 只改变返回值类型无法实现重载
  // public static float add(int x, int y) {
  //   return x + y;
  // }

/*改变参数的个数实现重载*/
  public static int add(int x, int y, int z) {
    return x + y + z;
  }
/*改变参数的类型实现重载*/
  public static float add(float x, float y) {
    return x + y;
  }
}
