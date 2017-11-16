
class Person {
  // private int age;
  private String age;
  // public void setAge(int age) {
  public void setAge(String age) {
    this.age = age;
  }

  // public int getAge() {
  public String getAge() {
    return this.age;
  }
}

public class Generic {
  public static void main(String[] args) {
    Person p = new Person();
    // p.setAge(3);
    p.setAge("3 years old");
    System.out.println(p.getAge());
  }
}
