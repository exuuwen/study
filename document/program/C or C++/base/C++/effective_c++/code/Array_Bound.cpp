#include <iostream>
#include <vector>
using namespace std;
class Array
{
public:
	Array(int lowBound, int highBound):lBound(lowBound),hBound(highBound), data(highBound - lowBound + 1){}

	Array(const Array& rhs):lBound(rhs.lBound),hBound(rhs.hBound), data(rhs.hBound - rhs.lBound + 1)
	{
		data = rhs.data;
	}
	
	int& operator[](int index)
	{
		return data[index - lBound];
	}
	
	const int& operator[](int index) const
	{
		return data[index - lBound];
	}

	Array& operator=(const Array& rhs)
	{
		if(this == &rhs)
			return *this;
		lBound = rhs.lBound;
		hBound = rhs.hBound;
		data = rhs.data;
		
	}
	int lowBound() const
	{
		return lBound;
	}

	int highBound() const
	{
		return hBound;
	}
	
	int get_array_length()  const
	{
		return hBound - lBound + 1;
	}
	
	friend  ostream& operator<<(ostream& os, const Array& rhs)  //不能是成员函数 
	{
		for(int i = rhs.lBound; i <= rhs.hBound; i++)
			os<<"data["<<i<<"]:"<<rhs.data[i - rhs.lBound]<<endl;
		return os;
	}
	~Array(){}	
private:
	//Array& operator=(const Array& rhs)；/*如果要禁止该功能就这样来*/
	int lBound, hBound;
	vector<int> data;
	
};

int main()
{
	Array array(10, 20);
	int low = array.lowBound();
	int high = array.highBound();
	int length = array.get_array_length();
	
	cout<<"low:"<<low<<endl;
	cout<<"high:"<<high<<endl;
	cout<<"length:"<<length<<endl;	

	cout<<array<<endl;
	
	for(int i = low; i <= high; i++)
		array[i] = i;

	cout<<array<<endl;

	Array array_bk(array);
	
	array_bk[12] = 88;

	cout<<array_bk<<endl;

	Array empty(0, 0);
	cout<<empty<<endl;

	empty = array;
	empty[12] = 99;
	
	cout<<empty<<endl;	
	
}
