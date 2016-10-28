#include <iostream>
#include <vector>
using namespace std;
namespace test
{
	int a;
	char b;
}
namespace test//命名空间扩展性  当再定义一次，就相当于把新的加到原来的空间中，可以存在不同文件
{
	int add;
}
int main()
{
 	//a = 2;//wrong 找不到空间  下面有三种方法
	{
		using namespace test;
		a = 2;
		add = 0;
	}
	
	{
		using test::a;
		a = 2;	
	}

	{
		test::a = 2;
	}
	
}
