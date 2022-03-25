; Counts up indefinitely, storing it in 0x00

.org $4000

loop:
  adc #$01
  sta $00
  jmp loop
