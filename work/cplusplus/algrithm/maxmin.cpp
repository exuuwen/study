// minmax example
#include <iostream>     // std::cout
#include <algorithm>    // std::minmax

int mytest(int a, int b){ return b;}	

struct myclass {
    int operator()(int x, int y) {return x+3*y;}
} myobject;

int main () {

	std::cout << "min(1,2)==" << std::min(1,2) << '\n';
	//min_functor(b,a) ? b :a
	std::cout << "min(1,2,mytest)==" << std::min(1,2,mytest) << '\n';
	std::cout << "max(1,2)==" << std::max(1,2) << '\n';
	//max_functor(a,b) ? b :a
	std::cout << "max(1,2,myobject)==" << std::max(1,2,myobject) << '\n';
	std::cout << "minmax({1,2,3,4,5}): ";

	auto result = std::minmax({1,2,3,4,5});
	std::cout << result.first << ' ' << result.second << '\n';
	return 0;
}
