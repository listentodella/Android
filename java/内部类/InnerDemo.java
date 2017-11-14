
class Outer {
  private int a = 10;

  class Inner {
      public void printInfo() {
        System.out.println("a = " +a);
      }
  }
}


public class InnerDemo {
  public static void main(String[] args) {
    /**
     * 先实例化外部类的对象
     * 再实例化一个内部类对象
     * 如果想要一次性实例化内部类，需要给内部类加上static属性
     * 但是这样内部类值也只能访问外部类的static属性
     * Outer.Inner i = new Outer.Inner();
     */
    Outer o = new Outer();
    Outer.Inner i = o.new Inner();
    i.printInfo();
  }
}
