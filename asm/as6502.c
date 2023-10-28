/* as6502 --- John's 6502 assembler                         1983-06-16 */
/* Copyright (c) 1983 John Honniball, Bambleweeny Computer Systems     */

/* Modification:
 * 1983-06-16 JRH Initial coding (in Ratfor)
 * 1999-08-18 JRH Added hex format options
 * 1999-08-20 JRH Fixed error-checking in 'convert'
 * 1999-08-21 JRH Fixed error-checking in directives
 * 1999-08-22 JRH Added check for current address beyond $FFFF
 * 1999-08-22 JRH Fixed accumulator-mode syntax: ASL A
 * 2000-01-28 JRH Added cycle counts
 * 2000-02-01 JRH Changed behaviour of listing for EQU directives
 * 2000-02-02 JRH Improved handling of erroneous input
 * 2000-02-15 JRH Made lower-case A, X and Y registers legal
 * 2000-02-17 JRH Made mnemonics and directives case-insensitive
 * 2000-06-21 JRH Made assembler return non-zero exit status on error
 * 2000-06-28 JRH Generate code when address bad, to maintain code size
 * 2023-10-25 JRH Add EOF record to checksum hex output file
 * 2023-10-25 JRH Protect against filling up symbol table and detect duplicate labels
 */
 
/* #define DB */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "as6502.h"

#define AREG(c) (((c) == 'A') || ((c) == 'a'))
#define XREG(c) (((c) == 'X') || ((c) == 'x'))
#define YREG(c) (((c) == 'Y') || ((c) == 'y'))

int     Errs,              /* Error counter */
        Pass,              /* Pass 1 or 2 */
        Nline,             /* Line number */
        Nlabels,           /* Number of labels */
        Hexfmt,            /* Type of hex file */
        Nbytes;            /* Number of bytes for current instruction */
address Addr;              /* Current assembly address */

FILE    *Source,            /* Source code fp */
        *Object,            /* Object code fp */
        *Listing,           /* Listing fp */ 
        *Errorfd;           /* Error list fp */

struct Sym {
   char    Label[MAXLABEL];   /* Label name      */
   address Address;           /* Label address   */
   int     References;        /* Reference count */
};

struct Sym Symbol[MAXSYMBOLS];   /* The symbol table */

char      Line[MAXLINE];         /* Text of current line */

int     Byte[MAXBYTES],          /* Bytes of current instruction */
        Block[MAXBLOCK],         /* Checksum block for 'save' */
        Blkptr,                  /* Pointer to next space in Block */
        Nblocks;                 /* Number of blocks of checksum data */
address Blkaddr;                 /* Start address of block */

/* inh, imm, abs, abs,X, abs,Y, zpage, zpage,X, zpage,Y, ind,X, ind,Y, rel, ind */

