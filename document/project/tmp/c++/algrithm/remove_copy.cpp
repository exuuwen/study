// remove_copy example
#include <iostream>     // std::cout
#include <algorithm>    // std::remove_copy
#include <vector>       // std::vector

bool IsOdd (int i) { return ((i%2)==1); }

int main () {
  int myints[] = {10,20,30,30,20,10,10,20};               // 10 20 30 30 20 10 10 20
  std::vector<int> myvector(8);

  std::remove_copy (myints,myints+8,myvector.begin(),20); // 10 30 30 10 10 0 0 0

  std::cout << "myvector contains:";
  for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';

  int myint[] = {1,2,3,4,5,6,7,8,9};
  std::vector<int> myvector2(9);

  std::remove_copy_if (myint,myint+9,myvector2.begin(),IsOdd);

  std::cout << "myvector contains:";
  for (std::vector<int>::iterator it=myvector2.begin(); it!=myvector2.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';
  return 0;
}
