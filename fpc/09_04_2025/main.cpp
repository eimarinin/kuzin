#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <complex>
#include <iostream>
#include <chrono>

#define IMG_WIDTH 800
#define IMG_HEIGHT 800
#define ITER_LIMIT 500

struct FractalPixel
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

int calculateFractalPoint(const std::complex<double> &point)
{
    std::complex<double> z(0, 0);
    int counter = 0;
    while (std::norm(z) <= 4.0 && counter < ITER_LIMIT)
    {
        z = z * z + point;
        ++counter;
    }
    return counter;
}

FractalPixel determinePixelColor(int counter)
{
    if (counter == ITER_LIMIT)
    {
        return {0, 0, 0};
    }
    return {
        static_cast<unsigned char>(8 * (255 - counter * 255 / ITER_LIMIT) % 256),
        static_cast<unsigned char>(15 * (255 - counter * 255 / ITER_LIMIT) % 256),
        static_cast<unsigned char>(9 * (255 - counter * 255 / ITER_LIMIT) % 256)};
}

void generateFractalImage(cv::Mat &outputImage)
{
    const double realStart = -2.0, realEnd = 1.0;
    const double imagStart = -1.5, imagEnd = 1.5;

    for (int row = 0; row < outputImage.rows; ++row)
    {
        for (int col = 0; col < outputImage.cols; ++col)
        {
            double real = realStart + (col / static_cast<double>(outputImage.cols)) * (realEnd - realStart);
            double imag = imagStart + (row / static_cast<double>(outputImage.rows)) * (imagEnd - imagStart);
            int counter = calculateFractalPoint(std::complex<double>(real, imag));
            outputImage.at<FractalPixel>(row, col) = determinePixelColor(counter);
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int processRank, processCount;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    if (processRank == 0)
    {
        cv::Mat referenceImage(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
        auto timeStart = std::chrono::high_resolution_clock::now();

        generateFractalImage(referenceImage);

        auto timeEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = timeEnd - timeStart;
        std::cout << "Reference computation took " << elapsed.count() << " seconds\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    auto parallelStart = std::chrono::high_resolution_clock::now();

    const int baseRows = IMG_HEIGHT / processCount;
    const int remainingRows = IMG_HEIGHT % processCount;
    const int localStartRow = processRank * baseRows + std::min(processRank, remainingRows);
    const int localRowCount = baseRows + (processRank < remainingRows ? 1 : 0);

    cv::Mat localImage(localRowCount, IMG_WIDTH, CV_8UC3);

    const double realMin = -2.0, realMax = 1.0;
    const double imagMin = -1.5, imagMax = 1.5;

    for (int y = 0; y < localRowCount; ++y)
    {
        for (int x = 0; x < IMG_WIDTH; ++x)
        {
            double real = realMin + (x / static_cast<double>(IMG_WIDTH)) * (realMax - realMin);
            double imag = imagMin + ((localStartRow + y) / static_cast<double>(IMG_HEIGHT)) * (imagMax - imagMin);
            int counter = calculateFractalPoint(std::complex<double>(real, imag));
            localImage.at<FractalPixel>(y, x) = determinePixelColor(counter);
        }
    }

    cv::Mat completeImage;
    if (processRank == 0)
    {
        completeImage = cv::Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
    }

    int *receiveCounts = nullptr;
    int *displacements = nullptr;
    if (processRank == 0)
    {
        receiveCounts = new int[processCount];
        displacements = new int[processCount];
        int currentOffset = 0;
        for (int i = 0; i < processCount; ++i)
        {
            int rows = baseRows + (i < remainingRows ? 1 : 0);
            receiveCounts[i] = rows * IMG_WIDTH * 3;
            displacements[i] = currentOffset;
            currentOffset += receiveCounts[i];
        }
    }

    MPI_Gatherv(localImage.data, localRowCount * IMG_WIDTH * 3, MPI_UNSIGNED_CHAR,
                completeImage.data, receiveCounts, displacements, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    auto parallelEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelDuration = parallelEnd - parallelStart;

    if (processRank == 0)
    {
        std::cout << "Parallel computation completed in " << parallelDuration.count() << " seconds\n";
        cv::imshow("Fractal Visualization", completeImage);
        cv::waitKey(0);
        delete[] receiveCounts;
        delete[] displacements;
    }

    MPI_Finalize();
    return 0;
}