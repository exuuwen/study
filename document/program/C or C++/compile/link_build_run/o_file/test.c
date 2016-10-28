int global_init_var = 1;
int global_uninit_var;
int printf(const char* format, ...);

void func(int data)
{
	printf("%d\n", data);
}

int main()
{
	static int static_init_var = 7;
	static int static_uninit_var;
	int a = 2;

	func(a);

	return 0;
}
