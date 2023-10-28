; testerr --- test program for the 6502 assembler              2002-03-30
; Copyright (c) John Honniball. All rights reserved

; This test program attempts to make plausible coding errors
; in order to test the assembler's error detection and reporting
; mechanism.  For a test program that should assemble without
; errors, see 'testok.asm'.

KEY             EQU     $df00             ; Keyboard port
ZP              EQU     42                ; Zero-page
VEC             EQU     $22
ABS             equ     $4200+$0042
THERE           equ     $4242-$0042
IND             equ     $0040|$0002
BADEQU          equ     $600D
                equ     $BAD0             ; Un-named EQU
DUPEQU          equ     $DEAD             ; Duplicate EQU
DUPEQU          equ     $BEEF
LASTBYTE        equ     $FFFF             ; Highest address
                                          ; Semi-blank line
ORG_LABEL       ORG     $0400             ; ORG should not be labelled
                
START
UNUSED_LABEL                              ; Should generate a warning but not an error
LONG__BUT_IS_OK                           ; Long label name
TOO__LONG_BY_ONE                          ; One char too long
TOO__LONG__BY_TWO                         ; A bit more too long
LABEL_THAT_IS_WAAAAAY_TOO_LONG            ; Will be truncated with a warning

START1          ; begin here
TOO_LONG__BY_ONE  BRK
TOO_LONG__BY__TWO brk
                asl     a
                lda     ABS=X             ; Syntax error
                LDA     ABS=X
BAD-LABEL       LDA     ABS
2ND_BAD_LABEL   LDA     ABS
Z%$@|           LDA     ABS

a               ASL     A                 ; Invalid labels
A               ASL     a

                LEA     ABS,X             ; Invalid mnemonic
                MOV     D1,D3
                INC     R3+
                PHX
                SEX
                LEAX
                MOV.L
                MOVSB
                MOVSB.W                   ; Will be truncated
                
                RTS     ZP                ; Invalid address modes
                LDA
                LDA     A
                ASL     X
                ROR     Y
                INX     ABS
                LDX     ABS,X
                LDY     ZP,Y
                BCC     ABS,Y
                STA     (ZP),X
                STX     (ZP)
                STY     (ZP,Y)
                LDA     [X]
                STA     [Y]
                JMP     [ABS]
                JSR     [BP+2]
                
                LDA     ABS,Q             ; Invalid register names
                BNE     ABS,Z
                SBC     ZP,A
                LDX     ABS,R1
                LDY     ZP,A3
                STA     ABS,HL
                
                lda     #256              ; Immediate data out-of-range
                ldx     #$ffff
                ldy     #65536
                lda     65536             ; Address out-of-range
                sta     $fffff
                
                PHP
                JMP     NOWHERE
                JMP     LASTBYTE+1
                JMP     LASTBYTE+256
                JMP     LONG__BUT_IS_OK
                JMP     TOO_LONG__BY_ONE  ; Will be silently truncated
                JMP     TOO__LONG_BY_ONE  ; Will be silently truncated
                JMP     TOO__LONG__BY_TWO ; Will be silently truncated
                JMP     TOO_LONG__BY__TWO ; Will be silently truncated
                JMP     LABEL_THAT_IS_WAAAAAY_TOO_LONG ; Will be silently truncated
                JMP     ORG_LABEL
                JMP     COLON_LABEL
                
                CLC
DUPLABEL        PLP                       ; Duplicate label
                SEC
DUPLABEL        PHA
                cli
COLON_LABEL::   pla                       ; One colon is OK, two are not
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
                LDA     DUPEQU
                
                ORG     $0500
NEXTPG          byt     $ff,$fe,$fd,$fc
                WRD     0,1,2,3
                TEX     "Hello, world"
                .asciiz "Hello, world"    ; Directive too long, will be truncated
                TEX     "1234567890123456789012345678901234567890123456789012345678901234567890123456789012"
                BYT     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,30,30,31,32
                fcb     256,257,258
                fcb     $100,$fff,$ffff,$fffff
                .fcb    13,10,255
                FCW     65536,65537
                fcw     $1000,$fffff
                FCB     <FORWARD+1
                FCB     <FORWARD+2
                FCW     FORWARD+1         ; Bogus 'Address out of range' error
                FCW     FORWARD+2         ; Bogus 'Address out of range' error
                FCW     NOWHERE
                BYT     UNDEF_BYTE
                FCW     -1
                BYT     -1
                FCW     ,1
                byt     1,2,
                byt     1,,3
                
FORWARD         LDA     LASTBYTE
                LDA     LASTBYTE-1
                JMP     NEARTOP+256
                JMP     ENDBYTE
                nop
                nop

BOGUS_ORG       org     $FFF0             ; Can't label ORGs
NEARTOP         jmp     .
                jmp     BOGUS_ORG
                jmp     .
                jmp     .
                jmp     .
                nop                       ; This NOP is at $FFFF, which is valid. But the assembler fails anyway
ENDBYTE         end
