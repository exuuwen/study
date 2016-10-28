#include<iostream.h>
#include<string.h>
class Student 
{
public:	
	Student(char *pName="no name",int  ssId=0)
	{
		id=ssId;
		strcpy(name,pName);
		cout<<"construct new student "<<pName<<endl;
	}
	 Student(Student& s)
	{
		cout<<"construct copy of "<<s.name<<endl;
		strcpy(name,"copy of ");
		strcat(name,s.name);
		id=s.id;
	}
	~Student()
	{
		cout<<"Destruct  "<<name<<endl;
	}

protected:
	char name[40];
	int id;
};

void fn(Student s)
{
	cout<<"In function fn()"<<endl;
}
Student one()
{
	Student  CR7("CR7",7);
	return   CR7;
}
void main()
{ 
	Student  CR7("CR7",7);
	Student   p1=CR7;
	Student s=one();
	Student  s=Student("CR7",7);
}