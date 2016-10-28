//============================================================================
// Name        : point_count.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "point_count.h"

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	int *ptr1 = new int(10);
	int *ptr2 = new int(20);
	int *ptr3 = new int(30);
	Hasptr one(1, ptr1);
	cout<<"one et_ptr_use():"<<one.get_ptr_use()<<endl;
	Hasptr eight(8, ptr3);
	cout<<"one get_ptr_val():"<<one.get_ptr_val()<<endl;
	one.set_ptr_val(0);
	cout<<"ptr1 is "<<ptr1<<endl;
	Hasptr two(1, ptr2);
	cout<<"two et_ptr_use():"<<two.get_ptr_use()<<endl;
	Hasptr three(two);  //it is in the Hasptr(Hasptr& orig)
	cout<<"two et_ptr_use():"<<two.get_ptr_use()<<endl;
	Hasptr four = two;	//it is in the Hasptr(Hasptr& orig)
	cout<<"two et_ptr_use():"<<two.get_ptr_use()<<endl;
	one = two;			//Hasptr&  operator = (Hasptr& orig)
	cout<<"one et_ptr_use():"<<one.get_ptr_use()<<endl;

	return 0;
}
