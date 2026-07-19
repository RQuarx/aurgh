#pragma once
#include <filesystem>

#include <glibmm/ustring.h>
#include <sigc++/signal.h>

#include "alpm/async.hh"
#include "aur.hh"
#include "git.hh"
#include "result.hh"


namespace aurgh
{
    class client
    {
    public:
        class clone_process
        {
            friend class client;

        public:
            clone_process(std::shared_ptr<git::cloning> &&cloning);

            clone_process(clone_process &&other) noexcept;
            auto operator=(clone_process &&other) noexcept -> clone_process &;

            clone_process(const clone_process &)  = delete;
            auto operator=(const clone_process &) = delete;


            [[nodiscard]]
            auto signal_on_clone_complete() const
                -> sigc::signal<void(result<std::filesystem::path>)>;

            [[nodiscard]]
            auto signal_on_clone_progress() const -> sigc::signal<void(double)>;

        private:
            Glib::Dispatcher m_complete_dispatcher;
            Glib::Dispatcher m_progress_dispatcher;

            sigc::signal<void(result<std::filesystem::path>)> m_signal_on_complete;
            sigc::signal<void(double)>                        m_signal_on_progress;

            result<std::filesystem::path> m_dst;
            std::atomic<double>           m_progress;

            std::shared_ptr<git::cloning> m_cloning;
        };


        [[nodiscard]]
        static auto create(const std::shared_ptr<http::client> &http,
                           std::filesystem::path                clone_dir,
                           const std::filesystem::path &pacman_conf = "/etc/pacman.conf") noexcept
            -> result<std::unique_ptr<client>>;


        auto search(const std::string &query) noexcept -> result<void>;
        auto info(const std::vector<std::string> &args) noexcept -> result<void>;
        auto clone(std::string_view url) noexcept -> result<std::reference_wrapper<clone_process>>;


        [[nodiscard]]
        auto signal_on_search_complete() const -> sigc::signal<void(result<std::vector<package>>)>;

        [[nodiscard]]
        auto signal_on_info_complete() const
            -> sigc::signal<void(result<std::vector<package_details>>)>;

    private:
        template <typename T>
        struct operation
        {
            template <typename U, typename Arg, typename Request>
            using method = auto (U::*)(Arg) noexcept -> result<Request>;

            friend class client;

            std::mutex mutex;

            result<T>                     results;
            Glib::Dispatcher              dispatcher;
            sigc::signal<void(result<T>)> signal;
            std::atomic<std::uint8_t>     counter;

            std::shared_ptr<aur::request<T>>         aur_request;
            std::shared_ptr<alpm::async::request<T>> alpm_request;


            operation()
            {
                dispatcher.connect([this] { signal.emit(results); });
            }


            template <typename U, typename V>
            auto
            perform(auto                                           val,
                    aur                                           &aur,
                    alpm::async                                   &alpm,
                    method<aurgh::aur, U, decltype(aur_request)>   aur_method,
                    method<alpm::async, V, decltype(alpm_request)> alpm_method) noexcept
                -> result<void>
            {
                if (aur_request != nullptr)
                    if (auto res = aur_request->cancel(); !res) return res.error().unexpected();
                if (alpm_request != nullptr) alpm_request->cancel();

                if (auto res = (aur.*aur_method)(val); res.has_value())
                {
                    aur_request = std::move(res.value());
                    attach_handler(aur_request, results, counter, dispatcher);
                }
                else
                    return res.error().unexpected();

                if (auto res = (alpm.*alpm_method)(val); res.has_value())
                {
                    alpm_request = std::move(res.value());
                    attach_handler(alpm_request, results, counter, dispatcher);
                }
                else
                    return res.error().unexpected();
                return {};
            }


            void
            attach_handler(auto &req, auto &results, auto &counter, auto &dispatcher)
            {
                req->on_result(
                       [this, &results, &counter, &dispatcher](auto &&res)
                       {
                           std::lock_guard lock { mutex };
                           if (!results) [[unlikely]]
                               return; /* dispatcher already emitted from the other thread */

                           results->insert(results->end(), std::make_move_iterator(res.begin()),
                                           std::make_move_iterator(res.end()));
                           if (++counter == 2) dispatcher.emit();
                       })
                    .on_error(
                        [this, &results, &dispatcher](error err)
                        {
                            std::lock_guard lock { mutex };
                            results = err.unexpected();
                            dispatcher.emit();
                        });
            }
        };


        std::shared_ptr<http::client> m_client;
        std::filesystem::path         m_clone_dir;

        aur         m_aur;
        alpm::async m_alpm;

        std::mutex                 m_clone_mutex;
        std::vector<clone_process> m_clones;

        operation<std::vector<package>>         m_search_operation;
        operation<std::vector<package_details>> m_info_operation;


        client(const std::shared_ptr<http::client> &http,
               std::filesystem::path              &&clone_dir,
               alpm::handle                       &&handle);
    };

}
