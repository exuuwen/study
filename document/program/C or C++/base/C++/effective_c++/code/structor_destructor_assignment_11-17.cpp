#include <iostream>
/*当一个类为base类那么它的析构函数应该设为虚函数*/
using namespace std;
class Base
{
public:
	Base(int num = 0):father(num){ cout<<"in the base()"<<endl;}
	Base& operator=(const Base& rhs)
	{
		if(this == &rhs)   //看是否是自己给自己赋值
			return *this;
		father = rhs.father;
		return *this;
	}
	virtual void show()
	{
		cout<<"base:father:"<<father<<endl;
	}
	virtual ~Base(){ cout<<"in the ~base()"<<endl; }//must be virtual
private:
	int father;
};

class Derive:public Base
{
public:
	Derive(int num_f = 0, int num_c = 0):Base(num_f),child(num_c){ cout<<"in the derive()"<<endl;}//为所有成员赋值，以列表方式,依次赋值顺序为成员函数顺序
	Derive& operator=(const Derive& rhs)
	{
		if(this == &rhs)   //看是否是自己给自己赋值
			return *this;  
		Base::operator=(rhs);  //为所有成员赋值
		child = rhs.child;
		return *this;
	}
	void show()
	{
		Base::show();
		cout<<"derive:child:"<<child<<endl;
	}
	~Derive(){ cout<<"in the ~derive()"<<endl; }
private:
	int child;
};
int main()
{
	Base *b = new Derive(19, 29);  /*如果不是需函数那么就不会调用到~Derive()*/
	b->show();
	Base b1(2);
	Derive d1(4, 3);
	b1.show();
	d1.show();
	Derive d2 = d1;
	d2.show();
	Base b2 = d1;
	b2.show();
	delete b;
}
