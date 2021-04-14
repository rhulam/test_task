#include "words_counter.hpp"

#include <iostream>
#include <fstream>
#include <future>

namespace
{
    std::streampos get_file_length(const std::string_view& file)
    {
        std::ifstream ifs(file.data());

        if (!ifs)
        {
            return -1;
        }

        ifs.seekg (0, ifs.end);

        if (!ifs)
        {
            return -1;
        }

        auto length = ifs.tellg();

        return length;
    }

    long get_threads_num(intmax_t length)
    {
        constexpr unsigned buff_size = 8192;
        long hardware_threads = std::thread::hardware_concurrency();

        return std::min(std::max(intmax_t(1) , length / buff_size), hardware_threads);
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Expected file name as parameter" << std::endl;
        return EXIT_FAILURE;
    }

    auto length = get_file_length(argv[1]);

    if (length < 1)
    {
        std::cerr << "Can't get file length or file is empty" << std::endl;
        return EXIT_FAILURE;
    }

    auto threads_num = get_threads_num(length);

    if (threads_num < 1)
    {
        std::cerr << "Division by zero" << std::endl;
    }

    auto chunk_size = length / threads_num;

    words_counter worker(argv[1], threads_num);
    long start;
    long end;

    for (int i = 0; i < threads_num; ++i)
    {
        start = i * chunk_size;

        if (i + 1 == threads_num)
        {
            end = length;
        }
        else
        {
            end = (i + 1) * chunk_size;
        }

        worker.process_chunk(start, end);
    }

    worker.wait_for_merge();
    auto result = worker.get_number_of_words();

    if (result < 1)
    {
        return EXIT_FAILURE;
    }

    std::cout << result << std::endl;
    return EXIT_SUCCESS;
}
