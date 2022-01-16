; Counts forever but uses indexing to store A in 16 spots instead (range is 0x00 -> 0x0F)
adc #$01
sta $00,X
inx
cpx #$10
beq #$04
jmp $0000
ldx #$00
jmp $0000
