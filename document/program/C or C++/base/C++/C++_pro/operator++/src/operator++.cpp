//============================================================================
// Name        : shuzu++.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "operator++.h"


int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	CheckPtr one(a, &a[9]);
	for(int i = 0; i<9; i++)
	{
		cout<<(++one).get_cur_value()<<" : ";
		//++one;
	}
	cout<<endl;
	for(int i = 0; i<9; i++)
	{
		cout<<(--one).get_cur_value()<<" : ";
		//--one;
	}
	cout<<endl;
	for(int i = 0; i<9; i++)
	{
		//one++;
		cout<<(one++).get_cur_value()<<" : ";
	}
	cout<<endl;
	for(int i = 0; i<9; i++)
	{
		//one--;
		cout<<(one--).get_cur_value()<<" : ";
	}
	cout<<endl;
	return 0;
}
