#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>

constexpr int MATRIX_SIZE = 500;   // Размер матрицы
constexpr int PRINT_ELEMENTS = 5;  // Количество выводимых элементов
constexpr int MIN_RAND_VALUE = 1;  // Минимальное случайное значение
constexpr int MAX_RAND_VALUE = 10; // Максимальное случайное значение

void print_matrix(const std::vector<int> &matrix, int rows, int cols)
{
	for (int i = 0; i < std::min(rows, PRINT_ELEMENTS); i++)
	{
		for (int j = 0; j < std::min(cols, PRINT_ELEMENTS); j++)
		{
			std::cout << matrix[i * cols + j] << "\t";
		}
		std::cout << (cols > PRINT_ELEMENTS ? "..." : "") << "\n";
	}
	if (rows > PRINT_ELEMENTS)
		std::cout << "...\n";
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (MATRIX_SIZE % size != 0 && rank == 0)
	{
		std::cerr << "Error: Matrix size must be divisible by number of processes\n";
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	const int local_rows = MATRIX_SIZE / size;
	std::vector<int> A, B(MATRIX_SIZE * MATRIX_SIZE), C;
	std::vector<int> local_A(local_rows * MATRIX_SIZE), local_C(local_rows * MATRIX_SIZE, 0);

	if (rank == 0)
	{
		A.resize(MATRIX_SIZE * MATRIX_SIZE);
		C.resize(MATRIX_SIZE * MATRIX_SIZE);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(MIN_RAND_VALUE, MAX_RAND_VALUE);

		std::generate(A.begin(), A.end(), [&]()
					  { return dist(gen); });
		std::generate(B.begin(), B.end(), [&]()
					  { return dist(gen); });

		if (MATRIX_SIZE <= 20)
		{
			std::cout << "Matrix A:\n";
			print_matrix(A, MATRIX_SIZE, MATRIX_SIZE);
			std::cout << "\nMatrix B:\n";
			print_matrix(B, MATRIX_SIZE, MATRIX_SIZE);
		}
	}

	auto start_time = std::chrono::high_resolution_clock::now();

	MPI_Scatter(A.data(), local_rows * MATRIX_SIZE, MPI_INT,
				local_A.data(), local_rows * MATRIX_SIZE, MPI_INT,
				0, MPI_COMM_WORLD);
	MPI_Bcast(B.data(), MATRIX_SIZE * MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < local_rows; i++)
	{
		for (int k = 0; k < MATRIX_SIZE; k++)
		{
			int temp = local_A[i * MATRIX_SIZE + k];
			for (int j = 0; j < MATRIX_SIZE; j++)
			{
				local_C[i * MATRIX_SIZE + j] += temp * B[k * MATRIX_SIZE + j];
			}
		}
	}

	MPI_Gather(local_C.data(), local_rows * MATRIX_SIZE, MPI_INT,
			   C.data(), local_rows * MATRIX_SIZE, MPI_INT,
			   0, MPI_COMM_WORLD);

	auto end_time = std::chrono::high_resolution_clock::now();
	double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();

	if (rank == 0)
	{
		std::cout << "\nResult matrix C (part):\n";
		print_matrix(C, MATRIX_SIZE, MATRIX_SIZE);
		std::cout << "\nExecution time: " << elapsed_time << " seconds\n";
		std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << "\n";
		std::cout << "Processes used: " << size << "\n";
	}

	MPI_Finalize();
	return 0;
}