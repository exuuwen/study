
#ifndef OPERATOR_H_
#define OPERATOR_H_
#include <iostream>

using namespace std;


class CheckPtr
{
public:
	CheckPtr(int* begin, int* ended):beg(begin), cur(begin), end(ended){}
	CheckPtr& operator++()
	{
		if(cur == end)
			cout << "WARNING the CheckPtr is at end can not ++"<<endl;
		else
		{
			cur++;
		}
		return *this;
	}
	CheckPtr& operator--()
	{
		if(cur == beg)
			cout << "WARNING the CheckPtr is at beg can not --"<<endl;
		else
		{
			cur--;
		}
		return *this;
	}
	CheckPtr operator++(int)
	{
			CheckPtr temp(*this);
			++(*this);
			return temp;
	}
	CheckPtr operator--(int)
	{
			CheckPtr temp(*this);
			--(*this);
			return temp;
	}
	int get_cur_value() const
	{
		return *cur;
	}
private:
	int* beg;
	int* cur;
	int* end;
};




















#endif
