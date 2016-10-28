#include<iostream.h>
#include<string.h>
class Student 
{
public:	
	Student(char *pName="no name")
	{
		num++;
		strcpy(name,pName);
		cout<<"construct new student "<<pName<<endl;
		cout<<"the num  is  "<<num<<endl;
	}
	 Student(Student& s)
	{	
		num++;
		cout<<"construct copy of "<<s.name<<endl;
		cout<<"the num  is  "<<num<<endl;
		strcpy(name,"copy of ");
		strcat(name,s.name);
	}
	~Student()
	{
		num--;
		cout<<"Destruct  "<<name<<endl;
		cout<<"the num  is  "<<num<<endl;
	}
	static  int  nums()
	{
		return  num;
		//cout<<name<<endl;   error,不与对象相关
	}

protected:
	static int  num;
	char name[40];
};
int Student::num=0;

void main()
{ 
	Student  CR7("CR7");
	cout<<Student::nums()<<endl;
	
	Student   s1;

	Student  s2=Student("messi");
}