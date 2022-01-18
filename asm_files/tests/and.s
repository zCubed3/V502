; Tests all variants of the AND instruction

main:
  lda #%11110000
  sta $00
  sta $01

  ; NOW
  and #%11000000

  ldx #$01
  ldy #$01

  ; ZPG
  lda #$FF
  and $00

  ; ZPG + X
  lda #$FF
  and $00,X

  ; ABS
  lda #$FF
  and $0000

  ; ABS + X
  lda #$FF
  and $0000

  ; ABS + Y
  lda #$FF
  and $0000

  ; IND + X
  lda #$FF
  and $0010,X

  ; IND + Y
  lda #$FF
  and $0010,Y

  jmp main
