#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <tr1/memory>

/*g++  -std=c++0x -o unique_ptr unique_ptr.cpp */
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


int main() 
{
	{
		std::unique_ptr<B> pInt1(new B(2));
		pInt1->PrintSomething();
		//error
		//unique_ptr<B> pInt = pInt1;
                unique_ptr<B> p2 = move(pInt1);//move the owership to p2
                assert(pInt1 == NULL);
		p2->PrintSomething();
	}	
	
	return 0;
	
}
