//============================================================================
// Name        : template.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "template.h"



int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	int ret1 = compares(2, 3);
	cout << "ret1 " << ret1 <<endl;

	int ret2 = compares("sa", "as");
	cout << "ret2 " << ret2 <<endl;

	int ret3 = compares('a', 'a');
	cout << "ret3 " << ret3 <<endl;

	int ret4 = compares(2.0, 1);
	cout << "ret4 " << ret4 <<endl;

	int ret5 = compares(2.0, 'a');  //can not "aaa" with 'a'
	cout << "ret5 " << ret5 <<endl;
////////////////////////////////////////////////////
	int b[2];
	array_init(b);
	for(int i =0; i < 3; i++)
		cout << b[i] <<endl;
	cout << endl;
	cout << endl;
//////////////////////////////////////////////////
	Queue<int> qi;
	Queue<int> q2;

	qi.push(1);
	qi.push(2);
	q2 = qi;
	qi.push(3);
	qi.push(4);
	qi.push(5);
	//q2.push(1);
	//cout << qi << endl;
	//cout << qi.pop() << endl;
	//cout << qi.pop() << endl;
	//cout << qi.pop() << endl;

	cout << q2 <<endl;
	cout << qi << endl;

	//cout << "2221" << endl;
	int a[4] = {0, 1, 2, 3};

	Queue<int>  q(a, a+4);
	//vector<int>  v(a, a+4);
	//q.pop();
	cout << q << endl;
	q2.assign(a, a+4);
	cout << q2 << endl;
	cout << q2.pop() << endl;
	cout << q2 << endl;
///////////////////////////////
	Data<int> *a_data = new Data<int>(10);
	Handle< Data<int> >  a_handle(a_data);
	int d = a_handle->get_data();
	cout << d <<endl;
	d = (*a_handle).get_data();
	cout << d <<endl;

	Data<int> *b_data = new Data<int>(9);
	Handle< Data<int> >  b_handle(b_data);

	a_handle = b_handle;
	d = a_handle->get_data();
	cout << d <<endl;
	d = (*a_handle).get_data();
	cout << d <<endl;


	//a_handle->

	return 0;
}
