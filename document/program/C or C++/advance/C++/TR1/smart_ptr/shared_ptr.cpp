#include <iostream>
#include <tr1/memory>
#include <assert.h>
#include <stdio.h>

using namespace std;
using namespace std::tr1;


class B
{
public:
	B(int a) :data(a)
	{
		cout << "construct B!!! " << data <<endl;
	}

	~B() 
	{
	 	cout << "destruct B!!! " << data << endl;
	}

	void PrintSomething()
	{
		cout << "just print something " << data <<endl;
	}


private:
	int data;
};

class FileCloser
{
public:
	void operator()(FILE *pf)
	{
		if (pf)
		{
			cout << "close fp" << endl;
			fclose(pf);
		}
  	}
};


int main() 
{
 	//    B* ptrB0 = new B();
	//shared_ptr<B> ptrB1(new B);
	
	shared_ptr<B> ptr;
	assert (ptr == NULL);
	
	{
		shared_ptr<B> pInt2(new B(2));
		assert(pInt2.use_count() == 1);   // temp2还没有引用指针
		{

			shared_ptr<B> pInt1(new B(1));
			assert(pInt1.use_count() == 1); // new int(2)这个指针被引用1次
			pInt2->PrintSomething();
			pInt2 = pInt1;
			pInt2->PrintSomething();
			assert(pInt1.use_count() == 2); // new int(2)这个指针被引用2次
			assert(pInt2.use_count() == 2);
		} //pInt1离开作用域, 所以new int(2)被引用次数-1
		
		pInt2.reset(new B(100));  //destruct B(1), and create B(100)
	  	assert(pInt2.use_count() == 1);
	} // pInt2离开作用域，引用次数-1,现在new int(14被引用0次，所以销毁它

	{
		shared_ptr<B> temp1(new B(3));  // 资源获取即初始化
		assert(temp1.use_count() == 1);

		shared_ptr<B> temp2(temp1);
		assert(temp2.use_count() == 2);
		assert(temp1.use_count() == 2);

	}   // temp1,temp2都离开作用域，引用次数变为0,指针被销毁

	{
  		shared_ptr<FILE> fp(fopen("test.txt", "r"), FileCloser());    // 指定调用FileCloser函数对象销毁资源
	}


	// int* pInt = new int(14);
	// shared_ptr<int> temp1(pInt); //don't do thing like this , it will be miss operate
	//shared_ptr<int> temp2(pInt);  // 用一个指针初始化temp2，temp2认为pInt只被它拥有。所以这个指针被引用1次
  									// temp1,temp2都离开作用域，它们都销毁pInt. pInt被销毁了两次！系统终于崩溃了 


	return 0;
	
}
