#include "words_counter.hpp"

#include <fstream>

words_counter::words_counter(std::string filename, const long threads)
    : m_filename(std::move(filename))
    , m_pool(threads)
    , m_number_of_words(0)
{
}

void words_counter::process_chunk(long start, long end)
{
    ++m_active_chunks;
    auto job = [this, start, end]
    {
        std::ifstream ifs(m_filename);

        if (!ifs)
        {
            --m_active_chunks;
            m_cv.notify_one();
            return;
        }

        ifs.seekg(start, std::ios::beg);

        std::string word;
        std::unordered_set<std::string> set;

        if (start != 0)
        {
            while (ifs.get() != ' ')
            {
                if (!ifs)
                {
                    break;
                }
            };
        }

        while (ifs >> word)
        {
            set.insert(std::move(word));

            if (ifs.tellg() == end && ifs.get() == ' ')
            {
                ifs.putback(' ');
                break;
            }

            if (ifs.tellg() > end)
            {
                break;
            }
        }

        if (ifs.bad())
        {
            --m_active_chunks;
            m_cv.notify_one();
            return;
        }

        push_set(std::move(set));
        --m_active_chunks;
    };

    m_pool.push_task(job);
}

void words_counter::push_set(std::unordered_set<std::string> set)
{
    {
        std::scoped_lock lock(m_mtx);
        m_sets.push_back(std::move(set));
    }

    auto job = [this]
    {
        {
            std::scoped_lock lock(m_mtx);
            if (m_sets.size() >= 2)
            {
                m_sets[1].merge(m_sets.front());
                m_sets.pop_front();
            }

            m_cv.notify_one();
        }
    };

    m_pool.push_task(job);
}

uintmax_t words_counter::get_number_of_words() const
{
    return m_number_of_words;
}

words_counter::~words_counter()
{
}

void words_counter::wait_for_merge()
{
    {
        std::unique_lock lock(m_mtx);

        if (m_active_chunks || m_sets.size() >= 2)
        {
            m_cv.wait(lock, [this]{return !m_active_chunks && m_sets.size() < 2;});
        }
    }

    if (m_sets.size() != 1)
    {
        m_number_of_words = -1;
    }
    else
    {
        m_number_of_words = m_sets.front().size();
    }

    m_sets.clear();
}