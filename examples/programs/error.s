; Error tester, "real fake code" is here that should crash the assembler if it is behaving correctly
; jmp $00 ; Should be recognized as jmp but warn about an unknown variant
; sum $00 ; Should be unrecognized and warn about being unknown
