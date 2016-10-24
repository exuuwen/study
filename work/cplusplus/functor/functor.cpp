#include <iostream>
using namespace std;

template <class T>
struct myplus {
	T operator()(const T& x, const T& y) const { return x + y; }
};

template <class T>
struct myminus {
	T operator()(const T& x, const T& y) const { return x - y; }
};

struct cminus {
	int operator()(const int x, const int y) const { return x - y; }
};

int main()
{
	myplus<int> plusobj;
	myminus<int> minusobj;

	cminus cminobj;

	cout << plusobj(3,5) << endl;
	cout << minusobj(3,5) << endl;
	cout << cminobj(10, 8) << endl;

// 以下直接產生仿函式的暫時物件(第一對小括號),並呼叫之(第二對小括號)。
	cout << myplus<int>()(43,50) << endl;
	cout << myminus<int>()(43,50) << endl;
	cout << cminus()(10, 8) << endl;
}
