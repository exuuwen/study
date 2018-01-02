// search algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::search
#include <vector>       // std::vector

bool mypredicate (int i, int j) 
{
	return (i==j);
}

int main () 
{
	std::vector<int> haystack;

	// set some values:        haystack: 10 20 30 40 50 60 70 80 90
	for (int i=1; i<10; i++) haystack.push_back(i*10);

	// using default comparison:
	int needle1[] = {40,50,60,70};
	std::vector<int>::iterator it;
	it = std::search (haystack.begin(), haystack.end(), needle1, needle1+4);

	if (it!=haystack.end())
		std::cout << "needle1 found at position " << (it-haystack.begin()) << '\n';
	else
		std::cout << "needle1 not found\n";

	// using predicate comparison:
	int needle2[] = {20,30,50};
	it = std::search (haystack.begin(), haystack.end(), needle2, needle2+3, mypredicate);

	if (it!=haystack.end())
		std::cout << "needle2 found at position " << (it-haystack.begin()) << '\n';
	else
		std::cout << "needle2 not found\n";

	int myints[]={10,20,30,30,20,10,10,20};
	std::vector<int> myvector (myints,myints+8);

	// using default comparison:
	it = std::search_n (myvector.begin(), myvector.end(), 2, 30);

	if (it!=myvector.end())
		std::cout << "two 30s found at position " << (it-myvector.begin()) << '\n';
	else
		std::cout << "match not found\n";

	// using predicate comparison:
	it = std::search_n (myvector.begin(), myvector.end(), 3, 20, mypredicate);

	if (it!=myvector.end())
		std::cout << "three 20s found at position " << int(it-myvector.begin()) << '\n';
	else
		std::cout << "match not found\n";

	return 0;
}
