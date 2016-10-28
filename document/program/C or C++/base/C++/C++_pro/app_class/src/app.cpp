#include "app.h"

/*Screen::Screen(index cur,index hei,index len)
{
		cursor=cur;
		height=hei;
		length=len;
		//contents(0);
}*/
//void test_friend_func(Sale_item& s)


/////////////////////////////////////////
char Screen::get(index row, index len) const
{
	return contents[row*length+len];
}
Screen& Screen::set(index row, index len, char data)
{
	contents[row*length+len] = data;
	return (*this);
}
Screen& Screen::set(char data)
{
	contents[cursor] = data;
	return (*this);
}

Screen& Screen::move(index row, index len)
{
	cursor=row*length+len;
	return (*this);
}

/*////////////////////////////////////////////////

Sale_item

////////////////////////////////////////////////*/
const unsigned int Sale_item::orgin_price=15;

 /*void Sale_item::set_orgin_price(unsigned int pri)
{
	orgin_price=pri;
}*/
const string& Sale_item::get_isdn() const
{
	return isdn;
}

unsigned int Sale_item::get_orgin_price()
{
	return orgin_price;
}
double Sale_item:: get_avg_price() const
{
	return  ave_price;
}

bool Sale_item::same_book(const Sale_item &item)  const
{
	return (item.isdn==isdn);
}
unsigned int Sale_item::get_price() const
{
	return price;
}
unsigned int Sale_item::get_sold_num() const
{
	return sold_num;
}
void Sale_item::set_new_price(unsigned int pri)
{
	price=pri;
	return;
}
void Sale_item::sold_some_book(unsigned int num)
{
	sold_num+=num;
	revenue+=num*price;
	ave_price=revenue/sold_num;
}

double Sale_item::get_revenue() const
{
	return revenue;
}
