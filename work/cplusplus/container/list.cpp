#include <vector>
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;
/*can't access by []*/
int main()
{
	list<int> array;

//high performance
	//fast insert 
	for (int i=0; i<5;)
	{
		array.push_back(i++);
		array.push_front(i++); 
	}
	
	list<int>::iterator iter = array.begin();
	for (; iter != array.end(); iter++)
	{
		cout << *iter << " ";
	}
	cout << endl;

	int& front = array.front();
	int end = array.back();
	cout << "c.front:" << front << ". c.end():" << end << endl;
	front = 1000;
	end = 100;

	for (auto & item : array)
	{
		cout << item << " ";
	}
	cout << endl;

	//fast earse
	array.pop_back();//must be not empty
	array.pop_front(); //must be not empty

	//insert
	iter = array.begin();
	array.insert(iter, 10);

	for (auto item : array)
	{
		cout << item << " ";
	}
	cout << endl;

	vector<int> lists = {1, 6, 9, 7};
	iter = array.begin();
	vector<int>::iterator liter = lists.begin();
	liter++;
	iter++;
	array.insert(iter, liter, lists.end());//list itertor no operator +, -

	for (auto item : array)
	{
		cout << item << " ";
	}
	cout << endl;

	//earse
	iter = array.begin();
	iter++, iter++;
	iter = array.erase(iter);
	array.erase(array.begin(), iter);//[)	

	for (auto item : array)
	{
		cout << item << " ";
	}
	cout << endl;
	array.clear();
	return 0;
}

