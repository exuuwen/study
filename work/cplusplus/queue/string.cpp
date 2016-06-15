#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
	string::size_type size;
	string s2("value data");
	if(!s2.empty())
	{
		size = s2.size();
		cout << "s2 size " << size << ":" << s2 << endl;
		cout << "s2[0]:" << s2[0] << endl;
	}
	else
	{
		cout << "s2 is empty" << endl;
	}

	string s3(s2);/*[]:can noly refer to exist items*/
	//s3[10] = 's';  /*error*/
	if(!s3.empty())
	{
		size = s3.size();
		cout << "s3 size " << size << ":" << s3 << endl;
		cout << "s3[0]:" << s3[0] << endl;
	}
	else
	{
		cout << "s3 is empty" << endl;
	}

	string s4(5, 'c');
	if(!s4.empty())
	{
		size = s4.size();
		cout << "s4 size " << size << ":" << s4 << endl;
		cout << "s4[0]:" << s4[0] << endl;
	}
	else
	{
		cout << "s4 is empty" << endl;
	}

	string s5(s4, 3);/*n can't out of range*, s4 can be char* s4 = {'a', 'b', 'c', 'd', ...}*/
	size = s5.size();
	cout << "s5 size " << size << ":" << s5 << endl;
	cout << "s5[0]:" << s5[0] << endl;

	string s6(s3, 3, 2);/*pos1+n can't out of range*i, s3 can be char* s3 = {'a', 'b', 'c', 'd', .....}*/
	size = s6.size();
	cout << "s6 size " << size << ":" << s6 << endl;
	cout << "s6[0]:" << s6[0] << endl;
		
	if(s2 == s3)
	{
		cout << "s2 == s3" << endl;
	}

	s3 = s4;
	cout << "new s3:" << s3 << endl;

	s2 += s3;
	cout << "s2 + s3:" << s2 << endl;

//substr(p, n)
	string sh("hello world");
	string sh2 = sh.substr(0, 5); // s2 = hello
	cout << "sh2:" << sh2 << endl; 
	string sh3 = sh.substr(6); // s3 = world
	cout << "sh3:" << sh3 << endl; 
	string sh4 = sh.substr(6, 11); // s3 = world
	cout << "sh4:" << sh4 << endl; 
	//string sh5 = sh.substr(12);  //out of range

//find(): if it does not find, return string::npos
	string sf("hello llollhew");
	string sw("ll");
	cout << "s find l:" << sf.find('l') << endl; //rfind:from backward
	cout << "s find l:" << sf.find('l', 5) << endl; //rfind:from backward
	cout << "s find ll:" << sf.find(sw) << endl; //rfind:from backward, sw can be char *ws = {};
	cout << "s find ll:" << sf.find(sw, 3) << endl; //rfind:from backward, sw can be char *ws = {};


	string sm("oew");//sw can be char *ws = {};
	cout << "s find (oew):" << sf.find_first_of(sm) << endl; //find_last_of:from backward
	cout << "s find (oew):" << sf.find_first_of(sm, 5) << endl; //find_last_of:from backward
	cout << "s find not (oew):" << sf.find_first_not_of(sm) << endl; //find_last_not_of:from backward
	cout << "s find not (oew):" << sf.find_first_not_of(sw, 3) << endl; //find_last_not_of:from backward

//Numberic:-std=c++0x
	cout << to_string(10) << " " << to_string(1.23) << endl;
	string si("10");
	string sl("100000000");
	string sff("2.3");
	string sd("2.1234567");
	cout << stoi(si) << " " << stol(sl) << endl;
	cout << stof(sff) << " " << stod(sd) << endl;

//string to char*
	const char *p = s2.c_str();  // return a const pointer;
	printf("*p %s\n", p);

	//char* to string
	string ss(p);
	cout<< "ss:" << ss << endl;
	
	string s;
	//end-of-file uot of while: ctrl+D 
	cout << "input a line or end with ctrl+D" << endl;
	while(getline(cin, s))  //get a line just only a enter can end it
	{
		cout << "input" << s << endl;
		cout << "input a line or end with ctrl+D" << endl;
	}

	return 0;
}

