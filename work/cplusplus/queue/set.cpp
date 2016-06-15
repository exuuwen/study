#include <string>
#include <iostream>
#include <stdio.h>
#include <set>

using namespace std;

int main()
{
	set<string> sset;

	sset.insert("wx");
	//set no [] operation

	pair<set<string>::iterator, bool> ret = sset.insert("wx");
	if(!ret.second)
	{
		cout << "insert wx fail" << endl;  // insert can fail i the key is exist
	}

	sset.insert("ztt");
	sset.insert("wty");
	sset.insert("wat");
	sset.insert("test");

	for (auto item : sset)
	{
		cout << item << " ";
	}

	cout << endl;

	int num = sset.count("ean"); //check key exist or not
	if(num)
	{
		cout << "ean" << " is exist" << endl;
	}
	else
	{
		cout << "ean" << " is not exist" << endl;
	}

	set<string>::iterator iter = sset.find("wx"); // find key
	if(iter != sset.end())
	{
		cout << "wx is exist value:" << *iter << endl;
	}
	else
	{
		cout << "wx is not exist" << endl;
	}
	
	if(sset.erase("test"))
	{
		cout << "test is exist and remove ok "<< endl;	
	}
	else
	{
		cout << "test is not exist and remove fail" << endl;
	}

	
	for(iter = sset.begin(); iter != sset.end(); iter++)
	{
		cout << *iter << " ";
	}

	cout << endl;

	return 0;
}
