# Configuration
One of the requirements of this project is minimal dependencies, so I've implemented a very basic `conf` parser. There are specific caveats to keep in mind and more details about how `trap2` handles the configuration loading.

### Loading
By default, a configuration file will be read from `$XDG_CONFIG_HOME/trap2/trap2.conf` and fallback to `$HOME/.config/trap2/trap2.conf` if the XDG variable is not found/defined. Keep in mind that if values are not defined in the configuration, then they will fallback to their original values:
```c
config_t config = {
    .width = 800,
    .height = 600,
    .font = "",
    .font_size = 12,
    .shell = "/bin/bash",
    .argv = {"bash", NULL},
    .opacity = 1.0,
};
```

### Reference
- `Shell`: Path to the shell for the terminal to use
- `ShellArgs`: Launch arguments (not including the program name)
- `Font`: A path to a `.ttf` file or an installed font on macOS
- `FontSize`: A floating point value for your font size (ex: 12.5)
- `Width`: The window width to launch the terminal with
- `Height`: The window height to launch the terminal with
- `Opacity`: A value between 0.0 and 1.0 for background opacity

It's mostly straight-forward, so here is an example configuration.
```conf
Shell /bin/zsh
ShellArgs "--interactive --login"
Font ~/fonts/iosevka.ttf
FontSize 15
Width 800
Height 600
Opacity 0.75
```

### Caveats
- When the font is blank, the default macOS system font will be loaded, or it'll fall back to a fuzzy box.
- On macOS, the font field can be the name of the font, on other platforms it needs to be a path to a `.ttf`.
- Opacity currently has no effect because it causes issues with OpenGL on macOS.
- Terminal rows and columns are calculated automatically based on the font and window size.
- Single, double, and no quotes are supported in the configuration
- Comments start with `#` and can go at the end of lines too
- `~` will expand to the value of `$HOME`
