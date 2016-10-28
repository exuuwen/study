#include <iostream>
#include <string>
#include <list>
using namespace std;
//operator() (type)
class GT
{
public:
	GT(size_t len):bound(len){}
	bool operator()(const string& s)
	{
		return s.size() >= bound;
	}
private:
	size_t bound;
};
int main()
{
	string test("wwwwws");
	//
	GT gt(6);
	if(gt(test))
	{
		cout << "words length is six or more than six"<<endl;
	}
	else
	{
		cout << "words length is small than six"<<endl;
	}
}

