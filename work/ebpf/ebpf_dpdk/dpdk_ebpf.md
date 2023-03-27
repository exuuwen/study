[root@CentOS7 bpf]# readelf -S t3.o
There are 6 section headers, starting at offset 0x1c8:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .strtab           STRTAB           0000000000000000  00000170
       0000000000000053  0000000000000000           0     0     1
  [ 2] .text             PROGBITS         0000000000000000  00000040
       0000000000000080  0000000000000000  AX       0     0     8
  [ 3] .rel.text         REL              0000000000000000  00000150
       0000000000000020  0000000000000010           5     2     8
  [ 4] .llvm_addrsig     LOOS+0xfff4c03   0000000000000000  00000170
       0000000000000000  0000000000000000   E       5     0     1
  [ 5] .symtab           SYMTAB           0000000000000000  000000c0
       0000000000000090  0000000000000018           1     3     8
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  p (processor specific)


1. get section .text and it's id
2. get section .rel.text, verify the sc.info(section .text id) .text and get the .symtab through sc.link  

typedef struct
{
  Elf64_Addr    r_offset;               /* Address */
  Elf64_Xword   r_info;                 /* Relocation type and symbol index */
} Elf64_Rel;

# readelf -r t3.o

Relocation section '.rel.text' at offset 0x150 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name
000000000048  000500000001 unrecognized: 1       0000000000000000 stdout
000000000068  00040000000a unrecognized: a       0000000000000000 rte_pktmbuf_dump

3. get rel.offset(0x68-->104 for rte_pktmbuf_dump) and symtab id (rel.info >> 32, 5 for rte_pktmbuf_dump)
 readelf -s t3.o

Symbol table '.symtab' contains 6 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS t3.c
     2: 0000000000000070     0 NOTYPE  LOCAL  DEFAULT    2 LBB0_2
     3: 0000000000000000   128 FUNC    GLOBAL DEFAULT    2 entry
     4: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND rte_pktmbuf_dump
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND stdout

4. get section symtab and get the symbol name(rte_pktmbuf_dump) base on symtab id

# llvm-objdump -S t3.o

t3.o:	file format elf64-bpf

Disassembly of section .text:

0000000000000000 <entry>:
       0:	bf 12 00 00 00 00 00 00	r2 = r1
       1:	69 21 10 00 00 00 00 00	r1 = *(u16 *)(r2 + 16)
       2:	79 23 00 00 00 00 00 00	r3 = *(u64 *)(r2 + 0)
       3:	0f 13 00 00 00 00 00 00	r3 += r1
       4:	71 31 0c 00 00 00 00 00	r1 = *(u8 *)(r3 + 12)
       5:	71 33 0d 00 00 00 00 00	r3 = *(u8 *)(r3 + 13)
       6:	67 03 00 00 08 00 00 00	r3 <<= 8
       7:	4f 13 00 00 00 00 00 00	r3 |= r1
       8:	55 03 05 00 08 06 00 00	if r3 != 1544 goto +5 <LBB0_2>
       9:	18 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00	r1 = 0 ll
      11:	79 11 00 00 00 00 00 00	r1 = *(u64 *)(r1 + 0)
      12:	b7 03 00 00 40 00 00 00	r3 = 64
      13:	85 10 00 00 ff ff ff ff	call -1

0000000000000070 <LBB0_2>:
      14:	b7 00 00 00 01 00 00 00	r0 = 1
      15:	95 00 00 00 00 00 00 00	exit

5. repalce the call insn based on the rel.offset 104 = 13*8 and synbol name in the pre-define prm->xsym
