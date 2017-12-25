#include "iostream"
#include "string"
#include "unistd.h"
using namespace std;

template<typename T>
class AAA{
private:
	T t;
public:
	void test_func(const T &t);
	void print(void);
};

template<typename T>
void AAA<T>::test_func(const T &t)
{
	this->t = t;
	cout<<t<<endl;
}

template<typename T> void AAA<T>::print(void)
{
	cout<<t<<endl;
}

int main()
{
	AAA<int> a;
	a.test_func(1);
	a.print();

	AAA<double> b;
	b.test_func(1);
	b.print();

	return 0;
}