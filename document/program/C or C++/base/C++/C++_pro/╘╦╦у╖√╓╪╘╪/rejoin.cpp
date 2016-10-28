#include<iostream.h>

class  RMB
{
public:
	RMB(unsigned int y,unsigned int j)
	{
		yuan=y;
		jf=j;
		while(jf>100)
		{
			yuan++;
			jf-=100;
		}
	}
	RMB(double data)
	{
		yuan=data;
		jf=(data-yuan)*100;
	}

	void display()
	{
		cout<<"it is RMB **";
		cout<<"yuan is "<<yuan;
		cout<<" ** jf is "<<jf<<endl;
	}
	 RMB operator +(RMB& s)                //c=a+b
	 {
		unsigned int y=yuan+s.yuan;
		unsigned int j=jf+s.jf;
		while(jf>100)
		{
			y++;
			j-=100;
		}
		return RMB(y,j);

	 }
		friend RMB operator +(RMB& s,double data);  //c=b+2.7
		friend RMB operator +(double data,RMB& s);  //c=2.7+b
		
		RMB& operator +=(RMB& s)    //c+=b
		 {
			jf+=s.jf;
			yuan+=s.yuan;
			 while(jf>100)
			{
				yuan++;
				jf-=100;
			}
			return *this;			 
		 }

		RMB& operator +=(double data)  //c+=2.7
		 {
			jf+=(data-(int)data)*100;
			yuan+=(int)data;
			 while(jf>100)
			{
				yuan++;
				jf-=100;
			}
			return *this;			 
		 }
	
		RMB& operator ++()  //++c;
	 {
		 jf++;
		 if(jf>100)
		{
			yuan++;
			jf-=100;
		}
		return *this;
	 }

		RMB operator ++(int)  //c++
		{
			RMB temp(*this);
			jf++;
			if(jf>100)
			{
				yuan++;
				jf-=100;
			}
			return temp;
		}

private:
	
		unsigned int yuan;
		unsigned int jf;
	
};




		 RMB operator +(RMB& s,double data)
	 {
		unsigned int y=s.yuan+(int)data;
		unsigned int j=s.jf+(data-int(data))*100;
		while(j>100)
		{
			y++;
			j-=100;
		}
		return RMB(y,j);
	 }

		 RMB operator +(double data,RMB& s)
		 {
			 return s+data;
		 }
void main()
{
	RMB d1(1,60);
	RMB d2(2,50);
	RMB d3(0,0);
	/*RMB d4(20.16);
	d3=d2+d1;
	d3.display();
	++(++d3);
	d3.display();
	d4.display();
	d4=2.7+d3;
	d4.display();*/
	//d1+=d2;
	//d1.display();
	//d2+=3.8;
	d3=++d2;
	d2.display();
	d3.display();
	(d3=d1)++;
	d3.display();
}