#pragma once
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <memory>
#include <thread>

#include <glibmm/dispatcher.h>
#include <sigc++/signal.h>

#include "alpm/handle.hh"
#include "result.hh"


namespace aurgh::alpm
{
    class async
    {
    public:
        template <typename T>
        class request
        {
            friend class async;
            using error_signal  = sigc::signal<void(error)>;
            using result_signal = sigc::signal<void(T)>;

        public:
            auto
            on_result(const typename result_signal::slot_type &slot) -> request &
            {
                m_signal_on_result.connect(slot);
                return *this;
            }


            auto
            on_error(const typename error_signal::slot_type &slot) -> request &
            {
                m_signal_on_error.connect(slot);
                return *this;
            }


            void
            cancel()
            { m_cancelled = true; }

        private:
            error_signal  m_signal_on_error;
            result_signal m_signal_on_result;
            bool          m_cancelled = false;


            void
            complete(result<T> res)
            {
                if (m_cancelled) return;

                if (res.has_value())
                    m_signal_on_result.emit(std::move(res.value()));
                else
                    m_signal_on_error.emit(res.error());
            }
        };


        explicit async(handle &&h);
        ~async();

        [[nodiscard]]
        auto search(std::string query) noexcept
            -> result<std::shared_ptr<request<std::vector<package>>>>;


        [[nodiscard]]
        auto info(const std::vector<std::string> &args) noexcept
            -> result<std::shared_ptr<request<std::vector<package_details>>>>;

    private:
        handle m_handle;

        std::thread                                 m_thread;
        std::mutex                                  m_mutex;
        std::condition_variable                     m_cv;
        std::deque<std::move_only_function<void()>> m_queue;
        bool                                        m_stopping = false;


        void run();


        template <typename T>
        [[nodiscard]]
        static auto
        make_request() -> std::shared_ptr<request<T>>
        { return std::shared_ptr<request<T>> { new request<T> {} }; }
    };
}
