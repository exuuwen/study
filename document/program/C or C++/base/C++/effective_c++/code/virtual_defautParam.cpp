#include <iostream>
#include <string>
using namespace std;
class Base
{
public:
	Base(int d = 1, int pd = 2):data(d),protected_data(pd){}
	int show()
	{
		return data;
	}
	virtual void v_func(int a = 0)
	{
		cout<<"data:"<<a<<endl;
	}
protected:
	/*int show_protected_data()
	{
		return protected_data;
	}*/	
	int protected_data;
private:
	int data;
	//int protected_data;
};

class Drive:public  Base
{
public:
	int show_protected_data()
	{
		return protected_data;
	}
	virtual void v_func(int a = 2)
	{
		cout<<"d data:"<<a<<endl;
	}
};

int main()
{
	Drive  d;
	//int a = d.show()
	//d.protected_data = 1;
	cout << d.show() <<endl;
	cout << d.show_protected_data() <<endl;
	Drive *dd = new Drive;
	Base *b = dd;	
	b->v_func();
}
