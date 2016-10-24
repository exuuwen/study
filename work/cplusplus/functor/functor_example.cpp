#include <iostream>
#include <functional>
using namespace std;

int main()
{
//arithmetic
	
	plus<int> plusobj;
	minus<int> minusobj;
	multiplies<int> multipliesobj;
	divides<int> dividesobj;
	modulus<int> modulusobj;
	negate<int> negateobj;

	cout << "arithmetic" << endl;
	cout << plusobj(3, 5) << endl;
	cout << minusobj(3, 5) << endl;
	cout << multipliesobj(3, 5) << endl;
	cout << dividesobj(3, 5) << endl;
	cout << modulusobj(3, 5) << endl;
	cout << negateobj(5) << endl;

//relation
	equal_to<int> equal_to_obj;
	not_equal_to<int> not_equal_to_obj;
	greater<int> greater_obj;
	greater_equal<int> greater_equal_obj;
	less<int> less_obj;
	less_equal<int> less_equal_obj;

	cout << "relation" << endl;
	cout << equal_to_obj(3, 5) << endl;
	cout << not_equal_to_obj(3, 5) << endl;
	cout << greater_obj(3, 5) << endl;
	cout << greater_equal_obj(3, 5) << endl;
	cout << less_obj(3, 5) << endl;
	cout << less_equal_obj(3, 5) << endl;

//logic
	logical_and<int> logic_and_obj;
	logical_or<int> logic_or_obj;
	logical_not<int> logic_not_obj;

	cout << "logical" << endl;
	cout << logic_and_obj(true, true) << endl;
	cout << logic_or_obj(true, true) << endl;
	cout << logic_not_obj(true) << endl;

	return 0;

}
