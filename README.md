# V502 - 6502 Simulation Toolkit 
### 6502 Simulator, Assembler, and Disassembler

---

### Implementation Info

1) **Written in C99 using a modern C standard library! This library should be portable enough for you to integrate into a modern or old C/C++ project!**
   1) Functions like `strtol` and other modern `stdlib.h` and `string.h` functions used might reduce the portability of this library, this should be fixable by providing your own implementations of the following functions
2) **The frontends are written using the C++17 standard but could be backported to earlier C++ standards, their main purpose is to provide example code!**
   1) As far as I am aware, most of the features used are present in C++14 and below, but probably wont work with super old standards!

---

#### Warning: I only have about 2 1/2 years of experience using C and C++, some of the code within this project might display some of the worst practices you'll ever lay your eyes on. I apologize for this, I'm still learning!

### Why C?
* Portability but also because I prefer C over C++ despite C++ being superior for this task

### Platform Support
* Linux
* Windows

### Features
* Static Library! Can be linked into any program and used for anything you want!
* 6502 CPU simulator
* 6502 assembler (give it a .s file and it'll compile a .bin of it!)

### Building
1) Download CMake for your platform
   1) For Linux, download it using your package manager
   2) For Windows download it from https://cmake.org/download/
2) Generate Build files using CMake
   1) For Linux, running `cmake -S . -B cmake-build` is sufficient when CD'ed into the source
   2) For Windows, generate the cmake project and compile it in Visual Studio or Codeblocks
   3) **NOTE: If building the frontends too put `-DV502_FRONTENDS=""` and `-DV502_FRONTEND_GUI=""` in the cmake arguments!**
3) Compile
   1) For Linux, if using make generator do `make -C cmake-build` or cd into `cmake-build` and run `make`
   2) This is self explanatory for Windows

### GUI Frontend Third Party Software
#### [Credits](CREDITS)

* [imgui](https://github.com/ocornut/imgui/) - MIT License [here](https://github.com/ocornut/imgui/blob/master/LICENSE.txt)
* [glew](https://github.com/nigels-com/glew) - BSD License [here](http://glew.sourceforge.net/glew.txt)
* [GLFW](https://github.com/glfw/glfw) - zlib License [here](https://www.glfw.org/license)