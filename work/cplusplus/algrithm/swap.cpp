// swap algorithm example (C++98)
#include <iostream>     // std::cout
#include <algorithm>    // std::swap
#include <vector>       // std::vector

int main() 
{
	int x=10, y=20;                              // x:10 y:20
	std::swap(x, y);                              // x:20 y:10

	std::vector<int> foo(4, x), bar(6, y);       // foo:4x20 bar:6x10
	std::swap(foo, bar);                          // foo:6x10 bar:4x20

	std::cout << "foo contains:";
	for (auto& x : foo)
		std::cout << x << " ";
	std::cout << std::endl;

	std::vector<int> foo1(5,10);        // foo: 10 10 10 10 10
	std::vector<int> bar1(5,33);        // bar: 33 33 33 33 33

	std::swap_ranges(foo1.begin()+1, foo1.end()-1, bar1.begin());

	// print out results of swap:
	std::cout << "foo1 contains:";
	for (auto& x : foo1)
		std::cout << x << " ";
	std::cout << std::endl;

	std::cout << "bar1 contains:";
	for (auto& x : bar1)
		std::cout << x << " ";
	std::cout << std::endl;

	
	return 0;
}
