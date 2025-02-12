#include <atomic>
#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#define TIMER_START(id) auto start_##id = std::chrono::high_resolution_clock::now();
#define TIMER_STOP(id) auto end_##id = std::chrono::high_resolution_clock::now();
#define TIMER_RESULT(id) std::chrono::duration_cast<std::chrono::microseconds>(end_##id - start_##id).count()

typedef std::vector<std::vector<int>> matrix_t;

class Matrix
{
public:
	Matrix() : dim(0), ref(nullptr), row_offset(0), col_offset(0) {}

	Matrix(int size) : Matrix()
	{
		dim = size;
		data.resize(dim, std::vector<int>(dim));
	}

	Matrix(Matrix &other) : Matrix(other.size())
	{
		for (int i = 0; i < size(); i++)
			for (int j = 0; j < size(); j++)
				data[i][j] = other[i][j];
	}

	Matrix(const Matrix *parent, int row, int col) : dim(parent->size() - 1), ref(parent), row_offset(row), col_offset(col) {}

	virtual ~Matrix() {}

	bool valid_position(int r, int c) const { return r >= 0 && c >= 0 && r < dim && c < dim; }

	int get(int r, int c) const
	{
		if (!valid_position(r, c))
			throw std::out_of_range("Invalid position");
		if (!ref)
			return data[r][c];
		if (r >= row_offset)
			r++;
		if (c >= col_offset)
			c++;
		return ref->get(r, c);
	}

	void set(int r, int c, int val)
	{
		if (!valid_position(r, c))
			throw std::out_of_range("Invalid position");
		if (!ref)
			data[r][c] = val;
		else
			throw std::runtime_error("Read-only matrix");
	}

	std::vector<int> &operator[](int i)
	{
		if (i < 0 || i >= dim)
			throw std::out_of_range("Invalid position");
		if (!ref)
			return data[i];
		throw std::runtime_error("Read-only matrix");
	}

	friend std::ostream &operator<<(std::ostream &out, const Matrix &m)
	{
		for (int i = 0; i < m.data.size(); i++)
		{
			for (int j = 0; j < m.data.size(); j++)
				out << m.get(i, j) << " ";
			out << "\n";
		}
		return out;
	}

	int size() const { return dim; }

	int determinant() const
	{
		if (dim == 1)
			return get(0, 0);

		int result = 0;
		for (int i = 0; i < dim; i++)
			result += (i % 2 ? -1 : 1) * get(i, 0) * minor_matrix(i, 0).determinant();
		return result;
	}

	int determinant_parallel() const
	{
		if (dim == 1)
			return get(0, 0);

		std::atomic<int> result = 0;
		std::vector<std::thread> tasks;
		for (int i = 0; i < dim; i++)
		{
			tasks.emplace_back(std::thread([this, i, &result]
										   {
                                               int temp = (i % 2 ? -1 : 1) * get(i, 0) * minor_matrix(i, 0).determinant();
                                               result.fetch_add(temp); }));
		}
		for (auto &t : tasks)
			t.join();

		return result;
	}

	Matrix minor_matrix(int r, int c) const { return Matrix(this, r, c); }

private:
	int dim;
	matrix_t data;
	const Matrix *ref;
	int row_offset, col_offset;
};

int main()
{
	std::ifstream input_file("matrix.txt");

	if (!input_file.is_open())
	{
		std::cout << "Error: Unable to open matrix.txt" << std::endl;
		return 1;
	}

	int size;
	input_file >> size;

	Matrix mat(size);

	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			input_file >> mat[i][j];

	std::cout << "Input Matrix:\n"
			  << mat;

	TIMER_START(seq)
	int det_seq = mat.determinant();
	TIMER_STOP(seq)

	TIMER_START(par)
	int det_par = mat.determinant_parallel();
	TIMER_STOP(par)

	assert(det_seq == det_par);

	std::cout << "Determinant (Sequential): " << det_seq << "\nTime: " << TIMER_RESULT(seq) << "us" << std::endl;
	std::cout << "Determinant (Parallel): " << det_par << "\nTime: " << TIMER_RESULT(par) << "us" << std::endl;

	return 0;
}
