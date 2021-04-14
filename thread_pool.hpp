#ifndef KEEPIT_THREAD_POOL_HPP
#define KEEPIT_THREAD_POOL_HPP

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>

class thread_pool
{
    using job_type = std::function<void()>;

public:
    explicit thread_pool(long num_of_threads);
    ~thread_pool();

    void push_task(job_type job);

private:
    void thread_loop();

    std::vector<std::thread> m_threads;
    std::condition_variable m_cv;
    std::queue<job_type> m_tasks;
    std::atomic<bool> m_enabled;
    std::mutex m_mtx;
};


#endif //KEEPIT_THREAD_POOL_HPP
