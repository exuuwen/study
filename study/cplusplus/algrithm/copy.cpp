// copy algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::copy
#include <vector>       // std::vector

bool function(int i)
{
	return !(i<0);
}

int main () {

  	int myints[]={10, 20, 30, 40, 50, 60, 70};
	std::vector<int> myvector(7);
	std::vector<int> myvector2(5);

	//copy 不能插入空的容器中
	std::copy(myints, myints+7, myvector.begin()); //reverse_copy
	std::cout << "myvector contains:";
	for (auto item : myvector)
		std::cout << item << " ";
	std::cout <<std::endl;

	std::copy_n(myints, 5, myvector2.begin());
	std::cout << "myvector2 contains:";
	for (auto item : myvector2)
		std::cout << item << " ";
	std::cout <<std::endl;

	std::vector<int> foo = {25, 15, 5, -5, -15};
	std::vector<int> bar(foo.size());

	// copy only positive numbers:
	auto it = std::copy_if(foo.begin(), foo.end(), bar.begin(), function); 
	bar.resize(std::distance(bar.begin(), it));  // shrink container to new size

	std::cout << "bar contains:";
	for (auto x: bar) 
		std::cout << x << " ";
	std::cout << std::endl;

  	std::vector<int> a;

	for (int i=1; i<10; ++i) a.push_back(i);   // 1 2 3 4 5 6 7 8 9
	std::reverse(a.begin(), a.end());    // 9 8 7 6 5 4 3 2 1
	std::cout << "a contains:";
	for (auto x: a) 
		std::cout << x << " ";
	std::cout << std::endl;

  	return 0;
}
