#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Параметры программы
	const int N = 1'000'000; // Размер массива
	const int seed = 42;	 // Seed для генератора случайных чисел

	std::vector<int> data;
	if (rank == 0)
	{
		data.resize(N);
		std::mt19937 gen(seed);
		std::uniform_int_distribution<> dist(0, 99);
		for (int &val : data)
		{
			val = dist(gen);
		}
	}

	// Вычисление размера части для каждого процесса
	const int localSize = N / size;
	std::vector<int> localData(localSize);

	// Последовательное суммирование (только на процессе 0)
	double seqTime = 0.0;
	int seqSum = 0;
	if (rank == 0)
	{
		auto start = std::chrono::high_resolution_clock::now();
		for (int val : data)
			seqSum += val;
		seqTime = std::chrono::duration<double>(
					  std::chrono::high_resolution_clock::now() - start)
					  .count();
	}

	// Параллельное суммирование
	auto parallelStart = std::chrono::high_resolution_clock::now();

	// Распределение данных
	MPI_Scatter(data.data(), localSize, MPI_INT,
				localData.data(), localSize, MPI_INT,
				0, MPI_COMM_WORLD);

	// Локальное суммирование
	int localSum = 0;
	for (int val : localData)
		localSum += val;

	// Сбор результатов
	int parallelSum = 0;
	MPI_Reduce(&localSum, &parallelSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	double parallelTime = std::chrono::duration<double>(
							  std::chrono::high_resolution_clock::now() - parallelStart)
							  .count();

	// Вывод результатов (только процесс 0)
	if (rank == 0)
	{
		std::cout << "Array size: " << N << "\n";
		std::cout << "Processes: " << size << "\n";
		std::cout << "Sequential sum: " << seqSum << " (Time: " << seqTime << " sec)\n";
		std::cout << "Parallel sum:   " << parallelSum << " (Time: " << parallelTime << " sec)\n";
		std::cout << "Speedup: " << seqTime / parallelTime << "\n";
	}

	MPI_Finalize();
	return 0;
}