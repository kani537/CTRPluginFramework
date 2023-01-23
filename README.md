# CTRPluginFramework

**CTRPluginFramework** is a framework to build plugins for Nintendo 3DS games in *3GX* format.

## Building

The latest version of devkitARM and libctru is needed to build the framework. You can run `make dist-bin` in the `Library` folder to generate a *tar* file containing the library and include files.

## Installing & Updating

The installation and updating is done through `devkitpro-pacman`.

### Adding package repository
The first time you install any *ThePixellizerOSS* software, you will need to add the package database. After you have added the database for the first time, you no longer need to do the following steps again.

#### Windows
1. Navigate to `C:/devkitPro/msys2` and run `msys2_shell.bat`.
2. Copy and paste the following text and press Enter.
```
if ! grep -Fxq "[thepixellizeross]" /etc/pacman.conf; then echo -e "\n\n[thepixellizeross]\nServer = https://thepixellizeross.gitlab.io/packages/any\nSigLevel = Optional\n" >> /etc/pacman.conf; fi
```
3. Run `pacman -Sy` and verify it mentions the `thepixellizeross` database.

### Installing
The following steps are required to install **CTRPluginFramwork**.

#### Windows
1. Navigate to `C:/devkitPro/msys2` and run `msys2_shell.bat`.
2. Run `pacman -S libctrpf` to install the package

### Updating
Any updates to **CTRPluginFramework**, alongside any other packages can be performed with the following commands.

#### Windows
1. Navigate to `C:/devkitPro/msys2` and run `msys2_shell.bat`.
2. Run `pacman -Syu` to search for updates. Make sure to run this command multiple times until it prompts there are no pending updates.

## Uninstalling
The following steps are required to remove **CTRPluginFramwork**.

#### Windows
1. Navigate to `C:/devkitPro/msys2` and run `msys2_shell.bat`.
2. Run `pacman -R libctrpf` to remove the package

## License

Copyright (c) The Pixellizer Group

This software is licensed under the following terms:

```
Permission to use, copy, modify, and/or distribute this software for any purpose is hereby granted as long as the following three conditions are met.

1) Access to any work in binary form that is based or uses part or the totality of this software must not be restricted to individuals that have been charged a fee or any other kind of compensation.

2) You are exempt of the condition 1 of this license as long as the source code of the work that is based or uses part or the totality of this software is provided along with its binary form.

3) Any copy, modification or distribution of the source code of this software must retain the same license text.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```