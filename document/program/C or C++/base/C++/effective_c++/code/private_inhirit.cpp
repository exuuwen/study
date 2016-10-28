#include <iostream>
#include <string>
using namespace std;

//has-a  利用私有成员进行实现 但是遇到需要虚函数，和私有成员不能直接让用户访问时  就用private_inhirit
class GenericStack
{
protected:
	GenericStack():top(0){}
	void* pop()
	{
		if(!empty())
		{
			StackNode* tmp = top;
			top =top->next;
			void* d = tmp->item;
			delete tmp;
			return d;
		}
		return 0;
	}
	void push(void *d)
	{
		top = new StackNode(d,top);
	}
	~GenericStack()
	{
		while(top)
		{
			StackNode* tmp = top;
			top =top->next;
			delete tmp;
		}
	}
	bool empty() const
	{
		return top == 0;
 	}

private:
	struct StackNode
	{
		void*  item;
		StackNode *next;
		StackNode( void* d, StackNode *nextnode):item(d),next(nextnode){}
	};
	StackNode*  top;

	GenericStack(const GenericStack& rhs);//forbid
	GenericStack& operator=(const GenericStack& rhs);//forbid
};

class IntStack:private GenericStack
{
public:
	void push(int * data)
	{
		GenericStack::push(data);
	}
	int* pop()
	{
		return  static_cast<int*>(GenericStack::pop());
		
	}
	bool empty() const
	{
		return GenericStack::empty();
	}
};

int main()
{
	//GenericStack s;  //error
	IntStack  is;
	int a0 = 0;
	int a1 = 1;
	int a2 = 2;
	int a3 = 3;
	int a4 = 4;
	int a5 = 5;
	
	if(is.empty())
		cout<<"Stack is empty"<<endl;
	else 
		cout<<"Stack is not  empty"<<endl;
	is.push(&a4);
	is.push(&a1);
	is.push(&a2);
	is.push(&a3);
	
	if(is.empty())
		cout<<"Stack is empty"<<endl;
	else 
		cout<<"Stack is not  empty"<<endl;
	
	for(int i=0; i<2; i++)
	{		
		if(!is.empty())
		{
			int* tmp = is.pop();
			cout<<"pop data:"<<*tmp<<endl;
		}
		else
		{
			cout<<"Stack is empty"<<endl;
		}
	}

	is.push(&a0);
	is.push(&a5);
	is.push(&a3);
	
	for(int i=0; i<8; i++)
	{		
		if(!is.empty())
		{
			int* tmp = is.pop();
			cout<<"pop data:"<<*tmp<<endl;
		}
		else
		{
			cout<<"Stack is empty"<<endl;
		}
	}

	//cout<<"pop data:"<<*tmp<<endl;
	return 0;




	
}
