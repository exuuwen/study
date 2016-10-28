/*from this file we can conclude:



*/
#include <iostream>
using namespace std;
class A
{
public:
	A(int a = 0):data(a)
	{}
	A(const A &a)
	{
		cout<<"in the  A(const A &a)"<<endl;
		data = a.data;
	}
	const A& copy(const A& a)
	{
		cout<<"in the copy(const A& a)"<<endl;
		data = a.data;
		return *this;
	}
	A& operator=(const A& a)
	{
		cout<<"in the operator(const A& a)"<<endl;
		data = a.data;
		return *this;
	}
	~A(){}
private:
	int data;
};
int main()
{
	A t(2);
	const A* point = new A(1);
	const A& z = t;
	A value;

	value = z;/*call operator=*/
	value = *point;/*call operator=*/

	//A& quote = t.copy(z)/*z*/;/*invalid initialization of reference of type ‘A&’ from expression of type ‘const A’*/
	//A* p = point;/*error: invalid conversion from ‘const A*’ to ‘A*’*/
	
	const A* const_point = point;/*const_point -> const_point*/
	const A& const_quote = z;/*const_quote -> const_quote*/
	
	A& quote = t;
	const A const1 = t;
	//A& quote = const1; /*error: invalid initialization of reference of type ‘A&’ from expression of type ‘const A’*/
	const A& const_quote1 = const1;

	A* point1 = new A(2);
	A* tmp = point1;
	*point1 = t;
	point1 = &t; 
	
	
	//const A const_value1 = t;
	//point1 = &const_value1; /*invalid conversion from ‘const A*’ to ‘A*’*/
	
	delete point;
	delete tmp;
	return 0;
}
