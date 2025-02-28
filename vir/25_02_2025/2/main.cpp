#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

#ifdef _WIN32
#include <cstdlib>
#endif

int main()
{
#ifdef _WIN32
	// Устанавливаем кодировку консоли в UTF-8 для Windows
	std::system("chcp 65001 > nul");
#endif

	// Путь к изображению
	std::string imagePath = "./image.webp";

	// Загрузка изображения
	Mat image = imread(imagePath, IMREAD_COLOR);
	if (image.empty())
	{
		std::cerr << "Ошибка: не удалось загрузить изображение!" << std::endl;
		return -1;
	}

	// Клонируем исходное изображение
	Mat originalImage = image.clone();

	// Преобразование BGR в HSV
	Mat hsvImage;
	cvtColor(image, hsvImage, COLOR_BGR2HSV);

	// Обработка красного цвета
	Mat redMask1, redMask2, redMask;
	inRange(hsvImage, Scalar(0, 100, 100), Scalar(10, 255, 255), redMask1);
	inRange(hsvImage, Scalar(160, 100, 100), Scalar(180, 255, 255), redMask2);
	bitwise_or(redMask1, redMask2, redMask);

	Mat redResult;
	bitwise_and(originalImage, originalImage, redResult, redMask);

	// Обработка зелёного цвета
	Mat greenMask;
	inRange(hsvImage, Scalar(35, 50, 50), Scalar(85, 255, 255), greenMask);

	Mat greenResult;
	bitwise_and(originalImage, originalImage, greenResult, greenMask);

	// Отображение результатов
	imshow("Original Image", originalImage);
	imshow("Red Mask", redMask);
	imshow("Red Result", redResult);
	imshow("Green Mask", greenMask);
	imshow("Green Result", greenResult);

	waitKey(0);
	destroyAllWindows();

	return 0;
}
