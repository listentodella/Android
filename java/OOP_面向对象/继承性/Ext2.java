class Father {
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

  public void printInfo2() {
    System.out.println("This is Father");
  }

}



class Son extends Father {
  public void printInfo2() {//可以继承、复写
    System.out.println("This is Son");
  }
  // private void printInfo2() {//但不可以私吞
  //   System.out.println("This is Son");
  // }
  public void printInfo() {//由于父类的是私有的，子类的是独创
    System.out.println("This is Son");
  }
}

public class Ext2 {
  public static void main(String[] args) {
    Son son = new Son();
    // son.money = 100;
    son.setMoney(100);//可以调用父类提供的接口
    // son.printInfo();调用不了私有方法
  }
}
