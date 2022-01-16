# V502 - A 6502 emulation toolkit

#### Written in C++17 but would most likely compile with older standards

### Platform Support
* Linux
* Possibly any POSIX platform supporting `time.h` and GCC/Clang with C++17
* ~~Windows~~ (TODO but should work with minimal changes!)

### Features
* Static Library! Can be linked into any program and used for anything you want!
* 6502 CPU emulation
* 6502 assembler (give it a .s file and it'll compile a .bin of it!)

### TODO
* Better assembler (labels, error handling, more instructions)
* Handling more instructions

#### Future Hopes
* Basic Commodore 64 emulation (SID chip and more complex peripherals require more work on top of CPU and system emulation)