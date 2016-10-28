#include <string>
#include <iostream>
#include <tr1/memory>

using namespace std;
using namespace std::tr1;

class parent;
class children;

typedef shared_ptr<parent> parent_ptr;
typedef shared_ptr<children> children_ptr;

typedef weak_ptr<parent> weak_ptrs;

class parent
{
public:
	parent() {cout <<"creating parent\n"; }	
	~parent() {cout <<"destroying parent\n"; }

public:
	children_ptr children;
};

class children
{
public:
	children() { cout <<"creating children\n"; }
	~children() { cout <<"destroying children\n"; }

public:
	weak_ptrs  parent;  //mostly member ptr should be weak one's, or the target of ptr will not be destroy until this class traget is destory. The target of ptr will nerver be destroy until there is no shared ptr point to it
	//parent_ptr parent;  /*it leads to undestroying both parent and child*/ 
};


void test()
{
	parent_ptr father(new parent());
	children_ptr son(new children());

	father->children = son;
	son->parent = father;
}

int main()
{
	cout<<"begin test...\n";
	test();
	cout<<"end test.\n";

	return 0;
}

