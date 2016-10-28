#include<iostream.h>
#include"Account.h"
#include"saving.h"

int saving::minbalance=500;
saving::saving(int num,int bal)
:Account(num,bal)
{}
void saving::Withdraw(int money)
{
	if(minbalance+balance>=money)
		balance-=money;
	else
		cout<<"you don not have  enough  money"<<endl;
}
void saving::Display()
{
	cout<<"saving number :"<<acntNumber<<"="<<balance<<endl;
}