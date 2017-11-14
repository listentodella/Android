
package a;
import b.*;

public class Pack {
  public static void main(String[] args) {
    //如果Mymath不是public则不可以被不同包访问
    Mymath m = new Mymath();
  }
}
