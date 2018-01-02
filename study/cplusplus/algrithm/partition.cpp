// partition algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::partition
#include <vector>       // std::vector

bool IsOdd (int i) { return (i%2)==1; }

int main () {
	std::vector<int> myvector;

	// set some values:
	for (int i=1; i<10; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9
	
	std::vector<int>::iterator bound;
	bound = std::partition (myvector.begin(), myvector.end(), IsOdd);

	// print out content:
  	std::cout << "odd elements:";
	for (std::vector<int>::iterator it=myvector.begin(); it!=bound; ++it)
		std::cout << ' ' << *it;
  	std::cout << '\n';

	std::cout << "even elements:";
	for (std::vector<int>::iterator it=bound; it!=myvector.end(); ++it)
		std::cout << ' ' << *it;
	std::cout << '\n';


	//partition_copy
	std::vector<int> foo {1,2,3,4,5,6,7,8,9};
	std::vector<int> odd, even;

	// resize vectors to proper size:
	unsigned n = std::count_if (foo.begin(), foo.end(), IsOdd);
	odd.resize(n); even.resize(foo.size()-n);

	// partition:
	std::partition_copy(foo.begin(), foo.end(), odd.begin(), even.begin(), IsOdd);

	// print contents:
	std::cout << "odd: ";  for (int& x:odd)  std::cout << ' ' << x; std::cout << '\n';
	std::cout << "even: "; for (int& x:even) std::cout << ' ' << x; std::cout << '\n';

	return 0;
}
