// remove algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::remove

bool IsOdd (int i) { return ((i%2)==1); }

int main () 
{
	int myints[] = {10,20,30,30,20,10,10,20};      // 10 20 30 30 20 10 10 20

	// bounds of range:
	int* pbegin = myints;                          // ^
	int* pend = myints+sizeof(myints)/sizeof(int); // ^                       ^

	pend = std::remove(pbegin, pend, 20);         // 10 30 30 10 10 ?  ?  ?
                                                 // ^              ^
	std::cout << "range contains:";
	for (int* p=pbegin; p!=pend; ++p)
		std::cout << *p << " ";
	std::cout << std::endl;

	int myint[] = {1,2,3,4,5,6,7,8,9};            // 1 2 3 4 5 6 7 8 9

	// bounds of range:
	pbegin = myint;                          // ^
	pend = myint+sizeof(myint)/sizeof(int); // ^                 ^

	pend = std::remove_if(pbegin, pend, IsOdd);   // 2 4 6 8 ? ? ? ? ?
                                                 // ^       ^
	std::cout << "the range contains:";
	for (int* p=pbegin; p!=pend; ++p)
		std::cout << *p << " ";
	std::cout << std::endl;

	return 0;
}
