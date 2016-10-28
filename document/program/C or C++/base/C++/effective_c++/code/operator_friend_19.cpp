#include <iostream>
#include <vector>
using namespace std;
class Rational
{
public:
	Rational(int n = 0, int d = 1):numerator(n),denominator(d){}
	
	friend const Rational  operator*(const Rational& rs1, const Rational& rs2)//必须是返回值，不能返回内部引用，除非*this//必须设为非成员函数才能使左边参数类型自动转换，否则就不能2*ratinal//参数为const
传引用有效
	{							     //必须是const 避免(a*b) = c;	
		return Rational(rs1.numerator*rs2.numerator, rs1.denominator*rs2.denominator);
	}
	int get_numerator()
	{
		return numerator;
	}
	int get_denominator()
	{
		return denominator;
	}
private:
	int numerator;
	int denominator;
};

int main()
{
	Rational r(5, 2);
	r = 2*r;
	int numerator = r.get_numerator();
	int denominator = r.get_denominator();
	cout<<"numerator:"<<numerator<<endl; 
	cout<<"denominator:"<<denominator<<endl;
	
	
}
