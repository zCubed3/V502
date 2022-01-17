; Indirect jumping example, we load our counter functions location then jump to it indirectly
; Syntax sugar I've added, if you do label[0] or label[1] you can get the high and low byte

first:
  lda count[0]
  sta $00
  lda count[1]
  sta $01

  jmp main

main:
  jmp ($0000)

count:
  adc #$01
  jmp main