struct {
   char mnem[4];
   int flags;
   int obj[MAXMODES];
   int cyc[MAXMODES];
} Opcodes[] = {
   {"ADC", 1,
   { ERR, 0x69, 0x6D, 0x7D, 0x79, 0x65, 0x75,  ERR, 0x61, 0x71,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"AND", 1,
   { ERR, 0x29, 0x2D, 0x3D, 0x39, 0x25, 0x35,  ERR, 0x21, 0x31,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"ASL", 0,
   {0x0A,  ERR, 0x0E, 0x1E,  ERR, 0x06, 0x16,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"BCC", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x90,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BCS", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0xB0,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BEQ", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0xF0,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BIT", 0,
   { ERR,  ERR, 0x2C,  ERR,  ERR, 0x24,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    4,    0,    0,    3,    0,    0,    0,    0,    0,    0}},
   {"BMI", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x30,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BNE", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0xD0,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BPL", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x10,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BRK", 0,
   {0x00,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   7,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"BVC", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x50,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"BVS", 0,
   { ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x70,  ERR},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,    0}},
   {"CLC", 0,
   {0x18,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"CLD", 0,
   {0xD8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"CLI", 0,
   {0x58,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"CLV", 0,
   {0xB8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"CMP", 0,
   { ERR, 0xC9, 0xCD, 0xDD, 0xD9, 0xC5, 0xD5,  ERR, 0xC1, 0xD1,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"CPX", 0,
   { ERR, 0xE0, 0xEC,  ERR,  ERR, 0xE4,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    2,    4,    0,    0,    3,    0,    0,    0,    0,    0,    0}},
   {"CPY", 0,
   { ERR, 0xC0, 0xCC,  ERR,  ERR, 0xC4,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    2,    4,    0,    0,    3,    0,    0,    0,    0,    0,    0}},
   {"DEC", 0,
   { ERR,  ERR, 0xCE, 0xDE,  ERR, 0xC6, 0xD6,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"DEX", 0,
   {0xCA,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"DEY", 0,
   {0x88,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"EOR", 1,
   { ERR, 0x49, 0x4D, 0x5D, 0x59, 0x45, 0x55,  ERR, 0x41, 0x51,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"INC", 0,
   { ERR,  ERR, 0xEE, 0xFE,  ERR, 0xE6, 0xF6,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"INX", 0,
   {0xE8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"INY", 0,
   {0xC8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"JMP", 0,
   { ERR,  ERR, 0x4C,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR, 0x6C},
   {   0,    0,    3,    0,    0,    0,    0,    0,    0,    0,    0,    5}},
   {"JSR", 0,
   { ERR,  ERR, 0x20,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    6,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"LDA", 1,
   { ERR, 0xA9, 0xAD, 0xBD, 0xB9, 0xA5, 0xB5,  ERR, 0xA1, 0xB1,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"LDX", 1,
   { ERR, 0xA2, 0xAE,  ERR, 0xBE, 0xA6,  ERR, 0xB6,  ERR,  ERR,  ERR,  ERR},
   {   0,    2,    4,    0,    4,    3,    0,    4,    0,    0,    0,    0}},
   {"LDY", 1,
   { ERR, 0xA0, 0xAC, 0xBC,  ERR, 0xA4, 0xB4,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    2,    4,    4,    0,    3,    4,    0,    0,    0,    0,    0}},
   {"LSR", 0,
   {0x4A,  ERR, 0x4E, 0x5E,  ERR, 0x46, 0x56,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"NOP", 0,
   {0xEA,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"ORA", 0,
   { ERR, 0x09, 0x0D, 0x1D, 0x19, 0x05, 0x15,  ERR, 0x01, 0x11,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"PHA", 0,
   {0x48,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   3,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"PHP", 0,
   {0x08,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   3,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"PLA", 0,
   {0x68,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   4,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"PLP", 0,
   {0x28,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   4,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"ROL", 0,
   {0x2A,  ERR, 0x2E, 0x3E,  ERR, 0x26, 0x36,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"ROR", 0,
   {0x6A,  ERR, 0x6E, 0x7E,  ERR, 0x66, 0x76,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    6,    7,    0,    5,    6,    0,    0,    0,    0,    0}},
   {"RTI", 0,
   {0x40,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   6,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"RTS", 0,
   {0x60,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   6,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"SBC", 1,
   { ERR, 0xE9, 0xED, 0xFD, 0xF9, 0xE5, 0xF5,  ERR, 0xE1, 0xF1,  ERR,  ERR},
   {   0,    2,    4,    4,    4,    3,    4,    0,    6,    5,    0,    0}},
   {"SEC", 0,
   {0x38,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"SED", 0,
   {0xF8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"SEI", 0,
   {0x78,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"STA", 0,
   { ERR,  ERR, 0x8D, 0x9D, 0x99, 0x85, 0x95,  ERR, 0x81, 0x91,  ERR,  ERR},
   {   0,    0,    4,    5,    5,    3,    4,    0,    6,    6,    0,    0}},
   {"STX", 0,
   { ERR,  ERR, 0x8E,  ERR,  ERR, 0x86,  ERR, 0x96,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    4,    0,    0,    3,    0,    4,    0,    0,    0,    0}},
   {"STY", 0,
   { ERR,  ERR, 0x8C,  ERR,  ERR, 0x84, 0x94,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   0,    0,    4,    0,    0,    3,    4,    0,    0,    0,    0,    0}},
   {"TAX", 0,
   {0xAA,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"TAY", 0,
   {0xA8,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"TSX", 0,
   {0xBA,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"TXA", 0,
   {0x8A,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"TXS", 0,
   {0x9A,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}},
   {"TYA", 0,
   {0x98,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR},
   {   2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}
};

#ifdef __STDC__
int main (int argc, const char * *argv);
void chop_up (const char *lin, char *label, char *mnem, char *operand, char *comment);
int assemble (const char *mnem, const char *oper, char *cycles);
void instruction (int mn, const char *oper, char *cycles);
int operand (const char *oper, int *modep, address *opp);
int valid_symbol (const char *label);
int add_symbol (const char *label, address addr);
void symbols (void);
int look_up (const char *mnem);
int opcode_for (int mn, int *modep, address *opp, char *cycles);
void directive (int dir, const char *oper);
int eval (const char *str, address *nump);
int evaluate (const char *str, int *ip, address *nump);
int convert (const char *str, int *ip, address *nump);
int sym (const char *str, int *ip, address *nump);
void nerd (const char *str);
void for_ref (const char *str);
void unused (const char *str);
void set_up (int argc, const char * *argv);
void list_it (const char *cycles, const char *label, const char *mnem, int mn, const char *oper, const char *comment);
void putbyte (int byte);
void putblock (void);
void puteof (void);
void cant (const char *path, int bomb);
address gctol (const char *str, int *ip, int base);
#else
#define const
int main ();
void chop_up ();
int assemble ();
void instruction ();
int operand ();
int valid_symbol ();
int add_symbol ();
void symbols ();
int look_up ();
int opcode_for ();
void directive ();
int eval ();
int evaluate ();
int convert ();
int sym ();
void nerd ();
void for_ref ();
void unused ();
void set_up ();
void list_it ();
void putbyte ();
void putblock ();
void puteof ();
void cant ();
address gctol ();
#endif   /* __STDC__ */

void strupr (char *s);

int main (argc, argv)
const int argc;
const char *argv[];
{
   char label[MAXLABEL], mnem[MAXMNEM];
   char oper[MAXOPER], comment[MAXCOMMENT];
   char cycles[MAXCYCSTR];
   int i;
   int mn;
   static char vers[] = "2.1";

   set_up (argc, argv);    /* Set up files & globals */
   Pass = 1;               /* First pass */

   while (fgets (Line, MAXLINE, Source) != NULL) { /* Pass 1 */
      Nline++;
      Nbytes = 0;
      cycles[0] = EOS;
#ifdef DB
      fprintf (stderr, "%4d: %s", Nline, Line);
#endif   /* DB */

      chop_up (Line, label, mnem, oper, comment);

      if (label[0] != EOS) {     /* Fill in the Symbol Table */
         if (valid_symbol (label) == OK)
            if (add_symbol (label, Addr) == ERR)
               nerd ("Duplicate label");
      }     

      if (mnem[0] != EOS) {      /* Ignore comment lines */
         if (assemble (mnem, oper, cycles) == ERR)
            nerd ("Unfroodish mnemonic");
      }

      Addr += ADDR(Nbytes);
   }

   rewind (Source);     /* rewind source file for second pass */
   Nline = 0;           /* reset line number counter */
   Nblocks = 0;         /* Reset hex block counter */
   Pass = 2;            /* Second pass */
   Addr  = ADDR(0);     /* reset current address pointer */

   while (fgets (Line, MAXLINE, Source) != NULL) { /* Pass 2 */
      Nline++;
      Nbytes = 0;
      cycles[0] = EOS;
      mn = ERR;

      chop_up (Line, label, mnem, oper, comment);

      if (mnem[0] != EOS)     /* Ignore comments */
         mn = assemble (mnem, oper, cycles);

      if (mn != ERR) {
         for (i = 0; i < Nbytes; i++)
            putbyte (Byte[i]);
      }

      list_it (cycles, label, mnem, mn, oper, comment);
      Addr += ADDR(Nbytes);
   }

   if (Blkptr != 0)
      putblock ();   /* Put out the last block of hex. */
   
   puteof ();  /* Write EOF marker */
   
   symbols ();

   fprintf (TTY, "%04d ERRORS [6502 ASSEMBLER Rev.%s]\n", Errs, vers);

   if (Errs == 0)
      return (0);
   else
      return (1);
}


/* chop_up --- chop up the source line */

void chop_up (lin, label, mnem, operand, comment)
const char lin[];
char label[], mnem[], operand[], comment[];
{
   int i, j;

   label[0]   = EOS;
   mnem[0]    = EOS;
   operand[0] = EOS;
   comment[0] = EOS;

   if (lin[0] == NEWLINE)     /* Blank line */
      return;
   else if (lin[0] == COMMENT_SYM) {   /* Entire line is a comment */
      for (i = 0; lin[i] != NEWLINE; i++)
         comment[i] = lin[i];

      comment[i] = EOS;
      return;
   }

   for (i = j = 0; (lin[i] != ' ') && (lin[i] != LABEL_SYM) && (lin[i] != NEWLINE); i++)
      if (i < (MAXLABEL - 1))    /* Truncate excessively long labels */
         label[j++] = lin[i];

   label[j] = EOS;
   
   if (i != j)
      fprintf (Errorfd, "Warning: %s: label truncated at %d characters\n", label, MAXLABEL - 1);
   
   if (lin[i] == LABEL_SYM)   /* Skip the colon if there was one */
      i++;
   
   SKIPBL(lin, i);   /* Skip blanks between label and mnemonic */

   if (lin[i] != COMMENT_SYM) {
      for (j = 0; lin[i] != ' ' && lin[i] != NEWLINE; i++)
         if (j < (MAXMNEM - 1))
            mnem[j++] = lin[i];    /* Grab mnemonic */

      mnem[j] = EOS;
   }
   else
      mnem[0] = EOS;

   while (lin[i] == ' ' && lin[i] != NEWLINE)  /* Skip blanks between mnemonic and operand */
      i++;

   if (lin[i] != COMMENT_SYM && lin[i] != NEWLINE) {
      if (lin[i] == '"' || lin[i] == '\'') {   /* Allow for quoted operands */
         const char quote = lin[i];    /* terminating character */
         operand[0] = quote;
         i++;           /* Skip the opening quote */
         for (j = 1; lin[i] != quote && lin[i] != NEWLINE ; i++)
            if (j < (MAXOPER - 2))
               operand[j++] = lin[i]; /* Grab operand */

         operand[j++] = quote;
         operand[j] = EOS;
         i++;
         SKIPBL(lin, i);
      }
      else {   /* Normal operand - not quoted */
         for (j = 0; lin[i] != ' ' && lin[i] != NEWLINE; i++)
            if (j < (MAXOPER - 1))
               operand[j++] = lin[i]; /* Grab operand */

         operand[j] = EOS;

         while (lin[i] == ' ' && lin[i] != NEWLINE)
            i++;
      }
   }

   for (j = 0; lin[i] != NEWLINE; i++, j++)
      comment[j] = lin[i];

   comment[j] = EOS;
}


/* assemble --- do a line of assembly */

int assemble (mnem, oper, cycles)
const char mnem[], oper[];
char cycles[];
{
   int mn;
      
   if (Addr > 0xffffL)
      nerd ("Current address beyond $FFFF");

   mn = look_up (mnem);

   if (mn == ERR)       /* Ignore illegal mnemonics */
      return (mn);

   if (IS_DIRECTIVE(mn)) { /* Sort out directives */
      directive (mn, oper);
      cycles[0] = EOS;
   }
   else
      instruction (mn, oper, cycles);
      
   return (mn);
}


/* instruction --- handle instructions */

void instruction (mn, oper, cycles)
const int mn;
const char oper[];
char cycles[];
{
   int mode;
   address op;

   if (operand (oper, &mode, &op) != ERR) {
      Byte[0] = opcode_for (mn, &mode, &op, cycles);  /* Work out opcode */
      if (Byte[0] == ERR)
         nerd ("Illegal instruction/address mode");

      switch (mode) {
      case INHERENT:
         Nbytes = 1;
         break;         /* No address bytes */
      case RELATIVE:
         Byte[1] = NUM(op & 0xff);
         Nbytes = 2;
         break;
      case IMMEDIATE:
      case Z_PAGE:
      case Z_INDEX_X:
      case Z_INDEX_Y:
      case INDIRECT_X:
      case INDIRECT_Y:
         if (op > 0xff) {
            nerd ("operand too big");
            Byte[1] = 0xff;
         }
         else
            Byte[1] = NUM(op & 0xff);

         Nbytes = 2;
         break;
      case ABSOLUTE:
      case INDEX_X:
      case INDEX_Y:
      case INDIRECT:
         Byte[1] = NUM(op & 0xff);
         Byte[2] = NUM(op / 256);
         Nbytes = 3;
         break;
      default:
         fprintf (stderr, "%d: bad mode\n", mode);
         /* Falls through */
      case ERR:
         Nbytes = 0;
         cycles[0] = EOS;
         break;
      }
   }
}


/* operand --- sort out the operand and address mode */

int operand (oper, modep, opp)
const char oper[];
int *modep;
address *opp;
{
   int mode;
   int i;
   int stat;

   mode = ERR;          /* Guilty until proven innocent... */
   stat = ERR;

   if ((oper[0] == EOS) || (AREG(oper[0]) && (oper[1] == EOS))) {
      mode = INHERENT;
      stat = OK;
      *modep = mode;
      return (OK);
   }
   else if (oper[0] == IMM_SYM) {
      mode = IMMEDIATE;
      i = 1;            /* Skip the '#'... */
      stat = evaluate (oper, &i, opp);
   }
   else if (oper[0] == INDIRECT_SYM) {
      i = 1;
      stat = evaluate (oper, &i, opp);
      if (oper[i] == INDEX_SYM && XREG(oper[i+1]) &&
          oper[i+2] == INDIRECT_END && oper[i+3] == EOS) {
         i += 3;
         mode = INDIRECT_X;      /* LDA (PTR,X) */
      }
      else if (oper[i] == INDIRECT_END && oper[i+1] == INDEX_SYM &&
               YREG(oper[i+2]) && oper[i+3] == EOS) {
         i += 3;
         mode = INDIRECT_Y;      /* LDA (PTR),Y */
      }                   
      else if (oper[i] == INDIRECT_END && oper[i+1] == EOS) {
         i++;   
         mode = INDIRECT;        /* JMP (PTR) */
      }
      else
         mode = ERR;
   }
   else {
      i = 0;
      stat = evaluate (oper, &i, opp);
      if (oper[i] == INDEX_SYM) {
         if (XREG(oper[i+1]) && oper[i+2] == EOS) {
            i += 2;
            mode = INDEX_X;   /* LDA TAB,X */
         }
         else if (YREG(oper[i+1]) && oper[i+2] == EOS) {
            i += 2;
            mode = INDEX_Y;   /* LDA TAB,Y */
         }
         else
            mode = ERR;
      }
      else
         mode = ABSOLUTE;     /* LDA ABS */
   }
   
#ifdef DB
   fprintf (stderr, "stat = %d, mode = %d, i = %d\n", stat, mode, i);
#endif

   *modep = mode;

   if ((stat == ERR) && PASS2) {
      nerd ("Undefined label");
      *opp = (address)0xffff;    /* Dummy address */
      return (OK); /* Allow code generation anyway */
   }

   if (PASS1 && (mode == ERR || oper[i] != EOS)) {
      nerd ("Bad syntax in operand");  /* Look for extra characters at end of line */
      return (ERR);
   }

   return (OK);
}


/*  valid_symbol --- test a label for invalid characters */

int valid_symbol (label)
const char label[];
{
   int i;
   
   if (isdigit (label[0])) {
      nerd ("Label names must not begin with a digit");
      return (ERR);
   }
   
   for (i = 0; label[i] != EOS; i++) {
      if (!(isalpha (label[i]) || isdigit (label[i]) || (label[i] == '_'))) {
         nerd ("Invalid character(s) in label name");
         return (ERR);
      }
   }
   
   if (AREG(label[0]) && (label[1] == EOS)) {
      nerd ("Labels may not be named 'A' or 'a'");
      return (ERR);
   }
   
   return (OK);
}


/* add_symbol --- add a symbol to the symbol table */

int add_symbol (label, addr)
const char label[];
const address addr;
{
   int i;
   
#ifdef DB
   fprintf (stderr, "add_symbol: label = '%s'\n", label);
#endif
   
   if (Nlabels >= (MAXSYMBOLS - 1)) {
      nerd ("Too many labels");
      exit (1);
   }
   
   for (i = 0; i < Nlabels; i++) {
      if (strcmp (label, Symbol[i].Label) == 0)
         return (ERR);
   }
      
   strcpy (Symbol[Nlabels].Label, label);
   Symbol[Nlabels].Address = addr;

   Nlabels++;
   
   return (OK);
}


/* symbols --- print the symbol table */

void symbols ()
{
   int i;

   fprintf (Listing, "\nSymbol Table\n\n");

   for (i = 0; i < Nlabels; i++) {
      if (Symbol[i].References == 0)
         unused (Symbol[i].Label);
         
      fprintf (Listing, "%-14.14s %04lX  ", Symbol[i].Label, Symbol[i].Address);
      if ((i % 4) == 3)
         putc (NEWLINE, Listing);
   }

   fprintf (Listing, "\n\n%d labels used\n", Nlabels);
}


/* look_up --- look up a mnemonic in the list of opcodes */

int look_up (mnem)
const char mnem[];
{
   int i;
   char str[MAXMNEM];
   static struct {
      char dir[MAXMNEM];
      int token;
   } dirtab[] = {
      {"ORG", ORG},  /* ORG and LOC are synonyms */
      {"LOC", ORG},
      {"FCB", FCB},  /* FCB and BYT are synonyms */
      {"BYT", FCB},
      {"FCW", FCW},  /* FCW and WRD are synonyms */
      {"WRD", FCW},
      {"RMB", RMB},  /* RMB not yet implemented */
      {"TEX", TEX},
      {"EQU", EQU},
      {"END", END}   /* END does nothing */
   };

   strcpy (str, mnem);  /* Copy the mnemonic... */
   strupr (str);        /* Map to upper case */
   
   for (i = 0; i < (sizeof (Opcodes) / sizeof (Opcodes[0])); i++)
      if (strcmp (str, Opcodes[i].mnem) == 0)
         return (i);

   for (i = 0; i < (sizeof (dirtab) / sizeof (dirtab[0])); i++)
      if (strcmp (str, dirtab[i].dir) == 0)
         return (dirtab[i].token);
         
   return (ERR);
}


/* opcode_for --- find the opcode for a given mnemonic */

int opcode_for (mn, modep, opp, cycles)
const int mn;
int *modep;
address *opp;
char cycles[];
{
#ifdef DB
   fprintf (stderr, "opcode_for: mn = %d (%s), mode = %d\n", mn, Opcodes[mn].mnem, *modep);
#endif

   if ((*modep == ABSOLUTE || *modep == INDEX_X || *modep == INDEX_Y) &&
         (*opp < 256 && *opp >=0))   /* Zero page ? */
      if (Opcodes[mn].obj[*modep + Z_OFFSET] != ERR)
         *modep += Z_OFFSET;
   
   if (*modep == ABSOLUTE && Opcodes[mn].obj[*modep] == ERR) { 
      const address a1 = Addr + ADDR(2);    /* Calculate relative addressing */
      const address a2 = *opp;
      address rel = a2 - a1;
      
      if (PASS2 && (rel > 127 || rel < -128)) {
         nerd ("Branch too far");
         rel = 0;
      }

      if (rel < 0)    /* sort out two's complement */
         rel += 256;

      *modep = RELATIVE;
      *opp = rel;

      if ((a1 & 0xff00) == (a2 & 0xff00))
         strcpy (cycles, "2/4"); /* Branch within same page */
      else
         strcpy (cycles, "2/5"); /* Branch across page boundary */
   }
   else
      snprintf (cycles, MAXCYCSTR, " %1d ", Opcodes[mn].cyc[*modep]);

   return (Opcodes[mn].obj[*modep]);
}


/* directive --- handle directives */

void directive (dir, oper)
const int dir;
const char oper[];
{
   int i, stat;
   address op;

   Nbytes = 0;
   i = 0;
   switch (dir) {
   case ORG:
      if (eval (oper, &op) == ERR)
         for_ref ("ORG");
      else
         Addr = op;

      if (PASS2) {
         if (Blkptr != 0)     /* Flush out any remaining object code */
            putblock ();

         Blkaddr = Addr;
      }
      break;
   case EQU:
      if (PASS1) {   /* Ignore EQU second time around */
         if (eval (oper, &op) == ERR)
            for_ref ("EQU");
         else
            Symbol[Nlabels - 1].Address = op;
      }
      break;
   case FCB:
      do {
         stat = evaluate (oper, &i, &op);
         
         if (stat == ERR) {
            Byte[Nbytes++] = ERR;   /* May just be a forward reference in pass 1. Maintain code size */
            
            if (PASS2)
               nerd ("Undefined label in FCB directive");
         }
         else if (op <= ADDR(0xff)) {
            Byte[Nbytes++] = NUM(op);
         }
         else {
            nerd ("Byte value out of range");
            Byte[Nbytes++] = ERR;  /* Maintain code size by generating one dud byte */
         }
      } while (!(oper[i++] != ',' || Nbytes >= MAXBYTES));
      
      break;
   case FCW:
      do {
         stat = evaluate (oper, &i, &op);
         
         if (stat == ERR) {
            Byte[Nbytes++] = ERR;   /* May just be a forward reference in pass 1. Maintain code size */
            Byte[Nbytes++] = ERR;
            
            if (PASS2)
               nerd ("Undefined label in FCW directive");
         }
         else if (op <= ADDR(0xffff)) {
            Byte[Nbytes++] = NUM(op & 0xff);
            Byte[Nbytes++] = NUM(op / 256);
         }
         else {
            nerd ("Word value out of range");
            Byte[Nbytes++] = ERR;  /* Maintain code size by generating two dud bytes */
            Byte[Nbytes++] = ERR;
         }
      } while (!(oper[i++] != ',' || Nbytes >= MAXBYTES));
      
      break;
   case TEX:
      Nbytes = strlen (oper) - 2;

      if (PASS2) {
         const char term = oper[0];   /* Save the quote */
         for (i = 1; oper[i] != term && oper[i] != EOS; i++)
            Byte[i-1] = oper[i];
      }
      break;
   case END:
      /* Do nothing */
      break;
   case RMB:
      if (eval (oper, &op) == ERR)
         for_ref ("RMB");
      else
         Addr += op;    /* Skip as many bytes as the RMB directive requests */
      
      if (PASS2) {
         if (Blkptr != 0)     /* Flush out any remaining object code */
            putblock ();

         Blkaddr = Addr;
      }
      
      break;
   }
}


/* eval --- evaluate the string 'str' into an int */

int eval (str, nump)
const char str[];
address *nump;
{
   int i;

   i = 0;
   if (evaluate (str, &i, nump) == ERR || str[i] != EOS)
      return (ERR);

   return (OK);
}


/* evaluate --- evaluate the string 'str' into an integer, updating i */

int evaluate (str, ip, nump)
const char str[]; /* The string to be evaluated, */
int *ip;          /* starting at position '*ip', */
address *nump;    /* putting the resulting address here */
{
   int stat, stat1;
   address val;

   stat = OK;
   stat1 = convert (str, ip, nump);

   switch (str[*ip]) { /* Now deal with LABEL+1 and NBYTES*2 */
   case ADD:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump += val;
      break;
   case SUBTRACT:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump -= val;
      break;
   case MULTIPLY:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump *= val;
      break;
   case DIVIDE:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump /= val;
      break;
   case LOGIC_AND:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump &= val;
      break;
   case LOGIC_OR:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump |= val;
      break;
   case LOGIC_XOR:
      (*ip)++;
      stat = convert (str, ip, &val);
      *nump ^= val;
      break;
   }
   
   if (*nump > 0xffff) {
      nerd ("Address out of range");
      return (ERR);
   }

   if (stat1 == ERR || stat == ERR)
      return (ERR);

   return (OK);
}


/* convert --- convert one term of an expression to an address */

int convert (str, ip, nump)
const char str[]; /* the number to be converted, */
int *ip;          /* starting at position 'i', */
address *nump;    /* resulting address */
{
   const char prefix = str[*ip];  /* Remember high or low byte prefix - '>' or '<' */
   int stat;     /* Error flag */
   
   stat = OK;        /* no errors so far... */

   if (prefix == HIBYTE || prefix == LOBYTE)   /* skip Hi & LO prefixes */
      (*ip)++;       

   if (isalpha (str[*ip]) || (str[*ip] == '_'))
      stat = sym (str, ip, nump);  /* look up labels & return status */
   else if (isdigit (str[*ip]))
      *nump = gctol (str, ip, 10);  /* Base 10 conversion */
   else {
      switch (str[*ip]) {
      case HEX:         /* Base 16 conversion */
         (*ip)++;
         *nump = gctol (str, ip, 16);
         break;
      case BINARY:      /* Base 2 conversion */
         (*ip)++;
         *nump = gctol (str, ip, 2);
         break;
      case ASCII:       /* ASCII code constant */
         (*ip)++;
         if (str[*ip] == '^') {   /* Check for control chars ("^C) */
            (*ip)++;
            if (str[*ip] == EOS || str[*ip] == ASCII)
               *nump = ADDR('^');                    /* Caret (^) */
            else {
               *nump = ADDR(str[*ip] & 0x1f);       /* Control char */
               (*ip)++;
            }
         }
         else {
            *nump = ADDR(str[*ip]);
            (*ip)++;
         }
         if (str[*ip] == ASCII)   /* skip the closing quote */
            (*ip)++;
         break;
      case OCTAL:       /* Base 8 conversion */
         (*ip)++;   
         *nump = gctol (str, ip, 8);
         break;
      case PC:          /* Program counter - '.' */
         (*ip)++;
         *nump = Addr;
         break;
      default:
         nerd ("Syntax error in expression");
         break;
      }
   }

   if (prefix == HIBYTE)      /* Now see about the prefix... */
      *nump /= 256;
   else if (prefix == LOBYTE)
      *nump &= 0xff;

   return (stat);
}


/* sym --- locate a symbol's address if possible */

int sym (str, ip, nump)
const char str [];
int *ip;
address *nump;
{
   int j;
   char label[MAXLABEL];

   for (j = 0; isalpha(str[*ip]) || isdigit(str[*ip])
               || str[*ip] == '_'; (*ip)++)
      if (j < (MAXLABEL - 1))
         label[j++] = str[*ip];

   label[j] = EOS;
   *nump = FORWARD;    /* set value to $FFFF if not found */

   for (j = 0; j < Nlabels; j++)
      if (strcmp (label, Symbol[j].Label) == 0) {
         *nump = Symbol[j].Address;

#ifdef DB
   fprintf (stderr, "sym: label = '%s', value = %lx\n", label, *nump);
#endif

         if (PASS2)
            Symbol[j].References++;
            
         return (OK);
      }

   return (ERR);  /* return ERR if label not found */
}


/* nerd --- error message printer */

void nerd (str)
const char str[];
{
   fprintf (Errorfd, "%s at line %d\n", str, Nline);
   fputs (Line, Errorfd);

   Errs++;        
}


/* for_ref --- forward reference error */

void for_ref (str)
const char str[];
{
   fprintf (Errorfd, "%s: can't forward reference, line %d\n", str, Nline);
   Errs++;
}


/* unused --- unused label warning */

void unused (str)
const char str[];
{
   fprintf (Errorfd, "Warning: %s: unused label\n", str);
}


/* set_up --- open files, initialise globals */

void set_up (argc, argv)
const int argc;
const char *argv[];
{
   int i;
   
   if (argc > 1) {
      Source = fopen (argv[1], READ);
      if (Source == NULL)
         cant (argv[1], YES);
   }
   else
      Source = stdin;

   if (argc > 2) {
      Object = fopen (argv[2], WRITE);
      if (Object == NULL)
         cant (argv[2], YES);
   }
   else
      Object = stdout;

   if (argc > 3) {
      Listing = fopen (argv[3], WRITE);
      if (Listing == NULL)
         cant (argv[3], YES);
   }
   else
      Listing = stdout;

   Errorfd = stderr;

   Nline   = 0;
   Errs    = 0;
   Nlabels = 0;               /* Number of labels */
   
   for (i = 0; i < MAXSYMBOLS; i++) {
      Symbol[i].Label[0]   = EOS;
      Symbol[i].Address    = FORWARD;
      Symbol[i].References = 0;
   }

   for (i = 0; i < MAXBYTES; i++)
      Byte[i] = ERR;

   Nbytes  = 0;               /* Number of bytes for current instruction */
   Addr    = ADDR(0);         /* Current assembly address */
   Blkaddr = ADDR(0);         /* Address of first checksum block */
   Blkptr  = 0;               /* Block pointer */
   Hexfmt  = MOS_HEX;         /* Hex output file format */
}


/* list_it --- do stuff for the listing */

void list_it (cycles, label, mnem, mn, oper, comment)
const char cycles[], label[], mnem[];
const int mn;
const char oper[], comment[];
{
   int i;

   if (mnem[0] != EOS) {
      if (mn == EQU) {
         address eq;
         
         i = 0;

         if (sym (label, &i, &eq) == ERR)
            nerd ("Internal error in EQU directive");
            
         fprintf (Listing, "%4d: %04lX ", Nline, eq);
      }
      else
         fprintf (Listing, "%4d: %04lX ", Nline, Addr);

      for (i = 0; i < 5; i++) {    /* Why FIVE ?? */
         if (i < Nbytes && Byte[i] != ERR)
            fprintf (Listing, "%02X ", Byte[i]);
         else
            fprintf (Listing, "   ");
      }

      fprintf (Listing, "%-3.3s ", cycles);
      
      fprintf (Listing, "%-16.16s%-4.4s%-20.20s%s\n", label, mnem, oper, comment);
   }
   else if (label[0] != EOS) {
      fprintf (Listing, "%4d: %04lX                    %-16.16s                        %s\n", Nline, Addr, label, comment);
   }
   else {
      fprintf (Listing, "%4d:                         %s\n", Nline, comment);
   }
}


/* putbyte --- output a byte to the code file */

void putbyte (byte)
const int byte;
{
   Block[Blkptr++] = byte;

   if (Blkptr >= BYTES_PER_BLOCK)
      putblock ();
}


/* putblock --- puts a block of code onto the object file */

void putblock ()
{
   int i;
   int blklen;
   unsigned int checksum;

   if (Blkptr != 0) {   /* See if there's anything in the block */
      blklen = Blkptr;
      Blkptr = 0;

      switch (Hexfmt) {
      case MOS_HEX:
         fprintf (Object, ";%02X%04lX", blklen, Blkaddr);
         break;
      case SREC_HEX:
         fprintf (Object, "S1%02X%04lX", blklen, Blkaddr);
         break;
      case INTEL_HEX:
         fprintf (Object, ":%02X%04lX00", blklen, Blkaddr);
         break;
      default:
         break;
      }

      checksum = blklen;
      checksum += Blkaddr & 0xff;
      checksum += (Blkaddr >> 8) & 0xff;

      for (i = 0; i < blklen; i++) {
         fprintf (Object, "%02X", Block[i]);
         checksum += Block[i];
      }

      fprintf (Object, "%04X\n", checksum);
      
      Nblocks++;
   }

   Blkaddr += ADDR(blklen);
}


/* puteof --- puts EOF marker onto the object file */

void puteof ()
{
   int blklen;
   unsigned int checksum;

   blklen = 0;

   switch (Hexfmt) {
   case MOS_HEX:
      fprintf (Object, ";%02X%04lX", blklen, ADDR(Nblocks));
      break;
   case SREC_HEX:
      fprintf (Object, "S9%02X%04lX", blklen, 0L);
      break;
   case INTEL_HEX:
      fprintf (Object, ":%02X%04lX01", blklen, 0L);
      break;
   default:
      break;
   }

   checksum = blklen;
   checksum += ADDR(Nblocks) & 0xff;
   checksum += (ADDR(Nblocks) >> 8) & 0xff;

   fprintf (Object, "%04X\n", checksum);
}


/* cant --- print a standard error message */

void cant (path, bomb)
const char *path;
const int bomb;
{
   fputs (path, stderr);
   fputs (": can't open\n", stderr);
   
   if (bomb)
      exit (1);
}

address gctol (str, ip, base)
const char str[];
int *ip;
const int base;
{
   const char *p, *newp;
   address val;
   
   p = str + *ip;
   
   val = strtol (p, (char **)&newp, base);
   
   *ip += newp - p;
   
   return (val);
}


void strupr (char *s)
{
   while (*s) {
      if (islower (*s))
         *s = toupper (*s);
         
      s++;
   }
}
