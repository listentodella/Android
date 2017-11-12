/**
 * 在进行对象的向下转换前，必须首先发生对象的向上转换
 * Father f = new Son();
 * Son son = (Son)f;
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

  public void drink() {
    System.out.println("drink...");
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
  public void dance() {
    System.out.println("dance....");
  }
}

public class Cnv_down {
  public static void main(String[] args) {
    Father f = new Father();
    Son s = new Son();
    Daughter d = new Daughter();

    print(f);
    print(s);
    print(d);

    printAction(f);
    printAction(s);
    printAction(d);

  }


  public static void print(Father f) {
    f.printInfo();
  }

  public static void printAction(Father f) {
    if(f instanceof Son) {
      Son son = (Son)f;
      son.playGame();
    } else if(f instanceof Daughter) {
      Daughter d = (Daughter)f;
      d.dance();
    } else if (f instanceof Father) {//所有的子类调用 instanceof 父类时都是成立的
      f.drink();
    }
  }

  
}
