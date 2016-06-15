#include <vector>
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;
int main()
{
	vector<int> array;

//high performance
	//fast insert 
	cout << "fast --------------------------------" << endl;
	for (int i=0; i<5; i++)
		array.push_back(i);
		// array.push_front //err: no this
	//fast access
	for (vector<int>::size_type i=0; i<array.size(); i++)
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

	for (auto & item : array) {
		cout << item << " "; 	
	}

	cout << endl;

	//fast earse
	array.pop_back();//must be not empty
	//array.pop_front(); //err:no this operator	

//low performance
	cout << "slow --------------------------------" << endl;
	//insert
	vector<int>::iterator iter = array.begin();
	array.insert(iter, 10);

	iter = array.begin();
	for (; iter!=array.end(); iter++)
	{
		cout << *iter << " ";
	}
	cout << endl;

	list<int> lists = {1, 6, 9, 7};
	iter = array.begin();
	list<int>::iterator liter = lists.begin();
	liter++;
	array.insert(iter + 1, liter, lists.end());

	for (auto & item : array) {
		cout << item << " "; 	
	}

	//earse
	iter = array.begin();
	
	iter = array.erase(iter + 2);
	array.erase(array.begin(), iter);//[)	

	for (auto & item : array) {
		cout << item << " "; 	
	}
	cout << endl;

	array.clear();

//capacity
	cout << "capacity --------------------------------" << endl;
	vector<int> v1;
	
	v1.reserve(5);//for the first time reserve and as the base to increase double every time
	cout << "v vector capacity:" << v1.capacity() << endl; 

	for(int i=0; i<15; i++)
	{
		v1.push_back(i);//add a item to the end
		cout << "v vector capacity:" << v1.capacity() << endl;
		if(i==5)
			v1.reserve(12);//if it is ok to set the reserve value(must than the capacity value) and as the base to increase double every time need allocate agian afterwards
	}

	return 0;
}

