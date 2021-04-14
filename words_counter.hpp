#ifndef KEEPIT_WORDS_COUNTER_HPP
#define KEEPIT_WORDS_COUNTER_HPP

#include "thread_pool.hpp"

#include <condition_variable>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>

class words_counter
{
public:
    explicit words_counter(std::string filename, long threads);
    void process_chunk(long start, long end);
    uintmax_t get_number_of_words() const;
    ~words_counter();
    void wait_for_merge();

private:
    void push_set(std::unordered_set<std::string> set);

    std::deque<std::unordered_set<std::string>> m_sets;
    std::atomic<int> m_active_chunks {};
    const std::string m_filename;
    std::condition_variable m_cv;
    uintmax_t m_number_of_words;
    thread_pool m_pool;
    std::mutex m_mtx;
};


#endif //KEEPIT_WORDS_COUNTER_HPP
