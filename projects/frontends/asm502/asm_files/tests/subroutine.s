; Subroutine tester

main:
  jsr count
  jmp main

count:
  adc #$01
  rts
