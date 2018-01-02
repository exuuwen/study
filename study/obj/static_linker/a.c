extern int shared;
int main()
{
	int a = 1;

	swap(&a, &shared);
	return 0;
}
