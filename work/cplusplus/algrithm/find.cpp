#include <vector>
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;

bool IsOdd (int i) { return ((i%2)==1); }

class A
{
public:
	A(int data = 0);
	~A();
    
	int get();
private:
	int a;
};


A::A(int data):a(data)
{}

int A::get()
{
	return a;
}

A::~A()
{}

class CompareInt
{
public:
	CompareInt(int d) : data(d) {}
    bool operator()(vector<A>::value_type &x) const
    {
		return (x.get() == data);
    }

private:
	int data;

};

int main()
{
	vector<int> v1;

	for(int i=0; i<10; i++)
	{
		v1.push_back(i);
	}

//find: operator=
	if (std::find(v1.begin(), v1.end(), 9) != v1.end())// using find_if for class 
	{
		cout << " find 10 in the v1" << endl;
	}

	std::vector<int>::iterator it = std::find_if(v1.begin(), v1.end(), IsOdd);
  	std::cout << " The first odd value is " << *it << '\n';
	
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

	/*
	while(first!=last && !binary_op(*first)) first++;
	return first;
	*/
	
	//find_if_not
	//find_end
	//find_first_of

	return 0;
}

