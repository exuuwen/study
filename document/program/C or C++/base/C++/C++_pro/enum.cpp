#include <iostream>
using namespace std;

enum test {one = 1, two = 3, three = 8};//enum 没有范围  和 c++ special说的不一样
int main()
{
	cout << "enum length:"<< sizeof(test) << endl;//32bit
	test a = test(-7);
	cout << "test(-7):"<< a <<endl;
	//a = 1  //错误 类型不对  不能隐式的从整形转化而来
	a = test(166);
	cout << "test(166):"<< a <<endl;
	cout << "one:"<< one <<endl;
	return 0;
}
