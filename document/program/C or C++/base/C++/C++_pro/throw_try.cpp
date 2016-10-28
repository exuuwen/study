#include <iostream>
#include <string>
#include <list>
using namespace std;
//operator() (type)
class Range_error
{
public:
	Range_error(int ii):i(ii){}
private:
	int i;	
};
char int_to_char(int i)
{
	if(i < -128 || i > 127)
		throw 	Range_error(i);	
	return i;
}
int main()
{
	try{
		char a = int_to_char(48);
		cout<<"a:"<<a<<endl;	
	}
	catch(Range_error)
	{
		cerr<<"oops!Range_error"<<endl;
	}
	try{
		char a = int_to_char(148);
		cout<<"a:"<<a<<endl;	
	}
	catch(Range_error)
	{
		cerr<<"oops!Range_error"<<endl;
	}
}
