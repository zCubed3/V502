; Instruction Tester, contains all the instructions we have implemented for testing

test:
  nop

  ; X Register
  inx
  txa
  dex
  tax

  ldx $00 ; zpg
  ldx $00,Y ; zpg + y
  ldx $0000 ; abs
  ldx $0000,Y ; abs + y

  tsx
  txs

  ldx #$0F

  ; Y Register
  iny
  tya
  dey
  tay

  ldy $00 ; zpg
  ldy $00,X ; zpg + x
  ldy $0000 ; abs
  ldy $0000,X ; abs + x

  ldy #$F0

  ; Accumulator
  lda #$09 ; now
  adc #$01
  sta $00
  sbc #$05
  sta $FF

  lda $00 ; zpg
  lda $00,X ; zpg + x
  lda $0000 ; abs
  lda $0000,X ; abs + x
  lda $0000,Y ; abs + y
  lda ($00,X) ; zpg + x, indirect
  lda ($00),Y ; zpg, indirect + y

  ; Flow
  jsr sub_stack
  jmp beq_test

beq_test: ; Sets up the branching tests
  lda #$00
  ldx #$00
  jmp beq_a_test

beq_a_test: ; Add 1 to A until it's equal to 2, then branch
  adc $01
  cmp #$02
  beq beq_x_test
  jmp beq_a_test

beq_x_test: ; Increment X until it's equal to 2, then branch
  inx
  cpx #$02
  beq ind_jmp_test
  jmp beq_x_test

ind_jmp_test: ; Indirect jumping back to the start
  lda test[0]
  sta $00
  lda test[1]
  sta $01
  jmp ($0000)

sub_stack: ; Subroutine that does stack operations
  pha
  pla

  php
  plp

  rts
