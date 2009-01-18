; testok --- test program for the 6502 assembler               30/03/2002
; Copyright (c) John Honniball. All rights reserved

; This test program aims to test each combination of 6502 mnemonic
; and addressing mode.  All the assembler directives are also
; tested, and some corner cases are probed.  None of the code
; in this file should be incorrect, that is, it should assemble
; with no errors.  See 'testerr.asm' for error cases.

KEY             EQU     $df00             ; Keyboard port
ZP              EQU     42                ; Zero-page
VEC             EQU     $22
ABS             equ     $4200+$0042
THERE           equ     $4242-$0042
IND             equ     $0040|$0002
BADEQU          equ     $600D
LASTBYTE        equ     $FFFF             ; Highest address
                                          ; Semi-blank line
                ORG     $0400             ; Start of user RAM
                
; Labelled but otherwise blank line
START
START1          ; begin here
                BRK                       ; Mnemonics are case-insensitive
                brk
                asl a
                ASL A                     ; Either 'a' or 'A' for Accumulator
                PHP
                CLC
                PLP
                SEC
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

                ASL
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
                BVS     .+2
                BCC     .+2
                BCS     NEXTPG
                BNE     NEXTPG
                BEQ     NEXTPG

                JMP     THERE
                JMP     (IND)
                JMP     START
                JMP     START1
                
; Time to test the directives
                ORG     $0500
NEXTPG          byt     $ff,$fe,$fd,$fc
                byt     "A"
                WRD     0,1,2,3
                WRD     "Z"
                TEX     "Hello, world"
                fcb     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                fcw     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                
                LDA     LASTBYTE
                LDA     LASTBYTE-1
                JMP     ENDBYTE
                nop
                nop

                org     $FFF0             ; Make sure addresses are OK
                jmp     .                 ; right up to the very end
                jmp     .+3
                lda     .
                ldy     .
                jmp     .                 ; Corner case: use very last address
ENDBYTE         end
