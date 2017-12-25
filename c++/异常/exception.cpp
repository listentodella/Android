#include "iostream"
#include "string"
#include "unistd.h"
using namespace std;
void C()
{

}

void B()
{
	C();
	throw 1;
}
void A()
{
	try{
		B();
	}catch(int i){
		cout<<"catch exception"<<endl;
	}
}


int main()
{
	
	A();
	return 0;
}


