#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

constexpr int THREADS = 4;
constexpr int N = 10000;

#define PERF_START(id) auto start_##id = std::chrono::high_resolution_clock::now();
#define PERF_END(id) auto end_##id = std::chrono::high_resolution_clock::now();
#define PERF_RESULT(id) std::chrono::duration_cast<std::chrono::milliseconds>(end_##id - start_##id).count()

using matrix = std::vector<std::vector<int>>;
using vec = std::vector<int>;

// Функция генерации случайных данных
matrix generate_matrix(int rows, int cols)
{
	matrix result(rows, std::vector<int>(cols));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(-100, 100);

	for (auto &row : result)
		for (auto &elem : row)
			elem = dist(gen);

	return result;
}

vec generate_vector(int size)
{
	vec result(size);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(-100, 100);

	for (auto &elem : result)
		elem = dist(gen);

	return result;
}

// Последовательное умножение матрицы на вектор
vec multiply(const matrix &A, const vec &x)
{
	int rows = A.size();
	int cols = A[0].size();
	vec result(rows, 0);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			result[i] += A[i][j] * x[j];

	return result;
}

// Параллельное умножение матрицы на вектор
vec multiply_parallel(const matrix &A, const vec &x)
{
	int rows = A.size();
	int cols = A[0].size();
	vec result(rows, 0);

#pragma omp parallel for num_threads(THREADS)
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			result[i] += A[i][j] * x[j];

	return result;
}

// Сравнение двух векторов
bool compare(const vec &a, const vec &b)
{
	return a == b;
}

int main()
{
	omp_set_num_threads(THREADS);
	matrix A = generate_matrix(N, N);
	vec x = generate_vector(N);

	std::cout << "Data generated\n";

	PERF_START(serial)
	vec serial_result = multiply(A, x);
	PERF_END(serial)

	PERF_START(parallel)
	vec parallel_result = multiply_parallel(A, x);
	PERF_END(parallel)

	if (!compare(serial_result, parallel_result))
	{
		std::cerr << "Results do not match!" << std::endl;
		return 1;
	}

	std::cout << "SERIAL: " << PERF_RESULT(serial) << " ms" << std::endl;
	std::cout << "PARALLEL: " << PERF_RESULT(parallel) << " ms" << std::endl;

	return 0;
}
