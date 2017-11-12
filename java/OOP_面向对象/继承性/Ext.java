class Person {
  private int age;

  public void setAge(int age) {
    if(age < 0 || age > 200)
      age = 0;
    else
      this.age = age;
  }

  public int getAge() {
    return age;
  }

  public void printInfo() {
    System.out.println("age =" +age);
  }

  public Person() {
    System.out.System.out.println("person construct");
  }
  public Person(int age) {
    System.out.System.out.println("person construct");
    this.age = age;
  }
}





class Student extends Person {
  private String school;

  public void setSchool(String school) {
    this.school = school;
  }
//子类的构造方法调用前，会先调用父类的构造方法——默认先调用无参的
  public String getSchool() {
    return school;
  }

  public Student(String school) {
    /*will call the super()*/
    //相当于 super();
    //super(5);//调用父类有对应参数的构造方法
    this.school = school;
  }
//复写,override
  public void printInfo() {
    System.out.println("school = " +school";age =" +getAge());
  }
}

public class Ext {
  public static void main(String[] args) {
    Student stu = new Student();
    stu.setAge(10);
    System.out.println(stu.getAge());
    System.out.println(stu.getSchool());
    stu.printInfo();
  }
}
