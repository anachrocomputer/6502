# asm #

A 6502 cross-assembler in C to run on Linux.

## History ##

Originally written in Ratfor on a Prime P750 running the
Georgia Tech Software Tools Subsystem.
Ratfor is a language that is converted into FORTRAN before being compiled by
the Prime's standard FORTRAN compiler.
Some functions and #define names are remnants of the Software Tools version,
such as EOS, ERR, MAXLINE, SKIPBL(), 'gctol()', and 'cant()'.

Subsequently converted into K&R C for the PC running MS-DOS and the
Atari ST running TOS.
As such, the code assumed a 16-bit 'int' and had to use a 'long int' for
any data type that might exceed 16 bits.

Later still, converted to ANSI C by the addition of function prototypes.
The old K&R style function definitions were retained for backward compatibility.

Ported to Linux and placed in a Subversion repository.
One remaining sign of this port is the 'strupr()' function that replaces a
function in the Microsoft C library on MS-DOS.
Another Software Tools function 'gctol()' was replaced with a small wrapper that
calls 'strtol()'.

## Building the Program ##

We will need the C compiler, linker and libraries:

`sudo apt-get install build-essential`

Once those are installed, we can simply:

`make`

## TODO ##

Fix test case for use of byte at address $FFFF. Also fix resulting bug in assembler.

Fix stack smashing when the comment is too long. First, make a test case.

Fix listing of address of start of RMB directive. Labels at this location work as they should,
but the listing shows the wrong address. Code generation is also correct. Fixing this would
also allow ORG directives to be labelled, with the label taking the value of the Program Counter
before the ORG takes effect (if that's any use).

Fix bogus 'Address out of range' error in pass 1 when adding constants to a forward-reference symbol.

Add warnings when long operands or comments are truncated. Probably needs a generic warning mechanism.

Maybe, just maybe, allow TABs between fields.

Fully implement the generation of Motorola S-Records and Intel Hex format files.

Add CMOS 6502 instructions and op-codes.
The assembler was written at a time when the 65C02 was very new,
if available at all.

Add a check in pass 2 that verifies that label addresses are identical to those in pass 1.
Any discrepancy is an assembler internal error.

Add .asciiz directive.

Allow directive names to begin with a dot.

Generate a warning if a JMP indirect vector crosses a page boundary (NMOS 6502 bug)

Implement a file inclusion mechanism.

Sort symbol table into alphabetical order in listing file.

Implement a better symbol table mechanism.
The original was coded (in Ratfor) as a big array with a linear search algorithm.
We can do better.

Use standard C types from 'stdtypes.h' and 'stdbool.h'.

Fully convert to ANSI C by removing K&R style function definitions.
Are any K&R-only C compilers still in use?

