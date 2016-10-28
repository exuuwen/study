#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#include "gencode.h"
#include "bpf.h"
#define NCHUNKS 16
#define CHUNK0SIZE 1024

#define OFF_LINKTYPE    12
#define OFF_UDPTYPE	9
#define MAC_LEN		14

#define RET 0x60
#define MAX_DP_INDEX  0x1111
#define JMP(c) ((c)|BPF_JMP|BPF_K)

enum e_offrel {
	OR_PACKET,	/* relative to the beginning of the packet */
	OR_LINK,	/* relative to the beginning of the link-layer header */
	OR_MACPL,	/* relative to the end of the MAC-layer header */
	OR_NET,		/* relative to the network-layer header */
	OR_NET_NOSNAP,	/* relative to the network-layer header, with no SNAP header at the link layer */
	OR_TRAN_IPV4,	/* relative to the transport-layer header, with IPv4 network layer */
	OR_TRAN_IPV6	/* relative to the transport-layer header, with IPv6 network layer */
};

struct chunk {
	u_int n_left;
	void *m;
};

static struct chunk chunks[NCHUNKS];
static int cur_chunk;

struct block *root;
static struct slist *ret_s = NULL;

static void *
newchunk(n)
	u_int n;
{
	struct chunk *cp;
	int k;
	size_t size;

	/* XXX Round up to nearest long. */
	n = (n + sizeof(long) - 1) & ~(sizeof(long) - 1);

	cp = &chunks[cur_chunk];
	if (n > cp->n_left) {
		++cp, k = ++cur_chunk;
		if (k >= NCHUNKS)
			;//bpf_error("out of memory");
		size = CHUNK0SIZE << k;
		cp->m = (void *)malloc(size);
		if (cp->m == NULL)
			;//bpf_error("out of memory");
		memset((char *)cp->m, 0, size);
		cp->n_left = size;
		if (n > size)
			;//bpf_error("out of memory");
	}
	cp->n_left -= n;
	return (void *)((char *)cp->m + cp->n_left);
}

static void
freechunks()
{
	int i;

	cur_chunk = 0;
	for (i = 0; i < NCHUNKS; ++i)
		if (chunks[i].m != NULL) {
			free(chunks[i].m);
			chunks[i].m = NULL;
		}
}



static inline struct block *
new_block(code)
	int code;
{
	struct block *p;

	p = (struct block *)newchunk(sizeof(*p));
	p->s.code = code;
	p->head = p;

	return p;
}


static inline struct slist *
new_stmt(code)
	int code;
{
	struct slist *p;

	p = (struct slist *)newchunk(sizeof(*p));
	p->s.code = code;

	return p;
}

static struct block *
gen_retblk(v)
	int v;
{
	struct block *b = new_block(BPF_RET|BPF_K);

	b->s.k = v;
	return b;
}

