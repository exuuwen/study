
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <errno.h>

#include "gencode.h"

static int cur_mark;
#define isMarked(p) ((p)->mark == cur_mark)
#define unMarkAll() cur_mark += 1
#define Mark(p) ((p)->mark = cur_mark)

#define NOP -1

static struct bpf_insn *fstart;
static struct bpf_insn *ftail;

static int
slength(s)
	struct slist *s;
{
	int n = 0;

	for (; s; s = s->next)
		if (s->s.code != NOP)
			++n;
	return n;
}

static int
count_stmts(p)
	struct block *p;
{
	int n;

	if (p == 0 || isMarked(p))
		return 0;
	Mark(p);
	n = count_stmts(JT(p)) + count_stmts(JF(p));
	return slength(p->stmts) + n + 1 + p->longjt + p->longjf;
}


static int
convert_code_r(p)
	struct block *p;
{
	struct bpf_insn *dst;
	struct slist *src;
	int slen;
	u_int off;
	int extrajmps;		/* number of extra jumps inserted */
	struct slist **offset = NULL;

	if (p == 0 || isMarked(p))
		return (1);
	Mark(p);

	if (convert_code_r(JF(p)) == 0)
		return (0);
	if (convert_code_r(JT(p)) == 0)
		return (0);

	slen = slength(p->stmts);
	dst = ftail -= (slen + 1 + p->longjt + p->longjf);
		/* inflate length by any extra jumps */

	p->offset = dst - fstart;

	/* generate offset[] for convenience  */
	if (slen) {
		offset = (struct slist **)calloc(slen, sizeof(struct slist *));
		if (!offset) {
			/*bpf_error*/printf("not enough core");
			/*NOTREACHED*/
		}
	}
	src = p->stmts;
	for (off = 0; off < slen && src; off++) {
#if 0
		printf("off=%d src=%x\n", off, src);
#endif
		offset[off] = src;
		src = src->next;
	}

	off = 0;
	for (src = p->stmts; src; src = src->next) {
		if (src->s.code == NOP)
			continue;
		dst->code = (u_short)src->s.code;
		dst->k = src->s.k;

		/* fill block-local relative jump */
		if (BPF_CLASS(src->s.code) != BPF_JMP || src->s.code == (BPF_JMP|BPF_JA)) {
#if 0
			if (src->s.jt || src->s.jf) {
				bpf_error("illegal jmp destination");
				/*NOTREACHED*/
			}
#endif
			goto filled;
		}
		if (off == slen - 2)	/*???*/
			goto filled;

	    {
		int i;
		int jt, jf;
		const char *ljerr = "%s for block-local relative jump: off=%d";

#if 0
		printf("code=%x off=%d %x %x\n", src->s.code,
			off, src->s.jt, src->s.jf);
#endif

		if (!src->s.jt || !src->s.jf) {
			//bpf_error(ljerr, "no jmp destination", off);
			/*NOTREACHED*/
		}

		jt = jf = 0;
		for (i = 0; i < slen; i++) {
			if (offset[i] == src->s.jt) {
				if (jt) {
					//bpf_error(ljerr, "multiple matches", off);
					/*NOTREACHED*/
				}

				dst->jt = i - off - 1;
				jt++;
			}
			if (offset[i] == src->s.jf) {
				if (jf) {
					//bpf_error(ljerr, "multiple matches", off);
					/*NOTREACHED*/
				}
				dst->jf = i - off - 1;
				jf++;
			}
		}
		if (!jt || !jf) {
			//bpf_error(ljerr, "no destination found", off);
			/*NOTREACHED*/
		}
	    }
filled:
		++dst;
		++off;
	}
	if (offset)
		free(offset);

#ifdef BDEBUG
	bids[dst - fstart] = p->id + 1;
#endif
	dst->code = (u_short)p->s.code;
	dst->k = p->s.k;
	if (JT(p)) {
		extrajmps = 0;
		off = JT(p)->offset - (p->offset + slen) - 1;
		if (off >= 256) {
		    /* offset too large for branch, must add a jump */
		    if (p->longjt == 0) {
		    	/* mark this instruction and retry */
			p->longjt++;
			return(0);
		    }
		    /* branch if T to following jump */
		    dst->jt = extrajmps;
		    extrajmps++;
		    dst[extrajmps].code = BPF_JMP|BPF_JA;
		    dst[extrajmps].k = off - extrajmps;
		}
		else
		    dst->jt = off;
		off = JF(p)->offset - (p->offset + slen) - 1;
		if (off >= 256) {
		    /* offset too large for branch, must add a jump */
		    if (p->longjf == 0) {
		    	/* mark this instruction and retry */
			p->longjf++;
			return(0);
		    }
		    /* branch if F to following jump */
		    /* if two jumps are inserted, F goes to second one */
		    dst->jf = extrajmps;
		    extrajmps++;
		    dst[extrajmps].code = BPF_JMP|BPF_JA;
		    dst[extrajmps].k = off - extrajmps;
		}
		else
		    dst->jf = off;
	}
	return (1);
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
	while (1) {
	    unMarkAll();
	    n = *lenp = count_stmts(root);

	    fp = (struct bpf_insn *)malloc(sizeof(*fp) * n);
	    if (fp == NULL)
		    ;//bpf_error("malloc");
	    memset((char *)fp, 0, sizeof(*fp) * n);
	    fstart = fp;
	    ftail = fp + n;

	    unMarkAll();
	    if (convert_code_r(root))
		break;
	    free(fp);
	}

	return fp;
}

