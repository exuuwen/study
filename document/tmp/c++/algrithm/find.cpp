#include <vector>
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;

class A
{
public:
	A(int data = 0);
	~A();
    
	int a;//msut be public
};


A::A(int data):a(data)
{}

A::~A()
{}

class CompareInt
{
public:
	CompareInt(int d) : data(d) {}
    bool operator()(vector<A>::value_type &x) const
    {
		return (x.a == data);
    }

private:
	int data;

};

int main()
{
	vector<int> v1;

	for(vector<int>::size_type i=0; i<10; i++)
	{
		v1.push_back(i);
	}

//find: operator=
	if (std::find(v1.begin(), v1.end(), 9) != v1.end())// using find_if for class 
	{
		cout << " find 10 in the v1" << endl;
	}
	
	vector<A> v2;

	A a1(1);
	v2.push_back(a1);
	A a2(2);
	v2.push_back(a2);
	A a3(3);
	v2.push_back(a3);
	A a4(4);
	v2.push_back(a4);
	
//find_if
	if (std::find_if(v2.begin(), v2.end(), CompareInt(3)) != v2.end())// using find_if for class 
	{
		cout << " find_if 3 in the v2" << endl;
	}
	
	return 0;
}

