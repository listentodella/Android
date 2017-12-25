#include "iostream"
#include "string"
#include "unistd.h"
#include "stdlib.h"
#include "exception"
using namespace std;
class MyException {
public:
	virtual void what(void){cout<<"this is MyException"<<endl;}
};

class MySubException:public MyException{
public:
	void what(void){cout<<"this is MySubException"<<endl;}
};



void C(int i) throw(int ,double)//这样声明了之后，即便代码里声明了其他的异常，但是系统仍会使用默认的异常处理，不会使用我们的代码
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
	else if(i == 3)
		throw c;
	else if(i == 4)
		throw MyException();
	else if(i == 5)
		throw MySubException();
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
	}catch(double d){
		cout<<"catch double"<<d<<endl;
	}catch(MySubException &e){//先捕捉子类的，否则会被父类“包含”
		e.what();
	}catch(MyException &e){
		e.what();
	}catch(...){
		cout<<"catch others..."<<endl;
	}
}

void my_unexcepted_func()
{
	cout<<"my_unexcepted func"<<endl;
}

void my_terminate_func()
{
	cout<<"my_terminate func"<<endl;
}

int main(int argc, char **argv)
{
	int i;
	
	set_unexpected(my_unexcepted_func);//设置自己的异常处理函数
	set_terminate(my_terminate_func);

	if(argc != 2){
		cout<<"usage:"<<endl;
		cout<<argv[0]<<" <0 | 1 | 2 | 3>"<<endl;
		return -1;
	}
	i = strtoul(argv[1], NULL, 0);


	
	A(i);
	return 0;
}


