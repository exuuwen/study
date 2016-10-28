#include<iostream.h>
#include"Account.h"
#include"checking.h"

checking::checking(int mun,int bal):Account(mun,bal)
{
	remites=other;
}
void checking::Display()
{
	cout<<"checking number :"<<acntNumber<<"="<<balance<<endl;
}
void checking::Withdraw(int money)
{
	if(remites==post)
		money=money+10;
	else if(remites==cable)
		money=money+20;
		else  money=money+30;
	if(balance>=money)
		balance-=money;
	else
		cout<<"you don not have  enough  money"<<endl;
}
void checking::setremit(REMIT re)
{
	remites=re;
}