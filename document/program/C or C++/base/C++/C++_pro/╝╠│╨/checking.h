#ifndef  CKECKINGS
#define  CKECKINGS

#include"Account.h"

enum REMIT {post,cable,other};

class checking:public Account
{
public:
	checking(int mun,int bal=0);
	virtual void Withdraw(int);
	void Display();
	void setremit(REMIT re);
protected:
	REMIT remites;
};

#endif