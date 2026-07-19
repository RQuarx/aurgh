#pragma once
#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>

#include <git2/repository.h>
#include <glibmm/dispatcher.h>
#include <sigc++/signal.h>

#include "result.hh"


namespace aurgh::git
{
    class cloning
    {
        using transfer_progress_signal = sigc::signal<void(double)>;
        using completed_signal         = sigc::signal<void(std::filesystem::path)>;
        using error_signal             = sigc::signal<void(error)>;

    public:
        cloning(std::string_view url, std::filesystem::path base);


        auto
        on_transfer_progress(const transfer_progress_signal::slot_type &slot) -> cloning &
        {
            m_signal_on_transfer_progress.connect(slot);
            return *this;
        }


        auto
        on_completed(const completed_signal::slot_type &slot) -> cloning &
        {
            m_signal_on_completed.connect(slot);
            return *this;
        }


        auto
        on_error(const error_signal::slot_type &slot) -> cloning &
        {
            m_signal_on_error.connect(slot);
            return *this;
        }


        void cancel();

    private:
        std::string           m_url;
        std::filesystem::path m_base;
        std::filesystem::path m_dst;

        std::jthread    m_thread;
        std::stop_token m_stop_token;

        mutable std::mutex m_mutex;
        double             m_latest_progress;

        std::optional<error> m_pending_error;

        Glib::Dispatcher m_dispatch_progress;
        Glib::Dispatcher m_dispatch_done;

        transfer_progress_signal m_signal_on_transfer_progress;
        completed_signal         m_signal_on_completed;
        error_signal             m_signal_on_error;


        void mf_run(std::stop_token token);
        void mf_on_progress() const;
        void mf_on_done() const;

        static auto transfer_progress_callback(const git_indexer_progress *stats, void *payload)
            -> int;
    };


    [[nodiscard]]
    auto clone(std::string_view url, std::filesystem::path base) noexcept
        -> result<std::shared_ptr<cloning>>;
}
