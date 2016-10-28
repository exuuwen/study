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

	shared_ptr<B> get_shared_ptr()
	{
		cout << "get shared ptr" << endl;
		return shared_from_this();
	}

private:
	int data;
};


int main() 
{
	{
		shared_ptr<B> p(new B(2));
		assert(p.use_count() == 1);   
		shared_ptr<B> p1 = p->shared_from_this();

		assert(p == p1);
		assert(p.use_count() == 2); 
		
		B& b_r = *p;
		shared_ptr<B> p2 = b_r.get_shared_ptr();
		assert(p == p2);
		
		assert(p.use_count() == 3); 

		//B b(2);
		//b.get_shared_ptr();
		/*error : must base on a shared_ptr*/
	}	

	return 0;
	
}
