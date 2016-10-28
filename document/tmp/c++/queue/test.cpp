#include <vector>
#include <deque>
#include <list>
#include <string>
#include <iostream>

class A
{
public:
	A(int a, int b)
	{
	}
};


class noncopyable
{
public:
        noncopyable() {}
        ~noncopyable() {}
private:
        noncopyable(const noncopyable&);
        const noncopyable& operator=(const noncopyable&);
};

class B : noncopyable
{
public:
        B(){}
private:
        int data;
};

int main()
{

//init for all the vector/list/deque
	//std::vector<A> array;  //err: no default constructor
	std::vector<A> array(10, A(4, 5));
	//std::vector<A> arraya(array); //err: no cpoy constructor
	
	std::vector<int> zlarray; //zero item with zero value in default 
	std::vector<int> zarrayi(10); //10 item with zero value in default 
	std::vector<int> iarray(10, 3);
	std::vector<int> cpy(iarray);

	/* 
	vector<B> a;
	vector<B> b(a); //err:no copy constructor
	*/

	//std::list<std::string> authors = {"Milton", "Shakespeare", "Austen"};//g++ std=c++0x and only used for initialization

	std::list<std::string> authors(4, "haha");// = {"Milton", "Shakespeare", "Austen"};
	
	std::list<std::string>::iterator eiter = authors.end();
	eiter--;
	//init with other typeof iterator with the same typeof item
	std::vector<std::string> str(authors.begin(), eiter);
	
	std::vector<std::string>::iterator siter = str.begin();
	std::cout << "str :" << std::endl;
	for (; siter!=str.end(); siter++)
	{
		std::cout << " " << *siter;
	}	
	std::cout << std::endl;

// type
	std::vector<int>::size_type size;	
	size = iarray.size();
	std::cout << "iarray :" << std::endl;
	for (std::vector<int>::size_type i = 0; i < size; i++)
	{
		std::cout << " " << iarray[i];//[]:only can access the exist one
		iarray[i] = i;
	}	
	std::cout << std::endl;
	
	
	std::vector<int>::iterator  iter = iarray.begin();
	//auto iter = iarray.begin();  //g++ -std=c++0x

	std::cout << "iarray begin:" << *iter << std::endl;
	*iter = 7;
	std::cout << "iarray :" << std::endl;
	for (; iter!=iarray.end(); iter++)
	{
		std::cout << " " << *iter;
	}	

	std::cout << std::endl;

	//g++ -std=c++0x
	std::vector<int>::const_iterator citer = iarray.cbegin();

	std::cout << "iarray cbegin:" << *citer << std::endl;

	std::vector<int>::difference_type cnt = iarray.begin() - iarray.end();
	std::cout << "difference type:" << cnt << std::endl;

// assignment

	std::deque<int> a1(10, 1); 
	std::deque<int> a2; 
	a1 = a2; // replaces elements in a1

	std::deque<B> a3(10); 
	std::deque<B> a4(2);
	//a4 = a3;
	//exchange a1 with a2	
	a1.swap(a2);
	
    //assign
	zlarray.assign(10, 2);
	array.assign(2, A(3, 5));
	//cpy.assign(iarray);//err
	
	std::list<std::string>::iterator meiter = authors.end();
	meiter--;
	//init with other typeof iterator with the same typeof item
	std::vector<std::string> mstr(10, "hsh");
	mstr.assign(authors.begin(), meiter);
	
	std::vector<std::string>::iterator msiter = mstr.begin();
	std::cout << "mstr :" << std::endl;
	for (; msiter!=mstr.end(); msiter++)
	{
		std::cout << " " << *msiter;
	}	
	std::cout << std::endl;

//size

	std::cout << "zlarray.size():" << zlarray.size() << std::endl;
	std::cout << "zlarray.max_size():" << zlarray.max_size() << std::endl;
	std::cout << "zlarray.empty():" << zlarray.empty() << std::endl;

//rational
	std::vector<int> v1 = { 1, 3, 5, 7, 9, 12 };
	std::vector<int> v2 = { 1, 3, 9 };
	std::vector<int> v3 = { 1, 3, 5, 7 };
	std::vector<int> v4 = { 1, 3, 5, 7, 9, 12 };
	
	std::cout << "v1 < v2:" << (v1 < v2) << std::endl; 
	std::cout << "v1 < v3:" << (v1 < v3) << std::endl; 
	std::cout << "v1 == v4:" << (v1 == v4) << std::endl;
	std::cout << "v1 == v2:" << (v1 == v2) << std::endl;

	std::vector<B> a(2);	
	std::vector<B> b(2);

	//a == b; //err: A has no == operator	
	
	return 0;
}
