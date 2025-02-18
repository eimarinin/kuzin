#include <opencv2/opencv.hpp>
#include <iostream>
#include <locale>
#include <codecvt>

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

	// Координаты начала и конца линии
	int x1 = 50, y1 = 50;	// Начальная точка
	int x2 = 300, y2 = 300; // Конечная точка

	// Цвет (BGR) и толщина линии
	Scalar color(0, 0, 255); // Красная линия
	int thickness = 2;

	line(img, Point(x1, y1), Point(x2, y2), color, thickness);

	imshow("Окно с изображением", img);

	waitKey(0);
	destroyAllWindows();

	return 0;
}