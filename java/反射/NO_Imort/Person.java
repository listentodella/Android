package a.b.c.d;
class Person {
  private String name;

  public void setName(String name) {
    this.name = name;
  }

  public String getName() {
    return this.name = name;
  }

  public Person() {
    System.out.println("Construct of Person");
  }

  public Person(String name) {
    this.name = name;
    System.out.println("Construct of Person, name is "+this.name);
  }
}
