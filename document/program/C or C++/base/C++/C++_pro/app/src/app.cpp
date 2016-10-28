//============================================================================
// Name        : app.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

# include"app.h"

int main() {
	int *p_z;
	int n=2;
	p_z =  new int[n];
	p_z[0] = 1;
	p_z[1] = 2;
	for(int i =0 ;i<n;i++)
	cout<<p_z[i]<<endl;

    delete [] p_z;

    int *p = new int;
    int *p_1 = new int(123);
    int *p_0 = new int();
    cout<<*p<<endl;
    cout<<*p_1<<endl;
    cout<<*p_0<<endl;

	return 0;
}
