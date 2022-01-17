; Counts forever but uses indexing to store A in 16 spots instead (range is 0x00 -> 0x0F

; org directive is optional, if we don't pass it, we by default map from 4000 and up, it's here just to be explicit for demonstration
.org 4000 ; Places the program inside 4000 and above

main:
  sta $00,X
  adc #$01
  inx
  cpx #$10
  beq reset ; Jump to reset if cpx returns equal
  jmp main

reset:
  ldx #$00
  jmp main
