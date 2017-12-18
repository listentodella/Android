// #include "Chinese.h"
// #include "Englishman.h"
#include "Human.h"

void test_eating(Human *h)
{
  h->eating();
}

int main(int argc, char const *argv[]) {
  // Englishman e("Bill", 100, "London");
  // Chinese c;
  Human &e = CreateEnglishman("Bill", 100, "London");
  Human &c = CreateChinese("Zhangsan", 10, "Beijing");
  Human *h[2] = {&e, &c};
  for(int i = 0; i < 2; i++){
    test_eating(h[i]);
  }

  delete &e;
  delete &c;
  return 0;
}
