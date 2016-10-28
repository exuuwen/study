#include<iostream.h>
#include"Tdate.h"


     	   Student::Student()
	{ 
		hours=10; gpa=3.5;
	    cout<<"construct   student \hours: "<<hours<<"  \gpa: "<<gpa<<endl;
	}
	
	   Student::Student(int h,float g)
	{ 
		hours=h; gpa=g;
	    cout<<"construct   student \hours: "<<hours<<"  \gpa: "<<gpa<<endl;
	}
	   Student::~Student()
	   {
			cout<<"ddddeconstruct   student"<<endl;
	   }
	
	   Teacher::Teacher()
	   {
		cout<<"construct  teacher\n";
	   
	   }

	    Teacher::~Teacher()
	   {
		cout<<"deeeeconstruct  teacher\n";
	   
	   }


		Tutorpair::Tutorpair(int h,float g):student(h,g)

	   {
	    meetimes=0;
		cout<<"construct Tutorpair   meetime: "<<meetimes<<endl;
	   }

		Tutorpair::Tutorpair()
		{
			meetimes=0;
		cout<<"construct Tutorpair   meetime: "<<meetimes<<endl;
		}
	   Tutorpair::~Tutorpair()

	   {
	   
		cout<<"deeedconstruct Tutorpair   "<<endl;
	   }
 