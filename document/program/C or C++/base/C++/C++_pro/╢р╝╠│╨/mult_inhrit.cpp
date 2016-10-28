#include<iostream.h>

class fur
{
public:
	fur()
	{
		cout<<"fur buliding"<<endl;
	}
	void display()
	{
		cout<<"it is a fur"<<endl;
	}
	void setweight(int w)
	{
		weight=w;
	}
	int getweight()
	{
		return weight;
	}
private:
	int weight;
};

class sofa:virtual public fur
{
public:
	sofa():fur()
	{
		cout<<"sofa buliding"<<endl;
	}
	void watch_tv()
	{
		cout<<"watch tv"<<endl;
	}

};

class bed:virtual public fur
{
public:
	bed():fur()
	{
		cout<<"bed buliding"<<endl;
	}
	void sleep()
	{
		cout<<"sleep"<<endl;
	}


};

class sofabed:public sofa,public bed
{
public:
	sofabed():sofa(),bed()
	{
		cout<<"sofa and bed buliding"<<endl;
	}
	void flod_out()
	{
		cout<<"flod out"<<endl;
	}

};

void main()
{
	sofabed sb;
	cout<<"***************"<<endl;
	sb.display();
	sb.sleep();
	sb.watch_tv();
	sb.flod_out();
	cout<<"***************"<<endl;
	sb.setweight(10);
	cout<<"the weight is "<<sb.getweight()<<endl;
	
}