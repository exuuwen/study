#include <iostream>     // std::cout
#include <algorithm>    // std::sort
#include <vector>       // std::vector

bool myfunction (int i, int j) { return (i<j); }

struct myclass {
  bool operator() (int i,int j) { return (i<j);}
} myobject;

bool myfunction1 (int i, int j) {
  return (i==j);
}
int main () 
{
	int myints[] = {32,71,2,12,2,45,26,2,80,53,33};
  	std::vector<int> myvector (myints, myints + 11);               // 32 71 2 12 2 45 26 2 80 53 33

	// using default comparison (operator <):
  	std::sort(myvector.begin(), myvector.begin() + 4);           

	std::cout << "myvector contains:";
	for (auto x: myvector) 
		std::cout << x << " ";
	std::cout << std::endl;
  	// using function as comp
  	std::sort(myvector.begin() + 4, myvector.end(), myfunction); 

	std::cout << "myvector contains:";
	for (auto x: myvector) 
		std::cout << x << " ";
	std::cout << std::endl;

	// using object as comp
  	std::sort(myvector.begin(), myvector.end(), myobject);     

  	// print out content:
	std::cout << "myvector contains:";
	for (auto x: myvector) 
		std::cout << x << " ";
	std::cout << std::endl;

	// just remove the same data consecutive
	std::vector<int>::iterator it;
  	it = std::unique(myvector.begin(), myvector.end()); 
	// std::unique(myvector.begin(), myvector.end(), myfunction1);
	myvector.erase(it, myvector.end());
	//myvector.resize(std::distance(myvector.begin(), it));
  	
	std::cout << "myvector contains:";
	for (auto x: myvector) 
		std::cout << x << " ";
	std::cout << std::endl;

	//unique_copy

	return 0;
}
