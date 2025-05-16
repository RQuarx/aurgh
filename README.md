<div align=center>

# AURGH

AURGH is an **Arch User Repository Graphical Helper**,
<br/>
made with C++23 to deal with daily AUR shenanigans.
<br/>

![Badge gtk3]
![Badge gtk4]
[![Badge License]][License]
![Badge Language]

<img src=https://github.com/user-attachments/assets/c9ad8212-6e43-493f-94d9-ddb37c222c5c width=700></img>

</div>

## Features

>[!IMPORTANT]
>AURGH is not a pacman frontend, nor is it a frontend of any other helpers.
>AURGH tries to be independent from any other available frontend.

- [x] Basic searching of AUR packages.
- [x] Package information on each package list.
- [x] Cross compatible with GTK3 and GTK4.
- [ ] Downloading and installing AUR packages.
- [ ] Basic AUR package management

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

```bash
user$ git clone https://github.com/RQuarx/aurgh/
user$ cd aurgh

# This will defaults to clang++, lld, and GTK4.
user$ make CXX=... CXX_LD=... GTK_VERSION=...

root# make install
```

## Donating

If you like this project, you could support it by making my parents think that this project is not useless.
<br>
<br>
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/I2I11ERX5G)

## License
This project is licensed under the [GNU General Public License v3](LICENSE).

[License]: LICENSE

[Badge Workflow]: https://github.com/RQuarx/aurgh/actions/workflows/check_build.yml/badge.svg
[Badge gtk3]: https://github.com/RQuarx/aurgh/actions/workflows/gtk3_build.yml/badge.svg
[Badge gtk4]: https://github.com/RQuarx/aurgh/actions/workflows/gtk4_build.yml/badge.svg
[Badge Language]: https://img.shields.io/github/languages/top/RQuarx/aurgh
[Badge License]: https://img.shields.io/github/license/RQuarx/aurgh
