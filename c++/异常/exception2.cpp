#include "iostream"
#include "string"
#include "unistd.h"
#include "stdlib.h"

using namespace std;
void C(int i)
{
	int a = 1;
	double b = 1.2;
	float c = 1.3;
	if(i == 0)
		cout <<"In C, it is OK"<<endl;
	else if(i == 1)
		throw 1;
	else if(i == 2)
		throw b;
	else 
		throw c;
}

void B(int i)
{
	cout<<"call c..."<<endl;
	C(i);
	cout<<"after call C"<<endl;
}
void A(int i)
{
	try{
		B(i);
	}catch(int j){
		cout<<"catch exception"<<j<<endl;
	}
}


int main(int argc, char **argv)
{
	int i;
	if(argc != 2){
		cout<<"usage:"<<endl;
		cout<<argv[0]<<" <0 | 1 | 2 | 3>"<<endl;
		return -1;
	}
	i = strtoul(argv[1], NULL, 0);


	
	A(i);
	return 0;
}


