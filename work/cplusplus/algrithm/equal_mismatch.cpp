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

	/*
	for ( ; first1 !=last1; first1++, first2++) {
		 if (!binary_op(*first1, *first2)
			return false;
	}
	
	retrun true;
	*/

	auto const& mpair = mismatch(s1.begin(), s1.end(), s2.begin());
	auto const& iter1 = mpair.first;
	auto const& iter2 = mpair.second;
	if (iter1 != s1.end())
		cout << "first mismatch "<< *iter1 << endl;
	if (iter2 != s2.end())
		cout << "second mismatch "<< *iter2 << endl;

	/*
	while (first1 != last1 && binary_op(*first1, *first2) {
		first1++;
		first2++;
	}
	*/

	return 0;
}
