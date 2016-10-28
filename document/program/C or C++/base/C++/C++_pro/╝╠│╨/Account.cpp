#include<iostream.h>
#include"Account.h"

Account* Account::pFirst=0;
int Account::count=0;

Account::Account(int Number,int Bal)
{
	Account* p;
	acntNumber=Number;
	balance=Bal;
	pNext=0;
	if(pFirst==0)
		pFirst=this;
	else
	{
		
		for(p=pFirst;p->pNext;p=p->pNext);
			p->pNext=this;
	}
	pNext=0;
	count++;
}
void Account:: Deposit(int money)
{
	balance+=money;
}
int Account:: Accountno()
{
	return count;
}
int Account:: Noaccounts()
{
	return acntNumber;
}
void Account::Display()
{
	cout<<"Account number :"<<acntNumber<<"="<<balance<<endl;
}
int  Account::Acntbalan()
{
	return balance;
}
Account* Account::First()
{
return  pFirst;
}
Account* Account::Next()
{
return  pNext;
}
void Account::Withdraw(int money)
{
	return;
}