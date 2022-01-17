; Pushes a value to the stack 10 times then pops it 10 times!

push_loop:
  pha
  adc #$01
  cmp #$10
  beq pop_loop
  jmp push_loop

pop_loop:
  pla
  cmp #$00
  beq push_loop
  jmp pop_loop
