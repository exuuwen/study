// replace algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::replace
#include <vector>       // std::vector

bool IsOdd (int i) { return ((i%2)==1); }

int main () 
{
	int myints[] = { 10, 20, 30, 30, 20, 10, 10, 20 };
  	std::vector<int> myvector (myints, myints + 8);            // 10 20 30 30 20 10 10 20

  	std::replace (myvector.begin(), myvector.end(), 20, 99); // 10 99 30 30 99 10 10 99

  	std::cout << "myvector contains:";
  	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    		std::cout << ' ' << *it;
  	std::cout << '\n';
  
  myvector.clear();

  // set some values:
  for (int i=1; i<10; i++) myvector.push_back(i);               // 1 2 3 4 5 6 7 8 9

  std::replace_if (myvector.begin(), myvector.end(), IsOdd, 0); // 0 2 0 4 0 6 0 8 0

  std::cout << "myvector contains:";
  for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';
  	return 0;
}
