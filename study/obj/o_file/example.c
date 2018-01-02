int global_init_var = 1;
int global_uninit_var;
int global_zero_var = 0;
int printf(const char* format, ...);

__attribute__((section("wxztt"))) long long happy_life = 199;

void func(int data)
{
	printf("%d\n", data);
}

int main()
{
	static int static_init_var = 7;
	static int static_uninit_var;
	static int static_zero_var = 0;
	int a = 2;

	func(a);

	return 0;
}
