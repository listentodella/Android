
// 这样写编译似乎会冲突
// import a.b.c.d1.Math;
// import a.b.c.d2.Math;

import a.b.c.d1.*;
import a.b.c.d1.*;

public class Import {
  public static void main(String[] args) {
    /*
     *add
     *如果两个包里有同名的类、方法，需要加上路径以区分
     *不同名的类不需要加上包区分
     */
    System.out.println(a.b.c.d1.Math.add(1, 2));
    System.out.println(a.b.c.d2.Math.add(1, 2));

    /*sub*/
    System.out.println(a.b.c.d1.Math.sub(1, 2));
  }
}
