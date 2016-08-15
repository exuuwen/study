#include <iostream>     // std::cout
#include <algorithm>    // std::equal
#include <vector>       // std::vector

using namespace std;

int main() 
{
	int a[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	vector<int> s1(a, a+5);
	vector<int> s2(a, a+9);

	cout << equal(s1.begin(), s1.end(), s2.begin()) << endl;
	cout << equal(s1.begin(), s1.end(), &a[3]) << endl;
	cout << equal(s1.begin(), s1.end(), s2.begin() + 3, less<int>()) << endl;

	return 0;
}
