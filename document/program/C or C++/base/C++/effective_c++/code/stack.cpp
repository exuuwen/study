#include <iostream>
#include <string>
using namespace std;
class Stack
{
public:
	typedef int Data;
	Stack():top(0){}
	Data pop()
	{
		if(!empty())
		{
			StackNode* tmp = top;
			top =top->next;
			Data d = tmp->item;
			delete tmp;
			return d;
		}
		return -1;
	}
	void push(Data d)
	{
		top = new StackNode(d,top);
	}
	~Stack()
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
		Data  item;
		StackNode *next;
		StackNode(const Data& d, StackNode *nextnode):item(d),next(nextnode){}
	};
	StackNode*  top;

	Stack(const Stack& rhs);//forbid
	Stack& operator=(const Stack& rhs);//forbid
};

int main()
{
	Stack a;
	int tmp;
	if(a.empty())
		cout<<"Stack is empty"<<endl;
	for(int i=0; i<10; i++)
	{
		a.push(i);
		cout<<"push data:"<<i<<endl;	
	}
	if(a.empty())
		cout<<"Stack is empty"<<endl;
	for(int i=0; i<6; i++)
	{
		tmp = a.pop();
		cout<<"pop data:"<<tmp<<endl;
	}
	if(a.empty())
		cout<<"Stack is empty"<<endl;
	else 
		cout<<"Stack is not  empty"<<endl;
	for(int i=0; i<6; i++)
	{
		tmp = a.pop();
		cout<<"pop data:"<<tmp<<endl;
	}
	if(a.empty())
		cout<<"Stack is empty"<<endl;
}
