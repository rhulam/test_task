#include "thread_pool.hpp"

thread_pool::thread_pool(long num_of_threads) : m_enabled(true)
{
    while (num_of_threads--)
    {
        m_threads.emplace_back(std::thread(&thread_pool::thread_loop, this));
    }
}

void thread_pool::thread_loop()
{
    while (m_enabled)
    {
        std::unique_lock lock(m_mtx);

        m_cv.wait(lock, [this]{return !m_enabled || !m_tasks.empty();});
        if (!m_enabled || m_tasks.empty())
        {
            break;
        }

        auto job = m_tasks.front();
        m_tasks.pop();
        lock.unlock();
        job();
    }
}

thread_pool::~thread_pool()
{
    m_enabled = false;
    m_cv.notify_all();

    for (auto& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void thread_pool::push_task(job_type job)
{
    {
        std::scoped_lock lock(m_mtx);
        m_tasks.push(std::move(job));
    }
    m_cv.notify_all();
}