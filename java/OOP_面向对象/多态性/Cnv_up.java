/*向上转换
 *父类只能调用被子类复写的方法，除非子类没去复写
 *父类不能调用子类独有的方法
*/

class Father {
  private int money;
  public int getMoney() {
    return money;
  }

  public int setMoney(int money) {
    return this.money = money;
  }

  public void printInfo() {
    System.out.println("This is Father");
  }

}


class Son extends Father {
  public void printInfo() {
    System.out.println("This is Son");
  }
  public void playGame() {
    System.out.println("play game....");
  }
}

class Daughter extends Father {
  public void printInfo() {
    System.out.println("This is Daughter");
  }
}

public class Cnv_up {
  public static void main(String[] args) {
    Son son = new Son();
    Daughter daughter = new Daughter();
/*向上转换
 *父类只能调用被子类复写的方法，除非子类没去复写
 *父类不能调用子类独有的方法
 *可以预见，传参时会默认执行下面的转换，根据参数类型的不同可以方便地进行调用
*/
    Father f = son;
    f.printInfo();
    // f.playGame();
    f = daughter;
    f.printInfo();
  }
}
