#include <iostream>     // std::cout
#include <algorithm>    // std::count
#include <vector>       // std::vector

bool IsOdd (int i) { return ((i%2)==1); }

int main () 
{
	// counting elements in array:
	int myints[] = {10,20,31,33,25,10,10,23};   // 8 elements
	int mycount = std::count(myints, myints+8, 10);
	std::cout << "10 appears " << mycount << " times.\n";

	// counting elements in container:
	std::vector<int> myvector (myints, myints+8);
	mycount = std::count_if(myvector.begin(), myvector.end(), IsOdd);
	std::cout << "odd " << mycount  << " times.\n";

	/*
	for (; first!=last first++) {
		if (binary_Op(*first)
			++n;
	}
	
	return n;
	*/

	return 0;
}
