#include<iostream.h>
#include"Account.h"
#include"saving.h"
#include"checking.h"
int  main()
{
//	Account on(2,1);
	saving one(1,20);
	saving t(2,2);
	saving ts(3,2);
	cout<<one.Accountno()<<endl;
	cout<<one.Acntbalan()<<endl;
	saving* p;
	p=(saving*)one.Next();
	if(p!=0)
	{
		int a;
	a=p->Noaccounts();
	cout<<a<<endl;
	}
	checking two(2,30);
	
	one.Deposit(15);
	one.Display();
	one.Withdraw(500);
	one.Display();
    one.Withdraw(500);
	one.Display();
	two.Display();
	two.Deposit(30);
	two.Display();
	two.Withdraw(10);
	two.Display();
	//two.setremit(post);
	two.Withdraw(10);
    two.Display();

return 0;
}