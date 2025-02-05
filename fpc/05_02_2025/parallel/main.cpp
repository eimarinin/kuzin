#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#define DEFAULT_THREADS 4

typedef long long ll;

ll computeFactorialSequential(int number)
{
    if (number < 0)
    {
        throw std::invalid_argument("Ошибка: число меньше 0");
    }
    if (number <= 1)
    {
        return 1;
    }
    ll result = 1;
    for (int i = 2; i <= number; i++)
    {
        result *= i;
    }
    return result;
}

std::mutex result_mutex;

void partialFactorial(std::vector<ll> &results, ll start, int segmentLength, int limit)
{
    if (start > limit)
    {
        return;
    }
    ll partialResult = start;
    for (int i = 1; i < segmentLength; i++)
    {
        if (start + i > limit)
        {
            break;
        }
        partialResult *= (start + i);
    }
    std::lock_guard<std::mutex> lock(result_mutex);
    results.push_back(partialResult);
}

ll computeFactorialParallel(int number, int threadCount)
{
    if (number < 0)
    {
        throw std::invalid_argument("Ошибка: число меньше 0");
    }
    if (number <= 1)
    {
        return 1;
    }

    ll result = 1;
    int segmentSize = std::ceil(1.0 * number / threadCount);
    std::vector<std::thread> threads;
    std::vector<ll> results;

    for (int i = 0; i < threadCount; i++)
    {
        threads.emplace_back(std::thread(partialFactorial, std::ref(results), segmentSize * i + 1, segmentSize, number));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    for (ll partial : results)
    {
        result *= partial;
    }

    return result;
}

int main(int argc, const char **argv)
{
    std::system("chcp 65001 > nul");

    int threadCount = DEFAULT_THREADS;
    if (argc > 1)
    {
        threadCount = std::atoi(argv[1]);
        if (threadCount <= 0)
        {
            std::cerr << "Ошибка: Некорректное количество потоков" << std::endl;
            return 1;
        }
    }

    int number;
    std::cout << "Введите число для вычисления факториала: ";
    if (!(std::cin >> number))
    {
        std::cerr << "Ошибка: Введено некорректное значение" << std::endl;
        return 1;
    }

    try
    {
        ll sequentialResult = computeFactorialSequential(number);
        ll parallelResult = computeFactorialParallel(number, threadCount);
        assert(sequentialResult == parallelResult);

        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Последовательный факториал: " << computeFactorialSequential(number) << std::endl;
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Время выполнения: " << duration.count() << "мкс" << std::endl;

        start = std::chrono::high_resolution_clock::now();
        std::cout << "Параллельный факториал: " << computeFactorialParallel(number, threadCount) << std::endl;
        stop = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Время выполнения: " << duration.count() << "мкс" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
