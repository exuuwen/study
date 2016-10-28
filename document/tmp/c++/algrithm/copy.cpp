// copy algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::copy
#include <vector>       // std::vector

bool function(int i)
{
    return !(i<0);
}

int main () {
  int myints[]={10,20,30,40,50,60,70};
  std::vector<int> myvector (7);

  std::copy ( myints, myints+7, myvector.begin() );

  std::cout << "myvector contains:";
  for (std::vector<int>::iterator it = myvector.begin(); it!=myvector.end(); ++it)
    std::cout << ' ' << *it;

  std::cout << '\n';


  std::vector<int> foo = {25,15,5,-5,-15};
  std::vector<int> bar (foo.size());

  // copy only positive numbers:
  auto it = std::copy_if (foo.begin(), foo.end(), bar.begin(), function); 
  bar.resize(std::distance(bar.begin(),it));  // shrink container to new size

  std::cout << "bar contains:";
  for (int& x: bar) std::cout << ' ' << x;
  std::cout << '\n';

  //reverse_copy
  return 0;
}
