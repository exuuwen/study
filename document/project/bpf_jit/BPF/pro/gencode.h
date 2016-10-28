#ifndef GENCODE_H
#define GENCODE_H

#include "bpf.h"

/* Address qualifiers. */

#ifndef HAVE___ATTRIBUTE__
#define __attribute__(x)
#endif /* HAVE___ATTRIBUTE__ */


#define Q_HOST		1
#define Q_NET		2
#define Q_PORT		3
#define Q_GATEWAY	4
#define Q_PROTO		5
#define Q_PROTOCHAIN	6
#define Q_PORTRANGE	7

/* Protocol qualifiers. */

#define Q_LINK		1
#define Q_IP		2

#define Q_SCTP		5
#define Q_TCP		6
#define Q_UDP		7

/*L3 protocol*/
#define	ETHERTYPE_IP	0x0800	/* IP protocol */
#define	IPPROTO_UDP	17	/*UDP protocol*/
#define	IPPROTO_TCP	6		/* TCP */
#define IPPROTO_SCTP	132
#define GTP_U_PORT	2152



#define UDP_LEN_OFF	4

#define PROTO_UNDEF	-1

/* Directional qualifiers. */

#define Q_SRC		1
#define Q_DST		2
#define Q_OR		3
#define Q_AND		4
#define Q_ADDR1		5
#define Q_ADDR2		6
#define Q_ADDR3		7
#define Q_ADDR4		8

#define Q_DEFAULT	0
#define Q_UNDEF		255



#define BPF_MEMWORDS 16

 


struct slist;

struct stmt {
	int code;
	struct slist *jt;	/*only for relative jump in block*/
	struct slist *jf;	/*only for relative jump in block*/
	bpf_int32 k;
};

struct slist {
	struct stmt s;
	struct slist *next;
};

/*
 * A bit vector to represent definition sets.  We assume TOT_REGISTERS
 * is smaller than 8*sizeof(atomset).
 */
typedef bpf_u_int32 atomset;
#define ATOMMASK(n) (1 << (n))
#define ATOMELEM(d, n) (d & ATOMMASK(n))

/*
 * An unbounded set.
 */
typedef bpf_u_int32 *uset;

/*
 * Total number of atomic entities, including accumulator (A) and index (X).
 * We treat all these guys similarly during flow analysis.
 */
#define N_ATOMS (BPF_MEMWORDS+2)

struct qual {
	unsigned char addr;
	unsigned char proto;
	unsigned char dir;
	unsigned char pad;
};

struct edge {
	int id;
	int code;
	uset edom;
	struct block *succ;
	struct block *pred;
	struct edge *next;	/* link list of incoming edges for a node */
};

struct block {
	int id;
	struct slist *stmts;	/* side effect stmts */
	struct stmt s;		/* branch stmt */
	int mark;
	int longjt;		/* jt branch requires long jump */
	int longjf;		/* jf branch requires long jump */
	int level;
	int offset;
	int sense;
	struct edge et;
	struct edge ef;
	struct block *head;
	struct block *link;	/* link field used by optimizer */
	uset dom;
	uset closure;
	struct edge *in_edges;
	atomset def, kill;
	atomset in_use;
	atomset out_use;
	int oval;
	int val[N_ATOMS];
};

struct arth {
	struct block *b;	/* protocol checks */
	struct slist *s;	/* stmt list */
	int regno;		/* virtual register number of result */
};




struct arth *gen_loadi(int);
struct arth *gen_load(int, struct arth *, int);
struct arth *gen_loadlen(void);
struct arth *gen_neg(struct arth *);
struct arth *gen_arth(int, struct arth *, struct arth *);

void gen_and(struct block *, struct block *);
void gen_or(struct block *, struct block *);
void gen_not(struct block *);

struct block *gen_scode(const char *, struct qual);
struct block *gen_ecode(const u_char *, struct qual);
struct block *gen_acode(const u_char *, struct qual);
struct block *gen_mcode(const char *, const char *, int, struct qual);

struct block *gen_ncode(const char *, bpf_u_int32, struct qual);
struct block *gen_proto_abbrev(int);
struct block *gen_relation(int, struct arth *, struct arth *, int);
struct block *gen_less(int);
struct block *gen_greater(int);
struct block *gen_byteop(int, int, int);
struct block *gen_broadcast(int);
struct block *gen_multicast(int);
struct block *gen_inbound(int);

struct block *gen_vlan(int);
struct block *gen_mpls(int);

struct block *gen_pppoed(void);
struct block *gen_pppoes(void);

struct block *gen_atmfield_code(int atmfield, bpf_int32 jvalue, bpf_u_int32 jtype, int reverse);
struct block *gen_atmtype_abbrev(int type);
struct block *gen_atmmulti_abbrev(int type);

struct block *gen_mtp2type_abbrev(int type);
struct block *gen_mtp3field_code(int mtp3field, bpf_u_int32 jvalue, bpf_u_int32 jtype, int reverse);

struct block *gen_pf_ifname(const char *);
struct block *gen_pf_rnr(int);
struct block *gen_pf_srnr(int);
struct block *gen_pf_ruleset(char *);
struct block *gen_pf_reason(int);
struct block *gen_pf_action(int);
struct block *gen_pf_dir(int);

struct block *gen_p80211_type(int, int);
struct block *gen_p80211_fcdir(int);

void bpf_optimize(struct block **);
void bpf_error(const char *, ...)
    __attribute__((noreturn, format (printf, 1, 2)));

void finish_parse(struct block *);
char *sdup(const char *);

struct bpf_insn *icode_to_fcode(struct block *, int *);
int pcap_parse(void);
void lex_init(const char *);
void lex_cleanup(void);
void sappend(struct slist *, struct slist *);

/* XXX */
#define JT(b)  ((b)->et.succ)
#define JF(b)  ((b)->ef.succ)

extern int no_optimize;

#endif
