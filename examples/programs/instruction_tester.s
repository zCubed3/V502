; Instruction Tester, contains all the instructions we have implemented for testing

test:
  nop

  ; X Register
  inx
  ldx #$01

  ; Y Register
  iny

  ; Accumulator
  lda #$09
  adc #$01
  sta $00

  ; Stack operations
  pha
  pla

  php
  plp

  jsr sub
  jmp beq_test

beq_test:
  lda #$00
  ldx #$00
  jmp beq_a_test

beq_a_test:
  adc $01
  cmp #$02
  beq beq_x_test
  jmp beq_a_test

beq_x_test:
  inx
  cpx #$02
  beq jmp_test
  jmp beq_x_test

jmp_test:
  lda test[0]
  sta $00
  lda test[1]
  sta $01
  jmp ($0000)

sub:
  rts
