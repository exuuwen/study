#include <stdio.h>
struct item
{
	char name[4];
	unsigned int index;
};
struct item_block
{
	int len;
	struct item addr[0];
};
int main()
{
	unsigned int  n = 10;
	printf("sizeof(struct item):%ld,sizeof(struct item_block):%ld\n", sizeof(struct item), sizeof(struct item_block));
	struct item_block *p = (struct item_block*)malloc(sizeof(struct item_block) + sizeof(struct item)*n);
	printf("p addr:0x%p\n", p);
	printf("p->addr addr:0x%p\n", p->addr);
	
	free(p);
	return 0;
}
/* struct item addr[0];叫柔性数组成员，char[0]是一种非标准形式，C99支持的标准形式用的是不完整类型：struct item addr[ ];柔性数组成员是这样使用的：

  struct item_block *p = new char[ sizeof( struct item ) + sizeof( struct item_block ) * n ];

于是( ( struct item_block* )p )->address就是sizeof(struct item ) * n这一段内存单元的首地址了。
addr在item_block中只是一个占位符，不具有大小，所以sizeof(struct item_block)为4，这种方式比struct item *addr方式有效 */
