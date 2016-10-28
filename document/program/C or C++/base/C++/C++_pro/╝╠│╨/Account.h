#ifndef account
#define account

class Account
{
public:
	Account(int Number,int Bal=0);
	virtual void Withdraw(int)=0;
	void Deposit(int money);
	int  Accountno();
	int  Noaccounts();
	void Display();
	int  Acntbalan();
	static Account* First();
	Account* Next();
protected:
	static Account* pFirst;
	Account* pNext;
	static int count;
	int acntNumber;
	int balance;
};

#endif