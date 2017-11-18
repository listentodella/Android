package a.b.c.d;

class Student {
  private String name;

  public void setName(String name) {
    this.name = name;
  }

  public String getName() {
    return this.name = name;
  }

  public Student() {
    System.out.println("Construct of Student");
  }

  public Student(String name) {
    this.name = name;
    System.out.println("Construct of Student, name is "+this.name);
  }
}
