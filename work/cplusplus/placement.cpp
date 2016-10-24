#include <stdlib.h>
#include <iostream>

using namespace std;

class MyClass
{
public:
    MyClass(int a = 10)
    {
        cout <<"Constructors "<< a << endl;
    }
    ~MyClass()
    {
        cout <<"Destructors "<< a << endl;
    }

private:
	int a;
};

int main(int argc, char* argv[])
{
   MyClass* pMyClass =new MyClass;
   delete pMyClass;

	void *p = malloc(sizeof(MyClass));
	MyClass* p_myclass = new(p) MyClass(20);

	p_myclass->~MyClass();
	free(p);
}
