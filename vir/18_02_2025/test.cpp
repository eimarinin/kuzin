#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int main()
{
	std::system("chcp 65001 > nul");

	std::string image_path = "C:/Users/GIGABYTE/Downloads/cmake_asadagar.webp";
	Mat img = imread(image_path, IMREAD_COLOR);
	if (img.empty())
	{
		std::cout << "Ошибка при загрузке изображения!" << std::endl;
		return -1;
	}

	int gridSize;
	std::cout << "Введите размер сетки (2, 3, 4): ";
	std::cin >> gridSize;
	if (gridSize < 2 || gridSize > 4)
	{
		std::cout << "Некорректный ввод! Используем 2x2." << std::endl;
		gridSize = 2;
	}

	int segmentWidth = img.cols / gridSize;
	int segmentHeight = img.rows / gridSize;
	Mat result = img.clone();

	for (int i = 0; i < gridSize; ++i)
	{
		for (int j = 0; j < gridSize; ++j)
		{
			// Область интереса
			Rect roi(j * segmentWidth, i * segmentHeight, segmentWidth, segmentHeight);
			Mat segment = result(roi);

			int index = i * gridSize + j;

			if (index % 4 == 0)
			{
				segment.setTo(Scalar(0, 255, 0)); // Заливка зелёным
			}
			else if (index % 3 == 0)
			{
				Mat grayPart;
				cvtColor(segment, grayPart, COLOR_BGR2GRAY);
				cvtColor(grayPart, grayPart, COLOR_GRAY2BGR);
				grayPart.copyTo(segment);
			}
			else if (index % 2 == 1)
			{
				bitwise_not(segment, segment);
			}
		}
	}

	imshow("Оригинальное изображение", img);
	imshow("Обработанное изображение", result);

	waitKey(0);
	destroyAllWindows();

	return 0;
}
