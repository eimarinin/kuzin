#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

#define PERF_START(id) \
	auto start_##id = std::chrono::high_resolution_clock::now();

#define PERF_END(id) \
	auto end_##id = std::chrono::high_resolution_clock::now();

#define PERF_RESULT(id) \
	std::chrono::duration_cast<std::chrono::microseconds>(end_##id - start_##id).count()

const int NUM_THREADS = 5;
const int INCREMENTS_PER_THREAD = 1000;

std::mutex mtx;			// Мьютекс для защиты общего ресурса
int shared_counter = 0; // Общий ресурс
int counter = 0;
std::mutex lock;

void increment_counter(int id)
{
	for (int i = 0; i < INCREMENTS_PER_THREAD; ++i)
	{
		std::lock_guard<std::mutex> lock(mtx); // Блокировка мьютекса
		++shared_counter;
	}
	std::cout << "Thread " << id << " finished.\n";
}

void increase(bool should_lock)
{
	for (int i = 0; i < 10000; i++)
	{
		if (should_lock)
		{
			lock.lock();
		}
		counter++;
		if (should_lock)
		{
			lock.unlock();
		}
	}
}

void decrease(bool should_lock)
{
	for (int i = 0; i < 10000; i++)
	{
		if (should_lock)
		{
			lock.lock();
		}
		counter--;
		if (should_lock)
		{
			lock.unlock();
		}
	}
}

int main()
{
	std::vector<std::thread> threads;

	// Создание и запуск потоков
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		threads.emplace_back(increment_counter, i);
	}

	// Ожидание завершения всех потоков
	for (auto &th : threads)
	{
		th.join();
	}

	std::cout << "Final counter value: " << shared_counter << "\n";

	std::thread inc(increase, false);
	std::thread dec(decrease, false);

	inc.join();
	dec.join();

	std::cout << "NO LOCK: " << counter << std::endl;

	counter = 0;

	std::thread inc_lock(increase, true);
	std::thread dec_lock(decrease, true);

	inc_lock.join();
	dec_lock.join();

	std::cout << "LOCK: " << counter << std::endl;

	return 0;
}
