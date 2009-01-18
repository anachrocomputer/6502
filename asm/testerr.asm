; testerr --- test program for the 6502 assembler              30/03/2002
; Copyright (c) John Honniball. All rights reserved

; This test program attempts to make plausible coding errors
; in order to test the assembler's error detection and reporting
; mechanism.  For a test program that should assemble without
; errors, see 'testok.asm'.

KEY             EQU     $df00             ; Keyboard port
KEY             EQU     $df01             ; Duplicate label
ZP              EQU     42                ; Zero-page
VEC             EQU     $22
ABS             equ     $4200+$0042
THERE           equ     $4242-$0042
IND             equ     $0040|$0002
BADEQU          equ     $600D
                equ     $BAD0             ; Un-named EQU
LASTBYTE        equ     $FFFF             ; Highest address
                                          ; Semi-blank line
                ORG     $0400             ; Start of user RAM
                
START
START1          ; begin here
                BRK
                brk
                asl a
                lda ABS=X                 ; Syntax error
                LDA ABS=X
BAD-LABEL       LDA ABS
2ND_BAD_LABEL   LDA ABS
Z%$@|           LDA ABS
a               ASL A                     ; Invalid labels
A               ASL a
                LEA ABS,X                 ; Invalid mnemonic
                RTS ZP                    ; Invalid address modes
                LDA
                INX ABS
                LDX ABS,X
                LDY ZP,Y
                PHP
                JMP     NOWHERE
                JMP     LASTBYTE+1
                JMP     LASTBYTE+256
                CLC
DUPLABEL        PLP                       ; Duplicate label
                SEC
DUPLABEL        PHA
                cli
                pla
                sei
                dey
                tya
                tay
                clv
                iny
                cld
                inx
                sed
                
THISADDR        EQU     .
THATADDR        EQU     .-8
HERE            JMP     HERE

TOO_FAR         ASL
                ROL
                LSR
                ROR
                TXA
                TXS
                TAX
                TSX
                DEX
                NOP
                
                RTI
                RTS

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

                BPL     _OK_LABEL
                JSR     THERE+4
                BMI     GOOD_LABEL
                BVC     GOOD_LABEL2
                BEQ     TOO_FAR
                BVS     .+2
                BCC     .+2
                BCS     NEXTPG
                BNE     NEXTPG
                BEQ     NEXTPG

                JMP     THERE
                JMP     (IND)
                JMP     START
                JMP     START1
                JMP     DUPLABEL
                
                ORG     $0500
NEXTPG          byt     $ff,$fe,$fd,$fc
                WRD     0,1,2,3
                TEX     "Hello, world"
                fcb     13,10,256
                FCW     4,65536
                FCW     NOWHERE
                BYT     UNDEF_BYTE
                FCW     -1
                BYT     -1
                FCW     ,1
                byt     1,2,
                byt     1,,3
                
                LDA     LASTBYTE
                LDA     LASTBYTE-1
                JMP     NEARTOP+256
                JMP     ENDBYTE
                nop
                nop

BOGUS           org     $FFF0             ; Can't label ORGs
NEARTOP         jmp     .
                jmp     BOGUS
ENDBYTE         end
