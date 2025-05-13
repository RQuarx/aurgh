<div align=center>

# AURGH

AURGH is an **Arch User Repository Graphical Helper**,
<br>
made with C++23 to deal with daily AUR shenanigans.
<br>

![Badge gtk3]
![Badge gtk4]
[![Badge License]][License]
![Badge Language]

<img src=https://github.com/user-attachments/assets/4f2be868-7e7b-4f2d-b373-cfbb484011a5 width=700>

</div>

## Features

- [x] Basic searching of AUR packages.
- [x] Package information on each package list.
- [x] Cross compatible with GTK3 and GTK4.
- [ ] Downloading and installing AUR packages.
- [ ] Basic AUR package management

## Dependencies

| Package Name                                                  | Uses                       |
|:--------------------------------------------------------------|:---------------------------|
| **[meson](https://mesonbuild.com/)**                          | Build system               |
| **[curl](https://curl.se/)**                                  | Fetching data from the AUR |
| **[jsoncpp](https://github.com/open-source-parsers/jsoncpp)** | JSON handling              |
| **[gtkmm-3/4](https://gtkmm.gnome.org/en/)**                  | C++ interface for GTK      |

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

# For gtk3 builds use
user$ meson setup -Dbuildtype=debugoptimized target
# and for gtk4 use
user$ meson setup -Dbuildtype=debugoptimized -Duse-gtk4=true target

user$ meson compile -C target
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
