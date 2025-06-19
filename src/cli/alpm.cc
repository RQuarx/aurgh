#include <cstring>

#include "logger.hh"
#include "utils.hh"
#include "alpm.hh"
#include "data.hh"


Alpm::Alpm(
    const std::string          &root_path,
    const std::string          &db_path,
    std::string                 prefix,
    alpm_errno_t               &err_msg
) :
    m_prefix(std::move(prefix)),
    m_err(err_msg),
    m_handle(alpm_initialize(root_path.c_str(), db_path.c_str(), &m_err)),
    m_removal_flags(ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE)
{}


Alpm::~Alpm()
{ alpm_release(m_handle); }

 
auto
Alpm::remove_packages(const std::vector<std::string> &pkgs) -> bool
{
    alpm_list_t *data {};

    if (alpm_trans_init(m_handle, m_removal_flags) < 0) {
        data::logger->log(
            Logger::Error,
            "alpm_trans_init failed when removing packages: {}",
            alpm_strerror(m_err)
        );
        return false;
    }

    alpm_db_t *local_db = alpm_get_localdb(m_handle);

    for (const auto &pkg_name : pkgs) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(local_db, pkg_name.c_str());

        if (pkg == nullptr) {
            data::logger->log(
                Logger::Error,
                "alpm_db_get_pkg failed when removing packages: {}",
                alpm_strerror(m_err)
            );
            return false;
        }
        if (alpm_remove_pkg(m_handle, pkg) < 0) {
            data::logger->log(
                Logger::Error,
                "alpm_remove_pkg failed: {}",
                alpm_strerror(m_err)
            );
            return false;
        }
    }

    if (alpm_trans_prepare(m_handle, &data) < 0) {
        data::logger->log(
            Logger::Error,
            "alpm_trans_prepare failed when removing packages: {}",
            alpm_strerror(m_err)
        );

        if (data != nullptr) alpm_list_free(data);
        return false;
    }

    if (alpm_trans_commit(m_handle, &data) < 0) {
        data::logger->log(
            Logger::Error,
            "alpm_trans_commit failed when removing packages: {}",
            alpm_strerror(m_err)
        );
        if (data != nullptr) alpm_list_free(data);
        return false;
    }

    if (data != nullptr) alpm_list_free(data);
    if (alpm_trans_release(m_handle) == 0) {
        return true;
    }
    data::logger->log(
        Logger::Error,
        "alpm_trans_release failed when removing packages: {}",
        alpm_strerror(m_err)
    );

    return false;
}


auto
Alpm::download_and_install_package(
    const std::string &pkg
) -> bool
{
    if (chdir(m_prefix.c_str()) != 0) {
        data::logger->log(
            Logger::Error,
            "Failed to change directory: {}",
            strerror(errno)
        );
        return false;
    }

    std::string clone_cmd {
        std::format("git clone https://aur.archlinux.org/{}.git", pkg)
    };
    auto res = utils::run_command(clone_cmd, 1024);

    if (res->second != 0) {
        data::logger->log(
            Logger::Error,
            "Failed to run {}: {}",
            clone_cmd, res->first
        );
        m_str_err = res->first;
        return false;
    }

    if (chdir(std::format("{}/{}", m_prefix, pkg).c_str()) != 0) {
        data::logger->log(
            Logger::Error,
            "Failed to change directory: {}",
            strerror(errno)
        );
        return false;
    }

    res = utils::run_command("makepkg", 1024);

    if (res->second == 0) { return true; }

    data::logger->log(
        Logger::Error,
        "Failed to run makepkg: {}",
        res->first
    );
    return false;
}


auto
Alpm::get_str_err() const -> std::string
{ return m_str_err; }


void
Alpm::set_removal_flags(int32_t flag)
{ m_removal_flags = flag; }