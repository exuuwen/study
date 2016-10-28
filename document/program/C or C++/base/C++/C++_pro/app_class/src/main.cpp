//============================================================================
// Name        : main.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "app.h"


int main() {
	cout << "!!!Hello World haha!!!" << endl; // prints !!!Hello World!!!
	class Sale_item one("2222");
	class Sale_item two("1234");
	class Sale_item& three=one;
	const class Sale_item four("44444");
	four.get_avg_price();//can not call set_new_price  it is const

	one.set_new_price(20);
	one.sold_some_book(30);
	cout<<"isdn is "<<one.get_isdn()<<endl;
	cout<<"the price is "<<one.get_price()<<endl;
	cout<<"the revenue is "<<one.get_revenue()<<endl;
	cout<<"the ave_price is "<<one.get_avg_price()<<endl;
	cout<<"the sold_num is "<<one.get_sold_num()<<endl;
	cout<<"the orign_price is "<<one.get_orgin_price()<<endl;
	one.set_new_price(50);
	one.sold_some_book(20);
	cout<<"isdn is "<<one.get_isdn()<<endl;
	cout<<"the price is "<<one.get_price()<<endl;
	cout<<"the revenue is "<<one.get_revenue()<<endl;
	cout<<"the ave_price is "<<one.get_avg_price()<<endl;
	cout<<"the sold_num is "<<one.get_sold_num()<<endl;
	cout<<"the orign_price is "<<one.get_orgin_price()<<endl;

	class Screen a(0,4,6,"qwertyuiopasdfghjklzmxncbv;'p,jybdskfuh");
	cout<<"position cursor is "<<a.get()<<endl;
	cout<<"position (0,6) is "<<a.get(0,6)<<endl;
	cout<<"position (2,6) is "<<a.get(2,6)<<endl;
	a.set('a');
	a.set(0,6,'m');
	cout<<"position cursor is "<<a.get()<<endl;
	cout<<"position (0,6) is "<<a.get(0,6)<<endl;

	cout<<"position cursor is "<<a.move(2,6).get()<<endl;

	a.test_friend_class(three);
	test_friend_func(three);

	return 0;
}
