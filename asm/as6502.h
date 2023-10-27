/* Definitions for the 6502 assembler                                */
/* Copyright (c) 1983 John Honniball, Bambleweeny Computer Systems   */

#define MAXSYMBOLS    500
#define MAXCOMMENT     80
#define MAXLABEL       13
#define MAXMNEM         5
#define MAXCYCSTR       5
#define MAXOPER        81
#define MAXBLOCK       80
#define MAXBYTES      256

#define PASS1  (Pass == 1)
#define PASS2  (Pass == 2)

#define SKIPBL(lin, i) while (lin[i] == ' ' || lin[i] == '\t') i++

#define EOS         '\0'
#define NEWLINE     '\n'
#define TTY       stderr

#define OK             0
#define ERR           -1
#define YES            1
#define NO             0

#define MAXLINE      256

#define READ         "r"
#define WRITE        "w"

#define COMMENT_SYM  ';'
#define LABEL_SYM    ':'   /* Allow colon after a label because many assemblers require that */
#define INDEX_SYM    ','
#define INDIRECT_SYM '('
#define INDIRECT_END ')'
#define IMM_SYM      '#'

#define HEX          '$'
#define BINARY       '%'
#define ASCII        '"'
#define OCTAL        '@'
#define PC           '.'
#define HIBYTE       '>'
#define LOBYTE       '<'

/* Binary operators */

#define ADD          '+'
#define SUBTRACT     '-'
#define MULTIPLY     '*'
#define DIVIDE       '/'
#define LOGIC_AND    '&'
#define LOGIC_OR     '|'
#define LOGIC_XOR    '^'

/* Addressing modes -- used to index array in the opcode table */

#define INHERENT        0
#define IMMEDIATE       1
#define ABSOLUTE        2
#define INDEX_X         3
#define INDEX_Y         4
#define Z_PAGE          5
#define Z_INDEX_X       6
#define Z_INDEX_Y       7
#define INDIRECT_X      8
#define INDIRECT_Y      9
#define RELATIVE       10
#define INDIRECT       11
#define Z_OFFSET        3    /* Offset from ABS to Z_PAGE */
#define MAXMODES       12

#define ORG          -100
#define FCB          -101
#define FCW          -102
#define RMB          -103
#define EQU          -104
#define END          -105
#define TEX          -108
#define IS_DIRECTIVE(m) (m < 0)

typedef long int address;
#define ADDR(n)  ((address)(n))
#define NUM(n)   ((int)(n))
#define FORWARD     0xffff      /* Forward reference value */

#define BYTES_PER_BLOCK  24

#define MOS_HEX      1           /* MOS Technology Paper tape format */
#define SREC_HEX     2           /* Motorola S-Record hex format */
#define INTEL_HEX    3           /* Intel Hex format */

