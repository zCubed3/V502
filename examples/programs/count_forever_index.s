; Counts forever but uses indexing to store A in 16 spots instead (range is 0x00 -> 0x0F)
sta $00,X
adc #$01
inx
cpx #$10
beq #$04 ; Jump 4 spaces in memory ahead to ldx
jmp $0000

ldx #$00
jmp $0000
