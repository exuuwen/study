/*
 * type_operator.h
 *
 *  Created on: Oct 27, 2010
 *      Author: CR7
 */

#ifndef TYPE_OPERATOR_H_
#define TYPE_OPERATOR_H_
#include <iostream>
using namespace std;

class SmallInt
{
public:
	SmallInt(int i = 0):val(i)//covert int to SmallInt
	{
		if(i < 0 || i >255)
		{
			cout<<"WARING!!!!,the i is not 0<i<255,we chage it to 0"<<endl;
			val = 0;
		}
		cout<<"in the SmallInt int"<<endl;
	}
	SmallInt(double i = 0):val(i)//covert double to SmallInt
	{
		if(i < 0 || i >255)
		{
			cout<<"WARING!!!!,the double i is not 0<i<255,we chage it to 0"<<endl;
			val = 0;
		}
		cout<<"in the SmallInt double"<<endl;
	}
	operator int() const //convert SmallInt to int
	{
		cout<<"in the operator int() "<<endl;
		return val;
	}
	operator double() const //convert SmallInt to double  ,define more operator() is not good, you qi yi
	{
		cout<<"in the operator double() "<<endl;
		return val;
	}
private:
	std::size_t val;
};


#endif /* TYPE_OPERATOR_H_ */
