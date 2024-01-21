# trap2
> A state machine powered, GPU accelerated Terminal for macOS.

`trap2` is a terminal emulator for macOS that uses the GPU to render the terminal.
It's built with OpenGL 4.1 Core Profile, GLFW 3, and `libvterm`, with a focus on simplicity and performance.
Currently, this project is in a very unstable and it most likely won't even compile for you right now.

### Future Goals
This is very much a work in progress, and I have a lot of ideas for the future:

- [ ] Integrate clipboard support
- [ ] Better testing harnesses
- [ ] Switch to Dawn (Google's WebGPU implementation) for rendering
- [ ] Determine the possibility of using native windowing APIs instead of GLFW

### Rationale
The development of this terminal emulator ties closely to my CS367 Systems Engineering class at George Mason University.
One of the projects required me to implement a shell from scratch, so I decided to try and also build a basic terminal emulator.
I've always been interested in terminal emulators, and I've always wanted to build one myself.

I'm perfectly content with the terminal I currently use, [WezTerm](https://wezfurlong.org/wezterm), but I just wanted to experiment.
This leads into the 3 limitations that I'm defining for this project (subject to change):
- No support for scrollback since I use [tmux](https://github.com/tmux/tmux) as my multiplexer
- No support for mouse input since I don't use the mouse in the terminal
- It'll really only be tested properly on macOS for now, this may change

### Technical Details
- The program uses `libvterm` to handle the terminal state machine
- A multi-threaded GLFW setup is used to handle the input and rendering
    - The main thread handles opening the window and taking events
    - The render thread makes all of the OpenGL draw calls
    - The `pty` thread handles reading the child fork output
    - Minimal synchronization is needed or used to keep things fast
- Rendering is done using `freetype2` and complex math to position glyphs

### Building
The project uses [CMake](https://cmake.org/) as its build system. Install `cmake` and then run the following steps:

0. Define a configuration file, see [doc/config.md](doc/config.md) for more details.
1. `mkdir build && cd build`
2. `cmake ..`
3. `make`

This should produce a binary in your `build` folder which you can run with `./build/trap2`.

> Copyright (c) 2023, Aarnav Tale
