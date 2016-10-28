#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#define NCHUNKS 16
#define CHUNK0SIZE 1024

#define JT(b)  ((b)->et.succ)
#define JF(b)  ((b)->ef.succ)

typedef  unsigned int u_int;

static int n_blocks;
static  u_int cur_block_id;
static  u_int cur_edge_id;

static struct block *root;
static struct bpf_insn *fstart;
static struct bpf_insn *ftail;

struct bpf_insn {
	u_int	data;
	u_char 	jt;
	u_char 	jf;
	
};
/*
 * A block is marked if only if its mark equals the current mark.
 * Rather than traverse the code array, marking each item, 'cur_mark' is
 * incremented.  This automatically makes each element unmarked.
 */
static int cur_mark;
#define isMarked(p) ((p)->mark == cur_mark)
#define unMarkAll() cur_mark += 1
#define Mark(p) ((p)->mark = cur_mark)


struct chunk {
	u_int n_left;
	void *m;
};


struct edge {
	int id;
	struct block *succ;
	//struct block *pred;
	//struct edge *next;	/* link list of incoming edges for a node */
};

struct block {
	int id;
	int private_data;
	int offset;
	int mark;
	int sense;
	struct edge et;
	struct edge ef;
	struct block *head;
	
};


static inline struct block *
new_block(data)
	int data;
{
	struct block *p;

	p = (struct block *)newchunk(sizeof(*p));
	p->private_data = data;
	p->head = p;

	return p;
}

static void
backpatch(list, target)
	struct block *list, *target;
{
	struct block *next;

	while (list) {
		if (!list->sense) {
			next = JT(list);
			JT(list) = target;
		} else {
			next = JF(list);
			JF(list) = target;
		}
		list = next;
	}
}

static void
merge(b0, b1)
	struct block *b0, *b1;
{
	register struct block **p = &b0;

	/* Find end of list. */
	while (*p)
		p = !((*p)->sense) ? &JT(*p) : &JF(*p);

	/* Concatenate the lists. */
	*p = b1;
}

void
gen_and(b0, b1)
	struct block *b0, *b1;
{
	backpatch(b0, b1->head);
	b0->sense = !b0->sense;
	b1->sense = !b1->sense;
	merge(b1, b0);
	b1->sense = !b1->sense;
	b1->head = b0->head;
}

void
gen_or(b0, b1)
	struct block *b0, *b1;
{
	b0->sense = !b0->sense;
	backpatch(b0, b1->head);
	b0->sense = !b0->sense;
	merge(b1, b0);
	b1->head = b0->head;
}

void
gen_not(b)
	struct block *b;
{
	b->sense = !b->sense;
}

void gen_finnal(struct block *p)
{
	backpatch(p, new_block(0x60));
	p->sense = !p->sense;
	backpatch(p, new_block(0));
	root = p->head;
}


static void
number_blks_r(p)
	struct block *p;
{
	int n;

	if (p == 0 || isMarked(p))
		return;

	Mark(p);
	n = n_blocks++;
	p->id = n;
	printf("p->id:%d,p->data:%d\n",p->id, p->private_data);

	number_blks_r(JT(p));
	number_blks_r(JF(p));
}
static int
count_blocks(struct block*p)
{
	int n;

	if (p == 0 || isMarked(p))
		return 0;
	Mark(p);
	return count_blocks(JT(p)) + count_blocks(JF(p)) + 1;
} 

static void
convert_code_r(struct block *p)
{
	struct bpf_insn *dst;

	if (p == 0 || isMarked(p))
		return ;
	Mark(p);

	convert_code_r(JF(p));
	convert_code_r(JT(p));
	
	dst = ftail -= 1;
	p->offset = dst - fstart;
	dst->data = p->private_data;
	if(JT(p))
	{
		dst->jt = JT(p)->offset - p->offset -1;
		dst->jf = JF(p)->offset - p->offset -1;
	}
		
}


struct bpf_insn *
icode_to_fcode(root, lenp)
	struct block *root;
	int *lenp;
{
	int n;
	struct bpf_insn *fp;

	/*
	 * Loop doing convert_code_r() until no branches remain
	 * with too-large offsets.
	 */
	/*while (1) {*/
	    unMarkAll();
	    n = *lenp = count_blocks(root);
	    printf("count_blocks:n:%d\n", n);
	    fp = (struct bpf_insn *)malloc(sizeof(*fp) * n);
	    if (fp == NULL){
		   printf("malloc error");
		   exit(1);
	    }
	    memset((char *)fp, 0, sizeof(*fp) * n);
	    fstart = fp;
	    ftail = fp + n;

	    unMarkAll();
	    convert_code_r(root);
		//break;
	    //free(fp);
	//}

	return fp;
}


void bpf_dump(struct bpf_insn *insn, int bf_len)
{
	
	int i;
	int n = bf_len;

	for (i = 0; i < n; ++insn, ++i)
		printf("{ %d, %d, %d},\n",
		       insn->data, insn->jt, insn->jf);
	return;
	
}

int main()
{
	struct bpf_insn *fp;
	int len;
	struct block *b1 = new_block(1);
	struct block *b2 = new_block(2);
	struct block *b3 = new_block(3);
	struct block *b4 = new_block(4);
	struct block *b5 = new_block(5);
	struct block *b6 = new_block(6);
	struct block *b7 = new_block(7);
	

	gen_and(b1, b2);
	gen_or(b3, b4);
	gen_and(b2, b4);
	gen_or(b5, b6);
	gen_and(b4, b6);
	gen_or(b6, b7);

	gen_finnal(b7);

	unMarkAll();
	n_blocks = 0;
	number_blks_r(root);
	fp = icode_to_fcode(root, &len);
	
	bpf_dump(fp, len);
	
	free(fp);

	freechunks();
	
	
}

