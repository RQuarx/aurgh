<div align=center>

# AURGH

AURGH is an **Arch User Repository[^1] Graphical Helper**.
<br>
Built with GTK3[^2] and modern C++23 to deal with daily AUR shenanigans.
<br>

![Badge Workflow]
[![Badge License]][License]
![Badge Language]

</div>

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
| **[jsoncpp](https://github.com/open-source-parsers/jsoncpp)** | JSON handling              |
| **[gtkmm-3.0](https://gtkmm.gnome.org/en/)**                  | C++ interface for GTK      |

## Installation

>[!NOTE]
>Everything done here would require the user to clone the repo first.

<details>
<summary>Installing dependencies</summary>

```bash
root$ pacman -S - < required.txt
```

or

```bash
root$ make install-deps
```

</details>

### Installing the package

```bash
user$ make setup-release && make compile
```

```bash
root$ cp target/aurgh /usr/bin
```

## Donating

If you like this project, you could support it by donating to me.
<br>
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/I2I11ERX5G)

## License
This project is licensed under the [GNU General Public License v3](LICENSE)[^3].

[^1]: https://aur.archlinux.org/
[^2]: https://www.gtk.org/
[^3]: https://www.gnu.org/licenses/gpl-3.0.html

[License]: LICENSE

[Badge Workflow]: https://github.com/RQuarx/aurgh/actions/workflows/check_build.yml/badge.svg
[Badge Language]: https://img.shields.io/github/languages/top/RQuarx/aurgh
[Badge License]: https://img.shields.io/github/license/RQuarx/aurgh
