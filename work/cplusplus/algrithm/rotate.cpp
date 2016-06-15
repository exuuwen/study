// rotate algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::rotate
#include <vector>       // std::vector

int main () {
	std::vector<int> myvector;

	// set some values:
	for (int i=1; i<10; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9

	std::rotate(myvector.begin(),myvector.begin()+3,myvector.end());
                                                  // 4 5 6 7 8 9 1 2 3
	// print out content:
	std::cout << "myvector contains:";
	for (auto& x:myvector)
		std::cout << x << " ";
  	std::cout << '\n';

	int myints[] = {10,20,30,40,50,60,70};

	std::vector<int> myvector2(7);

	std::rotate_copy(myints, myints+3, myints+7, myvector2.begin());
	
	// print out content:
  	std::cout << "myvector2 contains:";
	for (auto& x:myvector2)
		std::cout << x << " ";
  	std::cout << '\n';

	return 0;
}
