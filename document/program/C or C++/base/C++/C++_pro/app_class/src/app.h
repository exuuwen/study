/*
 * app.h
 *
 *  Created on: Oct 7, 2010
 *      Author: dragon
 */

#ifndef APP_H_
#define APP_H_
#include <iostream>
//#include <stdio.h>
using namespace std;


class Sale_item{
	public:
	friend void test_friend_func(Sale_item& s)
	{

			double kk;
			kk=s.revenue;
			cout<<"revenue is "<<kk<<endl;

	}
	friend class Screen;
	Sale_item(const std::string& book_id=""):isdn(book_id),price(5),sold_num(0),ave_price(0.0),revenue(0.0){}
	double get_avg_price() const;
	bool same_book(const Sale_item &item) const;
	unsigned int get_price() const;
	double get_revenue() const;
	unsigned int get_sold_num() const;
	void	set_new_price(unsigned int pri);
	void    sold_some_book(unsigned int num);
	static  unsigned int get_orgin_price() ;
	//void set_orgin_price(unsigned int pri);
	const string& get_isdn() const;//how to back to string& only const string& ,const & can get a data from constant
	private:
		string isdn;
		unsigned int price;
		unsigned int sold_num;
		double ave_price;
		double revenue;
		const static unsigned int orgin_price;  //static and read only
};


class Screen{
	public :

	typedef string::size_type  index;

	Screen(index cur=0,index hei=20,index len=50,const std::string& content=""):cursor(cur),height(hei),length(len),contents(content){ };
	char get(){return contents[cursor];}
	char get(index row, index len) const;
	Screen& set(index row, index len, char data);
	Screen& set(char data);
	Screen& move(index row,index len);
	void test_friend_class(Sale_item& s)
	{
		double kk;
		kk=s.revenue;
		cout<<"revenue is "<<kk<<endl;
	}
	private:
	index cursor;
	index height,length;
	string contents;


};



#endif /* APP_H_ */
