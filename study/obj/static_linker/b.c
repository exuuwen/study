int shared = 7;
void swap(int *a, int *b)
{
	*a ^= *b ^= *a ^= *b;
}
