//接口里变量都是常量，不管是否是有关键字
//接口里的方法只能是抽象方法
interface A {
  public static final int i = 10;
  public abstract int getNum();
}

interface B {
  public static String name = "InterfaceB";
  public abstract String getName();
}



class Son implements A,B {
  public void getNum() {
    return i;
  }
  public String getName() {
    return name;
  }
}

public class Interface {
  public static void main(String[] args) {
    // 抽象类不能实例化
    // Father f = new Father();
    Son son = new Son();
    System.out.println(son.getName());
    System.out.println(son.getNum());
  }
}
