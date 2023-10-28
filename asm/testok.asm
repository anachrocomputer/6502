; testok --- test program for the 6502 assembler               2002-03-30
; Copyright (c) John Honniball. All rights reserved

; This test program aims to test each combination of 6502 mnemonic
; and addressing mode.  All the assembler directives are also
; tested, and some corner cases are probed.  None of the code
; in this file should be incorrect, that is, it should assemble
; with no errors.  See 'testerr.asm' for error cases.

KEY             EQU     $df00             ; Keyboard port
ZP              EQU     42                ; Zero-page
VEC             EQU     $22
ABS             equ     $4200+$0042       ; Absolute addresses
THERE           equ     $4242-$0042
IND             equ     $0040|$0002       ; Zero-page address

CASE            equ     "U"               ; Labels are case-sensitive but
case            EQU     "u"               ;  mnemonics are not

FIVE            equ     3+2               ; Check arithmetic
SIX             equ     3*2
SEVEN           equ     49/7
EIGHT           equ     10-2
CHKAND          equ     $4242&$FF
CHKEOR          equ     $B2B2^$F0F0

BINARY          equ     %1010101010101010 ; Check number bases
OCTAL           equ     @377
HEXUC           equ     $55AA
HEXLC           equ     $aa55
DECIMAL         equ     65535
HIBY            EQU     >HEXUC
LOBY            EQU     <HEXUC
HIHEX           equ     >$AA55
LOHEX           equ     <$AA55

LASTBYTE        equ     $FFFF             ; Highest valid address
                                          ; Semi-blank line
                ORG     $0400             ; Start of user RAM
                
; Labelled but otherwise blank line
START
START1          ; begin here
START2          BRK                       ; Mnemonics are case-insensitive
L23456789012    brk                       ; Labels can be up to 12 characters long
AAAAAAAAAAAA    PHP
ZZZZZZZZZZZZ    pha
_ZZZZZZZZZZZ    PLP                       ; Labels can begin with an underscore
_A_A_A_A_A_A    pla                       ;  and contain underscores
COLON_LABEL:    SEC                       ; Allow a label to be followed by a colon for
COLON:          CLC                       ;  compatibility with assemblers that require one
                cli
                sei
                clv
                cld
                sed
                dex
                dey
                inx
                iny
                txa
                tya
                tax
                tay
                txs
                tsx
                NOP
                
                RTI
                RTS
                
THISADDR        EQU     .
THATADDR        EQU     .-8
HERE            JMP     HERE

                asl     a
                ASL     A                 ; Either 'a' or 'A' for Accumulator
                ASL
                ROL
                ROL     a
                rol     a
                LSR
                LSR     A
                lsr     a
                ROR
                ROR     A
                ror     a

                ADC     #$2A
                ADC     ZP
                ADC     ZP,X
                ADC     ABS
                ADC     ABS,X
                ADC     ABS,Y
                ADC     (VEC,X)
                ADC     (VEC),Y

                AND     #$2A
                AND     ZP
                AND     ZP,X
                AND     ABS
                AND     ABS,X
                AND     ABS,Y
                AND     (VEC,X)
                AND     (VEC),Y

                CMP     #$2A
                CMP     ZP
                CMP     ZP,X
                CMP     ABS
                CMP     ABS,X
                CMP     ABS,Y
                CMP     (VEC,X)
                CMP     (VEC),Y

                EOR     #$2A
                EOR     ZP
                EOR     ZP,X
                EOR     ABS
                EOR     ABS,X
                EOR     ABS,Y
                EOR     (VEC,X)
                EOR     (VEC),Y

                LDA     #$2A
                LDA     ZP
                LDA     ZP,X
                LDA     ABS
                LDA     ABS,X
                LDA     ABS,Y
                LDA     (VEC,X)
                LDA     (VEC),Y

                ORA     #$2A
                ORA     ZP
                ORA     ZP,X
                ORA     ABS
                ORA     ABS,X
                ORA     ABS,Y
                ORA     (VEC,X)
                ORA     (VEC),Y

                SBC     #$2A
                SBC     ZP
                SBC     ZP,X
                SBC     ABS
                SBC     ABS,X
                SBC     ABS,Y
                SBC     (VEC,X)
                SBC     (VEC),Y

                STA     ZP
                STA     ZP,X
                STA     ABS
                STA     ABS,X
                STA     ABS,Y
                STA     (VEC,X)
                STA     (VEC),Y

                CPX     #$2a
                CPX     ZP
                CPX     ABS

                CPY     #$2A
                CPY     ZP
                CPY     ABS

                LDX     #$2a
                LDX     ZP
                LDX     ZP,Y
                LDX     ABS
                LDX     ABS,Y

                LDY     #$2A
                LDY     ZP
                LDY     ZP,X
                LDY     ABS
                LDY     ABS,X

                STX     ABS
                STX     ZP
                STX     ZP,Y

