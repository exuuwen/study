// generate algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::generate
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand

// function generator:
int RandomNumber() { return (std::rand()%100); }

// class generator:
struct c_unique {
	int current;
	c_unique() {current=0;}
	int operator()() {return ++current;}
} UniqueNumber;

int main() {
	std::srand(unsigned(std::time(0)));

	std::vector<int> myvector(8);

	std::generate (myvector.begin(), myvector.end(), RandomNumber);

	std::cout << "myvector contains:";
	for (auto& x : myvector)
		std::cout << x << " ";
	std::cout << std::endl;

	std::generate (myvector.begin(), myvector.end(), UniqueNumber);

	std::cout << "myvector contains:";
	for (auto& x : myvector)
		std::cout << x << " ";
	std::cout << std::endl;
 
	return 0;
}
