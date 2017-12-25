# 异常
A > B > C
```
C()
{
	return xx;
}
B()
{
	if(C()){
	}else
		return err;
}
A()
{
	if(B()){
	...
	} else {
	...
	}
}
```
* C语言：使用长跳转来解决类似的问题
A:setjmp
C:longjmp
* C++:使用异常
函数A捕捉函数C发出的异常
1. 谁捕捉异常？A函数
2. 谁抛出异常？C函数
3. 捕捉到异常之后怎么处理？随A

