# trap2
> A state machine powered, GPU accelerated Terminal for macOS.

`trap2` is a terminal emulator for macOS that uses the GPU to render the terminal.
It's built with OpenGL2/SDL2 and `libvterm`, with a focus on simplicity and performance.
Currently, this project is in a very unstable and it won't even compile for you right now.

### Future Goals
This is very much a work in progress, and I have a lot of ideas for the future:

- [ ] Switch to a more powerful rendering pipeline
- [ ] Integrate clipboard support
- [ ] Batch rendering calls
- [ ] Improve font rendering
- [ ] Better testing harnesses

### Rationale
The development of this terminal emulator ties closely to my CS367 Systems Engineering class at George Mason University.
One of the projects required me to implement a shell from scratch, so I decided to try and also build a basic terminal emulator.
I've always been interested in terminal emulators, and I've always wanted to build one myself.

### Limitations
Since this is a personal project, it will revolve around my own preferences:
- No support for scrollback since I use [tmux](https://github.com/tmux/tmux) as my multiplexer
- No support for mouse input since I don't use the mouse in the terminal
- It'll really only be tested properly on macOS for now, this may change

### Building
Soon™

> Copyright (c) 2023, Aarnav Tale
