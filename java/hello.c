#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  for(int i = 0; i < 3; i++)
    printf("hello world\n");

  int *p = (int *)malloc(sizeof(int));
  char str[100];
  free(p);
  return 0;
}