static struct block *
gen_gtp_retblk()
{
	struct block *b = new_block(BPF_RET|BPF_A);
	b->stmts = ret_s;
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

void
sappend(s0, s1)
	struct slist *s0, *s1;
{
	/*
	 * This is definitely not the best way to do this, but the
	 * lists will rarely get long.
	 */
	while (s0->next)
		s0 = s0->next;
	s0->next = s1;
}

static void
merge(b0, b1)
	struct block *b0, *b1;
{
	struct block **p = &b0;

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



static struct slist *
gen_load_macplrel(offset, size)
	u_int offset, size;
{
	struct slist *s;
	
	s = new_stmt(BPF_LD|BPF_ABS|size);
	s->s.k = MAC_LEN + offset;
	
	return s;
}

static struct slist *
gen_load_llrel(offset, size)
	u_int offset, size;
{
	struct slist *s;
	
	s = new_stmt(BPF_LD|BPF_ABS|size);
	s->s.k = offset;
	
	return s;
}


static struct slist *
gen_loadx_iphdrlen()
{
	struct slist *s;

	s = new_stmt(BPF_LDX|BPF_MSH|BPF_B);
	s->s.k = MAC_LEN; 
	
	return s;
}



static struct block *
gen_teid()
{
	struct slist *s, *s2;
	struct  block *b;
	//s = gen_loadx_iphdrlen();
		
	s = new_stmt(BPF_LD|BPF_ABS|BPF_W);
	s->s.k = MAC_LEN + 12 + 20;			/*8 udp + 4 GTP_U*/


	b = new_block(JMP(BPF_JEQ));
	b->stmts = s;
	b->s.k = 0;
	
	return b;
}

static struct block *
gen_index()
{
	struct slist *s, *s2;
	struct  block *b;
	//s = gen_loadx_iphdrlen();
		
	s = new_stmt(BPF_LD|BPF_ABS|BPF_W);
	s->s.k = MAC_LEN + 12 + 20;			/*8 udp + 4 GTP_U*/

	s2 = new_stmt(BPF_ALU|BPF_RSH|BPF_K);
	s2->s.k = 21;			
	sappend(s, s2);

	s2 = new_stmt(BPF_ALU|BPF_AND|BPF_K);
	s2->s.k = 0x1ff;			
	sappend(s, s2);

	b = new_block(JMP(BPF_JGE));
	b->stmts = s;
	b->s.k = MAX_DP_INDEX;
	
	gen_not(b);

	
	return b;
}


static struct slist *
gen_load_a(offrel, offset, size)
	enum e_offrel offrel;
	u_int offset, size;
{
	struct slist *s, *s2;

	switch (offrel) {

	case OR_LINK:
	    s = gen_load_llrel(offset, size);
	    break;

	case OR_NET:
	    s = gen_load_macplrel(offset, size);
	    break;
	case OR_TRAN_IPV4:
	    /*s = gen_loadx_iphdrlen();*/  /*just for ip head length = 20*/
		
	    s = new_stmt(BPF_LD|BPF_ABS|size);
	    s->s.k = MAC_LEN + offset + 20;
	   // sappend(s, s2);

	    break;

	default:
	    abort();
	    return NULL;
	}
	return s;
}

static struct block *
gen_ncmp(offrel, offset, size, mask, jtype, reverse, v)
	enum e_offrel offrel;
	bpf_int32 v;
	bpf_u_int32 offset, size, mask, jtype;
	int reverse;
{
	struct slist *s, *s2;
	struct block *b;

	s = gen_load_a(offrel, offset, size);

	if (mask != 0xffffffff) {
		s2 = new_stmt(BPF_ALU|BPF_AND|BPF_K);
		s2->s.k = mask;
		sappend(s, s2);
	}

	b = new_block(JMP(jtype));
	b->stmts = s;
	b->s.k = v;
	if (reverse && (jtype == BPF_JGT || jtype == BPF_JGE))
		gen_not(b);
	return b;
}


static struct block *
gen_cmp(offrel, offset, size, v)
	enum e_offrel offrel;
	u_int offset, size;
	bpf_int32 v;
{
	return gen_ncmp(offrel, offset, size, 0xffffffff, BPF_JEQ, 0, v);
}

static struct block *
gen_cmp_gt(offrel, offset, size, v)
	enum e_offrel offrel;
	u_int offset, size;
	bpf_int32 v;
{
	return gen_ncmp(offrel, offset, size, 0xffffffff, BPF_JGT, 0, v);
}

static struct  block*
gen_ip_len()
{
	struct slist *s, *s1;
	struct block *b;
	
	s = gen_loadx_iphdrlen();

	s1 = new_stmt(BPF_STX);
	s1->s.k = 0; 
	sappend(s, s1);
	
	s1 = new_stmt(BPF_LD|BPF_MEM);
	s1->s.k = 0; 
	sappend(s, s1);
	
	b = new_block(JMP(BPF_JEQ));
	b->stmts = s;
	b->s.k = 20;

	return b;
}

static struct block *
gen_ethertype_ip(int proto)
{
	struct block *b0, *b;

	b0 = gen_cmp(OR_LINK, OFF_LINKTYPE, BPF_H, (bpf_int32)proto);	
	b = gen_ip_len();
	
	gen_and(b0, b);
	return b;
	
}


static struct block *
gen_proto(v, proto)
	int v;
	int proto;
{
	struct block *b0, *b1;
	switch (proto) {

	case Q_IP:
	    b0 = gen_ethertype_ip(ETHERTYPE_IP);
	    b1 = gen_cmp(OR_NET, OFF_UDPTYPE, BPF_B, (bpf_int32)v);
	    gen_and(b0, b1);
	    break;
	
	default:
	    abort();
	    return NULL;
	}
	
	return b1;
}

struct block *
gen_proto_abbrev(int proto)
{
	struct block *b0;
	struct block *b1;

	switch (proto) {
	
	case Q_UDP:
	    b1 = gen_proto(IPPROTO_UDP, Q_IP);
	    break;

	case Q_IP:
	    b1 = gen_ethertype_ip(ETHERTYPE_IP);
	    break;
	
	default:
	    abort();
	    return NULL;
	}

	return b1;
}


static struct block *
gen_portatom(off, v)
	int off;
	bpf_int32 v;
{
	return gen_cmp(OR_TRAN_IPV4, off, BPF_H, v);
}

static struct block *
gen_gtp_len(off, v)
	int off;
	bpf_int32 v;
{
	return gen_cmp_gt(OR_TRAN_IPV4, off, BPF_H, v);
}

/*static struct block *
gen_ipfrag()
{
	struct slist *s;
	struct block *b;

	// not ip frag 
	s = gen_load_a(OR_NET, 6, BPF_H);
	b = new_block(JMP(BPF_JSET));
	b->s.k = 0x1fff;
	b->stmts = s;
	gen_not(b);

	return b;
}*/

struct block *
gen_portop(port, proto, dir)
	int port, proto, dir;
{
	struct block *b0, *b1, *tmp;

	/* ip proto 'proto' */
	b0/*tmp*/ = gen_cmp(OR_NET, 9, BPF_B, (bpf_int32)proto);
	/*b0 = gen_ipfrag();
	gen_and(tmp, b0);*/

	switch (dir) {
	case Q_SRC:
		b1 = gen_portatom(0, (bpf_int32)port);
		break;

	case Q_DST:
		b1 = gen_portatom(2, (bpf_int32)port);
		break;

	case Q_OR:
	case Q_DEFAULT:
		tmp = gen_portatom(0, (bpf_int32)port);
		b1 = gen_portatom(2, (bpf_int32)port);
		gen_or(tmp, b1);
		break;

	case Q_AND:
		tmp = gen_portatom(0, (bpf_int32)port);
		b1 = gen_portatom(2, (bpf_int32)port);
		gen_and(tmp, b1);
		break;

	default:
		abort();
		return NULL;
	}
	gen_and(b0, b1);
	
	return b1;
}

static struct block *
gen_port(port, ip_proto, dir)
	int port;
	int ip_proto;
	int dir;
{
	struct block *b0, *b1, *tmp;

	b0 =  gen_ethertype_ip(ETHERTYPE_IP);

	switch (ip_proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		b1 = gen_portop(port, ip_proto, dir);
		break;

	case PROTO_UNDEF:
		tmp = gen_portop(port, IPPROTO_TCP, dir);
		b1 = gen_portop(port, IPPROTO_UDP, dir);
		gen_or(tmp, b1);
		break;

	default:
		abort();
		return NULL;
	}
	gen_and(b0, b1);
	
	if((ip_proto == IPPROTO_UDP)&&(port == GTP_U_PORT))
	{
		tmp = gen_gtp_len(UDP_LEN_OFF, 16);//get udp_len > udp_head_len + gtp_u_len(8 + 8 = 16)
		gen_and(b1, tmp);
	
		b0 = gen_teid();
		gen_not(b0);	
		b1 = gen_index();
		gen_and(b0, b1);

		b0 = gen_teid();
		gen_or(b0, b1);
		
		
		gen_and(tmp, b1);
	}
	//return tmp;
	return b1;
}

struct block *
gen_ncode(s, v, q)
	register const char *s;
	bpf_u_int32 v;
	struct qual q;
{
	bpf_u_int32 mask;
	int proto = q.proto;
	int dir = q.dir;
	int vlen;

	if (s == NULL)
		vlen = 32;

	switch (q.addr) {

	case Q_PORT:
	    if (proto == Q_UDP)
		proto = IPPROTO_UDP;
	    else if (proto == Q_TCP)
		proto = IPPROTO_TCP;
	    else if (proto == Q_DEFAULT)
		proto = PROTO_UNDEF;
	    else 
		return NULL;	
	
	    return gen_port((int)v, proto, dir);
	
	default:
            abort();
	    return NULL;
	}
}

void
finish_parse(p)
	struct block *p;
{
	//if(ret_s == NULL)
		backpatch(p, gen_retblk(RET));
	/*else
		backpatch(p, gen_gtp_retblk());*/
	p->sense = !p->sense;
	backpatch(p, gen_retblk(0));
	root = p->head;
}



bpf_dump(const struct bpf_program *p, int option)
{
	const struct bpf_insn *insn;
	int i;
	int n = p->bf_len;

	insn = p->bf_insns;
	if (option > 2) {
		for (i = 0; i < n; ++insn, ++i) 
			puts(bpf_image(insn, i));
		
	}
	/*printf("\n\n");
	if (option > 1) {
		for (i = 0; i < n; ++insn, ++i)
			printf("{ 0x%x, %d, %d, 0x%08x },\n",
			       insn->code, insn->jt, insn->jf, insn->k);
		return;
	}*/
}

int	
bfp_compile(const char *buf, struct bpf_program *program)
{	
	int len;	
	const char * volatile xbuf = buf;
	
	lex_init(xbuf);
	(void)pcap_parse();
	
	if (root == NULL)
		root = gen_retblk(0x60);

	program->bf_insns = icode_to_fcode(root, &len);
	program->bf_len = len;

	lex_cleanup();
	freechunks();
	return 0;
}
