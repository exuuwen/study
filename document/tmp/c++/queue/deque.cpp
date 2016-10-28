#include <deque>
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;
int main()
{
	deque<int> array;

//high performance
	//fast insert 
	for (int i=0; i<5;)
	{
		array.push_back(i++);
		array.push_front(i++); 
	}
	//fast access
	for (deque<int>::size_type i=0; i<array.size(); i++)
	{
		/*[]:can noly refer to exist items*/
		cout << "a[" << i << "]:" << array[i] << " "; 	
	}
	cout << endl;
	
	int& front = array.front();
	int end = array.back();
	cout << "c.front:" << front << ". c.end():" << end << endl;
	front = 1000;
	end = 100;
	for (deque<int>::size_type i=0; i<array.size(); i++)
	{
		/*[]:can noly refer to exist items*/
		cout << "a[" << i << "]:" << array[i] << " "; 	
	}
	cout << endl;

	//fast earse
	array.pop_back();//must be not empty
	array.pop_front(); //must be not empty

//low performance
	//insert
	deque<int>::iterator iter = array.begin();
	array.insert(iter, 10);

	iter = array.begin();
	for (; iter!=array.end(); iter++)
	{
		cout << " " << *iter;
	}
	cout << endl;

	list<int> lists = {1, 6, 9, 7};
	iter = array.begin();
	list<int>::iterator liter = lists.begin();
	liter++;
	array.insert(iter + 1, liter, lists.end());

	iter = array.begin();
	for (; iter!=array.end(); iter++)
	{
		cout << " " << *iter;
	}
	cout << endl;

	//earse
	iter = array.begin();
	iter = array.erase(iter + 2);
	array.erase(array.begin(), iter);//[)	

	for (deque<int>::size_type i=0; i<array.size(); i++)
	{
		/*[]:can noly refer to exist items*/
		cout << "a[" << i << "]:" << array[i] << " "; 	
	}
	cout << endl;
	array.clear();
	return 0;
}

