#pragma once
#include <string_view>
#include <array>


static constexpr std::string_view
    AUR_URL     { "https://aur.archlinux.org/rpc/v5" },
    PREFIX_PATH { "${HOME}/.cache/aurgh" },
    DB_PATH     { "/var/lib/pacman" },
    PKEXEC_BIN  { "/usr/bin/pkexec" },
    ROOT        { "/" };


/* A list of where the program should search for data dir */
static constexpr std::array<std::string_view, 2> DATA_SEARCH_PATHS {{
    ".",
    "/usr/share/aurgh"
}};