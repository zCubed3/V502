; Subroutines! The things that predate the higher level concept of returning functions!

main:
  jsr load
  jsr reset
  jmp main

load:
  lda #$FF
  pha
  pla
  jsr load2
  rts

load2:
  sta $00,X
  inx
  rts

reset:
  lda #$00
  rts
