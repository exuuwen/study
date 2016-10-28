#include <iostream>
#include <tr1/memory>
#include <assert.h>
#include <stdio.h>

using namespace std;
using namespace std::tr1;

class B : public enable_shared_from_this<B>
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
	weak_ptr<B> pIntw1;
	{
		shared_ptr<B> pInts(new B(2));
		pIntw1 = pInts;/*weak_ptr only can be constructed by shared_ptr or weak_ptr*/
		pInts->PrintSomething();
		assert(pInts.use_count() == 1); // only 1

		weak_ptr<B> pIntw2 = pIntw1;
		/*pIntw->PrintSomething();: weak ptr can not do this*/
		assert(pIntw1.use_count() == 1); // only 1

		shared_ptr<B> pInts1 = pIntw2.lock();// enhance the weak to it's shared 
		
		assert(pIntw1.use_count() == 2);  // only 1
		
	}	
	
	assert(pIntw1.expired());
	shared_ptr<B> pInt = pIntw1.lock();
	assert(!pInt);
	

	return 0;
	
}
