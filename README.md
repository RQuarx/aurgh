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

**Core dependencies** (common to both GTK3 and GTK4 builds):
```console
meson pkgconf curl jsoncpp pacman
```

**GTK3 Build**:
```console
gtkmm3 glibmm
```
**GTK4 Build**:
```console
gtkmm-4.0 glibmm-2.68
```

## Installation

### 1. Clone the repository

```console
$ git clone https://github.com/RQuarx/aurgh/
$ cd aurgh
```
---

### 2. Build the project

Use the `make` command to compile the project. You can customize the build with these optional environment variables:

| Variable      | Description                 | Default   |
| ------------- | --------------------------- | --------- |
| `CXX`         | C++ compiler                | `clang++` |
| `CXX_LD`      | Linker                      | `lld`     |
| `GTK_VERSION` | GTK version to compile with | `4`       |
| `TARGET`      | Output directory            | `target`  |

**Example:**

```console
$ make CXX=g++ CXX_LD=bfd GTK_VERSION=3 TARGET=build
```
---
### 3. Install the program

The install script requires root permission to run, make sure to run it with `sudo`
or a similar command

```console
# make install
```

## Donating

If you find this project useful, consider donating to support its development
(and help convince my parents this wasn’t a waste of time 😄).
<br>
<br>
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/I2I11ERX5G)

## License
This project is licensed under the [GNU General Public License v3](COPYING).

<!-- Badge references -->
[License]: COPYING

[Badge Workflow]: https://github.com/RQuarx/aurgh/actions/workflows/check_build.yml/badge.svg
[Badge gtk3]: https://github.com/RQuarx/aurgh/actions/workflows/gtk3_build.yml/badge.svg
[Badge gtk4]: https://github.com/RQuarx/aurgh/actions/workflows/gtk4_build.yml/badge.svg
[Badge Language]: https://img.shields.io/github/languages/top/RQuarx/aurgh
[Badge License]: https://img.shields.io/github/license/RQuarx/aurgh
