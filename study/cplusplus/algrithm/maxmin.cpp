// minmax example
#include <iostream>     // std::cout
#include <algorithm>    // std::minmax

int mytest(int a, int b){ return b;}	

struct myclass {
    int operator()(int x, int y) {return x+3*y;}
} myobject;

int main () {

	std::cout << "min(1,2)==" << std::min(1,2) << '\n';
	//return comp(b, a) ? b :a
	std::cout << "min(1,2,mytest)==" << std::min(1,2,mytest) << '\n';
	std::cout << "max(1,2)==" << std::max(1,2) << '\n';
	//return comp(a, b) ? b :a
	std::cout << "max(1,2,myobject)==" << std::max(1,2,myobject) << '\n';
	std::cout << "minmax({1,2,3,4,5}): ";

	auto result = std::minmax({1,2,3,4,5});
	std::cout << result.first << ' ' << result.second << '\n';


	std::vector<int> myvector={10, 20, 30, 40, 50, 60, 70};

	auto const& iter_max = std::max_element(myvector.begin(), myvector.end());
	auto const& iter_min = std::min_element(myvector.begin(), myvector.end());

	std::cout << "max " << *iter_max << std::endl;
	/*
		while (++first!=last)
			if (comp(*result, *first)) result = first;
	*/

	std::cout << "min " << *iter_min << std::endl;
	/*
		while (++first!=last)
			if (comp(*first, *result)) result = first;
	*/

	return 0;
}
