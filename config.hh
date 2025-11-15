#pragma once
#include <string_view>

#define GLOBAL(p_name, ...) \
    static constexpr std::string_view p_name { __VA_ARGS__ }

/* The URL pointing to the AUR RPC */
GLOBAL(AUR_URL, "https://aur.archlinux.org/rpc/v5");

/* The socket path the AURGH-helper will use */
GLOBAL(SOCK_PATH, "/run/user/${UID}/aur_graphical_helper.sock");

/* The prefix/cache path of AURGH */
GLOBAL(PREFIX_PATH, "${HOME}/.cache/aurgh");

/* Path to the system's pacman database */
GLOBAL(DB_PATH, "/var/lib/pacman");

/* Path pointing to the pkexec binary */
GLOBAL(PKEXEC_BIN, "/usr/bin/pkexec");

/* Path pointing to the filesystem root */
GLOBAL(ROOT, "/");

#undef GLOBAL
