#include <iostream>
#include <assert.h>
#include <stdio.h>

#include "Base.h"

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
	{
		scoped_ptr<B> pInt1(new B(2));
		pInt1->PrintSomething();

		//error
		//scoped_ptr<B> pInt = pInt1;
		
	}	
	
	return 0;
	
}
