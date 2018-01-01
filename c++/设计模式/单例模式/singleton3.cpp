#include "iostream"
#include "pthread"
#include "unistd"

using namespace std;

class Singleton;

Singleton *gInstance;
static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER;

class Singleton{
public:
  static Singleton *getInstance()
  {
    pthread_mutex_lock(&g_tMutex);
    if(gInstance == NULL)
      gInstance == new Singleton;
    pthread_mutex_unlock(&g_tMutex);
    return gInstance;
  }

  Singleton()
  {
    std::cout << "Singleton()" << '\n';
  }

  void printInfo(){std::cout << "This is Singleton" << '\n';}

};

void *start_routine_tid1(void *arg)
{
  std::cout << "this is tid1..." << '\n';
  Singleton *s = Singleton::getInstance();
  s->printInfo();
  return NULL;
}

void *start_routine_tid2(void *arg)
{
  std::cout << "this is tid2..." << '\n';
  Singleton *s = Singleton::getInstance();
  s->printInfo();
  return NULL;
}

int main()
{
  Singleton *s1 = Singleton::getInstance();
  s1->printInfo();

  Singleton *s2 = Singleton::getInstance();
  s2->printInfo();

  Singleton *s3 = Singleton::getInstance();
  s3->printInfo();

  /*创建线程，在线程里也调用Singleton::getInstance*/
  /*在线程中也不会重新执行实例化*/
  pthread_t tid1, tid2;
  pthread_create(&tid1, NULL, start_routine_tid1, NULL);
  pthread_create(&tid2, NULL, start_routine_tid2, NULL);

  sleep(3);
  return 0;
}
