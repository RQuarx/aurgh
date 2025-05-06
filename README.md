<div align=center>

# AURGH

AURGH is an **[Arch User Repository](https://aur.archlinux.org/) Graphical Helper**.
<br>
Built with [GTK3](https://www.gtk.org/) and modern C++23 to deal with daily AUR shenanigans.
<br>

![Badge Workflow]
[![Badge License]][License]
![Badge Language]

<div align=left>

<br>

> [!TIP]
> The program might be unsafe/unsecure.
> As at this point I only work alone on this program, and has limited knowledge on permissions.

## Features

- [x] Basic searching of AUR packages.
- [x] Package information on each package list.
- [ ] Downloading and installing AUR packages.
- [ ] Basic AUR package management

## Dependencies

| Package Name                                                  | Uses                       |
|:--------------------------------------------------------------|:---------------------------|
| **[meson](https://mesonbuild.com/)**                          | Build system               |
| **[clang](https://clang.llvm.org/)**                          | Compiler and Linker        |
| **[curl](https://curl.se/)**                                  | Fetching data from the AUR |
| **[JsonCpp](https://github.com/open-source-parsers/jsoncpp)** | JSON handling              |
| **[gtkmm-3.0](https://gtkmm.gnome.org/en/)**                  | C++ interface for GTK      |

## License
This project is licensed under the [GNU General Public License v3](LICENSE).

[License]: LICENSE

[Badge Workflow]: https://github.com/RQuarx/aurgh/actions/workflows/check_build.yml/badge.svg
[Badge Language]: https://img.shields.io/github/languages/top/RQuarx/aurgh
[Badge License]: https://img.shields.io/github/license/RQuarx/aurgh
