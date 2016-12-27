

int foo()
{
	return 11;
}

__attribute__((visibility("hidden"))) int bar()
{
	return 12;
}
