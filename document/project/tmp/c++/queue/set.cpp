#include <string>
#include <iostream>
#include <stdio.h>
#include <set>

using namespace std;

int main()
{
	set<string> sset;

	sset.insert("exuuwen");
	//set no [] operation

	pair<set<string>::iterator, bool> ret = sset.insert("exuuwen");
	if(!ret.second)
	{
		cout << "insert fail" << endl;  // insert can fail i the key is exist
	}

	sset.insert("esan");
	sset.insert("eqin");
	sset.insert("www");

	int num = sset.count("ean"); //check key exist or not
	if(num)
	{
		cout << "ean" << " is exist" << endl;
	}
	else
	{
		cout << "ean" << " is not exist" << endl;
	}

	set<string>::iterator iter = sset.find("exuuwen"); // find key
	if(iter != sset.end())
	{
		cout << "exuuwen is exist value:" << *iter << endl;
	}
	else
	{
		cout << "exuuwen is not exist" << endl;
	}
	
	if(sset.erase("eqin"))
	{
		cout << "eqin is exist and remove ok "<< endl;	
	}
	else
	{
		cout << "eqin is not exist and remove fail" << endl;
	}

	
	for(iter = sset.begin(); iter != sset.end(); iter++)
	{
		cout << "*iter" << ":" << *iter << endl;
	}


	return 0;
}
