//============================================================================
// Name        : hrient.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "hrient.h"

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	Item_base base_one("base_1", 20.0);
	Item_base base_two("base_2", 10.0);
	Bulk_item bulk_one("bulk_1", 20.0, 5, 0.2);
	Bulk_item bulk_two("bulk_2", 20.0, 10, 0.2);
	//Item_base *b1 = &bulk_one;
	print_total(cout, base_one, 10);

	print_total(cout, base_two, 15);

	print_total(cout, bulk_one, 10);

	print_total(cout, bulk_two, 5);

	Sales_item sale_one(base_one);
	Sales_item sale_two(bulk_one);
	//b1->net_price();
	print_total(cout, *sale_one , 10);
	print_total(cout, *sale_two, 10);
	cout << sale_two->net_price(10) << endl;

	A a;
	B b;
	A *p = &b;
	A &r = b;
	p->f();
	r.f();
	//b.f();
	return 0;
}
