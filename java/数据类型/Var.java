/**
 * java里的数组里的值永远是放在堆里面的
 * ---->>>p还是存放地址值，但是指向的数据，是存放在堆里的
 *
 *
 * 而C则是放在栈里的(除非使用malloc,此时需要free)
 */
public class Var{
  public static void main(String[] args) {
    int a = 3;
    // float f = 3.14;从double转换到float可能会有损失
    float f = (float) 3.14;//强转
    float f2 = 3.14f;

    int i = 4;
    short s = 4;//对于整数，未超出范围编译器则会进行转换
    // short s2 = 40000;//会出错，超出short的范围
    short s2 = (short) 40000;//强转

    // s = i;对于变量，事先是不知道范围的
    // s = s + 1;//对于byte, short算术运算，为了保证精度，会自动转换为int
    //s = s + s2;//也是同样的道理，也只能强转
    s = (short)(s + 1);

/*java 里没有指针，也不能malloc,但是有new*/
    // int *p = malloc(10 * sizeof(int));
    int p[] = new int[10];
    int p2[] = {1, 2, 4};//静态分配
    // char str[100];
    char str[] = new char[100];
    // char str1[] = "abc";
    String str2 = "abc";//str2还是存放地址值，但是指向的数据，是存放在堆里的

/*java不需要free，将其置为null即可，
 *如果有多个地址与堆相连，只要都为null，垃圾回收机制就将地址与堆的连接断开了
 */
    p = null;
    p2 = null;
    str = null;
    str2 = null;
  }
}
