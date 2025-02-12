#include <opencv2/opencv.hpp>
#include <iostream>
#include <locale>
#include <codecvt>

using namespace cv;

int main()
{
	std::system("chcp 65001 > nul");

	// Путь к изображению
	std::string image_path = "C:/Users/GIGABYTE/Downloads/cmake_asadagar.webp"; // Замените на актуальный путь к изображению

	// Загрузка изображения
	Mat img = imread(image_path, IMREAD_COLOR);

	// Проверка, удалось ли загрузить изображение
	if (img.empty())
	{
		std::cout << "Ошибка при загрузке изображения!" << std::endl;
		return -1; // Возврат с ошибкой
	}

	// Показ изображения в окне
	imshow("Окно с изображением", img); // Отображение окна с русским названием

	// Ожидание нажатия клавиши
	int k = waitKey(0); // Ожидает нажатия клавиши

	// Закрытие всех окон
	destroyAllWindows();

	return 0;
}
