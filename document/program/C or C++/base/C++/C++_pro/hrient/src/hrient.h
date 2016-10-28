/*
 * hrient.h
 *
 *  Created on: Nov 12, 2010
 *      Author: dragon
 */

#ifndef HRIENT_H_
#define HRIENT_H_

#include <iostream>
using namespace std;

class Item_base
{
public:
	Item_base(const string & book = "", double pri = 0.0):isdn(book), price(pri){}
	/*Item_base(const Item_base& orgin)
	{

	}*/
	string book_isdn() const {return isdn;}
	virtual Item_base* clone() const {return new Item_base(*this);}
	virtual double net_price(size_t num) const  //virtual 参数为基类引用动态绑定
	{
		//cout << "in the item_base net_price" << endl;
		return price * num;
	}
	const string print_class_name()  const //都有  但是调用时看参数是那个类 就是哪个
	{
		return  "Item_base" ;
	}

	double book_price() const {return price;}
	virtual ~Item_base(){}
protected:
	double price;
private:
	string isdn;

};

class Disc_item : public Item_base
{
public:
	Disc_item(const string & book = "", double pri = 0.0, size_t qty = 0, double disc = 0.0):Item_base(book, pri), quantity(qty), discount(disc){}
	const string print_class_name() const  //都有  但是调用时看参数是那个类 就是哪个
	{
		return "Disc_item" ;
	}
	virtual Disc_item* clone() const = 0;
	virtual double net_price(size_t num) const = 0;
protected:
	size_t quantity;
	double  discount;
};

class Bulk_item : public Disc_item
{
public:
	Bulk_item(const string & book = "", double pri = 0.0, size_t qty = 0, double disc = 0.0):Disc_item(book, pri, qty, disc){}
	double net_price(size_t num) const
	{
		//cout << "in the bulk_item net_price" << endl;
		if(num >= quantity)
			return price * num * (1 - discount);
		else
			return price * num;
	}
	Bulk_item* clone() const {return new Bulk_item(*this);}
	const string print_class_name() const  //都有  但是调用时看参数是那个类 就是哪个
	{
		return "Bulk_item" ;
	}

};

class Sales_item
{
public:
	Sales_item():p(0), use(new size_t(1)){}
	Sales_item(const Item_base& item):p(item.clone()), use(new size_t(1)){}
	Sales_item(const Sales_item& item):p(item.p),use(item.use){++*use;}
	~Sales_item(){decr_use();}
	Sales_item& operator=(const Sales_item& item)
	{
		decr_use();
		p = item.p;
		use = item.use;
		++*use;
		return *this;
	}
	const Item_base& operator*() const
	{
		if(p)
			return *p;
		//else

	}
	const Item_base* operator->() const
	{
		if(p)
			return p;
		//else
	}
private:
	Item_base *p;
	size_t *use;
	void decr_use()
	{
		if(--*use == 0)
		{
			delete p;
			delete use;
		}
	}

};
void print_total(ostream &os, const Item_base &item, size_t n)
{
	os << "isdb:" << item.book_isdn() << endl;
	os << "price:" << item.book_price() << endl;
	os << "total num:" << n << endl;
	os << "total price:" << item.net_price(n) << endl;
	os << "class name is:" << item.print_class_name() <<endl;
}

class A
{
public:
		virtual void f(){ cout<<"in the A"<<endl;}
};

class B:public A
{
public:
		void f(int a){ cout<<"in the B"<<endl;}

};

#endif /* HRIENT_H_ */
