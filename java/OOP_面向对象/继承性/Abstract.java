abstract class Father {
  private int money;
  public int getMoney() {
    return money;
  }

  public int setMoney(int money) {
    return this.money = money;
  }

  private void printInfo() {
    System.out.println("This is Father");
  }
//父类只需要声明，子类必须要实现
  public abstract void study();

}



class Son extends Father {
  public void study() {
    System.out.println("I am studying");
  }
}

public class Abstract {
  public static void main(String[] args) {
    // 抽象类不能实例化
    // Father f = new Father();
    Son son = new Son();
    son.study();
  }
}
