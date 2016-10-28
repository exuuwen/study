#include <iostream>
#include <memory>
#include <assert.h>
#include <stdio.h>

using namespace std;

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


int main() 
{
	auto_ptr<B> pInt1(new B(2));
	
	if(pInt1.get() != NULL);  
	{
		pInt1->PrintSomething();
	}	
	
	auto_ptr<B> pInt2 = pInt1; /* 1. It is better not used the = operation, so can not be used as function parameter and in vector*/

	if(pInt2.get() != NULL);   
	{
		pInt2->PrintSomething();
	}

	/* core dump: The Pint1 can not be used anymore.*/
	/*
	if(pInt1.get() != NULL);   
	{
		pInt1->PrintSomething();
	}
	*/
	
	/*2. auto_ptr can be owned by only one user!*/

	if(pInt2.get() != NULL);   
	{
		B* t = pInt2.release();/*3. release do not free the memory just return it and the ownship of pInt2, so now we need delete by my self*/
		delete t;
	}
	
	

	return 0;
	
}
