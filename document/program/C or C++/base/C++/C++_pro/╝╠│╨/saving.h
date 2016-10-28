#ifndef savings
  #define savings

#include"Account.h"

class saving:public Account
{
public:
	saving(int,int);
	virtual void Withdraw(int);
	void Display();
protected:
	static int minbalance;
};
 
#endif