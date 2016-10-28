#include <iostream>
using namespace std;
//cast_conversion强制转换
/*
  dynamic_const:运行是指针和引用对象转换
  reinterpret_cast:底层的类似I/O转换，比如一个物理I/O地址转换为指针

  const_cast:去掉const属性
  static_cast:相关类型的转换 
*/

int main()
{
//static_cast
	int data = 2;
	cout << "data:" << data << endl;
	void* temp = &data;
	int* back = static_cast<int *>(temp);//conver back to int*  不加的话会出现 test.cpp:15: error: invalid conversion from ‘void*’ to ‘int*’
	cout << "back:" << *back << endl;	
//const_cast
	const int* const_data = new int(3);
	cout << "const_data:" << *const_data << endl;
	int *p = const_cast<int*>(const_data);//不加会出现 test.cpp:23: error: invalid conversion from ‘const int*’ to ‘int*’
	*p = 2;
	cout << "const_data:" << *const_data << endl;//好危险 这样const值就能被改变类
	
}

