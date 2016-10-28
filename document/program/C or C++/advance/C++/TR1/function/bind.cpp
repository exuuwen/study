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
    void printit(const std::string& s, int a) const 
    {
        cout << name_ << " says " << s << a << endl;
    }
private:
    const string name_;
};

int main()
{
    A a("Joe");
    function<void ()> f = bind(&A::printit, &a, "hahah", 20);
    f();
    function<void (const string &s, int a)> f1 = bind(&A::printit, &a, _1, _2);
    f1("hello", 2);
    
    function<void (const string &s)> f2 = bind(&A::printit, &a, _1, 2);
    f2("wwwaa");
    function<void (int a)> f3 = bind(&A::printit, &a, "dddsa", _1);
    f3(100);

}