_OK_LABEL       STY     ABS
GOOD_LABEL      STY     ZP
GOOD_LABEL2     STY     ZP,X

                LDA     L23456789012      ; Use the long labels so we don't get a warning
                LDA     AAAAAAAAAAAA      ;  and also as a test
                LDA     ZZZZZZZZZZZZ
                LDA     _ZZZZZZZZZZZ,x
                LDA     _A_A_A_A_A_A,y
                LDA     #<_ZZZZZZZZZZZ+<AAAAAAAAAAAA
                LDY     #>_A_A_A_A_A_A|>ZZZZZZZZZZZZ
                STA     COLON_LABEL
                STY     COLON

; Confusing but legal label names
X               NOP
Y               NOP
x               jmp     y
y               jmp     x
                BEQ     X
                LDA     X,Y
                BCC     Y
                STA     Y,x
                STX     Y
                STY     X
                ASL     X
                LSR     Y
                ROR     x
                ROL     y

                BPL     _OK_LABEL
                JSR     THERE+4
                BMI     GOOD_LABEL
                BVC     GOOD_LABEL2
                LDA     #>GOOD_LABEL
                LDY     #<GOOD_LABEL
                
                org     $0580
                BNE     .
                BEQ     .
                BMI     .+1               ; Should this address be illegal?
                BPL     .+1
                BVS     .+2
                BCC     .+2
                BCS     NEXTPG
                BCC     NEXTPG
                BEQ     NEXTPG
                BNE     NEXTPG
                BMI     NEXTPG
                BPL     NEXTPG
                BVS     NEXTPG
                BVC     NEXTPG

                JMP     THERE
                JMP     (IND)
                JMP     START
                JMP     START1
                JMP     START2
                
                
; Time to test the directives
                ORG     $0600
NEXTPG          byt     $ff,$fe,$fd,$fc
                byt     "A","B","C","D"
                byt     %00,%01,%10,%11
                byt     10,11,12,13
                byt     @10,@11,@12,@13
                byt     $10,$11,$12,$13
                byt     255,@377,$ff,%11111111
                BYT     <HERE,>HERE,<GOOD_LABEL,>GOOD_LABEL
                WRD     0,1,2,3
                WRD     "Y","Z"
                wrd     %1010101001010101,%1111000011110000
                wrd     10,11
                wrd     @10,@11
                wrd     $aa55,$f0f0
                wrd     65535,@177777
                wrd     $ffff,%1111111111111111
                WRD     <HERE,>HERE
                WRD     <GOOD_LABEL,>GOOD_LABEL
                tex     "abcde"
                TEX     "12345"
                TEX     "Hello, world"
                fcb     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                fcw     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                FCB     $00,$01,$FE,$FF
                FCW     $0000,$0001
                FCW     $FFFE,$FFFF
                FCW     LASTBYTE-1,LASTBYTE
                FCW     ENDBYTE-1,ENDBYTE
                FCB     <FORWARD
                FCB     <FORWARD-1
                FCB     <FORWARD-2
                FCW     FORWARD
                FCW     FORWARD-1
                FCW     FORWARD-2
                
FORWARD         LDA     LASTBYTE
                LDA     LASTBYTE-1
                LDX     <HERE
                LDY     >HERE
                STA     <ZP
                STY     >ZP
                JMP     ENDBYTE
                nop
                nop

; Some tests for the HEX file and checksums
                org     $0700
                fcw     $0000,$0000       ; 24 bytes of $00 to minimise the checksum
                fcw     $0000,$0000
                fcw     $0000,$0000
                fcw     $0000,$0000
                fcw     $0000,$0000
                fcw     $0000,$0000
                
                fcw     $ffff,$ffff       ; 24 bytes of $FF to maximise the checksum
                fcw     $ffff,$ffff
                fcw     $ffff,$ffff
                fcw     $ffff,$ffff
                fcw     $ffff,$ffff
                fcw     $ffff,$ffff
                
; Test bug in NMOS 6502
                org     $07ff
JMP_VEC         WRD     $AAAA             ; JMP vector crosses page boundary
                LDA     #<HERE
                LDY     #>HERE
                STA     JMP_VEC
                STY     JMP_VEC+1
                JMP     (JMP_VEC)         ; JMP indirect will fail
                
; Some checks around the address $8000
                ORG     $7ffa
                bcc     .+16
                bpl     ABOVE
                nop
BELOW           nop                       ; At address $7FFF
                nop                       ; At address $8000
ABOVE           beq     BELOW                                
                bne     .-16
                
                org     $FFEC             ; Make sure addresses are OK
                jmp     .                 ; right up to the very end
                jmp     .+3
                beq     ENDBYTE
                bne     ENDBYTE
                lda     .
                ldy     .
                jmp     .                 ; Corner case: use very last address
ENDBYTE         end
