// fill algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::fill
#include <vector>       // std::vector

int main () 
{
	std::vector<int> myvector(8);                       // myvector: 0 0 0 0 0 0 0 0

	//vector must have the space
	std::fill(myvector.begin(), myvector.begin() + 4, 5);   // myvector: 5 5 5 5 0 0 0 0
	std::fill(myvector.begin() + 3, myvector.end() - 2, 8);   // myvector: 5 5 5 8 8 8 0 0
	std::fill_n (myvector.end() - 2, 2, 3);   // myvector: 5 5 5 8 8 8 3 3

	std::cout << "myvector contains:";
	for (auto const& item : myvector)
		std::cout << item << " ";
	std::cout << std::endl;

	return 0;
}
