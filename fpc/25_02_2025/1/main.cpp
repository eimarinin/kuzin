#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>
#include <cassert>
#include <omp.h>

constexpr size_t THREADS = 4;
constexpr size_t N = 10'000'000;

std::vector<int> generate(size_t n)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(-100, 100);

	std::vector<int> result(n);
	for (size_t i = 0; i < n; i++)
	{
		result[i] = dist(gen);
	}
	return result;
}

long sum(const std::vector<int> &array)
{
	return std::accumulate(array.begin(), array.end(), 0L);
}

long sum_parallel(const std::vector<int> &array)
{
	long r = 0;
#pragma omp parallel for reduction(+ : r)
	for (size_t i = 0; i < array.size(); i++)
	{
		r += array[i];
	}
	return r;
}

int main()
{
	omp_set_num_threads(THREADS);

	std::vector<int> vec = generate(N);

	auto start_serial = std::chrono::high_resolution_clock::now();
	long r1 = sum(vec);
	auto end_serial = std::chrono::high_resolution_clock::now();

	auto start_parallel = std::chrono::high_resolution_clock::now();
	long r2 = sum_parallel(vec);
	auto end_parallel = std::chrono::high_resolution_clock::now();

	assert(r1 == r2);

	std::cout << "SUM: " << r1 << std::endl;
	std::cout << "SERIAL: "
			  << std::chrono::duration_cast<std::chrono::milliseconds>(end_serial - start_serial).count()
			  << "ms" << std::endl;
	std::cout << "PARALLEL: "
			  << std::chrono::duration_cast<std::chrono::milliseconds>(end_parallel - start_parallel).count()
			  << "ms" << std::endl;

	return 0;
}
