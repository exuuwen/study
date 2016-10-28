#include <iostream>

using std::cout;
using std::endl;

class Aclass
{
public:
	void interface(int a = 5)
	{
	//	cout << "in the base interfce:" << a << endl;  
		do_interface(a);
	}
private:
	virtual void do_interface(int a)
	{
		cout << "in base do interface:" << a << endl;
	}
};

class Dclass : public Aclass
{
private:
	virtual void do_interface(int a)
	{
		cout << "in derived do interface:" << a << endl;
	}
};


int main()
{
	Aclass a;
	Dclass d;

	Aclass &af = a;
	Aclass &df = d;

	af.interface();
	df.interface(45);

	return 0;
}
