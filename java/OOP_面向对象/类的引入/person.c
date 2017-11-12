#include <stdio.h>

struct person {
  char *name;
  void (*print_name)(struct person *per);
};

void print_name(struct person *per)
{
  printf("%s\n", per->name);
}

int  main(int argc, char const *argv[]) {
  struct person p1 = {"zhangsan", print_name};
  struct person p2 = {"lisi", print_name};

  p1.print_name(&p1);
  p2.print_name(&p2);

  return 0;
}
