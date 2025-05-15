#include <print>

#include "alpm.hh"


Alpm::Alpm(
    const std::string &root_path,
    const std::string &db_path,
    alpm_errno_t      &err_msg
) :
    m_err(err_msg),
    m_handle(alpm_initialize(root_path.c_str(), db_path.c_str(), &m_err)),
    m_removal_flags(ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE)
{}


Alpm::~Alpm()
{ alpm_release(m_handle); }


auto
Alpm::remove_packages(const std::vector<std::string> &pkgs) -> bool
{
    alpm_list_t *data{};

    if (alpm_trans_init(m_handle, m_removal_flags) < 0) {
        return false;
    }

    alpm_db_t *local_db = alpm_get_localdb(m_handle);

    for (const auto &pkg_name : pkgs) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(local_db, pkg_name.c_str());

        if (pkg == nullptr) return false;
        if (alpm_remove_pkg(m_handle, pkg) < 0) return false;
    }

    if (alpm_trans_prepare(m_handle, &data) < 0) {
        if (data != nullptr) alpm_list_free(data);
        return false;
    }

    if (alpm_trans_commit(m_handle, &data) < 0) {
        if (data != nullptr) alpm_list_free(data);
        return false;
    }

    if (data != nullptr) alpm_list_free(data);
    if (alpm_trans_release(m_handle) < 0) return false;


    return true;
}


void
Alpm::set_removal_flags(int32_t flag)
{ m_removal_flags = flag; }