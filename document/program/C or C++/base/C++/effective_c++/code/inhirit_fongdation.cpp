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
	void no_virtual()
	{
		cout<<"base novirtual"<<endl;
	}
	void acessy_pubilc_inhirit()
	{
		cout<<"acessy_pubilc_inhirit"<<endl;
	}
protected://代表其继承类可以访问 不管是private 还是 public继承
	int protected_data;
private:
	int data;
	//int protected_data;
};
// public 继承是 is-a 用子类类型传给父类的参数也可以,父类的成员在子类中的开放属性不变
class Drive:public  Base
{
public:
	int show_protected_data()
	{
		return protected_data;
	}
	virtual void v_func(int a = 2)  //不要重定义带默认参数的需函数  函数是动态加载，而默认参数是静态加载
	{
		cout<<"d data:"<<a<<endl;
	}
	void no_virtual()
	{
		cout<<"virtual novirtual"<<endl;//不要重定义非虚拟函数，其是静态型
	}
	
};


//private 继承 不是is-a 不能用子类类型传给父类的参数，他是has-a  而且父类成员在子类中全是private
class Drive_private:private  Base
{
public:
	int show_protected_data()
	{
		return protected_data;
	}
	
	
};

int main()
{
	Drive  d;

	cout << d.show() <<endl;
	cout << d.show_protected_data() <<endl;


	Drive *dd = new Drive;
	Base *b = dd;	//静态是Base动态是Drive
	b->v_func(); //结果是d data: 0  指针是所以调用动态函数但是参数是静态 就是base的 0

	b->no_virtual();//结果是Base类的函数


	Drive_private *pd = new Drive_private;
	cout << pd->show_protected_data() <<endl;
	
	//pd->acessy_pubilc_inhirit(); //error  不能被获取
	
}
