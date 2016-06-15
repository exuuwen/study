// xx_of example
#include <iostream>     // std::cout
#include <algorithm>    // std::xx_of
#include <array>        // std::array

bool odd(int i) { return i%2;}

int main () {
  
	std::array<int, 8> foo = {3,5,7,11,13,17,19,23};

	if (std::all_of(foo.begin(), foo.end(), [](int i){return i%2;}))
 		std::cout << "All the elements are odd numbers.\n";

	if (std::all_of(foo.begin(), foo.end(), odd))
 		std::cout << "All the elements are odd numbers.\n";

	
	std::array<int, 8> foo2 = {2,5,7,11,13,17,19,23};
	if (std::any_of(foo2.begin(), foo2.end(), odd))
 		std::cout << "there are elements are odd numbers.\n";

	std::array<int,8> foo3 = {1,2,4,8,16,32,64,128};
	if (std::none_of(foo3.begin(), foo3.end(), [](int i){return i<0;}))
		std::cout << "There are no negative elements in the range.\n";


	std::for_each(foo.begin(), foo.end(), [](int i){std::cout << i << " ";});
	std::cout << std::endl;

	return 0;
}
