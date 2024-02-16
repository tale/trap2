# trap2
> A "suckless-inspired" GPU accelerated terminal emulator.

`trap2` is a terminal emulator that uses the GPU to render the terminal.
It's built with OpenGL 3.3 Core Profile, GLFW 3, and `libvterm`, with a focus on simplicity and performance.

### Future Goals
This is very much a work in progress, and I have a lot of ideas for the future:

- [ ] Integrate clipboard support
- [ ] Better testing harnesses

### Rationale
The development of this terminal emulator ties closely to my CS367 Systems Engineering class at George Mason University.
One of the projects required me to implement a shell from scratch, so I decided to try and also build a basic terminal emulator.
I've always been interested in terminal emulators, and I've always wanted to build one myself.

This is a learning project for me, so it will always be an amalgamation of new concepts and optimizations that I learn.
You can refer to these in the [doc](doc) folder, where I have most of the technical details and design decisions.

This leads into the 3 limitations that I'm defining for this project (subject to change):
- No support for scrollback since I use [tmux](https://github.com/tmux/tmux) as my multiplexer
- No support for mouse input since I don't use the mouse in the terminal
- It'll really only be tested properly on macOS for now, this may change

### Technical Details
- The program uses `libvterm` to handle the terminal state machine
- A multi-threaded GLFW setup is used to handle the input and rendering
    - The main thread handles windowing and making rendering calls
    - The `pty` thread handles reading the child fork output
    - Minimal synchronization is needed or used to keep things fast
- Rendering is done using `freetype2` and complex math to position glyphs

### Building
The project uses [CMake](https://cmake.org/) as its build system. Install `cmake` and then run the following steps:

0. Define a configuration file, see [doc/config.md](doc/config.md) for more details.
1. `mkdir build && cd build`
2. `cmake ..`
3. `cd .. && cmake --build build --parallel`

This should produce a binary in your `build` folder which you can run with `./build/trap2`.

> Copyright (c) 2023, Aarnav Tale
