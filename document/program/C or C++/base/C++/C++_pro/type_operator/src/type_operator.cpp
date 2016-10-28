//============================================================================
// Name        : type_operator.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "type_operator.h"
void print_int(int a)
{
	cout<<"print_int:"<<a<<endl;
}
void print_int(double a)
{
	cout<<"print_double:"<<a<<endl;
}
void print_a(SmallInt a)
{
	cout<<"hahah"<<endl;
}
int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	SmallInt a(5);
	SmallInt c(4.5);
	int b=0;
	print_a(b);//convert int to SmallInt
	print_int(static_cast<int>(a));  //convert SmallInt to int//must be like this ,or there be  ambigu
	print_int(static_cast<int>(c)); //convert SmallInt to double
	return 0;
}
