#include "Chinese.h"
#include "Englishman.h"

void test_eating(Human *h)
{
  h->eating();
}

int main(int argc, char const *argv[]) {
  Englishman e("Bill", 100, "London");
  Chinese c;

  Human *h[2] = {&e, &c};
  for(int i = 0; i < 2; i++){
    test_eating(h[i]);
  }
  return 0;
}
