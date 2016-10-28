#include <iostream>
#include <string>
#include <tr1/functional>

using namespace std::tr1::placeholders;
using namespace std;
using namespace std::tr1;

class A 
{
public:
    A(const string& n) : name_(n) {}
    void printit(const string& s, int a) const 
    {
        cout << name_ << " says:" << s << a << endl;
    }
private:
    const string name_;
};

struct func
{
    void operator()(const string& s, int a) const
    {
        cout << "func() say:" << s << a << endl;
    }
};

void raw_func(const string& s, int a)
{
    cout << "raw_func say:" << s << a << endl;
}


int main()
{
    A a("Joe");
    function<void (const A& a, const string& s, int data)> f = &A::printit;
    f(a, "haha", 100);

    function<void (const string& s, int a)> f1 = raw_func;
    f1("haha1", 200);

    function<void (const string& s, int a)> f2 = func();
    f2("haha2", 300);

}
