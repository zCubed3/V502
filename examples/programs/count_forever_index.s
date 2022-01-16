; Counts forever but uses indexing to store A in 16 spots instead (range is 0x00 -> 0x0F

; org directive is optional, if we don't pass it, we by default map from 4000 and up, it's here just to be explicit
; since we don't have an OS for this simulator, we look for a text label
; .org 4000 ; Places the program inside 4000 and above

sta $00,X
adc #$01
inx
cpx #$10
beq #$04 ; Jump 4 spaces in memory ahead to ldx
jmp $0000

ldx #$00
jmp $0000
