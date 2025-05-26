<div align=center>

# AURGH

**AURGH** is an **Arch User Repository Graphical Helper**
crafted in **C++23** to simplify your everyday AUR workflow.
Independent, minimal, and modern.

![Badge gtk3]
![Badge gtk4]
[![Badge License]][License]
![Badge Language]

<img src=https://github.com/user-attachments/assets/c9ad8212-6e43-493f-94d9-ddb37c222c5c width=700></img>

</div>

## Features

> [!IMPORTANT]
> **AURGH is _not_ a frontend for pacman or other AUR helpers.**
> It is a standalone interface built to interact directly with the AUR and `libalpm`.

- [x] Basic searching of AUR packages.
- [x] Package information on each package list.
- [x] Cross compatible with GTK3 and GTK4.
- [x] AUR package removal.
- [ ] Downloading and installing AUR packages.
- [ ] Core Arch repository management.
- [ ] Basic AUR package management.

## Dependencies

| Package Name                                                  | Uses                                      |
|:--------------------------------------------------------------|:------------------------------------------|
| **[meson](https://mesonbuild.com/)**                          | Build system                              |
| **[libalpm](https://man.archlinux.org/man/libalpm.3)**        | library for Arch Linux Package Management |
| **[curl](https://curl.se/)**                                  | Fetching data from the AUR                |
| **[jsoncpp](https://github.com/open-source-parsers/jsoncpp)** | JSON handling                             |
| **[gtkmm-3/4](https://gtkmm.gnome.org/en/)**                  | C++ interface for GTK                     |

## Installation

<details>
<summary>Installing dependencies</summary>

For gtk3 builds
```bash
root# pacman -S - < required-gtk3.txt
```
or for gtk4 builds
```bash
root# pacman -S - < required-gtk4.txt
```

</details>

### Installing the package

```console
- Clone the repository and move change directory to the repository
$ git clone https://github.com/RQuarx/aurgh/ && cd aurgh

- Compiles the program
- CXX         = compiler, defaults to clang++
- CXX_LD      = linker, defaults to lld
- GTK_VERSION = GTK version to use, defaults to 4
- TARGET      = target directory, defaults to 'target'
$ make CXX=... CXX_LD=... GTK_VERSION=... TARGET=...

- Installs the program
# make install
```

## Donating

If you find this project useful, consider helping out to make my parents believe this wasn't a waste of time 😄
<br>
<br>
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/I2I11ERX5G)

## License
This project is licensed under the [GNU General Public License v3](COPYING).

[License]: COPYING

[Badge Workflow]: https://github.com/RQuarx/aurgh/actions/workflows/check_build.yml/badge.svg
[Badge gtk3]: https://github.com/RQuarx/aurgh/actions/workflows/gtk3_build.yml/badge.svg
[Badge gtk4]: https://github.com/RQuarx/aurgh/actions/workflows/gtk4_build.yml/badge.svg
[Badge Language]: https://img.shields.io/github/languages/top/RQuarx/aurgh
[Badge License]: https://img.shields.io/github/license/RQuarx/aurgh
