#include <string>
#include <iostream>
#include <stdio.h>
#include <map>

using namespace std;

int main()
{

	typedef map<string, int>::value_type valType;

	map<string, int> dict;

	dict.insert(valType("wx", 1));
	dict["wx"] = 2; // as insert or modify key's value

	pair<map<string, int>::iterator, bool> ret = dict.insert(valType("wx", 3));
	if(!ret.second)
	{
		cout << "wx insert fail" << endl;  // insert can fail i the key is exist
	}

	dict.insert(valType("ztt", 1));
	dict.insert(valType("wty", 4));
	dict.insert(valType("wat", 5));

	cout << "dict[wx]:" << dict["wx"] << endl;
	cout << "dict[ztt]:" << dict["ztt"] << endl;
	
	cout << "dict[www]:" << dict["www"] << endl; // [] operation may come out a new value ifkey is not exist

	cout << "show-------------" << endl;
	for (auto & item : dict)
	{
		cout << item.first << ":" << item.second << endl;
	}

	int num = dict.count("ztt"); //check key exist or not
	if(num)
	{
		cout << "dict[ztt]" << " is exist" << endl;
	}
	else
	{
		cout << "dict[ztt]" << " is not exist" << endl;
	}

	map<string, int>::iterator iter = dict.find("wx"); // find key
	if(iter != dict.end())
	{
		cout << "dict[wx] is exist value:" << iter->second << endl;
	}
	else
	{
		cout << "dict[wx]" << " is not exist" << endl;
	}
	
	if(dict.erase("www"))
	{
		cout << "dict[www] is exist and remove ok "<< endl;	
	}
	else
	{
		cout << "dict[www] is not exist and remove fail" << endl;
	}

	
	cout << "show-------------" << endl;
	for(iter = dict.begin(); iter != dict.end(); iter++)
	{
		cout << iter->first << ":" << iter->second << endl;
	}

	
	

	return 0;
}
