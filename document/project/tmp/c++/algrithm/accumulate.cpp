#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace std;

int myfunction (int x, int y) {return x + 2*y;}
int main()
{
	vector<int> v1;

	for(vector<int>::size_type i=0; i<10; i++)
	{
		v1.push_back(i);
	}

//accumulate: operator+

	int sum = accumulate(v1.begin(), v1.end(), 1000);
	cout << "sum is " << sum << endl;

	sum = accumulate(v1.begin(), v1.end(), 1000, myfunction);
        cout << "sum2 is " << sum << endl;

	return 0;
}

