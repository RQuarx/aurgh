#include "alpm/async.hh"

using aurgh::alpm::async;


async::async(handle &&h) : m_handle { std::move(h) }
{ m_thread = std::thread { &async::run, this }; }


async::~async()
{
    if (m_thread.joinable())
    {
        {
            std::lock_guard lock { m_mutex };
            m_stopping = true;
        }

        m_cv.notify_all();
        m_thread.join();
    }
}


void
async::run()
{
    while (true)
    {
        std::move_only_function<void()> job;

        {
            std::unique_lock lock { m_mutex };

            m_cv.wait(lock, [this] { return m_stopping or !m_queue.empty(); });

            if (m_stopping and m_queue.empty()) break;

            job = std::move(m_queue.front());
            m_queue.pop_front();
        }

        job();
    }
}


auto
async::search(std::string query) noexcept -> result<std::shared_ptr<request<std::vector<package>>>>
{
    auto req = make_request<std::vector<package>>();

    {
        std::lock_guard lock { m_mutex };

        m_queue.emplace_back([this, req, name = std::move(query)] mutable
                             { req->complete(m_handle.search(name)); });
    }

    m_cv.notify_one();
    return req;
}


auto
async::info(const std::vector<std::string> &args) noexcept
    -> result<std::shared_ptr<request<std::vector<package_details>>>>
{
    auto req = make_request<std::vector<package_details>>();

    {
        std::lock_guard lock { m_mutex };

        m_queue.emplace_back([this, req, args] mutable { req->complete(m_handle.info(args)); });
    }

    m_cv.notify_one();
    return req;
}
