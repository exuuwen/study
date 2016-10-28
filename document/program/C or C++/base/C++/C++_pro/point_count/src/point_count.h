/*
 * app.h
 *
 *  Created on: Oct 7, 2010
 *      Author: dragon
 */

#ifndef APP_H_
#define APP_H_
#include <iostream>
//#include <stdio.h>
using namespace std;



class U_ptr{
friend class Hasptr;
int *point;
size_t use;
U_ptr(int *p):point(p),use(1){}
~U_ptr() { delete point; }
};


class Hasptr{
	public :
		Hasptr(int value , int *p):val(value), ptr(new U_ptr(p)){}
		Hasptr(Hasptr& orig):val(orig.val), ptr(orig.ptr)
		{
			cout<<"it is in the Hasptr(Hasptr& orig)"<<endl;
			++ptr->use;
		}
		Hasptr&  operator=(const Hasptr& orig)
		{
			cout<<"xxxHasptr&  operator = (Hasptr& orig)"<<endl;
			++orig.ptr->use;
			//orig.val=0;
			cout<<"it is hh1"<<endl;
			if(--ptr->use == 0)
			{
				cout<<"it is hh2"<<endl;
				delete ptr ;
			}
			cout<<"it is hh3"<<endl;
			val = orig.val;
			ptr = orig.ptr;
			return (*this);
		}
		~Hasptr()
		{
			cout<<"it is in ~Hasptr()"<<endl;
			if(--ptr->use == 0)
			{
				cout<<"haha"<<endl;
				delete ptr;

			}
		}
		int get_val() const;
		void set_val(int val);
		int *get_ptr()  const
		{
			return ptr->point;
		}
		void set_ptr(int *p)
		{
			 ptr->point = p;
		}
		int get_ptr_val()  const
		{
			return *(ptr->point);
		}
		void set_ptr_val(int value)
		{
			 *(ptr->point) = value;
		}
		int get_ptr_use()
		{
			return ptr->use;
		}
	private:
		int val;
		U_ptr *ptr;
};



#endif /* APP_H_ */
