#include <string>
#include <iostream>
#include <stdio.h>
#include <map>

using namespace std;

int main()
{

	typedef map<string, int>::value_type valType;

	map<string, int> dict;

	dict.insert(valType("exuuwen", 1));
	dict["exuuwen"] = 2; // as insert or modify key's value

	pair<map<string, int>::iterator, bool> ret = dict.insert(valType("exuuwen", 3));
	if(!ret.second)
	{
		cout << "inseart fail" << endl;  // insert can fail i the key is exist
	}

	dict.insert(valType("esan", 4));

	dict.insert(valType("eqin", 5));

	cout << "dict[exuuwen]:" << dict["exuuwen"] << endl;
	cout << "dict[esan]:" << dict["esan"] << endl;
	
	cout << "dict[www]:" << dict["www"] << endl; // [] operation may come out a new value ifkey is not exist

	int num = dict.count("ean"); //check key exist or not
	if(num)
	{
		cout << "dict[ean]" << " is exist" << endl;
	}
	else
	{
		cout << "dict[esan]" << " is not exist" << endl;
	}

	map<string, int>::iterator iter = dict.find("exuuwen"); // find key
	if(iter != dict.end())
	{
		cout << "dict[exuuwen] is exist value:" << iter->second << endl;
	}
	else
	{
		cout << "dict[exuuwen]" << " is not exist" << endl;
	}
	
	if(dict.erase("eqin"))
	{
		cout << "dict[eqin] is exist and remove ok "<< endl;	
	}
	else
	{
		cout << "dict[eqin] is not exist and remove fail" << endl;
	}

	
	for(iter = dict.begin(); iter != dict.end(); iter++)
	{
		cout << iter->first << ":" << iter->second << endl;
	}

	
	

	return 0;
}
