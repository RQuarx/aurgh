#include <cstring>

#include "logger.hh"
#include "utils.hh"
#include "alpm.hh"
#include "data.hh"

#define Err Logger::Error


Alpm::Alpm(const str    &p_root_path,
           const str    &p_db_path,
           str           p_prefix,
           alpm_errno_t &p_err_msg) :
    m_prefix(std::move(p_prefix)),
    m_err(p_err_msg),
    m_handle(alpm_initialize(p_root_path.c_str(), p_db_path.c_str(), &m_err)),
    m_removal_flags(ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE)
{}


Alpm::~Alpm()
{ alpm_release(m_handle); }


auto
Alpm::remove_packages(const vec<str> &p_pkgs) -> bool
{
    /* Step 0: Create a new transaction. */
    data::logger->log(Logger::Debug, "Creating transaction.");
    if (alpm_trans_init(m_handle, m_removal_flags) == -1) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to init transaction: {}",
                          alpm_strerror(m_err));
        return false;
    }

    /* Step 1: Add targets to the created transaction. */
    data::logger->log(Logger::Debug, "Adding targets.");
    for (const str &pkg : p_pkgs) {
        if (!remove_package(pkg.c_str())) [[unlikely]] {
            if (alpm_trans_release(m_handle) == -1) [[unlikely]] {
                data::logger->log(Err,
                                  "Failed to release transaction: {}",
                                  alpm_strerror(alpm_errno(m_handle)));
            }
            return false;
        }
    }

    /* Step 2: Prepare for the removal. */
    alpm_list_t *data = nullptr;
    if (alpm_trans_prepare(m_handle, &data) == -1) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to commit transactions: {}",
                          alpm_strerror(alpm_errno(m_handle)));

        if (m_err == ALPM_ERR_UNSATISFIED_DEPS) {
            for (alpm_list_t *i = data; i != nullptr; i = alpm_list_next(data)) {
                auto *miss  = static_cast<alpm_depmissing_t *>(i->data);
                str dep_str = alpm_dep_compute_string(miss->depend);
                data::logger->log(Logger::Error,
                                  "Removing {} breaks dependency "
                                  "'{}' required by {}.",
                                  miss->causingpkg, dep_str, miss->target);
                alpm_depmissing_free(miss);
            }
        }

        if (alpm_trans_release(m_handle) == -1) [[unlikely]] {
            data::logger->log(Err,
                              "Failed to release transaction: {}",
                              alpm_strerror(alpm_errno(m_handle)));
        }

        alpm_list_free(data);
        return false;
    }
    alpm_list_free(data);

    /* Step 3: Perform the removal. */
    data::logger->log(Logger::Debug, "Removing packages.");
    if (alpm_trans_commit(m_handle, nullptr) == -1) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to release transaction: {}",
                          alpm_strerror(alpm_errno(m_handle)));
        return false;
    }

    if (alpm_trans_release(m_handle) == -1) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to release transaction: {}",
                          alpm_strerror(alpm_errno(m_handle)));
        return false;
    }

    data::logger->log(Logger::Debug,
                      "Successfully removed {} packages.",
                      p_pkgs.size());
    return true;
}


auto
Alpm::remove_package(const char *p_target) -> bool
{
    alpm_db_t  *local_db = alpm_get_localdb(m_handle);
    alpm_pkg_t *pkg      = alpm_db_get_pkg(local_db, p_target);

    if (pkg == nullptr) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to get package from local-db: {}",
                          alpm_strerror(m_err));
        return false;
    }

    if (alpm_remove_pkg(m_handle, pkg) == -1) [[unlikely]] {
        data::logger->log(Err,
                          "Failed to remove package {} from local-db: {}",
                          p_target, alpm_strerror(m_err));
        return false;
    }
    return true;
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